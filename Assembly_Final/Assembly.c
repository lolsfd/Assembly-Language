
#include<stdio.h>
#include<stdlib.h>
#include <math.h>
#include<string.h>
typedef struct listNode *listPointer;
typedef struct listNode{
	char data[5];
	char label[10];
	char code[10];
	char operd[10];
	int offloc;
	listPointer next;
}listNode;
listPointer first=NULL;
listPointer head=NULL;
int offloc;
void decTohex(int num,char hex[])
{
	char arr[]="0123456789ABCDEF";

	int i=3;
	while(num)
	{
	 	hex[i--]=arr[num%16];
	 	num/=16;
	}

}
void create(char str[],char num[])    //opcode
{
	listPointer node=(listPointer)malloc(sizeof(listNode));
	strcpy(node->data,num);
	strcpy(node->label,str);
	node->next=NULL;
	if(first==NULL)
	{
		first=node;
		return;
	}
	else
	{
		listPointer last=first;
		while(last->next!=NULL)
		{
			last=last->next;
		}
		last->next=node;
	}
}

void create_label(char lab[],char cd[],char opd[])
{
	listPointer node=(listPointer)malloc(sizeof(listNode));
	strcpy(node->label,lab);
	strcpy(node->code,cd);
	strcpy(node->operd,opd);
	node->next=NULL;
	if(head==NULL)
	{
		head=node;
		return;
	}
	listPointer temp=head;
	while(temp->next!=NULL)
	{
		temp=temp->next;
	}
	temp->next=node;
}

void show(listPointer head,FILE* A)
{
	listPointer node=head;
	char str[10];
	if(head==NULL)
	{
		printf("No Data");
		return;
	}
	else
	{
		while(node!=NULL)
		{
			if(strcmp(node->code,"END")==0)
			{
				fprintf(A,"\t");
				fprintf(A,"%s	",node->label);
				fprintf(A,"%s	",node->code);
				fprintf(A,"%s\n",node->operd);
				break;
			}
			decTohex(node->offloc,str);
			fprintf(A,"%s	",str);
			fprintf(A,"%s	",node->label);
			fprintf(A,"%s	",node->code);
			fprintf(A,"%s\n",node->operd);
			node=node->next;
		}
	}
}
void showtable(listPointer head,FILE* A)
{
	listPointer node=head;
	if(head==NULL)
	{
		printf("No Data");
		return;
	}
	char str[10];
	while(node!=NULL)
	{
		if(strlen(node->label)!=0)
		{
			fprintf(A,"%s	",node->label);
			decTohex(node->offloc,str);
			fprintf(A,"%s\n",str);	
		}
		node=node->next;
	}
} 
int check(char label[])
{
	listPointer temp;
	temp=first;
	while(temp!=NULL)
	{
		if(strcmp(temp->label,label)==0)
		{
			return 0;		//find
		}
		temp=temp->next;
	}
	return 1;		//not found
}
int check_op(char label[])
{
	listPointer temp;
	temp=first;
	while(temp!=NULL)
	{
		if(strcmp(temp->code,label)==0)
		{
			return 0;		//find
		}
		temp=temp->next;
	}
	return 1;		//not found
}
void clear(char arr[])
{
	memset(arr,0,strlen(arr));
}

int hextodec(char num[]){
    int index=0,i,temp,t=0;
    for(i=strlen(num)-1;i>=0;i--){
        switch(num[i]){
            case 'A': temp=10; break;
            case 'B': temp=11; break;
            case 'C': temp=12; break;
            case 'D': temp=13; break;
            case 'E': temp=14; break;
            case 'F': temp=15; break;
            default :temp=num[i]-'0'; break;
        }
        index+=pow(16,t)*temp;
        t++;
    }
    return index;
}
void cal(listPointer head)
{
	listPointer temp=head;
	char floc[5];
	sprintf(floc, "%d", offloc);
	int cut=hextodec(floc);
	temp->offloc=cut;
	while(temp!=NULL)
	{
		if(check(temp->code)==0)
		{
			cut+=3;
			temp->next->offloc=cut;	
		}
		else if(strcmp(temp->code,"RESB")==0)
		{
			cut+=atoi(temp->operd);
			temp->next->offloc=cut;	
		}
		else if(strcmp(temp->code,"RESW")==0)
		{
			cut+=3*atoi(temp->operd);
			temp->next->offloc=cut;	
		}
		else if(strcmp(temp->code,"BYTE")==0)
		{
			if(temp->operd[0]=='C')	
			{
				cut+=strlen(temp->operd)-3;
				temp->next->offloc=cut;	
			}
			else if(temp->operd[0]=='X')
			{
				cut+=(strlen(temp->operd)-3)/2;	
				temp->next->offloc=cut;	
			}
		}
		else if(strcmp(temp->code,"WORD")==0)
		{
			cut+=3;
			temp->next->offloc=cut;	
		}
		temp=temp->next;
	}
}
int main() {
	FILE *fptr;
	FILE *fptr2;
	FILE *fptr3;
	FILE *opcode;
	char str[100],label[10];
	char op[10],loc[3];
	int hex;
	int flag;
	char offset[10];
	char a[10];
	char b[10];
	char ll[10];
	fptr=fopen("source.txt","r");
	fptr2= fopen("Intermediate file.txt","w+");
	opcode=fopen("opcode.txt","r");
	fptr3= fopen("Symboltable.txt","w+");
	listPointer node=(listPointer)malloc(sizeof(listNode));
	if(opcode!=NULL)                  //讀取opcode 
	{
		while(fscanf(opcode,"%s %s",&op,&loc) != EOF)
		{
			create(op,loc);
		}
	}
	if(fptr!=NULL)					//讀取source.txt 
	{
		char buffer[15];
		char label[10];
		char op[10];
		char loc[5];
		int i,cnt,precnt,flag;
		fscanf(fptr,"%s\t%s\t%s ",label,op,loc);
		if(strcmp(op,"START")==0)  //讀取第一行 
		{
			offloc=atoi(loc);
			strcpy(a,label);
			strcpy(b,op);
			strcpy(ll,loc);
		}
		while(fscanf(fptr,"%[^\n] ",str) !=EOF){
			flag=0;
			for(i=0;i<strlen(str);i++)
			{
				if(str[i]=='\t')
				{
					flag++;	
				}
			}
			for(i=0,cnt=0;i<strlen(str);i++)
			{
				if(str[i]=='\t')
				{
					cnt++;
					if(cnt==1)
					{	
						clear(op);
						clear(label);
						strncpy(label,str,i);
						strncpy(op,str,i);
						if(check(label)==1&&strcmp(label,"END")!=0) //column1
						{
						}
						else  ////column2
						{
							clear(label);
							//printf("	%s",op);
						}
						precnt=i;	
					}
					if(cnt==1&&str[i+1]!=' '&&flag==1)   //column3
					{
						clear(buffer);
						strncpy(buffer,str+i+1,strlen(str));   //str 
					}
					if(cnt==2) //column2
					{
						clear(op);
						strncpy(op,str+precnt+1,i-precnt-1);
					}
					if(cnt==2&&str[i+1]!=' ')   //column3
					{
						clear(buffer);
						strncpy(buffer,str+i+1,strlen(str)-i);
					}
				}
				//clear(buffer);
			}
			if(cnt==0)               //column2
			{
				clear(op);
				clear(buffer);
				clear(label);
				strcpy(op,str);
			}
			create_label(label,op,buffer);
		}
	}
	cal(head);
	show(head,fptr2) ;
	showtable(head,fptr3);
	fclose(fptr);
	fclose(fptr2);
	free(first);
	free(head);
	return 0;
}

