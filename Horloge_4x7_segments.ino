/*
  DATE : 13/04/2024
  AUTEUR : Exaucé KIMBEMBE


  Ce programme permet de lire la date et l'heure via le module
  RTC DS3231 puis de l'afficher dans le moniteur série.

  NB: Après téléversement du programme, supprimer la fonction
  "config_RTC()" puis téléverser de nouveau le programme.
*/

#include <RTClib.h>

#define ANODE_COMMUNE false // Si différent remplacé par : ANODE_COMMUNE true
#define ON false // Signal d'activation des digits : remplacé par true si ANODE_COMMUNE true

// Pins attachés aux différents segments
#define SEG_A 2
#define SEG_B 3
#define SEG_C 4
#define SEG_D 5
#define SEG_E 6
#define SEG_F 7
#define SEG_G 8
#define SEG_DATE  10
#define SEG_HEURE 9

// Pins attachés aux différents digits
#define DIGIT_1   11
#define DIGIT_2   12
#define DIGIT_3   A0
#define DIGIT_4   A1
#define PIN_POINT 13

#define ATTENTE 10000 // Temps d'affiche des digits

// Tableaux contenant les pins utilisés
const uint8_t pin_segment[7]= {SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,SEG_F,SEG_G};
const uint8_t pin_digit[4]  = {DIGIT_1,DIGIT_2,DIGIT_3,DIGIT_4};

int heure[4] = {}; // L'heure à afficher
int date[4]  = {}; // La date à afficher

// Digit possible à afficher sous format : abcdefg
const int digit[10] = {0x3F,0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F}; 

// Variable
String str_jour;
String str_mois;
String str_heure;
String str_minute;

RTC_DS3231 rtc; // Création de l'objet RTC_DS1307
DateTime   Now; // Création de l'objet DateTime

// Prototypes des fonctions
void affiche(int attente, int tab[]);
void config_RTC(int decalage = 10);
void horloge(void);

void setup() {
  // Définitions des pins comme sortie 
  pinMode(SEG_DATE,OUTPUT);
  pinMode(SEG_HEURE,OUTPUT);
  pinMode(PIN_POINT,OUTPUT);
  digitalWrite(PIN_POINT,ON);
  for (int i = 0; i < 7; ++i){
    pinMode(pin_segment[i],OUTPUT);
    if (i < 4){pinMode(pin_digit[i],OUTPUT);}    
  }

  rtc.begin(); // Initialisation de la communication au module RTC_DS3231
  /*  
    Après avoir téléversé le programme, mettre la fonction config_RTC() en commentaire
    puis téléverser le programme à nouveau.
    NB : si elle n'est pas mise en commentaire, à chaque lancement du programme,
    la date et l'heure seront configurées à la date du téléversement du
    programme.

    Insérer le retard approximatif dans les paramètres de la fonction
    EX : config_RTC(20); // valeur maxi 60
  */
  //config_RTC(); 
}

void loop() {
  horloge();
  
  unsigned long depart = millis(); // Départ temps d'affichage 
  unsigned long depart_blink = millis(); // Départ temps de clignotement
  bool etat = not ON;
  // Affichage de l'heure
  while(true){
    affiche(10, heure); // appel à la fonction
    
    // Clignotement chaque seconde
    if(millis()-depart_blink > 1000){
      etat = not etat;
      depart_blink = millis();
      digitalWrite(SEG_HEURE,etat); 
    }

    // Sortie de la boucle
    if ((millis()-depart) > ATTENTE){
      digitalWrite(SEG_HEURE,ON); // Extinction
      break;
    }
  }

  depart = millis();
  // Affichage de la date
  while(true){
    digitalWrite(SEG_DATE,not ON); // Allumage
    affiche(10, date); // appel à la fonction

    if ((millis()-depart) > ATTENTE){
      digitalWrite(SEG_DATE,ON); // Extinction
      break;
    }
  }
  
}

/*
  Cette fonction permet d'allumer la combinaison des segments contenue
  dans le tableau heure ou date.
  Paramètre :
    Type : int
    Nom : attente
    Rôle : définis la durée que les segments seront allumés  
*/
void affiche(int attente, int tab[]){
  for (int i = 0; i < 4; ++i){
    digitalWrite(pin_digit[i],ON);
    for (int l = 0; l < 7; ++l){
      #if ANODE_COMMUNE
        digitalWrite(pin_segment[l],not bitRead(digit[tab[i]],l));
      #else
        digitalWrite(pin_segment[l],bitRead(digit[tab[i]],l));
      #endif
    }
    delay(attente);
    digitalWrite(pin_digit[i],not ON);
  }
}


void horloge(void){
  Now = rtc.now();

  str_jour   = String(Now.day());
  str_mois   = String(Now.month());
  str_heure  = String(Now.hour());
  str_minute = String(Now.minute());

  String U; // Unité
  String D; // Dizaine

  // Mise à jour du tableau de la date
  if(str_jour.length()==1){
    date[0] = 0;
    date[1] = str_jour.toInt();
  }
  else{
    U = str_jour[0];
    D = str_jour[1];

    date[0] = U.toInt();
    date[1] = D.toInt();
  }

  if(str_mois.length()==1){
    date[2] = 0;
    date[3] = str_mois.toInt();
  }
  else{
    U = str_mois[0]; 
    D = str_mois[1]; 

    date[2] = U.toInt();
    date[3] = D.toInt();
  }

  // Mise à jour du tableau de l'heure
  if(str_heure.length()==1){
    heure[0] = 0; 
    heure[1] = str_heure.toInt();
  }
  else{
    U = str_heure[0]; 
    D = str_heure[1]; 

    heure[0] = U.toInt();
    heure[1] = D.toInt();
  }

  if(str_minute.length()==1){
    heure[2] = 0; 
    heure[3] = str_minute.toInt();
  }
  else{
    U = str_minute[0]; 
    D = str_minute[1]; 

    heure[2] = U.toInt();
    heure[3] = D.toInt();
  }
}


void config_RTC(int decalage){
  // Si le décalage dépasse 60 secondes
  if (decalage > 60)
    decalage = 60;
  /*
    Configuration de la date et l'heure du module RTC à la date et l'heure de la compilation du programme.
    Configuration manuelle 27 avril 2022 à 11:00:00
    rtc.adjust(DateTime(2022, 4, 27, 11, 0, 0));
  */
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Now = rtc.now(); // Mise à jour

  int date   = (int) Now.day();
  int mois   = (int) Now.month();
  int annee  = (int) Now.year();

  int H = (int) Now.hour();
  int M = (int) Now.minute();
  int S = (int) Now.second();

  int S_Total =  S + decalage;       // Nombre de secondes total
  int M_Total =  M + (S_Total / 60); // Nombre de minutes total
  int H_Total =  H + (M_Total / 60); // Nombre d'heures total

  S = S_Total % 60;
  M = M_Total % 60;
  H = H_Total % 24;

  // Configuration de la date et l'heure du module RTC à la date et l'heure de la compilation du programme
  // plus le décalage issu lors du transfert vers l'arduino
  rtc.adjust(DateTime(annee, mois, date, H, M, S));
}
