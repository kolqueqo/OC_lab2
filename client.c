#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "There must be two arguments\n" );
        return 0;
    }
    if (argc > 2){
        fprintf(stderr, "There must be two arguments\n" );
        return 0;
    }

    char *p;
    errno = 0;
    long PID = strtol(argv[1], &p, 10);
    if (*p != '\0' || errno != 0){
        fprintf(stderr, "PID must be number.\n");
        return 0;
    }

    if(PID<0){
        fprintf(stderr, "PID must be positive \n" );
        return 0;
    }

    char inbuf[4096];
    char outbuf[4096];
    int fd = open("/proc/lab2/struct_info", O_RDWR);
    sprintf(inbuf, "%s", argv[1]);
    write(fd, inbuf, 17);


    int fd2 = open("/proc/lab2/struct_info", O_RDWR);
    read(fd2, outbuf, 4096);

    puts(outbuf);
    return 0;
}