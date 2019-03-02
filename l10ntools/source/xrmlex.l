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
 * lexer for parsing xml-property source files (*.xml)
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
#include <xrmlex.hxx>
#include <stdlib.h>
#include <stdio.h>

#include <sal/main.h>

#define YY_NO_UNISTD_H

static int bText=0;
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



"<!--"  {
    int c1 = 0, c2 = 0;
    int c3 = yyinput();
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
        "Error: \"%s\" in line %d: \"%s\"\n", s, yylineno, yytext  );
    SetError();
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    /* error level */
    int nRetValue = 0;
    FILE *pFile;

    if ( !GetOutputFile( argc, argv ) )
    {
        return 1;
    }
    pFile = GetXrmFile();
    InitXrmExport( getFilename() );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
