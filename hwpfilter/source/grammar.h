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

typedef union {
    const char *dval;
    const char *str;
    Node *ptr;
} YYSTYPE;

enum Grammar
{
    ACCENT              = 257,
    SMALL_GREEK         = 258,
    CAPITAL_GREEK       = 259,
    BINARY_OPERATOR     = 260,
    RELATION_OPERATOR   = 261,
    ARROW               = 262,
    GENERAL_IDEN        = 263,
    GENERAL_OPER        = 264,
    BIG_SYMBOL          = 265,
    FUNCTION            = 266,
    ROOT                = 267,
    FRACTION            = 268,
    SUBSUP              = 269,
    EQOVER              = 270,
    DELIMETER           = 271,
    LARGE_DELIM         = 272,
    DECORATION          = 273,
    SPACE_SYMBOL        = 274,
    CHARACTER           = 275,
    STRING              = 276,
    OPERATOR            = 277,
    EQBEGIN             = 278,
    EQEND               = 279,
    EQLEFT              = 280,
    EQRIGHT             = 281,
    NEWLINE             = 282,
    LEFT_DELIM          = 283,
    RIGHT_DELIM         = 284,
    DIGIT               = 285,
};

extern YYSTYPE yylval;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
