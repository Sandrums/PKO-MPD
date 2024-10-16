#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>

using namespace std;

void print_dominos(vector<pair<int, int>> &dominos ) {
    for (pair<int, int> domino : dominos) {
        cout << domino.first << domino.second << " ";
    }
}

/// Konkatenācija pieņem, ka a un b ir viencipara
/// Atgriež 1, ja skaitli x var iegūt no a un b, veicot atļautās darbības
int matches_pattern(int x, int a, int b) {
    if (a < b) {swap(a,b);}
    if (a+b == x || a-b == x || (b != 0 && a%b == 0 && a/b == x) || a*b == x || (a != 0 && a*10+b == x) || (b != 0 && b*10+a == x)) {return 1;}
    return 0;
}

/// Izrēķina izmaksas
int my_cost(vector<int>& pattern, vector<pair<int, int>>& solution) {
    int cost = 0;
    /// Pārbauda atbilstību šablonam
    for (int i=0; i<pattern.size(); i++) {
        cost += 1-matches_pattern(pattern[i], solution[i].first, solution[i].second);
    }
    /// Pārbauda atbilstību "domino likumam" - ka kauliņu pusēs, kas saskaras, ierakstīto punktu skaits sakrīt
    for (int i=0; i<pattern.size()-1; i++) {
        if (solution[i].second != solution[i+1].first) {cost++;}
    }
    if (solution[pattern.size()-1].second != solution[0].first) {cost++;}
    return cost;
}

/// Atgriež kādu domino virkni, ko var ar vienu gājienu iegūt no dotās virknes; r - skaitlis no 0 līdz 99, kas norāda, cik bieži mainīt vietām divus kauliņus (alternatīva - pagriezt kauliņu otrādi)
vector<pair<int, int>> random_neighbor(vector<pair<int, int>> solution, int r) {
    if (rand() % 100 >= r) {
        int i = rand() % solution.size();
        int j = rand() % solution.size();
        while (i == j) {j = rand() % solution.size();}
        swap(solution[i], solution[j]);
    }
    else {
        int i = rand() % solution.size();
        swap(solution[i].first, solution[i].second);
    }
    return solution;
}

/// Meklē risinājumu ar LAHC algoritmu. Beidz, kad 1000 reizes pēc kārtas nav bijuši uzlabojumi
vector<pair<int, int>> lahc(vector<int>& pattern, vector<pair<int, int>> solution, int n, int r) {
    vector<pair<int, int>> best = solution;
    vector<pair<int, int>> i = solution;
    int cost_best = my_cost(pattern, best);
    int cost_i = my_cost(pattern, i);
    vector<int> L;
    for (int k=0; k<n; k++) {L.push_back(cost_i);}
    int k = 0;
    int no_improvements = 0;
    while (no_improvements < 1000) {
        vector<pair<int, int>> j = random_neighbor(i, r);
        k %= n;
        int cost_j = my_cost(pattern, j);
        if (cost_j < cost_best) {
            best = j;
            cost_best = my_cost(pattern, best);
            no_improvements = 0;
        }
        else {
            no_improvements++;
        }
        if (cost_j < cost_i  || cost_j < L[k]) {
            i = j;
            cost_i = my_cost(pattern, i);
        }
        L[k] = cost_j;
        k++;
    }
    return best;
}

/// Meklē risinājumu ar evolucionāro algoritmu. Beidz, kad 1000 reizes pēc kārtas nav bijuši uzlabojumi
vector<pair<int, int>> evolutionary(vector<int>& pattern, vector<pair<int, int>> solution, int n, int r) {
    vector<vector<pair<int, int>>> I;
    vector<vector<pair<int, int>>> mating_pool;
    vector<pair<int, int>> best;
    int best_cost = 1e9;
    /// Sākumā populācija ir dotais risinājums solution un n-1 nejauša tā permutācija
    for (int i=0; i<n; i++) {
        I.push_back(solution);
        random_shuffle(solution.begin(), solution.end());
    }
    int no_improvements = 0;
    while (no_improvements < 1000) {
        /// Lai nebūtu problēmu ar float neprecizitātēm, izmanto nevis vidējo veselību, bet veselību summu.
        int fitness_sum = 0;
        for (int i=0; i<n; i++) {
            fitness_sum += my_cost(pattern, I[i]);
        }
        for (int i=0; i<n; i++) {
            int cost_curr = my_cost(pattern, I[i]);
            if (cost_curr*n < fitness_sum) {
                mating_pool.push_back(I[i]);
            }
            if (cost_curr < best_cost) {
                best_cost = cost_curr;
                best = I[i];
                no_improvements = 0;
            }
            else {
                no_improvements++;
            }
        }
        /// Ja mating pool ir tukšs (tā var gadīties, kad visiem ir vienādas izmaksas, tātad, nevienam izmaksas nav labākas par vidējām), ieliek tajā katru otro risinājumu
        if (mating_pool.size() == 0) {
            for (int i=0; i<n; i+=2) {
                mating_pool.push_back(I[i]);
            }
        }
        I = mating_pool;
        int mating_size = mating_pool.size();
        /// Kamēr I vēl ir brīvas vietas, pēc kārtas no katra mating pool indivīda paņem kādu kaimiņu
        for (int i=0; i<n-mating_size; i++) {
            I.push_back(random_neighbor(mating_pool[i%mating_size], r));
        }
        mating_pool.clear();
    }
    return best;
}

int main(int argc, char * argv[]){
    int pattern_nr = atoi(argv[1]);
    int algorithm = atoi(argv[2]);
    int max_a = atoi(argv[3]);
    int min_n = atoi(argv[4]);
    int max_n = atoi(argv[5]);
    int min_r = atoi(argv[6]);
    int max_r = atoi(argv[7]);

    int max_pts = 6;
    vector<int> pattern;
    if (pattern_nr == 1) {pattern = {2, 1, 1, 1, 1, 3, 1, 1, 2, 2, 4, 2, 2, 8, 5, 6, 0, 20, 3, 1, 3, 6, 6, 1, 10, 5, 0, 0};}
    else {pattern = {1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 1, 2, 1, 1, 10, 0, 0, 2, 4, 6, 6, 4, 5, 12, 12, 2, 1, 0};}
    cout << "Pattern: ";
    for (int p : pattern) {cout << p << " ";}
    cout << "\n";

    if (algorithm == 1) {cout << "LAHC\n";}
    else {cout << "Evolutionary\n";}

    /// Sākuma risinājums ir visi domino pēc kārtas
    vector<pair<int, int>> solution;
    for (int i=0; i<=max_pts; i++) {
        for (int j=i; j<=max_pts; j++) {
            solution.push_back({i,j});
        }
    }

    int the_best_cost = 100;
    vector<pair<int, int>> the_best;
    vector<pair<int, int>> best;
    for (int n=min_n; n<=max_n; n*=2) {
        for (int r=min_r; r<=max_r; r+=5) {
            for (int a=0; a<max_a; a++) {
                if (algorithm == 1) {
                    best = lahc(pattern, solution, n, r);
                }
                else {
                    best = evolutionary(pattern, solution, n, r);
                }
                if (my_cost(pattern, best) < the_best_cost) {
                    the_best_cost = my_cost(pattern, best);
                    the_best = best;
                }
            }
        }
    }
    cout << "Cost: " << the_best_cost << "\nSolution: ";
    for (pair<int, int> domino : the_best) {
        cout << domino.first << " " << domino.second << "   ";
    }

    return 0;
}
