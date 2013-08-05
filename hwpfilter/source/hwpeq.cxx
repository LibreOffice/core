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
#include <ctype.h>

// DVO: always use standard headers:
#include <istream>
#include <sstream>
using namespace std;

#include "mzstring.h"
#include "hwpeq.h"
#include <sal/types.h>
#include <sal/macros.h>

/* @Man: hwp수식을 LaTeX으로 바꾸기 */
#ifdef WIN32
# define ENDL  "\r\n"
#else /* !WIN32 */
# define ENDL  "\n"
#endif

#define WS  " \t\r\n\v\f"

#define EQ_CASE 0x01    // case sensitive cmd
#define EQ_ENV  0x02    // equiv to latex environment
#define EQ_ATOP 0x04    // must revert order

#define IS_WS(ch)   (strchr(WS, ch))
#define IS_BINARY(ch)   (strchr("+-<=>", ch))

#ifdef WIN32
#define STRICMP stricmp
#else
#define STRICMP strcasecmp
#endif

// sub and sup scipt script status
enum { SCRIPT_NONE, SCRIPT_SUB, SCRIPT_SUP, SCRIPT_ALL};

static int  eq_word(MzString& outs, istream *strm, int script = SCRIPT_NONE);
static bool eq_sentence(MzString& outs, istream *strm, const char *end = 0);

struct hwpeq {
  const char    *key;       // hwp math keyword
  const char    *latex;     // corresponding latex keywork
  int       nargs;      // # of argument
  unsigned char flag;       // case sensitive?
};

static const hwpeq eq_tbl[] = {
  { "!=",       "\\equiv ", 0,  0   },
  { "#",        "\\\\",     0,  0   },
  { "+-",       "\\pm ",    0,  0   },
  { "-+",       "\\mp ",    0,  0   },
  { "<=",       "\\leq ",   0,  0   },
  { "==",       "\\equiv ", 0,  0   },
  { ">=",       "\\geq ",   0,  0   },
  { "Pr",       NULL,       0,  0   },
  { "^",        "^",        1,  0   },
  { "_",        "_",        1,  0   },
  { "`",        "\\;",      0,  0   },
  { "acute",        NULL,       1,  0   },
  { "aleph",        NULL,       0,  0   },
  { "alpha",        NULL,       0,  EQ_CASE },
  { "amalg",        NULL,       0,  0   },
  { "and",      NULL,       0,  0   },
  { "angle",        NULL,       0,  0   },
  { "angstrom",     NULL,       0,  0   },
  { "approx",       NULL,       0,  0   },
  { "arc",      NULL,       0,  0   },
  { "arccos",       NULL,       0,  0   },
  { "arch",     NULL,       0,  0   },
  { "arcsin",       NULL,       0,  0   },
  { "arctan",       NULL,       0,  0   },
  { "arg",      NULL,       0,  0   },
  { "assert",       "\\vdash",  0,  0   },
  { "ast",      NULL,       0,  0   },
  { "asymp",        NULL,       0,  0   },
  { "atop",     NULL,       1,  EQ_ATOP },
  { "backslash",    NULL,       0,  0   },
  { "bar",      NULL,       1,  0   },
  { "because",      NULL,       0,  0   },
  { "beta",     NULL,       0,  EQ_CASE },
  { "big",      NULL,       0,  EQ_CASE },
  { "bigcap",       NULL,       0,  0   },
  { "bigcirc",      NULL,       0,  0   },
  { "bigcup",       NULL,       0,  0   },
  { "bigg",     NULL,       0,  EQ_CASE },
  { "bigodiv",      NULL,       0,  0   },
  { "bigodot",      NULL,       0,  0   },
  { "bigominus",    NULL,       0,  0   },
  { "bigoplus",     NULL,       0,  0   },
  { "bigotimes",    NULL,       0,  0   },
  { "bigsqcap",     NULL,       0,  0   },
  { "bigsqcup",     NULL,       0,  0   },
  { "biguplus",     NULL,       0,  0   },
  { "bigvee",       NULL,       0,  0   },
  { "bigwedge",     NULL,       0,  0   },
  { "binom",        NULL,       2,  0   },
  { "bmatrix",      NULL,       0,  EQ_ENV  },
  { "bold",     NULL,       0,  0   },
  { "bot",      NULL,       0,  0   },
  { "breve",        NULL,       1,  0   },
  { "buildrel",     NULL,       0,  0   }, // LATER
  { "bullet",       NULL,       0,  0   },
  { "cap",      NULL,       0,  0   },
  { "cases",        NULL,       0,  EQ_ENV  },
  { "ccol",     NULL,       0,  0   }, /* 세로로 가운데 */
  { "cdot",     NULL,       0,  0   },
  { "cdots",        NULL,       0,  0   },
  { "check",        NULL,       1,  0   },
  { "chi",      NULL,       0,  EQ_CASE },
  { "choose",       NULL,       0,  EQ_ATOP },
  { "circ",     NULL,       0,  0   },
  { "col",      NULL,       0,  0   }, // LATER
  { "cong",     NULL,       0,  0   },
  { "coprod",       NULL,       0,  0   },
  { "cos",      NULL,       0,  0   },
  { "cosec",        NULL,       0,  0   },
  { "cosh",     NULL,       0,  0   },
  { "cot",      NULL,       0,  0   },
  { "coth",     NULL,       0,  0   },
  { "cpile",        NULL,       0,  0   }, // LATER
  { "csc",      NULL,       0,  0   },
  { "cup",      NULL,       0,  0   },
  { "dagger",       NULL,       0,  0   },
  { "dashv",        NULL,       0,  0   },
  { "ddagger",      NULL,       0,  0   },
  { "ddot",     NULL,       1,  0   },
  { "ddots",        NULL,       0,  0   },
  { "def",      NULL,       0,  0   },
  { "deg",      NULL,       0,  0   },
  { "del",      NULL,       0,  0   },
  { "delta",        NULL,       0,  EQ_CASE },
  { "diamond",      NULL,       0,  0   },
  { "dim",      NULL,       0,  0   },
  { "div",      NULL,       0,  0   },
  { "divide",       NULL,       0,  0   },
  { "dline",        NULL,       0,  0   },
  { "dmatrix",      NULL,       0,  EQ_ENV  },
  { "dot",      NULL,       1,  0   },
  { "doteq",        NULL,       0,  0   },
  { "dotsaxis",     NULL,       0,  0   },
  { "dotsdiag",     NULL,       0,  0   },
  { "dotslow",      "\\ldots",  0,  0   },
  { "dotsvert",     "\\vdots",  0,  0   },
  { "downarrow",    NULL,       0,  EQ_CASE },
  { "dsum",     "+",        0,  0   },
  { "dyad",     NULL,       0,  0   }, // LATER
  { "ell",      NULL,       0,  0   },
  { "emptyset",     NULL,       0,  0   },
  { "epsilon",      NULL,       0,  EQ_CASE },
  { "eqalign",      NULL,       0,  EQ_ENV  },
  { "equiv",        NULL,       0,  0   },
  { "eta",      NULL,       0,  EQ_CASE },
  { "exarrow",      NULL,       0,  0   },
  { "exist",        "\\exists", 0,  0   },
  { "exists",       NULL,       0,  0   },
  { "exp",      NULL,       0,  EQ_CASE },
  { "for",      NULL,       0,  0   },
  { "forall",       NULL,       0,  0   },
  { "from",     "_",        1,  0   },
  { "gamma",        NULL,       0,  EQ_CASE },
  { "gcd",      NULL,       0,  0   },
  { "ge",       "\\geq",    0,  0   },
  { "geq",      NULL,       0,  0   },
  { "ggg",      NULL,       0,  0   },
  { "grad",     NULL,       0,  0   },
  { "grave",        NULL,       1,  0   },
  { "hat",      "\\widehat",    1,  0   },
  { "hbar",     NULL,       0,  0   },
  { "hom",      NULL,       0,  0   },
  { "hookleft",     NULL,       0,  0   },
  { "hookright",    NULL,       0,  0   },
  { "identical",    NULL,       0,  0   }, // LATER
  { "if",       NULL,       0,  0   },
  { "imag",     NULL,       0,  0   },
  { "image",        NULL,       0,  0   },
  { "imath",        NULL,       0,  0   },
  { "in",       NULL,       0,  0   },
  { "inf",      "\\infty",  0,  0   },
  { "infinity",     "\\infty",  0,  0   },
  { "infty",        NULL,       0,  0   },
  { "int",      NULL,       0,  0   },
  { "integral",     "\\int",    0,  0   },
  { "inter",        "\\bigcap", 0,  0   },
  { "iota",     NULL,       0,  EQ_CASE },
  { "iso",      NULL,       0,  0   }, // ams
  { "it",       NULL,       0,  0   },
  { "jmath",        NULL,       0,  0   },
  { "kappa",        NULL,       0,  EQ_CASE },
  { "ker",      NULL,       0,  0   },
  { "lambda",       NULL,       0,  EQ_CASE },
  { "land",     NULL,       0,  0   }, // LATER
  { "langle",       NULL,       0,  0   },
  { "larrow",       "\\leftarrow",  0,  EQ_CASE },
  { "lbrace",       NULL,       0,  0   },
  { "lbrack",       "[",        0,  0   },
  { "lceil",        NULL,       0,  0   },
  { "lcol",     NULL,       0,  0   }, // LATER
  { "ldots",        NULL,       0,  0   },
  { "le",       NULL,       0,  0   },
  { "left",     NULL,       0,  0   },
  { "leftarrow",    NULL,       0,  EQ_CASE },
  { "leq",      NULL,       0,  0   },
  { "lfloor",       NULL,       0,  0   },
  { "lg",       NULL,       0,  0   },
  { "lim",      NULL,       0,  EQ_CASE },
  { "line",     "\\vert",   0,  0   },
  { "liter",        "\\ell",    0,  0   },
  { "lll",      NULL,       0,  0   }, // ams
  { "ln",       NULL,       0,  0   },
  { "log",      NULL,       0,  0   },
  { "lor",      "\\vee",    0,  0   },
  { "lparen",       "(",        0,  0   },
  { "lpile",        NULL,       0,  0   }, // LATER
  { "lrarrow",    "\\leftrightarrow",   0,  EQ_CASE },
  { "lrharpoons", "\\leftrightharpoons",0,  0   },
  { "mapsto",       NULL,       0,  0   },
  { "massert",      "\\dashv",  0,  0   },
  { "matrix",       NULL,       0,  EQ_ENV  },
  { "max",      NULL,       0,  0   },
  { "mho",      NULL,       0,  0   }, // ams
  { "min",      NULL,       0,  0   },
  { "minusplus",    NULL,       0,  0   },
  { "mit",      "",     0,  0   }, // font
  { "mod",      "\\bmod",   0,  0   },
  { "models",       NULL,       0,  0   },
  { "msangle",      NULL,       0,  0   }, // LATER
  { "mu",       NULL,       0,  EQ_CASE },
  { "nabla",        NULL,       0,  0   },
  { "ne",       NULL,       0,  0   },
  { "nearrow",      NULL,       0,  0   },
  { "neg",      NULL,       0,  0   },
  { "neq",      NULL,       0,  0   },
  { "nequiv",       NULL,       0,  0   },
  { "ni",       NULL,       0,  0   },
  { "not",      NULL,       0,  0   },
  { "notin",        NULL,       0,  0   },
  { "nu",       NULL,       0,  EQ_CASE },
  { "nwarrow",      NULL,       0,  0   },
  { "odiv",     NULL,       0,  0   },
  { "odot",     NULL,       0,  0   },
  { "oint",     NULL,       0,  0   },
  { "omega",        NULL,       0,  EQ_CASE },
  { "omicron",      NULL,       0,  EQ_CASE },
  { "ominus",       NULL,       0,  0   },
  { "oplus",        NULL,       0,  0   },
  { "or ",      NULL,       0,  0   },
  { "oslash",       NULL,       0,  0   },
  { "otimes",       NULL,       0,  0   },
  { "over",     NULL,       1,  EQ_ATOP },
  { "overline",     NULL,       1,  0   },
  { "owns",     "\\ni",     0,  0   },
  { "parallel",     NULL,       0,  0   },
  { "partial",      NULL,       0,  0   },
  { "phantom",      NULL,       0,  0   },
  { "phi",      NULL,       0,  EQ_CASE },
  { "pi",       NULL,       0,  EQ_CASE },
  { "pile",     NULL,       0,  0   }, // LATER
  { "plusminus",    "\\pm",     0,  0   },
  { "pmatrix",      NULL,       0,  EQ_ENV  },
  { "prec",     NULL,       0,  0   },
  { "prep",     NULL,       0,  0   },
  { "prime",        NULL,       0,  0   },
  { "prod",     NULL,       0,  0   },
  { "propto",       NULL,       0,  0   },
  { "psi",      NULL,       0,  EQ_CASE },
  { "rangle",       NULL,       0,  0   },
  { "rarrow",       "\\rightarrow", 0,  EQ_CASE },
  { "rbrace",       "]",        0,  0   },
  { "rbrace",       NULL,       0,  0   },
  { "rceil",        NULL,       0,  0   },
  { "rcol",     NULL,       0,  0   }, // LATER
  { "real",     "\\Re",     0,  0   },
  { "reimage",      NULL,       0,  0   },
  { "rel",      NULL,       0,  0   },
  { "rfloor",       NULL,       0,  0   },
  { "rho",      NULL,       0,  EQ_CASE },
  { "right",        NULL,       0,  0   },
  { "rightarrow",   NULL,       0,  EQ_CASE },
  { "rlharpoons",   NULL,       0,  0   },
  { "rm",       NULL,       0,  0   },
  { "root",     "\\sqrt",   1,  0   },
  { "rparen",       ")",        0,  0   },
  { "rpile",        NULL,       0,  0   }, // LATER
  { "rtangle",      NULL,       0,  0   },
  { "sangle",       NULL,       0,  0   },
  { "scale",        NULL,       0,  0   },
  { "searrow",      NULL,       0,  0   },
  { "sec",      NULL,       0,  0   },
  { "sigma",        NULL,       0,  EQ_CASE },
  { "sim",      NULL,       0,  0   },
  { "simeq",        NULL,       0,  0   },
  { "sin",      NULL,       0,  0   },
  { "sinh",     NULL,       0,  0   },
  { "slash",        NULL,       0,  0   },
  { "smallint",     NULL,       0,  0   },
  { "smallinter",   NULL,       0,  0   },
  { "smalloint",    NULL,       0,  0   },
  { "smallprod",    NULL,       0,  0   },
  { "smallsum",     NULL,       0,  0   },
  { "smallunion",   NULL,       0,  0   },
  { "smcoprod",     NULL,       0,  0   },
  { "sqcap",        NULL,       0,  0   },
  { "sqcup",        NULL,       0,  0   },
  { "sqrt",     NULL,       1,  0   },
  { "sqsubset",     NULL,       0,  0   },
  { "sqsubseteq",   NULL,       0,  0   },
  { "sqsupset",     NULL,       0,  0   },
  { "sqsupseteq",   NULL,       0,  0   },
  { "star",     NULL,       0,  0   },
  { "sub",      "_",        0,  0   },
  { "subset",       NULL,       0,  0   },
  { "subseteq",     NULL,       0,  0   },
  { "succ",     NULL,       0,  0   },
  { "sum",      NULL,       0,  0   },
  { "sup",      "^",        0,  0   },
  { "superset",     NULL,       0,  0   },
  { "supset",       NULL,       0,  0   },
  { "supseteq",     NULL,       0,  0   },
  { "swarrow",      NULL,       0,  0   },
  { "tan",      NULL,       0,  0   },
  { "tanh",     NULL,       0,  0   },
  { "tau",      NULL,       0,  EQ_CASE },
  { "therefore",    NULL,       0,  0   },
  { "theta",        NULL,       0,  EQ_CASE },
  { "tilde",        "\\widetilde",  1,  0   },
  { "times",        NULL,       0,  0   },
  { "to",       "^",        1,  0   },
  { "top",      NULL,       0,  0   },
  { "triangle",     NULL,       0,  0   },
  { "triangled",    NULL,       0,  0   },
  { "trianglel",    NULL,       0,  0   },
  { "triangler",    NULL,       0,  0   },
  { "triangleu",    NULL,       0,  0   },
  { "udarrow",      "\\updownarrow",0,  EQ_CASE },
  { "under",        "\\underline",  1,  0   },
  { "underline",    "\\underline",  1,  0   },
  { "union",        "\\bigcup", 0,  0   },
  { "uparrow",      NULL,       0,  EQ_CASE },
  { "uplus",        NULL,       0,  0   },
  { "upsilon",      NULL,       0,  EQ_CASE },
  { "varepsilon",   NULL,       0,  0   },
  { "varphi",       NULL,       0,  0   },
  { "varpi",        NULL,       0,  0   },
  { "varrho",       NULL,       0,  0   },
  { "varsigma",     NULL,       0,  0   },
  { "vartheta",     NULL,       0,  0   },
  { "varupsilon",   NULL,       0,  0   },
  { "vdash",        NULL,       0,  0   },
  { "vdots",        NULL,       0,  0   },
  { "vec",      NULL,       1,  0   },
  { "vee",      NULL,       0,  0   },
  { "vert",     NULL,       0,  0   },
  { "wedge",        NULL,       0,  0   },
  { "wp",       NULL,       0,  0   },
  { "xi",       NULL,       0,  EQ_CASE },
  { "xor",      NULL,       0,  0   },
  { "zeta",     NULL,       0,  EQ_CASE }
};

static const hwpeq *lookup_eqn(char *str)
{
  static const int eqCount = SAL_N_ELEMENTS(eq_tbl);
  int       m, k, l = 0, r = eqCount;
  const hwpeq *result = 0;

  while( l < r ) {
    m = (l + r) / 2;
    k = strcmp(eq_tbl[m].key, str);
    if( k == 0 ) {
      result = eq_tbl + m;;
      break;
    }
    else if( k < 0 )
      l = m + 1;
    else
      r = m;
  }
  return result;
}

/* 첫자만 대문자이거나 전부 대문자면 소문자로 바꾼다. */

static char *make_keyword( char *keyword, const char *token)
{
  assert(keyword);
  char  *ptr;
  int   result = true, len = strlen(token);

  if( 255 < len )
    strncpy(keyword, token, 255);
  else
    strcpy(keyword, token);

  if( (token[0] & 0x80) || islower(token[0]) ||
      strlen(token) < 2 )
    return keyword;

  int capital = isupper(keyword[1]);
  for( ptr = keyword + 2; *ptr && result; ptr++ )
    if( (*ptr & 0x80) ||
    (!capital && isupper(*ptr)) ||
    (capital && islower(*ptr)) )
      result = false;

  if( result ) {
    ptr = keyword;
    while( *ptr ) {
      if( isupper(*ptr) )
    *ptr = sal::static_int_cast<char>(tolower(*ptr));
      ptr++;
    }
  }
  return keyword;
}

// token reading funtion
struct eq_stack {
  MzString  white;
  MzString  token;
  istream   *strm;

  eq_stack() { strm = 0; };
  bool state(istream *s) {
    if( strm != s) { white = 0; token = 0; }
    return token.length() != 0;
  }
};

static eq_stack *stk = 0;

void push_token(MzString &white, MzString &token, istream *strm)
{
  // one time stack
  assert(stk->state(strm) == false);

  stk->white = white;
  stk->token = token;
  stk->strm = strm;
}

/* 읽은 토큰의 길이를 반환한다. */
/* control char, control sequence, binary sequence,
   alphabet string, sigle character */
static int next_token(MzString &white, MzString &token, istream *strm)
{
  int  ch = 0;

  if( stk->state(strm) ) {
    white = stk->white;
    token = stk->token;
    stk->token = 0;
    stk->white = 0;
    return token.length();
  }

  token = 0;
  white = 0;
  if( !strm->good() || (ch = strm->get()) == EOF )
    return 0;

  // read preceeding ws
  if( IS_WS(ch) ) {
    do white << (char) ch;
    while( IS_WS(ch = strm->get()) );
  }

  if( ch == '\\' || ch & 0x80 || isalpha(ch) ) {
    if( ch == '\\' ) {
      token << (char) ch;
      ch = strm->get();
    }
    do {
      token << (char) ch;
      ch = strm->get();
    } while( ch != EOF && (ch & 0x80 || isalpha(ch)) ) ;
    strm->putback(sal::static_int_cast<char>(ch));
    /*  sub, sub, over, atop 특수 처리
        그 이유는 next_state()에 영향을 미치기 때문이다.
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
    do token << (char) ch;
    while( IS_BINARY(ch = strm->get()) );
    strm->putback(sal::static_int_cast<char>(ch));
  }
  else if( isdigit(ch) ) {
    do token << (char) ch;
    while( isdigit(ch = strm->get()) );
    strm->putback(sal::static_int_cast<char>(ch));
  }
  else
    token << (char) ch;

  return token.length();
}

static int read_white_space(MzString& outs, istream *strm)
{
  int   result;

  if( stk->state(strm) ) {
    outs << stk->white;
    stk->white = 0;
    result = stk->token[0];
  }
  else {
    int ch;
    while( IS_WS(ch = strm->get()) )
      outs << (char )ch;
    strm->putback(sal::static_int_cast<char>(ch));
    result = ch;
  }
  return result;
}

/* 인수가 필요하지 않은 경우 각 항목간의 구분은 space와 brace
     sqrt {ab}c = sqrt{ab} c
   (, }는 grouping
   ^, _ 는 앞뒤로 결합한다.

   sqrt 등과 같이 인수가 있는 형식 정리
     sqrt a  -> sqrt{a}
     sqrt {a}    -> sqrt{a}
   1 이상의 인수가 있는 경우 인수들간의 역백은 없앤다.
     \frac a b   -> frac{a}{b}
   over의 형식 정리
     a over b    -> {a}over{b}
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
    /* 정상적인 token */
    int script_status = SCRIPT_NONE;
    while( 1 ) {
      state << white << token;
      make_keyword(keyword, token);
      if( token[0] == '^' )
    script_status |= SCRIPT_SUP;
      else if( token[0] == '_' )
    script_status |= SCRIPT_SUB;
      else
    script_status = SCRIPT_NONE;

      if( 0 != (eq = lookup_eqn(keyword)) ) {
    int nargs = eq->nargs;
    int ch;
    while( nargs-- ) {
      ch = read_white_space(state, strm);
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
      (!strchr("^_", token[0]) && white.length()) ) {
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
    state =  0;
    read_white_space(outs, strm);
  }
  outs << state;
  return multiline;
}

static char eq2ltxconv(MzString& sstr, istream *strm, const char *sentinel)
{
  MzString  white, token;
  char      key[256];
  int       ch, result;
  const hwpeq *eq = 0;

  while( 0 != (result = next_token(white, token, strm)) ) {
    if( sentinel && (result == 1) && strchr(sentinel, token[0]) )
      break;
    make_keyword(key, token);
    if( (eq = lookup_eqn(key)) != 0 ) {
      if( eq->latex )
    strcpy(key, eq->latex);
      else {
    key[0] = '\\';
    strcpy(key + 1, eq->key);
      }
      if( (eq->flag & EQ_CASE) && isupper(token[0]) )
    key[1] = sal::static_int_cast<char>(toupper(key[1]));
      token = key;
    }

    if( token[0] == '{' ) { // grouping
      sstr << white << token;
      eq2ltxconv(sstr, strm, "}");
      sstr << '}';
    }
    else if( eq && (eq->flag & EQ_ENV) ) {
      next_token(white, token, strm);
      if( token[0] != '{' ) return 0;
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
      while( (ch = strm->get()) != EOF && IS_WS(ch) )
    sstr << (char)ch;
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

void eq2latex(MzString& outs, char *s)
{
  assert(s);
  if( stk == 0 )
    stk = new eq_stack;

  MzString  tstr;

  istringstream tstrm(s);
  bool eqnarray = eq_sentence(tstr, &tstrm);
  istringstream strm(tstr.c_str());

  if( eqnarray )
    outs << "\\begin{array}{rllll}" << ENDL;
  eq2ltxconv(outs, &strm, 0);
  outs << ENDL;
  if( eqnarray )
    outs << "\\end{array}" << ENDL;
  delete stk;
  stk = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
