// TODO: check and remove unused header files
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define SERVER_PORT 80

#define MAXLINE 4096
#define SA struct sockaddr

void error_and_die(const char *fmt, ...);

int main(int argc, char **argv)
{
  int sockfd;
  int n;
  int sendbytes;
  struct sockaddr_in servaddr;
  char sendline[MAXLINE];
  char recvline[MAXLINE];

  if (argc != 2)
  {
    error_and_die("Usage: %s <server address>", argv[0]);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    error_and_die("Error while creating the socket");
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_PORT);
  
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
  {
    error_and_die("inet_pton error for %s", argv[1]);
  }

  if (connect(sockfd, (SA *) &servaddr, sizeof (servaddr)) < 0)
  {
    error_and_die("Connect failed!");
  }

  // We're connected. Prepare the message.
  sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
  sendbytes = strlen(sendline);

  // Send the request -- making sure you send it all
  // This code is a bit fragile, since it bails if only some of the
  // bytes are sent
  if (write(sockfd, sendline, sendbytes) != sendbytes)
  {
    error_and_die("Write error");
  }

  memset(recvline, 0, MAXLINE);

  // Now read the server's response.
  while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
  {
    printf("%s", recvline);
  }

  if (n < 0)
  {
    error_and_die("Read error");
  }

  exit(0);
}

void error_and_die(const char *fmt, ...)
{
  int errno_save;
  va_list     ap;


  // any system or library call can set errno
  // so we need to save it now
  errno_save = errno;

  // print out the fmt + args to standard out
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
  fflush(stdout);

  // print out error message if errno was set
  if (errno_save != 0)
  {
    fprintf(
      stdout, "(errno=%d) : %s\n",
      errno_save,
      strerror(errno_save)
    );

    fprintf(stdout, "\n");
    fflush(stdout);
  }
  va_end(ap);

  // This is the ..and_die part.
  // Terminate with an error.
  exit(1);
}



