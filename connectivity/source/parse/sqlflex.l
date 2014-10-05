%{

//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file incorporates work covered by the following license notice:
//
//   Licensed to the Apache Software Foundation (ASF) under one or more
//   contributor license agreements. See the NOTICE file distributed
//   with this work for additional information regarding copyright
//   ownership. The ASF licenses this file to you under the Apache
//   License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of
//   the License at http://www.apache.org/licenses/LICENSE-2.0 .
//

#include "sal/config.h"

#define YY_EXIT 1               // YY_FATAL will not halt the application

#ifndef _CSTDARG_
#include <cstdarg>
#endif

#include <string.h>

#include <connectivity/internalnode.hxx>

#ifndef _CONNECTIVITY_SQLYACC_HXX
#define _CONNECTIVITY_SQLYACC_HXX

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
#pragma warning(push, 1)
/**/
#ifdef yywrap
#undef  yywrap
#define yywrap() 1
#endif
/**/
#endif
#define YY_NO_UNISTD_H

using namespace connectivity;

//=============================================================================
//
// Erzeugung der Blaetter fuer die Token
// Blaetter werden generell vom Lexer erzeugt

static sal_Int32    gatherString(int delim, sal_Int32 nTyp);
static sal_Int32    gatherName(const sal_Char*);
static sal_Int32    gatherNamePre(const sal_Char* );
// has to be set before the parser starts
OSQLScanner* xxx_pGLOBAL_SQLSCAN = NULL;

#define SQL_NEW_NODE(text, token)   \
        SQLyylval.pParseNode = new OSQLInternalNode(text, token);

#define SQL_NEW_KEYWORD(token)                      \
        SQLyylval.pParseNode = new OSQLInternalNode("", SQL_NODE_KEYWORD, (token));       return token;

#define SQL_NEW_INTNUM      SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_INTNUM); return SQL_TOKEN_INTNUM;
#define SQL_NEW_APPROXNUM   SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_APPROXNUM); return SQL_TOKEN_APPROXNUM;
#define SQL_NEW_DATE        SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_ACCESS_DATE); return SQL_TOKEN_ACCESS_DATE;

#define YY_INPUT(buf,result,max_size)               \
{                                                   \
    int c = xxx_pGLOBAL_SQLSCAN->SQLyygetc();       \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1);\
}

#define YY_FATAL_ERROR(msg)                         \
{                                                   \
    xxx_pGLOBAL_SQLSCAN->SQLyyerror(msg);           \
}

//
//=============================================================================

%}

%s SQL
%s PREDICATE_ENG
%s PREDICATE_GER
%s DATE
%s STRING

%option noyywrap
%option never-interactive
%%

abs                 {SQL_NEW_KEYWORD(SQL_TOKEN_ABS);  }
acos                {SQL_NEW_KEYWORD(SQL_TOKEN_ACOS);  }
after               {SQL_NEW_KEYWORD(SQL_TOKEN_AFTER);  }
all                 {SQL_NEW_KEYWORD(SQL_TOKEN_ALL);  }
alter               {SQL_NEW_KEYWORD(SQL_TOKEN_ALTER);  }
and                 {SQL_NEW_KEYWORD(SQL_TOKEN_AND);  }
any                 {SQL_NEW_KEYWORD(SQL_TOKEN_ANY);  }
array_agg           {SQL_NEW_KEYWORD(SQL_TOKEN_ARRAY_AGG);  }
as                  {SQL_NEW_KEYWORD(SQL_TOKEN_AS);  }
asc                 {SQL_NEW_KEYWORD(SQL_TOKEN_ASC);  }
ascii               {SQL_NEW_KEYWORD(SQL_TOKEN_ASCII);  }
asin                {SQL_NEW_KEYWORD(SQL_TOKEN_ASIN);  }
at                  {SQL_NEW_KEYWORD(SQL_TOKEN_AT);  }
atan                {SQL_NEW_KEYWORD(SQL_TOKEN_ATAN);  }
atan2               {SQL_NEW_KEYWORD(SQL_TOKEN_ATAN2);  }
atomic              {SQL_NEW_KEYWORD(SQL_TOKEN_ATOMIC);  }
authorization       {SQL_NEW_KEYWORD(SQL_TOKEN_AUTHORIZATION);  }
avg                 {SQL_NEW_KEYWORD(SQL_TOKEN_AVG);  }

before              {SQL_NEW_KEYWORD(SQL_TOKEN_BEFORE);  }
begin               {SQL_NEW_KEYWORD(SQL_TOKEN_BEGIN);  }
between             {SQL_NEW_KEYWORD(SQL_TOKEN_BETWEEN);  }
bigint              {SQL_NEW_KEYWORD(SQL_TOKEN_BIGINT);  }
binary              {SQL_NEW_KEYWORD(SQL_TOKEN_BINARY);  }
bit                 {SQL_NEW_KEYWORD(SQL_TOKEN_BIT);  }
bit_length          {SQL_NEW_KEYWORD(SQL_TOKEN_BIT_LENGTH);  }
blob                {SQL_NEW_KEYWORD(SQL_TOKEN_BLOB);  }
both                {SQL_NEW_KEYWORD(SQL_TOKEN_BOTH);  }
by                  {SQL_NEW_KEYWORD(SQL_TOKEN_BY);  }

call                {SQL_NEW_KEYWORD(SQL_TOKEN_CALL);  }
case                {SQL_NEW_KEYWORD(SQL_TOKEN_CASE);  }
cast                {SQL_NEW_KEYWORD(SQL_TOKEN_CAST);  }
ceiling             {SQL_NEW_KEYWORD(SQL_TOKEN_CEILING);  }
char                {SQL_NEW_KEYWORD(SQL_TOKEN_CHAR);  }
character           {SQL_NEW_KEYWORD(SQL_TOKEN_CHARACTER);  }
char(acter)?_length {SQL_NEW_KEYWORD(SQL_TOKEN_CHAR_LENGTH);  }
check               {SQL_NEW_KEYWORD(SQL_TOKEN_CHECK);  }
clob                {SQL_NEW_KEYWORD(SQL_TOKEN_CLOB);  }
coalesce            {SQL_NEW_KEYWORD(SQL_TOKEN_COALESCE);  }
collate             {SQL_NEW_KEYWORD(SQL_TOKEN_COLLATE);  }
collect             {SQL_NEW_KEYWORD(SQL_TOKEN_COLLECT);  }
commit              {SQL_NEW_KEYWORD(SQL_TOKEN_COMMIT);  }
concat              {SQL_NEW_KEYWORD(SQL_TOKEN_CONCAT);  }
continue            {SQL_NEW_KEYWORD(SQL_TOKEN_CONTINUE);  }
convert             {SQL_NEW_KEYWORD(SQL_TOKEN_CONVERT);  }
cos                 {SQL_NEW_KEYWORD(SQL_TOKEN_COS);  }
cot                 {SQL_NEW_KEYWORD(SQL_TOKEN_COT);  }
count               {SQL_NEW_KEYWORD(SQL_TOKEN_COUNT);  }
create              {SQL_NEW_KEYWORD(SQL_TOKEN_CREATE);  }
cross               {SQL_NEW_KEYWORD(SQL_TOKEN_CROSS);  }
cume_rank           {SQL_NEW_KEYWORD(SQL_TOKEN_CUME_DIST);  }
current             {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT);  }
current_date        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DATE);  }
current_catalog                 {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_CATALOG);  }
current_default_transform_group {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP);  }
current_path                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_PATH);  }
current_role                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_ROLE);  }
current_schema                  {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_SCHEMA);  }
current_user                    {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_USER);  }
curdate             {SQL_NEW_KEYWORD(SQL_TOKEN_CURDATE);  }
current_time        {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIME);  }
curtime             {SQL_NEW_KEYWORD(SQL_TOKEN_CURTIME);  }
current_timestamp   {SQL_NEW_KEYWORD(SQL_TOKEN_CURRENT_TIMESTAMP);  }
cursor              {SQL_NEW_KEYWORD(SQL_TOKEN_CURSOR);  }

d                   {SQL_NEW_KEYWORD(SQL_TOKEN_D);  }
date                {SQL_NEW_KEYWORD(SQL_TOKEN_DATE);  }
datediff            {SQL_NEW_KEYWORD(SQL_TOKEN_DATEDIFF);  }
datevalue           {SQL_NEW_KEYWORD(SQL_TOKEN_DATEVALUE);  }
day                 {SQL_NEW_KEYWORD(SQL_TOKEN_DAY);  }
dayname             {SQL_NEW_KEYWORD(SQL_TOKEN_DAYNAME);  }
dayofmonth          {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFMONTH);  }
dayofweek           {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFWEEK);  }
dayofyear           {SQL_NEW_KEYWORD(SQL_TOKEN_DAYOFYEAR);  }
dec                 {SQL_NEW_KEYWORD(SQL_TOKEN_DEC);  }
decimal             {SQL_NEW_KEYWORD(SQL_TOKEN_DECIMAL);  }
declare             {SQL_NEW_KEYWORD(SQL_TOKEN_DECLARE);  }
default             {SQL_NEW_KEYWORD(SQL_TOKEN_DEFAULT);  }
degrees             {SQL_NEW_KEYWORD(SQL_TOKEN_DEGREES);  }
delete              {SQL_NEW_KEYWORD(SQL_TOKEN_DELETE);  }
dense_rank          {SQL_NEW_KEYWORD(SQL_TOKEN_DENSE_RANK);  }
desc                {SQL_NEW_KEYWORD(SQL_TOKEN_DESC);  }
difference          {SQL_NEW_KEYWORD(SQL_TOKEN_DIFFERENCE);  }
distinct            {SQL_NEW_KEYWORD(SQL_TOKEN_DISTINCT);  }
double              {SQL_NEW_KEYWORD(SQL_TOKEN_DOUBLE);  }
drop                {SQL_NEW_KEYWORD(SQL_TOKEN_DROP);  }

each                {SQL_NEW_KEYWORD(SQL_TOKEN_EACH);  }
else                {SQL_NEW_KEYWORD(SQL_TOKEN_ELSE);  }
end                 {SQL_NEW_KEYWORD(SQL_TOKEN_END);  }
every               {SQL_NEW_KEYWORD(SQL_TOKEN_EVERY);  }
escape              {SQL_NEW_KEYWORD(SQL_TOKEN_ESCAPE);  }
except              {SQL_NEW_KEYWORD(SQL_TOKEN_EXCEPT);  }
exclude             {SQL_NEW_KEYWORD(SQL_TOKEN_EXCLUDE);  }
exists              {SQL_NEW_KEYWORD(SQL_TOKEN_EXISTS);  }
exp                 {SQL_NEW_KEYWORD(SQL_TOKEN_EXP);  }
extract             {SQL_NEW_KEYWORD(SQL_TOKEN_EXTRACT);  }

false               {SQL_NEW_KEYWORD(SQL_TOKEN_FALSE);  }
fetch               {SQL_NEW_KEYWORD(SQL_TOKEN_FETCH);  }
first               {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST);  }
first_value         {SQL_NEW_KEYWORD(SQL_TOKEN_FIRST_VALUE);  }
float               {SQL_NEW_KEYWORD(SQL_TOKEN_FLOAT);  }
floor               {SQL_NEW_KEYWORD(SQL_TOKEN_FLOOR);  }
fn                  {SQL_NEW_KEYWORD(SQL_TOKEN_FN);  }
following           {SQL_NEW_KEYWORD(SQL_TOKEN_FOLLOWING);  }
for                 {SQL_NEW_KEYWORD(SQL_TOKEN_FOR);  }
foreign             {SQL_NEW_KEYWORD(SQL_TOKEN_FOREIGN);  }
found               {SQL_NEW_KEYWORD(SQL_TOKEN_FOUND);  }
from                {SQL_NEW_KEYWORD(SQL_TOKEN_FROM);  }
full                {SQL_NEW_KEYWORD(SQL_TOKEN_FULL);  }
fusion              {SQL_NEW_KEYWORD(SQL_TOKEN_FUSION);  }

grant               {SQL_NEW_KEYWORD(SQL_TOKEN_GRANT);  }
group               {SQL_NEW_KEYWORD(SQL_TOKEN_GROUP);  }

having              {SQL_NEW_KEYWORD(SQL_TOKEN_HAVING);  }
hour                {SQL_NEW_KEYWORD(SQL_TOKEN_HOUR);  }

ignore              {SQL_NEW_KEYWORD(SQL_TOKEN_IGNORE);  }
in                  {SQL_NEW_KEYWORD(SQL_TOKEN_IN);  }
inner               {SQL_NEW_KEYWORD(SQL_TOKEN_INNER);  }
insert              {SQL_NEW_KEYWORD(SQL_TOKEN_INSERT);  }
instead             {SQL_NEW_KEYWORD(SQL_TOKEN_INSTEAD);  }
int(eger)?          {SQL_NEW_KEYWORD(SQL_TOKEN_INTEGER);  }
intersect           {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECT);  }
interval            {SQL_NEW_KEYWORD(SQL_TOKEN_INTERVAL);  }
intersection        {SQL_NEW_KEYWORD(SQL_TOKEN_INTERSECTION);  }
into                {SQL_NEW_KEYWORD(SQL_TOKEN_INTO);  }
is                  {SQL_NEW_KEYWORD(SQL_TOKEN_IS);  }

join                {SQL_NEW_KEYWORD(SQL_TOKEN_JOIN);  }

key                 {SQL_NEW_KEYWORD(SQL_TOKEN_KEY);  }

lag                 {SQL_NEW_KEYWORD(SQL_TOKEN_LAG);  }
large               {SQL_NEW_KEYWORD(SQL_TOKEN_LARGE);  }
last                {SQL_NEW_KEYWORD(SQL_TOKEN_LAST);  }
last_value          {SQL_NEW_KEYWORD(SQL_TOKEN_LAST_VALUE);  }
lcase               {SQL_NEW_KEYWORD(SQL_TOKEN_LCASE);  }
lead                {SQL_NEW_KEYWORD(SQL_TOKEN_LEAD);  }
leading             {SQL_NEW_KEYWORD(SQL_TOKEN_LEADING);  }
left                {SQL_NEW_KEYWORD(SQL_TOKEN_LEFT);  }
length              {SQL_NEW_KEYWORD(SQL_TOKEN_LENGTH);  }
like                {SQL_NEW_KEYWORD(SQL_TOKEN_LIKE);  }
limit               {SQL_NEW_KEYWORD(SQL_TOKEN_LIMIT);  }
ln                  {SQL_NEW_KEYWORD(SQL_TOKEN_LN);  }
local               {SQL_NEW_KEYWORD(SQL_TOKEN_LOCAL);  }
locate              {SQL_NEW_KEYWORD(SQL_TOKEN_LOCATE);  }
log                 {SQL_NEW_KEYWORD(SQL_TOKEN_LOG);  }
logf                {SQL_NEW_KEYWORD(SQL_TOKEN_LOGF);  }
log10               {SQL_NEW_KEYWORD(SQL_TOKEN_LOG10);  }
lower               {SQL_NEW_KEYWORD(SQL_TOKEN_LOWER);  }
ltrim               {SQL_NEW_KEYWORD(SQL_TOKEN_LTRIM);  }

max                 {SQL_NEW_KEYWORD(SQL_TOKEN_MAX);  }
min                 {SQL_NEW_KEYWORD(SQL_TOKEN_MIN);  }
minute              {SQL_NEW_KEYWORD(SQL_TOKEN_MINUTE);  }
mod                 {SQL_NEW_KEYWORD(SQL_TOKEN_MOD);  }
month               {SQL_NEW_KEYWORD(SQL_TOKEN_MONTH);  }
monthname           {SQL_NEW_KEYWORD(SQL_TOKEN_MONTHNAME);  }

national            {SQL_NEW_KEYWORD(SQL_TOKEN_NATIONAL);  }
natural             {SQL_NEW_KEYWORD(SQL_TOKEN_NATURAL);  }
nchar               {SQL_NEW_KEYWORD(SQL_TOKEN_NCHAR);  }
nclob               {SQL_NEW_KEYWORD(SQL_TOKEN_NCLOB);  }
new                 {SQL_NEW_KEYWORD(SQL_TOKEN_NEW);  }
next                {SQL_NEW_KEYWORD(SQL_TOKEN_NEXT);  }
no                  {SQL_NEW_KEYWORD(SQL_TOKEN_NO);  }
not                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOT);  }
now                 {SQL_NEW_KEYWORD(SQL_TOKEN_NOW);  }
nth_value           {SQL_NEW_KEYWORD(SQL_TOKEN_NTH_VALUE);  }
ntile               {SQL_NEW_KEYWORD(SQL_TOKEN_NTILE);  }
null                {SQL_NEW_KEYWORD(SQL_TOKEN_NULL);  }
nullif              {SQL_NEW_KEYWORD(SQL_TOKEN_NULLIF);  }
nulls               {SQL_NEW_KEYWORD(SQL_TOKEN_NULLS);  }
numeric             {SQL_NEW_KEYWORD(SQL_TOKEN_NUMERIC);  }

object              {SQL_NEW_KEYWORD(SQL_TOKEN_OBJECT);  }
octet_length        {SQL_NEW_KEYWORD(SQL_TOKEN_OCTET_LENGTH);  }
of                  {SQL_NEW_KEYWORD(SQL_TOKEN_OF);  }
offset              {SQL_NEW_KEYWORD(SQL_TOKEN_OFFSET);  }
oj                  {SQL_NEW_KEYWORD(SQL_TOKEN_OJ);  }
old                 {SQL_NEW_KEYWORD(SQL_TOKEN_OLD);  }
on                  {SQL_NEW_KEYWORD(SQL_TOKEN_ON);  }
only                {SQL_NEW_KEYWORD(SQL_TOKEN_ONLY);  }
option              {SQL_NEW_KEYWORD(SQL_TOKEN_OPTION);  }
or                  {SQL_NEW_KEYWORD(SQL_TOKEN_OR);  }
order               {SQL_NEW_KEYWORD(SQL_TOKEN_ORDER);  }
others              {SQL_NEW_KEYWORD(SQL_TOKEN_OTHERS);  }
outer               {SQL_NEW_KEYWORD(SQL_TOKEN_OUTER);  }
over                {SQL_NEW_KEYWORD(SQL_TOKEN_OVER);  }

partition           {SQL_NEW_KEYWORD(SQL_TOKEN_PARTITION);  }
percent_rank        {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENT_RANK);  }
percentile_cont     {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENTILE_CONT);  }
percentile_disc     {SQL_NEW_KEYWORD(SQL_TOKEN_PERCENTILE_DISC);  }
pi                  {SQL_NEW_KEYWORD(SQL_TOKEN_PI);  }
position            {SQL_NEW_KEYWORD(SQL_TOKEN_POSITION);  }
power               {SQL_NEW_KEYWORD(SQL_TOKEN_POWER);  }
preceding           {SQL_NEW_KEYWORD(SQL_TOKEN_PRECEDING);  }
precision           {SQL_NEW_KEYWORD(SQL_TOKEN_PRECISION);  }
primary             {SQL_NEW_KEYWORD(SQL_TOKEN_PRIMARY);  }
privileges          {SQL_NEW_KEYWORD(SQL_TOKEN_PRIVILEGES);  }
procedure           {SQL_NEW_KEYWORD(SQL_TOKEN_PROCEDURE);  }
public              {SQL_NEW_KEYWORD(SQL_TOKEN_PUBLIC);  }

quarter             {SQL_NEW_KEYWORD(SQL_TOKEN_QUARTER);  }

radians             {SQL_NEW_KEYWORD(SQL_TOKEN_RADIANS);  }
rand                {SQL_NEW_KEYWORD(SQL_TOKEN_RAND);  }
range               {SQL_NEW_KEYWORD(SQL_TOKEN_RANGE);  }
rank                {SQL_NEW_KEYWORD(SQL_TOKEN_RANK);  }
real                {SQL_NEW_KEYWORD(SQL_TOKEN_REAL);  }
references          {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCES);  }
referencing         {SQL_NEW_KEYWORD(SQL_TOKEN_REFERENCING);  }
repeat              {SQL_NEW_KEYWORD(SQL_TOKEN_REPEAT);  }
replace             {SQL_NEW_KEYWORD(SQL_TOKEN_REPLACE);  }
respect             {SQL_NEW_KEYWORD(SQL_TOKEN_RESPECT);  }
rollback            {SQL_NEW_KEYWORD(SQL_TOKEN_ROLLBACK);  }
round               {SQL_NEW_KEYWORD(SQL_TOKEN_ROUND);  }
roundmagic          {SQL_NEW_KEYWORD(SQL_TOKEN_ROUNDMAGIC);  }
row                 {SQL_NEW_KEYWORD(SQL_TOKEN_ROW);  }
rows                {SQL_NEW_KEYWORD(SQL_TOKEN_ROWS);  }
row_number          {SQL_NEW_KEYWORD(SQL_TOKEN_ROW_NUMBER);  }
right               {SQL_NEW_KEYWORD(SQL_TOKEN_RIGHT);  }
rtrim               {SQL_NEW_KEYWORD(SQL_TOKEN_RTRIM);  }

schema              {SQL_NEW_KEYWORD(SQL_TOKEN_SCHEMA);  }
second              {SQL_NEW_KEYWORD(SQL_TOKEN_SECOND);  }
select              {SQL_NEW_KEYWORD(SQL_TOKEN_SELECT);  }
set                 {SQL_NEW_KEYWORD(SQL_TOKEN_SET);  }
size                {SQL_NEW_KEYWORD(SQL_TOKEN_SIZE);  }
sign                {SQL_NEW_KEYWORD(SQL_TOKEN_SIGN);  }
sin                 {SQL_NEW_KEYWORD(SQL_TOKEN_SIN);  }
smallint            {SQL_NEW_KEYWORD(SQL_TOKEN_SMALLINT);  }
some                {SQL_NEW_KEYWORD(SQL_TOKEN_SOME);  }
soundex             {SQL_NEW_KEYWORD(SQL_TOKEN_SOUNDEX);  }
space               {SQL_NEW_KEYWORD(SQL_TOKEN_SPACE);  }
sqrt                {SQL_NEW_KEYWORD(SQL_TOKEN_SQRT);  }
stddev_pop          {SQL_NEW_KEYWORD(SQL_TOKEN_STDDEV_POP);  }
stddev_samp         {SQL_NEW_KEYWORD(SQL_TOKEN_STDDEV_SAMP); }
statement           {SQL_NEW_KEYWORD(SQL_TOKEN_STATEMENT);  }
substring           {SQL_NEW_KEYWORD(SQL_TOKEN_SUBSTRING);  }
sum                 {SQL_NEW_KEYWORD(SQL_TOKEN_SUM);  }
session_user        {SQL_NEW_KEYWORD(SQL_TOKEN_SESSION_USER);  }
system_user         {SQL_NEW_KEYWORD(SQL_TOKEN_SYSTEM_USER);  }

table               {SQL_NEW_KEYWORD(SQL_TOKEN_TABLE);  }
tan                 {SQL_NEW_KEYWORD(SQL_TOKEN_TAN);  }
then                {SQL_NEW_KEYWORD(SQL_TOKEN_THEN);  }
ties                {SQL_NEW_KEYWORD(SQL_TOKEN_TIES);  }
time                {SQL_NEW_KEYWORD(SQL_TOKEN_TIME);  }
timestamp           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMP);  }
timestampadd        {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMPADD);  }
timestampdiff       {SQL_NEW_KEYWORD(SQL_TOKEN_TIMESTAMPDIFF);  }
timevalue           {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEVALUE);  }
timezone_hour       {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEZONE_HOUR);  }
timezone_minute     {SQL_NEW_KEYWORD(SQL_TOKEN_TIMEZONE_MINUTE);  }
to                  {SQL_NEW_KEYWORD(SQL_TOKEN_TO);  }
trailing            {SQL_NEW_KEYWORD(SQL_TOKEN_TRAILING);  }
translate           {SQL_NEW_KEYWORD(SQL_TOKEN_TRANSLATE);  }
trigger             {SQL_NEW_KEYWORD(SQL_TOKEN_TRIGGER);  }
trim                {SQL_NEW_KEYWORD(SQL_TOKEN_TRIM);  }
true                {SQL_NEW_KEYWORD(SQL_TOKEN_TRUE);  }
truncate            {SQL_NEW_KEYWORD(SQL_TOKEN_TRUNCATE);  }
ts                  {SQL_NEW_KEYWORD(SQL_TOKEN_TS);  }
t                   {SQL_NEW_KEYWORD(SQL_TOKEN_T);  }

ucase               {SQL_NEW_KEYWORD(SQL_TOKEN_UCASE);  }
unbounded           {SQL_NEW_KEYWORD(SQL_TOKEN_UNBOUNDED);  }
union               {SQL_NEW_KEYWORD(SQL_TOKEN_UNION);  }
unique              {SQL_NEW_KEYWORD(SQL_TOKEN_UNIQUE);  }
unknown             {SQL_NEW_KEYWORD(SQL_TOKEN_UNKNOWN);  }
update              {SQL_NEW_KEYWORD(SQL_TOKEN_UPDATE);  }
upper               {SQL_NEW_KEYWORD(SQL_TOKEN_UPPER);  }
usage               {SQL_NEW_KEYWORD(SQL_TOKEN_USAGE);  }
user                {SQL_NEW_KEYWORD(SQL_TOKEN_USER);  }
using               {SQL_NEW_KEYWORD(SQL_TOKEN_USING);  }

varbinary           {SQL_NEW_KEYWORD(SQL_TOKEN_VARBINARY);  }
varchar             {SQL_NEW_KEYWORD(SQL_TOKEN_VARCHAR);  }
varying             {SQL_NEW_KEYWORD(SQL_TOKEN_VARYING);  }
var_pop             {SQL_NEW_KEYWORD(SQL_TOKEN_VAR_POP);  }
var_samp            {SQL_NEW_KEYWORD(SQL_TOKEN_VAR_SAMP);  }
value               {SQL_NEW_KEYWORD(SQL_TOKEN_VALUE);  }
values              {SQL_NEW_KEYWORD(SQL_TOKEN_VALUES);  }
view                {SQL_NEW_KEYWORD(SQL_TOKEN_VIEW);  }

week                {SQL_NEW_KEYWORD(SQL_TOKEN_WEEK);  }
when                {SQL_NEW_KEYWORD(SQL_TOKEN_WHEN);  }
where               {SQL_NEW_KEYWORD(SQL_TOKEN_WHERE);  }
with                {SQL_NEW_KEYWORD(SQL_TOKEN_WITH);  }
within              {SQL_NEW_KEYWORD(SQL_TOKEN_WITHIN);  }
without             {SQL_NEW_KEYWORD(SQL_TOKEN_WITHOUT);  }
work                {SQL_NEW_KEYWORD(SQL_TOKEN_WORK);  }

year                {SQL_NEW_KEYWORD(SQL_TOKEN_YEAR);  }

zone                {SQL_NEW_KEYWORD(SQL_TOKEN_ZONE);  }

"<"                 { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_LESS);return SQL_LESS;}
">"                 { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_GREAT);return SQL_GREAT;}
"="                 { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_EQUAL);return SQL_EQUAL;}
"<="                { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_LESSEQ);return SQL_LESSEQ;}
">="                { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_GREATEQ);return SQL_GREATEQ;}
"<>"                { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"!="                { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_NOTEQUAL);return SQL_NOTEQUAL;}
"||"                { SQL_NEW_NODE(::rtl::OUString(SQLyytext,strlen(SQLyytext),RTL_TEXTENCODING_UTF8), SQL_NODE_CONCAT);return SQL_CONCAT;}


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
 * nTyp == 0 -> SQL_NODE_NAME
 * nTyp == 1 -> SQL_NODE_STRING
 * nTyp == 2 -> SQL_NODE_ACCESS_DATE
 */
sal_Int32 gatherString(int delim, sal_Int32 nTyp)
{
    int ch;
    ::rtl::OStringBuffer sBuffer(256);

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
                    SQL_NEW_NODE(::rtl::OStringToOUString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8), SQL_NODE_NAME);
                    return SQL_TOKEN_NAME;
                case 1:
                    SQL_NEW_NODE(::rtl::OStringToOUString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8), SQL_NODE_STRING);
                    return SQL_TOKEN_STRING;
                case 2:
                    SQL_NEW_NODE(::rtl::OStringToOUString(sBuffer.makeStringAndClear(),RTL_TEXTENCODING_UTF8), SQL_NODE_ACCESS_DATE);
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
        case IParseContext::KEY_LIKE:       nTokenID = SQL_TOKEN_LIKE;      break;
        case IParseContext::KEY_NOT:        nTokenID = SQL_TOKEN_NOT;       break;
        case IParseContext::KEY_NULL:       nTokenID = SQL_TOKEN_NULL;      break;
        case IParseContext::KEY_TRUE:       nTokenID = SQL_TOKEN_TRUE;      break;
        case IParseContext::KEY_FALSE:      nTokenID = SQL_TOKEN_FALSE;     break;
        case IParseContext::KEY_IS:         nTokenID = SQL_TOKEN_IS;        break;
        case IParseContext::KEY_BETWEEN:    nTokenID = SQL_TOKEN_BETWEEN;   break;
        case IParseContext::KEY_OR:         nTokenID = SQL_TOKEN_OR;        break;
        case IParseContext::KEY_AND:        nTokenID = SQL_TOKEN_AND;       break;
        case IParseContext::KEY_AVG:        nTokenID = SQL_TOKEN_AVG;       break;
        case IParseContext::KEY_COUNT:      nTokenID = SQL_TOKEN_COUNT;     break;
        case IParseContext::KEY_MAX:        nTokenID = SQL_TOKEN_MAX;       break;
        case IParseContext::KEY_MIN:        nTokenID = SQL_TOKEN_MIN;       break;
        case IParseContext::KEY_SUM:        nTokenID = SQL_TOKEN_SUM;       break;
        case IParseContext::KEY_EVERY:      nTokenID = SQL_TOKEN_EVERY;     break;
        case IParseContext::KEY_ANY:        nTokenID = SQL_TOKEN_ANY;       break;
        case IParseContext::KEY_SOME:       nTokenID = SQL_TOKEN_SOME;      break;
        case IParseContext::KEY_STDDEV_POP: nTokenID = SQL_TOKEN_STDDEV_POP; break;
        case IParseContext::KEY_STDDEV_SAMP: nTokenID = SQL_TOKEN_STDDEV_SAMP; break;
        case IParseContext::KEY_VAR_SAMP:   nTokenID = SQL_TOKEN_VAR_SAMP;  break;
        case IParseContext::KEY_VAR_POP:    nTokenID = SQL_TOKEN_VAR_POP;   break;
        case IParseContext::KEY_COLLECT:    nTokenID = SQL_TOKEN_COLLECT;   break;
        case IParseContext::KEY_FUSION:     nTokenID = SQL_TOKEN_FUSION;    break;
        case IParseContext::KEY_INTERSECTION: nTokenID = SQL_TOKEN_INTERSECTION; break;
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
        case IParseContext::KEY_LIKE:
        case IParseContext::KEY_NOT:
        case IParseContext::KEY_NULL:
        case IParseContext::KEY_TRUE:
        case IParseContext::KEY_FALSE:
        case IParseContext::KEY_IS:
        case IParseContext::KEY_BETWEEN:
        case IParseContext::KEY_OR:
        case IParseContext::KEY_AND:
        case IParseContext::KEY_COUNT:
        case IParseContext::KEY_AVG:
        case IParseContext::KEY_MAX:
        case IParseContext::KEY_MIN:
        case IParseContext::KEY_SUM:
        case IParseContext::KEY_EVERY:
        case IParseContext::KEY_ANY:
        case IParseContext::KEY_SOME:
        case IParseContext::KEY_STDDEV_POP:
        case IParseContext::KEY_STDDEV_SAMP:
        case IParseContext::KEY_VAR_SAMP:
        case IParseContext::KEY_VAR_POP:
        case IParseContext::KEY_COLLECT:
        case IParseContext::KEY_FUSION:
        case IParseContext::KEY_INTERSECTION:
            nToken = mapEnumToToken(eKeyCode);
            SQL_NEW_KEYWORD(nToken);
            break;
        default:
            SQL_NEW_NODE(::rtl::OUString(text,strlen(text),RTL_TEXTENCODING_UTF8), SQL_NODE_NAME);
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
    sal_Int32 nToken;
    OSL_ENSURE(xxx_pGLOBAL_SQLSCAN,"You forgot to set the scanner!");
    IParseContext::InternationalKeyCode eKeyCode = xxx_pGLOBAL_SQLSCAN->getInternationalTokenID(text);
    switch (eKeyCode)
    {
        case IParseContext::KEY_LIKE:
        case IParseContext::KEY_NOT:
        case IParseContext::KEY_NULL:
        case IParseContext::KEY_TRUE:
        case IParseContext::KEY_FALSE:
        case IParseContext::KEY_IS:
        case IParseContext::KEY_BETWEEN:
        case IParseContext::KEY_OR:
        case IParseContext::KEY_AND:
        case IParseContext::KEY_COUNT:
        case IParseContext::KEY_AVG:
        case IParseContext::KEY_MAX:
        case IParseContext::KEY_MIN:
        case IParseContext::KEY_SUM:
        case IParseContext::KEY_EVERY:
        case IParseContext::KEY_ANY:
        case IParseContext::KEY_SOME:
        case IParseContext::KEY_STDDEV_POP:
        case IParseContext::KEY_STDDEV_SAMP:
        case IParseContext::KEY_VAR_SAMP:
        case IParseContext::KEY_VAR_POP:
        case IParseContext::KEY_COLLECT:
        case IParseContext::KEY_FUSION:
        case IParseContext::KEY_INTERSECTION:
            nToken = mapEnumToToken(eKeyCode);
            SQL_NEW_KEYWORD(nToken);
            break;
        default:
        // we need a special handling for parameter
        {
            ::rtl::OString sStmt = xxx_pGLOBAL_SQLSCAN->getStatement();
            sal_Int32 nLength = strlen(text);
            sal_Int32 nPos = xxx_pGLOBAL_SQLSCAN->GetCurrentPos() - nLength - 2;
            if (sStmt.getStr()[nPos] == ':')
            {
                SQL_NEW_NODE(::rtl::OUString(text,nLength,RTL_TEXTENCODING_UTF8), SQL_NODE_NAME);
                nToken = SQL_TOKEN_NAME;
            }
            else
            {
                SQL_NEW_NODE(::rtl::OUString(text,nLength,RTL_TEXTENCODING_UTF8), SQL_NODE_STRING);
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
            : m_pContext(NULL)
            , m_nCurrentPos(0)
            , m_bInternational(sal_False)
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
    m_sErrorMessage = ::rtl::OUString(fmt,strlen(fmt),RTL_TEXTENCODING_UTF8);
    if (m_nCurrentPos < m_sStatement.getLength())
    {
        m_sErrorMessage += ::rtl::OUString(": ");

        ::rtl::OUString aError;
        static sal_Int32 BUFFERSIZE = 256;
        static sal_Char* Buffer = 0;
        if(!Buffer)
            Buffer = new sal_Char[BUFFERSIZE];

        sal_Char *s = Buffer;
        sal_Int32 nPos = 1;
        int ch = SQLyytext ? (SQLyytext[0] == 0 ? ' ' : SQLyytext[0]): ' ';
        *s++ = ch;
        while (!checkeof(ch = yyinput()))
        {
            if (ch == ' ')
            {
                if ((ch = yyinput()) != ' ')
                {
                    if (!checkeof(ch))
                        unput(ch);
                }
                *s = '\0';
                aError = ::rtl::OUString(Buffer,nPos,RTL_TEXTENCODING_UTF8);
                break;
            }
            else
            {
                *s++ = ch;
                if (++nPos == BUFFERSIZE)
                {
                    ::rtl::OString aBuf(Buffer);
                    delete[] Buffer;
                    BUFFERSIZE *=2;
                    Buffer = new sal_Char[BUFFERSIZE];
                    for(sal_Int32 i=0;i<aBuf.getLength();++i,++Buffer)
                        *Buffer = aBuf.getStr()[i];
                    s = &Buffer[nPos];
    			}
			}
		}
		m_sErrorMessage += aError;
		delete[] Buffer;
		Buffer = NULL;
	}
	IN_SQLyyerror = false;
	YY_FLUSH_BUFFER;
}

//------------------------------------------------------------------------------
void OSQLScanner::prepareScan(const ::rtl::OUString & rNewStatement, const IParseContext* pContext, bool bInternational)
{
    YY_FLUSH_BUFFER;
    BEGIN(m_nRule);

    m_sErrorMessage = ::rtl::OUString();
    m_sStatement = ::rtl::OUStringToOString(rNewStatement, RTL_TEXTENCODING_UTF8);
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
    return (m_bInternational) ? m_pContext->getIntlKeyCode(::rtl::OString(sToken) ) : IParseContext::KEY_NONE;
}
// -------------------------------------------------------------------------
sal_Int32	OSQLScanner::GetGERRule() const		{ return PREDICATE_GER; }
sal_Int32	OSQLScanner::GetENGRule() const		{ return PREDICATE_ENG; }
sal_Int32	OSQLScanner::GetSQLRule() const		{ return SQL; }
sal_Int32	OSQLScanner::GetDATERule() const	{ return DATE; }
sal_Int32	OSQLScanner::GetSTRINGRule() const	{ return STRING; }
// -------------------------------------------------------------------------
void OSQLScanner::setScanner(bool _bNull)
{
	xxx_pGLOBAL_SQLSCAN = _bNull ? NULL : this;
}
// -------------------------------------------------------------------------
sal_Int32 OSQLScanner::SQLlex()
{
	return SQLyylex();
}

#if defined _MSC_VER
#pragma warning(pop)
#endif
