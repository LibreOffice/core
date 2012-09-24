
%{
/*
 * lexer for parsing resource source files (*.src)
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
extern "C" FILE * init(int, char **);
extern "C" int SetError();
extern "C" int GetError();
extern "C" void Close();

/* forwards */
void YYWarning();
%}

%option yylineno
%option never-interactive

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
	char c1 = 0,c2 = yyinput();
	char pChar[2];
	pChar[1] = 0x00;
	pChar[0] = c2;

	WorkOnTokenSet( COMMENT, yytext );
	WorkOnTokenSet( COMMENT, pChar );
	for(;;) {
		if ( c2 == EOF )
			break;
		if ( c1 == '*' && c2 == '/' )
			break;
		c1 = c2;
		c2 = yyinput();
		pChar[0] = c2;
		WorkOnTokenSet( COMMENT, pChar );
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
[a-zA-Z]+[ \t]+[^={;\n]+\n?([ \t]*"//".*\n)*[ \t]*"{"	{
/* RESOURCE // String TTT_XX ... */
	WorkOnTokenSet( RESOURCE, yytext );
}

^[\t ]*[a-zA-Z_]+[\t ]*"\\"?[\t ]*\n?[ \t]*"{"[\t ]*"\\"?	{
/* SMALRESOURCE // String ... */
	WorkOnTokenSet( SMALRESOURCE, yytext );
}

[\t ]*[a-zA-Z0-9_]+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*)?=[ \t]*L?\".*\".*\n?	{
/* TEXTLINE // TextTyp = "A Text" */
	WorkOnTokenSet( TEXTLINE, yytext );
}

[\t ]*[a-zA-Z0-9_]+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*)?(\n[ \t]*)?=([ \t]*\n)?(([a-zA-Z0-9_]+)|(\".*\")|([ \t\n]*))*\".*\"(([a-zA-Z0-9_]+)|(\".*\")|([ \t\n]*))*;	{
/* LONGTEXTLINE // TextTyp = "A Text" HHH_XXX "A Text" ZZZ_TTT ... */
	WorkOnTokenSet( LONGTEXTLINE, yytext );
}

\".*\" {
/* TEXT // "A Text" */
	WorkOnTokenSet( TEXT, yytext );
}

"{"[ \t]*\\?	{
/* LEVELUP */
	WorkOnTokenSet( LEVELUP, yytext );
}

"}"[ \t]*;([ \t]*\\)?	{
/* LEVELDOWN */
	WorkOnTokenSet( LEVELDOWN, yytext );
}

[a-zA-Z0-9_]+[ \t]*"="[ \t]*"MAP_APPFONT"[ \t]*"(".+")".*	{
/* APPFONTMAPPING  Typ = MAP_APPFONT( ... ) */
	WorkOnTokenSet( APPFONTMAPPING, yytext );
}

[ \t]*[a-zA-Z0-9_]+[ \t]*=[ \t]*[0123456789]{1,5}[ \t]*";"?\\? {
/* TEXTREFID // TextTyp = 12345 */
	WorkOnTokenSet( TEXTREFID, yytext );
}

[a-zA-Z0-9_]+[ \t]*"="[\t ]*([ \t]*"//".*\n)*.*	|
[a-zA-Z0-9_]+[ \t]*"=".*	{
/* ASSIGNMENT  Typ = ...  */
 WorkOnTokenSet( ASSIGNMENT, yytext );
}



[a-zA-Z0-9_]+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*)?"="[ \t]*(\\[ \t]*)?\n?[ \t]*"{"[ \t]*(\\[ \t]*)?\n?[ \t]*"<"	{
/* LISTASSIGNMENT  Typ [ ... ] = ... */
	WorkOnTokenSet( LISTASSIGNMENT, yytext );
}

"StringList"+[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*)?"="[ \t]*(\\[ \t]*)?\n?[ \t]*"{"[ \t]*(\\[ \t]*)?\n?[ \t]*	{
/* LISTASSIGNMENT  Typ [ ... ] = ... */
	WorkOnTokenSet( LISTASSIGNMENT, yytext );
}

"UIEntries"[ \t]*("["[ \t]*[a-zA-Z0-9_\-]+[ \t]*"]"[ \t]*)?"="[ \t]*(\\[ \t]*)?\n?[ \t]*"{"	{
/* UIENTRIES */
	WorkOnTokenSet( UIENTRIES, yytext );
}

"<"?[ \t]*L?\".*\".*">" {
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

"{"?[ \t]*\".*\"[ \t]*";"[ \t]*"}" {
/* _LISTTEXT */
	WorkOnTokenSet( _LISTTEXT, yytext );
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
	fprintf( stderr, "Warning: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
}

/*****************************************************************************/
void yyerror( const char *s )
/*****************************************************************************/
{
	/* write error to stderr */
	fprintf( stderr, "Error: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
	SetError();
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    int e;
    yyin = init(argc, argv);
    yylex();
    e = GetError();
    Close();
    return EXIT_SUCCESS;
}
