/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/* *
 * Copyright (c) 2024 Orange Innovation Poland
 * Copyright (c) 2024 Orange Innovation Egypt
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
 * Authors: Andrea Lacava <thecave003@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Argha Sen <arghasen10@gmail.com>
 *          Kamil Kociszewski <kamil.kociszewski@orange.com>
 *          Mostafa Ashraf <mostafa.ashraf.ext@orange.com>
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include <ns3/lte-ue-net-device.h>
#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "../src/mmwave/model/node-container-manager.h"
#include "ns3/lte-helper.h"
#include <sys/time.h>
#include <ctime>
#include <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include <random>
#include <chrono>
#include <cmath>
#include <fstream>

using namespace ns3;
using namespace mmwave;

std::map<uint64_t, uint16_t> imsi_cellid;
std::map<uint16_t, std::set<uint64_t>> imsi_list;
std::map<uint16_t, Ptr<Node>> cellid_node;
std::map<uint32_t, uint16_t> ue_cellid_usinghandover;
std::map<uint64_t, uint32_t> ueimsi_nodeid;
int ue_assoc_list[10] = {0};
double maxXAxis;
double maxYAxis;
bool esON_list[10] = {0};

NS_LOG_COMPONENT_DEFINE ("ScenarioThree_ES");

void
PrintGnuplottableUeListToFile (std::string filename)
{
    std::ofstream outFile;
    outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
    if (!outFile.is_open ())
    {
        NS_LOG_ERROR ("Can't open file " << filename);
        return;
    }
    for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
        Ptr<Node> node = *it;
        int nDevs = node->GetNDevices ();
        for (int j = 0; j < nDevs; j++)
        {
            Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject<LteUeNetDevice> ();
            Ptr<MmWaveUeNetDevice> mmuedev = node->GetDevice (j)->GetObject<MmWaveUeNetDevice> ();
            Ptr<McUeNetDevice> mcuedev = node->GetDevice (j)->GetObject<McUeNetDevice> ();
            if (uedev)
            {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                outFile << "set label \"" << uedev->GetImsi () << "\" at " << pos.x << "," << pos.y
                        << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 1 ps "
                           "0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;
            }
            else if (mmuedev)
            {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                outFile << "set label \"" << mmuedev->GetImsi () << "\" at " << pos.x << "," << pos.y
                        << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 1 ps "
                           "0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;
            }
            else if (mcuedev)
            {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                outFile << "set label \"" << mcuedev->GetImsi () << "\" at " << pos.x << "," << pos.y
                        << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 1 ps "
                           "0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;
            }
        }
    }
}

void
PrintGnuplottableEnbListToFile (uint64_t m_startTime)
{

    //uint64_t m_startTime = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    uint64_t timestamp = m_startTime + (uint64_t) Simulator::Now ().GetMilliSeconds ();
    //
    std::string filename1 = "enbs.txt";
    std::string filename2 = "gnbs.txt";
    //

    for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
        Ptr<Node> node = *it;
        int nDevs = node->GetNDevices ();
        for (int j = 0; j < nDevs; j++)
        {
            Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject<LteEnbNetDevice> ();
            Ptr<MmWaveEnbNetDevice> mmdev = node->GetDevice (j)->GetObject<MmWaveEnbNetDevice> ();
            if (enbdev)
            {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                std::ofstream outFile1;
                outFile1.open (filename1.c_str (), std::ios_base::out | std::ios_base::app);
                if (!outFile1.is_open ())
                {
                    NS_LOG_ERROR ("Can't open file " << filename1);
                    return;
                }
                //outFile1 << timestamp << "," << enbdev->GetCellId() << "," << pos.x << "," << pos.y << pos.z << std::endl;
                outFile1 << timestamp << "," << enbdev->GetCellId() << "," << pos.x << "," << pos.y << ","
                         << m_startTime << "," << "0" << "," << "30" << std::endl;
                outFile1.close ();
            }
            else if (mmdev)
            {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                std::ofstream outFile2;
                outFile2.open (filename2.c_str (), std::ios_base::out | std::ios_base::app);
                if (!outFile2.is_open ())
                {
                    NS_LOG_ERROR ("Can't open file " << filename2);
                    return;
                }
                auto ueMap = mmdev->GetUeMap ();
                Ptr<MmWaveEnbPhy> enbPhy = node->GetDevice (j)->GetObject<MmWaveEnbNetDevice> ()->GetPhy ();
                double es_power = enbPhy->GetTxPower ();
                for (const auto &ue : ueMap)
                {
                    uint64_t imsi_assoc = ue.second->GetImsi ();
                    //NS_LOG_UNCOND ("IMSI: " << imsi_assoc << " associated with cell: "  << mmdev->GetCellId ());
                    ue_assoc_list[imsi_assoc - 1] = mmdev->GetCellId ();
                }
                uint16_t cell_id = mmdev->GetCellId ();

                //outFile2 << timestamp << "," << enbdev->GetCellId() << "," << pos.x << "," << pos.y << pos.z << std::endl;
                outFile2 << timestamp << "," << cell_id << "," << pos.x << "," << pos.y << ","
                         << m_startTime << "," << esON_list[cell_id] << "," << es_power << std::endl;
                outFile2.close ();
            }
        }
    }
}

void
ClearFile (std::string Filename, uint64_t m_startTime)
{
    std::string filename = Filename;
    std::ofstream outFile;
    outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
    if (!outFile.is_open ())
    {
        NS_LOG_ERROR ("Can't open file " << filename);
        return;
    }
    outFile.close ();
    //  struct timeval time_now{};
    //  gettimeofday (&time_now, nullptr);
    //uint64_t m_startTime = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    uint64_t timestamp = m_startTime + (uint64_t) Simulator::Now ().GetMilliSeconds ();
    std::ofstream outFile1;
    outFile1.open (filename.c_str (), std::ios_base::out | std::ios_base::app);

    if (Filename == "ue_position.txt")
    {
        outFile1 << "timestamp,id,x,y,type,cell,simid" << std::endl;
    }
    else
    {
        outFile1 << "timestamp,id,x,y,simid,ESstate,ESpower" << std::endl;
        outFile1 << timestamp << "," << "0" << "," << maxXAxis << "," << maxYAxis << std::endl;
    }
    outFile1.close ();
}

/*void
SetBSTX (Ptr<MmWaveEnbPhy> phy, int val, uint16_t cellid, bool m_esON)
{
    if (val == 0)
    {

        NS_LOG_UNCOND ("Cell turned off " << cellid);
    }
    else
    {
        NS_LOG_UNCOND ("Cell turned on " << cellid);
    }
    esON_list[cellid] = m_esON; //ES status flag
    phy->SetTxPower (val); //set Cell TX power
    if (val == 0)
    {
        phy->SetNoiseFigure (100); //high noise
    }
    else
    {
        phy->SetNoiseFigure (5); //default
    }
}*/
void
GetESStates (Ptr<MmWaveEnbNetDevice> mmdev, uint16_t cellid)
{
    esON_list[cellid] = mmdev->GetESStates ();
    NS_LOG_UNCOND ("Current ES state " << esON_list[cellid] << " for cell " << cellid);
}

void
PrintPosition (Ptr<Node> node, int iterator, std::string Filename, uint64_t m_startTime)
{

    //uint64_t m_startTime = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    uint64_t timestamp = m_startTime + (uint64_t) Simulator::Now ().GetMilliSeconds ();

    int imsi;
    Ptr<Node> node1 = NodeList::GetNode (iterator);
    int nDevs = node->GetNDevices ();
    std::string filename = Filename;
    std::ofstream outFile;
    for (int j = 0; j < nDevs; j++)
    {
        Ptr<McUeNetDevice> mcuedev = node1->GetDevice (j)->GetObject<McUeNetDevice> ();
        Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject<LteUeNetDevice> ();
        Ptr<MmWaveUeNetDevice> mmuedev = node->GetDevice (j)->GetObject<MmWaveUeNetDevice> ();
        if (mcuedev)
        {
            imsi = int (mcuedev->GetImsi ());
            if (ue_assoc_list[imsi - 1] == 0)
            {
                //NS_LOG_UNCOND ("Position of UE not stored, UE not associated to any cell!");
                return;
            }
            Ptr<MobilityModel> model = node->GetObject<MobilityModel> ();
            Vector position = model->GetPosition ();
            NS_LOG_UNCOND ("Position of UE with IMSI " << imsi << " is " << model->GetPosition ()
                                                       << " at time "
                                                       << Simulator::Now ().GetSeconds ());

            outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::app);
            if (!outFile.is_open ())
            {
                NS_LOG_ERROR ("Can't open file " << filename);
                return;
            }
            int serving_cell = ue_assoc_list[imsi - 1];

            outFile << timestamp << "," << imsi << "," << position.x << "," << position.y << ",mc,"
                    << serving_cell << "," << m_startTime << std::endl;
            outFile.close ();
        }
        else
        {
            //
        }
    }
}

static ns3::GlobalValue g_bufferSize ("bufferSize", "RLC tx buffer size (MB)",
                                      ns3::UintegerValue (10),
                                      ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_enableTraces ("enableTraces", "If true, generate ns-3 traces",
                                        ns3::BooleanValue (true), ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_e2lteEnabled ("e2lteEnabled", "If true, send LTE E2 reports",
                                        ns3::BooleanValue (true), ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_e2nrEnabled ("e2nrEnabled", "If true, send NR E2 reports",
                                       ns3::BooleanValue (true), ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_e2du ("e2du", "If true, send DU reports", ns3::BooleanValue (true),
                                ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_e2cuUp ("e2cuUp", "If true, send CU-UP reports", ns3::BooleanValue (true),
                                  ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_e2cuCp ("e2cuCp", "If true, send CU-CP reports", ns3::BooleanValue (true),
                                  ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_reducedPmValues ("reducedPmValues",
                                           "If true, use a subset of the the pm containers",
                                           ns3::BooleanValue (true), ns3::MakeBooleanChecker ());

static ns3::GlobalValue
        g_hoSinrDifference ("hoSinrDifference",
                            "The value for which an handover between MmWave eNB is triggered",
                            ns3::DoubleValue (5), ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue
        g_indicationPeriodicity ("indicationPeriodicity",
                                 "E2 Indication Periodicity reports (value in seconds)",
                                 ns3::DoubleValue (0.1), ns3::MakeDoubleChecker<double> (0.01, 2.0));

static ns3::GlobalValue g_simTime ("simTime", "Simulation time in seconds", ns3::DoubleValue (1000),
                                   ns3::MakeDoubleChecker<double> (0.1, 100000.0));

static ns3::GlobalValue
        g_outageThreshold ("outageThreshold",
                           "SNR threshold for outage events [dB]", // use -1000.0 with NoAuto
                           ns3::DoubleValue (-10.0), ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_numberOfRaPreambles (
        "numberOfRaPreambles",
        "how many random access preambles are available for the contention based RACH process",
        ns3::UintegerValue (40), // Indicated for TS use case, 52 is default
        ns3::MakeUintegerChecker<uint8_t> ());

static ns3::GlobalValue
        g_handoverMode ("handoverMode",
                        "HO euristic to be used,"
                        "can be only \"NoAuto\", \"FixedTtt\", \"DynamicTtt\",   \"Threshold\"",
                        ns3::StringValue ("DynamicTtt"), ns3::MakeStringChecker ());

static ns3::GlobalValue g_e2TermIp ("e2TermIp", "The IP address of the RIC E2 termination",
                                    ns3::StringValue ("127.0.0.1"), ns3::MakeStringChecker ());

static ns3::GlobalValue
        g_enableE2FileLogging ("enableE2FileLogging",
                               "If true, generate offline file logging instead of connecting to RIC",
                               ns3::BooleanValue (false), ns3::MakeBooleanChecker ());
static ns3::GlobalValue
        g_e2andlog ("E2andLogging",
                    "If true, generate offline file logging instead of connecting to RIC",
                    ns3::BooleanValue (false), ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_e2_func_id ("KPM_E2functionID", "Function ID to subscribe",
                                      ns3::DoubleValue (2), ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_rc_e2_func_id ("RC_E2functionID", "Function ID to subscribe",
                                         ns3::DoubleValue (3), ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_controlFileName ("controlFileName",
                                           "The path to the control file (can be absolute)",
                                           ns3::StringValue (""), ns3::MakeStringChecker ());

// TODO: running flags
static ns3::GlobalValue mmWave_nodes ("N_MmWaveEnbNodes", "Number of mmWaveNodes",
                                      ns3::UintegerValue (4), ns3::MakeUintegerChecker<uint8_t> ());
// TODO: next step(make it in correct way, regarding to position)
// static ns3::GlobalValue lteEnb_nodes ("N_LteEnbNodes", "Number of LteEnbNodes",
//                                       ns3::UintegerValue (1),
//                                       ns3::MakeUintegerChecker<uint8_t> ());

static ns3::GlobalValue ue_s ("N_Ues", "Number of User Equipments", ns3::UintegerValue (3),
                              ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue center_freq ("CenterFrequency", "Center Frequency Value",
                                     ns3::DoubleValue (3.5e9), ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue bandwidth_value ("Bandwidth", "Bandwidth Value", ns3::DoubleValue (20e6),
                                         ns3::MakeDoubleChecker<double> ());
// TODO: check for later
// static ns3::GlobalValue num_antennas_McUe ("N_AntennasMcUe", "Number of Antenna as McUe",
//                                       ns3::IntegerValue (1),
//                                       ns3::MakeIntegerChecker<int> ());

// static ns3::GlobalValue num_antennas_MmWave ("N_AntennasMmWave", "Number of Antenna as MmWave",
//                                       ns3::IntegerValue (1),
//                                       ns3::MakeIntegerChecker<int> ());

static ns3::GlobalValue interside_distance_value_ue ("IntersideDistanceUEs",
                                                     "Interside Distance Value",
                                                     ns3::DoubleValue (1000),
                                                     ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue interside_distance_value_cell ("IntersideDistanceCells",
                                                       "Interside Distance Value",
                                                       ns3::DoubleValue (1000),
                                                       ns3::MakeDoubleChecker<double> ());

int
main (int argc, char *argv[])
{
    LogComponentEnableAll (LOG_PREFIX_ALL);
    //  LogComponentEnable ("RicControlMessage", LOG_LEVEL_ALL);
    //  LogComponentEnable ("KpmIndication", LOG_LEVEL_DEBUG);
    LogComponentEnable ("KpmIndication", LOG_LEVEL_INFO);

    // LogComponentEnable ("Asn1Types", LOG_LEVEL_LOGIC);
    //   LogComponentEnable ("E2Termination", LOG_LEVEL_LOGIC);
    //  LogComponentEnable ("E2Termination", LOG_LEVEL_DEBUG);

    // LogComponentEnable ("LteEnbNetDevice", LOG_LEVEL_ALL);
    //LogComponentEnable ("MmWaveEnbNetDevice", LOG_LEVEL_DEBUG);

    // The maximum X coordinate of the scenario

    maxXAxis = 4000;
    // The maximum Y coordinate of the scenario
    maxYAxis = 4000;

    // Command line arguments
    CommandLine cmd;
    cmd.Parse (argc, argv);

    bool harqEnabled = true;

    UintegerValue uintegerValue;
    BooleanValue booleanValue;
    StringValue stringValue;
    DoubleValue doubleValue;

    GlobalValue::GetValueByName ("hoSinrDifference", doubleValue);
    double hoSinrDifference = doubleValue.Get ();
    GlobalValue::GetValueByName ("bufferSize", uintegerValue);
    uint32_t bufferSize = uintegerValue.Get ();
    GlobalValue::GetValueByName ("enableTraces", booleanValue);
    bool enableTraces = booleanValue.Get ();
    GlobalValue::GetValueByName ("outageThreshold", doubleValue);
    double outageThreshold = doubleValue.Get ();
    GlobalValue::GetValueByName ("handoverMode", stringValue);
    std::string handoverMode = stringValue.Get ();
    GlobalValue::GetValueByName ("e2TermIp", stringValue);
    std::string e2TermIp = stringValue.Get ();
    GlobalValue::GetValueByName ("enableE2FileLogging", booleanValue);
    bool enableE2FileLogging = booleanValue.Get ();
    GlobalValue::GetValueByName ("E2andLogging", booleanValue);
    bool g_e2andlog = booleanValue.Get ();
    GlobalValue::GetValueByName ("KPM_E2functionID", doubleValue);
    double g_e2_func_id = doubleValue.Get ();
    GlobalValue::GetValueByName ("RC_E2functionID", doubleValue);
    double g_rc_e2_func_id = doubleValue.Get ();

    if (enableE2FileLogging && g_e2andlog)
    {
        NS_LOG_ERROR (
                "Only one of this variables can be set to TRUE - enableE2FileLogging && g_e2andlog");
    }

    GlobalValue::GetValueByName ("numberOfRaPreambles", uintegerValue);
    uint8_t numberOfRaPreambles = uintegerValue.Get ();

    NS_LOG_UNCOND ("bufferSize " << bufferSize << " OutageThreshold " << outageThreshold
                                 << " HandoverMode " << handoverMode << " e2TermIp " << e2TermIp
                                 << " enableE2FileLogging " << enableE2FileLogging << " E2andLogging "
                                 << g_e2andlog << " E2 Function ID " << g_e2_func_id);

    GlobalValue::GetValueByName ("e2lteEnabled", booleanValue);
    bool e2lteEnabled = booleanValue.Get ();
    GlobalValue::GetValueByName ("e2nrEnabled", booleanValue);
    bool e2nrEnabled = booleanValue.Get ();
    GlobalValue::GetValueByName ("e2du", booleanValue);
    bool e2du = booleanValue.Get ();
    GlobalValue::GetValueByName ("e2cuUp", booleanValue);
    bool e2cuUp = booleanValue.Get ();
    GlobalValue::GetValueByName ("e2cuCp", booleanValue);
    bool e2cuCp = booleanValue.Get ();

    GlobalValue::GetValueByName ("reducedPmValues", booleanValue);
    bool reducedPmValues = booleanValue.Get ();

    GlobalValue::GetValueByName ("indicationPeriodicity", doubleValue);
    double indicationPeriodicity = doubleValue.Get ();
    GlobalValue::GetValueByName ("controlFileName", stringValue);
    std::string controlFilename = stringValue.Get ();

    NS_LOG_UNCOND ("e2lteEnabled " << e2lteEnabled << " e2nrEnabled " << e2nrEnabled << " e2du "
                                   << e2du << " e2cuCp " << e2cuCp << " e2cuUp " << e2cuUp
                                   << " controlFilename " << controlFilename
                                   << " indicationPeriodicity " << indicationPeriodicity);

    Config::SetDefault ("ns3::LteEnbNetDevice::ControlFileName", StringValue (controlFilename));
    Config::SetDefault ("ns3::LteEnbNetDevice::E2Periodicity", DoubleValue (indicationPeriodicity));
    Config::SetDefault ("ns3::MmWaveEnbNetDevice::E2Periodicity",
                        DoubleValue (indicationPeriodicity));

    Config::SetDefault ("ns3::MmWaveHelper::E2ModeLte", BooleanValue (e2lteEnabled));
    Config::SetDefault ("ns3::MmWaveHelper::E2ModeNr", BooleanValue (e2nrEnabled));

    // The DU PM reports should come from both NR gNB as well as LTE eNB,
    // since in the RLC/MAC/PHY entities are present in BOTH NR gNB as well as LTE eNB.
    // DU reports from LTE eNB are not implemented in this release
    Config::SetDefault ("ns3::MmWaveEnbNetDevice::EnableDuReport", BooleanValue (e2du));

    // The CU-UP PM reports should only come from LTE eNB, since in the NS3 “EN-DC
    // simulation (Option 3A)”, the PDCP is only in the LTE eNB and NOT in the NR gNB
    Config::SetDefault ("ns3::MmWaveEnbNetDevice::EnableCuUpReport", BooleanValue (e2cuUp));
    Config::SetDefault ("ns3::LteEnbNetDevice::EnableCuUpReport", BooleanValue (e2cuUp));

    Config::SetDefault ("ns3::MmWaveEnbNetDevice::EnableCuCpReport", BooleanValue (e2cuCp));
    Config::SetDefault ("ns3::LteEnbNetDevice::EnableCuCpReport", BooleanValue (e2cuCp));

    Config::SetDefault ("ns3::MmWaveEnbNetDevice::ReducedPmValues", BooleanValue (reducedPmValues));
    Config::SetDefault ("ns3::LteEnbNetDevice::ReducedPmValues", BooleanValue (reducedPmValues));

    Config::SetDefault ("ns3::LteEnbNetDevice::EnableE2FileLogging",
                        BooleanValue (enableE2FileLogging));
    Config::SetDefault ("ns3::MmWaveEnbNetDevice::EnableE2FileLogging",
                        BooleanValue (enableE2FileLogging));

    Config::SetDefault ("ns3::MmWaveEnbNetDevice::E2andLogging", BooleanValue (g_e2andlog));
    Config::SetDefault ("ns3::LteEnbNetDevice::e2andLogging", BooleanValue (g_e2andlog));

    Config::SetDefault ("ns3::LteEnbNetDevice::KPM_E2functionID", DoubleValue (g_e2_func_id));
    Config::SetDefault ("ns3::MmWaveEnbNetDevice::KPM_E2functionID", DoubleValue (g_e2_func_id));

    Config::SetDefault ("ns3::LteEnbNetDevice::RC_E2functionID", DoubleValue (g_rc_e2_func_id));

    Config::SetDefault ("ns3::MmWaveEnbMac::NumberOfRaPreambles",
                        UintegerValue (numberOfRaPreambles));

    Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (harqEnabled));
    Config::SetDefault ("ns3::MmWaveHelper::UseIdealRrc", BooleanValue (true));
    Config::SetDefault ("ns3::MmWaveHelper::E2TermIp", StringValue (e2TermIp));

    Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (harqEnabled));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::NumHarqProcess", UintegerValue (100));
    //Config::SetDefault ("ns3::MmWaveBearerStatsCalculator::EpochDuration", TimeValue (MilliSeconds (10.0)));

    // set to false to use the 3GPP radiation pattern (proper configuration of the bearing and downtilt angles is needed)
    Config::SetDefault ("ns3::ThreeGppAntennaArrayModel::IsotropicElements", BooleanValue (true));
    Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (100.0)));
    Config::SetDefault ("ns3::ThreeGppChannelConditionModel::UpdatePeriod",
                        TimeValue (MilliSeconds (100)));

    Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds (10.0)));
    Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer",
                        TimeValue (MilliSeconds (10.0)));
    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));
    Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize",
                        UintegerValue (bufferSize * 1024 * 1024));
    Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (bufferSize * 1024 * 1024));

    Config::SetDefault ("ns3::LteEnbRrc::OutageThreshold", DoubleValue (outageThreshold));
    Config::SetDefault ("ns3::LteEnbRrc::SecondaryCellHandoverMode", StringValue (handoverMode));
    Config::SetDefault ("ns3::LteEnbRrc::HoSinrDifference", DoubleValue (hoSinrDifference));

    // Carrier bandwidth in Hz
    GlobalValue::GetValueByName ("Bandwidth", doubleValue);
    double bandwidth = doubleValue.Get ();
    // Center frequency in Hz
    GlobalValue::GetValueByName ("CenterFrequency", doubleValue);
    double centerFrequency = doubleValue.Get ();
    // Distance between the mmWave BSs and the two co-located LTE and mmWave BSs in meters
    GlobalValue::GetValueByName ("IntersideDistanceUEs", doubleValue);
    double isd_ue = doubleValue.Get (); // (interside distance)
    GlobalValue::GetValueByName ("IntersideDistanceCells", doubleValue);
    double isd_cell = doubleValue.Get (); // (interside distance)

    // Number of antennas in each UE
    // GlobalValue::GetValueByName ("N_AntennasMcUe", uintegerValue);
    int numAntennasMcUe = 1; //uintegerValue.Get();
    // Number of antennas in each mmWave BS
    // GlobalValue::GetValueByName ("N_AntennasMmWave", uintegerValue);
    int numAntennasMmWave = 1; //uintegerValue.Get();

    NS_LOG_INFO ("Bandwidth " << bandwidth << " centerFrequency " << double (centerFrequency)
                              << " isd_ue " << isd_ue << " numAntennasMcUe " << numAntennasMcUe
                              << " numAntennasMmWave " << numAntennasMmWave);

    // Set the number of antennas in the devices
    Config::SetDefault ("ns3::McUeNetDevice::AntennaNum", UintegerValue (numAntennasMcUe));
    Config::SetDefault ("ns3::MmWaveNetDevice::AntennaNum", UintegerValue (numAntennasMmWave));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue (bandwidth));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue (centerFrequency));

    Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
    mmwaveHelper->SetPathlossModelType ("ns3::ThreeGppUmiStreetCanyonPropagationLossModel");
    mmwaveHelper->SetChannelConditionModelType ("ns3::ThreeGppUmiStreetCanyonChannelConditionModel");

    Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
    mmwaveHelper->SetEpcHelper (epcHelper);

    GlobalValue::GetValueByName ("N_MmWaveEnbNodes", uintegerValue);
    uint8_t nMmWaveEnbNodes = uintegerValue.Get ();
    //uint8_t nMmWaveEnbNodes = 2;
    // GlobalValue::GetValueByName ("N_LteEnbNodes", uintegerValue);
    uint8_t nLteEnbNodes = 1; //uintegerValue.Get();
    GlobalValue::GetValueByName ("N_Ues", uintegerValue);
    uint32_t ues = uintegerValue.Get ();
    // TODO: discuss number of UEs implementation
    //uint8_t nUeNodes = ues * nMmWaveEnbNodes;
    uint8_t nUeNodes = ues;
    NS_LOG_INFO (" Bandwidth " << bandwidth << " centerFrequency " << double (centerFrequency)
                               << " isd_cell " << isd_cell << " numAntennasMcUe " << numAntennasMcUe
                               << " numAntennasMmWave " << numAntennasMmWave << " nMmWaveEnbNodes "
                               << unsigned (nMmWaveEnbNodes));

    // Get SGW/PGW and create a single RemoteHost
    Ptr<Node> pgw = epcHelper->GetPgwNode ();
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (1);
    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    InternetStackHelper internet;
    internet.Install (remoteHostContainer);

    // Create the Internet by connecting remoteHost to pgw. Setup routing too
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
    // interface 0 is localhost, 1 is the p2p device
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
            ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    // create LTE, mmWave eNB nodes and UE node
    NodeContainer ueNodes;
    NodeContainer mmWaveEnbNodes;
    NodeContainer lteEnbNodes;
    NodeContainer allEnbNodes;
    mmWaveEnbNodes.Create (nMmWaveEnbNodes);
    lteEnbNodes.Create (nLteEnbNodes);
    ueNodes.Create (nUeNodes);
    allEnbNodes.Add (lteEnbNodes);
    allEnbNodes.Add (mmWaveEnbNodes);
    NodeContainerManager::GetInstance().SetMmWaveEnbNodes(mmWaveEnbNodes);
    // Position
    Vector centerPosition = Vector (maxXAxis / 2, maxYAxis / 2, 3);

    // Install Mobility Model
    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();

    // We want a center with one LTE enb and one mmWave co-located in the same place
    enbPositionAlloc->Add (centerPosition);
    enbPositionAlloc->Add (centerPosition);
    //enbPositionAlloc->Add (Vector (centerPosition.x - 500, centerPosition.y, 3));
    double x, y;
    double nConstellation = nMmWaveEnbNodes - 1;

    //// This guarantee that each of the rest BSs is placed at the same distance from the two co-located in the center
    for (int8_t i = 0; i < nConstellation; ++i)
    {
        x = isd_cell * cos ((2 * M_PI * i) / (nConstellation));
        y = isd_cell * sin ((2 * M_PI * i) / (nConstellation));
        enbPositionAlloc->Add (Vector (centerPosition.x + x, centerPosition.y + y, 3));
    }

    MobilityHelper enbmobility;
    enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    enbmobility.SetPositionAllocator (enbPositionAlloc);
    enbmobility.Install (allEnbNodes);

    MobilityHelper uemobility;

    Ptr<UniformDiscPositionAllocator> uePositionAlloc = CreateObject<UniformDiscPositionAllocator> ();

    uePositionAlloc->SetX (centerPosition.x);
    uePositionAlloc->SetY (centerPosition.y);
    uePositionAlloc->SetRho (isd_ue);
    Ptr<UniformRandomVariable> speed = CreateObject<UniformRandomVariable> ();
    speed->SetAttribute ("Min", DoubleValue (2.0));
    speed->SetAttribute ("Max", DoubleValue (4.0));

    uemobility.SetMobilityModel ("ns3::RandomWalk2dOutdoorMobilityModel", "Speed",
                                 PointerValue (speed), "Bounds",
                                 RectangleValue (Rectangle (0, maxXAxis, 0, maxYAxis)));
    uemobility.SetPositionAllocator (uePositionAlloc);
    uemobility.Install (ueNodes);

    // Install mmWave, lte, mc Devices to the nodesnumAntennasMmWave
    NetDeviceContainer lteEnbDevs = mmwaveHelper->InstallLteEnbDevice (lteEnbNodes);
    NetDeviceContainer mmWaveEnbDevs = mmwaveHelper->InstallEnbDevice (mmWaveEnbNodes);
    NetDeviceContainer mcUeDevs = mmwaveHelper->InstallMcUeDevice (ueNodes);

    // Install the IP stack on the UEs
    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (mcUeDevs));
    // Assign IP address to UEs, and install applications
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        Ptr<Node> ueNode = ueNodes.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting =
                ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    // Add X2 interfaces
    mmwaveHelper->AddX2Interface (lteEnbNodes, mmWaveEnbNodes);

    // Manual attachment
    mmwaveHelper->AttachToClosestEnb (mcUeDevs, mmWaveEnbDevs, lteEnbDevs);

    // Install and start applications
    // On the remoteHost there is UDP OnOff Application

    uint16_t portUdp = 60000;
    Address sinkLocalAddressUdp (InetSocketAddress (Ipv4Address::GetAny (), portUdp));
    PacketSinkHelper sinkHelperUdp ("ns3::UdpSocketFactory", sinkLocalAddressUdp);
    AddressValue serverAddressUdp (InetSocketAddress (remoteHostAddr, portUdp));

    ApplicationContainer sinkApp;
    sinkApp.Add (sinkHelperUdp.Install (remoteHost));

    ApplicationContainer clientApp;

    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        // Full traffic
        PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                             InetSocketAddress (Ipv4Address::GetAny (), 1234));
        sinkApp.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));
        UdpClientHelper dlClient (ueIpIface.GetAddress (u), 1234);
        dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (500)));
        dlClient.SetAttribute ("MaxPackets", UintegerValue (UINT32_MAX));
        dlClient.SetAttribute ("PacketSize", UintegerValue (1280));
        clientApp.Add (dlClient.Install (remoteHost));
    }

    // Start applications
    GlobalValue::GetValueByName ("simTime", doubleValue);
    double simTime = doubleValue.Get ();
    sinkApp.Start (Seconds (0));

    clientApp.Start (MilliSeconds (100));
    clientApp.Stop (Seconds (simTime - 0.1));

    struct timeval time_now{};
    gettimeofday (&time_now, nullptr);
    uint64_t t_startTime_simid = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    std::string ue_poss_out = "ue_position.txt";
    ClearFile (ue_poss_out, t_startTime_simid);
    ClearFile ("enbs.txt", t_startTime_simid);
    ClearFile ("gnbs.txt", t_startTime_simid);
    // Since nodes are randomly allocated during each run we always need to print their positions
    PrintGnuplottableUeListToFile ("ues.txt");

    int nodecount = int (NodeList::GetNNodes ());
    // NS_LOG_UNCOND ("number of nodes: " << nodecount);
    int UE_iterator = nodecount - int (nUeNodes);
    int numPrints = simTime / 0.1;

    for (int i = 0; i < numPrints; i++)
    {
        Simulator::Schedule (Seconds (i * simTime / numPrints), &PrintGnuplottableEnbListToFile, t_startTime_simid);
        for (uint32_t j = 0; j < ueNodes.GetN (); j++)
        {
            Simulator::Schedule (Seconds (i * simTime / numPrints), &PrintPosition, ueNodes.Get (j),
                                 j + UE_iterator, ue_poss_out, t_startTime_simid);
        }
    }
    for (uint32_t i = 0; i < mmWaveEnbNodes.GetN (); i++)
    {
        Ptr<MmWaveEnbPhy> enbPhy =
                mmWaveEnbNodes.Get (i)->GetDevice (0)->GetObject<MmWaveEnbNetDevice> ()->GetPhy ();
        Ptr<MmWaveEnbNetDevice> mmdev =
                DynamicCast<MmWaveEnbNetDevice> (mmWaveEnbNodes.Get (i)->GetDevice (0));
        uint16_t cell_id = mmdev->GetCellId ();
        Simulator::Schedule (Seconds (numPrints * 0.1), &GetESStates, mmdev, cell_id);
    }

    if (enableTraces)
    {
        mmwaveHelper->EnableTraces ();
    }

    // trick to enable PHY traces for the LTE stack
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    lteHelper->Initialize ();
    lteHelper->EnablePhyTraces ();
    lteHelper->EnableMacTraces ();

    // Since nodes are randomly allocated during each run we always need to print their positions
    //PrintGnuplottableUeListToFile ("ues.txt");
    // PrintGnuplottableEnbListToFile ("enbs.txt");

    bool run = true;
    if (run)
    {
        NS_LOG_UNCOND ("Simulation time is " << simTime << " seconds ");
        Simulator::Stop (Seconds (simTime));
        NS_LOG_INFO ("Run Simulation.");
        Simulator::Run ();
    }

    NS_LOG_INFO (lteHelper);
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
    return 0;
}