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

int yycolumn = 1;
#define YY_USER_ACTION yycolumn += yyleng;

/* external functions (C++ code, declared as extren "C" */
extern void workOnTokenSet( int, char* );
extern FILE * init(int, char **);

int bText=0;
%}

%p 24000
%e 1200
%n 500

%%

\<[^\>]*"xml:lang="\""x-no-translate"\"[^\<]*\/\>	{
	bText = 0;
	workOnTokenSet( CFG_TOKEN_NO_TRANSLATE, yytext );
}

\<.*\/\> {
	bText = 0;
	workOnTokenSet( ANYTOKEN, yytext );
}

\<[^\>]*"xml:lang="\".*\"[^\<]*\>	{
	bText = 1;
	workOnTokenSet( CFG_TEXT_START, yytext );
}


\<[^\/\!][^\>]*\>	{
	bText = 0;
	workOnTokenSet( CFG_TAG, yytext );
}

"<!"DOCTYPE[^\>]*\>	{
	bText = 0;
	workOnTokenSet( CFG_TAG, yytext );
}


\<\!\-\-	{
	char c1 = 0, c2 = 0, c3 = input();
	char pChar[2];
	pChar[1] = 0x00;
	pChar[0] = c3;

	workOnTokenSet( COMMEND, yytext );
	workOnTokenSet( COMMEND, pChar );

	for(;;) {
		if ( c3 == EOF )
			break;
		if ( c1 == '-' && c2 == '-' && c3 == '>' )
			break;
		c1 = c2;
		c2 = c3;
		c3 = input();
		pChar[0] = c3;
		workOnTokenSet( COMMEND, pChar );
	}
}

\<\/[^\>]*\> {
	bText = 0;
	workOnTokenSet( CFG_CLOSETAG, yytext );
}

\<[^\>\!]*\> {
	bText = 0;
	if ( yytext[ 1 ] == '!' && yytext[ 2 ] == '-' && yytext[ 3 ] == '-' )
		workOnTokenSet( COMMEND, yytext );
	else
		workOnTokenSet( CFG_UNKNOWNTAG, yytext );
}

.|\n {
    yycolumn = 1;
	if ( bText == 1 )
		workOnTokenSet( CFG_TEXTCHAR, yytext );
	else
		workOnTokenSet( UNKNOWNCHAR, yytext );
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
		"Warning: \"%s\" in line %d, column %d: \"%s\"\n", s, yylineno, yycolumn, yytext  );
}

/*****************************************************************************/
#ifdef GCC
void yyerror ( const char *s, ... )
#else
void yyerror ( const char *s )
#endif
/*****************************************************************************/
{
	/* write error to stderr */
	fprintf( stderr,
		"Error: \"%s\" in line %d, column %d: \"%s\"\n", s, yylineno, yycolumn, yytext  );
	exit(EXIT_FAILURE);
}

int
#if defined WNT
_cdecl
#endif
main(int argc, char ** argv) {
    yyin = init(argc, argv);
    yylex();
    return EXIT_SUCCESS;
}
