/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "formula.h"

#include "mzstring.h"
#include "nodes.h"
#include "mapping.h"
#include "hwpeq.h"
#include <iostream>
#include <list>

extern std::list<Node*> nodelist;

#ifndef DEBUG

#include "hcode.h"

#define ascii(x)  OUString::createFromAscii(x)
#define rstartEl(x,y) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->startElement(x,y); } while(false)
#define rendEl(x) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->endElement(x); } while(false)
#define rchars(x) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(ascii(x)); } while(false)
#define runistr(x) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(OUString(x)); } while(false)
#define reucstr(x,y) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(OUString(x,y, RTL_TEXTENCODING_EUC_KR)); } while(false)
#define padd(x,y,z)  pList->addAttribute(x,y,z)
#else
static int indent = 0;
#define inds indent++; for(int i = 0 ; i < indent ; i++) fprintf(stderr," ")
#define inde for(int i = 0 ; i < indent ; i++) fprintf(stderr," "); indent--
#define indo indent--;
#endif

extern Node *mainParse(const char *);


void Formula::makeMathML(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:math xmlns:math=\"http:
#else
     padd(ascii("xmlns:math"), ascii("CDATA"), ascii("http:
     rstartEl(ascii("math:math"), rList);
     pList->clear();
     rstartEl(ascii("math:semantics"), rList);
#endif
     if( tmp->child )
          makeLines( tmp->child );

#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:semantics/>\n");
     indo;
     inde;
     fprintf(stderr,"</math:math>\n");
#else
     rendEl(ascii("math:semantics"));
     rendEl(ascii("math:math"));
#endif
}

void Formula::makeLines(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;

     if( tmp->child ){
          if( tmp->child->id == ID_LINES )
                makeLines( tmp->child );
          else
                makeLine( tmp->child );
     }
     if( tmp->next )
          makeLine( tmp->next );
}

void Formula::makeLine(Node *res)
{
  if( !res ) return;
#ifdef DEBUG
     inds; fprintf(stderr,"<math:mrow>\n");
#else
     rstartEl(ascii("math:mrow"), rList);
#endif
     if( res->child )
         makeExprList( res->child );
#ifdef DEBUG
     inde; fprintf(stderr,"</math:mrow>\n");
#else
     rendEl(ascii("math:mrow"));
#endif
}

void Formula::makeExprList(Node *res)
{
   if( !res ) return;
    Node *tmp = res->child;
    if( !tmp ) return ;

    if( tmp->id == ID_EXPRLIST ){
         Node *next = tmp->next;
         makeExprList( tmp ) ;
         if( next )
              makeExpr( next );
    }
    else
         makeExpr( tmp );
}

void Formula::makeExpr(Node *res)
{
  if( !res ) return;
    Node *tmp = res->child;
    if( !tmp ) return;
    switch( tmp->id ) {
        case ID_PRIMARYEXPR:
             if( tmp->next ){
#ifdef DEBUG
                 inds;
                 fprintf(stderr,"<math:mrow>\n");
#else
                 rstartEl(ascii("math:mrow"), rList);
#endif
             }

             makePrimary(tmp);

             if( tmp->next ){
#ifdef DEBUG
                 inde; fprintf(stderr,"</math:mrow>\n");
#else
                 rendEl(ascii("math:mrow"));
#endif
             }
            break;
         case ID_SUBEXPR:
         case ID_SUPEXPR:
         case ID_SUBSUPEXPR:
             makeSubSup(tmp);
             break;
         case ID_FRACTIONEXPR:
         case ID_OVER:
             makeFraction(tmp);
             break;
         case ID_DECORATIONEXPR:
             makeDecoration(tmp);
             break;
         case ID_SQRTEXPR:
         case ID_ROOTEXPR:
             makeRoot(tmp);
             break;
         case ID_ARROWEXPR:
             makeArrow(tmp);
             break;
         case ID_ACCENTEXPR:
             makeAccent(tmp);
             break;
         case ID_PARENTH:
         case ID_ABS:
             makeParenth(tmp);
             break;
         case ID_FENCE:
             makeFence(tmp);
             break;
         case ID_BLOCK:
             makeBlock(tmp);
         case ID_BEGIN:
             makeBegin(tmp);
         case ID_END:
             makeEnd(tmp);
             break;
    }
}

void Formula::makeIdentifier(Node *res)
{
     Node *tmp = res;
  if( !tmp ) return;
  if( !tmp->value ) return;
     switch( tmp->id ){
     case ID_CHARACTER :
#ifdef DEBUG
          inds;
          fprintf(stderr,"<math:mi>%s</math:mi>\n",tmp->value);
          indo;
#else
          rstartEl(ascii("math:mi"), rList);
          rchars(tmp->value);
          rendEl(ascii("math:mi"));
#endif
          break;
     case ID_STRING :
          {
#ifdef DEBUG
#else
                rstartEl(ascii("math:mi"), rList);
                reucstr(tmp->value, strlen(tmp->value));
                rendEl(ascii("math:mi"));
#endif
          }
          break;
     case ID_IDENTIFIER :
#ifdef DEBUG
          inds;
          fprintf(stderr,"<math:mi>%s</math:mi>\n",
                  getMathMLEntity(tmp->value).c_str());
          indo;
#else
          rstartEl(ascii("math:mi"), rList);
          runistr(getMathMLEntity(tmp->value).c_str());
          rendEl(ascii("math:mi"));
#endif
          break;
     case ID_NUMBER :
#ifdef DEBUG
          inds;
          fprintf(stderr,"<math:mn>%s</math:mn>\n",tmp->value);
          indo;
#else
          rstartEl(ascii("math:mn"), rList);
          rchars(tmp->value);
          rendEl(ascii("math:mn"));
#endif
          break;
     case ID_OPERATOR :
     case ID_DELIMETER :
        {
#ifdef DEBUG
          inds; fprintf(stderr,"<math:mo>%s</math:mo>\n",tmp->value); indo;
#else
          rstartEl(ascii("math:mo"), rList);
          runistr(getMathMLEntity(tmp->value).c_str());
          rendEl(ascii("math:mo"));
#endif
          break;
        }
     }
}
void Formula::makePrimary(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return ;
     if( tmp->child ){
          if( tmp->child->id == ID_PRIMARYEXPR ){
                makePrimary(tmp->child);
          }
          else{
                makeIdentifier(tmp->child);
          }
     }
     if( tmp->next ){
          makeIdentifier(tmp->next);
     }
}

void Formula::makeSubSup(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;

#ifdef DEBUG
     inds;
     if( res->id == ID_SUBEXPR )
          fprintf(stderr,"<math:msub>\n");
     else if( res->id == ID_SUPEXPR )
          fprintf(stderr,"<math:msup>\n");
     else
          fprintf(stderr,"<math:msubsup>\n");
#else
     if( res->id == ID_SUBEXPR )
          rstartEl(ascii("math:msub"), rList);
     else if( res->id == ID_SUPEXPR )
          rstartEl(ascii("math:msup"), rList);
     else
          rstartEl(ascii("math:msubsup"), rList);
#endif

     tmp = tmp->child;
     if( res->id == ID_SUBSUPEXPR ) {
          makeExpr(tmp);
          makeBlock(tmp->next);
          makeBlock(tmp->next->next);
     }
     else{
          makeExpr(tmp);
          makeExpr(tmp->next);
     }

#ifdef DEBUG
     inde;
     if( res->id == ID_SUBEXPR )
          fprintf(stderr,"</math:msub>\n");
     else if( res->id == ID_SUPEXPR )
          fprintf(stderr,"</math:msup>\n");
     else
          fprintf(stderr,"</math:msubsup>\n");
#else
     if( res->id == ID_SUBEXPR )
          rendEl(ascii("math:msub"));
     else if( res->id == ID_SUPEXPR )
          rendEl(ascii("math:msup"));
     else
          rendEl(ascii("math:msubsup"));
#endif
}

void Formula::makeFraction(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;

#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mfrac>\n");
#else
     rstartEl(ascii("math:mfrac"), rList);
#endif

     tmp = tmp->child;
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mrow>\n");
#else
     rstartEl(ascii("math:mrow"), rList);
#endif

     if( res->id == ID_FRACTIONEXPR )
          makeBlock(tmp);
     else
          makeExprList(tmp);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mrow>\n");
     inds;
     fprintf(stderr,"<math:mrow>\n");
#else
     rendEl(ascii("math:mrow"));
     rstartEl(ascii("math:mrow"), rList);
#endif

     if( res->id == ID_FRACTIONEXPR )
          makeBlock(tmp->next);
     else
          makeExprList(tmp->next);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mrow>\n");
     inde;
     fprintf(stderr,"</math:mfrac>\n");
#else
     rendEl(ascii("math:mrow"));
     rendEl(ascii("math:mfrac"));
#endif
}

void Formula::makeDecoration(Node *res)
{
     int isover = 1;
     Node *tmp = res->child;
     if( !tmp ) return;
     if( !strncmp(tmp->value,"under", 5) )
          isover = 0;
#ifdef DEBUG
     inds;
     if( isover )
          fprintf(stderr,"<math:mover>\n");
     else
          fprintf(stderr,"<math:munder>\n");
#else
     /* accent�� ���� true�̰�, ����, false���� �𸣰ڴ�. */
     if( isover ){
          padd(ascii("accent"),ascii("CDATA"),ascii("true"));
          rstartEl(ascii("math:mover"), rList);
     }
     else{
          padd(ascii("accentunder"),ascii("CDATA"),ascii("true"));
          rstartEl(ascii("math:munder"), rList);
     }
     pList->clear();
#endif

     makeBlock(tmp->next);

#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mo>%s</math:mo>\n",
             getMathMLEntity(tmp->value).c_str());
     indo;
#else
     rstartEl(ascii("math:mo"), rList);
     runistr(getMathMLEntity(tmp->value).c_str());
     rendEl(ascii("math:mo"));
#endif

#ifdef DEBUG
     inde;
     if( isover )
          fprintf(stderr,"</math:mover>\n");
     else
          fprintf(stderr,"</math:munder>\n");
#else
     if( isover )
          rendEl(ascii("math:mover"));
     else
          rendEl(ascii("math:munder"));
#endif
}

void Formula::makeRoot(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;
#ifdef DEBUG
     inds;
     if( tmp->id == ID_SQRTEXPR )
          fprintf(stderr,"<math:msqrt>\n");
     else
          fprintf(stderr,"<math:mroot>\n");
#else
     if( tmp->id == ID_SQRTEXPR )
          rstartEl(ascii("math:msqrt"), rList);
     else
          rstartEl(ascii("math:mroot"), rList);
#endif

     if( tmp->id == ID_SQRTEXPR ){
          makeBlock(tmp->child);
     }
     else{
          makeBracket(tmp->child);
          makeBlock(tmp->child->next);
     }

#ifdef DEBUG
     inde;
     if( tmp->id == ID_SQRTEXPR )
          fprintf(stderr,"</math:msqrt>\n");
     else
          fprintf(stderr,"</math:mroot>\n");
#else
     if( tmp->id == ID_SQRTEXPR )
          rendEl(ascii("math:msqrt"));
     else
          rendEl(ascii("math:mroot"));
#endif
}

void Formula::makeArrow(Node * /*res*/)
{
}
void Formula::makeAccent(Node *res)
{
     makeDecoration( res );
}
void Formula::makeParenth(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mrow>\n");
     inds;
     if( tmp->id == ID_PARENTH ){
          fprintf(stderr,"<math:mo>(</math:mo>\n");
     }
     else
          fprintf(stderr,"<math:mo>|</math:mo>\n");
     indo; inds;
     fprintf(stderr,"<math:mrow>\n");
#else
     rstartEl(ascii("math:mrow"), rList);
     rstartEl(ascii("math:mo"), rList);
     if( tmp->id == ID_PARENTH )
          rchars("(");
     else
          rchars("|");
     rendEl(ascii("math:mo"));
     rstartEl(ascii("math:mrow"), rList);
#endif

     if( tmp->child )
          makeExprList(tmp->child);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mrow>\n");
     inds;
     if( tmp->id == ID_PARENTH )
          fprintf(stderr,"<math:mo>)</math:mo>\n");
     else
          fprintf(stderr,"<math:mo>|</math:mo>\n");
     indo;
     inde;
     fprintf(stderr,"</math:mrow>\n");
#else
     rendEl(ascii("math:mrow"));
     rstartEl(ascii("math:mo"), rList);
     if( tmp->id == ID_PARENTH )
          rchars(")");
     else
          rchars("|");
     rendEl(ascii("math:mo"));
     rendEl(ascii("math:mrow"));
#endif
}

void Formula::makeFence(Node *res)
{
     Node *tmp = res->child;
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mfenced open=\"%s\" close=\"%s\">\n",
                getMathMLEntity(tmp->value).c_str(),
                getMathMLEntity(tmp->next->next->value).c_str());
#else
     padd(ascii("open"), ascii("CDATA"),
             OUString(getMathMLEntity(tmp->value).c_str()) );
     padd(ascii("close"), ascii("CDATA"),
             OUString(getMathMLEntity(tmp->next->next->value).c_str()) );
     rstartEl(ascii("math:mfenced"), rList);
     pList->clear();
#endif

     makeExprList(tmp->next);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mfenced>\n");
#else
     rendEl(ascii("math:mfenced"));
#endif
}

void Formula::makeBracket(Node *res)
{
     makeBlock(res);
}

void Formula::makeBlock(Node *res)
{
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mrow>\n");
#else
     rstartEl(ascii("math:mrow"), rList);
#endif

     if( res->child )
          makeExprList(res->child);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mrow>\n");
#else
     rendEl(ascii("math:mrow"));
#endif
}


void Formula::makeBegin(Node * /*res*/)
{
}


void Formula::makeEnd(Node * /*res*/)
{
}

int Formula::parse()
{
     Node *res = 0L;
     if( !eq ) return 0;
     if( isHwpEQ ){
          MzString a;
         
          eq2latex(a,eq);

          int idx=a.find(sal::static_int_cast<char>(0xff));
          while(idx){
                
                a.replace(idx,0x20);
                if((idx = a.find(sal::static_int_cast<char>(0xff),idx+1)) < 0)
                     break;
          }

          char *buf = (char *)malloc(a.length()+1);
          bool bStart = false;
          int i, j;
          for( i = 0, j=0 ; i < a.length() ; i++){ 
                if( bStart ){
                     buf[j++] = a[i];
                }
                else{
                     if( a[i] != 32 && a[i] != 10 && a[i] != 13){
                          bStart = true;
                          buf[j++] = a[i];
                     }
                }
          }
          buf[j] = 0;
          for( i = j-1 ; i >= 0 ; i++ ){
                if( buf[i] == 32 || buf[i] == 10 || buf[i] == 13 ){
                     buf[i] = 0;
                }
                else
                     break;
          }
         
          if( strlen(buf) > 0 )
                res = mainParse( a.c_str() );
          else
                res = 0L;
          free(buf);
     }
     else{
          res = mainParse( eq );
     }

     if( res ){
          makeMathML( res );
     }
     Node *tmpNode;
     int count = nodelist.size();
     for( int i = 0 ; i < count ; i++ ){
	  tmpNode = nodelist.front();
	  nodelist.pop_front();
          delete tmpNode;
     }

     return 0;
}

void Formula::trim()
{
     int len = strlen(eq);
     char *buf = (char *)malloc(len+1);
     bool bStart = false;
     int i, j;
     for( i = 0, j=0 ; i < len ; i++){ 
          if( bStart ){
                buf[j++] = eq[i];
          }
          else{
                if( eq[i] != 32 && eq[i] != 10 && eq[i] != 13){
                     bStart = true;
                     buf[j++] = eq[i];
                }
          }
     }
     buf[j] = 0;
     for( i = j-1 ; i >= 0 ; i++ ){
          if( buf[i] == 32 || buf[i] == 10 || buf[i] == 13 ){
                buf[i] = 0;
          }
          else
                break;
     }
     if( strlen(buf) > 0 )
          strcpy(eq, buf);
     else
          eq = 0L;
     free(buf);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
