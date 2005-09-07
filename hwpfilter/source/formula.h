/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formula.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:31:29 $
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

#ifndef __FORMULA_H__
#define __FORMULA_H__

// DVO: remove DEBUG dependency
// #ifndef DEBUG
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include "attributes.hxx"
// DVO: remove DEBUG dependency
// #endif

class Node;

// DVO: remove DEBUG dependency
// #ifndef DEBUG
using namespace ::com::sun::star::xml::sax;
// #endif


class Formula{
public:
     Formula(char *_eq, int _ishwpeq = 1) {
          eq = _eq;
          isHwpEQ = _ishwpeq;
          trim();
     }
     virtual ~Formula(){ }

// DVO: remove DEBUG dependency
// #ifndef DEBUG
     void setDocumentHandler(Reference < XDocumentHandler > xHandler ){
          rDocumentHandler = xHandler;
     }
     void setAttributeListImpl( AttributeListImpl *p ){
          pList = p;
          rList = (XAttributeList *) pList;
     }
// DVO: remove DEBUG dependency
// #endif
     int parse();
private:
     void trim();
     void makeMathML(Node *res);
     void makeLines(Node *res);
     void makeLine(Node *res);
     void makeExprList(Node *res);
     void makeExpr(Node *res);
     void makePrimary(Node *res);
     void makeIdentifier(Node *res);
     void makeSubSup(Node *res);
     void makeUnderOver(Node *res);
     void makeFraction(Node *res);
     void makeDecoration(Node *res);
     void makeFunction(Node *res);
     void makeRoot(Node *res);
     void makeArrow(Node *res);
     void makeAccent(Node *res);
     void makeParenth(Node *res);
     void makeFence(Node *res);
     void makeBracket(Node *res);
     void makeBlock(Node *res);
     void makeOperator(Node *res);
     void makeDelimeter(Node *res);
     void makeBegin(Node *res);
     void makeEnd(Node *res);

private:
// DVO: remove DEBUG dependency
// #ifndef DEBUG
     Reference< XDocumentHandler >   rDocumentHandler;
     Reference< XAttributeList > rList;
     AttributeListImpl *pList;
// DVO: remove DEBUG dependency
// #endif
     char *eq;
     int isHwpEQ;
};

#endif
