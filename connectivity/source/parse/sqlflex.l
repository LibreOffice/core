/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

%{

#include "sal/config.h"

#define YY_EXIT 1               // YY_FATAL will not halt the application

#ifndef _CSTDARG_
#include <cstdarg>
#endif

#include <string.h>

#if defined _MSC_VER
#pragma warning ( push )
// Silence warnings about redefinition of INT8_MIN etc in stdint.h
// The flex-generated workdir/LexTarget/idlc/source/scanner.cxx defines them prior to these includes
#pragma warning ( disable : 4005 )
#endif
#include <connectivity/internalnode.hxx>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#ifndef INCLUDED_CONNECTIVITY_SOURCE_PARSE_SQLFLEX_L
#define INCLUDED_CONNECTIVITY_SOURCE_PARSE_SQLFLEX_L

#ifndef SQLYYDEBUG
#define SQLYYDEBUG 1
#endif

#include "sqlbison.hxx"
#undef SQLyylex
#undef SQLyyerror
#endif
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <connectivity/sqlparse.hxx>
#include <connectivity/sqlscan.hxx>

#if defined _MSC_VER
/**/
#ifdef yywrap
#undef  yywrap
#define yywrap() 1
#endif
/**/
#endif
#define YY_NO_UNISTD_H

using namespace connectivity;

// Creation of the pages for the tokens
// Pages generally are created from the Lexer

static sal_Int32    gatherString(int delim, sal_Int32 nTyp);
static sal_Int32    gatherName(const sal_Char*);
static sal_Int32    gatherNamePre(const sal_Char* );
// has to be set before the parser starts
OSQLScanner* xxx_pGLOBAL_SQLSCAN = nullptr;

#define SQL_NEW_NODE(text, token)   \
        SQLyylval.pParseNode = new OSQLInternalNode(text, token);

#define SQL_NEW_KEYWORD(token)                      \
        SQLyylval.pParseNode = new OSQLInternalNode("", SQLNodeType::Keyword, (token));       return token;

#define SQL_NEW_INTNUM      SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::IntNum); return SQL_TOKEN_INTNUM;
#define SQL_NEW_APPROXNUM   SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::ApproxNum); return SQL_TOKEN_APPROXNUM;
#define SQL_NEW_DATE        SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::AccessDate); return SQL_TOKEN_ACCESS_DATE;

#define YY_INPUT(buf,result,max_size)               \
{                                                   \
    int c = xxx_pGLOBAL_SQLSCAN->SQLyygetc();       \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1);\
}

// coverity[+kill]
static void do_fatal_error(const char* msg)
{
    xxx_pGLOBAL_SQLSCAN->SQLyyerror(msg);
    /*hack to silence -Wunused-function*/
    if (0) yy_fatal_error(msg);
}

#define YY_FATAL_ERROR(msg) \
{                           \
    do_fatal_error(msg);    \
}

%}

%s SQL
%s PREDICATE_ENG
%s PREDICATE_GER
%s DATE
%s STRING

%option noyywrap
%option never-interactive
%%

ABS                 {SQL_NEW_KEYWORD(SQL_TOKEN_ABS);  }
ACOS                {SQL_NEW_KEYWORD(SQL_TOKEN_ACOS);  }
AFTER               {SQL_NEW_KEYWORD(SQL_TOKEN_AFTER);  }
ALL                 {SQL_NEW_KEYWORD(SQL_TOKEN_ALL);  }
ALTER               {SQL_NEW_KEYWORD(SQL_TOKEN_ALTER);  }
AND                 {SQL_NEW_KEYWORD(SQL_TOKEN_AND);  }
ANY                 {SQL_NEW_KEYWORD(SQL_TOKEN_ANY);  }
ARRAY_AGG           {SQL_NEW_KEYWORD(SQL_TOKEN_ARRAY_AGG);  }
AS                  {SQL_NEW_KEYWORD(SQL_TOKEN_AS);  }
ASC                 {SQL_NEW_KEYWORD(SQL_TOKEN_ASC);  }
ASCII               {SQL_NEW_KEYWORD(SQL_TOKEN_ASCII);  }
ASIN                {SQL_NEW_KEYWORD(SQL_TOKEN_ASIN);  }
AT                  {SQL_NEW_KEYWORD(SQL_TOKEN_AT);  }
ATAN                {SQL_NEW_KEYWORD(SQL_TOKEN_ATAN);  }
ATAN2               {SQL_NEW_KEYWORD(SQL_TOKEN_ATAN2);  }
ATOMIC              {SQL_NEW_KEYWORD(SQL_TOKEN_ATOMIC);  }
AUTHORIZATION       {SQL_NEW_KEYWORD(SQL_TOKEN_AUTHORIZATION);  }
AVG                 {SQL_NEW_KEYWORD(SQL_TOKEN_AVG);  }

BEFORE              {SQL_NEW_KEYWORD(SQL_TOKEN_BEFORE);  }
BEGIN               {SQL_NEW_KEYWORD(SQL_TOKEN_BEGIN);  }
BETWEEN             {SQL_NEW_KEYWORD(SQL_TOKEN_BETWEEN);  }
BIGINT              {SQL_NEW_KEYWORD(SQL_TOKEN_BIGINT);  }
BINARY              {SQL_NEW_KEYWORD(SQL_TOKEN_BINARY);  }
BIT                 {SQL_NEW_KEYWORD(SQL_TOKEN_BIT);  }
BIT_LENGTH          {SQL_NEW_KEYWORD(SQL_TOKEN_BIT_LENGTH);  }
BLOB                {SQL_NEW_KEYWORD(SQL_TOKEN_BLOB);  }
BOTH                {SQL_NEW_KEYWORD(SQL_TOKEN_BOTH);  }
BY                  {SQL_NEW_KEYWORD(SQL_TOKEN_BY);  }

CALL                {SQL_NEW_KEYWORD(SQL_TOKEN_CALL);  }
CASE                {SQL_NEW_KEYWORD(SQL_TOKEN_CASE);  }
CAST                {SQL_NEW_KEYWORD(SQL_TOKEN_CAST);  }
CEILING             {SQL_NEW_KEYWORD(SQL_TOKEN_CEILING);  }
CHAR                {SQL_NEW_KEYWORD(SQL_TOKEN_CHAR);  }
CHARACTER           {SQL_NEW_KEYWORD(SQL_TOKEN_CHARACTER);  }
CHAR(ACTER)?_LENGTH {SQL_NEW_KEYWORD(SQL_TOKEN_CHAR_LENGTH);  }
CHECK               {SQL_NEW_KEYWORD(SQL_TOKEN_CHECK);  }
CLOB                {SQL_NEW_KEYWORD(SQL_TOKEN_CLOB);  }
COALESCE            {SQL_NEW_KEYWORD(SQL_TOKEN_COALESCE);  }
COLLATE             {SQL_NEW_KEYWORD(SQL_TOKEN_COLLATE);  }
COLLECT             {SQL_NEW_KEYWORD(SQL_TOKEN_COLLECT);  }
COMMIT              {SQL_NEW_KEYWORD(SQL_TOKEN_COMMIT);  }
CONCAT              {SQL_NEW_KEYWORD(SQL_TOKEN_CONCAT);  }
CONTINUE            {SQL_NEW_KEYWORD(SQL_TOKEN_CONTINUE);  }
CONVERT             {SQL_NEW_KEYWORD(SQL_TOKEN_CONVERT);  }
COS                 {SQL_NEW_KEYWORD(SQL_TOKEN_COS);  }
COT                 {SQL_NEW_KEYWORD(SQL_TOKEN_COT);  }
COUNT               {SQL_NEW_KEYWORD(SQL_TOKEN_COUNT);  }
CREATE              {SQL_NEW_KEYWORD(SQL_TOKEN_CREATE);  }
CROSS               {SQL_NEW_KEYWORD(SQL_TOKEN_CROSS);  }
CUME_RANK           {SQL_NEW_KEYWORD(SQL_TOKEN_CUME_DIST);  }
CURRENT             {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT);  }
CURRENT_DATE        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DATE);  }
CURRENT_CATALOG                 {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_CATALOG);  }
CURRENT_DEFAULT_TRANSFORM_GROUP {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP);  }
CURRENT_PATH                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_PATH);  }
CURRENT_ROLE                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_ROLE);  }
CURRENT_SCHEMA                  {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_SCHEMA);  }
CURRENT_USER                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_USER);  }
CURDATE             {SQL_NEW_KEYWORD(SQL_TOKEN_CURDATE);  }
CURRENT_TIME        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIME);  }
CURTIME             {SQL_NEW_KEYWORD(SQL_TOKEN_CURTIME);  }
CURRENT_TIMESTAMP   {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIMESTAMP);  }
CURSOR              {SQL_NEW_KEYWORD(SQL_TOKEN_CURSOR);  }

D                   {SQL_NEW_KEYWORD(SQL_TOKEN_D);  }
DATE                {SQL_NEW_KEYWORD(SQL_TOKEN_DATE);  }
DATEDIFF            {SQL_NEW_KEYWORD(SQL_TOKEN_DATEDIFF);  }
DATEVALUE           {SQL_NEW_KEYWORD(SQL_TOKEN_DATEVALUE);  }
DAY                 {SQL_NEW_KEYWORD(SQL_TOKEN_DAY);  }
DAYNAME             {SQL_NEW_KEYWORD(SQL_TOKEN_DAYNAME);  }
DAYOFMONTH          {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFMONTH);  }
DAYOFWEEK           {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFWEEK);  }
DAYOFYEAR           {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFYEAR);  }
DEC                 {SQL_NEW_KEYWORD(SQL_TOKEN_DEC);  }
DECIMAL             {SQL_NEW_KEYWORD(SQL_TOKEN_DECIMAL);  }
DECLARE             {SQL_NEW_KEYWORD(SQL_TOKEN_DECLARE);  }
DEFAULT             {SQL_NEW_KEYWORD(SQL_TOKEN_DEFAULT);  }
DEGREES             {SQL_NEW_KEYWORD(SQL_TOKEN_DEGREES);  }
DELETE              {SQL_NEW_KEYWORD(SQL_TOKEN_DELETE);  }
DENSE_RANK          {SQL_NEW_KEYWORD(SQL_TOKEN_DENSE_RANK);  }
DESC                {SQL_NEW_KEYWORD(SQL_TOKEN_DESC);  }
DIFFERENCE          {SQL_NEW_KEYWORD(SQL_TOKEN_DIFFERENCE);  }
DISTINCT            {SQL_NEW_KEYWORD(SQL_TOKEN_DISTINCT);  }
DOUBLE              {SQL_NEW_KEYWORD(SQL_TOKEN_DOUBLE);  }
DROP                {SQL_NEW_KEYWORD(SQL_TOKEN_DROP);  }

EACH                {SQL_NEW_KEYWORD(SQL_TOKEN_EACH);  }
ELSE                {SQL_NEW_KEYWORD(SQL_TOKEN_ELSE);  }
END                 {SQL_NEW_KEYWORD(SQL_TOKEN_END);  }
EVERY               {SQL_NEW_KEYWORD(SQL_TOKEN_EVERY);  }
ESCAPE              {SQL_NEW_KEYWORD(SQL_TOKEN_ESCAPE);  }
EXCEPT              {SQL_NEW_KEYWORD(SQL_TOKEN_EXCEPT);  }
EXCLUDE             {SQL_NEW_KEYWORD(SQL_TOKEN_EXCLUDE);  }
EXISTS              {SQL_NEW_KEYWORD(SQL_TOKEN_EXISTS);  }
EXP                 {SQL_NEW_KEYWORD(SQL_TOKEN_EXP);  }
EXTRACT             {SQL_NEW_KEYWORD(SQL_TOKEN_EXTRACT);  }

FALSE               {SQL_NEW_KEYWORD(SQL_TOKEN_FALSE);  }
FETCH               {SQL_NEW_KEYWORD(SQL_TOKEN_FETCH);  }
FIRST               {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST);  }
FIRST_VALUE         {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST_VALUE);  }
FLOAT               {SQL_NEW_KEYWORD(SQL_TOKEN_FLOAT);  }
FLOOR               {SQL_NEW_KEYWORD(SQL_TOKEN_FLOOR);  }
FN                  {SQL_NEW_KEYWORD(SQL_TOKEN_FN);  }
FOLLOWING           {SQL_NEW_KEYWORD(SQL_TOKEN_FOLLOWING);  }
FOR                 {SQL_NEW_KEYWORD(SQL_TOKEN_FOR);  }
FOREIGN             {SQL_NEW_KEYWORD(SQL_TOKEN_FOREIGN);  }
FOUND               {SQL_NEW_KEYWORD(SQL_TOKEN_FOUND);  }
FROM                {SQL_NEW_KEYWORD(SQL_TOKEN_FROM);  }
FULL                {SQL_NEW_KEYWORD(SQL_TOKEN_FULL);  }
FUSION              {SQL_NEW_KEYWORD(SQL_TOKEN_FUSION);  }

GRANT               {SQL_NEW_KEYWORD(SQL_TOKEN_GRANT);  }
GROUP               {SQL_NEW_KEYWORD(SQL_TOKEN_GROUP);  }

HAVING              {SQL_NEW_KEYWORD(SQL_TOKEN_HAVING);  }
HOUR                {SQL_NEW_KEYWORD(SQL_TOKEN_HOUR);  }

IGNORE              {SQL_NEW_KEYWORD(SQL_TOKEN_IGNORE);  }
IN                  {SQL_NEW_KEYWORD(SQL_TOKEN_IN);  }
INNER               {SQL_NEW_KEYWORD(SQL_TOKEN_INNER);  }
INSERT              {SQL_NEW_KEYWORD(SQL_TOKEN_INSERT);  }
INSTEAD             {SQL_NEW_KEYWORD(SQL_TOKEN_INSTEAD);  }
INT(EGER)?          {SQL_NEW_KEYWORD(SQL_TOKEN_INTEGER);  }
INTERSECT           {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECT);  }
INTERVAL            {SQL_NEW_KEYWORD(SQL_TOKEN_INTERVAL);  }
INTERSECTION        {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECTION);  }
INTO                {SQL_NEW_KEYWORD(SQL_TOKEN_INTO);  }
IS                  {SQL_NEW_KEYWORD(SQL_TOKEN_IS);  }

JOIN                {SQL_NEW_KEYWORD(SQL_TOKEN_JOIN);  }

KEY                 {SQL_NEW_KEYWORD(SQL_TOKEN_KEY);  }

LAG                 {SQL_NEW_KEYWORD(SQL_TOKEN_LAG);  }
LARGE               {SQL_NEW_KEYWORD(SQL_TOKEN_LARGE);  }
LAST                {SQL_NEW_KEYWORD(SQL_TOKEN_LAST);  }
LAST_VALUE          {SQL_NEW_KEYWORD(SQL_TOKEN_LAST_VALUE);  }
LCASE               {SQL_NEW_KEYWORD(SQL_TOKEN_LCASE);  }
LEAD                {SQL_NEW_KEYWORD(SQL_TOKEN_LEAD);  }
LEADING             {SQL_NEW_KEYWORD(SQL_TOKEN_LEADING);  }
LEFT                {SQL_NEW_KEYWORD(SQL_TOKEN_LEFT);  }
LENGTH              {SQL_NEW_KEYWORD(SQL_TOKEN_LENGTH);  }
LIKE                {SQL_NEW_KEYWORD(SQL_TOKEN_LIKE);  }
LIMIT               {SQL_NEW_KEYWORD(SQL_TOKEN_LIMIT);  }
LN                  {SQL_NEW_KEYWORD(SQL_TOKEN_LN);  }
LOCAL               {SQL_NEW_KEYWORD(SQL_TOKEN_LOCAL);  }
LOCATE              {SQL_NEW_KEYWORD(SQL_TOKEN_LOCATE);  }
LOG                 {SQL_NEW_KEYWORD(SQL_TOKEN_LOG);  }
LOGF                {SQL_NEW_KEYWORD(SQL_TOKEN_LOGF);  }
LOG10               {SQL_NEW_KEYWORD(SQL_TOKEN_LOG10);  }
LOWER               {SQL_NEW_KEYWORD(SQL_TOKEN_LOWER);  }
LTRIM               {SQL_NEW_KEYWORD(SQL_TOKEN_LTRIM);  }

MAX                 {SQL_NEW_KEYWORD(SQL_TOKEN_MAX);  }
MILLISECOND         {SQL_NEW_KEYWORD(SQL_TOKEN_MILLISECOND);  }
MIN                 {SQL_NEW_KEYWORD(SQL_TOKEN_MIN);  }
MINUTE              {SQL_NEW_KEYWORD(SQL_TOKEN_MINUTE);  }
MOD                 {SQL_NEW_KEYWORD(SQL_TOKEN_MOD);  }
MONTH               {SQL_NEW_KEYWORD(SQL_TOKEN_MONTH);  }
MONTHNAME           {SQL_NEW_KEYWORD(SQL_TOKEN_MONTHNAME);  }

NATIONAL            {SQL_NEW_KEYWORD(SQL_TOKEN_NATIONAL);  }
NATURAL             {SQL_NEW_KEYWORD(SQL_TOKEN_NATURAL);  }
NCHAR               {SQL_NEW_KEYWORD(SQL_TOKEN_NCHAR);  }
NCLOB               {SQL_NEW_KEYWORD(SQL_TOKEN_NCLOB);  }
NEW                 {SQL_NEW_KEYWORD(SQL_TOKEN_NEW);  }
NEXT                {SQL_NEW_KEYWORD(SQL_TOKEN_NEXT);  }
NO                  {SQL_NEW_KEYWORD(SQL_TOKEN_NO);  }
NOT                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOT);  }
NOW                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOW);  }
NTH_VALUE           {SQL_NEW_KEYWORD(SQL_TOKEN_NTH_VALUE);  }
NTILE               {SQL_NEW_KEYWORD(SQL_TOKEN_NTILE);  }
NULL                {SQL_NEW_KEYWORD(SQL_TOKEN_NULL);  }
NULLIF              {SQL_NEW_KEYWORD(SQL_TOKEN_NULLIF);  }
NULLS               {SQL_NEW_KEYWORD(SQL_TOKEN_NULLS);  }
NUMERIC             {SQL_NEW_KEYWORD(SQL_TOKEN_NUMERIC);  }

OBJECT              {SQL_NEW_KEYWORD(SQL_TOKEN_OBJECT);  }
OCTET_LENGTH        {SQL_NEW_KEYWORD(SQL_TOKEN_OCTET_LENGTH);  }
OF                  {SQL_NEW_KEYWORD(SQL_TOKEN_OF);  }
OFFSET              {SQL_NEW_KEYWORD(SQL_TOKEN_OFFSET);  }
OJ                  {SQL_NEW_KEYWORD(SQL_TOKEN_OJ);  }
OLD                 {SQL_NEW_KEYWORD(SQL_TOKEN_OLD);  }
ON                  {SQL_NEW_KEYWORD(SQL_TOKEN_ON);  }
ONLY                {SQL_NEW_KEYWORD(SQL_TOKEN_ONLY);  }
OPTION              {SQL_NEW_KEYWORD(SQL_TOKEN_OPTION);  }
OR                  {SQL_NEW_KEYWORD(SQL_TOKEN_OR);  }
ORDER               {SQL_NEW_KEYWORD(SQL_TOKEN_ORDER);  }
OTHERS              {SQL_NEW_KEYWORD(SQL_TOKEN_OTHERS);  }
OUTER               {SQL_NEW_KEYWORD(SQL_TOKEN_OUTER);  }
OVER                {SQL_NEW_KEYWORD(SQL_TOKEN_OVER);  }

PARTITION           {SQL_NEW_KEYWORD(SQL_TOKEN_PARTITION);  }
PERCENT_RANK        {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENT_RANK);  }
PERCENTILE_CONT     {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENTILE_CONT);  }
PERCENTILE_DISC     {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENTILE_DISC);  }
PI                  {SQL_NEW_KEYWORD(SQL_TOKEN_PI);  }
POSITION            {SQL_NEW_KEYWORD(SQL_TOKEN_POSITION);  }
POWER               {SQL_NEW_KEYWORD(SQL_TOKEN_POWER);  }
PRECEDING           {SQL_NEW_KEYWORD(SQL_TOKEN_PRECEDING);  }
PRECISION           {SQL_NEW_KEYWORD(SQL_TOKEN_PRECISION);  }
PRIMARY             {SQL_NEW_KEYWORD(SQL_TOKEN_PRIMARY);  }
PRIVILEGES          {SQL_NEW_KEYWORD(SQL_TOKEN_PRIVILEGES);  }
PROCEDURE           {SQL_NEW_KEYWORD(SQL_TOKEN_PROCEDURE);  }
PUBLIC              {SQL_NEW_KEYWORD(SQL_TOKEN_PUBLIC);  }

QUARTER             {SQL_NEW_KEYWORD(SQL_TOKEN_QUARTER);  }

RADIANS             {SQL_NEW_KEYWORD(SQL_TOKEN_RADIANS);  }
RAND                {SQL_NEW_KEYWORD(SQL_TOKEN_RAND);  }
RANGE               {SQL_NEW_KEYWORD(SQL_TOKEN_RANGE);  }
RANK                {SQL_NEW_KEYWORD(SQL_TOKEN_RANK);  }
REAL                {SQL_NEW_KEYWORD(SQL_TOKEN_REAL);  }
REFERENCES          {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCES);  }
REFERENCING         {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCING);  }
REPEAT              {SQL_NEW_KEYWORD(SQL_TOKEN_REPEAT);  }
REPLACE             {SQL_NEW_KEYWORD(SQL_TOKEN_REPLACE);  }
RESPECT             {SQL_NEW_KEYWORD(SQL_TOKEN_RESPECT);  }
ROLLBACK            {SQL_NEW_KEYWORD(SQL_TOKEN_ROLLBACK);  }
ROUND               {SQL_NEW_KEYWORD(SQL_TOKEN_ROUND);  }
ROUNDMAGIC          {SQL_NEW_KEYWORD(SQL_TOKEN_ROUNDMAGIC);  }
ROW                 {SQL_NEW_KEYWORD(SQL_TOKEN_ROW);  }
ROWS                {SQL_NEW_KEYWORD(SQL_TOKEN_ROWS);  }
ROW_NUMBER          {SQL_NEW_KEYWORD(SQL_TOKEN_ROW_NUMBER);  }
RIGHT               {SQL_NEW_KEYWORD(SQL_TOKEN_RIGHT);  }
RTRIM               {SQL_NEW_KEYWORD(SQL_TOKEN_RTRIM);  }

SCHEMA              {SQL_NEW_KEYWORD(SQL_TOKEN_SCHEMA);  }
SECOND              {SQL_NEW_KEYWORD(SQL_TOKEN_SECOND);  }
SELECT              {SQL_NEW_KEYWORD(SQL_TOKEN_SELECT);  }
SET                 {SQL_NEW_KEYWORD(SQL_TOKEN_SET);  }
SIZE                {SQL_NEW_KEYWORD(SQL_TOKEN_SIZE);  }
SIGN                {SQL_NEW_KEYWORD(SQL_TOKEN_SIGN);  }
SIN                 {SQL_NEW_KEYWORD(SQL_TOKEN_SIN);  }
SMALLINT            {SQL_NEW_KEYWORD(SQL_TOKEN_SMALLINT);  }
SOME                {SQL_NEW_KEYWORD(SQL_TOKEN_SOME);  }
SOUNDEX             {SQL_NEW_KEYWORD(SQL_TOKEN_SOUNDEX);  }
SPACE               {SQL_NEW_KEYWORD(SQL_TOKEN_SPACE);  }
SQRT                {SQL_NEW_KEYWORD(SQL_TOKEN_SQRT);  }
STDDEV_POP          {SQL_NEW_KEYWORD(SQL_TOKEN_STDDEV_POP);  }
STDDEV_SAMP         {SQL_NEW_KEYWORD(SQL_TOKEN_STDDEV_SAMP); }
STATEMENT           {SQL_NEW_KEYWORD(SQL_TOKEN_STATEMENT);  }
SUBSTRING           {SQL_NEW_KEYWORD(SQL_TOKEN_SUBSTRING);  }
SUM                 {SQL_NEW_KEYWORD(SQL_TOKEN_SUM);  }
SESSION_USER        {SQL_NEW_KEYWORD(SQL_TOKEN_SESSION_USER);  }
SYSTEM_USER         {SQL_NEW_KEYWORD(SQL_TOKEN_SYSTEM_USER);  }

TABLE               {SQL_NEW_KEYWORD(SQL_TOKEN_TABLE);  }
TAN                 {SQL_NEW_KEYWORD(SQL_TOKEN_TAN);  }
THEN                {SQL_NEW_KEYWORD(SQL_TOKEN_THEN);  }
TIES                {SQL_NEW_KEYWORD(SQL_TOKEN_TIES);  }
TIME                {SQL_NEW_KEYWORD(SQL_TOKEN_TIME);  }
TIMESTAMP           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMP);  }
TIMESTAMPADD        {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMPADD);  }
TIMESTAMPDIFF       {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMPDIFF);  }
TIMEVALUE           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEVALUE);  }
TIMEZONE_HOUR       {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEZONE_HOUR);  }
TIMEZONE_MINUTE     {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEZONE_MINUTE);  }
TO                  {SQL_NEW_KEYWORD(SQL_TOKEN_TO);  }
TRAILING            {SQL_NEW_KEYWORD(SQL_TOKEN_TRAILING);  }
TRANSLATE           {SQL_NEW_KEYWORD(SQL_TOKEN_TRANSLATE);  }
TRIGGER             {SQL_NEW_KEYWORD(SQL_TOKEN_TRIGGER);  }
TRIM                {SQL_NEW_KEYWORD(SQL_TOKEN_TRIM);  }
TRUE                {SQL_NEW_KEYWORD(SQL_TOKEN_TRUE);  }
TRUNCATE            {SQL_NEW_KEYWORD(SQL_TOKEN_TRUNCATE);  }
TS                  {SQL_NEW_KEYWORD(SQL_TOKEN_TS);  }
T                   {SQL_NEW_KEYWORD(SQL_TOKEN_T);  }

UCASE               {SQL_NEW_KEYWORD(SQL_TOKEN_UCASE);  }
UNBOUNDED           {SQL_NEW_KEYWORD(SQL_TOKEN_UNBOUNDED);  }
UNION               {SQL_NEW_KEYWORD(SQL_TOKEN_UNION);  }
UNIQUE              {SQL_NEW_KEYWORD(SQL_TOKEN_UNIQUE);  }
UNKNOWN             {SQL_NEW_KEYWORD(SQL_TOKEN_UNKNOWN);  }
UPDATE              {SQL_NEW_KEYWORD(SQL_TOKEN_UPDATE);  }
UPPER               {SQL_NEW_KEYWORD(SQL_TOKEN_UPPER);  }
USAGE               {SQL_NEW_KEYWORD(SQL_TOKEN_USAGE);  }
USER                {SQL_NEW_KEYWORD(SQL_TOKEN_USER);  }
USING               {SQL_NEW_KEYWORD(SQL_TOKEN_USING);  }

VARBINARY           {SQL_NEW_KEYWORD(SQL_TOKEN_VARBINARY);  }
VARCHAR             {SQL_NEW_KEYWORD(SQL_TOKEN_VARCHAR);  }
VARYING             {SQL_NEW_KEYWORD(SQL_TOKEN_VARYING);  }
VAR_POP             {SQL_NEW_KEYWORD(SQL_TOKEN_VAR_POP);  }
VAR_SAMP            {SQL_NEW_KEYWORD(SQL_TOKEN_VAR_SAMP);  }
VALUE               {SQL_NEW_KEYWORD(SQL_TOKEN_VALUE);  }
VALUES              {SQL_NEW_KEYWORD(SQL_TOKEN_VALUES);  }
VIEW                {SQL_NEW_KEYWORD(SQL_TOKEN_VIEW);  }

WEEK                {SQL_NEW_KEYWORD(SQL_TOKEN_WEEK);  }
WEEKDAY             {SQL_NEW_KEYWORD(SQL_TOKEN_WEEKDAY);  }
WHEN                {SQL_NEW_KEYWORD(SQL_TOKEN_WHEN);  }
WHERE               {SQL_NEW_KEYWORD(SQL_TOKEN_WHERE);  }
WITH                {SQL_NEW_KEYWORD(SQL_TOKEN_WITH);  }
WITHIN              {SQL_NEW_KEYWORD(SQL_TOKEN_WITHIN);  }
WITHOUT             {SQL_NEW_KEYWORD(SQL_TOKEN_WITHOUT);  }
WORK                {SQL_NEW_KEYWORD(SQL_TOKEN_WORK);  }

YEAR                {SQL_NEW_KEYWORD(SQL_TOKEN_YEAR);  }
YEARDAY             {SQL_NEW_KEYWORD(SQL_TOKEN_YEARDAY);  }

ZONE                {SQL_NEW_KEYWORD(SQL_TOKEN_ZONE);  }

"<"                 { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::Less);return SQL_LESS;}
">"                 { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::Great);return SQL_GREAT;}
"="                 { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::Equal);return SQL_EQUAL;}
"<="                { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::LessEq);return SQL_LESSEQ;}
">="                { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::GreatEq);return SQL_GREATEQ;}
"<>"                { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::NotEqual);return SQL_NOTEQUAL;}
"!="                { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::NotEqual);return SQL_NOTEQUAL;}
"||"                { SQL_NEW_NODE(OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQLNodeType::Concat);return SQL_CONCAT;}


[-+*/:(),.;?{}]    { return SQLyytext[0]; }


<SQL>[A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375][A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375_0-9]*      {return gatherName( SQLyytext);}

<SQL>([0-9]+)                   {SQL_NEW_INTNUM; }

<SQL>("."[0-9]*) |
<SQL>([0-9]+"."[0-9]*) |
<SQL>[0-9]+[eE][+-]?[0-9]+   |
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]*[eE][+-]?[0-9]+       {SQL_NEW_APPROXNUM;  }

<PREDICATE_GER,PREDICATE_ENG,DATE>[A-Za-z\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375][A-Za-z0-9_%.,*?\200-\277\300-\337\340-\357\360-\367\370-\373\374-\375]* {return gatherNamePre(SQLyytext);}

<PREDICATE_GER,PREDICATE_ENG>([0-9]+)       {SQL_NEW_INTNUM; }
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+)   {SQL_NEW_INTNUM; }
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+)   {SQL_NEW_INTNUM; }

<PREDICATE_ENG>([0-9]+"."[0-9]+) |
<PREDICATE_ENG>([0-9]{1,3}(","[0-9]{3})+"."[0-9]+) |
<PREDICATE_ENG>("."[0-9]+)                  {SQL_NEW_APPROXNUM;  }
<PREDICATE_ENG>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_ENG>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_ENG>"."[0-9]*[eE][+-]?[0-9]+     {SQL_NEW_APPROXNUM;  }

<PREDICATE_GER>([0-9]+","[0-9]+) |
<PREDICATE_GER>([0-9]{1,3}("."[0-9]{3})+","[0-9]+) |
<PREDICATE_GER>(","[0-9]+)                  {SQL_NEW_APPROXNUM;  }
<PREDICATE_GER>[0-9]+[eE][+-]?[0-9]+   |
<PREDICATE_GER>[0-9]+","[0-9]*[eE][+-]?[0-9]+ |
<PREDICATE_GER>","[0-9]*[eE][+-]?[0-9]+     {SQL_NEW_APPROXNUM;  }

<PREDICATE_GER,PREDICATE_ENG>[0-9.,][A-Za-z0-9_.,%]* {return gatherNamePre(SQLyytext);}

<SQL>\" { return gatherString('\"',0); }
<SQL>`  { return gatherString('`' ,0); }

<PREDICATE_GER,PREDICATE_ENG,DATE,SQL>"["       { return gatherString(']' ,0);}

\'      { return gatherString('\'',1); }

<PREDICATE_GER,PREDICATE_ENG,DATE># { return gatherString('#' ,2); }

<DATE>[0-9]{1,4}[^ ]*[0-9] |
<DATE>[0-9]{1,4}[^ ]*[0-9][ ][0-9]{1,4}[^ ]*[0-9]       { SQL_NEW_DATE; }

<STRING>["-""+""*""/"":""("")"",""."";""?""{""}"]       { return SQLyytext[0]; } /*  */
<STRING>"["                 { return gatherString(']' ,0); }
<STRING>[^ ':["?"]*         { return gatherNamePre(SQLyytext); }

\n              {}

[ \t\r]+        {}

"--".*$         {}

.               {YY_FATAL_ERROR("Invalid symbol"); return SQL_TOKEN_INVALIDSYMBOL;}

%%

// Kludge around a bug (well, Posix incompatibility) in flex 2.5.x
// http://bugs.debian.org/cgi-bin/bugreport.cgi?archive=no&bug=189332
#if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5

  #ifndef YY_FLUSH_BUFFER
    #define YY_FLUSH_BUFFER SQLyy_flush_buffer(YY_CURRENT_BUFFER )
  #endif

  #ifndef yytext_ptr
    #define yytext_ptr SQLyytext
  #endif

#endif

// Versions of flex apparently differ in whether input() resp. yyinput() returns
// zero or EOF upon end of file:
inline bool checkeof(int c) { return c == 0 || c == EOF; }

/*
 * Read SQL string literal
 * Valid strings:
 *  ''  'a string'  'quote '' within string'
 *  ""  "a string"  "quote "" within string"
 * nTyp == 0 -> SQLNodeType::Name
 * nTyp == 1 -> SQLNodeType::String
 * nTyp == 2 -> SQLNodeType::AccessDate
 */
sal_Int32 gatherString(int delim, sal_Int32 nTyp)
{
    int ch;
    OStringBuffer sBuffer(256);

    while (!checkeof(ch = yyinput()))
    {
        if (ch == delim)
        {
            if ((ch = yyinput()) != delim)
            {
                if (!checkeof(ch))
                    unput(ch);

                switch(nTyp)
                {
                case 0:
                    SQL_NEW_NODE(OStringToOUString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8), SQLNodeType::Name);
                    return SQL_TOKEN_NAME;
                case 1:
                    SQL_NEW_NODE(OStringToOUString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8), SQLNodeType::String);
                    return SQL_TOKEN_STRING;
                case 2:
                    SQL_NEW_NODE(OStringToOUString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8), SQLNodeType::AccessDate);
                    return SQL_TOKEN_ACCESS_DATE;
                }
            }
            else
            {
                sBuffer.append(static_cast<sal_Char>(ch));
            }

        }
        else if (nTyp != 1 && (ch == '\r' || ch == '\n') )
            break;
        else
        {
            sBuffer.append(static_cast<sal_Char>(ch));
        }
    }
    YY_FATAL_ERROR("Unterminated name string");
    return SQL_TOKEN_INVALIDSYMBOL;
}

sal_Int32 mapEnumToToken(IParseContext::InternationalKeyCode _eKeyCode )
{
    sal_Int32 nTokenID = 0;
    switch( _eKeyCode )
    {
        case IParseContext::InternationalKeyCode::Like:       nTokenID = SQL_TOKEN_LIKE;      break;
        case IParseContext::InternationalKeyCode::Not:        nTokenID = SQL_TOKEN_NOT;       break;
        case IParseContext::InternationalKeyCode::Null:       nTokenID = SQL_TOKEN_NULL;      break;
        case IParseContext::InternationalKeyCode::True:       nTokenID = SQL_TOKEN_TRUE;      break;
        case IParseContext::InternationalKeyCode::False:      nTokenID = SQL_TOKEN_FALSE;     break;
        case IParseContext::InternationalKeyCode::Is:         nTokenID = SQL_TOKEN_IS;        break;
        case IParseContext::InternationalKeyCode::Between:    nTokenID = SQL_TOKEN_BETWEEN;   break;
        case IParseContext::InternationalKeyCode::Or:         nTokenID = SQL_TOKEN_OR;        break;
        case IParseContext::InternationalKeyCode::And:        nTokenID = SQL_TOKEN_AND;       break;
        case IParseContext::InternationalKeyCode::Avg:        nTokenID = SQL_TOKEN_AVG;       break;
        case IParseContext::InternationalKeyCode::Count:      nTokenID = SQL_TOKEN_COUNT;     break;
        case IParseContext::InternationalKeyCode::Max:        nTokenID = SQL_TOKEN_MAX;       break;
        case IParseContext::InternationalKeyCode::Min:        nTokenID = SQL_TOKEN_MIN;       break;
        case IParseContext::InternationalKeyCode::Sum:        nTokenID = SQL_TOKEN_SUM;       break;
        case IParseContext::InternationalKeyCode::Every:      nTokenID = SQL_TOKEN_EVERY;     break;
        case IParseContext::InternationalKeyCode::Any:        nTokenID = SQL_TOKEN_ANY;       break;
        case IParseContext::InternationalKeyCode::Some:       nTokenID = SQL_TOKEN_SOME;      break;
        case IParseContext::InternationalKeyCode::StdDevPop: nTokenID = SQL_TOKEN_STDDEV_POP; break;
        case IParseContext::InternationalKeyCode::StdDevSamp: nTokenID = SQL_TOKEN_STDDEV_SAMP; break;
        case IParseContext::InternationalKeyCode::VarSamp:   nTokenID = SQL_TOKEN_VAR_SAMP;  break;
        case IParseContext::InternationalKeyCode::VarPop:    nTokenID = SQL_TOKEN_VAR_POP;   break;
        case IParseContext::InternationalKeyCode::Collect:    nTokenID = SQL_TOKEN_COLLECT;   break;
        case IParseContext::InternationalKeyCode::Fusion:     nTokenID = SQL_TOKEN_FUSION;    break;
        case IParseContext::InternationalKeyCode::Intersection: nTokenID = SQL_TOKEN_INTERSECTION; break;
        default:
            OSL_FAIL( "mapEnumToToken: unsupported key!" );
    }
    return nTokenID;
}
/*
 * Read SQL Name literal
 * Valid Names or international keywords:
 *  As we have international keywords, we test first on them
 */
sal_Int32 gatherName(const sal_Char* text)
{
    sal_Int32 nToken;
    OSL_ENSURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
    IParseContext::InternationalKeyCode eKeyCode = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
    switch (eKeyCode)
    {
        case IParseContext::InternationalKeyCode::Like:
        case IParseContext::InternationalKeyCode::Not:
        case IParseContext::InternationalKeyCode::Null:
        case IParseContext::InternationalKeyCode::True:
        case IParseContext::InternationalKeyCode::False:
        case IParseContext::InternationalKeyCode::Is:
        case IParseContext::InternationalKeyCode::Between:
        case IParseContext::InternationalKeyCode::Or:
        case IParseContext::InternationalKeyCode::And:
        case IParseContext::InternationalKeyCode::Count:
        case IParseContext::InternationalKeyCode::Avg:
        case IParseContext::InternationalKeyCode::Max:
        case IParseContext::InternationalKeyCode::Min:
        case IParseContext::InternationalKeyCode::Sum:
        case IParseContext::InternationalKeyCode::Every:
        case IParseContext::InternationalKeyCode::Any:
        case IParseContext::InternationalKeyCode::Some:
        case IParseContext::InternationalKeyCode::StdDevPop:
        case IParseContext::InternationalKeyCode::StdDevSamp:
        case IParseContext::InternationalKeyCode::VarSamp:
        case IParseContext::InternationalKeyCode::VarPop:
        case IParseContext::InternationalKeyCode::Collect:
        case IParseContext::InternationalKeyCode::Fusion:
        case IParseContext::InternationalKeyCode::Intersection:
            nToken = mapEnumToToken(eKeyCode);
            SQL_NEW_KEYWORD(nToken);
            break;
        default:
            SQL_NEW_NODE(OUString(text,strlen(text),RTL_TEXTENCODING_UTF8), SQLNodeType::Name);
            return SQL_TOKEN_NAME;
    }
}
/**
 Read SQL Name literal for predicate check
 Valid Names or international keywords:
 As we have international keywords, we test first on them
*/
sal_Int32 gatherNamePre(const sal_Char* text)
{
    sal_Int32 nToken;
    OSL_ENSURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
    IParseContext::InternationalKeyCode eKeyCode = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
    switch (eKeyCode)
    {
        case IParseContext::InternationalKeyCode::Like:
        case IParseContext::InternationalKeyCode::Not:
        case IParseContext::InternationalKeyCode::Null:
        case IParseContext::InternationalKeyCode::True:
        case IParseContext::InternationalKeyCode::False:
        case IParseContext::InternationalKeyCode::Is:
        case IParseContext::InternationalKeyCode::Between:
        case IParseContext::InternationalKeyCode::Or:
        case IParseContext::InternationalKeyCode::And:
        case IParseContext::InternationalKeyCode::Count:
        case IParseContext::InternationalKeyCode::Avg:
        case IParseContext::InternationalKeyCode::Max:
        case IParseContext::InternationalKeyCode::Min:
        case IParseContext::InternationalKeyCode::Sum:
        case IParseContext::InternationalKeyCode::Every:
        case IParseContext::InternationalKeyCode::Any:
        case IParseContext::InternationalKeyCode::Some:
        case IParseContext::InternationalKeyCode::StdDevPop:
        case IParseContext::InternationalKeyCode::StdDevSamp:
        case IParseContext::InternationalKeyCode::VarSamp:
        case IParseContext::InternationalKeyCode::VarPop:
        case IParseContext::InternationalKeyCode::Collect:
        case IParseContext::InternationalKeyCode::Fusion:
        case IParseContext::InternationalKeyCode::Intersection:
            nToken = mapEnumToToken(eKeyCode);
            SQL_NEW_KEYWORD(nToken);
            break;
        default:
        // we need a special handling for parameter
        {
            OString sStmt = xxx_pGLOBAL_SQLSCAN->getStatement();
            sal_Int32 nLength = strlen(text);
            sal_Int32 nPos = xxx_pGLOBAL_SQLSCAN->GetCurrentPos() - nLength - 2;
            if (sStmt.getStr()[nPos] == ':')
            {
                SQL_NEW_NODE(OUString(text,nLength,RTL_TEXTENCODING_UTF8), SQLNodeType::Name);
                nToken = SQL_TOKEN_NAME;
            }
            else
            {
                SQL_NEW_NODE(OUString(text,nLength,RTL_TEXTENCODING_UTF8), SQLNodeType::String);
                nToken = SQL_TOKEN_STRING;
            }
        }
    }
    return nToken;
}

using namespace connectivity;

static bool IN_SQLyyerror;
//------------------------------------------------------------------------------
OSQLScanner::OSQLScanner()
            : m_pContext(nullptr)
            , m_nCurrentPos(0)
            , m_bInternational(false)
            , m_nRule(0) // 0 is INITIAL
{
    IN_SQLyyerror = false;
}

//------------------------------------------------------------------------------
OSQLScanner::~OSQLScanner()
{
}
//------------------------------------------------------------------------------
void OSQLScanner::SQLyyerror(char const *fmt)
{

    if(IN_SQLyyerror)
        return;
    IN_SQLyyerror = true;

    OSL_ENSURE(m_pContext, "OSQLScanner::SQLyyerror: No Context set");
    m_sErrorMessage = OUString(fmt,strlen(fmt),RTL_TEXTENCODING_UTF8);
    if (m_nCurrentPos < m_sStatement.getLength())
    {
        m_sErrorMessage += ": ";

        OUString aError;
        OUStringBuffer Buffer(256);

        int ch = SQLyytext ? (SQLyytext[0] == 0 ? ' ' : SQLyytext[0]): ' ';
        Buffer.append((sal_Unicode)ch);
        while (!checkeof(ch = yyinput()))
        {
            if (ch == ' ')
            {
                if ((ch = yyinput()) != ' ')
                {
                    if (!checkeof(ch))
                        unput(ch);
                }
                aError = Buffer.makeStringAndClear();
                break;
            }
            else
            {
                Buffer.append((sal_Unicode)ch);
            }
        }
        m_sErrorMessage += aError;
    }
    IN_SQLyyerror = false;
    YY_FLUSH_BUFFER;
}

//------------------------------------------------------------------------------
void OSQLScanner::prepareScan(const OUString & rNewStatement, const IParseContext* pContext, bool bInternational)
{
    YY_FLUSH_BUFFER;
    BEGIN(m_nRule);

    m_sErrorMessage = OUString();
    m_sStatement = OUStringToOString(rNewStatement, RTL_TEXTENCODING_UTF8);
    m_nCurrentPos = 0;
    m_bInternational = bInternational;
    m_pContext = pContext;
}

//------------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLyygetc(void)
{
    sal_Int32 nPos = (m_nCurrentPos >= m_sStatement.getLength()) ? EOF : m_sStatement.getStr()[m_nCurrentPos];
    m_nCurrentPos++;
    return nPos;
}

//------------------------------------------------------------------------------
IParseContext::InternationalKeyCode OSQLScanner::getInternationalTokenID(const sal_Char* sToken) const
{
    OSL_ENSURE(m_pContext, "OSQLScanner::getInternationalTokenID: No Context set");
    return (m_bInternational) ? m_pContext->getIntlKeyCode(OString(sToken) ) : IParseContext::InternationalKeyCode::None;
}
sal_Int32   OSQLScanner::GetGERRule()       { return PREDICATE_GER; }
sal_Int32   OSQLScanner::GetENGRule()       { return PREDICATE_ENG; }
sal_Int32   OSQLScanner::GetSQLRule()       { return SQL; }
sal_Int32   OSQLScanner::GetDATERule()  { return DATE; }
sal_Int32   OSQLScanner::GetSTRINGRule()    { return STRING; }
void OSQLScanner::setScanner(bool _bNull)
{
    xxx_pGLOBAL_SQLSCAN = _bNull ? nullptr : this;
}
sal_Int32 OSQLScanner::SQLlex()
{
    return SQLyylex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
