/*************************************************************************
 *
 *  $RCSfile: scanner.ll,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-27 10:53:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

%{
/*
 * scanner.ll - Lexical scanner for IDLC 1.0
 */
#include <ctype.h>

#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _IDLC_FEHELPER_HXX_
#include <idlc/fehelper.hxx>
#endif

class AstExpression;
class AstArray;
class AstMember;

#include <parser.h>

sal_Int32		beginLine = 0;
::rtl::OString	docu;

static sal_Int64 asciiToInteger( sal_Int8 base, const sal_Char *s )
{
	sal_Int64    r = 0;
	sal_Int64    negative = 0;

	if (*s == '-')
   	{
    	negative = 1;
      	s++;
   	}
   	if (base == 8 && *s == '0')
		s++;
   	else if (base == 16 && *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X'))
    	s += 2;

   	for (; *s; s++)
   	{
   		if (*s <= '9' && *s >= '0')
        	r = (r * base) + (*s - '0');
      	else if (base > 10 && *s <= 'f' && *s >= 'a')
        	r = (r * base) + (*s - 'a' + 10);
      	else if (base > 10 && *s <= 'F' && *s >= 'A')
        	r = (r * base) + (*s - 'A' + 10);
       	else
        	break;
	}
   	if (negative) r *= -1;
	return r;
}

static double asciiToFloat(const sal_Char *s)
{
	sal_Char    *h = (sal_Char*)s;
   	double  	d = 0.0;
   	double  	f = 0.0;
   	double  	e, k;
   	sal_Int32  	neg = 0, negexp = 0;

   	if (*s == '-')
   	{
    	neg = 1;
       	s++;
   	}
   	while (*s >= '0' && *s <= '9')
   	{
    	d = (d * 10) + *s - '0';
       	s++;
   	}
   	if (*s == '.')
   	{
    	s++;
       	e = 10;
       	while (*s >= '0' && *s <= '9')
       	{
        	d += (*s - '0') / (e * 1.0);
           	e *= 10;
           	s++;
       	}
   	}
   	if (*s == 'e' || *s == 'E')
   	{
    	s++;
       	if (*s == '-')
        {
        	negexp = 1;
           	s++;
       	} else
       	{
        	if (*s == '+')
            	s++;
           	e = 0;
           	while (*s >= '0' && *s <= '9')
           	{
            	e = (e * 10) + *s - '0';
             	s++;
           	}
           	if (e > 0)
           	{
            	for (k = 1; e > 0; k *= 10, e--);
               	if (negexp)
                	d /= k;
               	else
                	d *= k;
           	}
		}
   	}
   	if (neg) d *= -1.0;
   	return d;
}

static void	idlParsePragma(sal_Char* pPragma)
{
	::rtl::OString pragma(pPragma);
	sal_Int32 index = pragma.indexOf("include");
	sal_Char* begin = pPragma + index + 8;
	sal_Char* offset = begin;
	while (*offset != ',') offset++;
	//::rtl::OString include = pragma.copy(index + 8, offset - begin);
	idlc()->insertInclude(pragma.copy(index + 8, offset - begin));
}	

static void parseLineAndFile(sal_Char* pBuf)
{
	sal_Char	*r = pBuf;
	sal_Char    *h;
	sal_Bool	bIsInMain = sal_False;

	/* Skip initial '#' */
	if (*r != '#')
		return;

	/* Find line number */
	for (r++; *r == ' ' || *r == '\t' || isalpha(*r); r++);
	h = r;
	for (; *r != '\0' && *r != ' ' && *r != '\t'; r++);
	*r++ = 0;
	idlc()->setLineNumber((sal_uInt32)asciiToInteger(10, h));

	/* Find file name, if present */
	for (; *r != '"'; r++)
	{
		if (*r == '\n' || *r == '\0')
			return;
	}
	h = ++r;
	for (; *r != '"'; r++);
	*r = 0;
	if (*h == '\0')
		idlc()->setFileName(::rtl::OString("standard input"));
	else
		idlc()->setFileName(::rtl::OString(h));

	bIsInMain = (idlc()->getFileName() == idlc()->getRealFileName()) ? sal_True : sal_False;
	idlc()->setInMainfile(bIsInMain);		
}	

%}

%option noyywrap
%option never-interactive

%x DOCU
%x COMMENT

DIGIT			[0-9]
OCT_DIGIT       [0-7]
HEX_DIGIT       [a-fA-F0-9]
ALPHA			[a-zA-Z]
INT_LITERAL		[1-9][0-9]*
OCT_LITERAL		0{OCT_DIGIT}*
HEX_LITERAL		(0x|0X){HEX_DIGIT}*
ESC_SEQUENCE1	"\\"[ntvbrfa\\\?\'\"]
ESC_SEQUENCE2   "\\"{OCT_DIGIT}{1,3}
ESC_SEQUENCE3   "\\"(x|X){HEX_DIGIT}{1,2}
ESC_SEQUENCE    ({ESC_SEQUENCE1}|{ESC_SEQUENCE2}|{ESC_SEQUENCE3})
CHAR        	([^\n\t\"\'\\]|{ESC_SEQUENCE})
CHAR_LITERAL    "'"({CHAR}|\")"'"
STRING_LITERAL	\"({CHAR}|"'")*\"

IDENTIFIER		({ALPHA}|_{ALPHA})({ALPHA}|{DIGIT}|_)*

%%

[ \t\r]+	; /* eat up whitespace */
[\n] 		{
	idlc()->incLineNumber();
}

attribute		return IDL_ATTRIBUTE;
bound			return IDL_BOUND;
case			return IDL_CASE;
const			return IDL_CONST;
constants		return IDL_CONSTANTS;
constrained		return IDL_CONSTRAINED;
default			return IDL_DEFAULT;
enum			return IDL_ENUM;
exception		return IDL_EXCEPTION;
interface		return IDL_INTERFACE;
maybeambiguous 	return IDL_MAYBEAMBIGUOUS;
maybedefault	return IDL_MAYBEDEFAULT;
maybevoid		return IDL_MAYBEVOID;
module			return IDL_MODULE;
needs			return IDL_NEEDS;
observes		return IDL_OBSERVES;
optional		return IDL_OPTIONAL;
property		return IDL_PROPERTY;
raises			return IDL_RAISES;
readonly		return IDL_READONLY;
removable 		return IDL_REMOVEABLE;
service			return IDL_SERVICE;
sequence		return IDL_SEQUENCE;
struct			return IDL_STRUCT;
switch			return IDL_SWITCH;
transient 		return IDL_TRANSIENT;
typedef			return IDL_TYPEDEF;
union			return IDL_UNION;

any			return IDL_ANY;				
boolean		return IDL_BOOLEAN;
byte		return IDL_BYTE;
char		return IDL_CHAR;
double		return IDL_DOUBLE;
float		return IDL_FLOAT;
hyper		return IDL_HYPER;
long		return IDL_LONG;
short		return IDL_SHORT;
string		return IDL_STRING;
type		return IDL_TYPE;
unsigned	return IDL_UNSIGNED;
void		return IDL_VOID;

TRUE		return IDL_TRUE;
True		return IDL_TRUE;
FALSE		return IDL_FALSE;
False		return IDL_FALSE;

in			return IDL_IN;
out			return IDL_OUT;
inout		return IDL_INOUT;
oneway		return IDL_ONEWAY;

("-")?{INT_LITERAL}+(l|L|u|U)?    {
            	yylval.ival = asciiToInteger( 10, yytext );
				return IDL_INTEGER_LITERAL;
            }

("-")?{OCT_LITERAL}+(l|L|u|U)?    {
            	yylval.ival = asciiToInteger( 8, yytext+1 );
				return IDL_INTEGER_LITERAL;
            }

("-")?{HEX_LITERAL}+(l|L|u|U)?    {
            	yylval.ival = asciiToInteger( 16, yytext+2 );
				return IDL_INTEGER_LITERAL;
            }

{CHAR_LITERAL}	{
            	yylval.cval = *yytext;
				return IDL_CHARACTER_LITERAL;
			}

{STRING_LITERAL}	{
            	yylval.sval = new ::rtl::OString(yytext+1, strlen(yytext)-2);
				return IDL_STRING_LITERAL;
			}

("-")?{DIGIT}*(e|E){1}(("+"|"-")?{DIGIT}+)+(f|F)?	|
("-")?"."{DIGIT}*((e|E)("+"|"-")?{DIGIT}+)?(f|F)?	|
("-")?{DIGIT}*"."{DIGIT}*((e|E)("+"|"-")?{DIGIT}+)?(f|F)?        {
            	yylval.dval = asciiToFloat( yytext );
				return IDL_FLOATING_PT_LITERAL;
            }

{IDENTIFIER}	{
				yylval.sval = new ::rtl::OString(yytext);
				return IDL_IDENTIFIER;
			}

\<\<  	{
		yylval.strval = yytext;
		return IDL_LEFTSHIFT;
	}
\>\>	{
		yylval.strval = yytext;
		return IDL_RIGHTSHIFT;
	}
\:\:	{
		yylval.strval = yytext;
		return IDL_SCOPESEPERATOR;
	}

"/*"	{ 
			BEGIN( COMMENT );
			docu = ::rtl::OString();
			beginLine = idlc()->getLineNumber();
		}

"/***"	{ 
			BEGIN( COMMENT );
			docu = ::rtl::OString();
			beginLine = idlc()->getLineNumber();
		}

<COMMENT>[^*]+	{
				docu += ::rtl::OString(yytext);
			} 

<COMMENT>"*"[^*/]+ 	{
				docu += ::rtl::OString(yytext);
			}

<COMMENT>"**" 	{
				docu += ::rtl::OString(yytext);
			}

<COMMENT>[*]+"/"  {
				docu = docu.trim();
				idlc()->setLineNumber( beginLine + docu.getTokenCount('\n') - 1);
			  	BEGIN( INITIAL );
			}

"/**"	{
			BEGIN( DOCU );
			docu = ::rtl::OString();
			beginLine = idlc()->getLineNumber();
		}

<DOCU>[^*\n]+	{
				docu += ::rtl::OString(yytext);
			} 
<DOCU>"\n"[ \t]*"*"{1} 	{
				docu += ::rtl::OString("\n");
			}

<DOCU>"\n"	{
				docu += ::rtl::OString(yytext);
			}

<DOCU>"*"[^*/\n]+ 	{
				docu += ::rtl::OString(yytext);
			}

<DOCU>"\n"[ \t]*"*/" 	{
				docu = docu.trim();
				idlc()->setLineNumber( beginLine + docu.getTokenCount('\n') - 1);
				idlc()->setDocumentation(docu);
			  	BEGIN( INITIAL );
			}

<DOCU>"*/"	{
				docu = docu.trim();
				idlc()->setLineNumber( beginLine + docu.getTokenCount('\n') - 1);
				idlc()->setDocumentation(docu);
			  	BEGIN( INITIAL );
			}

"//"[^/]{1}.*"\n" {
				/* only a comment */
				::rtl::OString docStr(yytext);
				docStr = docStr.copy( 0, docStr.lastIndexOf('\n') );
				docStr = docStr.copy( docStr.lastIndexOf('/')+1 );
				docStr = docStr.trim();
				idlc()->incLineNumber();
			}

"///".*"\n"  {
				::rtl::OString docStr(yytext);
				docStr = docStr.copy( 0, docStr.lastIndexOf('\n') );
				docStr = docStr.copy( docStr.lastIndexOf('/')+1 );
				docStr = docStr.trim();
				idlc()->incLineNumber();
				idlc()->setDocumentation(docStr);
			}

.	return yytext[0];

^#[ \t]*line[ \t]*[0-9]*" ""\""[^\"]*"\""\n    {
	parseLineAndFile(yytext);
}

^#[ \t]*[0-9]*" ""\""[^\"]*"\""" "[0-9]*\n {
	parseLineAndFile(yytext);
}

^#[ \t]*[0-9]*" ""\""[^\"]*"\""\n {
	parseLineAndFile(yytext);
}

^#[ \t]*[0-9]*\n {
	parseLineAndFile(yytext);
}

^#[ \t]*ident.*\n {
	/* ignore cpp ident */
	idlc()->incLineNumber();
}

^#[ \t]*pragma[ \t].*\n        {       /* remember pragma */
	idlParsePragma(yytext);
	idlc()->incLineNumber();
}

%%
