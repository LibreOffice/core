/*************************************************************************
 *
 *  $RCSfile: nodes.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:38:13 $
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
