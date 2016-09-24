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
#include "gConvPo.hxx"

#define LOCptr ((convert_po *)convert_gen::mcImpl)
#define YYLMAX 64000
#define YY_INPUT(buf,result,max_size) LOCptr->lexRead(buf, &result, max_size)
#define YY_NO_UNISTD_H 1
#define yytext_ptr potext_ptr
#define yy_flex_strncpy convert_gen::lexStrncpy 
}



%option prefix="po" 8bit noyywrap never-interactive
%array
%p 24000
%e 1200
%n 500

SPACE [ \t]*
%%


"\"" {
    char c, buildValue[8000];
    int j, i, iLineCnt;

    // loop across multiple "..." and combine them, while keeping the source
    for (j = -1, iLineCnt = i = 0; (c = yytext[i]) == '\"';) {
        // build current "..."
        for (; (buildValue[++j] = yytext[++i] = c = yyinput()) != '\"';)
            if (yytext[i] == '\\')
                buildValue[++j] = yytext[++i] = yyinput();
        --j;

        if (!LOCptr->mbExpectId && !LOCptr->mbExpectStr)
            break;

        // skip rest of line
        for (; (c = yytext[++i] = yyinput()) == ' ' || yytext[i] == '\t' || yytext[i] == '\n';)
            if (yytext[i] == '\n')
                ++iLineCnt;
    }

    if (LOCptr->mbExpectId || LOCptr->mbExpectStr) {
        if (yytext[i] != EOF) {
            unput(yytext[i]);
        }
        buildValue[j+1] = '\0';
        LOCptr->setValue(buildValue, iLineCnt);
    }
}


"#,"{SPACE}.*[fF][uU][zZ][zZ][yY] {
    LOCptr->setFuzzy();
}


"#:".* {
    LOCptr->setKey(yytext);
}


[mM][sS][gG][cC][tT][xX][tT].* |
"#*".*                         |
"# ".*                         |
"#~".*                         {
    // special comment, just skip
}


[mM][sS][gG][iI][dD]{SPACE} {
    LOCptr->setMsgId();
}


[mM][sS][gG][sS][tT][rR]{SPACE} {
    LOCptr->setMsgStr();
}


\n {
    LOCptr->handleNL();

    // Just to please compiler.
    if (false)
        REJECT;
}


.|\n {
}
%%


void dummyJustForCompiler()
{
    char *txt = NULL;
    //  yy_flex_strlen(txt);
    yyunput(0, txt);
}
