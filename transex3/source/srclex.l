%{
/*
 * lexer for parsing ressource source files (*.src)
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
extern InitExport( char * , char * );
extern Parse( int nTyp, char *pTokenText );
extern EndExport();
extern SetError();
extern GetError();
extern char *GetOutputFile( int argc, char* argv[]);
extern FILE *GetNextFile();
extern int isQuiet();

/* forwards */
void YYWarning();
%}

%p 24000
%e 1200
%n 500

%%

^[\t ]*"#pragma".*	{
	WorkOnTokenSet( PRAGMA, yytext );
}

^[ \t]*\n {
	WorkOnTokenSet( EMPTYLINE, yytext );
}

[\t ]+ 				|
^[\t ]*"#include".*	|
^[\t ]*"#undef".* 	|
"//".*				|
";" 				|
"<"					|
">"					|
\n	{
	WorkOnTokenSet( IGNOREDTOKENS, yytext );
}
"/*"	{
	char c1 = 0, c2 = input();
	char pChar[2];
	pChar[1] = 0x00;
	pChar[0] = c2;

	WorkOnTokenSet( COMMEND, yytext );
	WorkOnTokenSet( COMMEND, pChar );
	for(;;) {
		if ( c2 == EOF )
			break;
		if ( c1 == '*' && c2 == '/' )
			break;
		c1 = c2;
		c2 = input();
		pChar[0] = c2;
		WorkOnTokenSet( COMMEND, pChar );
	}
}

^[\t ]*"#ifndef".+$	|
^[\t ]*"#ifdef".+$	|
^[\t ]*"#if".+$		|
^[\t ]*"#elif".*$	|
^[\t ]*"#else".*$	|
^[\t ]*"#endif".*$	{
	WorkOnTokenSet( CONDITION, yytext );
}

[a-zA-Z]+[\t ]+[^={\n]+[\t ] {
/* defined Res */
	WorkOnTokenSet( DEFINEDRES, yytext );
}

[a-zA-Z]+[ \t]+[^={;\n]+\n[ \t]*"#".*\n[ \t]*"{"	|
[a-zA-Z]+[ \t]+[^={;\n]+\n{0,1}([ \t]*"//".*\n)*[ \t]*"{"	{
/* RESSOURCE // String TTT_XX ... */
	WorkOnTokenSet( RESSOURCE, yytext );
}

^[\t ]*[a-zA-Z_]+[\t ]*"\\"{0,1}[\t ]*\n{0,1}[ \t]*"{"[\t ]*"\\"{0,1}	{
/* SMALRESSOURCE // String ... */
	WorkOnTokenSet( SMALRESSOURCE, yytext );
}

[\t ]*[a-zA-Z0-9_]+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*){0,1}=[ \t]*L{0,1}\".*\".*\n{0,1}	{
/* TEXTLINE // TextTyp = "A Text" */
	WorkOnTokenSet( TEXTLINE, yytext );
}

[\t ]*[a-zA-Z0-9_]+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*){0,1}(\n[ \t]*){0,1}=([ \t]*\n){0,1}(([a-zA-Z0-9_]+)|(\".*\")|([ \t\n]*))*\".*\"(([a-zA-Z0-9_]+)|(\".*\")|([ \t\n]*))*;	{
/* LONGTEXTLINE // TextTyp = "A Text" HHH_XXX "A Text" ZZZ_TTT ... */
	WorkOnTokenSet( LONGTEXTLINE, yytext );
}

\".*\" {
/* TEXT // "A Text" */
	WorkOnTokenSet( TEXT, yytext );
}

"{"[ \t]*\\{0,1}	{
/* LEVELUP */
	WorkOnTokenSet( LEVELUP, yytext );
}

"}"[ \t]*;([ \t]*\\){0,1}	{
/* LEVELDOWN */
	WorkOnTokenSet( LEVELDOWN, yytext );
}

[a-zA-Z0-9_]+[ \t]*"="[ \t]*"MAP_APPFONT"[ \t]*"(".+")".*	{
/* APPFONTMAPPING  Typ = MAP_APPFONT( ... ) */
	WorkOnTokenSet( APPFONTMAPPING, yytext );
}

[ \t]*[a-zA-Z0-9_]+[ \t]*=[ \t]*[0123456789]{1,5}[ \t]*";"{0,1}\\{0,1} {
/* TEXTREFID // TextTyp = 12345 */
	WorkOnTokenSet( TEXTREFID, yytext );
}

[a-zA-Z0-9_]+[ \t]*"="[\t ]*([ \t]*"//".*\n)*.*	| 
[a-zA-Z0-9_]+[ \t]*"=".*	{ 
/* ASSIGNMENT  Typ = ...  */
 WorkOnTokenSet( ASSIGNMENT, yytext );
}



[a-zA-Z0-9_]+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*){0,1}"="[ \t]*(\\[ \t]*){0,1}\n{0,1}[ \t]*"{"[ \t]*(\\[ \t]*){0,1}\n{0,1}[ \t]*"<"	{
/* LISTASSIGNMENT  Typ [ ... ] = ... */
	WorkOnTokenSet( LISTASSIGNMENT, yytext );
}

"StringList"+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*){0,1}"="[ \t]*(\\[ \t]*){0,1}\n{0,1}[ \t]*"{"[ \t]*(\\[ \t]*){0,1}\n{0,1}[ \t]*	{
/* LISTASSIGNMENT  Typ [ ... ] = ... */
	WorkOnTokenSet( LISTASSIGNMENT, yytext );
}

"UIEntries"[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*){0,1}"="[ \t]*(\\[ \t]*){0,1}\n{0,1}[ \t]*"{"	{
/* UIENTRIES */
	WorkOnTokenSet( UIENTRIES, yytext );
}

"<"{0,1}[ \t]*L{0,1}\".*\".*">" {
/* LISTTEXT */
	WorkOnTokenSet( LISTTEXT, yytext );
}

[ \t]*"#define"[ \t]+[a-zA-Z0-9_]+.*"\\"	{
/* RSCDEFINE  #define ... */
	WorkOnTokenSet( RSCDEFINE, yytext );
}

[ \t]*"#define"[ \t]+[a-zA-Z0-9_]+.+ {
/* #define ... */
	WorkOnTokenSet( NORMDEFINE, yytext );
}

"\\" {
/* RSCDEFINELEND */
	WorkOnTokenSet( RSCDEFINELEND, yytext );
}

[a-zA-Z0-9_]+[ \t]*; {
/* allowed other tokens like "49 ;" or "SFX_... ;" */
	WorkOnTokenSet( ANYTOKEN, yytext );
}

.	{
	WorkOnTokenSet( UNKNOWNCHAR, yytext );
/*	YYWarning( "Unknown Char" ); */
}

"{"{0,1}[ \t]*\".*\"[ \t]*";"[ \t]*"}" {
/* _LISTTEXT */
	WorkOnTokenSet( _LISTTEXT, yytext );
}

%%

/*****************************************************************************/
int	yywrap(void)
/*****************************************************************************/
{
	FILE *pFile;
	pFile = GetNextFile();
	if ( pFile ) {
		yyin = pFile;
		yylineno = 0;
		return 0;
	}

   /* end of input reached */
	return 1;
}

/*****************************************************************************/
void YYWarning( char *s )
/*****************************************************************************/
{
	/* write warning to stderr */
	fprintf( stderr, "Warning: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
}

/*****************************************************************************/
void yyerror( char *s )
/*****************************************************************************/
{
	/* write error to stderr */
	fprintf( stderr, "Error: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
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
        fprintf( stdout, "\nTransEx 3.1 Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.\n" );
	    fprintf( stdout, "========================================================================\n" );
    }

    if ( !pOutput ) {
		fprintf( stdout, "Syntax:TRANSEX[-p Prj][-r PrjRoot]-i FileIn...[-o FileOut][-m DataBase][-e][-b][-u][-NOUTF8][-L l1,l2,...]\n" );
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
		return 1;
	}

	pFile = GetNextFile();
	if ( !pFile )
		return 1;
    
   	yyin = pFile;

	/* create global instance of class Export */
	InitExport( pOutput , pFile );

	/* start parser */
   	yylex();
   	Close();

	/* get error info. and end export */
	nRetValue = GetError();
	EndExport();

	if( !isQuiet() ) fprintf( stdout, "\n===================================\n\n" ); 

	/* return error level */
	return nRetValue;
}
