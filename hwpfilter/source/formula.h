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
