/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
%top{
#include <string>
#include <vector>
using namespace std;

#include "gL10nMem.hxx"
#include "gConvSrc.hxx"

#define LOCptr ((convert_src *)convert_gen::mcImpl)
#define YYLMAX 64000
#define YY_INPUT(buf,result,max_size) LOCptr->lexRead(buf, &result, max_size)
#define YY_NO_UNISTD_H 1
#define yytext_ptr srctext_ptr
#define yy_flex_strncpy convert_gen::lexStrncpy 
}



%option prefix="src" 8bit noyywrap never-interactive
%array
%p 24000
%e 1200
%n 500
%x CMD ID LANG LANG2 LANG3 LANG4

SPACE [ \t]*
IDENT ([(a-zA-Z0-9_][ a-zA-Z0-9_\-\+\*(,&]*[a-zA-Z0-9)_]|[a-zA-Z0-9_])
SUF   [ \t\r\n\\]+
%%
f


"/*" {
    int i = 1;
    for (;;) {
        while (yytext[i] != '*')
            yytext[++i] = yyinput();
        yytext[++i] = yyinput();
        if (yytext[i] == '/')
            break;
    }
    yytext[i+1] = '\0';

    LOCptr->copySource(yytext);
}



"//".* {
    LOCptr->copySource(yytext);
}



"{" {
    LOCptr->copySource(yytext);
    LOCptr->startBlock();
}



"}"{SPACE}";"* {
    LOCptr->copySource(yytext);
    LOCptr->stopBlock();
}



[iI][dD][eE][nN][tT][iI][fF][iI][eE][rR] {
    LOCptr->copySource(yytext);
	BEGIN(ID);
}



<ID>{SPACE}[=]*{SPACE} {
    LOCptr->copySource(yytext);
}



<ID>{IDENT} {
    LOCptr->setName(LOCptr->copySource(yytext), true);
    BEGIN(INITIAL);
}



[tT][eE][xX][tT] {
    LOCptr->copySource(yytext);
	BEGIN(LANG);
}



<LANG>{SPACE}"["{SPACE} {
    LOCptr->copySource(yytext);
	BEGIN(LANG2);
}



<LANG>{SPACE}"=" {
    LOCptr->copySource(yytext);
	BEGIN(INITIAL);
}



<LANG2>{IDENT} {
    if (LOCptr->setLang(LOCptr->copySource(yytext))) {
	    BEGIN(LANG3);
	}
	else {
	    BEGIN(INITIAL);
	}
}



<LANG3>{SPACE}"]"{SPACE}"="{SPACE} {
    LOCptr->copySource(yytext);
    BEGIN(LANG4);
}



<LANG4>"\"" {
    char buildValue[8000];
    int j, i;

    // loop across multiple "..." and combine them, while keeping the source
    buildValue[0] = yytext[0];
    for (j = i = 0; yytext[i] != ';' ;) {
        // build current "..."
        for (; (buildValue[++j] = yytext[++i] = yyinput()) != '\"';)
            if (yytext[i] == '\\')
                buildValue[++j] = yytext[++i] = yyinput();
        --j;
        for (; (yytext[++i] = yyinput()) != ';' && yytext[i] != '\"';) ;
    }
    yytext[++i]   =
    buildValue[j+1] = '\0';
    LOCptr->setValue(LOCptr->copySource(buildValue));
	BEGIN(INITIAL);
}


"#define"                                                                |
[cC][oO][nN][tT][rR][oO][lL]                                             |
[fF][iI][xX][eE][dD][tT][eE][xX][tT]                                     |
[mM][eE][nN][uU]                                                         |
[mM][eE][nN][uU][iI][tT][eE][mM]                                         |
[rR][eE][sS][oO][uU][rR][cC][eE]                                         |
[sS][fF][xX][sS][tT][yY][lL][eE][fF][aA][mM][iI][lL][iI][eS][sS]         |
[sS][fF][xX][sS][tT][yY][lL][eE][fF][aA][mM][iI][lL][yY][iI][tT][eE][mM] |
[sS][tT][rR][iI][nN][gG]                                                 |
[wW][iI][nN][dD][oO][wW]                                                 {
    LOCptr->setCmd(LOCptr->copySource(yytext));
    BEGIN(CMD);
}



<CMD>{IDENT} {
    LOCptr->setName(LOCptr->copySource(yytext),false);
    BEGIN(INITIAL);
}



<CMD>{SPACE} {
    LOCptr->copySource(yytext);
}



<CMD>{SUF} {
    LOCptr->copySource(yytext);
    BEGIN(INITIAL);
}


\n {
    LOCptr->copySource(yytext);
    LOCptr->endMacro();
}



\\[\r]*\n {
    LOCptr->copySource(yytext);
    LOCptr->defMacro();
}


[dD][iI][aA][lL][oO][gG][cC][oO][nN][tT][rR][oO][lL]  |
"#define".*[^\\][\r]*\n                               |
"#include".*                                          |
.                                                     {
    LOCptr->copySource(yytext);

    // Just to please compiler.
    if (false)
        REJECT;
}

%%



#if 0
PRE   ^[ \t]*
SUFT  [ \t\r\n\[]
KEYID [a-zA-Z0-9_-]+
%%



{PRE}"<"{SUF} {
    yyless(strlen(yytext)-1);
    LOCptr->setListItem(yytext, true);
}



">"{SPACE}";"{SUF} {
    yyless(strlen(yytext)-1);
    LOCptr->setListItem(yytext, false);
}



{PRE}[bB][iI][tT][mM][aA][pP]{SUF}                                                 |
{PRE}[bB][uU][tT][tT][oO][nN][iI][mM][aA][gG][eE]{SUF}                             |
{PRE}[cC][aA][nN][cC][eE][lL][bB][uU][tT][tT][oO][nN]{SUF}                         |
{PRE}[cC][hH][eE][cC][kK][bB][oO][xX]{SUF}                                         |
{PRE}[cC][oO][mM][bB][oO][bB][oO][xX]{SUF}                                         |
{PRE}[dD][oO][cC][kK][iI][nN][gG][wW][iI][nN][dD][oO][wW]{SUF}                     |
{PRE}[eE][dD][iI][tT]{SUF}                                                         |
{PRE}[eE][rR][rR][oO][rR][bB][oO][xX]{SUF}                                         |
{PRE}[fF][iI][xX][eE][dD][lL][iI][nN][eE]{SUF}                                     |
{PRE}[fF][lL][oO][aA][tT][iI][nN][gG][wW][iI][nN][dD][oO][wW]{SUF}                 |
{PRE}[gG][rR][oO][uU][pP][bB][oO][xX]{SUF}                                         |
{PRE}[hH][eE][lL][pP][bB][uU][tT][tT][oO][nN]{SUF}                                 |
{PRE}[iI][dD][lL][iI][sS][tT]{SUF}                                                 |
{PRE}[iI][mM][aA][gG][eE]{SUF}                                                     |
{PRE}[iI][mM][aA][gG][eE][lL][iI][sS][tT]{SUF}                                     |
{PRE}[iI][mM][aA][gG][eE][bB][uU][tT][tT][oO][nN]{SUF}                             |
{PRE}[iI][mM][aA][gG][eE][rR][aA][dD][iI][oO][bB][uU][tT][tT][oO][nN]{SUF}         |
{PRE}[iI][nN][fF][oO][bB][oO][xX]{SUF}                                             |
{PRE}[lL][iI][sS][tT][bB][oO][xX]{SUF}                                             |
{PRE}[mM][eE][nN][uU][bB][uU][tT][tT][oO][nN]{SUF}                                 |
{PRE}[mM][eE][sS][sS][bB][oO][xX]{SUF}                                             |
{PRE}[mM][eE][tT][rR][iI][cC][fF][iI][eE][lL][dD]{SUF}                             |
{PRE}[mM][oO][dD][aA][lL][dD][iI][aA][lL][oO][gG]{SUF}                             |
{PRE}[mM][oO][dD][eE][lL][eE][sS][sS][dD][iI][aA][lL][oO][gG]{SUF}                 |
{PRE}[mM][oO][rR][eE][bB][uU][tT][tT][oO][nN]{SUF}                                 |
{PRE}[mM][uU][lL][tT][iI][lL][iI][nN][eE][eE][dD][iI][tT]{SUF}                     |
{PRE}[nN][uU][mM][eE][rR][iI][cC][fF][iI][eE][lL][dD]{SUF}                         |
{PRE}[oO][kK][bB][uU][tT][tT][oO][nN]{SUF}                                         |
{PRE}[pP][aA][gG][eE][iI][tT][eE][mM]{SUF}                                         |
{PRE}[pP][aA][gG][eE][lL][iI][sS][tT]{SUF}                                         |
{PRE}[pP][uU][sS][hH][bB][uU][tT][tT][oO][nN]{SUF}                                 |
{PRE}[qQ][uU][eE][rR][yY][bB][oO][xX]{SUF}                                         |
{PRE}[rR][aA][dD][iI][oO][bB][uU][tT][tT][oO][nN]{SUF}                             |
{PRE}[sS][pP][iI][nN][fF][iI][eE][lL][dD]{SUF}                                     |
{PRE}[sS][tT][rR][iI][nN][gG][aA][rR][rR][aA][yY]{SUF}                             |
{PRE}[tT][aA][bB][cC][oO][nN][tT][rR][oO][lL]{SUF}                                 |
{PRE}[tT][aA][bB][dD][iI][aA][lL][oO][gG]{SUF}                                     |
{PRE}[tT][aA][bB][pP][aA][gG][eE]{SUF}                                             |
{PRE}[tT][iI][mM][eE][fF][iI][eE][lL][dD]{SUF}                                     |
{PRE}[tT][oO][oO][lL][bB][oO][xX]{SUF}                                             |
{PRE}[tT][oO][oO][lL][bB][oO][xX][iI][tT][eE][mM]{SUF}                             |
{PRE}[tT][rR][iI][sS][tT][aA][tT][eE][bB][oO][xX]{SUF}                             |
{PRE}[wW][aA][rR][nN][iI][nN][gG][bB][oO][xX]{SUF}                                 |
{PRE}[wW][oO][rR][kK][wW][iI][nN][dD][oO][wW]{SUF}                                 {



{PRE}[hH][eE][lL][pP][iI][dD]{SUF} {
    yyless(strlen(yytext)-1);
    LOCptr->setId(yytext, false);
}



{PRE}[sS][tT][yY][lL][eE][fF][aA][mM][iI][lL][yY]{SUF}  |



{PRE}[cC][uU][sS][tT][oO][mM][uU][nN][iI][tT][tT][eE][xX][tT]{SUFT} |
{PRE}[hH][eE][lL][pP][tT][eE][xX][tT]{SUFT}                         |
{PRE}[mM][eE][sS][sS][aA][gG][eE]{SUFT}                             |
{PRE}[qQ][uU][iI][cC][kK][hH][eE][lL][pP][tT][eE][xX][tT]{SUFT}     |
{PRE}[tT][eE][xX][tT]{SUFT}                                         |
{PRE}[tT][iI][tT][lL][eE]{SUFT}                                     {
    yyless(strlen(yytext)-1);
    LOCptr->setText(yytext);
}



{PRE}[fF][iI][lL][tT][eE][rR][lL][iI][sS][tT]{SUFT}                     |
{PRE}[iI][tT][eE][mM][lL][iI][sS][tT]{SUFT}                             |
{PRE}[sS][tT][yY][lL][eE][fF][aA][mM][iI][lL][yY][lL][iI][sS][tT]{SUF}  {
    yyless(strlen(yytext)-1);
    LOCptr->setList(yytext);
}



{PRE}[pP][aA][iI][rR][eE][dD][lL][iI][sS][tT]{SUFT}  |
{PRE}[sS][tT][rR][iI][nN][gG][lL][iI][sS][tT]{SUFT}  {
    yyless(strlen(yytext)-1);
    LOCptr->setList(yytext);
}



{KEYID} {
    LOCptr->setName(yytext);
}

#endif


void src_dummyJustForCompiler()
{
    char *txt = NULL;
    //  yy_flex_strlen(txt);
    yyunput(0, txt);
}
