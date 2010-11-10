%{
/*
 * lexer for parsing xml-property source files (*.xml)
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
extern int Argument( char * );
extern int InitXrmExport( char * , char * );
extern int EndXrmExport();
extern int GetError();
extern int SetError();
extern char *GetOutputFile( int argc, char* argv[]);
extern FILE *GetXrmFile();
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

"<p "[^\>]*xml:lang[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START , yytext );
}

"</p>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}

"<h1 "[^\>]*xml:lang[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START , yytext );
}

"</h1>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}
"<h2 "[^\>]*xml:lang[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START , yytext );
}

"</h2>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}
"<h3 "[^\>]*xml:lang[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START , yytext );
}

"</h3>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}
"<h4 "[^\>]*xml:lang[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START , yytext );
}

"</h4>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}
"<h5 "[^\>]*xml:lang[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START , yytext );
}

"</h5>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}






"<!--"	{
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

.|\n {
	if ( bText == 1 )
		WorkOnTokenSet( XML_TEXTCHAR, yytext );
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
		fprintf( stdout, "Syntax: XRMEX[-p Prj][-r PrjRoot]-i FileIn [-o FileOut][-m DataBase][-e][-b][-u][-NOUTF8][-L l1,l2,...]\n" );
		fprintf( stdout, " Prj:      Project\n" );
		fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
		fprintf( stdout, " FileIn:   Source files (*.src)\n" );
		fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
		fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
		fprintf( stdout, " -e: Disable writing errorlog\n" );
		fprintf( stdout, " -b: Break when Token \"HelpText\" found in source\n" );
		fprintf( stdout, " -u: [english] and [german] are allowed, Id is Taken from DataBase \n" );
		fprintf( stdout, " -NOUTF8: disable UTF8 as language independent encoding\n" );
		fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (de,en-US,es...)\n" );
		fprintf( stdout, "     A fallback language can be defined like this: l1=f1.\n" );
		fprintf( stdout, "     f1, f2,... are also elements of (de,en-US,es...)\n" );
		fprintf( stdout, "     Example: -L en-US,es=de\n" );
		fprintf( stdout, "              Restriction to es and en-US, de will be fallback for 99\n" );
//		fprintf( stdout, " -ISO99: IsoCode is the full qualified ISO language code for language 99" );
		return 1;
	}
	pFile = GetXrmFile();
	InitXrmExport( pOutput , getFilename() );

    if ( !pFile )
		return 1;

   	yyin = pFile;

	/* create global instance of class XmlExport */
	//InitXrmExport( pOutput );

	/* start parser */
   	yylex();

	/* get error info. and end export */
	nRetValue = GetError();
	EndXrmExport();

    removeTempFile();
	/* return error level */
	return nRetValue;
}
