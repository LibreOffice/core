%{
/*
 * lexer for parsing cfg source files
 *
 */


/* enlarge token buffer to tokenize whole strings */
#undef YYLMAX
#define YYLMAX 64000

/* to enable debug output define LEXDEBUG */
#define LEXDEBUG		1
#ifdef LEXDEBUG
#define OUTPUT	fprintf
#else
#define OUTPUT(Par1,Par2);
#endif

/* table of possible token ids */
#include "tokens.h"
#include <stdlib.h>
#include <stdio.h>

#if defined __GNUC__
#pragma GCC system_header
#elif defined __SINPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif

/* external functions (C++ code, declared as extren "C" */
extern int WorkOnTokenSet( int, char* );
extern int InitCfgExport( char * , char *);
extern int EndCfgExport();
extern int GetError();
extern int SetError();
extern char *GetOutputFile( int argc, char* argv[]);
extern FILE *GetCfgFile();
extern int isQuiet();
extern void removeTempFile();
extern char* getFilename();
/* forwards */
void YYWarning();

int bText=0;
%}

%p 24000
%e 1200
%n 500

%%

\<[^\>]*"xml:lang="\""x-no-translate"\"[^\<]*\/\>	{
	bText = 0;
	WorkOnTokenSet( CFG_TOKEN_NO_TRANSLATE, yytext );
}

\<.*\/\> {
	bText = 0;
	WorkOnTokenSet( ANYTOKEN, yytext );
}

\<[^\>]*"xml:lang="\".*\"[^\<]*\>	{
	bText = 1;
	WorkOnTokenSet( CFG_TEXT_START, yytext );
}


\<[^\/\!][^\>]*\>	{
	bText = 0;
	WorkOnTokenSet( CFG_TAG, yytext );
}

"<!"DOCTYPE[^\>]*\>	{
	bText = 0;
	WorkOnTokenSet( CFG_TAG, yytext );
}


\<\!\-\-	{
	char c1 = 0, c2 = 0, c3 = input();
	char pChar[2];
	pChar[1] = 0x00;
	pChar[0] = c3;

	WorkOnTokenSet( COMMEND, yytext );
	WorkOnTokenSet( COMMEND, pChar );

	for(;;) {
		if ( c3 == EOF )
			break;
		if ( c1 == '-' && c2 == '-' && c3 == '>' )
			break;
		c1 = c2;
		c2 = c3;
		c3 = input();
		pChar[0] = c3;
		WorkOnTokenSet( COMMEND, pChar );
	}
}

\<\/[^\>]*\> {
	bText = 0;
	WorkOnTokenSet( CFG_CLOSETAG, yytext );
}

\<[^\>\!]*\> {
	bText = 0;
	if ( yytext[ 1 ] == '!' && yytext[ 2 ] == '-' && yytext[ 3 ] == '-' )
		WorkOnTokenSet( COMMEND, yytext );
	else
		WorkOnTokenSet( CFG_UNKNOWNTAG, yytext );
}

.|\n {
	if ( bText == 1 )
		WorkOnTokenSet( CFG_TEXTCHAR, yytext );
	else
		WorkOnTokenSet( UNKNOWNCHAR, yytext );
}


%%

/*****************************************************************************/
int	yywrap(void)
/*****************************************************************************/
{
	return 1;
}

/*****************************************************************************/
void YYWarning( char *s )
/*****************************************************************************/
{
	/* write warning to stderr */
	fprintf( stderr,
		"Warning: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
}

/*****************************************************************************/
#ifdef GCC
void yyerror ( char *s, ... )
#else
void yyerror ( char *s )
#endif
/*****************************************************************************/
{
	/* write error to stderr */
	fprintf( stderr,
		"Error: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
	SetError();
}

/*****************************************************************************/
int
#ifdef WNT
_cdecl
#endif
main( int argc, char* argv[])
/*****************************************************************************/
{
	/* error level */
	int nRetValue = 0;
	char *pOutput;
	FILE *pFile;

	pOutput = GetOutputFile( argc, argv );

    if ( !pOutput ) {
		fprintf( stdout, "Syntax: CFGEX[-p Prj][-r PrjRoot]-i FileIn [-o FileOut][-m DataBase][-e][-b][-u][-f][-d DoneFile][-g[:dtd] ][-L l1,l2,...]\n" );
		fprintf( stdout, " Prj:      Project\n" );
		fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
		fprintf( stdout, " FileIn:   Source files (*.src)\n" );
		fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
		fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
        fprintf( stdout, " -e: Disable writing errorlog\n" );
		fprintf( stdout, " -b: Break when Token \"HelpText\" found in source\n" );
		fprintf( stdout, " -u: [english] and [german] are allowed, Id is Taken from DataBase \n" );
		fprintf( stdout, " -f: force extraction and merge even if only one language is existent\n" );
		fprintf( stdout, " -g[:dtd]: enables generation of properties (dtds if :dtd is set) - in this case FileOut is the output path\n" );
		fprintf( stdout, " -d: enables generation of *.don if work is done\n" );
		fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (de,en-US...)\n" );
		fprintf( stdout, "     A fallback language can be defined like this: l1=f1.\n" );
		fprintf( stdout, "     f1, f2,... are also elements of (de,en-US...)\n" );
		fprintf( stdout, "     Example: -L de,es=en-US\n" );
		fprintf( stdout, "              Restriction to de and es, en-US will be fallback for es\n" );
		return 1;
	}

	pFile = GetCfgFile();
	InitCfgExport( pOutput , getFilename() );
	if ( !pFile )
		return 1;

   	yyin = pFile;

	/* create global instance of class CfgExport */
	//InitCfgExport( pOutput );

	/* start parser */
   	yylex();

	/* get error info. and end export */
	nRetValue = GetError();
	EndCfgExport();

    
    removeTempFile();
/* return error level */
	return nRetValue;
}


