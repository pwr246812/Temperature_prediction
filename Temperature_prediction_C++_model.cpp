#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

vector<double> temperatura;
constexpr int maxepoki{1000};
constexpr double gammawsp = 0.005;

double maximumtemp = {0.0};
double minimumtemp = {100.00};
double wagi[6][9] = {};
int roznica_godzin = {6};

double max_roznica = {-100};
double min_roznica = {100};
double roznica = {0};
double roznica_abs = {100};
double srednia_zmiana = {0};

vector<vector<double>> ciag;
vector<double> error;

void czytaj_csv();

double przeskaluj(bool, double);

void ciag_uczacy();

void ustaw_wagi();

double f_akt(double);

void uczenie();

double siec(double, double);

double perceptron(double, double, double, double, double, int);

void print();

void sig();


int main() {
    czytaj_csv();
    przeskaluj(0, 0);
    ciag_uczacy();
    ustaw_wagi();
    sig();
    return 0;
}

void czytaj_csv() {
    vector<double>::iterator it;
    fstream file;
    file.open("C:/Users/Marcin/CLionProjects/GammyDwaWyjscia/brenna_2017_05.csv", ios::in);

    string temp, line;
    int iter = 0;
    while (!file.eof()) {
        string slowa[6];
        getline(file, line);
        int i = 0;
        for (int j = 0; j < line.size(); ++j) {
            if (line[j] == ';') {
                ++i;
                continue;
            }
            slowa[i] += line[j];
        }
        if (slowa[1] != "godzina") {
            temperatura.push_back(atof(slowa[2].c_str()));
            if (iter > 2) ++iter;
            else iter = 0;
        }
    }
    file.close();
}

double przeskaluj(bool reverse, double temp) {
    if (reverse) {
        temp *= (maximumtemp - minimumtemp);
        temp += minimumtemp;
        return temp;

    } else {
//        maximumtemp = temperatura[0];
//        minimumtemp = temperatura[0];
//        for (int i = 1; i < temperatura.size(); ++i) {
//             if (temperatura[i] > maximumtemp) maximumtemp = temperatura[i];
//             else if (temperatura[i] < minimumtemp) minimumtemp = temperatura[i];
//         }
//         ++maximumtemp;

        // MAX and MIN temp set to constant values to make sure we can achieve higher values than the ones which
        // are present in trining dataset
        maximumtemp = 50;
        minimumtemp = -5;
        for (int i = 0; i < temperatura.size(); ++i) {
            temperatura[i] = (temperatura[i] - minimumtemp) / (maximumtemp - minimumtemp);
        }
        return 0;
    }
}


void ciag_uczacy() {
    for (int i = 0; i < temperatura.size() - 4*roznica_godzin; ++i) {
        if (not(i % 1)) {
            vector<double> max = {temperatura[i], temperatura[i + roznica_godzin], temperatura[i + 2*roznica_godzin],
                                  temperatura[i + 3*roznica_godzin], temperatura[i + 4*roznica_godzin]};
            if (roznica > max_roznica) {
                max_roznica = roznica;
            }
            if (roznica < min_roznica) {
                min_roznica = roznica;
            }
            ciag.push_back(max);
        }
    }
}

void ustaw_wagi() {
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) wagi[i][j] = static_cast<double> (rand() % 101) / 50 - 1;
    }
}


double f_akt(double s) {
    return 1 / (1 + exp(-s));
}


double perceptron(double x1, double x2, double x3, double x4,double x5, int p) {
    return wagi[p][1] * x1 + wagi[p][2] * x2 + wagi[p][3] * x3 + wagi[p][4] * x4 + wagi[p][5] * x5 + wagi[p][0];
}


double siec(double x1, double x2, double x3, double x4) {
    double s1 = f_akt(perceptron(x1, x2, x3, x4, 0, 0));
    double s2 = f_akt(perceptron(x1, x2, x3, x4, 0, 1));
    double s3 = f_akt(perceptron(x1, x2, x3, x4, 0, 2));
    double s4 = f_akt(perceptron(x1, x2, x3, x4, 0, 3));
    double s5 = f_akt(perceptron(x1, x2, x3, x4, 0, 4));
    double s6 = f_akt(perceptron(s1, s2, s3, s4, s5, 5));
    return s6;
}


void uczenie() {
    error.push_back(0);
    unsigned int idx = error.size() - 1;
    double errorepok = 0;
    for (int i = 0; i < ciag.size(); ++i) {
        double wyjscie = siec(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3]);
        error[idx] = pow(ciag[i][4] - wyjscie, 2);
        errorepok += error[idx];
        double v1 = f_akt(perceptron(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3], 0, 0));
        double v2 = f_akt(perceptron(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3], 0, 1));
        double v3 = f_akt(perceptron(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3], 0, 2));
        double v4 = f_akt(perceptron(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3], 0, 3));
        double v5 = f_akt(perceptron(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3], 0, 4));

        wagi[0][0] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][1] * v1 * (1 - v1) * gammawsp;
        wagi[0][1] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][1] * v1 * (1 - v1) * ciag[i][0] * gammawsp;
        wagi[0][2] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][1] * v1 * (1 - v1) * ciag[i][1] * gammawsp;
        wagi[0][3] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][1] * v1 * (1 - v1) * ciag[i][2] * gammawsp;
        wagi[0][4] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][1] * v1 * (1 - v1) * ciag[i][3] * gammawsp;


        wagi[1][0] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][2] * v2 * (1 - v2) * gammawsp;
        wagi[1][1] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][2] * v2 * (1 - v2) * ciag[i][0] * gammawsp;
        wagi[1][2] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][2] * v2 * (1 - v2) * ciag[i][1] * gammawsp;
        wagi[1][3] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][2] * v2 * (1 - v2) * ciag[i][2] * gammawsp;
        wagi[1][4] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][2] * v2 * (1 - v2) * ciag[i][3] * gammawsp;


        wagi[2][0] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][3] * v3 * (1 - v3) * gammawsp;
        wagi[2][1] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][3] * v3 * (1 - v3) * ciag[i][0] * gammawsp;
        wagi[2][2] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][3] * v3 * (1 - v3) * ciag[i][1] * gammawsp;
        wagi[2][3] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][3] * v3 * (1 - v3) * ciag[i][2] * gammawsp;
        wagi[2][4] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][3] * v3 * (1 - v3) * ciag[i][3] * gammawsp;


        wagi[3][0] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][4] * v4 * (1 - v4) * gammawsp;
        wagi[3][1] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][4] * v4 * (1 - v4) * ciag[i][0] * gammawsp;
        wagi[3][2] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][4] * v4 * (1 - v4) * ciag[i][1] * gammawsp;
        wagi[3][3] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][4] * v4 * (1 - v4) * ciag[i][2] * gammawsp;
        wagi[3][4] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][4] * v4 * (1 - v4) * ciag[i][3] * gammawsp;


        wagi[4][0] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][5] * v5 * (1 - v5) * gammawsp;
        wagi[4][1] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][5] * v5 * (1 - v5) * ciag[i][0] * gammawsp;
        wagi[4][2] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][5] * v5 * (1 - v5) * ciag[i][1] * gammawsp;
        wagi[4][3] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][5] * v5 * (1 - v5) * ciag[i][2] * gammawsp;
        wagi[4][4] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * wagi[5][5] * v5 * (1 - v5) * ciag[i][3] * gammawsp;


        wagi[5][0] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * gammawsp;
        wagi[5][1] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * v1 * gammawsp;
        wagi[5][2] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * v2 * gammawsp;
        wagi[5][3] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * v3 * gammawsp;
        wagi[5][4] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * v4 * gammawsp;
        wagi[5][5] -= 2 * (ciag[i][4] - wyjscie) * (-1) * wyjscie * (1 - wyjscie) * v5 * gammawsp;
    }
    error[idx] = 1;
}

void sig() {
    do {
        uczenie();
    } while (error[error.size() - 1] != 0 && error.size() <= maxepoki);

    cout << "\nTEST" << endl;
    double blad = {0.0};
    for (int i = 0; i < ciag.size(); i++) {
        blad += abs(przeskaluj(1, siec(ciag[i][0], ciag[i][1], ciag[i][2], ciag[i][3])) - przeskaluj(1, ciag[i][4]));
        roznica = przeskaluj(1, ciag[i][4]) - przeskaluj(1, ciag[i][3]);
        srednia_zmiana += abs(roznica);

        if (roznica > max_roznica) {
            max_roznica = roznica;
        }
        if (roznica < min_roznica) {
            min_roznica = roznica;
        }
        if (roznica_abs > abs(roznica)) {
            roznica_abs = abs(roznica);
        }

    }
    cout << "SREDNI BEZWZGLEDNY BLAD NA ZBIORZE UCZACYM: " << blad / ciag.size() << endl;
    cout << "Maksymalna zmiana temperatury: " << max_roznica << endl;
    cout << "Minimalna  zmiana temperatury: " << min_roznica << endl;
    cout << "Minimalna bezwzgledna zmiana temperatury: " << roznica_abs << endl;
    cout << "Srednia bezwzgledna zmiana temperatury: " << srednia_zmiana/ciag.size() << endl;


    //Print which we use to copy weights to Arduino scrypt
    cout << "\n\n\n{";
    for (int i={0}; i<=5; ++i) {
        cout << "{";
        for (int j={0}; j<=5; ++j) {
            if (j==5) {
                if (i==5) cout << wagi[i][j] << "}};" << endl;
                else      cout << wagi[i][j] << "}, " << endl;
            }
            else cout << wagi[i][j] << ", ";
        }
    }

    cout << "float maximumtemp = {" << maximumtemp << "};" << endl;
    cout << "float minimumtemp = {" << minimumtemp << "};" << endl;
}