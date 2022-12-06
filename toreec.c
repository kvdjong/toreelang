#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_WORD_LEN 32 // Max Word Length
#define TOKENV_INC 64 // Token Vector Increment

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

enum __ops {
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
    char *tokenv = malloc(TOKENV_INC); // Token Vector
    int tokenv_idx = -1; // Token Vector Index
    int tokenv_allocs = 1; // Token Vector Allocations Done
    unsigned int chr; // Char
    char word[MAX_WORD_LEN + 1] = { 0 }; // Word (+1 is for null terminator)
    int word_idx = -1; // Word Index
    
    do {
        chr = fgetc(file);

        if (chr >= 0x41 && chr <= 0x5A) { // Uppercase
            if (word_idx + 1 == MAX_WORD_LEN) {
                printf("Error: Unparseable word: %s...\n", word);
                return NULL;
            }
            word_idx++;
            word[word_idx] = chr;
        } else if (chr >= 0x61 && chr <= 0x7A) { // Lowercase
            if (word_idx + 1 == MAX_WORD_LEN) {
                printf("Error: Unparseable word: %s...\n", word);
                return NULL;
            }
            word_idx++;
            word[word_idx] = chr - 0x20;
        }

        for (int i = 0; i < 8; i++) {
            char *op = ops[i];
            if (strncmp(op, word, strlen(op)) == 0) { // Word matches an operator
                // Reallocate token vector if it is too small
                if (tokenv_idx + 1 == tokenv_allocs * TOKENV_INC) {
                    tokenv_allocs++;
                    tokenv = realloc(tokenv, tokenv_allocs * TOKENV_INC);
                    if (tokenv == NULL) {
                        printf("Error: Could not reallocate memory for token vector\n");
                        return NULL;
                    }
                }

                tokenv_idx++;
                tokenv[tokenv_idx] = i;
                word_idx = -1;
                memset(word, 0, MAX_WORD_LEN);
                break;
            }
        }
    } while (chr != EOF);

    if (word_idx != -1) { // Current word is not empty
        printf("Error: Unparseable word: %s...\n", word);
        return NULL;
    }

    *len = tokenv_allocs * TOKENV_INC;
    return tokenv;
}

int transpile(FILE *file, char *tokens, int tokens_len) {
    fputs("#include <stdio.h>\nint main(void) {char array[30000] = {0};char *ptr = array;", file);
    for (int i = 0; i < tokens_len; i++) {
        char op = tokens[i];
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



int process(char *in_filename, char *out_filename) {
    FILE *file = fopen(in_filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", in_filename);
        return 1;
    }

    int tokens_len = 0;
    char *tokens = scan(file, &tokens_len);
    fclose(file);
    if (tokens == NULL) return 1;

    

    file = fopen(out_filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", out_filename);
        return 1;
    }
    int transpile_res = transpile(file, tokens, tokens_len);
    fclose(file);
    if (transpile_res != 0) return 1;

    printf("Successfully transpiled %s into %s!\n", in_filename, out_filename);
    return 0;
}
int main(int argc, char *argv[]) {
    if (argc != 2) { // Does not have 1 argument passed
        printf("Usage: %s [filename]\n", argv[0]);
        return 1;
    }
    
    char *in_filename = argv[1];

    size_t out_filename_size = strlen(in_filename) + 3;
    char *out_filename = malloc(out_filename_size);
    if (out_filename == NULL) {
        printf("Error: Could not allocate memory for output file name\n");
        return 1;
    }
    strncpy(out_filename, in_filename, out_filename_size);
    strncat(out_filename, ".c", out_filename_size);

    int res = process(in_filename, out_filename);
    free(out_filename);
    return res;
}