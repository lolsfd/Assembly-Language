#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<direct.h>
#include<math.h>
#define NUMOP 59
#define NUMAD 6
#define NUMREG 9
#define MAXSYM 50
struct optable{/*OP code Table*/
  char name[8];
	unsigned int opcode;
	unsigned int format;
}arOpTable[NUMOP]={"ADD",0x18,3,"ADDF",0x58,3,"ADDR",0x90,2,"AND",0x40,3,
"CLEAR",0xB4,2,"COMP",0x28,3,"COMPF",0x88,3,"COMPR",0xA0,2,
"DIV",0x24,3,"DIVF",0x64,3,"DIVR",0x9C,2,"FIX", 0xC4,1,
"FLOAT",0xC0,1,"HIO",0xF4,1,"J",0x3C,3,"JEQ",0x30,3,
"JGT",0x34,3,"JLT",0x38,3,"JSUB",0x48,3,"LDA",0x00,3,
"LDB",0x68,3,"LDCH",0x50,3,"LDF",0x70,3,"LDL",0x08,3,
"LDS",0x6C,3,"LDT",0x74,3,"LDX",0x04,3,"LPS",0xD0,3,
"MUL",0x20,3,"MULF",0x60,3,"MULR",0x98,2,"NORM",0xC8,1,
"OR",0x44,3,"RD",0xD8,3,"RMO",0xAC,2,"RSUB",0x4C,3,
"SHIFTL",0xA4,2,"SHIFTR",0xA8,2,"SIO",0xF0,1,"SSK",0xEC,3,
"STA",0x0C,3,"STB",0x78,3,"STCH",0x54,3,"STF",0x80,3,
"STI",0xD4,3,"STL",0x14,3,"STS",0x7C,3,"STSW",0xE8,3,
"STT",0x84,3,"STX",0x10,3,"SUB",0x1C,3,"SUBF",0x5C,3,
"SUBR",0x94,2,"SVC",0xB0,2,"TD",0xE0,3,"TIO",0xF8,1,
"TIX",0x2C,3,"TIXR",0xB8,2,"WD",0xDC,3};
struct addtable{/*Address Table*/
	char name[8];
}arAddTable[NUMAD]={"START","END","BYTE","WORD","RESB","RESW"};
unsigned int hextoint(char *S){/*Covert HEX strings to integer*/
	char Hex[17]="0123456789ABCDEF";
	unsigned int retInteger=0,nTemp,i,j;
	strupr(S);
	for(i=0;i<strlen(S);i++){
		for(j=0,nTemp=1;j<strlen(Hex);j++){
			if(*(S+i)==*(Hex+j)){
				nTemp*=j;
				break;
			}
		}
		retInteger+=(nTemp*=(int)pow(16,strlen(S)-i-1));
	}
	return retInteger;
}
unsigned int find_comment(char *S){/*Comment*/
	if(S[0]!='.'){
		return 0;
	}else{
		return 1;
	}
}
int find_optable(const char *S){/*find Op Table*/
	int i,nRet=-1;
	for(i=0;i<NUMOP;i++){
		if(stricmp(arOpTable[i].name,S)==0){
			nRet=i;
			break;
		}
	}
	return nRet;
}
int find_addtable(const char *S){/*find Address Table*/
	int i,nRet=-1;
	for(i=0;i<NUMAD;i++){
		if(stricmp(arAddTable[i].name,S)==0){
			nRet=i;
			break;
		}
	}
	return nRet;
}
/*==========Pass Second functions==========*/
int p2_find_symtable(FILE *SYMTABLE,const char *S){
	unsigned int hextoint(char *S);
	int nRet=-1;
	char line[64],name[32],address[32],SEP[]="	\n";
	rewind(SYMTABLE);
	while(!feof(SYMTABLE)){
		fgets(line,64,SYMTABLE);
		strcpy(name,strtok(line,SEP));
		strcpy(address,strtok(NULL,SEP));
		if(stricmp(name,S)==0){
			nRet=hextoint(address);
			break;
		}
	}
	return nRet;
}
char *stringtoascii(const char *S1,char *S2){
	int i,j,k;
	char temp[32],tempS1[32],tokenS1[16],SEP[]="\'";
	if(S1[0]=='C'||S1[0]=='c'){
		strcpy(tempS1,S1);
		strcpy(tokenS1,strtok(tempS1,SEP));
		strcpy(tokenS1,strtok(NULL,SEP));
		j=(int)strlen(tokenS1);
		for(i=0;i<j;i++){
			k=toascii(tokenS1[i]);
			itoa(k,temp,16);
			if(i==0){
				strupr(strcpy(S2,temp));
			}else{
				strupr(strcat(S2,temp));
			}
		}
	}else if(S1[0]=='X'||S1[0]=='x'){
		strcpy(tempS1,S1);
		strcpy(tokenS1,strtok(tempS1,SEP));
		strcpy(tokenS1,strtok(NULL,SEP));
		strupr(strcpy(S2,tokenS1));
	}else{
		strcpy(tempS1,strupr(itoa(atoi(S1),temp,16)));
		j=6-(int)strlen(tempS1);
		if(j>0){
			strcpy(S2,"0");
			for(i=1;i<j;i++){
				strcat(S2,"0");
			}
			strcat(S2,tempS1);
		}
	}
	return S2;
}
unsigned int funPassSecond(FILE *INTERMEDIATE,FILE *SYMTABLE,FILE *LISTFILE,FILE *OBJ){
	unsigned int retOPCODE=0,obj_counter=0,obj_length=0,sic_length=0;
	/*Fixed in May 7, 2009*/
	/*retfunSEC change to INT from UNSIGNED INT*/
	int retfunSEC=0;
	unsigned int sic_start=0,nSymCounter=0,Starting_Address=0,PC=0,flagRES=0,objLineRESET=0;
	/*Fixed in May 1, 2009*/
	/*retSYMTABLE and regX has been INT from UNSIGNED INT*/
	int retSYMTABLE=0,regX=0;
	char *ptrToken;
	char objCODE[32],objOPCODE[16],objOPERAND[16];
	char LINE[128],tempLINE[128],SEP[]="	\n";
	char ADDRESS[32],LABEL[32],OPCODE[32],OPERAND[32],X_OPERAND[32];
	char objLINE[128],objTEXT[128],temp_32[32];
	/*=====find sic length value=====*/
	while(!feof(INTERMEDIATE)){
		fgets(LINE,128,INTERMEDIATE);
		if(LINE[0]=='!'){
			break;
		}
	}
	strcpy(tempLINE,strtok(LINE," \n"));/*space*/
	strcpy(tempLINE,strtok(NULL," \n"));
	strcpy(tempLINE,strtok(NULL," \n"));
	sic_length=hextoint(tempLINE);
	rewind(INTERMEDIATE);
	/*read first input line(from intermediate file)*/
	fgets(LINE,128,INTERMEDIATE);
	strcpy(tempLINE,LINE);
	strcpy(ADDRESS,strtok(tempLINE,SEP));
	sic_start=hextoint(ADDRESS);
	strcpy(LABEL,strtok(NULL,SEP));
	strcpy(OPCODE,strtok(NULL,SEP));
	strcpy(OPERAND,strtok(NULL,SEP));
	if(stricmp(OPCODE,"START")==0){/*OPCODE='START'*/
		/*write listing line*/
		fputs(LINE,LISTFILE);
		/*read next input line*/
		fgets(LINE,128,INTERMEDIATE);
		strcpy(tempLINE,LINE);
	}/*if START*/
	/*write Header record to object program*/
	strcpy(objLINE,"H");
	strcat(objLINE,LABEL);
	strcat(objLINE,"  ");/*2 space*/
	strcat(objLINE,"00");/*2 zero*/
	strcat(objLINE,ADDRESS);
	strcat(objLINE,"00");/*2 zero*/
	strcat(objLINE,strupr(itoa(sic_length,temp_32,16)));
	strcat(objLINE,"\n");
	fputs(objLINE,OBJ);
	/*=====read next input line continue=====*/
	strcpy(ADDRESS,strtok(tempLINE,SEP));
	strcpy(LABEL,strtok(NULL,SEP));
	strcpy(OPCODE,strtok(NULL,SEP));
	strcpy(OPERAND,strtok(NULL,SEP));
	/*initialize first Text record*/
	strcpy(objLINE,"T00");
	strcat(objLINE,ADDRESS);
	Starting_Address=hextoint(ADDRESS);
	while(stricmp(OPCODE,"END")!=0){/*OPCODE!='END'*/
		if(find_comment(LINE)!=1){/*this is not a comment line*/
			/*search OPTAB for OPCODE*/
			if(stricmp(OPCODE,"LDX")==0){
				regX=1;
			}
			retOPCODE=find_optable(OPCODE);
			if(retOPCODE!=-1){/*found*/
				strcpy(objOPCODE,strupr(itoa(arOpTable[retOPCODE].opcode,temp_32,16)));
				if(hextoint(objOPCODE)<16){
					strcpy(temp_32,"0");
					strcat(temp_32,objOPCODE);
					strcpy(objOPCODE,temp_32);
				}
				if(stricmp(OPERAND,"	")!=0){/*there is a symbol in OPERAND field*/
					/*search SYMTAB for OPERAND*/
					if(strstr(OPERAND,",X")!=NULL){
						strcpy(X_OPERAND,OPERAND);
						ptrToken=strtok(X_OPERAND,",");
						retSYMTABLE=p2_find_symtable(SYMTABLE,ptrToken);
						if(regX==1){
							regX=retSYMTABLE;
							retSYMTABLE=32768|regX;
							regX=0;
						}
					}else{
						retSYMTABLE=p2_find_symtable(SYMTABLE,OPERAND);
					}
					if(retSYMTABLE!=-1){/*found*/
						/*store symbol value as operand address*/
						strcpy(objOPERAND,strupr(itoa(retSYMTABLE,temp_32,16)));
					}else{
						/*store 0 as operand address*/
						strcpy(objOPERAND,"000000");
						/*set error flag(undefined symbol)*/
						retfunSEC=-1;
					}
				}else{
					/*store 0 as operand address*/
					strcpy(objOPERAND,"0000");
				}
				/*assemble the object code instruction*/
				strcpy(objCODE,objOPCODE);
				strcat(objCODE,objOPERAND);
			}else if(find_addtable(OPCODE)==2||find_addtable(OPCODE)==3){/*OPCODE='BYTE'or'WORD'*/
				/*covert constant to object code*/
				stringtoascii(OPERAND,objCODE);
			}
			if(find_addtable(OPCODE)==4||find_addtable(OPCODE)==5){
				flagRES++;
				objLineRESET=1;
			}else{
				flagRES=0;
			}
			if(obj_counter==10||flagRES==1){/*object code will not fit into the current Text record*/
				/*write Text record to object program*/
				PC=hextoint(ADDRESS);
				obj_length=PC-Starting_Address;
				if(obj_length<16){
					strcat(objLINE,"0");
				}
				strcat(objLINE,strupr(itoa(obj_length,temp_32,16)));
				strcat(objLINE,objTEXT);
				strcat(objLINE,"\n");
				fputs(objLINE,OBJ);
				/*initialize new Text record*/
				obj_counter=0;
				strcpy(objLINE,"T00");
				strcat(objLINE,ADDRESS);
				Starting_Address=hextoint(ADDRESS);
			}
			if(flagRES==0&&objLineRESET==1){
				strcpy(objLINE,"T00");
				strcat(objLINE,ADDRESS);
				Starting_Address=hextoint(ADDRESS);
				objLineRESET=0;
			}
			/*add object code to Text record*/
			if(find_addtable(OPCODE)==4||find_addtable(OPCODE)==5){
				/*nothing*/
			}else{
				if(obj_counter==0){
					strcpy(objTEXT,objCODE);
				}else{
					strcat(objTEXT,objCODE);
				}
				obj_counter++;
			}
		}/*if not comment*/
		/*write listing line*/
		if(find_addtable(OPCODE)==4||find_addtable(OPCODE)==5){
			/*nothing*/
		}else{
			if(find_comment(LINE)!=1){
				LINE[strlen(LINE)-1]='\0';
				if(stricmp(objCODE,"	")!=0){
					if(stricmp(OPERAND,"	")==0){
						strcat(LINE,"			");
					}else if(strstr(OPERAND,",X")!=NULL){
						strcat(LINE,"	");
					}else{
						strcat(LINE,"		");
					}
					strcat(LINE,objCODE);
					strcat(LINE,"\n");
				}
			}
		}
		fputs(LINE,LISTFILE);
		/*read next input line*/
		fgets(LINE,128,INTERMEDIATE);
		strcpy(tempLINE,LINE);
		if(find_comment(LINE)!=1){
			strcpy(ADDRESS,strtok(tempLINE,SEP));
			if(stricmp(ADDRESS,"END")==0){
				strcpy(OPCODE,ADDRESS);
				strcpy(OPERAND,strtok(NULL,SEP));
				strcpy(ADDRESS,"	");
				strcpy(LABEL,"	");
			}else{
				strcpy(LABEL,strtok(NULL,SEP));
				ptrToken=strtok(NULL,SEP);
				if(ptrToken!=NULL){
					strcpy(OPCODE,ptrToken);
				}else{
					strcpy(OPCODE,"	");
				}
				if(find_optable(LABEL)!=-1){
					strcpy(OPERAND,OPCODE);
					strcpy(OPCODE,LABEL);
					strcpy(LABEL,"	");
				}else{
					strcpy(OPERAND,strtok(NULL,SEP));
				}
			}
		}
	}/*while not END*/
	/*write last Text record to object program*/
	if(stricmp(OPCODE,"END")==0){
		PC=sic_start+sic_length;
		obj_length=PC-Starting_Address;
		if(obj_length<16){
			strcat(objLINE,"0");
		}
		strcat(objLINE,strupr(itoa(obj_length,temp_32,16)));
		strcat(objLINE,objTEXT);
		strcat(objLINE,"\n");
		fputs(objLINE,OBJ);
	}
	/*write End record to object program*/
	strcpy(objLINE,"E");
	strcat(objLINE,"00");
	strcat(objLINE,strupr(itoa(sic_start,temp_32,16)));
	strcat(objLINE,"\n");
	fputs(objLINE,OBJ);
	/*write last listing line*/
	fputs(LINE,LISTFILE);
	return retfunSEC;
}/*Pass Second*/
/*main function*/
int main(int argc,char *argv[]){
	FILE *fIntermediate,*fSymTable,*fList,*fObj;
	unsigned int sic_length=0;
	char cwdBuffer[_MAX_PATH];
	char pathIntermediate[256],pathSymbolTable[256],pathList[256],pathObj[256];
	/*Fixed in May 1, 2009*/
	printf("(Dec 10, 2005) sic1.c Released!!\n");
	printf("(May 10, 2007) sic1.c Fixed!!\n");
	printf("(May 1, 2009) sic2.c Fixed!!\n");
	printf("Author's Website: http://tw.myblog.yahoo.com/mjshya/\n\n");
	getcwd(cwdBuffer,_MAX_PATH);
	strcpy(pathIntermediate,cwdBuffer);
	strcat(pathIntermediate,"\\intermed\\interMed.txt");
	strcpy(pathSymbolTable,cwdBuffer);
	strcat(pathSymbolTable,"\\intermed\\symTable.txt");
	strcpy(pathList,cwdBuffer);
	strcat(pathList,"\\intermed\\list.txt");
	strcpy(pathObj,cwdBuffer);
	strcat(pathObj,"\\intermed\\obj.txt");
	fIntermediate=fopen(pathIntermediate,"r");
	fSymTable=fopen(pathSymbolTable,"r");
	fList=fopen(pathList,"w");
	fObj=fopen(pathObj,"w");
	if(fIntermediate!=NULL||fSymTable!=NULL||fList!=NULL){
		if(funPassSecond(fIntermediate,fSymTable,fList,fObj)!=-1){
			fclose(fIntermediate);
			fclose(fSymTable);
			fclose(fList);
			fclose(fObj);
			printf("...Compile Completed!!\n");
			system("pause");
		}else{
			fclose(fIntermediate);
			fclose(fSymTable);
			fclose(fList);
			fclose(fObj);
			system("pause");
			exit(0);
		}
	}else{
		printf("Open Intermediate, Symbol_Table and List Files Failure!!\n");
		printf("Check Files Please!!\n");
		system("pause");
	}
	return 0;
}/*main function*/
