/*********/
/* NOTES */
/*********/
// À tester dans la durée pour voir si on perd la connexion ou pas.
// Quand ça tourne plus, les aiguillent restent bloquées en position, pas de retour à Zero.
// Comment tester si une interruption n'a pas eu lieu depuis plus de 60 sec (1 rpm) ? -> vitEolRPM = rpmVent= 0
// Mettre le module en veille quand aucune interruption n'a lieu.

#include <ESP8266WebServer.h>
#include <FS.h>
#include <WiFiUdp.h>  //pour upload de pgm Arduino via wifi
#include <ArduinoOTA.h> //pour upload de pgm Arduino via wifi

/****************/
/* DÉCLARATIONS */
/****************/
ESP8266WebServer server ( 8080 );   // on instancie un serveur ecoutant sur le port 80
#define pinHallAnemo D2   // le capteur à effet Hall est connecté à la pin D2
#define pinHallEol D1   // le second capteur à effet Hall est connecté à la pin D3
#define ssid      "xxxx"    // WiFi SSID
#define password  "XXXX"  // WiFi password
unsigned long rpmVent = 0;
unsigned long rpmEol = 0;
unsigned long vitVentKMH = 0;
unsigned long vitEolRPM = 0;
unsigned long dateDernierChangementVent = 0;
unsigned long dateDernierChangementEol = 0;

/*********/
/* SETUP */
/*********/
void setup() {
  Serial.begin ( 115200 );    // init du mode débug
  // Connexion au WiFi
  WiFi.begin ( ssid, password );
  // Attente de la connexion au réseau WiFi / Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); 
    Serial.print ( "." );
  }
  // Connexion WiFi établie
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

  // Montage de la zone mémoire SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount failed");
  } 
  else {
    Serial.println("SPIFFS Mount succesfull");
  }
  delay(50);

  // Pin capteurs
  attachInterrupt(pinHallAnemo, rpm_vent, FALLING); 
  attachInterrupt(pinHallEol, rpm_eol, FALLING); 
  
  // Pages web du serveur
  server.serveStatic("/js", SPIFFS, "/js");       // dossier js qui contient les fichiers JavaScripts
  server.serveStatic("/css", SPIFFS, "/css");     // dossier css qui contient les fichiers css
  server.serveStatic("/", SPIFFS, "/index.html"); // racine du serveur, pointe l'index.html
  server.on("/mesures.json", sendMesures);        // écrit le fichier json à l'appel de la fonction
  server.begin();                                 // démarre le serveur
  Serial.println ( "HTTP server started" );
  ArduinoOTA.setHostname("AnemometreWiFi");   // on donne une petit nom a notre module, pour upload de pgm Arduino via wifi
  // ArduinoOTA.setPassword((const char *)"1357");
  ArduinoOTA.begin();   // initialisation de l'OTA, pour upload de pgm Arduino via wifi
}

/*************/
/* FONCTIONS */
/*************/
void rpm_vent()   // appelée par l'interruption, Anémomètre vitesse du vent.
{ 
  unsigned long dateCourante = millis();
  float intervalle = (dateCourante - dateDernierChangementVent);
  Serial.print ( "intervalle en s : " );
  Serial.println (intervalle/1000); // affiche l'intervalle de temps entre deux passages
  if (intervalle != 0)  // attention si intervalle = 0, division par zero -> erreur
  {
    rpmVent = 60 / (intervalle /1000);  
  }
  vitVentKMH = ( rpmVent + 6.174 ) / 8.367;
  Serial.print ( "vitVentKMH : " );
  Serial.println ( vitVentKMH ); // affiche les rpm  
  Serial.println ( "" );
  dateDernierChangementVent = dateCourante;
}

void rpm_eol()    // appelée par l'interruption, Tachométre rotation éolienne.
{
  unsigned long dateCourante = millis();
  float intervalle = (dateCourante - dateDernierChangementEol);
  Serial.print ( "intervalle en s : " );
  Serial.println (intervalle/1000); // affiche l'intervalle de temps entre deux passages
  if (intervalle != 0)  // attention si intervalle = 0, division par zero -> erreur
  {  
    vitEolRPM = 60 / (intervalle /1000);
  }
  Serial.print ( "rpm : " );
  Serial.println ( vitEolRPM ); // affiche les rpm  
  Serial.println ( "" );
  dateDernierChangementEol = dateCourante;
}

void sendMesures()    // appelée par le serveur web
{  
  String json = "{\"VitesseVent\":\"" + String(vitVentKMH) + "\",\"rpmEolienne\":\"" + String(vitEolRPM) + "\"}"; // prépare et formate la valeur pour le fichier json sous la forme :
// prépare et formate la valeur pour le fichier json sous la forme : {"VitesseVent":"0.00","rpmEolienne":"300"}
//                          {
//                            "VitesseVent":"25",
//                            "rpmEolienne":"300"
//                          }
  server.send(200, "application/json", json);   // envoie dans le valeur dans le fichier json qui tourne en mémoire
//  Serial.println("Mesures envoyees");
}

/*************/
/* PROGRAMME */
/*************/
void loop()
{
  server.handleClient();  // à chaque iteration, on appelle handleClient pour que les requetes soient traitees
//  delay(100); // la boucle fait tourner sendMesures(), via handleClient, régler delais si besoin de mettre à jour le JSON qu'à une fréquence voulue plutôt qu'instantanément 
  ArduinoOTA.handle();  // régler upload speed à 9600 : verifie si un upload de programme Arduino est envoyé sur l'ESP8266
}
