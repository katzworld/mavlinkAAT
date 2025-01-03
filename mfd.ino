
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266Watchdog.h> 
#elif defined(ESP32)
#include <WiFi.h>

#endif
#include <WebServer.h>
#include <WiFiUdp.h>
#include <MAVLink.h>

// TBS
const char* ssid     = "tbs_tango2"; // Change this to your WiFi SSID of your crossfire /tango2
const char* password = ""; // NONE!
const char* host = "192.168.4.1";
const int port = 5760;

WiFiUDP Udp;
char packetBuffer[280];
mavlink_message_t msg;
mavlink_status_t status;

mavlink_gps_raw_int_t gps_raw_int;
mavlink_global_position_int_t global_position_int;
mavlink_heartbeat_t heartbeat;

unsigned long previousHeartbeatTime = 0;
const unsigned long heartbeatTimeout = 30000; // 30 seconds

unsigned long previousMillis = 0;
const long interval = 500; // Toggle every 500 milliseconds (2 Hz)



WebServer server(80); // Server on port 80


void handleRoot() {
server.send(200,
  "<!DOCTYPE html>\n"
  "<html>\n"
  "  <head>\n"
  "    <title>MAVLink Data</title>\n"
  "  </head>\n"
  "  <body>\n"
  "    <h1>MAVLink Data</h1>\n"
  "    <div id='gps_raw_int'>\n"
  "      <h2>GPS_RAW_INT</h2>\n"
  "      <p>Time (usec): <span id='gps_raw_int_time_usec'></span></p>\n"
  "      <p>Fix Type: <span id='gps_raw_int_fix_type'></span></p>\n"
  "      <p>Lat: <span id='gps_raw_int_lat'></span></p>\n"
  "      <p>Lon: <span id='gps_raw_int_lon'></span></p>\n"
  "      <p>Alt: <span id='gps_raw_int_alt'></span></p>\n"
  "      <p>Eph: <span id='gps_raw_int_eph'></span></p>\n"
  "      <p>Epv: <span id='gps_raw_int_epv'></span></p>\n"
  "      <p>Vel: <span id='gps_raw_int_vel'></span></p>\n"
  "      <p>Cog: <span id='gps_raw_int_cog'></span></p>\n"
  "      <p>Sats Visible: <span id='gps_raw_int_sats_visible'></span></p>\n"
  "    </div>\n"
  "    <div id='global_position_int'>\n"
  "      <h2>GLOBAL_POSITION_INT</h2>\n"
  "      <p>Time (usec): <span id='global_position_int_time_usec'></span></p>\n"
  "      <p>Lat: <span id='global_position_int_lat'></span></p>\n"
  "      <p>Lon: <span id='global_position_int_lon'></span></p>\n"
  "      <p>Alt: <span id='global_position_int_alt'></span></p>\n"
  "      <p>Relative Alt: <span id='global_position_int_rel_alt'></span></p>\n"
  "      <p>vx: <span id='global_position_int_vx'></span></p>\n"
  "      <p>vy: <span id='global_position_int_vy'></span></p>\n"
  "      <p>vz: <span id='global_position_int_vz'></span></p>\n"
  "    </div>\n"
  "  </body>\n"
  "</html>\n"
);
}
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

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
  pinMode(LED_BUILTIN, OUTPUT);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  
}

void loop() {
  unsigned long currentMillis = millis();
  server.handleClient();
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
              Serial.print("  Time (usec): ");
              Serial.print(gps_raw_int.time_usec);
              Serial.print("  Fix Type: ");
              Serial.print(gps_raw_int.fix_type);
              Serial.print("  Lat: ");
              Serial.printf("%.7f", (double)gps_raw_int.lat / 10000000.0);
              Serial.print("  Lon: ");
              Serial.printf("%.7f", (double)gps_raw_int.lon / 10000000.0);
              Serial.print("  Alt: ");
              Serial.print(gps_raw_int.alt);
              Serial.print("  Eph: ");
              Serial.print(gps_raw_int.eph);
              Serial.print("  Epv: ");
              Serial.print(gps_raw_int.epv);
              Serial.print("  Vel: ");
              Serial.print(gps_raw_int.vel);
              Serial.print("  Cog: ");
              Serial.print(gps_raw_int.cog);
              Serial.print("  Sats Visible: ");
              Serial.println(gps_raw_int.satellites_visible);
              break;

            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
              mavlink_msg_global_position_int_decode(&msg, &global_position_int);
              Serial.print("GLOBAL_POSITION_INT: ");
              Serial.print("  Time (usec): ");
              Serial.print(global_position_int.time_boot_ms);
              Serial.print("  Lat: ");
              Serial.printf("%.7f", (double)global_position_int.lat * 1.0e-7);
              Serial.print("  Lon: ");
              Serial.printf("%.7f", (double)global_position_int.lon * 1.0e-7);
              Serial.print("  Alt: ");
              Serial.print((double)global_position_int.alt * 1.0e-3);
              Serial.print("  Relative Alt: ");
              Serial.print(global_position_int.relative_alt);
              Serial.print("  vx: ");
              Serial.print(global_position_int.vx);
              Serial.print("  vy: ");
              Serial.print(global_position_int.vy);
              Serial.print("  vz: ");
              Serial.println(global_position_int.vz);
              break;

            case MAVLINK_MSG_ID_HEARTBEAT:
              mavlink_msg_heartbeat_decode(&msg, &heartbeat);
              previousHeartbeatTime = millis(); // Reset heartbeat timer

              if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;

                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // Toggle the LED
              }
              //Serial.print("HEARTBEAT: \n");
              break;

            default:
              break;
          }
        }
      }
    }
  }

  // Check if heartbeat timeout has occurred
  if (millis() - previousHeartbeatTime >= heartbeatTimeout) {
    // Handle timeout (e.g., log an error, reset the board)
    Serial.println("Heartbeat timeout!"); 
    ESP.restart();
    // No need for ESP.restart() as we are already in a sleep/wake cycle
  }

}
