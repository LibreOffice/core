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


/*  A Bison parser, made from grammar.y
    by GNU Bison version 1.28  */

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 0
#endif

#include <vector>
#include <stdlib.h>
#include <string.h>

#include "grammar.hxx"
#include "lexer.hxx"
#include "nodes.h"

extern "C" {
#include "grammar.h"
}

std::vector<std::unique_ptr<Node>> nodelist;

static void yyerror(const char *);

static Node *top=nullptr;

int Node::count = 0;

#ifdef PARSE_DEBUG
#define debug printf
#else
static int debug(const char *format, ...);
#endif

#include <stdio.h>

#define YYFINAL     102
#define YYFLAG      -32768
#define YYNTBASE    43

#define YYTRANSLATE(x) (static_cast<unsigned>(x) <= 285 ? yytranslate[x] : 66)

const char yytranslate[] = {     0,
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


const short yyr1[] = {     0,
    43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
    43,    43,    43,    43,    43,    43,    43,    44,    44,    45,
    45,    45,    45,    45,    45,    45,    46,    46,    46,    46,
    46,    46,    46,    47,    48,    48,    49,    49,    50,    50,
    51,    51,    51,    52,    53,    54,    55,    56,    57,    58,
    58,    59,    59,    60,    61,    62,    62,    63,    64,    64,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    65,    65
};

const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
     2,     2,     2,     2,     3,     1,     2,     2,     2,     2,
     2,     3,     1,     3,     3,     2,     3,     2,     3,     2,
     3,     3,     5,     3,     5,     1,     2,     1,     2,     2,
     3,     2,     3,     2,     1,     1,     3,     1,     1,     2,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1
};

const short yydefact[] = {     0,
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

const short yydefgoto[] = {    28,
    29,    30,    84,    31,    32,    33,    51,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,   100,    44,    45,
    46,    47
};

const short yypact[] = {   393,
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

const short yypgoto[] = {    -2,
-32768,-32768,-32768,-32768,-32768,   -11,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -33,
   -24,   -27
};


#define YYLAST      457


const short yytable[] = {    50,
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

const short yycheck[] = {    11,
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

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define YYEMPTY     -2
#define YYEOF       0
#define YYACCEPT    goto yyacceptlab
#define YYABORT     goto yyabortlab

#define YYTERROR    1

#define YYLEX       yylex()

/* If nonreentrant, generate the variables here */

static int yychar;  /*  the lookahead symbol        */
YYSTYPE yylval;         /*  the semantic value of the       */
                /*  lookahead symbol            */

static int yynerrs;     /*  number of parse errors so far       */

/*  YYINITDEPTH indicates the initial size of the parser's stacks   */

#ifndef YYINITDEPTH
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

#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
static int yyparse();
#endif

static int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  int yystate;
  int yyn;
  short *yyssp;
  YYSTYPE *yyvsp;
  int yyerrstatus;  /*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;      /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH]; /*  the state stack         */
  YYSTYPE yyvsa[YYINITDEPTH];   /*  the semantic value stack        */

  short *yyss = yyssa;      /*  refer to the stacks through separate pointers */
  YYSTYPE *yyvs = yyvsa;    /*  to allow yyoverflow to reallocate them elsewhere */

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

  YYSTYPE yyval;        /*  the variable used to return     */
  yyval.dval = nullptr;
                /*  semantic values from the action */
                /*  routines                */

  int yylen;

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;     /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. So pushing a state here events the stacks.  */
yynewstate:

  *++yyssp = sal::static_int_cast<short>(yystate);

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
    {
      yyerror("parser stack overflow");
      if (yyfree_stacks)
        {
          free (yyss);
          free (yyvs);
        }
      return 2;
    }
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
    yystacksize = YYMAXDEPTH;
      yyfree_stacks = 1;
      yyss = static_cast<short *>(malloc (yystacksize * sizeof (*yyssp)));
      memcpy (yyss, yyss1, size * sizeof (*yyssp));
      yyvs = static_cast<YYSTYPE *>(malloc (yystacksize * sizeof (*yyvsp)));
      memcpy (yyvs, yyvs1, size * sizeof (*yyvsp));

      // https://lists.gnu.org/archive/html/bug-bison/2001-11/msg00021.html
      if (yyss1 != yyssa)
          free (yyss1);
      if (yyvs1 != yyvsa)
          free (yyvs1);

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;

      if (yyssp >= yyss + yystacksize - 1)
    YYABORT;
    }

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
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)      /* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;       /* Don't call YYLEX any more */

    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

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

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;

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

  switch (yyn) {

case 1:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 2:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 3:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 4:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 5:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 6:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 7:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 8:
{ yyval.ptr = new Node(ID_CHARACTER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 9:
{ yyval.ptr = new Node(ID_OPERATOR); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 10:
{ yyval.ptr = new Node(ID_OPERATOR); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 11:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 12:
{ yyval.ptr = new Node(ID_DELIMITER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 13:
{ yyval.ptr = new Node(ID_DELIMITER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 14:
{ yyval.ptr = new Node(ID_IDENTIFIER); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Identifier : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 15:
{ yyval.ptr = new Node(ID_CHARACTER); yyval.ptr->value.reset(strdup("|")); debug("Identifier : '|'\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 16:
{ yyval.ptr = new Node(ID_NUMBER); yyval.ptr->value.reset(strdup(yyvsp[0].dval)); debug("Number : %s\n",yyvsp[0].dval); nodelist.emplace_back(yyval.ptr);
    break;}
case 17:
{ yyval.ptr = new Node(ID_STRING); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("String : %s\n",yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 18:
{ yyval.ptr = new Node(ID_PRIMARYEXPR); yyval.ptr->child = yyvsp[0].ptr; debug("PrimaryExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 19:
{ yyval.ptr = new Node(ID_PRIMARYEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyval.ptr->next = yyvsp[0].ptr; debug("PrimaryExpr : PrimaryExpr Identifier\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 20:
{ yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup("(")); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 21:
{ yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup("[")); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 22:
{   yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup("{")); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 23:
{   yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup("<")); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 24:
{ yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup("|")); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 25:
{ yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup("||")); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 26:
{ yyval.ptr = new Node(ID_LEFT); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("EQLeft \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 27:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup(")")); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 28:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup("]")); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 29:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup("}")); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 30:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup(">")); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 31:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup("|")); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 32:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup("||")); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 33:
{ yyval.ptr = new Node(ID_RIGHT); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("EQRight \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 34:
{ yyval.ptr = new Node(ID_FENCE); yyval.ptr->child=yyvsp[-2].ptr; yyvsp[-2].ptr->next=yyvsp[-1].ptr; yyvsp[-1].ptr->next=yyvsp[0].ptr; debug("Fence \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 35:
{ yyval.ptr = new Node(ID_PARENTH); yyval.ptr->child = yyvsp[-1].ptr; debug("Parenth: '(' ExprList ')' \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 36:
{ yyval.ptr = new Node(ID_PARENTH); debug("Parenth: '(' ')' \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 37:
{ yyval.ptr = new Node(ID_BLOCK); yyval.ptr->child = yyvsp[-1].ptr; debug("Block: '{' ExprList '}' \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 38:
{ yyval.ptr = new Node(ID_BLOCK); debug("Block: '{' '}' \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 39:
{ yyval.ptr = new Node(ID_BRACKET); yyval.ptr->child = yyvsp[-1].ptr; debug("Bracket \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 40:
{ yyval.ptr = new Node(ID_BRACKET); debug("Bracket \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 41:
{ yyval.ptr = new Node(ID_SUBEXPR); yyval.ptr->child = yyvsp[-2].ptr; yyvsp[-2].ptr->next = yyvsp[0].ptr; debug("SubSupExpr : ID_SUBEXPR\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 42:
{ yyval.ptr = new Node(ID_SUPEXPR); yyval.ptr->child = yyvsp[-2].ptr; yyvsp[-2].ptr->next = yyvsp[0].ptr; debug("SubSupExpr : ID_SUPEXPR\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 43:
{ yyval.ptr = new Node(ID_SUBSUPEXPR); yyval.ptr->child=yyvsp[-4].ptr; yyvsp[-4].ptr->next=yyvsp[-2].ptr; yyvsp[-2].ptr->next=yyvsp[0].ptr; debug("SubSupExpr : ID_SUBSUPEXPR\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 44:
{ yyval.ptr = new Node(ID_FRACTIONEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr; debug("FractionExpr : %s\n",yyvsp[-2].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 45:
{ yyval.ptr = new Node(ID_OVER); yyval.ptr->child = yyvsp[-3].ptr; yyvsp[-3].ptr->next = yyvsp[-1].ptr; debug("OverExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 46:
{ yyval.ptr = new Node(ID_ACCENTEXPR); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Accent : %s\n", yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 47:
{ yyval.ptr = new Node(ID_ACCENTEXPR); yyval.ptr->child=yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr; debug("AccentExpr \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 48:
{ yyval.ptr = new Node(ID_DECORATIONEXPR); yyval.ptr->value.reset(strdup(yyvsp[0].str)); debug("Decoration : %s\n", yyvsp[0].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 49:
{ yyval.ptr = new Node(ID_DECORATIONEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr; debug("DecorationExpr \n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 50:
{ yyval.ptr = new Node(ID_SQRTEXPR); yyval.ptr->child = yyvsp[0].ptr; debug("RootExpr : %s\n", yyvsp[-1].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 51:
{ yyval.ptr = new Node(ID_ROOTEXPR); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next= yyvsp[0].ptr; debug("RootExpr : %s\n", yyvsp[-2].str); nodelist.emplace_back(yyval.ptr);
    break;}
case 52:
{ yyval.ptr = new Node(ID_BEGIN); yyval.ptr->child = yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr);  debug("BeginExpr\n");
    break;}
case 53:
{ yyval.ptr = new Node(ID_BEGIN); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next= yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr); debug("BeginExpr\n");
    break;}
case 54:
{ yyval.ptr = new Node(ID_END); yyval.ptr->child = yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr); debug("EndExpr\n");
    break;}
case 55:
{ yyval.ptr = new Node(ID_MATHML); yyval.ptr->child = yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr); top = yyval.ptr; debug("MathML\n");
    break;}
case 56:
{ yyval.ptr = new Node(ID_LINES); yyval.ptr->child = yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr); debug("Lines\n");
    break;}
case 57:
{ yyval.ptr = new Node(ID_LINES); yyval.ptr->child = yyvsp[-2].ptr; yyvsp[-2].ptr->next = yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr); debug("Lines\n");
    break;}
case 58:
{ yyval.ptr = new Node(ID_LINE); yyval.ptr->child = yyvsp[0].ptr; nodelist.emplace_back(yyval.ptr); debug("Line\n");
    break;}
case 59:
{ yyval.ptr = new Node(ID_EXPRLIST); yyval.ptr->child = yyvsp[0].ptr; debug("ExprList : Expr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 60:
{ yyval.ptr = new Node(ID_EXPRLIST); yyval.ptr->child = yyvsp[-1].ptr; yyvsp[-1].ptr->next = yyvsp[0].ptr;  debug("ExprList : ExprList Expr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 61:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : Block\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 62:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : Parenth\n");  nodelist.emplace_back(yyval.ptr);
    break;}
case 63:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : Fence\n");  nodelist.emplace_back(yyval.ptr);
    break;}
case 64:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : SubSupExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 65:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : FractionExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 66:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : OverExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 67:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : DecorationExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 68:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : RootExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 69:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : AccentExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 70:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : PrimaryExpr\n"); nodelist.emplace_back(yyval.ptr);
    break;}
case 71:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : BeginExpr\n");  nodelist.emplace_back(yyval.ptr);
    break;}
case 72:
{ yyval.ptr = new Node(ID_EXPR); yyval.ptr->child = yyvsp[0].ptr; debug("Expr : EndExpr\n");  nodelist.emplace_back(yyval.ptr);
    break;}
}
   /* the action file gets copied in place of this dollarsign */

  yyvsp -= yylen;
  yyssp -= yylen;

  *++yyvsp = yyval;

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
      /* Start X at -yyn if negative to avoid negative indexes in yycheck.  */
      for (x = ((yyn < 0) ? -yyn : 0);
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
          for (x = ((yyn < 0) ? -yyn : 0);
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

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;      /* Each real token shifted decrements this */

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

  *++yyvsp = yylval;

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
    }
  return 1;
}

Node *mainParse(const char *_code)
{
    initFlex( _code );
    top = nullptr;
    yyparse();
    deinitFlex();

    if( top )
        return top;
    else
        return nullptr;
}

void yyerror(const char * /*err*/)
{
//  printf("REALKING ERR[%s]\n",err);
    // if error, delete all nodes.
    nodelist.clear();
    top = nullptr;
}

#ifndef PARSE_DEBUG
int debug(const char * /*format*/, ...)
{
    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
