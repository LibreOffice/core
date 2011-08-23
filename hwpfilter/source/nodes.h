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

#ifndef __NODES_H__
#define __NODES_H__

#include <stdio.h>
#include <stdlib.h>
#include "list.hxx"

enum IDLIST {
     ID_MATHML,
     ID_LINES,
     ID_LINE,
     ID_EXPRLIST,
     ID_EXPR,
     ID_BEGIN,
     ID_END,
     ID_LEFT,
     ID_RIGHT,
     ID_SUBEXPR,
     ID_SUPEXPR,
     ID_SUBSUPEXPR,
     ID_FRACTIONEXPR,
     ID_OVER,
     ID_DECORATIONEXPR,
     ID_SQRTEXPR,
     ID_ROOTEXPR,
     ID_ARROWEXPR,
     ID_ACCENTEXPR,
     ID_UNARYEXPR,
     ID_PRIMARYEXPR,
     ID_BRACKET,
     ID_BLOCK,
     ID_PARENTH,
     ID_FENCE,
     ID_ABS,
     ID_IDENTIFIER,
     ID_STRING,
     ID_CHARACTER,
     ID_NUMBER,
     ID_OPERATOR,
     ID_SPACE,
     ID_DELIMETER
};

class Node{
public:
     Node(int _id) : id(_id)
     { 
          value = 0L; 
          child = 0L; 
          next = 0L; 
#ifdef NODE_DEBUG
          count++;
          printf("Node count : [%d]\n",count);
#endif
     }
     ~Node()
     {
          if( value ) free( value );
         // if( child ) delete child;
         // if( next ) delete next;
          next = 0L;
          child = 0L;
#ifdef NODE_DEBUG
          count--;
          printf("Node count : [%d]\n",count);
#endif
     }
     void print(){
     }
public:
     static int count; /* For memory debugging */
     int id;
     char *value;
     Node *child;
     Node *next; 
};

//static LinkedList<Node> nodelist;

#endif
