#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

enum ERROR{
	NOERROR,
	FILEOPENERROR,
	DUPERROR,
	CHDIRERROR,
	FORKERROR
};

int mycd(char** arg)
{
	if(chdir(arg[1])==0)
		return 1;

	perror("Directory couldnot be opened!\n");
	exit(-CHDIRERROR);
}

int myquit(char** arg)
{
	return 0;
}

int myexit(char** arg)
{
	return 0;
}

//functions that are handled by parent.len is the length of the array
int (*myfunc[])(char**)={&mycd,&myquit,&myexit};
char* myfn[]={"cd","quit","exit"};
int len=3;

//for input and output redirection
char* output_file;
char* input_file;

int get_input(char** arg,int* background,int* input_redirection,int* output_redirection)
{
	char* buff=(char*)calloc(100,sizeof(char));

	int i=0;
	char c=getchar();

	while(c==' ')
		c=getchar();

	while(c!='\n')
	{
		scanf("%s",buff);

		//to handle only \n input we are taking a getchar at the beginning.That charecter is to be put
		//in buff if it was not a \n.
		if(!i)
		{
			int j=strlen(buff);

			for( ;j>=0;j--)
				buff[j+1]=buff[j];

			buff[0]=c;
		}

		//If a filename/command with space
		if(buff[0]=='\"')
		{

		}

		if(!strcmp(buff,"&"))
		{
			*background=1;		
		}
		else if(!strcmp(buff,"<"))
		{
			*input_redirection=1;
		}
		else if(!strcmp(buff,">"))
		{
			*output_redirection=1;
		}
		else
		{
			if(*output_redirection)
			{
				output_file=(char*)calloc(strlen(buff),sizeof(char));
				strcpy(output_file,buff);
			}
			else if(*input_redirection)
			{
				input_file=(char*)calloc(strlen(buff),sizeof(char));
				strcpy(input_file,buff);
			}
			else
			{

				arg[i]=(char*)calloc(strlen(buff),sizeof(char));

				strcpy(arg[i],buff);

				i++;
			}
			
		}

		c=getchar();
		
	}

	arg[i]=NULL;

	return i;

}



int create_child(char** arg,int background,int input_redirection,int output_redirection)
{

	pid_t pid=vfork();	

	if(pid==0)
	{
		int fdin,fdout;	

		if(input_redirection)
		{
			fdin=open(input_file,O_RDONLY);

			if(fdin<0)
			{
				perror("Input file not opened!!\n");
				exit(-FILEOPENERROR);
			}
			else
			{
				if(dup2(fdin,0)<0)
				{
					perror("Error in INput redirection!!\n");
					exit(-DUPERROR);
				}
			}
		}

		if(output_redirection)
		{
			fdout=open(output_file,O_WRONLY|O_CREAT,0777);

			if(fdout<0)
			{
				perror("Output file not opened!!\n");
				exit(-FILEOPENERROR);
			}
			else
			{

				if(dup2(fdout,1)<0)
				{
					perror("Error in output redirection!!\n");
					exit(-DUPERROR);
				}
			}
		}

		execvp(arg[0],arg);

		if(input_redirection)
			close(fdin);

		if(output_redirection)
		close(fdout);
	}
	else if(pid<0)
	{
		perror("Error encounterd!\n");
		exit(-FORKERROR);
	}
	else
	{
		int status=1;

		if(!background)
		{
			do
			{
				waitpid(pid,&status,WUNTRACED);
			}while(!WIFSIGNALED(status) && !WIFEXITED(status));
		}

		return 1;
	}

}

int execute(char** arg,int background,int input_redirection,int output_redirection)
{
	int i;

	for(i=0;i<len;i++)
	{
		if(!strcmp(arg[0],myfn[i]))
		{
			return (*myfunc[i])(arg);
		}
	}

	return create_child(arg,background,input_redirection,output_redirection);
}

main()
{	
	int ret=1,count;

	while(ret)
	{
		char * arg[100];
		int background=0,input_redirection=0,output_redirection=0;

		printf("MyShell$ ");
		count=get_input(arg,&background,&input_redirection,&output_redirection);

		if(count)
		ret=execute(arg,background,input_redirection,output_redirection);
	}
}
