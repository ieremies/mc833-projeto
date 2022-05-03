#include "net_utils.h"

#define MAX_MOVIE_GENRES 10

typedef struct {
    int id;
    char title[MAX_STR_LEN];
    int num_genres;
    char genre_list[MAX_MOVIE_GENRES][MAX_STR_LEN];
    char director_name[MAX_STR_LEN];
    int year;
    // imagem capa;
} Movie;

Movie *create_movie(int id, char *title, char *genre, char *director, int year);
void add_genre(Movie *movie, char *genre);
void print_movie(Movie *movie);
void print_title_id(Movie *movie);