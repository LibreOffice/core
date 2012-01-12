/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

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
