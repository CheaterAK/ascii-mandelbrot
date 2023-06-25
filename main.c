#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

typedef struct {
    double real;
    double img;
} complex;

#define MAX_ITER 100
int plus_iter = 0;
void draw_mandelbrot(complex min, complex max, int zoom_level, int width, int height);

void zoom_in(complex min, complex max, int zoom_level, int width, int height)
{
    double x_center = (min.real + max.real) / 2;
    double y_center = (min.img + max.img) / 2;
    double x_range = (max.real - min.real) / 2;
    double y_range = (max.img - min.img) / 2;
    draw_mandelbrot((complex){x_center - x_range / 2, y_center - y_range / 2}, (complex){x_center + x_range / 2, y_center + y_range / 2}, zoom_level + 1, width, height);
}

void zoom_out(complex min, complex max, int zoom_level, int width, int height)
{
    double x_center = (min.real + max.real) / 2;
    double y_center = (min.img + max.img) / 2;
    double x_range = (max.real - min.real) * 2;
    double y_range = (max.img - min.img) * 2;
    draw_mandelbrot((complex){x_center - x_range / 2, y_center - y_range / 2}, (complex){x_center + x_range / 2, y_center + y_range / 2}, zoom_level - 1, width, height);
}

void move_up(complex min, complex max, int zoom_level, int width, int height)
{
    double y_range = (max.img - min.img) / (1 << (zoom_level < 5 ? zoom_level : 5));
    draw_mandelbrot((complex){min.real, min.img - y_range}, (complex){max.real, max.img - y_range}, zoom_level, width, height);
}

void move_down(complex min, complex max, int zoom_level, int width, int height)
{
    double y_range = (max.img - min.img) / (1 << (zoom_level < 5 ? zoom_level : 5));
    draw_mandelbrot((complex){min.real, min.img + y_range}, (complex){max.real, max.img + y_range}, zoom_level, width, height);
}

void move_right(complex min, complex max, int zoom_level, int width, int height)
{
    double x_range = (max.real - min.real) / (1 << (zoom_level < 5 ? zoom_level : 5));
    draw_mandelbrot((complex){min.real + x_range, min.img}, (complex){max.real + x_range, max.img}, zoom_level, width, height);
}

void move_left(complex min, complex max, int zoom_level, int width, int height)
{
    double x_range = (max.real - min.real) / (1 << (zoom_level < 5 ? zoom_level : 5));
    draw_mandelbrot((complex){min.real - x_range, min.img}, (complex){max.real - x_range, max.img}, zoom_level, width, height);
}

void increase_iter(complex min, complex max, int zoom_level, int width, int height)
{
    plus_iter += 10;
    draw_mandelbrot(min, max, zoom_level, width, height);
}

void decrease_iter(complex min, complex max, int zoom_level, int width, int height)
{
    plus_iter -= 10;
    draw_mandelbrot(min, max, zoom_level, width, height);
}

void resize(int width, int height, complex min, complex max, int zoom_level)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    width = w.ws_col;
    height = w.ws_row - 1;
    draw_mandelbrot(min, max, zoom_level, width, height);
    draw_mandelbrot(min, max, zoom_level, width, height);
}

void reset(int width, int height)
{
    complex min = {-2.0, -2.0};
    complex max = {2.0, 2.0};
    draw_mandelbrot(min, max, 1, width, height);
}

void command_parser(char *command, complex min, complex max, int zoom_level, int width, int height)
{
    if (strcmp(command, "x\n") == 0)
        return;
    else if (strcmp(command, "+\n") == 0)
        zoom_in(min, max, zoom_level, width, height);
    else if (strcmp(command, "-\n") == 0)
        zoom_out(min, max, zoom_level, width, height);
    else if (strcmp(command, "w\n") == 0)
        move_up(min, max, zoom_level, width, height);
    else if (strcmp(command, "s\n") == 0)
        move_down(min, max, zoom_level, width, height);
    else if (strcmp(command, "d\n") == 0)
        move_right(min, max, zoom_level, width, height);
    else if (strcmp(command, "a\n") == 0)
        move_left(min, max, zoom_level, width, height);
    else if (strcmp(command, "q\n") == 0)
        decrease_iter(min, max, zoom_level, width, height);
    else if (strcmp(command, "e\n") == 0)
        increase_iter(min, max, zoom_level, width, height);
    else if (strcmp(command, ".\n") == 0)
        resize(width, height, min, max, zoom_level);
    else if (strcmp(command, "reset\n") == 0)
        reset(width, height);
    else if (strcmp(command, "help\n") == 0)
        printf("x: exit\n+: zoom in\n-: zoom out\nw: move up\ns: move down\nd: move right\na: move left\nq: decrease iteration\ne: increase iteration\n.: resize\nreset: reset image\nhelp: help\n");
    else
        draw_mandelbrot(min, max, zoom_level, width, height);
}


void draw_mandelbrot(complex min, complex max, int zoom_level, int width, int height) {
    double x_step = (max.real - min.real) / width;
    double y_step = (max.img - min.img) / height;
    char *grid = (char *) malloc(width * height * sizeof(char));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            complex c = {min.real + x * x_step, min.img + y * y_step};
            complex z = {0.0, 0.0};
            int i;
            for (i = 0; i < MAX_ITER + plus_iter; i++) {
                complex z_new = {z.real * z.real - z.img * z.img + c.real, 2 * z.real * z.img + c.img};
                z = z_new;
                if (z.real * z.real + z.img * z.img > 4.0) {
                    break;
                }
            }
            grid[y * width + x] = i % (126 - 33) + 33;
        }
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("\033[3%dm%c\033[0m",(grid[y * width + x] - 33) % 7 + 1, grid[y * width + x]);
        }
        printf("\n");
    }
    free(grid);
    printf("Enter a command ('help' to display commands): ");
    char command[10];
    fgets(command, 10, stdin);
    command_parser(command, min, max, zoom_level, width, height);
}



int main() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
    int height = w.ws_row - 1;
    int zoom_level = 1;
    complex min = {-2.0, -1.5};
    complex max = {1.0, 1.5};

    draw_mandelbrot(min, max, zoom_level, width, height);

    return 0;
}
