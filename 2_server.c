#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define FNAME file1
#define PORT 8010
#define BUFSIZE 128
#define LISTENQ 5
#define MAX_CMDSIZE 4

int main(int argc, char **argv)
{
	int	listenfd, connfd, fd, pid, n, size;
	struct sockaddr_in servaddr;
	char buf[BUFSIZE],fname[50];

	struct stat stat_buf; 	
	


	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(PORT);	

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);
		printf("listening\n");
	int cmd;			
	
	for ( ; ; ) 
	{
		connfd = accept(listenfd, (struct sockaddr *) NULL,NULL);
		printf("--- Handling connection request ---\n");
		
		while(1)
		{
			recv(connfd,&cmd,sizeof(cmd),0);
			printf("** %d **\n",cmd);
			
			switch(cmd)
			{
				//when client wants to retrieve file from server
				case 1:
				
					recv(connfd,fname,50,0);
					printf("Retrieve request for file %s : ", fname);  

					fd=open(fname,O_RDONLY,S_IRUSR);
					fstat(fd, &stat_buf);
					size = stat_buf.st_size;
						
					printf(" size is %d\n", size); 

					printf("\nopened file\n");
					while ( (n = read(fd, buf, BUFSIZE-1)) > 0) 
					{
						buf[n] = '\0';
						printf("%s\n",buf);
						write(connfd,buf,n);
					}
					printf("\nfile transfer completed \n");
					break;

				//when client wants to store files in server
				case 2:
			
					recv(connfd,fname,50,0);
					printf("Storage request for file %s\n", fname);  

					fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);		
					while ( (n = read(connfd, buf, BUFSIZE-1)) > 0)
					{
						buf[n] = '\0';
						printf("%s\n",buf);	
						write(fd,buf,n);
						if( n < BUFSIZE-2)
							break;
					}
					printf("Server: Recieved and Stored\n");
					break;
				

				case 3:
		
					system("ls > server_filelist.txt");
					fd=open("server_filelist.txt",O_RDONLY,S_IRUSR);
					
					while ( (n = read(fd, buf, BUFSIZE-1)) > 0) 
					{
						buf[n] = '\0';
						printf("%s\n",buf);
						write(connfd,buf,n);
					}
					printf("\nfile list sent to client\n");
					break;
				
			
				default:
					perror("Invalid Request :| \n");
					break;
			}
		}
		
		close(connfd);
		close(fd);
		//exit(1);
	
	}
}
