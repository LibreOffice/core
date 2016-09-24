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
#include "gConvXcu.hxx"

#define LOCptr ((convert_xcu *)convert_gen::mcImpl)
#define YYLMAX 64000
#define YY_INPUT(buf,result,max_size) LOCptr->lexRead(buf, &result, max_size)
#define YY_NO_UNISTD_H 1
#define yytext_ptr xcutext_ptr
#define yy_flex_strncpy convert_gen::lexStrncpy 
}



%option prefix="xcu" 8bit noyywrap never-interactive
%array
%p 24000
%e 1200
%n 500

SPACE [ \t]*
NAME  .*"oor:name="\"[^\"]+\"{SPACE}
FIN   [^/>]*">"
%%



"component-data" {
    LOCptr->addLevel();
    LOCptr->pushKey("");
    LOCptr->copySource(yytext, false);
}



"<oor:component-data"{NAME} {
    LOCptr->addLevel();
    LOCptr->pushKey("");
    LOCptr->pushKey(yytext);
}



"<prop"{NAME}{FIN} |
"<node"{NAME}{FIN} {
    LOCptr->pushKey(yytext);
}



"</oor:component-data" |
"</prop"               |
"</node"               {
    LOCptr->popKey(yytext);
}


"<value xml:lang="\"[^\"]+\"[^>]*">" {
    LOCptr->startCollectData(yytext);
}



"</value>" {
    LOCptr->stopCollectData(yytext);
}



"&amp;"  |
"&apos;" |
"&gt;"   |
"&lt;"   |
"&quot;" {
    LOCptr->copySpecial(yytext);
}



({SPACE}\n{SPACE})+ {
    LOCptr->copyNL(yytext);
}



. {
    LOCptr->copySource(yytext, LOCptr->mbNoCollectingData);

    // Just to please compiler.
    if (false)
        REJECT;
}



%%



void xcu_dummyJustForCompiler()
{
    char *txt = NULL;
    //  yy_flex_strlen(txt);
    yyunput(0, txt);
}
