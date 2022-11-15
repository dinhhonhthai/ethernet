#include <Arduino.h>
// #include <EtherCard.h>
#include <IPAddress.h>
#include <EthernetUdp.h>
 #include <Ethernet.h>
 #include <SoftwareSerial.h>
// SoftwareSerial rs232(25, 26); // RX, TX
#include <esp32ModbusRTU.h>
esp32ModbusRTU modbus(&Serial2, 4);  // rs485
esp32ModbusRTU modbus2(&Serial1,27);  // rs485
 // esp32ModbusRTU modbus2(&Seria3, 25, 26, 27);  // rs232
#define STATIC 1  // set to 1 to disable DHCP (adjust myip/gwip values below)
int val,val2 =  0; 
int dem = 0 ; 
// IPAddress ip(192, 168, 1, 177);
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 4, 8);
IPAddress ip_remote(192, 168, 4, 3);
unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[100];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

 
 
void setup() {
  // Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1);  // Modbus connection
  Serial1.begin(9600, SERIAL_8N1);
  // rs232.begin(9600);
  Ethernet.begin(mac, ip);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  Udp.begin(localPort);

//  ----------------------modbus
  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      //Serial.printf("%02x", data[i]);
       Serial.print(  data[i]);
       Serial.print("-");
    }
    val = data[0]*256+data[1];
   Serial.println(val); 
    //Serial.printf("Decimal number = %lld\n", "%02x", data);
    Serial.print("\n\n");
  });
  modbus.onError([](esp32Modbus::Error error) {
    Serial.printf("error: 0x%02x\n\n", static_cast<uint8_t>(error));
  });
  // -------------------------
    modbus2.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      //Serial.printf("%02x", data[i]);
       Serial.print(  data[i]);
       Serial.print("-");
    }
    val2 = data[0]*256+data[1];
   Serial.println(val); 
    //Serial.printf("Decimal number = %lld\n", "%02x", data);
    Serial.print("\n\n");
  });
  modbus2.onError([](esp32Modbus::Error error) {
    Serial.printf("error: 0x%02x\n\n", static_cast<uint8_t>(error));
  });
  // --------------------------
     modbus.begin();
     modbus2.begin();
modbus.writeSingleHoldingRegister(0x01, 1, 0);
modbus2.writeSingleHoldingRegister(0x02, 1, 0);
 
}
 void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, 100);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(packetBuffer);
    Udp.endPacket();
  }


    static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    dem++;
    Serial.print("sending Modbus request...\n");
    //modbus.readInputRegisters(0x01, 1, 1);
    if(val ==1 ) {
      modbus.writeSingleHoldingRegister(0x01, 2, dem);
       Udp.beginPacket(ip_remote, 8889);
       Udp.write(" RS485 ");
       Udp.endPacket();
    }
     if(val2 ==1 ) {
      modbus2.writeSingleHoldingRegister(0x02, 2, dem);
       Udp.beginPacket(ip_remote, 8889);
       Udp.write(" RS232 ");
       Udp.endPacket();
    }
      modbus2.readHoldingRegisters(0x02, 1,1);
      modbus.readHoldingRegisters(0x01, 1,1);
  }
  delay(10);
}