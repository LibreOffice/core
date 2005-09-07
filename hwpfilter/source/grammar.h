/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grammar.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:32:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

typedef union {
    char *dval;
    char *str;
    Node *ptr;
} YYSTYPE;
#define ACCENT  257
#define SMALL_GREEK 258
#define CAPITAL_GREEK   259
#define BINARY_OPERATOR 260
#define RELATION_OPERATOR   261
#define ARROW   262
#define GENERAL_IDEN    263
#define GENERAL_OPER    264
#define BIG_SYMBOL  265
#define FUNCTION    266
#define ROOT    267
#define FRACTION    268
#define SUBSUP  269
#define EQOVER  270
#define DELIMETER   271
#define LARGE_DELIM 272
#define DECORATION  273
#define SPACE_SYMBOL    274
#define CHARACTER   275
#define STRING  276
#define OPERATOR    277
#define EQBEGIN 278
#define EQEND   279
#define EQLEFT  280
#define EQRIGHT 281
#define NEWLINE 282
#define LEFT_DELIM  283
#define RIGHT_DELIM 284
#define DIGIT   285

#ifndef _WIN32
extern YYSTYPE yylval;
#endif
