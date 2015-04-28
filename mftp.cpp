#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

struct ftpserver {
	//Information for threads - chunksize, namename, pass, address, etc
	public:
	int sockfd;
	struct sockaddr_in serverAddress, clientAddress;
	string file;
	string hostname;
	int port;
	string name;
	string password;
	string log;
	
	bool hostnameonfile;
	bool ispassive;
	bool isbinary;
	bool isswarming;
	bool islogging;
}; 

string HELLO = "\nXXXXXXXXXXXXXXXXXXXXXXX\n";
int qq = 0;
int flag = 0;
pthread_mutex_t mutex;
ofstream myfile;

int sender(string input);
void *handler(char *input);
void log(char* message, string servername, int x);
int passive(string, int);
void *getfilesize(void *input);
int connecttoserver();
void createftpserver(int flag);

string swarmfile;
ftpserver swarmarray[100];


//Receives message from server, passes it to handler
void *receiver(void *input) 
{
	cout << "I AM RECEIVING THIS:\n";
	char buffer[10000];
	read(swarmarray[qq].sockfd,buffer,10000);
	cout << buffer;
	
	if(swarmarray[qq].islogging == true)
		log(buffer, swarmarray[qq].log, 0);
	handler(buffer);
}

//Checks to see first 4 character of message
//Passes the correct output to sender
void *handler(char *input)
{	
	cout << "I AM HANDLING" << endl;
	string str = input;
	string msgcode = str.substr(0,3);
	string toserv;
	bool ismultiline = false;
	cout << HELLO << endl;
	for(int i = 0; i < str.length(); i++){
		if(ismultiline == false){
			if(str.compare(i, 4, "220 ") == 0){
				toserv = "USER " + swarmarray[qq].name + "\r\n";
				sender(toserv);
				break;
			}		
			if(str.compare(i, 4, "331 ") == 0){
				toserv = "PASS " + swarmarray[qq].password + "\r\n";
				sender(toserv);
				break;
			}
			if(str.compare(i, 4, "230 ") == 0){
			
				if(swarmarray[qq].isbinary == true)
					toserv = "TYPE I\r\n";
				else
					toserv = "TYPE A\r\n";
				
				sender(toserv);
				break;
			
			}			
			if(str.compare(i, 4, "200 ") == 0){				
				if(swarmarray[qq].ispassive == true)
					toserv = "PASV\r\n";
				else
					toserv = "PORT\r\n";
				sender(toserv);
				break;
			}
			if(str.compare(i, 4, "227 ") == 0){
				qq++;
				char c[100];
				strcpy(c, str.c_str());
				char *tok;
				tok = strtok(c, " .,()");
				int count = 0;
				int num1, num2;
				
				//Remove excess information, isolate IP hostname and port
				while(tok != NULL)
				{
					if(count >= 4 && count <= 7)
					{
						string temp = tok;
						(swarmarray[qq].hostname).append(temp);
						(swarmarray[qq].hostname).append(".");
						
					}
					
					if(count == 8)
						num1 = atoi(tok);
					if(count == 9)
						num2 = atoi(tok);
					
					tok = strtok(NULL, " .,()\n");
					count++;
				}
				
				(swarmarray[qq].hostname).erase((swarmarray[qq].hostname).end()-1);
				
				cout << "THIS IS DATA CHANNEL: " << endl;
				cout << swarmarray[qq].hostname << endl;
				swarmarray[qq].port = (num1*256)+num2;
				cout << swarmarray[qq].port << endl << endl;
				
				cout << "THIS IS CONNECTION CHANNEL: " << endl;
				cout << swarmarray[qq-1].hostname << endl;
				cout << swarmarray[qq-1].port << endl << endl;				
				
				qq--;
				toserv = "LIST\r\n";
				sender(toserv);
				
				//getfilesize(NULL);
			}
			
			if(i == str.length()-2){
				ismultiline = true;
			}
		}
		
		else{
			i = 0;
			if(str.compare(i, 4, msgcode + "-") == 0){
				receiver(NULL);
			}
		}
	}
}

//Sends message to server, calls on receiver(NULL) to read next message
int sender(string output) 
{
	cout << "I AM SENDING THIS:" << endl;	
	char buffer[1000];
	bzero(buffer, 1000);
	strcpy(buffer, output.c_str());
	write(swarmarray[qq].sockfd, buffer, strlen(buffer));
	cout << buffer << endl;	
	if(swarmarray[qq].islogging == true)
		log(buffer, swarmarray[qq].log, 1);
	
	if(output == "LIST\r\n")
	{
		getfilesize(NULL);
	}
	else
		receiver(NULL);
}


void *getfilesize(void *input) 
{
	//Iterate to next ftpserver in ftpserver array
	qq++;
	createftpserver(1);
	connecttoserver();
}

void *getfile(void *input) 
{

}

char *getactive(int aport) 
{
	//create char-array for PORT command
}

void *getswarmservers() 
{
	string input;
	string line;
	string tempuser, temppass, temphostname, tempfile;
	int i = 0;
	
	unsigned found;
	char c[1000];
	fstream file(swarmfile.c_str(), ios::in);
	if(file)
	{
		while(file.good())
		{
			getline(file, line);
			line.erase(0, 6);
			cout << line << endl;
			unsigned found = line.find_first_of(":");
			int index1 = found;
			found = line.find_first_of("@");
			int index2 = found;
			found = line.find_first_of("/");
			int index3 = found;
			
			string substr1 = line.substr(0,index1);
			string substr2 = line.substr(index1+1,index2-index1-1);
			string substr3 = line.substr(index2+1,index3-index2-1);
			string substr4 = line.substr(index3,line.length()-index3-1);
			
			swarmarray[i].name = substr1;
			swarmarray[i].password = substr2;
			swarmarray[i].hostname = substr3;
			swarmarray[i].file = substr4;
			cout << endl;			
			i++;
		}
		file.close();
	}
	
	else
		cout << "UNABLE TO OPEN FILE";
	

}
int active(int aport) 
{
	//accept connection (active)
	//create socket
	//bind
	//listen
	//accept
	//return fd for new socket
}

void createftpserver(int flag)
{
	if(flag == 0)
	{
		swarmarray[qq].port = 21;
		swarmarray[qq].name = "anonymous";
		swarmarray[qq].password = "name@localhost.localnet";
		swarmarray[qq].file = "robots.txt";
		swarmarray[qq].hostnameonfile = false;
		swarmarray[qq].ispassive = true;
		swarmarray[qq].isbinary = true;
		swarmarray[qq].isswarming = false;
		swarmarray[qq].islogging = false;
	}
	else
	{
		swarmarray[qq].hostnameonfile = false;
		swarmarray[qq].ispassive = true;
		swarmarray[qq].isbinary = true;
		swarmarray[qq].isswarming = false;
		swarmarray[qq].islogging = false;
	}
}
int connecttoserver()
{
	cout << "I AM CONNECTING TO: " << endl;
	cout << "HOST: " << swarmarray[qq].hostname << endl;
	cout << "PORT: " << swarmarray[qq].port << "\n\n";
	const char *c = "ftp.ucsb.edu";
	struct hostent *serv = gethostbyname(c);
	
	//Create socket
	swarmarray[qq].sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(swarmarray[qq].sockfd < 0)
	{
		printf("Error opening socket.");
		exit(0);
	}
	//Get IP	
	if(serv == NULL)
	{
		fprintf(stderr, "Error, no such host\n");
		exit(0);
	}
	
	bzero((char*) &swarmarray[qq].serverAddress, sizeof(swarmarray[qq].serverAddress));
	swarmarray[qq].serverAddress.sin_family = AF_INET;
	bcopy((char*)serv->h_addr, (char*)&swarmarray[qq].serverAddress.sin_addr.s_addr, serv->h_length);
	swarmarray[qq].serverAddress.sin_port = htons(swarmarray[qq].port);
	
	//Connect to server
	if(connect(swarmarray[qq].sockfd,(struct sockaddr*) &swarmarray[qq].serverAddress,sizeof(swarmarray[qq].serverAddress)) < 0)
	{
		printf("Error connecting to server");
	}
	
	return swarmarray[qq].sockfd;
}

//Function to log the file
void log(char* message, string logf, int x) 
{
	pthread_mutex_unlock(&mutex);
	char *c;
	strcpy(c, logf.c_str());
	ofstream myfile;
	myfile.open(c, ios::app);
	
	
	
	if(x == 0)
		myfile << "S->C: " << message;
	else
		myfile << "C->S: " << message;
	myfile.close();
	
	pthread_mutex_lock(&mutex);
}

struct ftpserver getinput(const char* argv[], int argc) 
{
	//deal with flags & fill server struct
	int i;
	for(i = 1; i < argc; i++)
	{
		//-h or --help
		if((strcmp(argv[i], "-h") == 0) || (argv[1] == NULL))
		{
			cout << "This program connects to the server specified by -s server." << endl;
			cout << "Usage is as follows: " << endl;
			cout << "-h or --help: prints synopsis of files" << endl;
			cout << "-v or --version: prints application name, version number, author" << endl;
			cout << "-f file or --file file: specifies file to download from ftp server" << endl;
			cout << "-s hostname or --hostname: specifies server to download from" << endl;
			cout << "-p port or --port port: specifies the port to be used (default = 21)" << endl;
			cout << "-n user or --username user: specifies username (default = anonymous)" << endl;
			cout << "-P pass or --password pass: specifies password (default = user@localhost.localnet)" << endl;
			cout << "-a or --active: forces active behavior from server (default = passive)" << endl;
			cout << "-m mode or --mode mode: specifies mode (ASCII or binary) (default = binary)" << endl;
			cout << "-l logfile or --log logfile: logs conversation of server and client into logfile.txt" << endl;
			exit(0);
		}
		//-v or --version
		if(strcmp(argv[i], "-v") == 0)
		{
			printf("Application name: mftp\nVersion number: 0.1\nAuthor: Justin Liang\n");
			exit(0);
		}
		//-f file or --file file
		if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmarray[qq].file = argv[i+1];
			}
		}
		//-s hostname or --server hostname
		if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmarray[qq].hostname = argv[i+1];
			}
		}
		//-p port or --port port
		if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmarray[qq].port = atoi(argv[i+1]);
			}
		}
		//-n name or --namename name
		if(strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--namename") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmarray[qq].name = argv[i+1];
			}
		}
		//-P password or --password password
		if(strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--password") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmarray[qq].password = argv[i+1];
			}
		}
		//-a or --active
		if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--active") == 0)
		{
			cout << "SERVER IS SET TO ACTIVE" << endl;
			swarmarray[qq].ispassive = false;			
		}
		//-m mode or --mode mode
		if(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mode") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				if(strcmp(argv[i+1], "ascii") == 0)
					swarmarray[qq].isbinary = false;
				
			}
		}
		//-l logfile or --log logfile
		if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmarray[qq].log = argv[i+1];
				swarmarray[qq].islogging = true;
			}
		}
		//-w swarm-config file or --swarm swarm-config-file
		if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--swarm") == 0)
		{
			if(argv[i+1] != NULL && argv[i+1][0] != '-')
			{
				swarmfile = argv[i+1];				
				getswarmservers();
			}
		}
	}
	
	if(swarmarray[qq].ispassive == true)
	{
		connecttoserver();
		receiver(NULL);
	}
	return swarmarray[qq];
}

//Main function
int main (int argc, const char * argv[]) 
{
	createftpserver(0);
	getinput(argv, argc);
}
