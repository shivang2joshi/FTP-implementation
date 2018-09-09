#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<net/if_arp.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include<signal.h>

#define FNAME file1
#define PORT 8010
#define BUFSIZE 128
#define MAX_CMDSIZE 4
#define GET 1
#define PUT 2
#define LIST 3
#define EXIT 4

int getCommand(char* S)
{
	if(strcmp(S,"GET")==0)
		return GET;
	else if(strcmp(S,"PUT")==0)
		return PUT;
	else if(strcmp(S,"LIST")==0)
		return LIST;
	else if(strcmp(S,"EXIT")==0)
		return EXIT;
}

	
void handler(int sig)
{
	printf("\nFTP Command Usage: \nGET <press 1>		-to recieve file from server\n");
	printf("PUT <press 2>		-to store file on server\n");
	printf("LIST <press 3>		-to see the list of stored files at server\n");
	printf("EXIT <press 4>		-to quit to terminal\n");
	printf("\nNote: Please use FTP commands instead of terminating :)\n");
	printf("\nEnter command : ");
	fflush(stdout);
}


int main(int argc, char **argv)
{
	signal(SIGINT, handler);
	
	int sockfd, fd, n, size,count=0;
	long int size1,size2;
	char buf[BUFSIZE], fname[50];
	struct sockaddr_in servaddr;
	struct stat stat_buf;
	
	if (argc != 2) { 
		printf("Usage: %s server_address\n", argv[0]); 
		exit(1);
	}
	
	//****************** Connection Back-end code START *****************************//	
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit(1);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(PORT);	
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		exit(1);

	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		perror("Not working :(\n"),
		exit(1);
	printf("connection established\n");
	//****************** Connection Back-end code END *****************************//	


	printf("FTP Command Usage: \nGET  <press 1>		-to recieve file from server\n");
	printf("PUT  <press 2>		-to store file on server\n");
	printf("LIST <press 3>		-to see the list of stored files at server\n");
	printf("EXIT <press 4>		-to quit to terminal\n");
	
	bool active=1;

	while(active)
	{
		printf("\nEnter command : "); 	
		int cmd;
		scanf("%d",&cmd);
		
		switch(cmd)
		{
			case GET:
				send(sockfd,&cmd, sizeof(cmd) ,0);	//sizeof integer 4
				printf("File name : ");		
				scanf("%s",fname);						
				send(sockfd,fname,sizeof(fname),0);
				//printf("%s",fname);
						
				fd=open(strcat(fname,"_fromserver"),O_WRONLY|O_CREAT,S_IRWXU);
				fstat(fd, &stat_buf);
				size = stat_buf.st_size;
				if(size==0)
				{
					printf("The file is non-existing or empty :(. Press 3 to see list\n");
					break;
				}		
				while ( (n = read(sockfd, buf, BUFSIZE-1)) > 0)
				{
					buf[n] = '\0';
					printf("%s\n",buf);	
					write(fd,buf,n);
					if( n < BUFSIZE-2)
						break;
				}
				printf("file receiving completed, find your file in folder\n");
				close(fd);
				break;

			case PUT:
							
				printf("File name : ");				
				scanf("%s",fname);
				
				fd=open(fname,O_RDONLY,S_IRUSR);
				fstat(fd, &stat_buf);
				size = stat_buf.st_size;
				if(size==0)
				{
					printf("The file is non-existing or empty :(\n");
					break;
				}
						
				send(sockfd,&cmd, sizeof(cmd) ,0);	//sizeof integer 4
				send(sockfd,fname,sizeof(fname),0);
				//printf("%s",fname);
					
				printf("sending file %s : size is %d\n", fname,size); 

				printf("\nopened file\n");
				while ( (n = read(fd, buf, BUFSIZE-1)) > 0) 
				{
					buf[n] = '\0';
					printf("%s\n",buf);
					write(sockfd,buf,n);
				}
				printf("\nfile transfer completed \n");
				close(fd);		
				break;

			case LIST:
				
				send(sockfd,&cmd,sizeof(cmd),0);
				printf("following is the list of files and directories on server\n");
				while ( (n = read(sockfd, buf, BUFSIZE-1)) > 0)
				{
					buf[n] = '\0';
					printf("%s\n",buf);	
					if( n < BUFSIZE-2)
						break;
				}
				break;
		
			case EXIT:
				send(sockfd,&cmd,sizeof(cmd),0);
				active = 0;
				printf("Thanks for using FTP service, visit again :)\n");
				break;
				
			default:
				printf("Please enter allowed FTP commands :)\n");
		}//switch end

	}//while end
	close(sockfd);
	
	exit(0);
}
