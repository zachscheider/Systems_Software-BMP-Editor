/*
Zach Scheider
CS0449
Program that edits bmp files
*/

#pragma pack(1)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int check_headers(char filename[]);
void change_image(char filename[], char operation[]);
void invert_pixel(FILE *fp);
void grayscale_pixel(FILE *fp);

struct header{

    char identifier[2];
    unsigned int file_size[1];
    unsigned short reserve1[1];
    unsigned short reserve2[1];
    unsigned int pixel_start[1];

};

struct dib_header{

    unsigned int header_size[1];
    unsigned int width[1];
    unsigned int height[1];
    unsigned short color_planes[1];
    unsigned short bits_per_pixel[1];
    unsigned short compression[1];
    unsigned int image_size[1];
    unsigned int res_horizontal[1];
    unsigned int res_vertical[1];
    unsigned int palette_size[1];
    unsigned int num_colors[1];

};

struct pixel{

    unsigned char blue[1];
    unsigned char green[1];
    unsigned char red[1];

};

struct header the_header;
struct dib_header the_dib_header;

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Two arguments are expected.");
        return 0;
    }

    check_headers(argv[2]);
    change_image(argv[2], argv[1]);

    return 0;
}

// checks to make sure the file is valid
int check_headers(char filename[]){
    FILE *fp;

    fp = fopen(filename, "r");

    fread(the_header.identifier, 1, 2, fp);
    if('B' != the_header.identifier[0] && 'M' != the_header.identifier[1]){
        printf("We do not support the file format. Exiting now.");
        fclose(fp);
        return 0;
    }

    fread(the_header.file_size, 4, 1, fp);
    fread(the_header.reserve1, 2, 1, fp);
    fread(the_header.reserve2, 2, 1, fp);
    fread(the_header.pixel_start, 4, 1, fp);

    fread(the_dib_header.header_size, 4, 1, fp);
    if(40 != the_dib_header.header_size[0]){
        printf("We do not support the file format. Exiting now.");
        fclose(fp);
        return 0;
    }

    fread(the_dib_header.width, 4, 1, fp);
    fread(the_dib_header.height, 4, 1, fp);
    fread(the_dib_header.color_planes, 2, 1, fp);
    fread(the_dib_header.bits_per_pixel, 2, 1, fp);
    fread(the_dib_header.compression, 4, 1, fp);
    fread(the_dib_header.image_size, 4, 1, fp);
    fread(the_dib_header.res_horizontal, 4, 1, fp);
    fread(the_dib_header.res_vertical, 4, 1, fp);
    fread(the_dib_header.palette_size, 4, 1, fp);
    fread(the_dib_header.num_colors, 4, 1, fp);

    fclose(fp);
}

// changes the image depending on the argument given
void change_image(char filename[], char operation[]){
    FILE *fp;
    int row_counter, col_counter, pad;

    fp = fopen(filename, "r+");
    fseek(fp, the_header.pixel_start[0], SEEK_SET);

    if(0 == (3 * the_dib_header.width[0] % 4)){
        pad = 0;
    }else{
        pad = 4 - the_dib_header.width[0] % 4;
    }

    // loops through every pixel
    for(col_counter = 0; col_counter < the_dib_header.height[0]; col_counter++){
        for(row_counter = 0; row_counter < the_dib_header.width[0]; row_counter++){
            if(strcmp("-invert", operation) == 0){
                invert_pixel(fp);
            }else if(strcmp("-grayscale", operation) == 0){
                grayscale_pixel(fp);
            }else{
                printf("We do not support that operation. Exiting now.");
                fclose(fp);
                return;
            }
        }
        
        fseek(fp, pad, SEEK_CUR);
    }

    fclose(fp);
}

// inverts a pixel
void invert_pixel(FILE *fp){
    struct pixel curr_pixel;

    fread(&curr_pixel.blue, 1, 1, fp);
    fread(&curr_pixel.green, 1, 1, fp);
    fread(&curr_pixel.red, 1, 1, fp);

    curr_pixel.blue[0] = ~curr_pixel.blue[0];
    curr_pixel.green[0] = ~curr_pixel.green[0];
    curr_pixel.red[0] = ~curr_pixel.red[0];

    fseek(fp, -3, SEEK_CUR);
    fwrite(&curr_pixel.blue, 1, 1, fp);
    fwrite(&curr_pixel.green, 1, 1, fp);
    fwrite(&curr_pixel.red, 1, 1, fp);

}

// converts a pixel to grayscale
void grayscale_pixel(FILE *fp){
    double norm_blue, norm_green, norm_red, magic_y, exp;
    unsigned char grayscale_value;
    struct pixel curr_pixel;

    fread(curr_pixel.blue, 1, 1, fp);
    fread(curr_pixel.green, 1, 1, fp);
    fread(curr_pixel.red, 1, 1, fp);

    norm_blue = curr_pixel.blue[0] / 255.0;
    norm_green = curr_pixel.green[0] / 255.0;
    norm_red = curr_pixel.red[0] / 255.0;
    
    magic_y = norm_red * 0.2126 + norm_green * 0.7152 + norm_blue * 0.0722;

    if(magic_y > 0.0031308){
        exp = 1 / 2.4;
        grayscale_value = (1.055 * pow(magic_y, exp) - 0.055) * 255;
    }else{
        grayscale_value = (12.92 * magic_y) * 255;
    }

    fseek(fp, -3, SEEK_CUR);
    fwrite(&grayscale_value, 1, 1, fp);
    fwrite(&grayscale_value, 1, 1, fp);
    fwrite(&grayscale_value, 1, 1, fp);
    
}