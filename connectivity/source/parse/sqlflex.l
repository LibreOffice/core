%{

//--------------------------------------------------------------------------
//
// $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/connectivity/source/parse/sqlflex.l,v 1.2 2000-10-19 11:47:14 oj Exp $
//
// Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.
//
// First creation: 
//	OJ
//
// Last change:
//	$Author: oj $ $Date: 2000-10-19 11:47:14 $ $Revision: 1.2 $
//
// Description:
//
//
//--------------------------------------------------------------------------

#define YY_EXIT 1				// YY_FATAL will not halt the application

#ifndef _CSTDARG_
#include <cstdarg>      // std::va_list
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#include "internalnode.hxx"
#endif

#ifndef _CONNECTIVITY_SQLYACC_HXX
#define _CONNECTIVITY_SQLYACC_HXX

#ifndef SQLYYDEBUG
#define SQLYYDEBUG 1
#endif

#include "connectivity/sqlbison.hxx"
#endif
#ifndef _CONNECTIVITY_SQLSCAN_HXX
#include "sqlscan.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
using namespace connectivity;

//=============================================================================
//
// Erzeugung der Blaetter fuer die Token
// Blaetter werden generell vom Lexer erzeugt

static ::rtl::OUString aEmptyString;

static sal_Int32	gatherString(sal_Int32 delim, sal_Int32 nTyp);
static sal_Int32	gatherName(const sal_Char*);
static sal_Int32	gatherNamePre(const sal_Char* );
// has to be set before the parser starts
OSQLScanner* xxx_pGLOBAL_SQLSCAN = NULL;

#define SQL_NEW_NODE(text, token)	\
		SQLyylval.pParseNode = new OSQLInternalNode(text, token);	    

#define SQL_NEW_KEYWORD(token)						\
		SQLyylval.pParseNode = new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, (token));		

#define SQL_NEW_NAME 		SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_NAME)
#define SQL_NEW_INTNUM 		SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_INTNUM)
#define SQL_NEW_APPROXNUM 	SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_APPROXNUM)
#define SQL_NEW_STRING 		SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_STRING)
#define SQL_NEW_COMPARISON 	SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_COMPARISON)
#define SQL_NEW_AMMSC 		SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_AMMSC)
#define SQL_NEW_DATE 		SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_ACCESS_DATE)

#define YY_INPUT(buf,result,max_size) 				\
{													\
	buf[0] = xxx_pGLOBAL_SQLSCAN->SQLyygetc();		\
	result = buf[0] != -1;							\
}

#define YY_FATAL_ERROR(msg)							\
{													\
	xxx_pGLOBAL_SQLSCAN->SQLyyerror(msg);			\
}													
	
//
//=============================================================================

%}
%e 2500
%n 2900 
%p 8500

%s SQL
%s PREDICATE_ENG
%s PREDICATE_GER
%s DATE
%s STRING

%option noyywrap
%option never-interactive
%%

[Aa][Ll][Ll]									{SQL_NEW_KEYWORD(SQL_TOKEN_ALL); return SQL_TOKEN_ALL; }
[Aa][Ll][Tt][Ee][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_ALTER); return SQL_TOKEN_ALTER; }
[Aa][Nn][Dd]									{SQL_NEW_KEYWORD(SQL_TOKEN_AND); return SQL_TOKEN_AND; }
[Aa][Nn][Yy]									{SQL_NEW_KEYWORD(SQL_TOKEN_ANY); return SQL_TOKEN_ANY; }
[Aa][Ss]										{SQL_NEW_KEYWORD(SQL_TOKEN_AS); return SQL_TOKEN_AS; }
[Aa][Ss][Cc]									{SQL_NEW_KEYWORD(SQL_TOKEN_ASC); return SQL_TOKEN_ASC; }
[Aa][Tt]										{SQL_NEW_KEYWORD(SQL_TOKEN_AT); return SQL_TOKEN_AT; }
[Aa][Uu][Tt][Hh][Oo][Rr][Ii][Zz][Aa][Tt][Ii][Oo][Nn]   		{SQL_NEW_KEYWORD(SQL_TOKEN_AUTHORIZATION); return SQL_TOKEN_AUTHORIZATION; }
[Aa][Vv][Gg]									{SQL_NEW_KEYWORD(SQL_TOKEN_AVG); return SQL_TOKEN_AVG; }

[Bb][Ee][Tt][Ww][Ee][Ee][Nn]					{SQL_NEW_KEYWORD(SQL_TOKEN_BETWEEN); return SQL_TOKEN_BETWEEN; }
[Bb][Ii][Tt]									{SQL_NEW_KEYWORD(SQL_TOKEN_BIT); return SQL_TOKEN_BIT; }
[Bb][Ii][Tt]_[Ll][Ee][Nn][Gg][Tt][Hh]           {SQL_NEW_KEYWORD(SQL_TOKEN_BIT_LENGTH); return SQL_TOKEN_BIT_LENGTH; }
[Bb][Oo][Tt][Hh]								{SQL_NEW_KEYWORD(SQL_TOKEN_BOTH); return SQL_TOKEN_BOTH; }
[Bb][Yy]										{SQL_NEW_KEYWORD(SQL_TOKEN_BY); return SQL_TOKEN_BY; }

[Cc][Aa][Ll][Ll]								{SQL_NEW_KEYWORD(SQL_TOKEN_CALL); return SQL_TOKEN_CALL; }
[Cc][Aa][Ss][Tt]								{SQL_NEW_KEYWORD(SQL_TOKEN_CAST); return SQL_TOKEN_CAST; }
[Cc][Hh][Aa][Rr]([Aa][Cc][Tt][Ee][Rr])?			{SQL_NEW_KEYWORD(SQL_TOKEN_CHARACTER); return SQL_TOKEN_CHARACTER; }
[Cc][Hh][Aa][Rr]([Aa][Cc][Tt][Ee][Rr])?_[Ll][Ee][Nn][Gg][Tt][Hh]  		{SQL_NEW_KEYWORD(SQL_TOKEN_CHAR_LENGTH); return SQL_TOKEN_CHAR_LENGTH; }
[Cc][Hh][Ee][Cc][Kk]							{SQL_NEW_KEYWORD(SQL_TOKEN_CHECK); return SQL_TOKEN_CHECK; }
[Cc][Oo][Ll][Ll][Aa][Tt][Ee]					{SQL_NEW_KEYWORD(SQL_TOKEN_COLLATE); return SQL_TOKEN_COLLATE; }
[Cc][Oo][Mm][Mm][Ii][Tt]						{SQL_NEW_KEYWORD(SQL_TOKEN_COMMIT); return SQL_TOKEN_COMMIT; }
[Cc][Oo][Nn][Tt][Ii][Nn][Uu][Ee]				{SQL_NEW_KEYWORD(SQL_TOKEN_CONTINUE); return SQL_TOKEN_CONTINUE; }
[Cc][Oo][Nn][Vv][Ee][Rr][Tt]					{SQL_NEW_KEYWORD(SQL_TOKEN_CONVERT); return SQL_TOKEN_CONVERT; }
[Cc][Oo][Uu][Nn][Tt]							{SQL_NEW_KEYWORD(SQL_TOKEN_COUNT); return SQL_TOKEN_COUNT; }
[Cc][Rr][Ee][Aa][Tt][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_CREATE); return SQL_TOKEN_CREATE; }
[Cc][Rr][Oo][Ss][Ss]							{SQL_NEW_KEYWORD(SQL_TOKEN_CROSS); return SQL_TOKEN_CROSS; }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]					{SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT); return SQL_TOKEN_CURRENT; }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]_[Dd][Aa][Tt][Ee]	{SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DATE); return SQL_TOKEN_CURRENT_DATE; }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]_[Tt][Ii][Mm][Ee]	{SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIME); return SQL_TOKEN_CURRENT_TIME; }
[Cc][Uu][Rr][Rr][Ee][Nn][Tt]_[Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp]	{SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIMESTAMP); return SQL_TOKEN_CURRENT_TIMESTAMP; }
[Cc][Uu][Rr][Ss][Oo][Rr]						{SQL_NEW_KEYWORD(SQL_TOKEN_CURSOR); return SQL_TOKEN_CURSOR; }

[Dd]											{SQL_NEW_KEYWORD(SQL_TOKEN_D); return SQL_TOKEN_D; }
[Dd][Aa][Tt][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_DATE); return SQL_TOKEN_DATE; }
[Dd][Aa][Yy]									{SQL_NEW_KEYWORD(SQL_TOKEN_DAY); return SQL_TOKEN_DAY; }
[Dd][Ee][Cc]									{SQL_NEW_KEYWORD(SQL_TOKEN_DEC); return SQL_TOKEN_DEC; }
[Dd][Ee][Cc][Ii][Mm][Aa][Ll]					{SQL_NEW_KEYWORD(SQL_TOKEN_DECIMAL); return SQL_TOKEN_DECIMAL; }
[Dd][Ee][Cc][Ll][Aa][Rr][Ee]					{SQL_NEW_KEYWORD(SQL_TOKEN_DECLARE); return SQL_TOKEN_DECLARE; }
[Dd][Ee][Ff][Aa][Uu][Ll][Tt]					{SQL_NEW_KEYWORD(SQL_TOKEN_DEFAULT); return SQL_TOKEN_DEFAULT; }
[Dd][Ee][Ll][Ee][Tt][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_DELETE); return SQL_TOKEN_DELETE; }
[Dd][Ee][Ss][Cc]								{SQL_NEW_KEYWORD(SQL_TOKEN_DESC); return SQL_TOKEN_DESC; }
[Dd][Ii][Ss][Tt][Ii][Nn][Cc][Tt]				{SQL_NEW_KEYWORD(SQL_TOKEN_DISTINCT); return SQL_TOKEN_DISTINCT; }
[Dd][Oo][Uu][Bb][Ll][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_DOUBLE); return SQL_TOKEN_DOUBLE; }
[Dd][Rr][Oo][Pp]								{SQL_NEW_KEYWORD(SQL_TOKEN_DROP); return SQL_TOKEN_DROP; }

[Ee][Ss][Cc][Aa][Pp][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_ESCAPE); return SQL_TOKEN_ESCAPE; }
[Ee][Xx][Cc][Ee][Pp][Tt]						{SQL_NEW_KEYWORD(SQL_TOKEN_EXCEPT); return SQL_TOKEN_EXCEPT; }
[Ee][Xx][Ii][Ss][Tt][Ss]						{SQL_NEW_KEYWORD(SQL_TOKEN_EXISTS); return SQL_TOKEN_EXISTS; }
[Ee][Xx][Tt][Rr][Aa][Cc][Tt]					{SQL_NEW_KEYWORD(SQL_TOKEN_EXTRACT); return SQL_TOKEN_EXTRACT; }

[Ff][Aa][Ll][Ss][Ee]							{SQL_NEW_KEYWORD(SQL_TOKEN_FALSE); return SQL_TOKEN_FALSE; }
[Ff][Ee][Tt][Cc][Hh]							{SQL_NEW_KEYWORD(SQL_TOKEN_FETCH); return SQL_TOKEN_FETCH; }
[Ff][Ll][Oo][Aa][Tt]							{SQL_NEW_KEYWORD(SQL_TOKEN_FLOAT); return SQL_TOKEN_FLOAT; }
[Ff][Nn]										{SQL_NEW_KEYWORD(SQL_TOKEN_FN); return SQL_TOKEN_FN; }
[Ff][Oo][Rr]									{SQL_NEW_KEYWORD(SQL_TOKEN_FOR); return SQL_TOKEN_FOR; }
[Ff][Oo][Rr][Ee][Ii][Gg][Nn]					{SQL_NEW_KEYWORD(SQL_TOKEN_FOREIGN); return SQL_TOKEN_FOREIGN; }
[Ff][Oo][Uu][Nn][Dd]							{SQL_NEW_KEYWORD(SQL_TOKEN_FOUND); return SQL_TOKEN_FOUND; }
[Ff][Rr][Oo][Mm]								{SQL_NEW_KEYWORD(SQL_TOKEN_FROM); return SQL_TOKEN_FROM; }
[Ff][Uu][Ll][Ll]								{SQL_NEW_KEYWORD(SQL_TOKEN_FULL); return SQL_TOKEN_FULL; }

[Gg][Rr][Aa][Nn][Tt]							{SQL_NEW_KEYWORD(SQL_TOKEN_GRANT); return SQL_TOKEN_GRANT; }
[Gg][Rr][Oo][Uu][Pp]							{SQL_NEW_KEYWORD(SQL_TOKEN_GROUP); return SQL_TOKEN_GROUP; }

[Hh][Aa][Vv][Ii][Nn][Gg]						{SQL_NEW_KEYWORD(SQL_TOKEN_HAVING); return SQL_TOKEN_HAVING; }
[Hh][Oo][Uu][Rr]								{SQL_NEW_KEYWORD(SQL_TOKEN_HOUR); return SQL_TOKEN_HOUR; }

[Ii][Nn]										{SQL_NEW_KEYWORD(SQL_TOKEN_IN); return SQL_TOKEN_IN; }
[Ii][Nn][Nn][Ee][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_INNER); return SQL_TOKEN_INNER; }
[Ii][Nn][Ss][Ee][Rr][Tt]						{SQL_NEW_KEYWORD(SQL_TOKEN_INSERT); return SQL_TOKEN_INSERT; }
[Ii][Nn][Tt]([Ee][Gg][Ee][Rr])?					{SQL_NEW_KEYWORD(SQL_TOKEN_INTEGER); return SQL_TOKEN_INTEGER; }
[Ii][Nn][Tt][Ee][Rr][Ss][Ee][Cc][Tt]			{SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECT); return SQL_TOKEN_INTERSECT; }
[Ii][Nn][Tt][Oo]								{SQL_NEW_KEYWORD(SQL_TOKEN_INTO); return SQL_TOKEN_INTO; }
[Ii][Ss]										{SQL_NEW_KEYWORD(SQL_TOKEN_IS); return SQL_TOKEN_IS; }

[Jj][Oo][Ii][Nn]								{SQL_NEW_KEYWORD(SQL_TOKEN_JOIN); return SQL_TOKEN_JOIN; }

[Kk][Ee][Yy]									{SQL_NEW_KEYWORD(SQL_TOKEN_KEY); return SQL_TOKEN_KEY; }

[Ll][Ee][Aa][Dd][Ii][Nn][Gg]					{SQL_NEW_KEYWORD(SQL_TOKEN_LEADING); return SQL_TOKEN_LEADING; }
[Ll][Ee][Ff][Tt]								{SQL_NEW_KEYWORD(SQL_TOKEN_LEFT); return SQL_TOKEN_LEFT; }
[Ll][Ii][Kk][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_LIKE); return SQL_TOKEN_LIKE; }
[Ll][Oo][Cc][Aa][Ll]							{SQL_NEW_KEYWORD(SQL_TOKEN_LOCAL); return SQL_TOKEN_LOCAL; }
[Ll][Oo][Ww][Ee][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_LOWER); return SQL_TOKEN_LOWER; }

[Mm][Aa][Xx]									{SQL_NEW_KEYWORD(SQL_TOKEN_MAX); return SQL_TOKEN_MAX; }
[Mm][Ii][Nn]									{SQL_NEW_KEYWORD(SQL_TOKEN_MIN); return SQL_TOKEN_MIN; }
[Mm][Ii][Nn][Uu][Tt][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_MINUTE); return SQL_TOKEN_MINUTE; }
[Mm][Oo][Nn][Tt][Hh]							{SQL_NEW_KEYWORD(SQL_TOKEN_MONTH); return SQL_TOKEN_MONTH; }

[Nn][Aa][Tt][Uu][Rr][Aa][LL]					{SQL_NEW_KEYWORD(SQL_TOKEN_NATURAL); return SQL_TOKEN_NATURAL; }
[Nn][Cc][Hh][Aa][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_NCHAR); return SQL_TOKEN_NCHAR; }
[Nn][Oo][Tt]									{SQL_NEW_KEYWORD(SQL_TOKEN_NOT); return SQL_TOKEN_NOT; }
[Nn][Uu][Ll][Ll]								{SQL_NEW_KEYWORD(SQL_TOKEN_NULL); return SQL_TOKEN_NULL; }
[Nn][Uu][Mm][Ee][Rr][Ii][Cc]					{SQL_NEW_KEYWORD(SQL_TOKEN_NUMERIC); return SQL_TOKEN_NUMERIC; }

[Oo][Cc][Tt][Ee][Tt]_[Ll][Ee][Nn][Gg][Tt][Hh]	{SQL_NEW_KEYWORD(SQL_TOKEN_OCTECT_LENGTH); return SQL_TOKEN_OCTECT_LENGTH; }
[Oo][Ff]										{SQL_NEW_KEYWORD(SQL_TOKEN_OF); return SQL_TOKEN_OF; }
[Oo][Jj]										{SQL_NEW_KEYWORD(SQL_TOKEN_OJ); return SQL_TOKEN_OJ; }
[Oo][Nn]										{SQL_NEW_KEYWORD(SQL_TOKEN_ON); return SQL_TOKEN_ON; }
[Oo][Pp][Tt][Ii][Oo][Nn]						{SQL_NEW_KEYWORD(SQL_TOKEN_OPTION); return SQL_TOKEN_OPTION; }
[Oo][Rr]										{SQL_NEW_KEYWORD(SQL_TOKEN_OR); return SQL_TOKEN_OR; }
[Oo][Rr][Dd][Ee][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_ORDER); return SQL_TOKEN_ORDER; }
[Oo][Uu][Tt][Ee][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_OUTER); return SQL_TOKEN_OUTER; }

[Pp][Oo][Ss][Ii][Tt][Ii][Oo][Nn]	            {SQL_NEW_KEYWORD(SQL_TOKEN_POSITION); return SQL_TOKEN_POSITION; }
[Pp][Rr][Ee][Cc][Ii][Ss][Ii][Oo][Nn]            {SQL_NEW_KEYWORD(SQL_TOKEN_PRECISION); return SQL_TOKEN_PRECISION; }
[Pp][Rr][Ii][Mm][Aa][Rr][Yy]					{SQL_NEW_KEYWORD(SQL_TOKEN_PRIMARY); return SQL_TOKEN_PRIMARY; }
[Pp][Rr][Ii][Vv][Ii][Ll][Ee][Gg][Ee][Ss]        {SQL_NEW_KEYWORD(SQL_TOKEN_PRIVILEGES); return SQL_TOKEN_PRIVILEGES; }
[Pp][Rr][Oo][Cc][Ee][Dd][Uu][Rr][Ee]            {SQL_NEW_KEYWORD(SQL_TOKEN_PROCEDURE); return SQL_TOKEN_PROCEDURE; }
[Pp][Uu][Bb][Ll][Ii][Cc]						{SQL_NEW_KEYWORD(SQL_TOKEN_PUBLIC); return SQL_TOKEN_PUBLIC; }

[Rr][Ee][Aa][Ll]								{SQL_NEW_KEYWORD(SQL_TOKEN_REAL); return SQL_TOKEN_REAL; }
[Rr][Ee][Ff][Ee][Rr][Ee][Nn][Cc][Ee][Ss]        {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCES); return SQL_TOKEN_REFERENCES; }
[Rr][Oo][Ll][Ll][Bb][Aa][Cc][Kk]                {SQL_NEW_KEYWORD(SQL_TOKEN_ROLLBACK); return SQL_TOKEN_ROLLBACK; }
[Rr][Ii][Gg][Hh][Tt]							{SQL_NEW_KEYWORD(SQL_TOKEN_RIGHT); return SQL_TOKEN_RIGHT; }

[Ss][Cc][Hh][Ee][Mm][Aa]						{SQL_NEW_KEYWORD(SQL_TOKEN_SCHEMA); return SQL_TOKEN_SCHEMA; }
[Ss][Ee][Cc][Ee][Oo][Nn][Dd]					{SQL_NEW_KEYWORD(SQL_TOKEN_SECOND); return SQL_TOKEN_SECOND; }
[Ss][Ee][Ll][Ee][Cc][Tt]						{SQL_NEW_KEYWORD(SQL_TOKEN_SELECT); return SQL_TOKEN_SELECT; }
[Ss][Ee][Tt]									{SQL_NEW_KEYWORD(SQL_TOKEN_SET); return SQL_TOKEN_SET; }
[Ss][Ii][Zz][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_SIZE); return SQL_TOKEN_SIZE; }
[Ss][Mm][Aa][Ll][Ll][Ii][Nn][Tt]                {SQL_NEW_KEYWORD(SQL_TOKEN_SMALLINT); return SQL_TOKEN_SMALLINT; }
[Ss][Oo][Mm][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_SOME); return SQL_TOKEN_SOME; }
[Ss][Uu][Bb][Ss][Tt][Rr][Ii][Nn][Gg]			{SQL_NEW_KEYWORD(SQL_TOKEN_SUBSTRING); return SQL_TOKEN_SUBSTRING; }
[Ss][Uu][Mm]									{SQL_NEW_KEYWORD(SQL_TOKEN_SUM); return SQL_TOKEN_SUM; }

[Tt]											{SQL_NEW_KEYWORD(SQL_TOKEN_T); return SQL_TOKEN_T; }
[Tt][Aa][Bb][Ll][Ee]							{SQL_NEW_KEYWORD(SQL_TOKEN_TABLE); return SQL_TOKEN_TABLE; }
[Tt][Ii][Mm][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_TIME); return SQL_TOKEN_TIME; }
[Tt][Ii][Mm][Ee][Ss][Tt][Aa][Mm][Pp]			{SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMP); return SQL_TOKEN_TIMESTAMP; }
[Tt][Ii][Mm][Ee][Zz][Oo][Nn][Ee]_[Hh][Oo][Uu][Rr]	{SQL_NEW_KEYWORD(SQL_TOKEN_TIMEZONE_HOUR); return SQL_TOKEN_TIMEZONE_HOUR; }
[Tt][Ii][Mm][Ee][Zz][Oo][Nn][Ee]_[Mm][Ii][Nn][Uu][Tt][Ee]	{SQL_NEW_KEYWORD(SQL_TOKEN_TIMEZONE_MINUTE); return SQL_TOKEN_TIMEZONE_MINUTE; }
[Tt][Oo]										{SQL_NEW_KEYWORD(SQL_TOKEN_TO); return SQL_TOKEN_TO; }
[Tt][Rr][Aa][Ii][Ll][Ii][Nn][Gg]				{SQL_NEW_KEYWORD(SQL_TOKEN_TRAILING); return SQL_TOKEN_TRAILING; }
[Tt][Rr][Aa][Nn][Ss][Ll][Aa][Tt][Ee]			{SQL_NEW_KEYWORD(SQL_TOKEN_TRANSLATE); return SQL_TOKEN_TRANSLATE; }
[Tt][Rr][Ii][Mm]								{SQL_NEW_KEYWORD(SQL_TOKEN_TRIM); return SQL_TOKEN_TRIM; }
[Tt][Rr][Uu][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_TRUE); return SQL_TOKEN_TRUE; }
[Tt][Ss]										{SQL_NEW_KEYWORD(SQL_TOKEN_TS); return SQL_TOKEN_TS; }

[Uu][Nn][Ii][Oo][Nn]							{SQL_NEW_KEYWORD(SQL_TOKEN_UNION); return SQL_TOKEN_UNION; }
[Uu][Nn][Ii][Qq][Uu][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_UNIQUE); return SQL_TOKEN_UNIQUE; }
[Uu][Nn][Kk][Nn][Oo][Ww][Nn]					{SQL_NEW_KEYWORD(SQL_TOKEN_UNKNOWN); return SQL_TOKEN_UNKNOWN; }
[Uu][Pp][Pp][Ee][Rr]							{SQL_NEW_KEYWORD(SQL_TOKEN_UPPER); return SQL_TOKEN_UPPER; }
[Uu][Pp][Dd][Aa][Tt][Ee]						{SQL_NEW_KEYWORD(SQL_TOKEN_UPDATE); return SQL_TOKEN_UPDATE; }
[Uu][Ss][Aa][Gg][Ee]							{SQL_NEW_KEYWORD(SQL_TOKEN_USAGE); return SQL_TOKEN_USAGE; }
[Uu][Ss][Ee][Rr]								{SQL_NEW_KEYWORD(SQL_TOKEN_USER); return SQL_TOKEN_USER; }
[Uu][Ss][Ii][Nn][Gg]							{SQL_NEW_KEYWORD(SQL_TOKEN_USING); return SQL_TOKEN_USING; }

[Vv][Aa][Ll][Uu][Ee][Ss]						{SQL_NEW_KEYWORD(SQL_TOKEN_VALUES); return SQL_TOKEN_VALUES; }
[Vv][Ii][Ee][Ww]								{SQL_NEW_KEYWORD(SQL_TOKEN_VIEW); return SQL_TOKEN_VIEW; }

[Ww][Hh][Ee][Rr][Ee]							{SQL_NEW_KEYWORD(SQL_TOKEN_WHERE); return SQL_TOKEN_WHERE; }
[Ww][Ii][Tt][Hh]								{SQL_NEW_KEYWORD(SQL_TOKEN_WITH); return SQL_TOKEN_WITH; }
[Ww][Oo][Rr][Kk]								{SQL_NEW_KEYWORD(SQL_TOKEN_WORK); return SQL_TOKEN_WORK; }

[Yy][Ee][Aa][Rr]								{SQL_NEW_KEYWORD(SQL_TOKEN_YEAR); return SQL_TOKEN_YEAR; }

[Zz][Oo][Nn][Ee]								{SQL_NEW_KEYWORD(SQL_TOKEN_ZONE); return SQL_TOKEN_ZONE; }

"<"					{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_LESS);return LESS;}
">"					{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_GREAT);return GREAT;}
"="					{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_EQUAL);return EQUAL;}
"<="				{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_LESSEQ);return LESSEQ;}
">="				{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_GREATEQ);return GREATEQ;}
"<>"				{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_NOTEQUAL);return NOTEQUAL;}
"!="				{ SQL_NEW_NODE(::rtl::OUString::createFromAscii(SQLyytext), SQL_NODE_NOTEQUAL);return NOTEQUAL;}


[-+*/:(),.;?{}]    { return SQLyytext[0]; }

<SQL>[A-Za-z][A-Za-z0-9_]*		{return gatherName( SQLyytext);}

<SQL>([0-9]+) | 
<SQL>([0-9]+"."[0-9]*) | 
<SQL>("."[0-9]*)					{SQL_NEW_INTNUM; return SQL_TOKEN_INTNUM;}

<SQL>[0-9]+[eE][+-]?[0-9]+   |
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM; return SQL_TOKEN_APPROXNUM; }

<PREDICATE_GER,PREDICATE_ENG,DATE>[A-Za-z][A-Za-z0-9_%.,*?]* {return gatherNamePre(SQLyytext);}

<PREDICATE_GER,PREDICATE_ENG>([0-9]+)		{SQL_NEW_INTNUM; return SQL_TOKEN_INTNUM;}
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+) 	{SQL_NEW_INTNUM; return SQL_TOKEN_INTNUM;}
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+)	{SQL_NEW_INTNUM; return SQL_TOKEN_INTNUM;}

<PREDICATE_ENG>([0-9]+"."[0-9]+) |
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+"."[0-9]+) | 
<PREDICATE_ENG>("."[0-9]+)					{SQL_NEW_APPROXNUM; return SQL_TOKEN_APPROXNUM; }
<PREDICATE_ENG>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_ENG>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_ENG>"."[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM; return SQL_TOKEN_APPROXNUM; }

<PREDICATE_GER>([0-9]+","[0-9]+) |
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+","[0-9]+) | 
<PREDICATE_GER>(","[0-9]+)					{SQL_NEW_APPROXNUM; return SQL_TOKEN_APPROXNUM; }
<PREDICATE_GER>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_GER>[0-9]+","[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_GER>","[0-9]*[eE][+-]?[0-9]+		{SQL_NEW_APPROXNUM; return SQL_TOKEN_APPROXNUM; }

<PREDICATE_GER,PREDICATE_ENG>[0-9.,][A-Za-z0-9_.,%]* {return gatherNamePre(SQLyytext);}

<SQL>\"	{ return gatherString('\"',0); }
<SQL>`	{ return gatherString('`' ,0); }											 

<PREDICATE_GER,PREDICATE_ENG,DATE>"["		{ return gatherString(']' ,0);}

\'		{ return gatherString('\'',1); }

<PREDICATE_GER,PREDICATE_ENG,DATE>#	{ return gatherString('#' ,2); }	

<DATE>[0-9]{1,4}[^ ]*[0-9] |
<DATE>[0-9]{1,4}[^ ]*[0-9][ ][0-9]{1,4}[^ ]*[0-9]		{ SQL_NEW_DATE; return SQL_TOKEN_ACCESS_DATE;}

<STRING>["-""+""*""/"":""("")"",""."";""?""{""}"]		{ return SQLyytext[0]; } /*  */
<STRING>"["					{ return gatherString(']' ,0); }
<STRING>[^ ':[]*			{ return gatherNamePre(SQLyytext); }

\n              {}

[ \t\r]+        ;       

"--".*$         ;       

.				{YY_FATAL_ERROR("Invalid symbol"); return SQL_TOKEN_INVALIDSYMBOL;}

%%

/*
 * Read SQL string literal
 * Valid strings:
 *	''	'a string'	'quote '' within string'
 *	""	"a string"	"quote "" within string"
 * nTyp == 0 -> SQL_NODE_NAME
 * nTyp == 1 -> SQL_NODE_STRING
 * nTyp == 2 -> SQL_NODE_ACCESS_DATE
 */
sal_Int32 gatherString( sal_Int32 delim, sal_Int32 nTyp)
{
	sal_Int32 ch;
	static sal_Int32 BUFFERSIZE = 256;
	static sal_Char* Buffer = new sal_Char[BUFFERSIZE];
	
	sal_Char *s = Buffer;
	sal_Int32 nPos = 0;

	while ((ch = yyinput()) != EOF) 
	{
		if (ch == delim) 
		{
			if ((ch = yyinput()) != delim) 
			{
				if (ch != EOF)
					unput(ch);
				
				*s = '\0';		 		
				
				switch(nTyp)
				{
					case 0:
						SQL_NEW_NODE(::rtl::OUString(Buffer,BUFFERSIZE,RTL_TEXTENCODING_UTF8), SQL_NODE_NAME);
						return SQL_TOKEN_NAME;											
					case 1:
						SQL_NEW_NODE(::rtl::OUString(Buffer,BUFFERSIZE,RTL_TEXTENCODING_UTF8), SQL_NODE_STRING);
						return SQL_TOKEN_STRING;
					case 2:
						SQL_NEW_NODE(::rtl::OUString(Buffer,BUFFERSIZE,RTL_TEXTENCODING_UTF8), SQL_NODE_ACCESS_DATE);
						return SQL_TOKEN_ACCESS_DATE;
				}				
			} 
			else
			{
				*s++ = ch;
				if (++nPos == BUFFERSIZE)
				{
					::rtl::OString aBuf(Buffer);
					delete Buffer;
					BUFFERSIZE *=2;
					Buffer = new sal_Char[BUFFERSIZE];
					for(sal_Int32 i=0;i<aBuf.getLength();++i,++Buffer)
						*Buffer = aBuf.getStr()[i];
					s = &Buffer[nPos];
				}
			}

		} 
		else if (ch == '\r' || ch == '\n')
			break;					
		else
		{
			*s++ = ch;
			if (++nPos == BUFFERSIZE)
			{
				::rtl::OString aBuf(Buffer);
				delete Buffer;
				BUFFERSIZE *=2;
				Buffer = new sal_Char[BUFFERSIZE];
				for(sal_Int32 i=0;i<aBuf.getLength();++i,++Buffer)
					*Buffer = aBuf.getStr()[i];
				s = &Buffer[nPos];
			}
		}
	}
	*s = '\0';		 		
	YY_FATAL_ERROR("Unterminated name string"); 
	return SQL_TOKEN_INVALIDSYMBOL;		
}

/*
 * Read SQL Name literal
 * Valid Names or internatioanl keywords:
 *	As we have international keywords, we test first on them  
 */
sal_Int32 gatherName(const sal_Char* text)
{
	OSL_ENSHURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
	sal_Int32 nToken = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
	switch (nToken)
	{
		case SQL_TOKEN_LIKE:
		case SQL_TOKEN_NOT:
		case SQL_TOKEN_NULL:
		case SQL_TOKEN_TRUE:
		case SQL_TOKEN_FALSE:
		case SQL_TOKEN_IS:
		case SQL_TOKEN_BETWEEN:
		case SQL_TOKEN_OR:
		case SQL_TOKEN_AND:
		case SQL_TOKEN_COUNT:
		case SQL_TOKEN_AVG:
		case SQL_TOKEN_MAX:
		case SQL_TOKEN_MIN:
		case SQL_TOKEN_SUM:
			SQL_NEW_KEYWORD(nToken); 
			return nToken;
		default:
			SQL_NEW_NODE(::rtl::OUString::createFromAscii(text), SQL_NODE_NAME); 
			return SQL_TOKEN_NAME; 
	}
}
/**
 Read SQL Name literal for predicate check
 Valid Names or internatioanl keywords:
 As we have international keywords, we test first on them  
*/
sal_Int32 gatherNamePre(const sal_Char* text)
{ 
	OSL_ENSHURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
	sal_Int32 nToken = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
	switch (nToken)
	{
		case SQL_TOKEN_LIKE:
		case SQL_TOKEN_NOT:
		case SQL_TOKEN_NULL:
		case SQL_TOKEN_TRUE:
		case SQL_TOKEN_FALSE:
		case SQL_TOKEN_IS:
		case SQL_TOKEN_BETWEEN:
		case SQL_TOKEN_OR:
		case SQL_TOKEN_AND:
		case SQL_TOKEN_COUNT:
		case SQL_TOKEN_AVG:
		case SQL_TOKEN_MAX:
		case SQL_TOKEN_MIN:
		case SQL_TOKEN_SUM:
			SQL_NEW_KEYWORD(nToken); 
			return nToken;
		default:
			// we need a special handling for parameter
			if (yy_hold_char == ':')
			{
				SQL_NEW_NODE(::rtl::OUString::createFromAscii(text), SQL_NODE_NAME); 
				return SQL_TOKEN_NAME; 				
			}
			else
			{
				SQL_NEW_NODE(::rtl::OUString::createFromAscii(text), SQL_NODE_STRING);
				return SQL_TOKEN_STRING;			
			}
	}															   	
}

using namespace connectivity;

static sal_uInt32 Intl_TokenID[] =
{
	SQL_TOKEN_LIKE, SQL_TOKEN_NOT, SQL_TOKEN_NULL, SQL_TOKEN_TRUE,
	SQL_TOKEN_FALSE, SQL_TOKEN_IS, SQL_TOKEN_BETWEEN, SQL_TOKEN_OR,
	SQL_TOKEN_AND, SQL_TOKEN_AVG, SQL_TOKEN_COUNT, SQL_TOKEN_MAX,
	SQL_TOKEN_MIN, SQL_TOKEN_SUM
};

//------------------------------------------------------------------------------
OSQLScanner::OSQLScanner()
			: m_nCurrentPos(0)
		    , m_bInternational(sal_False)
			, m_pContext(NULL)
			, m_nRule(0) // 0 is INITIAL
{
}

//------------------------------------------------------------------------------
OSQLScanner::~OSQLScanner()
{
}

//------------------------------------------------------------------------------
void OSQLScanner::SQLyyerror(sal_Char *fmt)
{
	OSL_ENSHURE(m_pContext, "OSQLScanner::SQLyyerror: No Context set");	
	m_sErrorMessage = m_pContext->getErrorMessage(OParseContext::ERROR_GENERAL);
	if (m_nCurrentPos < m_sStatement.getLength())
	{
		m_sErrorMessage += ::rtl::OUString::createFromAscii(": ");
		m_sErrorMessage += m_pContext->getErrorMessage(OParseContext::ERROR_GENERAL_HINT);
		
		m_sErrorMessage.replaceAt(m_sErrorMessage.indexOf('#',0),1,::rtl::OUString::createFromAscii(SQLyytext));	
	}
}

//------------------------------------------------------------------------------
void OSQLScanner::prepareScan(const ::rtl::OUString & rNewStatement, OParseContext* pContext, sal_Bool bInternational)
{
	BEGIN(m_nRule);

	m_sErrorMessage = ::rtl::OUString();	
	m_sStatement = ::rtl::OString(rNewStatement,rNewStatement.getLength(), RTL_TEXTENCODING_UTF8);
	m_nCurrentPos = 0;
	m_bInternational = bInternational;
	m_pContext = pContext;
}

//------------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLyygetc(void)
{
	return (m_nCurrentPos >= m_sStatement.getLength()) ? -1 : m_sStatement.getStr()[m_nCurrentPos++];
}

//------------------------------------------------------------------------------
sal_Int32 OSQLScanner::getInternationalTokenID(const sal_Char* sToken) const
{		
	OSL_ENSHURE(m_pContext, "OSQLScanner::getInternationalTokenID: No Context set");			
	return (m_bInternational) ? m_pContext->getIntlKeyCode(::rtl::OString(sToken, RTL_TEXTENCODING_ASCII_US)) : 0;	
}
// -------------------------------------------------------------------------
sal_Int32	OSQLScanner::GetCurrentRule() const { return m_nRule; }
sal_Int32	OSQLScanner::GetGERRule() const		{ return PREDICATE_GER; }
sal_Int32	OSQLScanner::GetENGRule() const		{ return PREDICATE_ENG; }
sal_Int32	OSQLScanner::GetSQLRule() const		{ return SQL; }
sal_Int32	OSQLScanner::GetDATERule() const	{ return DATE; }
sal_Int32	OSQLScanner::GetSTRINGRule() const	{ return STRING; }
// -------------------------------------------------------------------------
void OSQLScanner::setScanner(sal_Bool _bNull)
{
	xxx_pGLOBAL_SQLSCAN = _bNull ? NULL : this;
}
// -------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLlex()
{
	return SQLyylex();
}
/*------------------------------------------------------------------------

	$Log: not supported by cvs2svn $
	Revision 1.2  2000/08/01 14:26:02  oj
	modified
	
	Revision 1.1  2000/07/25 10:39:47  oj
	new revision
	
	Revision 1.0 21.07.2000 10:54:08  oj
------------------------------------------------------------------------*/

