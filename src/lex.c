#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIEW_IMPLEMENTATION
#include "view.h"

typedef enum { YELLOW_COLOR = 1,
    BLUE_COLOR,
    GREEN_COLOR,
    RED_COLOR,
    CYAN_COLOR,
} Color_Pairs;

char *types[] = {
    "char",
    "double",
    "float",
    "int",
    "long",
    "short",
    "void",
    "size_t",
};

char *keywords[] = {
    "auto",
    "break",
    "case",
    "const",
    "continue",
    "default",
    "do",
    "else",
    "enum",
    "extern",
    "for",
    "goto",
    "if",
    "register",
    "return",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "volatile",
    "while",
};
#define NUM_KEYWORDS sizeof(keywords)/sizeof(*keywords)
#define NUM_TYPES sizeof(types)/sizeof(*types)

typedef enum {
    Type_None = 0,
    Type_Keyword,
    Type_Type,
    Type_Preprocessor,
    Type_Comment,
} Token_Type;

typedef struct {
    Token_Type type;
    size_t index;
    size_t size;
} Token;

int is_keyword(char *word, size_t word_s) {
    for(size_t i = 0; i < NUM_KEYWORDS; i++) {
        if(word_s < strlen(keywords[i])) continue;
        if(strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

int is_type(char *word, size_t word_s) {
    for(size_t i = 0; i < NUM_TYPES; i++) {
        if(word_s < strlen(types[i])) continue;
        if(strcmp(word, types[i]) == 0) return 1;
    }
    return 0;
}

int is_in_tokens_index(Token *token_arr, size_t token_s, size_t index, size_t *size, Color_Pairs *color) {
    for(size_t i = 0; i < token_s; i++) {
        if(token_arr[i].index == index) {
            *size = token_arr[i].size;
            switch(token_arr[i].type) {
                case Type_None:
                    break;
                case Type_Keyword:
                    *color = BLUE_COLOR;
                    break;
                case Type_Type:
                    *color = YELLOW_COLOR;
                    break;
                case Type_Preprocessor:
                    *color = CYAN_COLOR;
                    break;
                case Type_Comment:
                    *color = GREEN_COLOR;
                    break;
            }
            return 1;
        }
    }
    return 0;
}

Token generate_word(String_View *view, char *contents) {
    size_t index = view->data - contents;
    char word[32] = {0};
    size_t word_s = 0;
    while(view->len > 0 && (isalpha(view->data[0]) || view->data[0] == '_')) {
        if(word_s >= 32) break;
        word[word_s++] = view->data[0]; 
        view->data++;
        view->len--;
    }
    view->data--;
    view->len++;
    if(is_keyword(word, word_s)) {
        return (Token){.type = Type_Keyword, .index = index, .size = word_s};
    } else if(is_type(word, word_s)) {
        return (Token){.type = Type_Type, .index = index, .size = word_s};
    }
    return (Token){Type_None};
}

size_t generate_tokens(char *line, size_t line_s, Token *token_arr, size_t *token_arr_capacity) {
    size_t token_arr_s = 0;

    String_View view = view_create(line, line_s);
    while(view.len > 0) {
        view = view_trim_left(view);
        if(isalpha(view.data[0])) {
            Token token = generate_word(&view, line);
            if(token_arr_s >= *token_arr_capacity) {
                token_arr = realloc(token_arr, sizeof(Token)*(*token_arr_capacity)*2);
                *token_arr_capacity *= 2;
            }
            if(token.type != Type_None) {
                token_arr[token_arr_s++] = token;
            }
        } else if(view.data[0] == '#') {
            Token token = {
                .type = Type_Preprocessor,
                .index = 0,
                .size = view.len,
            };
            token_arr[token_arr_s++] = token;
        } else if(view.len >= 2 && view.data[0] == '/' && view.data[1] == '/') {
            Token token = {
                .type = Type_Comment,
                .index = view.data-line,
                .size = view.len,
            };
            token_arr[token_arr_s++] = token;
        }
        view.data++;
        if(view.len == 0) break;
        view.len--;
    }

    return token_arr_s;
}