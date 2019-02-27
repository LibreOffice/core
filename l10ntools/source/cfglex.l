/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

%{

/*
 * lexer for parsing cfg source files
 */

#include <sal/config.h>

/* enlarge token buffer to tokenize whole strings */
#undef YYLMAX
#define YYLMAX 64000

/* to enable debug output define LEXDEBUG */
#define LEXDEBUG        1
#ifdef LEXDEBUG
#define OUTPUT  fprintf
#else
#define OUTPUT(Par1,Par2);
#endif

/* table of possible token ids */
#include <tokens.h>
#include <stdlib.h>
#include <stdio.h>

#include <sal/main.h>

#include <cfglex.hxx>

#define YY_NO_UNISTD_H

static int yycolumn = 1;
#define YY_USER_ACTION yycolumn += yyleng;

static int bText=0;
%}

%option yylineno
%option never-interactive

%p 24000
%e 1200
%n 500

%%

\<[^\>]*"xml:lang="\""x-no-translate"\"[^\<]*\/\>   {
    bText = 0;
    workOnTokenSet( CFG_TOKEN_NO_TRANSLATE, yytext );
}

\<.*\/\> {
    bText = 0;
    workOnTokenSet( ANYTOKEN, yytext );
}

\<[^\>]*"xml:lang="\".*\"[^\<]*\>   {
    bText = 1;
    workOnTokenSet( CFG_TEXT_START, yytext );
}


\<[^\/\!][^\>]*\>   {
    bText = 0;
    workOnTokenSet( CFG_TAG, yytext );
}

"<!"DOCTYPE[^\>]*\> {
    bText = 0;
    workOnTokenSet( CFG_TAG, yytext );
}


\<\!\-\-    {
    char c1 = 0, c2 = 0;
    int c3 = yyinput();
    char pChar[2];
    pChar[1] = 0x00;
    pChar[0] = c3;

    workOnTokenSet( COMMENT, yytext );
    workOnTokenSet( COMMENT, pChar );

    for(;;) {
        if ( c3 == EOF )
            break;
        if ( c1 == '-' && c2 == '-' && c3 == '>' )
            break;
        c1 = c2;
        c2 = c3;
        c3 = yyinput();

        pChar[0] = c3;
        workOnTokenSet( COMMENT, pChar );
    }
}

\<\/[^\>]*\> {
    bText = 0;
    workOnTokenSet( CFG_CLOSETAG, yytext );
}

\<[^\>\!]*\> {
    bText = 0;
    if ( yytext[ 1 ] == '!' && yytext[ 2 ] == '-' && yytext[ 3 ] == '-' )
        workOnTokenSet( COMMENT, yytext );
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
int yywrap(void)
/*****************************************************************************/
{
    return 1;
}

/*****************************************************************************/
void yyerror ( const char *s )
/*****************************************************************************/
{
    /* write error to stderr */
    fprintf( stderr,
        "Error: \"%s\" in line %d, column %d: \"%s\"\n", s, yylineno, yycolumn, yytext  );
    exit(EXIT_FAILURE);
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    yyin = init(argc, argv);
    yylex();
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
