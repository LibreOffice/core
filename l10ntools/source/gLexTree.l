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
#include "gConvTree.hxx"

#define LOCptr ((convert_tree *)convert_gen::mcImpl)
#define YYLMAX 64000
#define YY_INPUT(buf,result,max_size) LOCptr->lexRead(buf, &result, max_size)
#define YY_NO_UNISTD_H 1
#define yytext_ptr treetext_ptr
#define yy_flex_strncpy convert_gen::lexStrncpy 
}



%option prefix="tree" 8bit noyywrap never-interactive
%array
%p 24000
%e 1200
%n 500

SP [ \t]*
IDENT [\.a-zA-Z0-9_-]+
%%


"\"" {
    int j;

    // build current "..."
    for (j = -1; (yytext[++j] = yyinput()) != '\"';)
        if (yytext[j] == '\\')
            yytext[++j] = yyinput();

    yytext[j+1] = '\0';
    LOCptr->setString(yytext);
}



\> {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_VALUE, convert_tree::STATE_VAL_NONE, NULL);
}



"id=" {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NONE, convert_tree::STATE_VAL_ID, NULL);
}



"application=" {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NONE, convert_tree::STATE_VAL_APPL, NULL);
}



"title=" {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NONE, convert_tree::STATE_VAL_TITLE, NULL);
}



"<help_section " {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_HELPSEC, convert_tree::STATE_VAL_NONE, "help_section");
}



"<node " {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NODE, convert_tree::STATE_VAL_NONE, "node");
}



"<topic " {
    //NOT NOW LOCptr->setState(yytext, convert_tree::STATE_TAG_TOPIC, convert_tree::STATE_VAL_NONE, NULL);
}



"</topic" {
    // NOT NOW LOCptr->setValue(yytext);
}



.|\n {
    LOCptr->copySourceSpecial(yytext, 0);

    // Just to please compiler.
    if (false)
        REJECT;
}



%%



void tree_dummyJustForCompiler()
{
    yyunput(0, NULL);
}
