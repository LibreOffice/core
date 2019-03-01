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

#include <assert.h>
#include <stdio.h>
#include <string.h>

// DVO: always use standard headers:
#include <istream>
#include <sstream>
using namespace std;

#include "mzstring.h"
#include "hwpeq.h"
#include <sal/types.h>
#include <sal/macros.h>

#include <rtl/character.hxx>

/* @Man: change the hwp formula to LaTeX */
#ifdef _WIN32
# define ENDL  "\r\n"
#else /* !_WIN32 */
# define ENDL  "\n"
#endif

#define EQ_CASE 0x01    // case sensitive cmd
#define EQ_ENV  0x02    // equiv to latex environment
#define EQ_ATOP 0x04    // must revert order

static bool IS_WS(std::istream::int_type ch) {
    return ch != std::istream::traits_type::eof()
        && rtl::isAsciiWhiteSpace(
            static_cast<unsigned char>(
                std::istream::traits_type::to_char_type(ch)));
}

static bool IS_BINARY(std::istream::int_type ch) {
    return ch != std::istream::traits_type::eof()
        && strchr("+-<=>", std::istream::traits_type::to_char_type(ch));
}

#ifdef _WIN32
#define STRICMP stricmp
#else
#define STRICMP strcasecmp
#endif

// sub and sup script status
enum { SCRIPT_NONE, SCRIPT_SUB, SCRIPT_SUP, SCRIPT_ALL};

static int  eq_word(MzString& outs, istream *strm, int script = SCRIPT_NONE);
static bool eq_sentence(MzString& outs, istream *strm, const char *end = nullptr);

struct hwpeq {
  const char    *key;       // hwp math keyword
  const char    *latex;     // corresponding latex keyword
  int           nargs;      // # of argument
  unsigned char flag;       // case sensitive?
};

static const hwpeq eq_tbl[] = {
  { "!=",         "\\equiv ", 0,  0   },
  { "#",          "\\\\",     0,  0   },
  { "+-",         "\\pm ",    0,  0   },
  { "-+",         "\\mp ",    0,  0   },
  { "<=",         "\\leq ",   0,  0   },
  { "==",         "\\equiv ", 0,  0   },
  { ">=",         "\\geq ",   0,  0   },
  { "Pr",         nullptr,       0,  0   },
  { "^",          "^",        1,  0   },
  { "_",          "_",        1,  0   },
  { "`",          "\\;",      0,  0   },
  { "acute",      nullptr,       1,  0   },
  { "aleph",      nullptr,       0,  0   },
  { "alpha",      nullptr,       0,  EQ_CASE },
  { "amalg",      nullptr,       0,  0   },
  { "and",        nullptr,       0,  0   },
  { "angle",      nullptr,       0,  0   },
  { "angstrom",   nullptr,       0,  0   },
  { "approx",     nullptr,       0,  0   },
  { "arc",        nullptr,       0,  0   },
  { "arccos",     nullptr,       0,  0   },
  { "arch",       nullptr,       0,  0   },
  { "arcsin",     nullptr,       0,  0   },
  { "arctan",     nullptr,       0,  0   },
  { "arg",        nullptr,       0,  0   },
  { "assert",     "\\vdash",  0,  0   },
  { "ast",        nullptr,       0,  0   },
  { "asymp",      nullptr,       0,  0   },
  { "atop",       nullptr,       1,  EQ_ATOP },
  { "backslash",  nullptr,       0,  0   },
  { "bar",        nullptr,       1,  0   },
  { "because",    nullptr,       0,  0   },
  { "beta",       nullptr,       0,  EQ_CASE },
  { "big",        nullptr,       0,  EQ_CASE },
  { "bigcap",     nullptr,       0,  0   },
  { "bigcirc",    nullptr,       0,  0   },
  { "bigcup",     nullptr,       0,  0   },
  { "bigg",       nullptr,       0,  EQ_CASE },
  { "bigodiv",    nullptr,       0,  0   },
  { "bigodot",    nullptr,       0,  0   },
  { "bigominus",  nullptr,       0,  0   },
  { "bigoplus",   nullptr,       0,  0   },
  { "bigotimes",  nullptr,       0,  0   },
  { "bigsqcap",   nullptr,       0,  0   },
  { "bigsqcup",   nullptr,       0,  0   },
  { "biguplus",   nullptr,       0,  0   },
  { "bigvee",     nullptr,       0,  0   },
  { "bigwedge",   nullptr,       0,  0   },
  { "binom",      nullptr,       2,  0   },
  { "bmatrix",    nullptr,       0,  EQ_ENV  },
  { "bold",       nullptr,       0,  0   },
  { "bot",        nullptr,       0,  0   },
  { "breve",      nullptr,       1,  0   },
  { "buildrel",   nullptr,       0,  0   }, // LATER
  { "bullet",     nullptr,       0,  0   },
  { "cap",        nullptr,       0,  0   },
  { "cases",      nullptr,       0,  EQ_ENV  },
  { "ccol",       nullptr,       0,  0   }, /* Center vertically */
  { "cdot",       nullptr,       0,  0   },
  { "cdots",      nullptr,       0,  0   },
  { "check",      nullptr,       1,  0   },
  { "chi",        nullptr,       0,  EQ_CASE },
  { "choose",     nullptr,       0,  EQ_ATOP },
  { "circ",       nullptr,       0,  0   },
  { "col",        nullptr,       0,  0   }, // LATER
  { "cong",       nullptr,       0,  0   },
  { "coprod",     nullptr,       0,  0   },
  { "cos",        nullptr,       0,  0   },
  { "cosec",      nullptr,       0,  0   },
  { "cosh",       nullptr,       0,  0   },
  { "cot",        nullptr,       0,  0   },
  { "coth",       nullptr,       0,  0   },
  { "cpile",      nullptr,       0,  0   }, // LATER
  { "csc",        nullptr,       0,  0   },
  { "cup",        nullptr,       0,  0   },
  { "dagger",     nullptr,       0,  0   },
  { "dashv",      nullptr,       0,  0   },
  { "ddagger",    nullptr,       0,  0   },
  { "ddot",       nullptr,       1,  0   },
  { "ddots",      nullptr,       0,  0   },
  { "def",        nullptr,       0,  0   },
  { "deg",        nullptr,       0,  0   },
  { "del",        nullptr,       0,  0   },
  { "delta",      nullptr,       0,  EQ_CASE },
  { "diamond",    nullptr,       0,  0   },
  { "dim",        nullptr,       0,  0   },
  { "div",        nullptr,       0,  0   },
  { "divide",     nullptr,       0,  0   },
  { "dline",      nullptr,       0,  0   },
  { "dmatrix",    nullptr,       0,  EQ_ENV  },
  { "dot",        nullptr,       1,  0   },
  { "doteq",      nullptr,       0,  0   },
  { "dotsaxis",   nullptr,       0,  0   },
  { "dotsdiag",   nullptr,       0,  0   },
  { "dotslow",    "\\ldots",  0,  0   },
  { "dotsvert",   "\\vdots",  0,  0   },
  { "downarrow",  nullptr,       0,  EQ_CASE },
  { "dsum",       "+",        0,  0   },
  { "dyad",       nullptr,       0,  0   }, // LATER
  { "ell",        nullptr,       0,  0   },
  { "emptyset",   nullptr,       0,  0   },
  { "epsilon",    nullptr,       0,  EQ_CASE },
  { "eqalign",    nullptr,       0,  EQ_ENV  },
  { "equiv",      nullptr,       0,  0   },
  { "eta",        nullptr,       0,  EQ_CASE },
  { "exarrow",    nullptr,       0,  0   },
  { "exist",      "\\exists", 0,  0   },
  { "exists",     nullptr,       0,  0   },
  { "exp",        nullptr,       0,  EQ_CASE },
  { "for",        nullptr,       0,  0   },
  { "forall",     nullptr,       0,  0   },
  { "from",       "_",        1,  0   },
  { "gamma",      nullptr,       0,  EQ_CASE },
  { "gcd",        nullptr,       0,  0   },
  { "ge",         "\\geq",    0,  0   },
  { "geq",        nullptr,       0,  0   },
  { "ggg",        nullptr,       0,  0   },
  { "grad",       nullptr,       0,  0   },
  { "grave",      nullptr,       1,  0   },
  { "hat",        "\\widehat",    1,  0   },
  { "hbar",       nullptr,       0,  0   },
  { "hom",        nullptr,       0,  0   },
  { "hookleft",   nullptr,       0,  0   },
  { "hookright",  nullptr,       0,  0   },
  { "identical",  nullptr,       0,  0   }, // LATER
  { "if",         nullptr,       0,  0   },
  { "imag",       nullptr,       0,  0   },
  { "image",      nullptr,       0,  0   },
  { "imath",      nullptr,       0,  0   },
  { "in",         nullptr,       0,  0   },
  { "inf",        "\\infty",  0,  0   },
  { "infinity",   "\\infty",  0,  0   },
  { "infty",      nullptr,       0,  0   },
  { "int",        nullptr,       0,  0   },
  { "integral",   "\\int",    0,  0   },
  { "inter",      "\\bigcap", 0,  0   },
  { "iota",       nullptr,       0,  EQ_CASE },
  { "iso",        nullptr,       0,  0   }, // ams
  { "it",         nullptr,       0,  0   },
  { "jmath",      nullptr,       0,  0   },
  { "kappa",      nullptr,       0,  EQ_CASE },
  { "ker",        nullptr,       0,  0   },
  { "lambda",     nullptr,       0,  EQ_CASE },
  { "land",       nullptr,       0,  0   }, // LATER
  { "langle",     nullptr,       0,  0   },
  { "larrow",     "\\leftarrow",  0,  EQ_CASE },
  { "lbrace",     nullptr,       0,  0   },
  { "lbrack",     "[",        0,  0   },
  { "lceil",      nullptr,       0,  0   },
  { "lcol",       nullptr,       0,  0   }, // LATER
  { "ldots",      nullptr,       0,  0   },
  { "le",         nullptr,       0,  0   },
  { "left",       nullptr,       0,  0   },
  { "leftarrow",  nullptr,       0,  EQ_CASE },
  { "leq",        nullptr,       0,  0   },
  { "lfloor",     nullptr,       0,  0   },
  { "lg",         nullptr,       0,  0   },
  { "lim",        nullptr,       0,  EQ_CASE },
  { "line",       "\\vert",   0,  0   },
  { "liter",      "\\ell",    0,  0   },
  { "lll",        nullptr,       0,  0   }, // ams
  { "ln",         nullptr,       0,  0   },
  { "log",        nullptr,       0,  0   },
  { "lor",        "\\vee",    0,  0   },
  { "lparen",     "(",        0,  0   },
  { "lpile",      nullptr,       0,  0   }, // LATER
  { "lrarrow",    "\\leftrightarrow",   0,  EQ_CASE },
  { "lrharpoons", "\\leftrightharpoons",0,  0   },
  { "mapsto",     nullptr,       0,  0   },
  { "massert",    "\\dashv",  0,  0   },
  { "matrix",     nullptr,       0,  EQ_ENV  },
  { "max",        nullptr,       0,  0   },
  { "mho",        nullptr,       0,  0   }, // ams
  { "min",        nullptr,       0,  0   },
  { "minusplus",  nullptr,       0,  0   },
  { "mit",        "",     0,  0   }, // font
  { "mod",        "\\bmod",   0,  0   },
  { "models",     nullptr,       0,  0   },
  { "msangle",    nullptr,       0,  0   }, // LATER
  { "mu",         nullptr,       0,  EQ_CASE },
  { "nabla",      nullptr,       0,  0   },
  { "ne",         nullptr,       0,  0   },
  { "nearrow",    nullptr,       0,  0   },
  { "neg",        nullptr,       0,  0   },
  { "neq",        nullptr,       0,  0   },
  { "nequiv",     nullptr,       0,  0   },
  { "ni",         nullptr,       0,  0   },
  { "not",        nullptr,       0,  0   },
  { "notin",      nullptr,       0,  0   },
  { "nu",         nullptr,       0,  EQ_CASE },
  { "nwarrow",    nullptr,       0,  0   },
  { "odiv",       nullptr,       0,  0   },
  { "odot",       nullptr,       0,  0   },
  { "oint",       nullptr,       0,  0   },
  { "omega",      nullptr,       0,  EQ_CASE },
  { "omicron",    nullptr,       0,  EQ_CASE },
  { "ominus",     nullptr,       0,  0   },
  { "oplus",      nullptr,       0,  0   },
  { "or ",        nullptr,       0,  0   },
  { "oslash",     nullptr,       0,  0   },
  { "otimes",     nullptr,       0,  0   },
  { "over",       nullptr,       1,  EQ_ATOP },
  { "overline",   nullptr,       1,  0   },
  { "owns",       "\\ni",     0,  0   },
  { "parallel",   nullptr,       0,  0   },
  { "partial",    nullptr,       0,  0   },
  { "phantom",    nullptr,       0,  0   },
  { "phi",        nullptr,       0,  EQ_CASE },
  { "pi",         nullptr,       0,  EQ_CASE },
  { "pile",       nullptr,       0,  0   }, // LATER
  { "plusminus",  "\\pm",     0,  0   },
  { "pmatrix",    nullptr,       0,  EQ_ENV  },
  { "prec",       nullptr,       0,  0   },
  { "prep",       nullptr,       0,  0   },
  { "prime",      nullptr,       0,  0   },
  { "prod",       nullptr,       0,  0   },
  { "propto",     nullptr,       0,  0   },
  { "psi",        nullptr,       0,  EQ_CASE },
  { "rangle",     nullptr,       0,  0   },
  { "rarrow",     "\\rightarrow", 0,  EQ_CASE },
  { "rbrace",     "]",        0,  0   },
  { "rbrace",     nullptr,       0,  0   },
  { "rceil",      nullptr,       0,  0   },
  { "rcol",       nullptr,       0,  0   }, // LATER
  { "real",       "\\Re",     0,  0   },
  { "reimage",    nullptr,       0,  0   },
  { "rel",        nullptr,       0,  0   },
  { "rfloor",     nullptr,       0,  0   },
  { "rho",        nullptr,       0,  EQ_CASE },
  { "right",      nullptr,       0,  0   },
  { "rightarrow", nullptr,       0,  EQ_CASE },
  { "rlharpoons", nullptr,       0,  0   },
  { "rm",         nullptr,       0,  0   },
  { "root",       "\\sqrt",   1,  0   },
  { "rparen",     ")",        0,  0   },
  { "rpile",      nullptr,       0,  0   }, // LATER
  { "rtangle",    nullptr,       0,  0   },
  { "sangle",     nullptr,       0,  0   },
  { "scale",      nullptr,       0,  0   },
  { "searrow",    nullptr,       0,  0   },
  { "sec",        nullptr,       0,  0   },
  { "sigma",      nullptr,       0,  EQ_CASE },
  { "sim",        nullptr,       0,  0   },
  { "simeq",      nullptr,       0,  0   },
  { "sin",        nullptr,       0,  0   },
  { "sinh",       nullptr,       0,  0   },
  { "slash",      nullptr,       0,  0   },
  { "smallint",   nullptr,       0,  0   },
  { "smallinter", nullptr,       0,  0   },
  { "smalloint",  nullptr,       0,  0   },
  { "smallprod",  nullptr,       0,  0   },
  { "smallsum",   nullptr,       0,  0   },
  { "smallunion", nullptr,       0,  0   },
  { "smcoprod",   nullptr,       0,  0   },
  { "sqcap",      nullptr,       0,  0   },
  { "sqcup",      nullptr,       0,  0   },
  { "sqrt",       nullptr,       1,  0   },
  { "sqsubset",   nullptr,       0,  0   },
  { "sqsubseteq", nullptr,       0,  0   },
  { "sqsupset",   nullptr,       0,  0   },
  { "sqsupseteq", nullptr,       0,  0   },
  { "star",       nullptr,       0,  0   },
  { "sub",        "_",        0,  0   },
  { "subset",     nullptr,       0,  0   },
  { "subseteq",   nullptr,       0,  0   },
  { "succ",       nullptr,       0,  0   },
  { "sum",        nullptr,       0,  0   },
  { "sup",        "^",        0,  0   },
  { "superset",   nullptr,       0,  0   },
  { "supset",     nullptr,       0,  0   },
  { "supseteq",   nullptr,       0,  0   },
  { "swarrow",    nullptr,       0,  0   },
  { "tan",        nullptr,       0,  0   },
  { "tanh",       nullptr,       0,  0   },
  { "tau",        nullptr,       0,  EQ_CASE },
  { "therefore",  nullptr,       0,  0   },
  { "theta",      nullptr,       0,  EQ_CASE },
  { "tilde",      "\\widetilde",  1,  0   },
  { "times",      nullptr,       0,  0   },
  { "to",         "^",        1,  0   },
  { "top",        nullptr,       0,  0   },
  { "triangle",   nullptr,       0,  0   },
  { "triangled",  nullptr,       0,  0   },
  { "trianglel",  nullptr,       0,  0   },
  { "triangler",  nullptr,       0,  0   },
  { "triangleu",  nullptr,       0,  0   },
  { "udarrow",    "\\updownarrow",0,  EQ_CASE },
  { "under",      "\\underline",  1,  0   },
  { "underline",  "\\underline",  1,  0   },
  { "union",      "\\bigcup", 0,  0   },
  { "uparrow",    nullptr,       0,  EQ_CASE },
  { "uplus",      nullptr,       0,  0   },
  { "upsilon",    nullptr,       0,  EQ_CASE },
  { "varepsilon", nullptr,       0,  0   },
  { "varphi",     nullptr,       0,  0   },
  { "varpi",      nullptr,       0,  0   },
  { "varrho",     nullptr,       0,  0   },
  { "varsigma",   nullptr,       0,  0   },
  { "vartheta",   nullptr,       0,  0   },
  { "varupsilon", nullptr,       0,  0   },
  { "vdash",      nullptr,       0,  0   },
  { "vdots",      nullptr,       0,  0   },
  { "vec",        nullptr,       1,  0   },
  { "vee",        nullptr,       0,  0   },
  { "vert",       nullptr,       0,  0   },
  { "wedge",      nullptr,       0,  0   },
  { "wp",         nullptr,       0,  0   },
  { "xi",         nullptr,       0,  EQ_CASE },
  { "xor",        nullptr,       0,  0   },
  { "zeta",       nullptr,       0,  EQ_CASE }
};

static const hwpeq *lookup_eqn(char const *str)
{
  static const int eqCount = SAL_N_ELEMENTS(eq_tbl);
  int l = 0, r = eqCount;
  const hwpeq *result = nullptr;

  while( l < r ) {
    const int m = (l + r) / 2;
    const int k = strcmp(eq_tbl[m].key, str);
    if( k == 0 ) {
      result = eq_tbl + m;
      break;
    }
    else if( k < 0 )
      l = m + 1;
    else
      r = m;
  }
  return result;
}

/* If only the first character is uppercase or all characters are uppercase, change to lowercase */
static void make_keyword( char *keyword, const char *token)
{
    char* ptr;
    bool result = true;
    int len = strlen(token);
    assert(keyword);

    if( 255 < len )
    {
        len = 255;
    }
    memcpy(keyword, token, len);
    keyword[len] = 0;

    if( (token[0] & 0x80) || rtl::isAsciiLowerCase(static_cast<unsigned char>(token[0])) || strlen(token) < 2 )
        return;

    bool capital = rtl::isAsciiUpperCase(
        static_cast<unsigned char>(keyword[1]));
    for( ptr = keyword + 2; *ptr && result; ptr++ )
    {
        if( (*ptr & 0x80) ||
            (!capital && rtl::isAsciiUpperCase(static_cast<unsigned char>(*ptr))) ||
            (capital && rtl::isAsciiLowerCase(static_cast<unsigned char>(*ptr))) )
        {
            result = false;
        }
    }

    if( result )
    {
        ptr = keyword;
        while( *ptr )
        {
            if( rtl::isAsciiUpperCase(static_cast<unsigned char>(*ptr)) )
                *ptr = sal::static_int_cast<char>(
                    rtl::toAsciiLowerCase(static_cast<unsigned char>(*ptr)));
            ptr++;
        }
    }
}

// token reading function
struct eq_stack {
  MzString  white;
  MzString  token;
  istream   *strm;

  eq_stack() { strm = nullptr; };
  bool state(istream const *s) {
    if( strm != s) { white = nullptr; token = nullptr; }
    return token.length() != 0;
  }
};

static eq_stack *stk = nullptr;

static void push_token(MzString const &white, MzString const &token, istream *strm)
{
  // one time stack
  assert(stk->token.length() == 0);

  stk->white = white;
  stk->token = token;
  stk->strm = strm;
}

/*
 * It returns the length of the read tokens.
 *
 * control char, control sequence, binary sequence,
 * alphabet string, single character */
static int next_token(MzString &white, MzString &token, istream *strm)
{
  std::istream::int_type ch = 0;

  if( stk->state(strm) ) {
    white = stk->white;
    token = stk->token;
    stk->token = nullptr;
    stk->white = nullptr;
    return token.length();
  }

  token = nullptr;
  white = nullptr;
  if( !strm->good() || (ch = strm->get()) == std::istream::traits_type::eof() )
    return 0;

  // read preceding ws
  if( IS_WS(ch) ) {
    do white << static_cast<char>(ch);
    while( IS_WS(ch = strm->get()) );
  }

  if( ch == '\\' || ch & 0x80
      || (ch != std::istream::traits_type::eof() && rtl::isAsciiAlpha(ch)) )
  {
    if( ch == '\\' ) {
      token << static_cast<char>(ch);
      ch = strm->get();
    }
    do {
      token << static_cast<char>(ch);
      ch = strm->get();
    } while( ch != std::istream::traits_type::eof()
             && (ch & 0x80 || rtl::isAsciiAlpha(ch)) ) ;
    strm->putback(static_cast<char>(ch));
    /* special treatment of sub, sub, over, atop
       The reason for this is that affect next_state().
     */
    if( !STRICMP("sub", token) || !STRICMP("from", token) ||
    !STRICMP("sup", token) || !STRICMP("to", token) ||
    !STRICMP("over", token) || !STRICMP("atop", token) ||
    !STRICMP("left", token) || !STRICMP("right", token) )
    {
      char buf[256];
      make_keyword(buf, token);
      token = buf;
    }
    if( !token.compare("sub") || !token.compare("from") )
      token = "_";
    if( !token.compare("sup") || !token.compare("to") )
      token = "^";
  }
  else if( IS_BINARY(ch) ) {
    do token << static_cast<char>(ch);
    while( IS_BINARY(ch = strm->get()) );
    strm->putback(static_cast<char>(ch));
  }
  else if( ch != std::istream::traits_type::eof() && rtl::isAsciiDigit(ch) ) {
    do {
        token << static_cast<char>(ch);
        ch = strm->get();
    } while( ch != std::istream::traits_type::eof() && rtl::isAsciiDigit(ch) );
    strm->putback(static_cast<char>(ch));
  }
  else
    token << static_cast<char>(ch);

  return token.length();
}

static std::istream::int_type read_white_space(MzString& outs, istream *strm)
{
  std::istream::int_type result;

  if( stk->state(strm) ) {
    outs << stk->white;
    stk->white = nullptr;
    result = std::istream::traits_type::to_int_type(stk->token[0]);
  }
  else {
    std::istream::int_type ch;
    while( IS_WS(ch = strm->get()) )
      outs << static_cast<char>(ch);
    strm->putback(static_cast<char>(ch));
    result = ch;
  }
  return result;
}

/* If the argument is not required, delimiters are space and brace for each items.
   sqrt {ab} c = sqrt {ab} c
   (,} are for grouping
   ^ ,_ are for combination

   Sorting of formats with arguments, such as sqrt
      sqrt a -> sqrt {a}
      sqrt {a} -> sqrt {a}
   If there is more than one argument, it eliminates backslash between arguments.
      \frac a b -> frac {a} {b}
   Clean the form of over
      a over b -> {a} over {b}
 */

static int eq_word(MzString& outs, istream *strm, int status)
{
  MzString  token, white, state;
  int       result;
  char      keyword[256];
  const hwpeq *eq;

  next_token(white, token, strm);
  if (token.length() <= 0)
      return 0;
  result = token[0];

  if( token.compare("{") == 0 ) {
    state << white << token;
    eq_sentence(state, strm, "}");
  }
  else if( token.compare("left") == 0 ) {
    state << white << token;
    next_token(white, token, strm);
    state << white << token;

    eq_sentence(state, strm, "right");

    next_token(white, token, strm);
    state << white << token;
  }
  else {
    /* Normal token */
    int script_status = SCRIPT_NONE;
    while( true ) {
      state << white << token;
      make_keyword(keyword, token);
      if( token[0] == '^' )
        script_status |= SCRIPT_SUP;
      else if( token[0] == '_' )
        script_status |= SCRIPT_SUB;
      else
        script_status = SCRIPT_NONE;

      if( nullptr != (eq = lookup_eqn(keyword)) ) {
        int nargs = eq->nargs;
        while( nargs-- ) {
          const std::istream::int_type ch = read_white_space(state, strm);
          if( ch != '{' ) state << '{';
          eq_word(state, strm, script_status);
          if( ch != '{' ) state << '}';
        }
      }

      if( !next_token(white, token, strm) )
        break;
      // end loop and restart with this
      if( (token[0] == '^' && status && !(status & SCRIPT_SUP)) ||
          (token[0] == '_' && status && !(status & SCRIPT_SUB)) ||
          strcmp("over", token) == 0 || strcmp("atop", token) == 0 ||
          strchr("{}#&`", token[0]) ||
          (!strchr("^_", token[0]) && white.length()) )
      {
        push_token(white, token, strm);
        break;
      }
    }
  }
  outs << state;

  return result;
}

static bool eq_sentence(MzString& outs, istream *strm, const char *end)
{
  MzString  state;
  MzString  white, token;
  bool      multiline = false;

  read_white_space(outs, strm);
  while( eq_word(state, strm) ) {
    if( !next_token(white, token, strm) ||
    (end && strcmp(token.c_str(), end) == 0) )
    {
      state << white << token;
      break;
    }
    push_token(white, token, strm);
    if( !token.compare("atop") || !token.compare("over") )
      outs << '{' << state << '}';
    else {
      if( !token.compare("#") )
        multiline = true;
      outs << state;
    }
    state =  nullptr;
    read_white_space(outs, strm);
  }
  outs << state;
  return multiline;
}

static char eq2ltxconv(MzString& sstr, istream *strm, const char *sentinel)
{
  MzString  white, token;
  char      key[256];
  std::istream::int_type ch;
  int       result;

  while( 0 != (result = next_token(white, token, strm)) ) {
    if( sentinel && (result == 1) && strchr(sentinel, token[0]) )
      break;
    make_keyword(key, token);
    const hwpeq *eq = nullptr;
    if( (eq = lookup_eqn(key)) != nullptr ) {
      if( eq->latex )
        strcpy(key, eq->latex);
      else {
        key[0] = '\\';
        strcpy(key + 1, eq->key);
      }
      if( (eq->flag & EQ_CASE)
          && rtl::isAsciiUpperCase(static_cast<unsigned char>(token[0])) )
        key[1] = sal::static_int_cast<char>(
            rtl::toAsciiUpperCase(static_cast<unsigned char>(key[1])));
      token = key;
    }

    if( token[0] == '{' ) { // grouping
      sstr << white << token;
      eq2ltxconv(sstr, strm, "}");
      sstr << '}';
    }
    else if( eq && (eq->flag & EQ_ENV) ) {
      next_token(white, token, strm);
      if( token[0] != '{' )
        return 0;
      sstr << "\\begin" << "{" << eq->key << "}" << ENDL ;
      eq2ltxconv(sstr, strm, "}");
      if( sstr[sstr.length() - 1] != '\n' )
        sstr << ENDL ;
      sstr << "\\end" << "{" << eq->key << "}" << ENDL ;
    }
    else if( eq && (eq->flag & EQ_ATOP) ) {
      if( sstr.length() == 0 )
        sstr << '{';
      else {
        int pos  = sstr.rfind('}');
        if( 0 < pos)
          sstr.replace(pos, ' ');
      }
      sstr << token;
      while( (ch = strm->get()) != std::istream::traits_type::eof()
             && IS_WS(ch) )
        sstr << static_cast<char>(ch);
      if( ch != '{' )
        sstr << "{}";
      else {
        eq2ltxconv(sstr, strm, "}");
        sstr << '}';
      }
    }
    else
      sstr << white << token;
  }
  return token[0];
}

void eq2latex(MzString& outs, char const *s)
{
  assert(s);
  if( stk == nullptr )
    stk = new eq_stack;

  MzString  tstr;

  istringstream tstrm(s);
  bool eqnarray = eq_sentence(tstr, &tstrm);
  istringstream strm(tstr.c_str());

  if( eqnarray )
    outs << "\\begin{array}{rllll}" << ENDL;
  eq2ltxconv(outs, &strm, nullptr);
  outs << ENDL;
  if( eqnarray )
    outs << "\\end{array}" << ENDL;
  delete stk;
  stk = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
