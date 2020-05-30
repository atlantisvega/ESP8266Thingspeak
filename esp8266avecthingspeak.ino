/* 
 envoi donnèes puissance photovoltaïque vers Thingspeak
 ESP8266 + pince SCT-013 100A + Thingspeak
 anthonyguerrier@free.fr
 
*/

#include <ESP8266WiFi.h>
#include <EmonLib.h>
 
// à remplacer par l'API Key write de votre "channel" thingspeak API
String apiKey = "xxxxxxxxxxxxx";

EnergyMonitor emon1;  // Create an instance

// à remplacer par le nom et le mot de passe de votre wifi
const char* ssid = "xxxxxxxxxxxxxxxx";
const char* password = "xxxxxxxxxxxxxxxxxxxxxxxx";
const char* server = "api.thingspeak.com";
 
WiFiClient client;
 
void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.print("Connexion en cours a :  ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("connexion au wifi ok ...");

/* Le capteur CT SCT-013-000 délivre en sortie un courant alternatif moyen de 50 mA (pour 100 A en entrée) --> un courant alternatif pic à pic de : (50 mA x √2) x 2 = 141,4 mA.
   Avec une résistance de 33 Ω pour transformer le courant en tension, la valeur maximale obtenue est : 141,4 mA x 33 Ω = 4,67 V --> 100 A = 4,67 V
   Avec un convertisseur 5 V - 10 bits, Arduino converti la tension récue en un nombre compris entre 0 et 1023.
   Comme dans notre cas, calcul ci-dessus, la tension maximale est de 4.67 V, la tension convertie sera un nombre compris entre 0 et 956
   La valeur du courant dans la fonction est obtenu par un produit en croix avec comme relation : 100 A = 4,67 V = 956 --> 100 A = 956

   The YHDC SCT-013-000 CT has 2000 turns, so the secondary peak current will be:
   Secondary peak-current = Primary peak-current / no. of turns = 141.4 A / 2000 = 0.0707A
   Ideal burden resistance = (AREF/2) / Secondary peak-current = 2.5 V / 0.0707 A = 35.4 Ω
   NB : Ce calcul pourrait être modifié lors du montage réel, 22 ohms pour une carte en 3.3v.
*/ 
   emon1.current(0, 50);             // Current: input pin, calibration  => current constant = (100 ÷ 0.050) ÷ 36 = 55.5
 
}
 
void loop() {
 
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only

  if (client.connect(server,80)) {  
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(Irms);
           postStr +="&field2=";
           postStr += String(Irms*231);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
 
     Serial.print("Ampérage: ");
     Serial.print(Irms);
     Serial.print(" Puissance: ");
     Serial.print(Irms*231);
     Serial.println("%. envoyes a Thingspeak.");
  }
  client.stop();
 
  Serial.println("en attente avant la prochaine lecture...");
  
  // thingspeak demande un minimum de 15 secondes entre deux remontee, ici j ai mis 20 secondes
  delay(20000);
}
