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

/* external functions (C++ code, declared as extren "C" */
extern WorkOnTokenSet( int, char* );
extern Argument( char * );
extern InitXrmExport( char * );
extern EndXrmExport();
extern GetError();
extern SetError();
extern char *GetOutputFile( int argc, char* argv[]);
extern FILE *GetXrmFile();
extern int isQuiet();

/* forwards */
void YYWarning();

int bText=0;
%}

%p 24000
%e 1200
%n 500

%%

"<Readme"[^\>]*\> {
	WorkOnTokenSet( XRM_README_START, yytext );
}

"</Readme>" {
	WorkOnTokenSet( XRM_README_END, yytext );
}

"<Section"[^\>]*\> {
	WorkOnTokenSet( XRM_SECTION_START, yytext );
}

"</Section>" {
	WorkOnTokenSet( XRM_SECTION_END, yytext );
}

"<Paragraph"[^\>]*\> {
	WorkOnTokenSet( XRM_PARAGRAPH_START, yytext );
}

"</Paragraph>" {
	WorkOnTokenSet( XRM_PARAGRAPH_END, yytext );
}

"<Text"[^\>]*\> {
	WorkOnTokenSet( XRM_TEXT_START, yytext );
}

"</Text>" {
	WorkOnTokenSet( XRM_TEXT_END, yytext );
}

"<List"[^\>]*\> {
	WorkOnTokenSet( XRM_LIST_START, yytext );
}

"</List>" {
	WorkOnTokenSet( XRM_LIST_END, yytext );
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
    if( !isQuiet() ){
        fprintf( stdout, "\nXrmEx 0.9 Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.\n" );
	    fprintf( stdout, "======================================================================\n" );
    }

	if ( !pOutput ) {
		fprintf( stdout, "Syntax: XRMEX[-p Prj][-r PrjRoot]-i FileIn [-o FileOut][-m DataBase][-e][-b][-u][-NOUTF8][-L l1,l2,...][-ISO99 IsoCode]\n" );
		fprintf( stdout, " Prj:      Project\n" );
		fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
		fprintf( stdout, " FileIn:   Source files (*.src)\n" );
		fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
		fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
		fprintf( stdout, " -e: Disable writing errorlog\n" );
		fprintf( stdout, " -b: Break when Token \"HelpText\" found in source\n" );
		fprintf( stdout, " -u: [english] and [german] are allowed, Id is Taken from DataBase \n" );
		fprintf( stdout, " -NOUTF8: disable UTF8 as language independent encoding\n" );
		fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (01,33,46,49...)\n" );
		fprintf( stdout, "     A fallback language can be defined like this: l1=f1.\n" );
		fprintf( stdout, "     f1, f2,... are also elements of (01,33,46,49...)\n" );
		fprintf( stdout, "     Example: -L 01,99=35\n" );
		fprintf( stdout, "              Restriction to 01 and 99, 35 will be fallback for 99\n" );
		fprintf( stdout, " -ISO99: IsoCode is the full qualified ISO language code for language 99" );
		return 1;
	}

	pFile = GetXrmFile();
	if ( !pFile )
		return 1;

   	yyin = pFile;

	/* create global instance of class XmlExport */
	InitXrmExport( pOutput );

	/* start parser */
   	yylex();

	/* get error info. and end export */
	nRetValue = GetError();
	EndXrmExport();

	if( !isQuiet() ) fprintf( stdout, "\n===================================\n\n" );

	/* return error level */
	return nRetValue;
}
