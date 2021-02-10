
#include <SPI.h>
#include <Ethernet.h>     // Make sure its 1.1.1 the newer version doesn't have socket.h
#include <Utility\Socket.h>

 static byte g_abyMyMacAddress[] = {0x00,0x1A,0x4B,0x38,0x0C,0x5C};
 byte g_TargetMacAddress[] = {0xE0,0xD5,0x5E,0xB4,0x62,0xAA};   // Put your PC MAC Address here
 EthernetClient client;
 EthernetServer server(80);

void setup() {
  // put your setup code here, to run once:
  Ethernet.begin(g_abyMyMacAddress);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
   client = server.available();
    if(client){
      while(client.connected()){
        if(client.available()){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 5");  // refresh the page automatically every 5 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<h1>Remote Power </h1>");
            client.println("</html>");
            SendWOLMagicPacket(g_TargetMacAddress);
            break;
          }
        }
        delay(1);
        client.stop();
        Ethernet.maintain();        
      }
}


void SendWOLMagicPacket(byte * pMacAddress)
{
  // The magic packet data sent to wake the remote machine. Target machine's
  // MAC address will be composited in here.
  const int nMagicPacketLength = 102;
  byte abyMagicPacket[nMagicPacketLength] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  byte abyTargetIPAddress[] = { 255, 255, 255, 255 }; // don't seem to need a real ip address.
  const int nWOLPort = 9;
  const int nLocalPort = 8888; // to "listen" on (only needed to initialize udp)
 
  
  // Compose magic packet to wake remote machine. 
  for (int ix=6; ix<102; ix++)
    abyMagicPacket[ix]=pMacAddress[ix%6];
  
  if (UDP_RawSendto(abyMagicPacket, nMagicPacketLength, nLocalPort, 
  abyTargetIPAddress, nWOLPort) != nMagicPacketLength)
    Serial.println("Error sending WOL packet");
}
 
int UDP_RawSendto(byte* pDataPacket, int nPacketLength, int nLocalPort, byte* pRemoteIP, int nRemotePort)
{
  int nResult;
  int nSocketId; // Socket ID for Wiz5100
 
  // Find a free socket id.
  nSocketId = MAX_SOCK_NUM;
  for (int i = 0; i < MAX_SOCK_NUM; i++) 
  {
    uint8_t s = W5100.readSnSR(i);
    if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT) 
    {
      nSocketId = i;
      break;
    }
  }
 
  if (nSocketId == MAX_SOCK_NUM)
    return 0; // couldn't find one. 
 
  if (socket(nSocketId, SnMR::UDP, nLocalPort, 0))
  {
    nResult = sendto(nSocketId,(unsigned char*)pDataPacket,nPacketLength,(unsigned char*)pRemoteIP,nRemotePort);
    close(nSocketId);
  } else
    nResult = 0;
 
  return nResult;
}
