#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"

#include "ns3/core-module.h"
#include "ns3/yans-error-rate-model.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/gnuplot.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <fstream>
#include <stdio.h>  
#include <stdlib.h>
#include "ns3/flow-monitor-module.h"
using namespace ns3;

using namespace std;

NS_LOG_COMPONENT_DEFINE ("IOt");

void calculateSD(double data[],int n)
{
    double sum = 0.0, mean, standardDeviation = 0.0;

    int i=0;

    for(i = 0; i < 10; ++i)
    {
        sum += data[i];
    }

    mean =  (double)n;

cout<<"Mean : "<<mean<<"\n";

    for(i = 0; i < n; ++i)
        standardDeviation += pow(data[i] - mean, 2);

double  variance = standardDeviation / (double)n;
cout<<"Variance : "<<variance<<"\n";
cout<<"standard Deviation : "<<sqrt(variance)<<"\n";
    
}

int 
main (int argc, char *argv[])
{
  uint32_t nnodes = 20; //if u want to make 30 nodes simply change here

 int state=2;  // 1 is static ,2 is dynamic 
  uint32_t slot = 9; 
  uint32_t sifs = 10;
  uint32_t ackTimeout = 88; 
  uint32_t ctsTimeout = 88; 
  uint32_t rifs = 2; 
 uint32_t  offered_load=30;

double m_TotalSimTime=10.0;
  CommandLine cmd;
  cmd.AddValue ("nnodes", "Number of IOT devices", nnodes);
  

  cmd.AddValue ("slot", "Slot time in microseconds", slot);
  cmd.AddValue ("sifs", "SIFS duration in microseconds", sifs);
  cmd.AddValue ("ackTimeout", "ACK timeout duration in microseconds", ackTimeout);
  cmd.AddValue ("ctsTimeout", "CTS timeout duration in microseconds", ctsTimeout);
  cmd.AddValue ("rifs", "RIFS duration in microseconds", rifs);
cmd.AddValue ("offeredLoad", "offered load mbit/s ", offered_load);
  cmd.Parse (argc,argv);
  NodeContainer allNodes;
  NodeContainer IOT_dev_node;
  IOT_dev_node.Create (nnodes);
  allNodes.Add (IOT_dev_node);
  NodeContainer wifiApNode ;
  wifiApNode.Create (1);
  allNodes.Add (wifiApNode);





  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
                
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, IOT_dev_node);
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Slot", TimeValue (MicroSeconds (slot)));
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Sifs", TimeValue (MicroSeconds (sifs)));
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/AckTimeout", TimeValue (MicroSeconds (ackTimeout)));
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/CtsTimeout", TimeValue (MicroSeconds (ctsTimeout)));
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Rifs", TimeValue (MicroSeconds (rifs)));


 std::vector <std::string> modes;
  modes.push_back ("OfdmRate54Mbps");


  NodeContainer Rpl_Router;
  Rpl_Router.Add (wifiApNode);
  Rpl_Router.Create (1);
  allNodes.Add (Rpl_Router.Get (1));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (Rpl_Router);

  NodeContainer Cloud_Controller;
  Cloud_Controller.Add (Rpl_Router.Get (1));
  Cloud_Controller.Create (1);
  allNodes.Add (Cloud_Controller.Get (1));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (Cloud_Controller);


LrWpanHelper lrWpanHelper;
  // Add and install the LrWpanNetDevice for each IOT Device node
  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(IOT_dev_node);

  // Fake PAN association and short address assignment.
  lrWpanHelper.AssociateToPan (lrwpanDevices, 0);
  SixLowPanHelper sixlowpan;
  NetDeviceContainer devices = sixlowpan.Install (lrwpanDevices); 


