/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%option bison-bridge
%option bison-locations
%option extra-type="unoidl::detail::SourceProviderScannerData *"
%option never-interactive
%option nounistd
%option noyywrap
%option reentrant
%option warn
%option yylineno

%top {

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>

}

%{

#include "rtl/math.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/textenc.h"
#include "sal/types.h"

#include "sourceprovider-parser-requires.hxx"
#include "sourceprovider-parser.hxx"
#include "sourceprovider-scanner.hxx"

namespace unoidl { namespace detail {

static std::size_t sourceProviderScannerInput(
    SourceProviderScannerData * data, char * buffer, std::size_t size)
{
    assert(data != nullptr);
    if (data->sourcePosition == data->sourceEnd) {
        return YY_NULL;
    }
    assert(data->sourcePosition < data->sourceEnd);
    size = std::min<std::size_t>(size, data->sourceEnd - data->sourcePosition);
    std::memcpy(buffer, data->sourcePosition, size);
    data->sourcePosition += size;
    return size;
}

} }

#define YY_INPUT(buf, result, max_size) ((result) = \
    ::unoidl::detail::sourceProviderScannerInput(yyextra, (buf), (max_size)))

namespace {

int nonZeroIntegerLiteral(
    char const * text, std::size_t length, sal_Int16 radix, sal_uInt64 * value,
    unoidl::detail::SourceProviderScannerData * data)
{
    assert(text != nullptr);
    assert(length != 0);
    assert(value != nullptr);
    assert(data != nullptr);
    std::size_t n = length;
    switch (text[length - 1]) {
    case 'L':
    case 'U':
    case 'l':
    case 'u':
        --n;
        break;
    default:
        break;
    }
    *value = OString(text, n).toUInt64(radix);
    if (*value == 0) {
        data->errorMessage = "out-of-range integer literal "
            + OUString(text, length, RTL_TEXTENCODING_ASCII_US);
        return TOK_ERROR;
    }
    return TOK_INTEGER;
}

}

%}

%x comment1 comment2 doc docdepr

DIGIT [0-9]
UPPER [A-Z]
LOWER [a-z]
ALPHA {UPPER}|{LOWER}
ALNUM {DIGIT}|{ALPHA}

%%

[ \t\r]
\n *yylloc = yylineno;

"//" BEGIN comment1;
"#" BEGIN comment1; //TODO: only at start of line
<comment1>.
<comment1>\n *yylloc = yylineno; BEGIN INITIAL;

"/*" BEGIN comment2;
"/**" BEGIN doc;
"/***" BEGIN comment2;

<comment2,doc>"*/" BEGIN INITIAL;
<docdepr>"*/" BEGIN INITIAL; return TOK_DEPRECATED;

<comment2,docdepr>.
<comment2,doc,docdepr>\n *yylloc = yylineno;
<comment2,doc,docdepr><<EOF>> {
    yyextra->errorMessage = "unterminated comment";
    return TOK_ERROR;
}

<doc>[ \t\r]
<doc>"@deprecated" BEGIN docdepr;
<doc>"*"
<doc>[^ \t\r\n*]+

[%&()*+,\-/:;<=>[\]^{|}~] return yytext[0];

"..." return TOK_ELLIPSIS;
"::" return TOK_COLONS;
"<<" return TOK_LEFTSHIFT;
">>" return TOK_RIGHTSHIFT;

"FALSE" return TOK_FALSE;
"False" return TOK_FALSE;
"TRUE" return TOK_TRUE;
"True" return TOK_TRUE;
"any" return TOK_ANY;
"attribute" return TOK_ATTRIBUTE;
"boolean" return TOK_BOOLEAN;
"bound" return TOK_BOUND;
"byte" return TOK_BYTE;
"char" return TOK_CHAR;
"const" return TOK_CONST;
"constants" return TOK_CONSTANTS;
"constrained" return TOK_CONSTRAINED;
"double" return TOK_DOUBLE;
"enum" return TOK_ENUM;
"exception" return TOK_EXCEPTION;
"float" return TOK_FLOAT;
"get" return TOK_GET;
"hyper" return TOK_HYPER;
"in" return TOK_IN;
"inout" return TOK_INOUT;
"interface" return TOK_INTERFACE;
"long" return TOK_LONG;
"maybeambiguous" return TOK_MAYBEAMBIGUOUS;
"maybedefault" return TOK_MAYBEDEFAULT;
"maybevoid" return TOK_MAYBEVOID;
"module" return TOK_MODULE;
"optional" return TOK_OPTIONAL;
"out" return TOK_OUT;
"property" return TOK_PROPERTY;
"published" return TOK_PUBLISHED;
"raises" return TOK_RAISES;
"readonly" return TOK_READONLY;
"removable" return TOK_REMOVABLE;
"sequence" return TOK_SEQUENCE;
"service" return TOK_SERVICE;
"set" return TOK_SET;
"short" return TOK_SHORT;
"singleton" return TOK_SINGLETON;
"string" return TOK_STRING;
"struct" return TOK_STRUCT;
"transient" return TOK_TRANSIENT;
"type" return TOK_TYPE;
"typedef" return TOK_TYPEDEF;
"unsigned" return TOK_UNSIGNED;
"void" return TOK_VOID;

{UPPER}("_"?{ALNUM})*|{LOWER}{ALNUM}* {
    yylval->sval = new OString(yytext);
    return TOK_IDENTIFIER;
}

({ALPHA}|"_")({ALNUM}|"_")* {
#if 1 //TODO
    yylval->sval=new OString(yytext);return TOK_IDENTIFIER;
#else
    yyextra->errorMessage = "illegal identifier "
        + OUString(yytext, yyleng, RTL_TEXTENCODING_ASCII_US);
    return TOK_ERROR;
#endif
}

0+[LUlu]? |
0[Xx]0+[LUlu]? {
    yylval->ival = 0;
    return TOK_INTEGER;
}

0[0-7]+[LUlu]? {
    return nonZeroIntegerLiteral(yytext, yyleng, 8, &yylval->ival, yyextra);
}

[1-9]{DIGIT}*[LUlu]? {
    return nonZeroIntegerLiteral(yytext, yyleng, 10, &yylval->ival, yyextra);
}

0[Xx][0-9A-Fa-f]+[LUlu]? {
    return nonZeroIntegerLiteral(
        yytext + 2, yyleng - 2, 16, &yylval->ival, yyextra);
}

{DIGIT}+[Ee][+\-]?{DIGIT}+[Ff]? |
{DIGIT}*"."{DIGIT}+([Ee][+\-]?{DIGIT}+)?[Ff]? {
    rtl_math_ConversionStatus s;
    yylval->fval = rtl_math_stringToDouble(
        yytext, yytext + yyleng, '.', 0, &s, nullptr);
    if (s == rtl_math_ConversionStatus_OutOfRange) {
        yyextra->errorMessage = "out-of-range floating-point literal "
            + OUString(yytext, yyleng, RTL_TEXTENCODING_ASCII_US);
        return TOK_ERROR;
    }
    return TOK_FLOATING;
}

{DIGIT}({ALNUM}|"_")* {
    yyextra->errorMessage = "illegal numeric literal "
        + OUString(yytext, yyleng, RTL_TEXTENCODING_ASCII_US);
    return TOK_ERROR;
}

. {
    unsigned char c = yytext[0];
    yyextra->errorMessage = c >= ' ' && c <= '~'
        ? OUString("invalid character \"" + OUStringLiteral1(c) + "\"")
        : OUString(
            "invalid byte x" + OUString::number(c, 16).toAsciiUpperCase());
    return TOK_ERROR;
}

%%

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
