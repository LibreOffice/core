/*************************************************************************
 *
 *  $RCSfile: grammar.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:35:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
