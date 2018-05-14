#include <iostream>
#include <iomanip>
#include <fstream>
#include <string> 
#define MAX_NODE_NUMBER 600
using namespace std;  

struct{
	int appearance;//number of times the node appears
	int left, right;//left child and right child (index)
}nodes[MAX_NODE_NUMBER];

struct{//every character has a unique code
	int length;
	int bitcode[256];
}CodeTable[256];//one character=one byte=eight bit=2^8 combinations=256 combinations

//when building the tree to record if it has been chosen or not
bool mark[MAX_NODE_NUMBER];

int path[256],bit_count=0;
//create a huffman code table
void trace(int root_index){
	if(nodes[root_index].left==-1 && nodes[root_index].right==-1){			

		CodeTable[root_index].length = bit_count; 
		for(int i=0;i<bit_count;i++){
			CodeTable[root_index].bitcode[i]=path[i];
		}	
//		cout<<(char)root_index<<":";
//		for(int i=0;i<CodeTable[root_index].length;i++){
//			cout<<CodeTable[root_index].bitcode[i];
//		}
//		cout<<endl;	
	}
	else{
		if(nodes[root_index].left!=-1){
			path[bit_count]=0;
			bit_count++;
			trace(nodes[root_index].left);
			bit_count--;
		}
		if(nodes[root_index].right!=-1){
			path[bit_count]=1;
			bit_count++;
			trace(nodes[root_index].right);
			bit_count--;
		}
	}
}

int main(int argc,char ** argv){
	
	string choice;
	int choose;
	getline(cin,choice);
	if(choice=="Compress"){
		choose=1;
	}
	if(choice=="Extract"){
		choose=2;
	}
	if(choice=="quit"){
		choose=3;
	}
	
	string input_filename;
	string output_filename;	
	
	while(choose==1 || choose==2 || choose==3){
		if(choose==1){//compress
			cin.clear();
			cin.sync();						
			cout<<"Input filename"<<endl;
			getline(cin,input_filename);
			cin.clear();
			cin.sync();				
			cout<<"Onput filename"<<endl;
			getline(cin,output_filename);
					
			//compress--
		    FILE *file = fopen(input_filename.c_str(), "rb");//read file
			if(!file){
				cerr<<"file not exist"<<endl;
				return 1;
			}
			//reset all nodes to zero
			for(int i=0; i<MAX_NODE_NUMBER; i++)
			{
				nodes[i].appearance=0;
				nodes[i].left=-1;
				nodes[i].right=-1;
			}
			//calculate the number of appearances
			char temp;
			while(!feof(file)){ 
		        if(fread(&temp,sizeof(char),1,file)){
		        	//store into array
					nodes[(unsigned char)temp].appearance++;
				}
			}
			//initialize all mark to false
			for(int i=0;i<MAX_NODE_NUMBER;i++){
				mark[i]=false;
			}
			//scan the nodes-array and find the two smallest number
			int size=256;
			while(1){
				int first,second,firstIndex,secondIndex;
				int i,j;
				first=2000000000;
				for(i=0;i<size;i++){
					if(nodes[i].appearance<first && nodes[i].appearance>0 && mark[i]==false){
						first=nodes[i].appearance;
						firstIndex=i;
					}
				}
				mark[firstIndex]=true;
				second=2000000000;
				for(j=0;j<size;j++){
					if(nodes[j].appearance<second && nodes[j].appearance>0 && mark[j]==false){
						second=nodes[j].appearance;
						secondIndex=j;
					}
				}
				mark[secondIndex]=true;	
				
				nodes[size].appearance=first+second;
				nodes[size].left=firstIndex;
				nodes[size].right=secondIndex;
				size++;
				
				//if all nodes are in one tree then break the while loop
				int nodes_count=0;
				int mark_count=0;
				for(int a=0;a<MAX_NODE_NUMBER;a++){
					if(nodes[a].appearance!=0){
						nodes_count++;
					}
				}
				for(int b=0;b<MAX_NODE_NUMBER;b++){
					if(mark[b]==true){
						mark_count++;
					}
				}
				//finish building binary-tree
				if(nodes_count==(mark_count+1)){
					break;
				} 
			}
			
			bit_count=0;
			int root_index;
			//root一定會在array的最後面 
			for(int i=0;i<size;i++){
				if(nodes[i].appearance!=0)
					root_index=i;
			}
			
			//store the root index in the tail of the array
			//因為後面的array不會用到 
			nodes[MAX_NODE_NUMBER-1].appearance=root_index;
			trace(root_index);//create a huffman-code table
			fclose(file);
			
			FILE *file_1 = fopen(input_filename.c_str(),"rb");//read file again
			char c;
			int ASCII_c;
			int current_number_of_bit=0;
			int valid_number_of_bit=0;//to aware of the last byte
			char write;
			ofstream out_1(output_filename.c_str(), ios::binary);//write file	
			//write valid-number-of-bit to the file
			//just reserve a space for it , when get the value in the back, write it again
			out_1.write((char*)&valid_number_of_bit, sizeof(valid_number_of_bit));
			//write huffman-tree to the file
			out_1.write((char*)nodes,sizeof(nodes));
			//then write the compressed content to the file 
			while(!feof(file_1)){//when read , write at the same time
		        if(fread(&c,sizeof(char),1,file_1)){//read one byte at a time  
					ASCII_c=(unsigned char)c;	
					//	cout<<ASCII_c<<" "<<c<<endl;
					//	cout<<c<<" ";
					//	for(int i=0;i<CodeTable[ASCII_c].length;i++){
					//		cout<<CodeTable[ASCII_c].bitcode[i];
					//	}
					//	cout<<endl;
					for(int i=0;i<CodeTable[ASCII_c].length;i++){
						if(current_number_of_bit<8){			
							write=write<<1;//shift write to left	
							write|=CodeTable[ASCII_c].bitcode[i];//then "or" the new bit
							current_number_of_bit++;	
						}
					if(current_number_of_bit==8){//write a byte to the file
						out_1<<write;
						current_number_of_bit=0;
						}
					}		
				}
			}
			if(current_number_of_bit!=0){
				valid_number_of_bit=current_number_of_bit;
				out_1<<write;
			}
			//back to the beginning and write the correct value of valid-number-of-bit
			out_1.seekp(0, ios::beg);
			out_1.write((char*)&valid_number_of_bit, sizeof(valid_number_of_bit));
			out_1.close();
			fclose(file_1);
			//compress--	
			/*
			unsigned char ch;
			ifstream fileIn("output.txt", ios::binary);
			fileIn.read((char*)&valid_number_of_bit, sizeof(valid_number_of_bit));
			fileIn.read((char*)nodes,sizeof(nodes));
			cout << endl; 
			while( fileIn.read((char*)&ch, 1) )
				for(int i=0; i<8; i++) {
					cout << ((ch & 128) >> 7);
					ch = ch << 1;
				}
			cout << endl;
			fileIn.close();*/	
		}
		if(choose==2){//extract
			cin.clear();
			cin.sync();					
			cout<<"Input filename"<<endl;
			getline(cin,input_filename);
			cin.clear();
			cin.sync();					
			cout<<"Onput filename"<<endl;
			getline(cin,output_filename);	
			
			//extract--
			int valid_number_of_bit;
			FILE *file_2_length = fopen(input_filename.c_str(),"rb");
			fseek(file_2_length,0,SEEK_END);
			int data_length=ftell(file_2_length)-sizeof(nodes)-sizeof(valid_number_of_bit);
			fclose(file_2_length);
			FILE *file_2 = fopen(input_filename.c_str(),"rb");
			fread((char*)&valid_number_of_bit, sizeof(valid_number_of_bit), 1,file_2);//read valid-number-of-bit
			fread((void*)nodes, sizeof(nodes[0]), MAX_NODE_NUMBER, file_2);//read huffman-tree
			int count=0;//to identify whether we are reading the last byte or not
		
			char b;
			unsigned int ASCII_b;	
			int eightbit[8];//store 8 bits read from the file
			//write file
			ofstream out_2(output_filename.c_str(),ios::out | ios::binary);
			
			bit_count=0;
			int rootIndex=nodes[MAX_NODE_NUMBER-1].appearance;
			int position=rootIndex;
			while(!feof(file_2)){	
		        if(fread(&b,sizeof(char),1,file_2)) {  	
		        	ASCII_b=(int)b;
					for(int i=7;i>=0;i--){
						eightbit[i]=ASCII_b%2;
						ASCII_b/=2;
					}
					//traverse the tree
					if(count==data_length-1){//if last byte				
						for(int i=8-valid_number_of_bit;i<8;i++){
							if(eightbit[i]==0){//go left
								position=nodes[position].left;
							}
							if(eightbit[i]==1){//go right
								position=nodes[position].right;
							}
							//when at leaf ,write to file
							if(nodes[position].left==-1 && nodes[position].right==-1){
								out_2<<(char)position;
								//traverse from root again
								position=rootIndex;
							}					
						}
					}
					else{//if not the last byte		
						for(int i=0;i<8;i++){	
							if(eightbit[i]==0){//go left
								position=nodes[position].left;
							}
							if(eightbit[i]==1){//go right
								position=nodes[position].right;
							}
							//when at leaf ,write to file
							if(nodes[position].left==-1 && nodes[position].right==-1){
								out_2<<(char)position;
								position=rootIndex;
							}
						}				
					}
					count++;	
				}				
			}
		//extract--								
		}
		if(choose==3){//quit
			return 0;
		}
		cin.clear();
		cin.sync();					
		cout<<"enter choice"<<endl;
		cin>>choice;
		if(choice=="Compress"){
			choose=1;
		}
		if(choice=="Extract"){
			choose=2;
		}
		if(choice=="quit"){
			choose=3;
		}				
	}
}
