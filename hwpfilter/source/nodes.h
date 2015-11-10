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

#ifndef INCLUDED_HWPFILTER_SOURCE_NODES_H
#define INCLUDED_HWPFILTER_SOURCE_NODES_H

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
          value = NULL;
          child = NULL;
          next = NULL;
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
          next = NULL;
          child = NULL;
#ifdef NODE_DEBUG
          count--;
          printf("Node count : [%d]\n",count);
#endif
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
