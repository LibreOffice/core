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

#include "sal/main.h"

#if defined __GNUC__
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-label"
#endif
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#elif defined __SINPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif

/* external functions (C++ code, declared as extern "C" */
extern "C" int WorkOnTokenSet( int, char* );
extern "C" int Argument( char * );
extern "C" int InitXrmExport( char * , char * );
extern "C" int EndXrmExport();
extern "C" int GetError();
extern "C" int SetError();
extern "C" char *GetOutputFile( int argc, char* argv[]);
extern "C" FILE *GetXrmFile();
extern "C" int isQuiet();
extern "C" char* getFilename();

/* forwards */
void YYWarning();

int bText=0;
%}

%option yylineno
%option never-interactive

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

"<display-name>" {
	WorkOnTokenSet( DESC_DISPLAY_NAME_START , yytext );
}

"</display-name>" {
	WorkOnTokenSet( DESC_DISPLAY_NAME_END, yytext );
}

"<name "[^\>]*lang[^\>]*\> {
	WorkOnTokenSet( DESC_TEXT_START , yytext );
}

"</name>" {
	WorkOnTokenSet( DESC_TEXT_END, yytext );
}

"<extension-description>" {
	WorkOnTokenSet( DESC_EXTENSION_DESCRIPTION_START , yytext );
}

"</extension-description>" {
	WorkOnTokenSet( DESC_EXTENSION_DESCRIPTION_END , yytext );
}

"<src "[^\>]*lang[^\>]*\> {
	WorkOnTokenSet( DESC_EXTENSION_DESCRIPTION_SRC , yytext );
}



"<!--"	{
	char c1 = 0, c2 = 0, c3 = yyinput();
	char pChar[2];
	pChar[1] = 0x00;
	pChar[0] = c3;

	WorkOnTokenSet( COMMENT, yytext );
	WorkOnTokenSet( COMMENT, pChar );

	for(;;) {
		if ( c3 == EOF )
			break;
		if ( c1 == '-' && c2 == '-' && c3 == '>' )
			break;
		c1 = c2;
		c2 = c3;
		c3 = yyinput();
		pChar[0] = c3;
		WorkOnTokenSet( COMMENT, pChar );
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
void YYWarning( const char *s )
/*****************************************************************************/
{
	/* write warning to stderr */
	fprintf( stderr,
		"Warning: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
}

/*****************************************************************************/
void yyerror ( const char *s )
/*****************************************************************************/
{
	/* write error to stderr */
	fprintf( stderr,
		"Error: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
	SetError();
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
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

	/* return error level */
	return nRetValue;
}
