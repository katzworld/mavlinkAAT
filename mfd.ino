#include <WiFi.h>
#include <WiFiUdp.h>
#include <MAVLink.h>

//TBS
// const char* ssid     = "tbs_tango2"; // Change this to your WiFi SSID of your crossfire /tango2
// const char* password = ""; // NONE!
// const char* host = "192.168.4.1";


//Port
const int port = 5760;


WiFiUDP Udp;
char packetBuffer[255];
mavlink_message_t msg;
mavlink_status_t status;

mavlink_gps_raw_int_t gps_raw_int;
mavlink_global_position_int_t global_position_int;

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println("...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(port);  // Local port to listen on
  Serial.print("Listening on UDP port ");
  Serial.println(port);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Read the packet from the UDP buffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      for (int i = 0; i < len; ++i) {
        // Parse each byte of the received data
        if (mavlink_parse_char(MAVLINK_COMM_0, packetBuffer[i], &msg, &status)) {
          // Handle received message here
          switch (msg.msgid) {
            case MAVLINK_MSG_ID_GPS_RAW_INT:
              mavlink_msg_gps_raw_int_decode(&msg, &gps_raw_int);
              Serial.print("GPS_RAW_INT: ");
              Serial.print(" | Fix Type: ");
              Serial.print(gps_raw_int.fix_type);
              Serial.print(" | Lat: ");
              Serial.print((double)gps_raw_int.lat * 1.0e-7);
              Serial.print(" | Lon: ");
              Serial.print((double)gps_raw_int.lon * 1.0e-7);
              Serial.print(" | Alt: ");
              Serial.print((double)gps_raw_int.alt * 1.0e-3);
              Serial.print(" | Sats Visible: ");
              Serial.println(gps_raw_int.satellites_visible) + "\n";
              break;
            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
              mavlink_msg_global_position_int_decode(&msg, &global_position_int);
              Serial.print("GLOBAL_POSITION_INT: ");
              Serial.print(" | Lat: ");
              Serial.print((double)global_position_int.lat * 1.0e-7);
              Serial.print(" | Lon: ");
              Serial.print((double)global_position_int.lon * 1.0e-7);
              Serial.print(" | Alt: ");
              Serial.print((double)global_position_int.alt * 1.0e-3);
              Serial.print(" | Relative Alt: ");
              Serial.println(global_position_int.relative_alt) + "\n";
              break;
            default:
              break;
          }
        }
      }
    }
  }
}
