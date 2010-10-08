/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

/* compile with flex++ -8 -f -+ -Sflex.skl -ortfparser.cxx rtfparser.lex */
%option yylineno
%{
  //#include <io.h>
#include <math.h>
#include <string.h>
#include <osl/file.h>
#include <assert.h>
#include <vector>

#if defined (UNX)
  #define stricmp strcasecmp
#endif

writerfilter::rtftok::RTFScanner* writerfilter::rtftok::RTFScanner::createRTFScanner(class writerfilter::rtftok::RTFInputSource& inputSource, writerfilter::rtftok::RTFScannerHandler &eventHandler)
{
  return new yyFlexLexer(&inputSource, eventHandler);
}



extern "C" {
//int isatty(int fd) { return 0; }
int yywrap(void) { return 1; }
}

/*
oslFileHandle yy_osl_in=NULL;
#define YY_INPUT(buf,result,max_size) \
{\
{\
	assert(yy_osl_in!=NULL);\
	sal_Bool isEOF;\
	oslFileError ret=osl_isEndOfFile( yy_osl_in, &isEOF );\
	assert(ret==osl_File_E_None);\
	if (isEOF)\
	{\
		result=YY_NULL;\
	}\
	else\
	{\
	sal_uInt64 bytesRead;\
	ret=osl_readFile( yy_osl_in, buf, max_size, &bytesRead);\
	assert(ret==osl_File_E_None);\
	result = bytesRead; \
	}\
}\
}
*/

//extern RtfTokenizer* this;
void yyFlexLexer::split_ctrl(char *_yytext, char* token, char *value)
   {
     int i=0; // skip first '\'
     while(_yytext[i]!=0 && (_yytext[i]=='\r' || _yytext[i]=='\n')) i++;
     while(_yytext[i]!=0 && (_yytext[i]<'A' || (_yytext[i]>'Z' && _yytext[i]<'a') || _yytext[i]>'z')) i++; 
     while(_yytext[i]!=0 && _yytext[i]>='A') *(token++)=_yytext[i++];
     *token=0;
     while(_yytext[i]!=0 && _yytext[i]>' ') *(value++)=_yytext[i++];
     *value=0;
   }

 void yyFlexLexer::raise_ctrl(char* _yytext)
   {
     char token[50];
     char value[50];
     split_ctrl(_yytext, token, value);
     eventHandler.ctrl(token, value);
   }

 void yyFlexLexer::raise_dest(char* _yytext)
   {
     char token[50];
     char value[50];
     split_ctrl(_yytext, token, value);
     eventHandler.dest(token, value);
   }
 
#define _num_of_destctrls (sizeof(_destctrls)/sizeof(_destctrls[0]))
static const char* _destctrls[] = {
"aftncn",
"aftnsep",
"aftnsepc",
"annotation",
"atnauthor",
"atndate",
"atnicn",
"atnid",
"atnparent",
"atnref",
"atntime",
"atrfend",
"atrfstart",
"author",
"background",
"bkmkend",
"bkmkstart",
"buptim",
"category",
"colortbl",
"comment",
"company",
"creatim",
"datafield",
"do",
"doccomm",
"docvar",
"dptxbxtext",
"falt",
"fchars",
"ffdeftext",
"ffentrymcr",
"ffexitmcr",
"ffformat",
"ffhelptext",
"ffl",
"ffname",
"ffstattext",
"field",
"file",
"filetbl",
"fldinst",
"fldrslt",
"fldtype",
"fname",
"fontemb",
"fontfile",
"fonttbl",
"footer",
"footer",
"footerf",
"footerl",
"footnote",
"formfield",
"ftncn",
"ftnsep",
"ftnsepc",
"g",
"generator",
"gridtbl",
"header",
"header",
"headerf",
"headerl",
"htmltag",
"info",
"keycode",
"keywords",
"lchars",
"levelnumbers",
"leveltext",
"lfolevel",
"list",
"listlevel",
"listname",
"listoverride",
"listoverridetable",
"listtable",
"listtext",
"manager",
"mhtmltag",
"nesttableprops",
"nextfile",
"nonesttables",
"nonshppict",
"objalias",
"objclass",
"objdata",
"object",
"objname",
"objsect",
"objtime",
"oldcprops",
"oldpprops",
"oldsprops",
"oldtprops",
"operator",
"panose",
"pgp",
"pgptbl",
"picprop",
"pict",
"pn",
"pnseclvl",
"pntext",
"pntxta",
"pntxtb",
"printim",
"private",
"pwd",
"pxe",
"result",
"revtbl",
"revtim",
"rsidtbl",
"rtf",
"rxe",
"shp",
"shpgrp",
"shpinst",
"shppict",
"shprslt",
"shptxt",
"sn",
"sp",
"stylesheet",
"subject",
"sv",
"tc",
"template",
"title",
"txe",
"ud",
"upr",
"urtf",
"userprops",
"xe"
};

 void yyFlexLexer::raise_destOrCtrl(char* _yytext)
   {
     char token[50];
     char value[50];
     split_ctrl(_yytext, token, value);
     char* result=(char*)bsearch(token, _destctrls, _num_of_destctrls, 20, (int (*)(const void*, const void*))stricmp);
     if (result)
       {
	 eventHandler.dest(token, value);
       }
     else 
       {
	 eventHandler.lbrace();
	 eventHandler.ctrl(token, value);
       }
   }

%}

%%
\{\\upr\{" "? { /* skip upr destination */
  int c;
  int br=1;
  while (br>0 && (c = yyinput()) != EOF)
    {
      if (c=='}') br--;
      if (c=='{') br++;
    }
  eventHandler.lbrace();
  num_chars+=yyleng;
}


\\bin(("+"|"-")?[0-9]*)?" "? {
  raise_dest(yytext);
  num_chars+=yyleng;
  int len=atoi(yytext+4);
   num_chars+=len;
   //   pictureBytes=2*len;
  while ( len )
    {
      int c = yyinput();
      eventHandler.addBinData((unsigned char)c);
      len--;      
    }
  eventHandler.rbrace();
}

\{[\r\n]*\\\*\\[A-Za-z]+(("+"|"-")?[0-9]*)?" "? { /* stared dest word */
        raise_dest(yytext);
        num_chars+=yyleng;
}
\{[\r\n]*\\[A-Za-z]+(("+"|"-")?[0-9]*)?" "? { /* dest word */
        raise_destOrCtrl(yytext);
}
\\[A-Za-z]+(("+"|"-")?[0-9]*)?" "? { /* ctrl word */
        raise_ctrl(yytext);
	star_flag=0;
	num_chars+=yyleng;
}
\\\'[A-Fa-f0-9][A-Fa-f0-9] { /* hex char */
  eventHandler.addHexChar(yytext);
  num_chars+=yyleng;
}
\\* { /* star */
	star_flag=1;
	num_chars+=yyleng;
}
\{ { /* lbrace */
        eventHandler.lbrace();
        num_chars+=yyleng;
}
\} { /* rbrace */
        eventHandler.rbrace();
        num_chars+=yyleng;
}
\\\| { num_chars+=yyleng;}
\\~ {num_chars+=yyleng; eventHandler.addCharU(0xa0);}
\\- {num_chars+=yyleng;}
\\_ {num_chars+=yyleng;}
\\\: {num_chars+=yyleng;}
\n   {   ++num_lines;num_chars+=yyleng;}
\r {num_chars+=yyleng;}
\t {num_chars+=yyleng;}
" "(" "+) { eventHandler.addSpaces(yyleng); num_chars+=yyleng;}
. { eventHandler.addChar(yytext[0]); num_chars+=yyleng;}
%%
