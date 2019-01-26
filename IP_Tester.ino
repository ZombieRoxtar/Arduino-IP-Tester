/*
 * Ethernet Wi-Fi IP Tester 1.0:
 * This sketch uses built-in Wifi to serve a web page
 * displaying the IP address of the Ethernet shield.
 * The Ethernet MAC is set to DEADBEEFCAFE.
 * 
 * Coded for Arduino Uno WiFi Rev2
 * and a cheap Ethernet shield.
 * 
 * This awesome sketch was written by Chris Roxby.
 * GNU GENERAL PUBLIC LICENSE
*/
#include <Ethernet.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 
// Please enter sensitive data in the Secrets tab - arduino_secrets.h

// Custom Eth State
enum EthStatuses
{
  EthUnk,
  LkOn,
  LkOff,
  DhcpErr,
  EthNoHw
};
EthStatuses eStatus = LkOff;

// Server instance
WiFiServer server(80);

// Page header/footer
String startPage,endPage;

void setup()
{
  // WiFi Creds
  const char* SSID = SECRET_SSID;
  const char* PASSPHRASE = SECRET_PASS;

  // WiFi State
  int wStatus = WL_IDLE_STATUS;

  // Open serial communications.
  Serial.begin(9600);  

  // Attempt to connect to Wifi.
  while (wStatus != WL_CONNECTED)
  {
    // Wait before attempting.
    delay(500);
    wStatus = WiFi.begin(SSID, PASSPHRASE);
  }
  // Start the web server on the port specified above.
  server.begin();

  // Prefill page header/footer
  FillWrappers();
}

void loop()
{
  // Listen for clients.
  WiFiClient client = server.available();

  if (client){                              // If we get a client...
    String currentLine = "";                // Make a String to hold the incoming data.
    while (client.connected()) {            // Loop while the client's connected.
      if (client.available()) {             // If there're byte(s) to read from the client...
        char c = client.read();             // Read a byte.
        if (c == '\n')                      // If the byte is a newline character...
        {
          // If the current line is blank, then we got two newline characters in a row.
          // That means the end of the client HTTP request, so send a response.
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            // The content of the HTTP response follows the header.

            // Print Header
            client.print(startPage);

            // Output our IP or the reason why we don't have one.
            if (eStatus==LkOn)
            {
              client.print("The Ethernet IP is " + DisplayAddress(Ethernet.localIP()) + ".\n");
            }
            else
            {
              String err = "An unknown error occured";
              client.print("Ethernet error: ");
              switch (eStatus)
              {
                case DhcpErr:
                err = "DHCP failure";
                break;

                case EthNoHw:
                err = "Ethernet hardware error";
                break;

                case LkOff:
                err = "No connection";
                break;

                default:
                err = "Unknown(" + String(eStatus) + ")";
              }
              client.print(err + ".\n");
            }
            // Print Footer
            client.print(endPage);

            // The HTTP response ends with another blank line.
            client.println();
            // Break out of the while loop.
            break;
          }
          else
          {
            // If we got a newline then clear currentLine.
            currentLine = "";
          }
        }
        else if (c != '\r')
        {  // If we got anything else other than a carriage return character,
          currentLine += c;
          // then add it to the end of the currentLine.
        }
      }
    }
    // Close the connection.
    client.stop();
  }
  // Restart the Ethernet.
  eStatus = StartEth();
}

EthStatuses StartEth()
{
  // Eth MAC, Dead Beef Cafe
  byte mac[] =
  {
    0xCA, 0xFE, 0xDE, 0xAD, 0xBE, 0xEF
  };
  EthStatuses eStatus = LkOn;

  // Start the Ethernet connection with the specfied MAC and DHCP timeout.
  if (Ethernet.begin(mac,500) == 0)
  {
    // Failed to configure Ethernet using DHCP.
    eStatus = DhcpErr;
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      // No Ethernet shield was found.
      eStatus = EthNoHw;
    }
    else if (Ethernet.linkStatus() != LinkON)
    {
      // The Ethernet cable is not connected.
      eStatus = LkOff;
    }
  }
  return eStatus;
}

String DisplayAddress(IPAddress address)
{
 return String(address[0]) + "." +
        String(address[1]) + "." +
        String(address[2]) + "." +
        String(address[3]);
}

void FillWrappers()
{
  startPage = String() +
      "<!DOCTYPE html>\n" +
      "<html>\n" +
      "  <head>\n" +
      "    <title>IP Report</title>\n" +
      "    <style>\n" +
      "      body\n" +
      "      {\n" +
      "        background: black;\n" +
      "        color: lightgray;\n" +
      "      }\n" +
      "      .content\n" +
      "      {\n" +
      "        text-align: center;\n" +
      "      }\n" +
      "    </style>\n" +
      "  </head>\n" +
      "  <body>\n" +
      "    <div class=\"content\">\n" +
      "      <br />\n" +
      "      ";
  endPage=String() +
      "      <br />\n" +
      "    </div>\n" +
      "  </body>\n" +
      "</html>\n";
}
