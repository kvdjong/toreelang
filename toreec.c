#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WORDMAXLEN 32 // Max Word Length
#define TOKENVINC 64 // Token Vector Increment

char *ops[] = {
    "RIGHT",
    "LEFT",
    "UP",
    "DOWN",
    "TOREEISTHEVERYBEST",
    "MANILOVETOHEARTHATSOUND",
    "DOITTILYOUGETENOUGH",
    "WEREALWAYSUPFORAFIGHT",
};

enum {
    SHIFT_RIGHT,
    SHIFT_LEFT,
    INCREMENT,
    DECREMENT,
    CHAR_OUT,
    CHAR_IN,
    LOOP_START,
    LOOP_END,
};

char *scan(FILE *file, int *len) {
    char *tokenv = malloc(TOKENVINC); // Token Vector
    int tokenvidx = -1; // Token Vector Index
    int tokenvallocs = 1; // Token Vector Allocations Done
    unsigned int chr; // Char
    char word[WORDMAXLEN] = { 0 }; // Word
    int wordidx = -1; // Word Index
    
    do {
        chr = fgetc(file);

        if (chr >= 0x41 && chr <= 0x5A) { // Uppercase
            if (wordidx + 1 == WORDMAXLEN) {
                printf("Error: Unparseable sentence: %s\n", word);
                return NULL;
            }
            wordidx++;
            word[wordidx] = chr;
        } else if (chr >= 0x61 && chr <= 0x7A) { // Lowercase
            if (wordidx + 1 == WORDMAXLEN) {
                printf("Error: Unparseable sentence: %s\n", word);
                return NULL;
            }
            wordidx++;
            word[wordidx] = chr - 0x20;
        } else {
            for (int i = 0; i < 8; i++) {
                char *op = ops[i];
                if (strncmp(op, word, strlen(op)) == 0) { // Word matches an operator
                    if (tokenvidx + 1 == tokenvallocs * TOKENVINC) { // Realloc if Token Vector is too small
                        tokenvallocs++;
                        tokenv = realloc(tokenv, tokenvallocs * TOKENVINC);
                        if (tokenv == NULL) {
                            printf("Error: Could not reallocate memory for Token Vector\n");
                            return NULL;
                        }
                    }

                    tokenvidx++;
                    tokenv[tokenvidx] = i;
                    wordidx = -1;
                    memset(word, 0, WORDMAXLEN);
                    break;
                }
            }
        }
    } while (chr != EOF);
    *len = tokenvallocs * TOKENVINC;
    return tokenv;
}

int compile(FILE *file, char *buf, int buflen) {
    fputs("#include <stdio.h>\nint main(void) {char array[30000] = {0};char *ptr = array;", file);
    for (int i = 0; i < buflen; i++) {
        char op = buf[i];
        if (op == SHIFT_RIGHT) fputs("++ptr;", file);
        else if (op == SHIFT_LEFT) fputs("--ptr;", file);
        else if (op == INCREMENT) fputs("++*ptr;", file);
        else if (op == DECREMENT) fputs("--*ptr;", file);
        else if (op == CHAR_OUT) fputs("putchar(*ptr);", file);
        else if (op == CHAR_IN) fputs("*ptr = getchar();", file);
        else if (op == LOOP_START) fputs("while (*ptr) {", file);
        else if (op == LOOP_END) fputs("}", file);
        else {
            printf("Error: Invalid token on Token Vector: %02X\n", op);
            return 1;
        }
    }
    fputs("}", file);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [filename]\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    int buflen = 0;
    char *buf = scan(file, &buflen);
    if (buf == NULL) return 1;
    fclose(file);

    char *outfilename = malloc(strlen(argv[1]) + 3);
    strcpy(outfilename, argv[1]);
    strcat(outfilename, ".c");
    file = fopen(outfilename, "w");
    if (compile(file, buf, buflen) != 0) {
        return 1;
    }
    fclose(file);

    printf("Successfully compiled %s into %s!\n", argv[1], outfilename);
    free(outfilename);
    return 0;
}