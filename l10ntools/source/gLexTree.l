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

#include "gL10nMem.hxx"
#include "gConvTree.hxx"

#define IMPLptr convert_gen::mcImpl
#define LOCptr ((convert_tree *)convert_gen::mcImpl)

/* enlarge token buffer to tokenize whole std::strings */
#undef  YYLMAX
#define YYLMAX 64000

/* change reader function (input) to our own version */
#define YY_INPUT(buf,result,max_size) {int xres; IMPLptr->lexRead(buf, &xres, max_size); result = xres;}

#define yytext_ptr treetext_ptr
#define YY_NO_UNISTD_H 1
}

%{
static void yy_flex_strncpy (char* s1, yyconst char * s2, int n )
{
    register int i;
    for ( i = 0; i < n; ++i )
        s1[i] = s2[i];
}
%}





/*****************************   O P T I O N S   *****************************/
/* 8bit               --> allow 8bit characters in the input stream          */
/* noyywrap           --> yywrap is not called (single file scan)            */
/* never-interactive  --> no check for console output                        */
/* prefix=            --> yyFlexLexer change name                            */
/* --- The following options are for future use (maybe)                      */
/* yyclass=           --> subClass yyFlexLexer to allow own functions        */
/* c++                --> generate C++ classes                               */
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
    LOCptr->setState(yytext, convert_tree::STATE_TAG_VALUE, convert_tree::STATE_VAL_NONE);
}



"id=" {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NONE, convert_tree::STATE_VAL_ID);
}



"application=" {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NONE, convert_tree::STATE_VAL_APPL);
}



"title=" {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NONE, convert_tree::STATE_VAL_TITLE);
}



"<help_section " {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_HELPSEC, convert_tree::STATE_VAL_NONE);
}



"<node " {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_NODE, convert_tree::STATE_VAL_NONE);
}



"<topic " {
    LOCptr->setState(yytext, convert_tree::STATE_TAG_TOPIC, convert_tree::STATE_VAL_NONE);
}



"</topic" {
    LOCptr->setValue(yytext);
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
    char *txt = NULL;
    //  yy_flex_strlen(txt);
    yyunput(0, txt);
}
