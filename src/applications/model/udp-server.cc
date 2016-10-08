/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "packet-loss-counter.h"

#include "seq-ts-header.h"
#include "udp-server.h"
#include <iterator>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpServer");

NS_OBJECT_ENSURE_REGISTERED (UdpServer);


TypeId
UdpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&UdpServer::GetPacketWindowSize,
                                         &UdpServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
    .AddAttribute ("ReceivedPacketsFilename",
                   "Name of the file where the number of received packets will be saved.",
                   StringValue ("UdpReceived.txt"),
                   MakeStringAccessor (&UdpServer::SetReceivedFilename),
                   MakeStringChecker ())
    .AddAttribute ("ReceivedSnFilename",
                   "Name of the file where the sequence numbers of received packets will be periodically written.",
                   StringValue ("UdpSn.txt"),
                   MakeStringAccessor (&UdpServer::SetSnFilename),
                   MakeStringChecker ())
    .AddAttribute ("SnVectorMaxSize",
               "Maximum size of the vector containing SNs. When it reaches this size, the SNs are written to file",
               UintegerValue (1024),
               MakeUintegerAccessor (&UdpServer::m_snMaxSize),
               MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

UdpServer::UdpServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received=0;
  m_snVector.clear();
}

UdpServer::~UdpServer ()
{
  NS_LOG_FUNCTION (this);
}

void 
UdpServer::SetReceivedFilename(std::string name)
{
  m_receivedFilename = name;
}

std::string 
UdpServer::GetReceivedFilename() const
{
  return m_receivedFilename;
}

void 
UdpServer::SetSnFilename(std::string name)
{
  m_snFilename = name;
}

std::string 
UdpServer::GetSnFilename() const
{
  return m_snFilename;
}

uint16_t
UdpServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
UdpServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
UdpServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint32_t
UdpServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

void
UdpServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  if(!m_udpReceivedFile.is_open())
  {
    m_udpReceivedFile.open(GetReceivedFilename().c_str());
  }
  m_udpReceivedFile << m_received << std::endl;
  m_udpReceivedFile.close();

  if(m_snVector.size() > 0)
  {
    if(!m_udpSnFile.is_open())
    {
      m_udpSnFile.open(GetSnFilename().c_str(), std::ofstream::app);
    }
    std::ostream_iterator<uint32_t> snIterator(m_udpSnFile, "\n");
    std::copy(m_snVector.begin(), m_snVector.end(), snIterator);
    m_snVector.clear();
    m_udpSnFile.close();
  }

  Application::DoDispose ();
}

void
UdpServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      m_socket->Bind (local);
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpServer::HandleRead, this));

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (),
                                                   m_port);
      m_socket6->Bind (local);
    }

  m_socket6->SetRecvCallback (MakeCallback (&UdpServer::HandleRead, this));

}

void
UdpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
UdpServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();
          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }
          else if (Inet6SocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }

          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;
          m_snVector.push_back(currentSequenceNumber);
          if(m_snVector.size() >= m_snMaxSize)
          {
            // write to file
            if(!m_udpSnFile.is_open())
            {
              m_udpSnFile.open(GetSnFilename().c_str(), std::ofstream::app);
            }
            std::ostream_iterator<uint32_t> snIterator(m_udpSnFile, "\n");
            std::copy(m_snVector.begin(), m_snVector.end(), snIterator);
            m_snVector.clear();
            m_udpSnFile.close();
          }
        }
      else
        {
          NS_LOG_INFO("Packet dropped in UDP server, size = 0");
        }
    }
}

} // Namespace ns3
