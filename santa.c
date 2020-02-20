// Copyright 2018 Nitu Nicolae Iulian
// cand o sa fac makefile ul sa pun parametru la gcc ca sa imi
// ruleze codul pe 32biti pt a evita problemele de Endianness
#include <stdio.h>
// #include <bits/libc-header-start.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

typedef unsigned char hex;

typedef struct glaciar {
    int height;
    int gloves;
    int id;
}
Glaciar;

#define MAX 255

typedef struct elf {
    char name[MAX];
    int x, y , hp, stamina, dmg , kills;
    bool alive;
}
Elf;

// in afara ghetarului
bool out_of_range(int x1, int y1, int x0, int y0, int R) {
    if (x1 > 2*R || x1 < 0 || y1 > 2*R || y1 < 0 ||
    (sqrt(pow(x1-x0, 2)+ pow(y1-y0, 2)) > R)) {
        return true;
    }
    return false;
}

bool in_range(int x1, int y1, int x0, int y0, int rd) {
    float test = sqrt(pow(x1-x0, 2) + pow(y1-y0, 2));
    if (test <= rd)
        return true;
    return false;
}

void swap_ints(int *a , int *b) {
    int aux = *a;
    *a = *b;
    *b = aux;
}

// player1 e mai puternic si el incepe fight-ul
void battle(int id1, int id2, Elf *player, Glaciar **ghetar, int x,
    int y, FILE* output) {
    while (player[id1].hp > 0 && player[id2].hp > 0) {
        player[id2].hp = player[id2].hp - player[id1].dmg;
        if (player[id2].hp <= 0) {
            player[id1].stamina += player[id2].stamina; // ia stamina invinsului
            fprintf(output, "%s sent %s back home.\n",
                player[id1].name, player[id2].name);
            player[id2].alive = false;
            player[id1].kills++; // nr de jucatori eliminati creste
            ghetar[x][y].id = id1; // actualizarea playerului din celula
            return;
        }
        player[id1].hp = player[id1].hp - player[id2].dmg;
        if (player[id1].hp <= 0) {
            player[id2].stamina += player[id1].stamina;
            fprintf(output, "%s sent %s back home.\n",
                player[id2].name, player[id1].name);
            player[id1].alive = false;
            player[id2].kills++;
            ghetar[x][y].id = id2;
            return;
        }
    }
}

// daca elful cade de pe ghetar
bool fell_off(Glaciar **ghetar, Elf *player, int row, int col, int id, int R,
    FILE* output) {
    if (player[id].alive == true) {
        if (out_of_range(row, col, R, R, R) == true) {
            player[id].alive = false;
            fprintf(output, "%s fell off the glacier.\n", player[id].name);
            return true;
        }
    }
    return false;
}

void move_oneCell(Elf *player, int row, int col, int id, int dif) {
    player[id].x = row;
    player[id].y = col;
    player[id].stamina = player[id].stamina - dif;
}

int enough_stamina(Glaciar **ghetar, int x1, int y1, int x2, int y2, int R) {
    if (out_of_range(x2, y2, x1, y1, R) == true) {
        return 0;
    } else {
        return abs(ghetar[x1][y1].height - ghetar[x2][y2].height);
    }
}

void swap_gloves(Glaciar **ghetar, Elf *player, int row, int col, int id) {
    if (player[id].dmg < ghetar[row][col].gloves) {
        swap_ints(&player[id].dmg, &ghetar[row][col].gloves);
    }
}

void move(Glaciar **ghetar, Elf *player, int id, int *row, int *col,
    int row_new, int col_new, int R, FILE* out) {
    if (row_new < 0 || row_new > 2 * R || col_new < 0 || col_new > 2 * R)
        return;
    int owner = 0, dif = 0;
    // nu are nevoie de stamina daca vrea sa fie sinucigas
    dif = enough_stamina(ghetar, *row, *col, row_new, col_new, R);
    if (dif <= player[id].stamina) {
        if (fell_off(ghetar, player, row_new, col_new, id, R, out) == false) {
            // se muta in celula urmatoare deoarece are stamina si nici nu cade
            move_oneCell(player, row_new, col_new, id, dif);
            ghetar[*row][*col].id = -1; // celula de unde a plecat va fi goala
            // schimba echipamentul ,daca gaseste mai bun
            swap_gloves(ghetar, player, row_new, col_new, id);
            if (ghetar[row_new][col_new].id != -1) {
                owner = ghetar[row_new][col_new].id;
                // cand staminele sunt egale, da primul cel care vine
                if (player[id].stamina >= player[owner].stamina) {
                    battle(id, owner, player, ghetar, row_new, col_new, out);
                } else {
                    battle(owner, id, player, ghetar, row_new, col_new, out);
                }
            } else {
                // daca nu e nici un locatar,playerul preia celula fara bataie
                ghetar[row_new][col_new].id = id;
            }
        } else {
            // daca a cazut de pe ghetar, celula veche a lui va fi goala
            ghetar[*row][*col].id = -1;
        }
    }
    *row = row_new;
    *col = col_new;
}

void mutare(Glaciar **ghetar, Elf *player, int id, char *direction,
    int R, FILE* out) {
    if (player[id].alive == false) {
        return;
    }
    int len = strlen(direction);
    int row  = player[id].x, col = player[id].y;
    for (int i = 0; i < len; i++) {
        if (player[id].alive == false)
            return;
        row = player[id].x, col = player[id].y;
        // valorile unde se afla playerul la momentul actual
        // daca avanseaza se vor incrementa row/col si se va initializa
        // si player[id].x/player[id].y in functia MoveOneCell
        if (direction[i] == 'U') {
            move(ghetar, player, id, &row, &col, row-1, col, R, out);
        }
        if (direction[i] == 'D') {
            move(ghetar, player, id, &row, &col, row+1, col, R, out);
        }
        if (direction[i] == 'L') {
            move(ghetar, player, id, &row, &col, row, col-1, R, out);
        }
        if (direction[i] == 'R') {
            move(ghetar, player, id, &row, &col, row, col+1, R, out);
        }
    }
}

void base10_to_base2(int n, hex *dmg, hex *R, hex *x, hex *y) {
    int k = n;
    *dmg = (k >> 24) & 0xFF;
    *R = (k >> 16) & 0xFF;
    *x = (k >> 8) & 0xFF;
    *y = k & 0xFF;
}

void snowstorm(Glaciar **ghetar, Elf *player, int dmg, int rd, int x, int y,
    int R, FILE* output) {
    int id = 0;
    for (int i = 0; i < 2*R+1; i++) {
        for (int j = 0; j < 2*R+1; j++) {
            if (in_range(i, j, x, y, rd) == true) {
                id = ghetar[i][j].id;
                if (id != -1) {
                    if (player[id].alive == true) {
                        player[id].hp = player[id].hp - dmg;
                        if (player[id].hp <= 0) {
                            fprintf(output, "%s was hit by snowstorm.\n",
                                player[id].name);
                            player[id].alive = false;
                            ghetar[i][j].id = -1;
                        }
                    }
                }
            }
        }
    }
}

void meltdown(Glaciar **ghetar, Elf *player, int stamina_meltdown, int P,
    int *R, FILE* output) {
    int R_old = *R, id, x, y;
    (*R)--;
    for (int i = 0; i < P; i++) {
        if (player[i].alive == true) {
            x = player[i].x, y = player[i].y;
            if (in_range(x, y, R_old, R_old, *R) == true) {
                player[i].stamina += stamina_meltdown;
            } else {
                ghetar[x][y].id = -1;
                player[i].alive = false;
                fprintf(output, "%s got wet because of global warming.\n",
                    player[i].name);
            }
        }
    }
    // mutare in sus a elementelor
    for (int i = 0; i < 2*R_old; i++) {
        for (int j = 0; j < 2*R_old; j++) {
            id = ghetar[i+1][j].id;
            ghetar[i][j].height = ghetar[i+1][j].height;
            ghetar[i][j].gloves = ghetar[i+1][j].gloves;
            ghetar[i][j].id = ghetar[i+1][j].id;
            if (id != -1) {
                player[id].x = player[id].x - 1;
            }
        }
    }
    // mutare stanga
    for (int i = 0; i < 2*R_old; i++) {
        for (int j = 0; j < 2*R_old; j++) {
            id = ghetar[i][j+1].id;
            ghetar[i][j].height = ghetar[i][j+1].height;
            ghetar[i][j].gloves = ghetar[i][j+1].gloves;
            ghetar[i][j].id = ghetar[i][j+1].id;
            if (id != -1)
                player[id].y = player[id].y - 1;
        }
    }

    for (int i = 2*(*R)+1; i < 2*R_old+1; i++) {
        free(ghetar[i]);
    }
    for (int j = 0; j < 2*(*R)+1; j++) {
        ghetar[j] = realloc(ghetar[j], (2*(*R)+1)*sizeof(Glaciar));
    }
}

// jocul se termina cand ramane un singur castigator :)
bool winner(Glaciar **ghetar, Elf *player, int R, FILE* output) {
    int count = 0, temp_i = 0, temp_j = 0;
    for (int i = 0; i < 2*R+1; i++) {
        for (int j = 0; j < 2*R+1; j++) {
            if (ghetar[i][j].id != -1) {
                // cand un player se muta,ar trebui sa modific in casuta
                // anterioara ca id == -1 deoarece el nu mai sta acolo..
                count++;
                temp_i = i;
                temp_j = j;
            }
        }
    }
    if (count == 1) {
        int id = ghetar[temp_i][temp_j].id;
        fprintf(output, "%s has won.\n", player[id].name);
        return true;
    }
    return false;
}

void swap_strings(char *name1, char *name2) {
    char *temp;
    if (strlen(name1) > strlen(name2)) {
        temp = malloc(strlen(name1)+1);
    } else {
        temp = malloc(strlen(name2)+1);
    }
    strncpy(temp, name1, strlen(name1)+1);
    strncpy(name1, name2, strlen(name2)+1);
    strncpy(name2, temp, strlen(temp)+1);
    free(temp);
}

void sortelo(char **wet, int kills[], char **name, int P) {
    int i, j;
    for (i = 0; i < P-1; i++) {
        for (j = 0; j < P-i-1; j++) {
           if (((strcmp(wet[j], "WET") == 0) &&
            (strcmp(wet[j+1], "DRY") == 0))) {
                swap_strings(wet[j], wet[j+1]);
                swap_strings(name[j], name[j+1]);
                swap_ints(&kills[j], &kills[j+1]);
           }
           if ((strcmp(wet[j], "DRY") && strcmp(wet[j+1], "DRY")) ||
                (strcmp(wet[j], "WET") && strcmp(wet[j+1], "WET"))) {
               if (kills[j] < kills[j+1]) {
                    swap_strings(wet[j], wet[j+1]);
                    swap_strings(name[j], name[j+1]);
                    swap_ints(&kills[j], &kills[j+1]);
               }
               if (kills[j] == kills[j+1]) {
                    if (strcmp(name[j], name[j+1]) > 0) {
                        swap_strings(wet[j], wet[j+1]);
                        swap_strings(name[j], name[j+1]);
                        swap_ints(&kills[j], &kills[j+1]);
                    }
                }
            }
        }
    }
}


void print_scoreboard(Elf *player, int P, int R, FILE* output) {
    int kills[P];
    char **wet = malloc(P * sizeof(char*));
    char **name = malloc(P * sizeof(char*));
    for (int i = 0; i < P; i++) {
        kills[i] = player[i].kills;
        name[i] = malloc(MAX * sizeof(char));
        wet[i] = malloc((strlen("DRY")+1) * sizeof(char));
        // wet daca .alive = false dry daca = true;
        if (player[i].alive == true) {
            strncpy(wet[i], "DRY", strlen("DRY")+1);
        } else {
            strncpy(wet[i], "WET", strlen("WET")+1);
        }
        strncpy(name[i], player[i].name, strlen(player[i].name)+1);
    }
    sortelo(wet, kills, name, P);
    fprintf(output, "SCOREBOARD:\n");
    for (int i = 0; i < P; i++) {
        fprintf(output, "%s\t%s\t%d\n", name[i], wet[i], kills[i]);
    }
    for (int i = 0; i < P; i++) {
        free(wet[i]);
        free(name[i]);
    }
    free(wet);
    free(name);
}

int main(int argc, char **argv) {
    char infile[] = "snowfight.in";
    char outfile[] = "snowfight.out";

    FILE* input = fopen(infile, "rt+");
    if (input == NULL) {
        printf("Couldn't open the file for reading\n");
        return 2;
    }

    FILE* output = fopen(outfile, "wt+");
    if (output == NULL) {
        printf("Couldn't open the file for writing\n");
        return 3;
    }
    int R, P, height, gloves;
    fscanf(input, "%d %d", &R, &P);
    Glaciar **ghetar = (Glaciar**) malloc ((2*R+1) * sizeof (Glaciar*));
    for (int i = 0; i < 2*R+1; i++) {
        ghetar[i] = (Glaciar*) malloc ((2*R+1) * sizeof (Glaciar));
    }
    for (int i = 0; i < 2*R+1; i++) {
        for (int j = 0; j < 2*R+1; j++) {
            fscanf(input, "%d %d", &height, &gloves);
            ghetar[i][j].height = height;
            ghetar[i][j].gloves = gloves;
            ghetar[i][j].id = -1;
        }
    }

    Elf *player = (Elf*) malloc(P * sizeof(Elf));
    char name[MAX], com[MAX], direction[MAX];
    for (int i = 0; i < P; i++) {
        player[i].alive = true;
        player[i].kills = 0;
        fscanf(input, "%s %d %d %d %d", name, &player[i].x, &player[i].y,
            &player[i].hp, &player[i].stamina);
        strncpy(player[i].name, name, strlen(name)+1);
        if (out_of_range(player[i].x, player[i].y, R, R, R) == true) {
            fprintf(output, "%s has missed the glacier.\n", player[i].name);
            player[i].alive = false;
        } else {
            player[i].dmg = ghetar[player[i].x][player[i].y].gloves;
            ghetar[player[i].x][player[i].y].gloves = 0;
            ghetar[player[i].x][player[i].y].id = i;
        }
    }
    int ok = 0, id = 0, stamina_meltdown = 0, k = 0;
    if (winner(ghetar, player, R, output) == true) {
    // pt situatia in care un singur elf nu e sinucigas la initializare
        ok = 1;
    }

    hex dmg, range, x, y;
    while (fscanf(input, "%s", com) != EOF && ok == 0) {
        if (strcmp("MOVE", com) == 0) {
            fscanf(input, "%d %s", &id, direction);
            mutare(ghetar, player, id, direction, R, output);
        }
        if (strcmp("MELTDOWN", com) == 0) {
            fscanf(input, "%d", &stamina_meltdown);
            meltdown(ghetar, player, stamina_meltdown, P, &R, output);
        }
        if (strcmp("SNOWSTORM", com) == 0) {
            fscanf(input, "%d", &k);
            base10_to_base2(k, &dmg, &range, &y, &x);
            snowstorm(ghetar, player, dmg, range, x, y, R, output);
        }
        if (strcmp("PRINT_SCOREBOARD", com) == 0) {
            print_scoreboard(player, P, R, output);
        }
        if (winner(ghetar, player, R, output) == true) {
            break;
        }
    }
    for (int i = 0; i < 2*R+1; i++) {
        free(ghetar[i]);
    }
    free(ghetar);
    free(player);
    fclose(input);
    fclose(output);
    printf("\n");
    return 0;
}
