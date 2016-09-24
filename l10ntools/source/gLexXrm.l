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
#include "gConvXrm.hxx"

#define LOCptr ((convert_xrm *)convert_gen::mcImpl)
#define YYLMAX 64000
#define YY_INPUT(buf,result,max_size) LOCptr->lexRead(buf, &result, max_size)
#define YY_NO_UNISTD_H 1
#define yytext_ptr xrmtext_ptr
#define yy_flex_strncpy convert_gen::lexStrncpy 
}



%option prefix="xrm" 8bit noyywrap never-interactive
%array
%p 24000
%e 1200
%n 500

IDENT [a-zA-Z0-9_-]*
NAME  \"{IDENT}\"
SP    [ \t]*
%%



"id"{SP}"="{SP}{NAME} {
    LOCptr->setId(yytext);
}



"lang"{SP}"="{SP}{NAME} {
    LOCptr->setLang(yytext);
}


"<h"[0-9]" " |
"<p "        {
    LOCptr->setTag(yytext);
}



"</h"[0-9]">"[ \t\r]*[\n] |
"</p>"[ \t\r]*[\n]        {
    LOCptr->stopCollectData(yytext);
}



">" {
    LOCptr->startCollectData(yytext);
}



.|\n {
    LOCptr->copySource(yytext, LOCptr->mbNoCollectingData);

    // Just to please compiler.
    if (false)
        REJECT;
}


%%

void xrm_dummyJustForCompiler()
{
    char *txt = NULL;
    //  yy_flex_strlen(txt);
    yyunput(0, txt);
}
