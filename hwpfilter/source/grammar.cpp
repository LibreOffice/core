/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


/*  A Bison parser, made from grammar.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#ifndef YYDEBUG 
#define YYDEBUG 0
#endif
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 0
#endif

#define	ACCENT	257
#define	SMALL_GREEK	258
#define	CAPITAL_GREEK	259
#define	BINARY_OPERATOR	260
#define	RELATION_OPERATOR	261
#define	ARROW	262
#define	GENERAL_IDEN	263
#define	GENERAL_OPER	264
#define	BIG_SYMBOL	265
#define	FUNCTION	266
#define	ROOT	267
#define	FRACTION	268
#define	SUBSUP	269
#define	EQOVER	270
#define	DELIMETER	271
#define	LARGE_DELIM	272
#define	DECORATION	273
#define	SPACE_SYMBOL	274
#define	CHARACTER	275
#define	STRING	276
#define	OPERATOR	277
#define	EQBEGIN	278
#define	EQEND	279
#define	EQLEFT	280
#define	EQRIGHT	281
#define	NEWLINE	282
#define	LEFT_DELIM	283
#define	RIGHT_DELIM	284
#define	DIGIT	285

#line 1 "grammar.y"

#include <stdlib.h>
#include <string.h>

#include "nodes.h"

LinkedList<Node> nodelist;

Node *mainParse(const char *_code);
void yyerror(const char *);
int yylex();
void initFlex( const char *s );
void viewResult(Node *res);

Node *top=0L;

int Node::count = 0;

#define allocValue(x,y) \
x = (char *)malloc( strlen(y) +1 ); \
strcpy(x,y)

#define allocChar(x,y) \
x = (char *)malloc(2); \
sprintf(x,"%c",y)
#ifdef PARSE_DEBUG
#define debug printf
#else
int debug(const char *format, ...);
#endif


#line 35 "grammar.y"
typedef union {
    char *dval;
    char *str;
    Node *ptr;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		102
#define	YYFLAG		-32768
#define	YYNTBASE	43

#define YYTRANSLATE(x) ((unsigned)(x) <= 285 ? yytranslate[x] : 66)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    33,
    37,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    36,
     2,    40,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    34,     2,    38,    42,    41,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    35,    32,    39,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    24,    26,    28,    30,    32,    34,    36,    39,
    42,    45,    48,    51,    54,    58,    60,    63,    66,    69,
    72,    75,    79,    81,    85,    89,    92,    96,    99,   103,
   106,   110,   114,   120,   124,   130,   132,   135,   137,   140,
   143,   147,   150,   154,   157,   159,   161,   165,   167,   169,
   172,   174,   176,   178,   180,   182,   184,   186,   188,   190,
   192,   194
};

static const short yyrhs[] = {     4,
     0,     9,     0,    10,     0,     5,     0,    11,     0,    12,
     0,    20,     0,    21,     0,     7,     0,     6,     0,    23,
     0,    17,     0,    18,     0,     8,     0,    32,     0,    31,
     0,    22,     0,    43,     0,    44,    43,     0,    26,    33,
     0,    26,    34,     0,    26,    35,     0,    26,    36,     0,
    26,    32,     0,    26,    32,    32,     0,    29,     0,    27,
    37,     0,    27,    38,     0,    27,    39,     0,    27,    40,
     0,    27,    32,     0,    27,    32,    32,     0,    30,     0,
    45,    64,    46,     0,    33,    64,    37,     0,    33,    37,
     0,    35,    64,    39,     0,    35,    39,     0,    34,    64,
    38,     0,    34,    38,     0,    65,    41,    65,     0,    65,
    42,    65,     0,    65,    41,    49,    42,    49,     0,    14,
    49,    49,     0,    35,    64,    16,    64,    39,     0,     3,
     0,    54,    49,     0,    19,     0,    56,    49,     0,    13,
    49,     0,    13,    50,    49,     0,    24,    49,     0,    24,
    49,    49,     0,    25,    49,     0,    62,     0,    63,     0,
    62,    28,    63,     0,    64,     0,    65,     0,    64,    65,
     0,    49,     0,    48,     0,    47,     0,    51,     0,    52,
     0,    53,     0,    57,     0,    58,     0,    55,     0,    44,
     0,    59,     0,    60,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    59,    61,    62,    63,    64,    65,    66,    67,    68,    69,
    70,    71,    72,    73,    74,    75,    76,    79,    81,    84,
    86,    87,    88,    89,    90,    91,    94,    96,    97,    98,
    99,   100,   101,   104,   108,   110,   113,   115,   118,   120,
   123,   125,   126,   129,   133,   138,   142,   145,   149,   153,
   155,   158,   160,   163,   168,   172,   174,   177,   181,   183,
   186,   188,   189,   190,   191,   192,   193,   194,   195,   196,
   197,   198
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","ACCENT",
"SMALL_GREEK","CAPITAL_GREEK","BINARY_OPERATOR","RELATION_OPERATOR","ARROW",
"GENERAL_IDEN","GENERAL_OPER","BIG_SYMBOL","FUNCTION","ROOT","FRACTION","SUBSUP",
"EQOVER","DELIMETER","LARGE_DELIM","DECORATION","SPACE_SYMBOL","CHARACTER","STRING",
"OPERATOR","EQBEGIN","EQEND","EQLEFT","EQRIGHT","NEWLINE","LEFT_DELIM","RIGHT_DELIM",
"DIGIT","'|'","'('","'['","'{'","'<'","')'","']'","'}'","'>'","'_'","'^'","Identifier",
"PrimaryExpr","EQLeft","EQRight","Fence","Parenth","Block","Bracket","SubSupExpr",
"FractionExpr","OverExpr","Accent","AccentExpr","Decoration","DecorationExpr",
"RootExpr","BeginExpr","EndExpr","MathML","Lines","Line","ExprList","Expr", NULL
};
#endif

static const short yyr1[] = {     0,
    43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
    43,    43,    43,    43,    43,    43,    43,    44,    44,    45,
    45,    45,    45,    45,    45,    45,    46,    46,    46,    46,
    46,    46,    46,    47,    48,    48,    49,    49,    50,    50,
    51,    51,    51,    52,    53,    54,    55,    56,    57,    58,
    58,    59,    59,    60,    61,    62,    62,    63,    64,    64,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
     2,     2,     2,     2,     3,     1,     2,     2,     2,     2,
     2,     3,     1,     3,     3,     2,     3,     2,     3,     2,
     3,     3,     5,     3,     5,     1,     2,     1,     2,     2,
     3,     2,     3,     2,     1,     1,     3,     1,     1,     2,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1
};

static const short yydefact[] = {     0,
    46,     1,     4,    10,     9,    14,     2,     3,     5,     6,
     0,     0,    12,    13,    48,     7,     8,    17,    11,     0,
     0,     0,    26,    16,    15,     0,     0,    18,    70,     0,
    63,    62,    61,    64,    65,    66,     0,    69,     0,    67,
    68,    71,    72,    55,    56,    58,    59,     0,     0,    50,
     0,     0,    52,    54,    24,    20,    21,    22,    23,    36,
     0,    38,     0,    19,     0,    47,    49,     0,    60,     0,
     0,    40,     0,     0,    51,    44,    53,    25,    35,     0,
    37,     0,    33,    34,    57,    61,    41,    42,    39,     0,
    31,    27,    28,    29,    30,     0,    45,    32,    43,     0,
     0,     0
};

static const short yydefgoto[] = {    28,
    29,    30,    84,    31,    32,    33,    51,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,   100,    44,    45,
    46,    47
};

static const short yypact[] = {   393,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -30,   -19,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -19,
   -19,    -3,-32768,-32768,-32768,   290,   107,-32768,   425,   393,
-32768,-32768,-32768,-32768,-32768,-32768,   -19,-32768,   -19,-32768,
-32768,-32768,-32768,   -20,-32768,   393,   -21,   218,   107,-32768,
   -19,   -19,   -19,-32768,   -15,-32768,-32768,-32768,-32768,-32768,
   325,-32768,    70,-32768,   360,-32768,-32768,   393,   -21,   393,
   393,-32768,   254,   144,-32768,-32768,-32768,-32768,-32768,   393,
-32768,   -25,-32768,-32768,-32768,   -31,   -21,   -21,-32768,   181,
   -14,-32768,-32768,-32768,-32768,   -19,-32768,-32768,-32768,    22,
    23,-32768
};

static const short yypgoto[] = {    -2,
-32768,-32768,-32768,-32768,-32768,   -11,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -33,
   -24,   -27
};


#define	YYLAST		457


static const short yytable[] = {    50,
    52,    61,    63,    48,    49,    65,    91,    68,    53,    54,
    96,    92,    93,    94,    95,    49,    78,    98,    69,    70,
    71,   101,   102,    73,    74,    66,    64,    67,    55,    56,
    57,    58,    59,    69,    85,    69,     0,    69,     0,    75,
    76,    77,    87,    88,     0,    69,    69,     0,     0,     0,
     0,     0,     0,     0,     0,    90,     0,     0,    86,     0,
     0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     1,     2,     3,     4,     5,     6,     7,     8,
     9,    10,    11,    12,    99,    80,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
    24,    25,    26,     0,    27,     0,     0,     0,    81,     1,
     2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
    12,     0,     0,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,     0,     0,    23,     0,    24,    25,    26,
     0,    27,     0,     0,     0,    62,     1,     2,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,     0,     0,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
     0,     0,    23,     0,    24,    25,    26,     0,    27,     0,
     0,     0,    81,     1,     2,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,     0,     0,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,     0,     0,    23,
     0,    24,    25,    26,     0,    27,     0,     0,     0,    97,
     1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,     0,     0,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,     0,     0,    23,     0,    24,    25,
    26,     0,    27,     0,     0,    72,     1,     2,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,     0,     0,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
     0,     0,    23,     0,    24,    25,    26,     0,    27,     0,
     0,    89,     1,     2,     3,     4,     5,     6,     7,     8,
     9,    10,    11,    12,     0,     0,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,     0,     0,    23,     0,
    24,    25,    26,     0,    27,     0,    60,     1,     2,     3,
     4,     5,     6,     7,     8,     9,    10,    11,    12,     0,
     0,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,     0,     0,    23,     0,    24,    25,    26,     0,    27,
     0,    79,     1,     2,     3,     4,     5,     6,     7,     8,
     9,    10,    11,    12,     0,     0,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    82,     0,    23,    83,
    24,    25,    26,     0,    27,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,     0,     0,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
     0,    23,     0,    24,    25,    26,     0,    27,     2,     3,
     4,     5,     6,     7,     8,     9,    10,     0,     0,     0,
     0,    13,    14,     0,    16,    17,    18,    19,     0,     0,
     0,     0,     0,     0,     0,    24,    25
};

static const short yycheck[] = {    11,
    12,    26,    27,    34,    35,    30,    32,    28,    20,    21,
    42,    37,    38,    39,    40,    35,    32,    32,    46,    41,
    42,     0,     0,    48,    49,    37,    29,    39,    32,    33,
    34,    35,    36,    61,    68,    63,    -1,    65,    -1,    51,
    52,    53,    70,    71,    -1,    73,    74,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    70,    -1,
    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    96,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    -1,    -1,    29,    -1,
    31,    32,    33,    -1,    35,    -1,    -1,    -1,    39,     3,
     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    -1,    -1,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    -1,    -1,    29,    -1,    31,    32,    33,
    -1,    35,    -1,    -1,    -1,    39,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    -1,    -1,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    -1,    -1,    29,    -1,    31,    32,    33,    -1,    35,    -1,
    -1,    -1,    39,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    -1,    -1,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    -1,    -1,    29,
    -1,    31,    32,    33,    -1,    35,    -1,    -1,    -1,    39,
     3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
    13,    14,    -1,    -1,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    -1,    -1,    29,    -1,    31,    32,
    33,    -1,    35,    -1,    -1,    38,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    -1,    -1,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    -1,    -1,    29,    -1,    31,    32,    33,    -1,    35,    -1,
    -1,    38,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    -1,    -1,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    -1,    -1,    29,    -1,
    31,    32,    33,    -1,    35,    -1,    37,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    -1,
    -1,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    -1,    -1,    29,    -1,    31,    32,    33,    -1,    35,
    -1,    37,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    -1,    -1,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    -1,    29,    30,
    31,    32,    33,    -1,    35,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    -1,    -1,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
    -1,    29,    -1,    31,    32,    33,    -1,    35,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    -1,    -1,    -1,
    -1,    17,    18,    -1,    20,    21,    22,    23,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    31,    32
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <sal/alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
     instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
         and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
                /*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
                /*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if defined __GNUC__ 
#if __GNUC__ > 1        /* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#endif
#else               /* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
  yyval.dval = 0;
                /*  semantic values from the action	*/
                /*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = sal::static_int_cast<short>(yystate);

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
     the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
     but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
         &yyss1, size * sizeof (*yyssp),
         &yyvs1, size * sizeof (*yyvsp),
         &yyls1, size * sizeof (*yylsp),
         &yystacksize);
#else
      yyoverflow("parser stack overflow",
         &yyss1, size * sizeof (*yyssp),
         &yyvs1, size * sizeof (*yyvsp),
         &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
    {
      yyerror("parser stack overflow");
      if (yyfree_stacks)
        {
          free (yyss);
          free (yyvs);
#ifdef YYLSP_NEEDED
          free (yyls);
#endif
        }
      return 2;
    }
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
    yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
           size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
           size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
           size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
    fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
    YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
    fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
    fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
    {
      fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
      /* Give the individual parser a way to print the precise meaning
         of a token, for further debugging info.  */
#ifdef YYPRINT
      YYPRINT (stderr, yychar, yylval);
#endif
      fprintf (stderr, ")\n");
    }
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
    goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
           yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
    fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 60 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 2:
#line 61 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 3:
#line 62 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 4:
#line 63 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 5:
#line 64 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 6:
#line 65 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 7:
#line 66 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 8:
#line 67 "grammar.y"
{ yyval.ptr = new Node(ID_CHARACTER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 9:
#line 68 "grammar.y"
{ yyval.ptr = new Node(ID_OPERATOR); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 10:
#line 69 "grammar.y"
{ yyval.ptr = new Node(ID_OPERATOR); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 11:
#line 70 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 12:
#line 71 "grammar.y"
{ yyval.ptr = new Node(ID_DELIMETER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 13:
#line 72 "grammar.y"
{ yyval.ptr = new Node(ID_DELIMETER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 14:
#line 73 "grammar.y"
{ yyval.ptr = new Node(ID_IDENTIFIER); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Identifier : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 15:
#line 74 "grammar.y"
{ yyval.ptr = new Node(ID_CHARACTER); allocChar(yyval.ptr->value , '|'); debug("Identifier : '|'\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 16:
#line 75 "grammar.y"
{ yyval.ptr = new Node(ID_NUMBER); allocValue(yyval.ptr->value , yyvsp[0].dval); debug("Number : %s\n",yyvsp[0].dval); nodelist.insert(yyval.ptr); ;
    break;}
case 17:
#line 76 "grammar.y"
{ yyval.ptr = new Node(ID_STRING); allocValue(yyval.ptr->value , yyvsp[0].str); debug("String : %s\n",yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 18:
#line 80 "grammar.y"
{ yyval.ptr = new Node(ID_PRIMARYEXPR); yyval.ptr->child = yyvsp[0].ptr; debug("PrimaryExpr\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 19:
#line 81 "grammar.y"
{ yyval.ptr = new Node(ID_PRIMARYEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyval.ptr->next = yyvsp[0].ptr; debug("PrimaryExpr : PrimaryExpr Identifier\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 20:
#line 85 "grammar.y"
{ yyval.ptr = new Node(ID_LEFT); allocChar(yyval.ptr->value , '('); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 21:
#line 86 "grammar.y"
{ yyval.ptr = new Node(ID_LEFT); allocChar(yyval.ptr->value , '['); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 22:
#line 87 "grammar.y"
{   yyval.ptr = new Node(ID_LEFT); allocChar(yyval.ptr->value , '{'); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 23:
#line 88 "grammar.y"
{   yyval.ptr = new Node(ID_LEFT); allocChar(yyval.ptr->value , '<'); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 24:
#line 89 "grammar.y"
{ yyval.ptr = new Node(ID_LEFT); allocChar(yyval.ptr->value , '|'); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 25:
#line 90 "grammar.y"
{ yyval.ptr = new Node(ID_LEFT); allocValue(yyval.ptr->value , "||"); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 26:
#line 91 "grammar.y"
{ yyval.ptr = new Node(ID_LEFT); allocValue(yyval.ptr->value , yyvsp[0].str); debug("EQLeft \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 27:
#line 95 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocChar(yyval.ptr->value , ')'); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 28:
#line 96 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocChar(yyval.ptr->value , ']'); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 29:
#line 97 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocChar(yyval.ptr->value , '}'); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 30:
#line 98 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocChar(yyval.ptr->value , '>'); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 31:
#line 99 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocChar(yyval.ptr->value , '|'); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 32:
#line 100 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocValue(yyval.ptr->value , "||"); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 33:
#line 101 "grammar.y"
{ yyval.ptr = new Node(ID_RIGHT); allocValue(yyval.ptr->value , yyvsp[0].str); debug("EQRight \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 34:
#line 105 "grammar.y"
{ yyval.ptr = new Node(ID_FENCE); yyval.ptr->child=yyvsp[-2].ptr; yyvsp[-2].ptr->next=yyvsp[-1].ptr; yyvsp[-1].ptr->next=yyvsp[0].ptr; debug("Fence \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 35:
#line 109 "grammar.y"
{ yyval.ptr = new Node(ID_PARENTH); yyval.ptr->child = yyvsp[-1].ptr; debug("Parenth: '(' ExprList ')' \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 36:
#line 110 "grammar.y"
{ yyval.ptr = new Node(ID_PARENTH); debug("Parenth: '(' ')' \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 37:
#line 114 "grammar.y"
{ yyval.ptr = new Node(ID_BLOCK); yyval.ptr->child = yyvsp[-1].ptr; debug("Block: '{' ExprList '}' \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 38:
#line 115 "grammar.y"
{ yyval.ptr = new Node(ID_BLOCK); debug("Block: '{' '}' \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 39:
#line 119 "grammar.y"
{ yyval.ptr = new Node(ID_BRACKET); yyval.ptr->child = yyvsp[-1].ptr; debug("Bracket \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 40:
#line 120 "grammar.y"
{ yyval.ptr = new Node(ID_BRACKET); debug("Bracket \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 41:
#line 124 "grammar.y"
{ yyval.ptr = new Node(ID_SUBEXPR); yyval.ptr->child = yyvsp[-2].ptr; yyvsp[-2].ptr->next = yyvsp[0].ptr; debug("SubSupExpr : ID_SUBEXPR\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 42:
#line 125 "grammar.y"
{ yyval.ptr = new Node(ID_SUPEXPR); yyval.ptr->child = yyvsp[-2].ptr; yyvsp[-2].ptr->next = yyvsp[0].ptr; debug("SubSupExpr : ID_SUPEXPR\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 43:
#line 126 "grammar.y"
{ yyval.ptr = new Node(ID_SUBSUPEXPR); yyval.ptr->child=yyvsp[-4].ptr; yyvsp[-4].ptr->next=yyvsp[-2].ptr; yyvsp[-2].ptr->next=yyvsp[0].ptr; debug("SubSupExpr : ID_SUBSUPEXPR\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 44:
#line 130 "grammar.y"
{ yyval.ptr = new Node(ID_FRACTIONEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr; debug("FractionExpr : %s\n",yyvsp[-2].str); nodelist.insert(yyval.ptr); ;
    break;}
case 45:
#line 134 "grammar.y"
{ yyval.ptr = new Node(ID_OVER); yyval.ptr->child = yyvsp[-3].ptr; yyvsp[-3].ptr->next = yyvsp[-1].ptr; debug("OverExpr\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 46:
#line 139 "grammar.y"
{ yyval.ptr = new Node(ID_ACCENTEXPR); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Accent : %s\n", yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 47:
#line 143 "grammar.y"
{ yyval.ptr = new Node(ID_ACCENTEXPR); yyval.ptr->child=yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr; debug("AccentExpr \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 48:
#line 146 "grammar.y"
{ yyval.ptr = new Node(ID_DECORATIONEXPR); allocValue(yyval.ptr->value , yyvsp[0].str); debug("Decoration : %s\n", yyvsp[0].str); nodelist.insert(yyval.ptr); ;
    break;}
case 49:
#line 150 "grammar.y"
{ yyval.ptr = new Node(ID_DECORATIONEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr; debug("DecorationExpr \n"); nodelist.insert(yyval.ptr); ;
    break;}
case 50:
#line 154 "grammar.y"
{ yyval.ptr = new Node(ID_SQRTEXPR); yyval.ptr->child = yyvsp[0].ptr; debug("RootExpr : %s\n", yyvsp[-1].str); nodelist.insert(yyval.ptr); ;
    break;}
case 51:
#line 155 "grammar.y"
{ yyval.ptr = new Node(ID_ROOTEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next= yyvsp[0].ptr; debug("RootExpr : %s\n", yyvsp[-2].str); nodelist.insert(yyval.ptr); ;
    break;}
case 52:
#line 159 "grammar.y"
{ yyval.ptr = new Node(ID_BEGIN); yyval.ptr->child = yyvsp[0].ptr; nodelist.insert(yyval.ptr);  debug("BeginExpr\n"); ;
    break;}
case 53:
#line 160 "grammar.y"
{ yyval.ptr = new Node(ID_BEGIN); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next= yyvsp[0].ptr; nodelist.insert(yyval.ptr); debug("BeginExpr\n"); ;
    break;}
case 54:
#line 164 "grammar.y"
{ yyval.ptr = new Node(ID_END); yyval.ptr->child = yyvsp[0].ptr; nodelist.insert(yyval.ptr); debug("EndExpr\n"); ;
    break;}
case 55:
#line 169 "grammar.y"
{ yyval.ptr = new Node(ID_MATHML); yyval.ptr->child = yyvsp[0].ptr; nodelist.insert(yyval.ptr); top = yyval.ptr; debug("MathML\n"); ;
    break;}
case 56:
#line 173 "grammar.y"
{ yyval.ptr = new Node(ID_LINES); yyval.ptr->child = yyvsp[0].ptr; nodelist.insert(yyval.ptr); debug("Lines\n"); ;
    break;}
case 57:
#line 174 "grammar.y"
{ yyval.ptr = new Node(ID_LINES); yyval.ptr->child = yyvsp[-2].ptr; yyvsp[-2].ptr->next = yyvsp[0].ptr; nodelist.insert(yyval.ptr); debug("Lines\n"); ;
    break;}
case 58:
#line 178 "grammar.y"
{ yyval.ptr = new Node(ID_LINE); yyval.ptr->child = yyvsp[0].ptr; nodelist.insert(yyval.ptr); debug("Line\n"); ;
    break;}
case 59:
#line 182 "grammar.y"
{ yyval.ptr = new Node(ID_EXPRLIST); yyval.ptr->child = yyvsp[0].ptr; debug("ExprList : Expr\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 60:
#line 183 "grammar.y"
{ yyval.ptr = new Node(ID_EXPRLIST); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr;  debug("ExprList : ExprList Expr\n"); nodelist.insert(yyval.ptr); ;
    break;}
case 61:
#line 187 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : Block\n"); nodelist.insert(yyval.ptr);;
    break;}
case 62:
#line 188 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : Parenth\n");  nodelist.insert(yyval.ptr);;
    break;}
case 63:
#line 189 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : Fence\n");  nodelist.insert(yyval.ptr);;
    break;}
case 64:
#line 190 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : SubSupExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 65:
#line 191 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : FractionExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 66:
#line 192 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : OverExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 67:
#line 193 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : DecorationExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 68:
#line 194 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : RootExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 69:
#line 195 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : AccentExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 70:
#line 196 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : PrimaryExpr\n"); nodelist.insert(yyval.ptr);;
    break;}
case 71:
#line 197 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : BeginExpr\n");  nodelist.insert(yyval.ptr);;
    break;}
case 72:
#line 198 "grammar.y"
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : EndExpr\n");  nodelist.insert(yyval.ptr);;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
    fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
    {
      int size = 0;
      char *msg;
      int x, count;

      count = 0;
      /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
      for (x = (yyn < 0 ? -yyn : 0);
           x < (sizeof(yytname) / sizeof(char *)); x++)
        if (yycheck[x + yyn] == x)
          size += strlen(yytname[x]) + 15, count++;
      msg = (char *) malloc(size + 15);
      if (msg != 0)
        {
          strcpy(msg, "parse error");

          if (count < 5)
        {
          count = 0;
          for (x = (yyn < 0 ? -yyn : 0);
               x < (sizeof(yytname) / sizeof(char *)); x++)
            if (yycheck[x + yyn] == x)
              {
            strcat(msg, count == 0 ? ", expecting `" : " or `");
            strcat(msg, yytname[x]);
            strcat(msg, "'");
            count++;
              }
        }
          yyerror(msg);
          free(msg);
        }
      else
        yyerror ("parse error; also virtual memory exceeded");
    }
      else
#endif /* YYERROR_VERBOSE */
    yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
    YYABORT;

#if YYDEBUG != 0
      if (yydebug)
    fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
    fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
    goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 202 "grammar.y"


Node *mainParse(const char *_code)
{
    initFlex( _code );
    top = 0L;
    yyparse();
    if( top )
        return top;
    else
        return 0L;
}

void yyerror(const char * /*err*/)
{
//	printf("REALKING ERR[%s]\n",err);
    // if error, delete all nodes.
    Node *pNode = 0L;
    int ncount = nodelist.count();
    for( int i = 0 ; i < ncount ; i++){
        pNode = nodelist.remove(0);
        delete pNode;
    }
    top = 0L;
}

#ifndef PARSE_DEBUG
int debug(const char * /*format*/, ...)
{
    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
