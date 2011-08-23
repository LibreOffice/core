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

typedef union {
    char *dval;
    char *str;
    Node *ptr;
} YYSTYPE;
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

#ifndef _WIN32
extern YYSTYPE yylval;
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
