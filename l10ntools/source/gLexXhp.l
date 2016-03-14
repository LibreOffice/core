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


/***************   O V E R W R I T I N G   F U N C T I O N S   ***************/
%top{
#include <string>
#include <vector>

#include "gL10nMem.hxx"
#include "gConvXhp.hxx"

#define IMPLptr convert_gen_impl::mcImpl
#define LOCptr ((convert_xhp *)convert_gen_impl::mcImpl)

/* enlarge token buffer to tokenize whole std::strings */
#undef  YYLMAX
#define YYLMAX 64000

/* change reader function (input) to our own version */
#define YY_INPUT(buf,result,max_size) {int xres; IMPLptr->lexRead(buf, &xres, max_size); result = xres;}

#define yytext_ptr xhptext_ptr
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
%option prefix="xhp" 8bit noyywrap never-interactive
%array
%p 24000
%e 1200
%n 500



/***********************   H E L P E R   M A C R O S   ***********************/
SP [ \t]*
IDENT [\.a-zA-Z0-9_-]+
/*******************   R U L E S   D E F I N I T I O N S   *******************/
%%



\" {
  LOCptr->setString(yytext);
}



\< {
  LOCptr->openTag(yytext);
}



\/\> {
  LOCptr->closeTagNOvalue(yytext);
}



\> {
  LOCptr->closeTag(yytext);
}



"id="{SP}\"{SP}{IDENT}{SP}\" {
  LOCptr->setId(yytext);
}



"xml-lang="{SP}\"{SP}{IDENT}{SP}\" {
  LOCptr->setLang(yytext);
}



"oldref="{SP}\"{SP}{IDENT}{SP}\" {
  LOCptr->setRef(yytext);
}


"<title "     |
"<bookmark "  |
"<paragraph " {
  LOCptr->openTransTag(yytext);
}


"</title>"     |
"</bookmark>"  |
"</paragraph>" {
  LOCptr->closeTransTag(yytext);
}



"<comment>" {
  LOCptr->startComment(yytext);
}



"</comment>" {
  LOCptr->stopComment(yytext);
}



"localize=\"false\"" {
  LOCptr->stopTransTag(yytext);
}



"&amp;"  |
"&gt;"   |
"&lt;"   |
"&quot;" {
  LOCptr->handleSpecial(yytext);
}


\r*\n\t* {
  LOCptr->handleDataEnd(yytext);
}



\\ {
  LOCptr->duplicate(yytext);
}



.|\n {
  LOCptr->copySourceSpecial(yytext, 0);

  // Just to please compiler.
  if (false)
    REJECT;
}



%%



void xhp_dummyJustForCompiler()
{
  char *txt = NULL;
//  yy_flex_strlen(txt);
  yyunput(0, txt);
}
