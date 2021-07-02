#include "DHT.h"          // biblioteka DHT
#include <CSV_Parser.h>   // biblioteka do odczytu CSV
#define DHTPIN A2          // numer pinu sygnałowego
#define DHTTYPE DHT11     // typ czujnika (DHT11)
DHT dht(DHTPIN, DHTTYPE); // definicja czujnika

int etap_procesu {1};
int indeks = 0;
float temperatury[25] = {0};
float predykcja = {0};
float predykcje[7] = {};
float gammawsp {0.9};
const int roznica_godzinowa = {6};
unsigned long przerwa {3600000};


float wagi[6][6] = 
{{-0.378496, -0.320038, -0.275931, 0.247721, -0.686012, 0},
{2.59748, -2.80267, -5.29436, 2.21468, 2.75833, 0},
{0.453285, -9.44311, -4.98096, 8.89919, 6.23039, 0},
{-2.19433, -0.195484, 2.86742, 3.07962, -2.12117, 0},
{-0.117509, 6.01793, 0.781171, -2.69347, -8.03303, 0},
{-0.787429, -0.568455, 5.47389, -5.3652, 3.31497, -7.95654}};

float maximumtemp = {50};
float minimumtemp = {-5};




int pomiary {3*roznica_godzinowa};


void przenumerowanie() {
  for (int i {1}; i<4*roznica_godzinowa + 1; ++i) {
    temperatury[i-1] = temperatury[i];
  }
}


void print_danych() {
  Serial.println("\nAktualne dane pomiarowe:");
  for (int k {0}; k<=24; ++k) {
    Serial.print((String) temperatury[k] + " ") ;
  }
}


float f_akt(float s) {
  return 1/(1 +  exp(-s));
}


float perceptron(float x1, float x2, float x3, float x4, float x5, int p) {
  return wagi[p][1] * x1 + wagi[p][2] * x2 + wagi[p][3] * x3 + wagi[p][4] * x4 + wagi[p][5] * x5 + wagi[p][0];
}


float siec(float x1, float x2, float x3, float x4) {
    double s1 = f_akt(perceptron(x1, x2, x3, x4, 0, 0));
    double s2 = f_akt(perceptron(x1, x2, x3, x4, 0, 1));
    double s3 = f_akt(perceptron(x1, x2, x3, x4, 0, 2));
    double s4 = f_akt(perceptron(x1, x2, x3, x4, 0, 3));
    double s5 = f_akt(perceptron(x1, x2, x3, x4, 0, 4));
    double s6 = f_akt(perceptron(s1, s2, s3, s4, s5, 5));
    return s6;
}


float przeskaluj(bool reverse, double temp) {
        if (reverse) {
            temp *= (maximumtemp - minimumtemp);
            temp += minimumtemp;
            return temp;
        } else {
            return (temp - minimumtemp)/(maximumtemp - minimumtemp);
        }
}


void setup()
{
  Serial.begin(9600);     // otworzenie portu szeregowego
  dht.begin();            // inicjalizacja czujnika
  Serial.println("Gromadzę pomiary ...");
    for (int i {0}; i<pomiary; ++i) {
      temperatury[i] = dht.readTemperature();
      print_danych();
      delay(przerwa);
      }
    temperatury[pomiary] = dht.readTemperature();
    print_danych();
    Serial.println("\n\nPoczątkowe pomiary gotowe!");
}

 
void loop()
{  switch(etap_procesu) {

    // Dokonujemy pomiaru temperatury po upływie kolejnej godziny i przechodzimy do kolejnego etapu
    case 0:
        pomiary++;
        temperatury[pomiary] = dht.readTemperature();
        print_danych();
        etap_procesu++;

    // Predykujemy temperaturę za 6 godzin
    // Jeśli nie mamy jeszcze 25 zmierzonych temperatur - wracamy do poprzedniego etapu 
    // W przeciwnym wypadku przechodzimy do kolejnego etapu
    case 1:
        predykcje[indeks] = siec(przeskaluj(0, temperatury[indeks]), przeskaluj(0, temperatury[indeks+roznica_godzinowa]), 
                                 przeskaluj(0, temperatury[indeks+2*roznica_godzinowa]), przeskaluj(0, temperatury[indeks+3*roznica_godzinowa]));

        if (indeks<=roznica_godzinowa - 1) {
          Serial.println((String) "\nPredykowana temperatura za 6 godzin: " + przeskaluj(1, predykcje[indeks]) + " *C");
          delay(przerwa); 
        }
        if (indeks>=roznica_godzinowa) {
          ++etap_procesu;
        }
        else {
          ++indeks;
          --etap_procesu;
        }
        break;

    case 2:
      Serial.println("\nMam już dane niezbędne do procesu uczenia\n");
      indeks = 0;
      ++etap_procesu;

    // Wykorzystujemy temperatury z indeksami 0 6 12 18 jako wejścia do sieci i tą z indeksem 24 jako oczekiwane wyjście
    // Przeprowadzamy jedną iterację procesu uczenia 
    // Wyświetlamy odnotowaną pomyłkę przy predykcji temperatury 6h temu
    // Przechodzimy do kolejnego etapu
    case 3:
        float v1 = f_akt(perceptron(temperatury[0], temperatury[roznica_godzinowa], temperatury[2*roznica_godzinowa], temperatury[3*roznica_godzinowa], 0, 0));
        float v2 = f_akt(perceptron(temperatury[0], temperatury[roznica_godzinowa], temperatury[2*roznica_godzinowa], temperatury[3*roznica_godzinowa], 0, 1));
        float v3 = f_akt(perceptron(temperatury[0], temperatury[roznica_godzinowa], temperatury[2*roznica_godzinowa], temperatury[3*roznica_godzinowa], 0, 2));
        float v4 = f_akt(perceptron(temperatury[0], temperatury[roznica_godzinowa], temperatury[2*roznica_godzinowa], temperatury[3*roznica_godzinowa], 0, 3));
        float v5 = f_akt(perceptron(temperatury[0], temperatury[roznica_godzinowa], temperatury[2*roznica_godzinowa], temperatury[3*roznica_godzinowa], 0, 4));

//        float d = przeskaluj(0, temperatury[sizeof(temperatury)/sizeof(temperatury[0]) - 1]);
        float d = przeskaluj(0, temperatury[4*roznica_godzinowa]);
        float y = siec(przeskaluj(0, temperatury[0]), przeskaluj(0, temperatury[roznica_godzinowa]), 
                       przeskaluj(0, temperatury[2*roznica_godzinowa]), przeskaluj(0, temperatury[3*roznica_godzinowa]));
        
//        float roznica = temperatury[sizeof(temperatury)/sizeof(temperatury[0]) - 1] - przeskaluj(1, y);
        float roznica = temperatury[4*roznica_godzinowa] - przeskaluj(1, predykcje[indeks]);

        Serial.println((String) "\nOstatnia odnotowana pomyłka sieci: " + roznica);
        Serial.println((String) "Predykcja, której to dotyczy: " + przeskaluj(1, predykcje[indeks]));
        Serial.println("Koryguję wagi ..."); 

        wagi[0][0] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][1] * v1 * (1 - v1) * gammawsp;
        wagi[0][1] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][1] * v1 * (1 - v1) * gammawsp * temperatury[0];
        wagi[0][2] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][1] * v1 * (1 - v1) * gammawsp * temperatury[roznica_godzinowa];
        wagi[0][3] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][1] * v1 * (1 - v1) * gammawsp * temperatury[2*roznica_godzinowa];
        wagi[0][4] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][1] * v1 * (1 - v1) * gammawsp * temperatury[3*roznica_godzinowa];

        wagi[1][0] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][2] * v2 * (1 - v2) * gammawsp;
        wagi[1][1] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][2] * v2 * (1 - v2) * gammawsp * temperatury[0];
        wagi[1][2] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][2] * v2 * (1 - v2) * gammawsp * temperatury[roznica_godzinowa];
        wagi[1][3] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][2] * v2 * (1 - v2) * gammawsp * temperatury[2*roznica_godzinowa];
        wagi[1][4] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][2] * v2 * (1 - v2) * gammawsp * temperatury[3*roznica_godzinowa];

        wagi[2][0] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][3] * v3 * (1 - v3) * gammawsp;
        wagi[2][1] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][3] * v3 * (1 - v3) * gammawsp * temperatury[0];
        wagi[2][2] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][3] * v3 * (1 - v3) * gammawsp * temperatury[roznica_godzinowa];
        wagi[2][3] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][3] * v3 * (1 - v3) * gammawsp * temperatury[2*roznica_godzinowa];
        wagi[2][4] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][3] * v3 * (1 - v3) * gammawsp * temperatury[3*roznica_godzinowa];

        wagi[3][0] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][4] * v4 * (1 - v4) * gammawsp;
        wagi[3][1] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][4] * v4 * (1 - v4) * gammawsp * temperatury[0];
        wagi[3][2] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][4] * v4 * (1 - v4) * gammawsp * temperatury[roznica_godzinowa];
        wagi[3][3] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][4] * v4 * (1 - v4) * gammawsp * temperatury[2*roznica_godzinowa];
        wagi[3][4] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][4] * v4 * (1 - v4) * gammawsp * temperatury[3*roznica_godzinowa];

        wagi[4][0] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][5] * v5 * (1 - v5) * gammawsp;
        wagi[4][1] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][5] * v5 * (1 - v5) * gammawsp * temperatury[0];
        wagi[4][2] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][5] * v5 * (1 - v5) * gammawsp * temperatury[roznica_godzinowa];
        wagi[4][3] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][5] * v5 * (1 - v5) * gammawsp * temperatury[2*roznica_godzinowa];
        wagi[4][4] -= 2 * (d - y) * (-1) * y * (1 - y) * wagi[5][5] * v5 * (1 - v5) * gammawsp * temperatury[3*roznica_godzinowa];

        wagi[5][0] -= 2 * (d - y) * (-1) * y * (1 - y) * gammawsp;
        wagi[5][1] -= 2 * (d - y) * (-1) * y * (1 - y) * v1 * gammawsp;
        wagi[5][2] -= 2 * (d - y) * (-1) * y * (1 - y) * v2 * gammawsp;
        wagi[5][3] -= 2 * (d - y) * (-1) * y * (1 - y) * v3 * gammawsp;
        wagi[5][4] -= 2 * (d - y) * (-1) * y * (1 - y) * v4 * gammawsp;
        wagi[5][5] -= 2 * (d - y) * (-1) * y * (1 - y) * v5 * gammawsp;

        y = siec(przeskaluj(0, temperatury[0]), przeskaluj(0, temperatury[roznica_godzinowa]), 
                 przeskaluj(0, temperatury[2*roznica_godzinowa]), przeskaluj(0, temperatury[3*roznica_godzinowa]));
        Serial.println((String) "\nPredykowana temperatura za 6 godzin: " + przeskaluj(1, y) + " *C");
        predykcje[indeks] = y;
        
        if (indeks>=roznica_godzinowa-1) {
          indeks = 0;
        }
        else {
          ++indeks;
        }
        
        ++etap_procesu;

    // Wykonujemy przenumerowanie listy z pomiarami temperatur - T(1) wskakuje na miejsce T(0) i tak dalej
    // W ten sposób na ostatnim i przedostatnim miejscu mamy chwilowo tę samą temperaturę, ale to nie problem, gdyż 
    // na ostatnie miejsce w tablicy danych pomiarowych wstawiamy przeprowadzony pomiar aktualnej temperatury, który
    // posłuży do procesu uczenia gdy powrócimy do poprzedniego etapu
    // Wcześniej wykonujemy predykcję temperatury za 6h na przenumerowanej już tablicy danych obserwacyjnych.
    // Wyświetlamy ją i wracamy do poprzedniego etapu.
    case 4:
        delay(przerwa);
        przenumerowanie();
        temperatury[sizeof(temperatury)/sizeof(temperatury[0]) - 1] = dht.readTemperature();
        print_danych();                           
        --etap_procesu;
  }
}
