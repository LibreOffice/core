/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
