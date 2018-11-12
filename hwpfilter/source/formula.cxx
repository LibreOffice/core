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

#include "formula.h"
#include "grammar.hxx"

#include "mzstring.h"
#include "nodes.h"
#include "mapping.h"
#include "hwpeq.h"
#include <iostream>

#ifndef DEBUG

#include "hcode.h"

#define rstartEl(x,y) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->startElement(x,y); } while(false)
#define rendEl(x) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->endElement(x); } while(false)
#define rchars(x) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(x); } while(false)
#define runistr(x) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(x); } while(false)
#define reucstr(x,y) do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(OUString(x,y, RTL_TEXTENCODING_EUC_KR)); } while(false)
#define padd(x,y,z)  mxList->addAttribute(x,y,z)
#else
static int indent = 0;
#define inds indent++; for(int i = 0 ; i < indent ; i++) fprintf(stderr," ")
#define inde for(int i = 0 ; i < indent ; i++) fprintf(stderr," "); indent--
#define indo indent--;
#endif

void Formula::makeMathML(Node *res)
{
     Node *tmp = res;
     if( !tmp ) return;
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:math xmlns:math=\"http://www.w3.org/1998/Math/MathML\">\n");
#else
     padd("xmlns:math", "CDATA", "http://www.w3.org/1998/Math/MathML");
     rstartEl("math:math", mxList.get());
     mxList->clear();
     rstartEl("math:semantics", mxList.get());
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
     rendEl("math:semantics");
     rendEl("math:math");
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
    rstartEl("math:mrow", mxList.get());
#endif
    if( res->child )
         makeExprList( res->child );
#ifdef DEBUG
    inde; fprintf(stderr,"</math:mrow>\n");
#else
    rendEl("math:mrow");
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
                 rstartEl("math:mrow", mxList.get());
#endif
             }

             makePrimary(tmp);

             if( tmp->next ){
#ifdef DEBUG
                 inde; fprintf(stderr,"</math:mrow>\n");
#else
                 rendEl("math:mrow");
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
             break;
         case ID_BEGIN:
         case ID_END:
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
          rstartEl("math:mi", mxList.get());
          rchars(OUString::createFromAscii(tmp->value));
          rendEl("math:mi");
#endif
          break;
     case ID_STRING :
          {
#ifdef DEBUG
#else
                rstartEl("math:mi", mxList.get());
                reucstr(tmp->value, strlen(tmp->value));
                rendEl("math:mi");
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
          rstartEl("math:mi", mxList.get());
          runistr(reinterpret_cast<sal_Unicode const *>(getMathMLEntity(tmp->value).c_str()));
          rendEl("math:mi");
#endif
          break;
     case ID_NUMBER :
#ifdef DEBUG
          inds;
          fprintf(stderr,"<math:mn>%s</math:mn>\n",tmp->value);
          indo;
#else
          rstartEl("math:mn", mxList.get());
          rchars(OUString::createFromAscii(tmp->value));
          rendEl("math:mn");
#endif
          break;
     case ID_OPERATOR :
     case ID_DELIMITER :
        {
#ifdef DEBUG
          inds; fprintf(stderr,"<math:mo>%s</math:mo>\n",tmp->value); indo;
#else
          rstartEl("math:mo", mxList.get());
          runistr(reinterpret_cast<sal_Unicode const *>(getMathMLEntity(tmp->value).c_str()));
          rendEl("math:mo");
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
          rstartEl("math:msub", mxList.get());
     else if( res->id == ID_SUPEXPR )
          rstartEl("math:msup", mxList.get());
     else
          rstartEl("math:msubsup", mxList.get());
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
          rendEl("math:msub");
     else if( res->id == ID_SUPEXPR )
          rendEl("math:msup");
     else
          rendEl("math:msubsup");
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
     rstartEl("math:mfrac", mxList.get());
#endif

     tmp = tmp->child;
#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mrow>\n");
#else
     rstartEl("math:mrow", mxList.get());
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
     rendEl("math:mrow");
     rstartEl("math:mrow", mxList.get());
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
     rendEl("math:mrow");
     rendEl("math:mfrac");
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
     /* FIXME: no idea when 'accent' is true or false. */
     if( isover ){
          padd("accent","CDATA","true");
          rstartEl("math:mover", mxList.get());
     }
     else{
          padd("accentunder","CDATA","true");
          rstartEl("math:munder", mxList.get());
     }
     mxList->clear();
#endif

     makeBlock(tmp->next);

#ifdef DEBUG
     inds;
     fprintf(stderr,"<math:mo>%s</math:mo>\n",
             getMathMLEntity(tmp->value).c_str());
     indo;
#else
     rstartEl("math:mo", mxList.get());
     runistr(reinterpret_cast<sal_Unicode const *>(getMathMLEntity(tmp->value).c_str()));
     rendEl("math:mo");
#endif

#ifdef DEBUG
     inde;
     if( isover )
          fprintf(stderr,"</math:mover>\n");
     else
          fprintf(stderr,"</math:munder>\n");
#else
     if( isover )
          rendEl("math:mover");
     else
          rendEl("math:munder");
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
          rstartEl("math:msqrt", mxList.get());
     else
          rstartEl("math:mroot", mxList.get());
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
          rendEl("math:msqrt");
     else
          rendEl("math:mroot");
#endif
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
     rstartEl("math:mrow", mxList.get());
     rstartEl("math:mo", mxList.get());
     if( tmp->id == ID_PARENTH )
          rchars("(");
     else
          rchars("|");
     rendEl("math:mo");
     rstartEl("math:mrow", mxList.get());
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
     rendEl("math:mrow");
     rstartEl("math:mo", mxList.get());
     if( tmp->id == ID_PARENTH )
          rchars(")");
     else
          rchars("|");
     rendEl("math:mo");
     rendEl("math:mrow");
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
     padd("open", "CDATA",
             OUString(reinterpret_cast<sal_Unicode const *>(getMathMLEntity(tmp->value).c_str())));
     padd("close", "CDATA",
             OUString(reinterpret_cast<sal_Unicode const *>(getMathMLEntity(tmp->next->next->value).c_str())));
     rstartEl("math:mfenced", mxList.get());
     mxList->clear();
#endif

     makeExprList(tmp->next);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mfenced>\n");
#else
     rendEl("math:mfenced");
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
     rstartEl("math:mrow", mxList.get());
#endif

     if( res->child )
          makeExprList(res->child);

#ifdef DEBUG
     inde;
     fprintf(stderr,"</math:mrow>\n");
#else
     rendEl("math:mrow");
#endif
}

void Formula::parse()
{
     Node *res = nullptr;
     if( !eq ) return;

     MzString a;
     // fprintf(stderr,"\n\n[BEFORE]\n[%s]\n",eq);
     eq2latex(a,eq);

     int idx=a.find(sal::static_int_cast<char>(0xff));
     while(idx){
           //printf("idx = [%d]\n",idx);
           a.replace(idx,0x20);
           if((idx = a.find(sal::static_int_cast<char>(0xff),idx+1)) < 0)
                break;
     }

     char *buf = static_cast<char *>(malloc(a.length()+1));
     bool bStart = false;
     int i, j;
     for( i = 0, j=0 ; i < a.length() ; i++){ // rtrim and ltrim 32 10 13
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
     // fprintf(stderr,"\n\n[RESULT]\n[%s]\n",a.c_str());
     if( buf[0] != '\0' )
           res = mainParse( a.c_str() );
     else
           res = nullptr;
     free(buf);

     if( res ){
          makeMathML( res );
     }
     nodelist.clear();
}

void Formula::trim()
{
     int len = strlen(eq);
     char *buf = static_cast<char *>(malloc(len+1));
     bool bStart = false;
     int i, j;
     for( i = 0, j=0 ; i < len ; i++){ // rtrim and ltrim 32 10 13
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
     if( buf[0] != '\0' )
          strcpy(eq, buf);
     else
          eq = nullptr;
     free(buf);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
