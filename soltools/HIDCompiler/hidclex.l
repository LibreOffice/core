/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
%{
// Suppress any warnings from generated code:
#if defined __GNUC__
#pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif

static char const Revision[]     = "$Revision: 1.9 $" ;

/*

XX  XX     XXXX   XXXXXX
XX  XX      XX     XX  XX
XX  XX      XX     XX  XX
XXXXXX      XX     XX  XX
XX  XX      XX     XX  XX
XX  XX      XX     XX  XX
XX  XX     XXXX   XXXXXX


  XXXX                                 XX      XXX
 XX  XX                                         XX
XX         XXXX   XXX XX   XX XXX     XXX       XX     XXXXX   XX XXX
XX        XX  XX  XX X XX   XX  XX     XX       XX    XX    X   XXX XX
XX    X   XX  XX  XX X XX   XX  XX     XX       XX    XXXXXXX   XX
 XX  XX   XX  XX  XX   XX   XXXXX      XX       XX    XX        XX
  XXXX     XXXX   XXX  XXX  XX        XXXX     XXXX    XXXXX   XXXX
                           XXXX

*/


/* length of buffer for reading with lex */
/**/
/* enlarge token buffer to tokenize whole strings */
#undef YYLMAX
#define YYLMAX 64000

int exclude_bracelevel=0; /* count braces in that start state */
static YY_BUFFER_STATE InputFiles[127];		// for recursive parse
static int nInputFileDepth = 0;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

int alloc_cnt = 0;
static bool bVerbose = false;
void freestring( char const * ); //forward

char* dot2underline( char* str )
{
	size_t len=strlen(str);
	for( size_t i=0; i<len; i++ )
		if( str[i]=='.' ) str[i]='_';
	return str;
}

void dotappend( char*& base, char const * suffix )
{
	// append like "str.suffix" if suffix else "str"

	if( suffix == NULL )
		return ;

	size_t siz;
	if( (siz=strlen(suffix)) == 0 )
		return ;

	size_t siz2 = base==NULL ? 0 : strlen(base);

	char* newstr;

	{//do never forget to increment/decrement alloc_cnt
	newstr = (char*) malloc( (siz2+siz+1) +1 /*for "." */ );
	alloc_cnt++;
	}

	if( newstr == NULL )
		fprintf( stderr,"error: dotappend() could not malloc() \n"), exit(1);

	if( base ) {
		strcpy( newstr, base );
		freestring( base );
	}
	else
		newstr[0] = '\0';

	strcat( newstr, "." );
	strcat( newstr, suffix );
	base = newstr;
}

void dotsubtract( char*& base, char const * suffix )
{
	// remove dotted suffix from base.suffix --> base

	if( base == NULL || suffix == NULL )
		return;
	if( strlen(suffix) == 0 )
		return;
	if( (strlen(suffix)+1) > strlen(base) )
		return;
	char * pos=strstr( base, suffix);
	if( pos && (pos-1 >= base) && ( *(pos-1) == '.') )
		if( *(pos+strlen(suffix)) == '\0' ) //only if at end of base
			*(pos-1)= '\0';
	return;
}

int level = 0;
char levelbuffer[512];

void adjust_levelbuffer()
{
	int i;
	for(i=0; i<level;i++) {
		levelbuffer[i] = '\t';
	}
	levelbuffer[i] = '\0';
}

void freestring( char const * ptr )
{
    alloc_cnt--;
    adjust_levelbuffer();
    if ( bVerbose )
	    fprintf(stdout,"%sinfo: freestring line %d\n",levelbuffer, __LINE__);
    free(const_cast< char * >(ptr)), ptr = NULL;
}

void makestring(char** newstr, char* oldstr)
{
	alloc_cnt++;
	adjust_levelbuffer();
	if ( bVerbose )
	    fprintf(stdout,"%sinfo: makestring on line %d\n",levelbuffer,__LINE__);
	strcpy( (*newstr=(char*)malloc(strlen(oldstr)+1)), oldstr);
	if( *newstr==NULL ) {
		fprintf( stderr,
		"error: cannot malloc for makestring() alloc_cnt==%d \n", alloc_cnt);
		exit(1);
	}
}

#ifndef WNT
int strcmpi(char const * stra, char const * strb)
{
	// like strcmp() but case insensitive
	size_t i;
	char a,b;
	for(i=0; ;i++){
		a = (char) tolower(stra[i]);
		b = (char) tolower(strb[i]);
		if( a < b )
			return -1;
		if( a > b )
			return 1;
		if( a == '\0' && b == '\0' )
			return 0;
	}
}
#endif

/* variables for listfile ( with project + pathname of src file ) */
char* listfilename;
#define MAXSRCFILES 2048
char* filename_tab[MAXSRCFILES];
char* project_tab[MAXSRCFILES];
int tab_entries = 0;

/* globale variablen */
FILE* outfile;
char const *filename = ""; //incl. path
char const *project = "";
char const *subpath = "";	//from project name downwards like source\ui\xxx.src

int firstprint = 1;

int in_define = 0;

class ident_ring
{
	#define MAXRING 2
	//nicht aendern wg externer Abfrage ->size() == 2

	char*  ring[MAXRING];
	int     ringpos;
	int     last;
	int 	broken;
public:
	        ident_ring()  {
							last = ringpos = -1;
							for(int i=0; i<MAXRING;i++)
								ring[i]=NULL;
							broken = 0;
						  }

	void    set_zero()    {
							last = ringpos = -1;
							for(int i=0; i<MAXRING;i++) {
								if( ring[i] != NULL )
									freestring( ring[i] );
								ring[i]=NULL;
							}
							broken = 0;
						  }
	void	set_broken()  { broken = 1; }
	int     size()        {
							if( ringpos == -1 )
								return 0;

							if( broken) {
								if( ringpos==0 )
									return 0;
								else
									return 1;
							} else {
								if( ringpos==0 )
									return 1;
								else
									return 2;
							}
						  }

	char*	extract_last(){
							if( last == -1 )  {
								return NULL;
							} else {
								char* ttt=ring[last];
								ring[last]=NULL;
								return ttt;
							}
					      }

	char*	extract_actual()  {
							if(ringpos == -1) {
								return NULL;
							} else {
								char *ttt=ring[ringpos];
								ring[ringpos]=NULL;
								return ttt;
							}
						  }

	void    set(char* id) {
							ringpos= (++ringpos) % MAXRING;
							if( ring[ringpos] != NULL )
								freestring( ring[ringpos] ); /*otherwise: memory lost*/
							ring[ ringpos ] = id;
							if ( bVerbose )
							    fprintf(stdout,
								    "info: ring[%d] == %s broken==%d \n"
								    ,ringpos,ring[ringpos],broken
							    );

							if( !(ringpos==0 && last==-1) )
								last = (++last) % MAXRING;
						  }
};


/* Notnagel: die letzten identifier/number merken, fuer klasse & globalID */
ident_ring *ring;

//
typedef int token;
//
#define TOK_SEMICOLON	';'
#define TOK_EQUAL		'='
#define TOK_OPENBRACKET	'['
#define TOK_CLOSEBRACKET ']'
#define TOK_OPENBRACE	'{'
#define TOK_CLOSEBRACE	'}'
#define TOK_KOMMA		','
#define TOK_LESS		'<'
#define TOK_GREATER		'>'
#define TOK_OPENPAREN	'('
#define TOK_CLOSEPAREN	')'
#define TOK_PLUS		'+'
#define TOK_MINUS		'-'
#define TOK_STAR		'*'
#define TOK_SLASH		'/'
//
#define TOK_POSorSIZE	146
#define TOK_POSSIZE		147
#define TOK_TEXTTAG		148
#define TOK_IDNUM		149
//
#define TOK_EXTRADATA	151
#define TOK_TEXT		152
#define TOK_MESSAGE		153
#define TOK_HELPTEXT	154
#define TOK_TRUE		155
#define TOK_FALSE		156
//
#define TOK_RESID	180
//
#define TOK_STRING		190
//
#define TOK_INVALID     (-1)

token  lasttoken = TOK_INVALID;




#define UNDEF (-1)

char *globalID = const_cast< char * >("");
char const *globalKLASSE = NULL;

void reset_globalID() {
	//
	if( globalID && (strlen(globalID) > 0 ) ) {
		freestring( globalID );
		globalID = const_cast< char * >("");
	}
}


//--------------------------------------------------------------------
class resource
{
private:
	resource ( const resource& );  //copy-ctor soll keiner benutzen
	void operator=( const resource& ); // zuweisung auch nicht
public:
	resource();
	~resource();
public:
	int	 lineno;
	token  lasttoken; //before opening {
	char const *klasse ;
	char *localID;
	char *helpID;
	int  residfound;
};

resource *ares ; /* aktuell bearbeitete resource */

resource::~resource()
{
	if( klasse != NULL )   freestring(klasse);
	if( localID != NULL )  freestring(localID);
	if( helpID != NULL )   freestring(helpID);

}

resource::resource()
{
	lineno =   UNDEF;
	lasttoken= TOK_INVALID;
	klasse =    NULL ;
	localID =  NULL ;
	helpID =   NULL ;
	residfound= 0;
}

int residfound = 0; // "Identifier = " auf momentanem level gefunden

#define   MAXSTACK 32
resource* stack[MAXSTACK]; /* resource stack */
#define   EMPTYSTACK (-1)
int       stackptr = EMPTYSTACK;

void push_resource( resource* r )
{
	stackptr++;
	if( stackptr >= MAXSTACK ) {
		fprintf( stderr, "error: resource stack is full %d \n", stackptr);
		exit(1);
	}
	stack[ stackptr ] = r;
}

resource* pop_resource()
{
	if( stackptr < 0 ) {
		fprintf( stderr, "error: pop resource from empty stack \n");
		exit(1);
	}
	return stack[ stackptr-- ];
}

/* forward */
int eat_comment();
int eat_cpp_comment();

/*===================================================*/
//
// '+' im identifier wg basic\source\classes\sb.src
//
// '<' im identifier wg sc subtdlg.src
//
// '&' im identifier wg scerror.src so2..nocode.src svxerr.src scwarngs.src


//string		(\"[^"]*\") alter einfacher string ohne "

%}

/* 89012 */
%option never-interactive


simple		([^\n\"]*)
%p 7000
string		\"{simple}((((\\\\)*(\\\"))?){simple})*\"

%x MACRO_STATE

%x EXCLUDE_STATE
resfilelist	    ([Ff][Ii][Ll][Ee][Ll][Ii][Ss][Tt])
resstringlist	([Ss][Tt][Rr][Ii][Nn][Gg][Ll][Ii][Ss][Tt])
resstring   	([Ss][Tt][Rr][Ii][Nn][Gg])

identifier ([a-z_A-Z]+[a-z_A-Z<+&0-9]*)
number		(([0-9]+)|(0x[0-9a-fA-F]+))
residentifier ([Ii][Dd][Ee][Nn][Tt][Ii][Ff][Ii][Ee][Rr])
w			([ \t\n]*)
wspecial	([\\ \t\n]*)
texttag		(([Tt][Ii][Tt][Ll][Ee])|([Tt][Ee][Xx][Tt])|([Mm][Ee][Ss][Ss][Aa][Gg][Ee]))
qhelptag	(([Qq][Uu][Ii][Cc][Kk])?([Hh][Ee][Ll][Pp][Tt][Ee][Xx][Tt]))
langtag		([a-zA-Z_]+)
helptag		([Hh][Ee][Ll][Pp][Ii][Dd])
helpid		([a-zA-Z_0-9]+)
num2tag		(([Pp][Oo][Ss])|([Ss][Ii][Zz][Ee]))
num4tag		(([Pp][Oo][Ss][Ss][Ii][Zz][Ee]))

%%
[ \t]		{
				/* forget whitespace */;
			}

^[ \t]*#include.*\.src[">].*	{
				char NewFile[255];		//long names???
				int i, j, GetIt;

				GetIt = 0;
				j = 0;
				nInputFileDepth++;
//	nicht schoen aber geht...
				for (i = 0; yytext[i+1] != 0; i++)
				{
					if ( GetIt == 1 )
					{
						if ( yytext[i] == '"' || yytext[i] == '>' )
							GetIt = 0;
						else
							NewFile[j++] = yytext[i];
					}
					if ( yytext[i] == '"' || yytext[i] == '<' )
						GetIt = 1;
				}
				NewFile[j] = '\0';
				FILE* pFile = NULL;
				pFile = fopen( NewFile, "r" );
				if( pFile == NULL ) {
					fprintf( stdout, "warning: could not open inputfile %s \n", NewFile );
					// try the new *_tmpl.src version instead
					// this hack was introduced to allow localisation of included src files
					const char* sStrTmpl = "_tmpl";
					j -= 4;
					for ( i = 0 ; i <5 ; i++,j++ )
					{
					    NewFile[j+5] = NewFile[j];
					    NewFile[j] = sStrTmpl[i];
					}
    				NewFile[j+4] = '\0';
					fprintf( stderr, "trying inputfile %s \n", NewFile );
    				pFile = fopen( NewFile, "r" );
				    if( pFile == NULL ) {
					    fprintf( stderr, "error: could not open inputfile %s \n", NewFile );
					    exit(1);
    			    }
				}
				InputFiles[ nInputFileDepth ] = yy_create_buffer( pFile, YY_BUF_SIZE );
                yy_switch_to_buffer( InputFiles[ nInputFileDepth ] );

                if ( bVerbose )
				    fprintf( stdout, "%s //ATTENTION!! %s gets included here\n", yytext, NewFile );
				fprintf( outfile, "// %s //ATTENTION!! %s gets included here\n\n", yytext, NewFile );
			}

^[ \t]*#include.*	{
				fprintf( outfile, "%s\n\n", yytext );
			}

^[ \t]*#(if|ifdef|ifndef|elif).*		{
				;
			}

^[ \t]*#else	{
				;
			}
^[ \t]*#endif	{
				;
			}

^[ \t]*#(undef|error|pragma).*	{
				;
			}

^[ \t]*#define	{
				fprintf( outfile, "%s", yytext );
				BEGIN MACRO_STATE;
			}

<INITIAL,MACRO_STATE>"/*"		{
				eat_comment();
			}

<INITIAL,MACRO_STATE>"//"		{
				eat_cpp_comment();
			}

<MACRO_STATE>\n {
				fprintf( outfile, "%s\n", yytext );
				BEGIN INITIAL;
			}

<MACRO_STATE>\\\n {
				fprintf( outfile, "\\\n" );
			;/* macro schadet nicht, koennte gebraucht werden */
			}


<MACRO_STATE>. {
			fprintf( outfile, "%s", yytext );
			;/* ignore all this unused input */
			}

";" 		{ring->set_zero(); lasttoken = TOK_SEMICOLON; }
"=" 		{ring->set_zero(); lasttoken = TOK_EQUAL; }
"[" 		{ring->set_broken(); lasttoken = TOK_OPENBRACKET; }
"]" 		{ring->set_broken(); lasttoken = TOK_CLOSEBRACKET; }


<EXCLUDE_STATE>"{" 		{
				exclude_bracelevel += 1;
			    if ( bVerbose )
				    fprintf( stdout,"info: lev %d : found {\n", exclude_bracelevel );
			}


\\\n		|
"{" 		{
				// define continues
				// or a brace opens a block

				if( in_define && !strcmp(yytext,"\\\n") ) {

					if( in_define++ == 1 )
						;// keep on working
					else
						goto blockend;
				}

				level++;
				if( ares != NULL ){
					if( level > 1 )
						dotappend( globalID, ares->localID );
					ares->residfound = residfound;
					push_resource( ares );
				}
				ares = new resource;

				residfound = 0;
				ares->residfound = 0;

				ares->lineno = yylineno;
				ares->lasttoken = lasttoken;
				if( ring->size() == 2 ) {
					ares->klasse = ring->extract_last();
					ares->localID = ring->extract_actual();
				} else if(ring->size() == 1) {
					ares->klasse = ring->extract_actual();
				}
				if( level==1 ){ //Ausnahme: Resource auf Ebene 1
					globalID= ares->localID;
					ares->localID = NULL;
					globalKLASSE= ares->klasse;
				}

                if ( bVerbose )
				    fprintf(stdout,"info: { level: %d\n", level);

				lasttoken = TOK_OPENBRACE;
				ring->set_zero();
			}

<EXCLUDE_STATE>"}" 		{
				//-----------------------------
				exclude_bracelevel -= 1;
				if ( bVerbose )
				    fprintf( stdout,"info: lev %d : found }\n", exclude_bracelevel );
				if( exclude_bracelevel==1 ) {
					BEGIN INITIAL;
					exclude_bracelevel=0;
				}
			}


\n			|
"}" 		{
				if ( bVerbose )
				    fprintf(stdout,"info:   } level: %d\n",level);

				if( !strcmp(yytext,"}") )
					;
				else if( in_define && (!strcmp(yytext,"\n") )) {
					if( in_define==1 ) {
						//no continuation line for #define
						in_define=0;
						goto blockend;
					}
					else {
						//there was a continuation line for #define
						in_define=0;
					}
				}
				else
					goto blockend;

				// ares ausgeben
				if( ares != NULL ) {

					#define LEER "leer"
					char const * globklasse =
                      globalKLASSE==NULL ? LEER:globalKLASSE;
					char const * local =
                      ares->localID==NULL ? LEER:ares->localID;
					char const * klasse =
                      ares->klasse==NULL ? LEER:ares->klasse;
					char const * glob =
                      globalID==NULL ? LEER:globalID;


					//wg. Starview-Klasse String in ehdl.c und doc.c
					// wenn generierte C++-Quellen compiliert werden
					//
					//if( !strcmp(globklasse,"String" )) globklasse = "string";
					//if( !strcmp(klasse,"String" )) klasse = "string";


					//---------------------------------------------------
					// generate the body of a new C main program,
					// which is filled with printf statements
					// to compute (via preproseccor & compiler)
					// the codenumbers for resource names like menu$RID_SVX$xyz

				   	if( firstprint ) {
				   		firstprint=0;
						fprintf(outfile, "#include \"starview.hid\" \n\n");
						fprintf(outfile, " int main() { \n\n\n");
					}

					char globunder[256];
				    strcpy(globunder,glob);
					dot2underline( globunder );
					char const * globsuffix = strrchr(glob,'.');
					globsuffix = globsuffix==NULL ? glob:globsuffix+1;

					if( ares->helpID ) {
						fprintf( outfile,"\n\t printf(\"%s \\t %cs %cu \\n\",\n",
						ares->helpID,'%','%');
						fprintf(outfile,"\t\"HelpID\", (%s) ); \n", ares->helpID);
					}
					else if( ares->localID ) {
						fprintf( outfile,"\n\t printf(\"%s:%s:%s:%s \\t %cs %cu %cs %cu \\n\",\n",
						project,klasse,globunder,local,'%','%','%','%');
						fprintf( outfile,"\t\"Norm %s\", (%s), \"%s\", (%s) );\n",
						globklasse,globsuffix, klasse,local);
					}
					else if( (strcmpi("MenuItem",klasse)==0) ||
						    (strcmpi("ToolBoxItem",klasse)==0) ) {
							; //no output (99% is a separator)
					}
					else {
						fprintf( outfile,"\n\t printf(\"%s:%s:%s \\t %cs %cu %cs \\n\",\n",
						project,klasse,globunder,'%','%','%');
						fprintf( outfile,"\t\"Norm %s\", (%s), \"%s\" );\n",
						globklasse,globsuffix, klasse);
					}

					delete ares;
				}


				// ein level zurueck
				if( level == 1) {
					reset_globalID();
					globalKLASSE = NULL;
				}
				level--;
				ares = pop_resource();
				residfound = ares->residfound;
				dotsubtract( globalID, ares->localID );


				//
				lasttoken = TOK_CLOSEBRACE;
				ring->set_zero();
		blockend:	;
			}

"," 		{ring->set_broken(); lasttoken = TOK_KOMMA; }

"<" 		{ring->set_broken(); lasttoken = TOK_LESS; }
">" 		{ring->set_broken(); lasttoken = TOK_GREATER; }

"(" 		{ring->set_broken(); lasttoken = TOK_OPENPAREN; }
")" 		{ring->set_broken(); lasttoken = TOK_CLOSEPAREN; }
"+" 		{ring->set_broken(); lasttoken = TOK_PLUS; }
"-" 		{ring->set_broken(); lasttoken = TOK_MINUS; }
"*" 		{ring->set_broken(); lasttoken = TOK_STAR; }
"/" 		{ring->set_broken(); lasttoken = TOK_SLASH; }


{helptag}{w}"="{w}{helpid}{w}";" {

			// extract text for helpid and put to ares
			char*     pos = strchr(yytext,'=');
			size_t offset = strspn(pos+1," \t\n");
			char*   start = pos+1+offset;
			size_t offset2= strcspn( start, "; \t\n");
			char*     end = start+offset2;
			*end = '\0';
			char *helpid;
			makestring( &helpid, start );
			ares->helpID = helpid;
			}

{residentifier}{w}"="[ \t\n]*({identifier}|{number}) {
			ring->set_zero();
			lasttoken = TOK_RESID;
			residfound=1;

			//extract resource id and store as localID
			char *after = strrchr(yytext,'=');
			char *resid = after + strspn(after,"= \t\n");
			char *localID;
			makestring( &localID, resid );
			ares->localID = localID;
			}

{resfilelist}	|
{resstring}     |
{resstringlist} {
			BEGIN EXCLUDE_STATE;
			exclude_bracelevel = 1;
			if ( bVerbose )
			    fprintf( stdout,"info: lev %d : found exclusion\n", exclude_bracelevel );
			}

^[ \t]*#define		|
{number}	        |
{identifier} {
		/* identifier/number in einem ring ablegen */
		char *identifier;
		char *def=strstr(yytext,"#define");
		if( def ) {
			in_define = 1;
			makestring( &identifier, def+1 );
		}
		else
			makestring( &identifier, yytext );
		ring->set( identifier );
		lasttoken = TOK_IDNUM;

	}
<INITIAL,EXCLUDE_STATE>{string}	{
			ring->set_broken();
            lasttoken = TOK_STRING;
            if ( bVerbose )
			    fprintf(stdout, "%6s %11s %8d %s \n",project,filename,yylineno, yytext);
			}


<INITIAL,EXCLUDE_STATE>.	{ if ( bVerbose ) fprintf( stdout,"warning: unused input on line %d of %s \n%s\n",
					yylineno, filename, yytext);
	}

<EXCLUDE_STATE>\n	{
						; //do nothing, ignore
	}


%%


void makeversion( char* version )
{
	char const *pos = strpbrk( Revision, "0123456789." );
	size_t siz = strspn( pos,      "0123456789." );
	if( pos && siz ) {
		strncpy(version, pos, siz);
		strcat( version, " ");
	}
	else
		strcpy( version," unknown " );
}

int main( int argc, char* argv[] )
{
	static char const *Compiler = "HID-Compiler ";
	static char const *Author   = "OG ";
	static char HIDCompiler[100];
	static char Version[100];

    // check for switches given on the command line
    if ( ( argc > 0 ) && ( 0 == strcmp( argv[0], "-verbose" ) ) )
    {
        bVerbose = true;
        for ( size_t i=0; i<argc-1; ++i )
        {
            argv[i] = argv[i+1];
            --argc;
        }
    }

	makeversion( Version );
	strcpy( HIDCompiler, Compiler );
	strcat( HIDCompiler, Version  );
	strcat( HIDCompiler, Author   );
	if ( bVerbose )
	    fprintf( stdout, "\n %s \n\n", HIDCompiler);
	if(argc < 4) {
		fprintf(
		stderr,
		"usage: hidc [-verbose] file.src file.c project\n"
		"\n"
		"You must give exactly 3 arguments.\n"
		"1 - an existing SRC file.\n"
		"2 - C file to be generated (which generates the HID file when run).\n"
		"3 - the project name (an arbitrary name).\n\n"
		);
		exit(1);
	}

	project = argv[3];

	char *outfilename = argv[2];
	if( (outfile=fopen( outfilename , "w" )) ==NULL ) {
		fprintf(stderr,"error: could not open outputfile '%s' \n", outfilename);
		exit(1);
	}

	filename = argv[1];
	nInputFileDepth = 0;
	FILE* pFile;
	pFile = fopen( filename, "r" );
	if( pFile == NULL ) {
		fprintf( stderr, "error: could not open inputfile %s \n", filename );
		exit(1);
	}
	InputFiles[ nInputFileDepth ] = yy_create_buffer( pFile, YY_BUF_SIZE );
    yy_switch_to_buffer( InputFiles[ nInputFileDepth ] );
	ring = new ident_ring;
	ares = new resource;

	fprintf(outfile, "/* Generated from %s */\n\n", HIDCompiler );
	fprintf(outfile, "/* Source was: %s */\n", filename );

	yylineno = 1;
	yylex(); /* do the real work here */

   	if( firstprint ) {
		fprintf(outfile, "#include \"starview.hid\" \n\n");
		fprintf(outfile, " int main() { \n\n\n");
	}
	fprintf(outfile, "\nreturn 0;");
	fprintf(outfile, "\n} /*main*/\n");
	return 0;
}

int yywrap()
{
    yy_delete_buffer( InputFiles[ nInputFileDepth ] );
	if ( nInputFileDepth == 0 )
		return 1;/* keine src files mehr */
	else
	{
		nInputFileDepth--;
    	fprintf(outfile, "// Done reading file\n\n");
        yy_switch_to_buffer( InputFiles[ nInputFileDepth ] );
		return 0;
	}
}

int eat_comment()
{
	int c;
	int lastc = ' ';
	while( (c=yyinput()) != EOF ) {
		if( c=='\n')
			;
		else if( c=='/' && lastc=='*' )
			break; /* end of comment found */
		lastc=c;
	}
	return 0;
}

int eat_cpp_comment()
{
	int c;
	while( (c=yyinput()) != EOF ) {
		if( c=='\n') {
			break;
		}
	}
	if( c != EOF )
		unput(c); /* because next #.... line was not found */
	return 0;
}
