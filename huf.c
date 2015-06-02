#include <sys/uio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int bit_len;

typedef struct node *link;

 struct node {  //index the char, code the encode, times the freq, left right and next are pointers to an other node
	char index, *code;
	int times;
	link left, right, next;
	};

static link head,headb;

link NEW(char buffer) // NEW creates and initialize a new node, with index = buffer. Return a pointer to the node
{
	link x = malloc(sizeof *x);
  	x->index = buffer;
	x->times=1;
	x->next = NULL;
	x->code = malloc(10*sizeof(char));
	return x;
}

void search_list(char buffer) { //searching the buffer who has the filcontentsts and find the freq of any char. Returns nothing
		link t;
		t = head->next;
		while(t!=NULL){
			if ( t->index == buffer){
				t->times++;
			}
			t=t->next;
		}
}

link sort_list(link heada)//1 parameter, the head of the list to be sorted. Returns a pointer to the new head
{
 headb = malloc(sizeof *headb);
 headb->next=NULL;
 link x,u,t;
	for (t = heada->next; t != NULL; t = u){
		u = t->next;
		for (x = headb; x->next != NULL; x = x->next){
			if (x->next->times < t->times) break;
		}
		t->next = x->next; x->next = t;
	}
 free(heada);
 return headb;
}

link find_nb_tail(link heada){ //Find the node before tail and return it. heada is the head of the list we want
 link a=heada;
 while( a->next->next != NULL ){
  a=a->next;
 }
 return a;
}

void sub_list(link heada){ // heada is the head of the list. Makes the huffman tree
 link l,r,n,a; // l = left node, r = right node, n = new node, a = help us put the new node in the end
 int rt, lt; //rt = right node times, lt left node times
 //initialize the new node;
 n = malloc(sizeof *n);
 n->next=NULL;

 //find and put the two last(min) nodes in left and right of the new
 //And pop the two nodes from the list
 l = find_nb_tail(heada);
 n->left = l->next;
 lt = l->next->times;
 l->next = NULL;

 r = find_nb_tail(heada);
 n->right = r->next;
 rt = r->next->times;
 r->next = NULL;

 //add the two freq to the new node
 n->times= lt + rt;

 //put the new node in the end of the list
 if ( heada->next == NULL){
 	heada->next = n;
 } else{
	 a = find_nb_tail(heada);
	 a->next->next = n;
 }
}


int encode(link ch, char *enc, link blist){//ch the head of the tree, enc the string we want to save it, blist the list We make after We destroy the tree
 int flag;					//Return 0 to take the node out of the tree, else 1
 link tree, b;
 tree = ch;
 b = blist;
 if ( tree->right != NULL ){
	strcat(enc,"0");
	flag = encode(tree->right, enc, b);
	if (flag == 0) {
		tree->right = NULL;
	}
 } else  if ( tree->left != NULL ){
        strcat(enc,"1");
        flag = encode(tree->left, enc, b);
        if (flag == 0) {
		tree->left = NULL;
		return 0;
	}
 } else {
	strcpy(tree->code,enc);
	bit_len += (strlen(enc) * tree->times);
	b->next = tree;
	strcpy(enc,"");
	return 0;
 }
 return 1;
}

void write_cm(int fd_cm, link l){ //writing to the .cm file. fd_cm the file discriptor of the .cm file, link l the list with the freq and index
 link ch = l;
 size_t size;
 int err;

 while(ch!=NULL) {
        //write the index
        err = write(fd_cm, &ch->index, 1);
        if ( err < 0){
        	perror("I am sorry");
                exit(-1);
	}
        //write a space
        err = write(fd_cm, " ", 1);
        if ( err < 0){
 	       perror("I am sorry");
               exit(-1);
	}
        //write the endoding
        size = strlen(ch->code);
        err = write(fd_cm, ch->code, size);
        if ( err < 0){
     		perror("I am sorry");
                exit(-1);
	}
        //write the end of line
        err = write(fd_cm, "\n", 1);
        if ( err < 0){
 	       perror("I am sorry");
               exit(-1);
	}
        ch=ch->next;
 }
}

void final_cod(char *enc, link l, char *buf){//write to string encoded the final output.  enc the string where we will save the encode, l the list and
	link ch;				//buf the string with the contents of the file
	size_t size;
	size = strlen(buf) - 1;
	int i;
	for(i=0; i<size; i++){
		ch=l;
		while(ch!=NULL) {
			if ( buf[i] == ch->index ){
				strcat(enc,ch->code);
			}
			ch=ch->next;
		}
	}
}

void write_huf(char *enc, int fd_h){ //enc the string with the encoding, fd_h the file discriptor of the .huf file
	unsigned char bits;
	size_t size, b=0;
	size = strlen(enc);
	int i,j,k, err, bit[8]={0,0,0,0,0,0,0,0};

	for(i=0; i<size; i++){
		j = i % 8;
		if ( j == 0 && i != 0){
			b++;
			err = write(fd_h, &bits, 1);
			if ( err < 0 ){
				perror("I am sorry");
				exit(-1);
			}
		 	bits = 0;
			for(k=0; k<8; k++){ bit[k] = 0;}
		}
		bit[j] = (int)(enc[i] - '0');
                if (bit[j]) {bits |= (1<<(7-j));}
	}
	err = write(fd_h, &bits, 1);
	if ( err < 0 ){
		perror("I am sorry");
		exit(-1);
	}
}

void read_cm(char *buff2, link l){//read .cm file, l the list we will save the encoding
	char *name_cm;
	struct stat st;
	int err, fd;
	size_t size;
	char *token;
	const char s[2] = "\n";
	link ch=l;

	name_cm=malloc(25);
	printf("Please give the .cm\n");
	scanf("%s", name_cm);

	//See if it exists
	err = stat(name_cm, &st);
	if ( err < 0){
		printf("File doesn't exist\n");
		exit(-1);
	}

	//opening file
        fd = open(name_cm, O_RDWR);
        if ( fd < 0 ) {
                perror("Problem opening file");
                exit(-1);
        } else printf("File %s opened\n", name_cm);

	size = st.st_size;
	buff2 = malloc(size);
	//take the encoding
	err = read(fd, buff2, size);

	//split the buffer and create the list with the encoding
	token = strtok(buff2, s);
	while( token != NULL ){
		if ( token[1] == ' '){
			ch->next=NEW(token[0]);
			ch=ch->next;
			ch->code=token+2;
		} else {
			ch->next=NEW('\n');
			ch=ch->next;
			ch->code=token+1;
		}
		token = strtok(NULL, s);
	}
}

void decomp(char b, char *bit){//decompress the .huf file
        int i;
        unsigned char bits=b;

        for(i=0; i<8; i++){
                bit[i] = '0';
                if( (bits & 1<<7-i) > 0){
                        bit[i] = '1';
                }
        }
}

int open_huf(char *name_huf){//returns file descriptor
	int fd, err;
        struct stat st;

        //See if it exists
        err = stat(name_huf, &st);
        if ( err < 0){
                printf("File doesn't exist\n");
                exit(-1);
        }

        //opening file
        fd = open(name_huf, O_RDWR);
        if ( fd < 0 ) {
                perror("Problem opening file");
                exit(-1);
        } else printf("File %s opened\n", name_huf);
	return fd;
}

void read_huf(char *enc, int fd){ //read the .huf file
        int err;
	char *bit, buff;

	bit=malloc(8*sizeof(int));

	//reading the file by bit and save it to the string encoded
	while(read(fd, &buff, 1) > 0){
		decomp(buff, bit);
		strcat(enc, bit);
	}
}

int open_txt(char *file_name){
	int fd;
	fd = open(file_name, O_RDWR | O_CREAT, (mode_t) 0600);
	if ( fd < 0 ) {
		perror("Problem Creating file");
		exit(-1);
	} else printf("File %s Created\n", file_name);
	return fd;
}

void write_txt(link l, int fd, char *enc){
	link ch;
	size_t size, len;
	int err, i=0;

	size = strlen(enc);

	while( i < size ){
		ch=l;
		while(ch!=NULL){
			len = strlen(ch->code);
			if ( strncmp(ch->code, enc, len) == 0 ){
				err=write(fd, &ch->index, 1);
				if ( err < 0 ){
					perror("I am sorry");
					exit(-1);
				}
				i += len;
				enc += len;
				break;
			}
			ch=ch->next;
		}
	if ( ch == NULL ) break;
	}
}

int main(int argc, char *argv[])
{
	int count=0, count_ch=0, i, err, ans;
	char *file_name, *name_cm, *name_huf, *buffer, *buffer2, *encoded;
	FILE *fd;
	int fd_cm, fd_huf, fd_txt;
	link ch, b;
	struct stat st;
	size_t size, len, fsize;
        char enc[10]="";

	bit_len=0;//Number of the final bits

	head=malloc(sizeof *head);
	ch=head;

	//print the options to the user
do{
	//clear the screen
	printf("\e[2J");
	printf("\e[0;0f");

	printf("1.Encode a .txt file\n2.Decode a .huf file\n[Please type your selection]\n");
	scanf("%d", &ans);
}while( ans > 2 || ans < 1 ); //if wrong answer, ask again

if( ans == 2 ) {
//for the second part
	read_cm(buffer2, ch);//reading the cm and save the encoding to struct

        name_huf=malloc(25); // save the name of the .huf file
        printf("Please give the .huf\n");
        scanf("%s", name_huf);

	fd_huf = open_huf(name_huf); // open the .huf file

	stat(name_huf, &st); // find the size in bytes of the .huf and malloc the encode
        size = st.st_size;
        encoded = malloc(8*size); //because every byte in .huf is 8 bit and we want to save it to encode bit by bit in chars
        strcpy(enc,"");

	read_huf(encoded, fd_huf);//reading the .huf and save it to encoded

	len = strlen(name_huf);
	file_name=malloc(len);
        memcpy(file_name, name_huf, len-3);// name the .txt from the .huf
        strcat(file_name, "txt");

	fd_txt = open_txt(file_name);

	ch=head->next;
	write_txt(ch, fd_txt, encoded);
	close(fd_txt);
} else {

	file_name=malloc(25);

	//Checking the parameters and take the file name from the user
	if( argc > 2 ){
		printf("Too many parameters, I am sorry\n");
		exit(-1);
	} else if ( argc == 2 ){
		strcpy(file_name, argv[1]);
	} else{
		printf("Enter filename:\n");
		scanf("%s", file_name);
	}

	//Opening the file
	printf("Searching for %s ...\n", file_name);
	sleep(2);
	fd = fopen(file_name, "r");
	if( fd == NULL ){
		perror("Error while opening the file\n");
		exit(EXIT_FAILURE);
	} else printf("File opened\n");

	stat(file_name, &st);

        //malloc the name_cm and name_huf from lenght of .txt
        len = strlen(file_name);
        name_cm = malloc((len-1)*sizeof(char));
        name_huf = malloc(len*sizeof(char));

        //name the .cm and .huf files
        memcpy(name_cm, file_name, len-4);
        strcat(name_cm, ".cm");
        memcpy(name_huf, file_name, len-4);
        strcat(name_huf, ".huf");

	size = st.st_size;
	buffer2 = (char *) malloc(size);
	buffer = (char *) malloc(1);

	//reading the file
	while(fread(buffer, 1, 1, fd)){
		count++;
		if(strpbrk(buffer2, buffer) == NULL )
		{
			count_ch++; //How many different chars the file has
			ch->next=NEW(*buffer);
			ch=ch->next;
		} else {
			search_list(*buffer);
		}

		strcat(buffer2, buffer);
		fseek(fd, count, SEEK_SET);
	}

	//sorting the list
	head=sort_list(head);

	//Constructing the huffman tree
	ch=head->next;
	while(ch->next != NULL){
		head = sort_list(head);
	        ch=head->next;
		sub_list(head);
	}

	//Encode the characters
        headb = malloc(sizeof *headb);
        headb->next=NULL;
	b = headb;
	for(i=0; i<count_ch; i++){
		ch = head->next;
		encode(ch, enc, b);
		b = b->next;
	}

	//open the .cm file
	printf("Creating file %s\n", name_cm);
	fd_cm = open(name_cm, O_RDWR | O_CREAT, (mode_t) 0600);
	if ( fd_cm < 0 ) {
		perror("Problem Creating file");
		exit(-1);
	}

	//write the .cm file
        ch=headb->next;
	write_cm(fd_cm, ch);

	//write the final string to encoded
	encoded = malloc(bit_len);//to right the encoded text
        ch=headb->next;
	final_cod(encoded, ch, buffer2); //encoded the final string, ch the list and buffer2 the buffer that keeps the starting string

	//open the .huf file
        printf("Creating file %s\n", name_huf);
        fd_huf = open(name_huf, O_RDWR | O_CREAT, (mode_t) 0600);
        if ( fd_huf < 0 ) {
                perror("Problem Creating file");
                exit(-1);
        }

	//write to .huf
	write_huf(encoded, fd_huf);
	stat(name_huf, &st);
	fsize = st.st_size;
	fclose(fd);
	printf("Size of %s = %d, Size of %s = %d, %d bytes smaller!!! \n", file_name, size, name_huf, fsize, size - fsize);
}
printf("Goodbye\n");

	//Free the allocated memory
/*	free(headb);
//	free_list(head);
	free(buffer);
	free(buffer2);*/
	close(fd_cm);
	close(fd_huf);
	return 0;
}
