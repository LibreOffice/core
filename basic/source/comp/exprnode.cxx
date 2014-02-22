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


#include <math.h>

#include <rtl/math.hxx>
#include "sbcomp.hxx"
#include "expr.hxx"


SbiExprNode::SbiExprNode()
{
    pLeft = NULL;
    pRight = NULL;
    pWithParent = NULL;
    pGen = NULL;
    eNodeType = SbxDUMMY;
    eType = SbxVARIANT;
    eTok = NIL;
    bError = false;
}

SbiExprNode::SbiExprNode( SbiParser* p, SbiExprNode* l, SbiToken t, SbiExprNode* r )
{
    BaseInit( p );

    pLeft     = l;
    pRight    = r;
    eTok      = t;
    nVal      = 0;
    eType     = SbxVARIANT;     
    eNodeType = SbxNODE;
}

SbiExprNode::SbiExprNode( SbiParser* p, double n, SbxDataType t )
{
    BaseInit( p );

    eType     = t;
    eNodeType = SbxNUMVAL;
    nVal      = n;
}

SbiExprNode::SbiExprNode( SbiParser* p, const OUString& rVal )
{
    BaseInit( p );

    eType     = SbxSTRING;
    eNodeType = SbxSTRVAL;
    aStrVal   = rVal;
}

SbiExprNode::SbiExprNode( SbiParser* p, const SbiSymDef& r, SbxDataType t, SbiExprList* l )
{
    BaseInit( p );

    eType     = ( t == SbxVARIANT ) ? r.GetType() : t;
    eNodeType = SbxVARVAL;
    aVar.pDef = (SbiSymDef*) &r;
    aVar.pPar = l;
    aVar.pvMorePar = NULL;
    aVar.pNext= NULL;
}


SbiExprNode::SbiExprNode( SbiParser* p, SbiExprNode* l, sal_uInt16 nId )
{
    BaseInit( p );

    pLeft      = l;
    eType      = SbxBOOL;
    eNodeType  = SbxTYPEOF;
    nTypeStrId = nId;
}


SbiExprNode::SbiExprNode( SbiParser* p, sal_uInt16 nId )
{
    BaseInit( p );

    eType     = SbxOBJECT;
    eNodeType = SbxNEW;
    nTypeStrId = nId;
}


void SbiExprNode::BaseInit( SbiParser* p )
{
    pGen = &p->aGen;
    eTok = NIL;
    pLeft       = NULL;
    pRight      = NULL;
    pWithParent = NULL;
    bError      = false;
}

SbiExprNode::~SbiExprNode()
{
    delete pLeft;
    delete pRight;
    if( IsVariable() )
    {
        delete aVar.pPar;
        delete aVar.pNext;
        SbiExprListVector* pvMorePar = aVar.pvMorePar;
        if( pvMorePar )
        {
            SbiExprListVector::iterator it;
            for( it = pvMorePar->begin() ; it != pvMorePar->end() ; ++it )
                delete *it;
            delete pvMorePar;
        }
    }
}

SbiSymDef* SbiExprNode::GetVar()
{
    if( eNodeType == SbxVARVAL )
        return aVar.pDef;
    else
        return NULL;
}

SbiSymDef* SbiExprNode::GetRealVar()
{
    SbiExprNode* p = GetRealNode();
    if( p )
        return p->GetVar();
    else
        return NULL;
}


SbiExprNode* SbiExprNode::GetRealNode()
{
    if( eNodeType == SbxVARVAL )
    {
        SbiExprNode* p = this;
        while( p->aVar.pNext )
            p = p->aVar.pNext;
        return p;
    }
    else
        return NULL;
}



bool SbiExprNode::IsIntConst()
{
    if( eNodeType == SbxNUMVAL )
    {
        if( eType >= SbxINTEGER && eType <= SbxDOUBLE )
        {
            double n;
            if( nVal >= SbxMININT && nVal <= SbxMAXINT && modf( nVal, &n ) == 0 )
            {
                nVal = (double) (short) nVal;
                eType = SbxINTEGER;
                return true;
            }
        }
    }
    return false;
}

bool SbiExprNode::IsNumber()
{
    return eNodeType == SbxNUMVAL;
}

bool SbiExprNode::IsVariable()
{
    return eNodeType == SbxVARVAL;
}

bool SbiExprNode::IsLvalue()
{
    return IsVariable();
}



short SbiExprNode::GetDepth()
{
    if( IsOperand() ) return 0;
    else
    {
        short d1 = pLeft->GetDepth();
        short d2 = pRight->GetDepth();
        return( (d1 < d2 ) ? d2 : d1 ) + 1;
    }
}








void SbiExprNode::Optimize()
{
    FoldConstants();
    CollectBits();
}



void SbiExprNode::CollectBits()
{
    if( pLeft )
    {
        pLeft->CollectBits();
        bError = bError || pLeft->bError;
    }
    if( pRight )
    {
        pRight->CollectBits();
        bError = bError || pRight->bError;
    }
}




void SbiExprNode::FoldConstants()
{
    if( IsOperand() || eTok == LIKE ) return;
    if( pLeft )
        pLeft->FoldConstants();
    if( pRight )
    {
        pRight->FoldConstants();
        if( pLeft->IsConstant() && pRight->IsConstant()
            && pLeft->eNodeType == pRight->eNodeType )
        {
            CollectBits();
            if( eTok == CAT )
                
                eType = SbxSTRING;
            if( pLeft->eType == SbxSTRING )
                
                eType = SbxSTRING;
            if( eType == SbxSTRING )
            {
                OUString rl( pLeft->GetString() );
                OUString rr( pRight->GetString() );
                delete pLeft; pLeft = NULL;
                delete pRight; pRight = NULL;
                if( eTok == PLUS || eTok == CAT )
                {
                    eTok = CAT;
                    
                    aStrVal = rl;
                    aStrVal += rr;
                    eType = SbxSTRING;
                    eNodeType = SbxSTRVAL;
                }
                else
                {
                    eType = SbxDOUBLE;
                    eNodeType = SbxNUMVAL;
                    int eRes = rr.compareTo( rl );
                    switch( eTok )
                    {
                    case EQ:
                        nVal = ( eRes == 0 ) ? SbxTRUE : SbxFALSE;
                        break;
                    case NE:
                        nVal = ( eRes != 0 ) ? SbxTRUE : SbxFALSE;
                        break;
                    case LT:
                        nVal = ( eRes < 0 ) ? SbxTRUE : SbxFALSE;
                        break;
                    case GT:
                        nVal = ( eRes > 0 ) ? SbxTRUE : SbxFALSE;
                        break;
                    case LE:
                        nVal = ( eRes <= 0 ) ? SbxTRUE : SbxFALSE;
                        break;
                    case GE:
                        nVal = ( eRes >= 0 ) ? SbxTRUE : SbxFALSE;
                        break;
                    default:
                        pGen->GetParser()->Error( SbERR_CONVERSION );
                        bError = true;
                        break;
                    }
                }
            }
            else
            {
                double nl = pLeft->nVal;
                double nr = pRight->nVal;
                long ll = 0, lr = 0;
                long llMod = 0, lrMod = 0;
                if( ( eTok >= AND && eTok <= IMP )
                   || eTok == IDIV || eTok == MOD )
                {
                    
                    bool err = false;
                    if( nl > SbxMAXLNG ) err = true, nl = SbxMAXLNG;
                    else if( nl < SbxMINLNG ) err = true, nl = SbxMINLNG;
                    if( nr > SbxMAXLNG ) err = true, nr = SbxMAXLNG;
                    else if( nr < SbxMINLNG ) err = true, nr = SbxMINLNG;
                    ll = (long) nl; lr = (long) nr;
                    llMod = (long) (nl < 0 ? nl - 0.5 : nl + 0.5);
                    lrMod = (long) (nr < 0 ? nr - 0.5 : nr + 0.5);
                    if( err )
                    {
                        pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );
                        bError = true;
                    }
                }
                bool bBothInt = ( pLeft->eType < SbxSINGLE
                                   && pRight->eType < SbxSINGLE );
                delete pLeft; pLeft = NULL;
                delete pRight; pRight = NULL;
                nVal = 0;
                eType = SbxDOUBLE;
                eNodeType = SbxNUMVAL;
                bool bCheckType = false;
                switch( eTok )
                {
                    case EXPON:
                        nVal = pow( nl, nr ); break;
                    case MUL:
                        bCheckType = true;
                        nVal = nl * nr; break;
                    case DIV:
                        if( !nr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = true;
                        } else nVal = nl / nr;
                        break;
                    case PLUS:
                        bCheckType = true;
                        nVal = nl + nr; break;
                    case MINUS:
                        bCheckType = true;
                        nVal = nl - nr; break;
                    case EQ:
                        nVal = ( nl == nr ) ? SbxTRUE : SbxFALSE;
                        eType = SbxINTEGER; break;
                    case NE:
                        nVal = ( nl != nr ) ? SbxTRUE : SbxFALSE;
                        eType = SbxINTEGER; break;
                    case LT:
                        nVal = ( nl <  nr ) ? SbxTRUE : SbxFALSE;
                        eType = SbxINTEGER; break;
                    case GT:
                        nVal = ( nl >  nr ) ? SbxTRUE : SbxFALSE;
                        eType = SbxINTEGER; break;
                    case LE:
                        nVal = ( nl <= nr ) ? SbxTRUE : SbxFALSE;
                        eType = SbxINTEGER; break;
                    case GE:
                        nVal = ( nl >= nr ) ? SbxTRUE : SbxFALSE;
                        eType = SbxINTEGER; break;
                    case IDIV:
                        if( !lr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = true;
                        } else nVal = ll / lr;
                        eType = SbxLONG; break;
                    case MOD:
                        if( !lr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = true;
                        } else nVal = llMod % lrMod;
                        eType = SbxLONG; break;
                    case AND:
                        nVal = (double) ( ll & lr ); eType = SbxLONG; break;
                    case OR:
                        nVal = (double) ( ll | lr ); eType = SbxLONG; break;
                    case XOR:
                        nVal = (double) ( ll ^ lr ); eType = SbxLONG; break;
                    case EQV:
                        nVal = (double) ( ~ll ^ lr ); eType = SbxLONG; break;
                    case IMP:
                        nVal = (double) ( ~ll | lr ); eType = SbxLONG; break;
                    default: break;
                }

                if( !::rtl::math::isFinite( nVal ) )
                    pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );

                
                if( bCheckType && bBothInt
                 && nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                {
                    
                    long n = (long) nVal;
                    nVal = n;
                    eType = ( n >= SbxMININT && n <= SbxMAXINT )
                          ? SbxINTEGER : SbxLONG;
                }
            }
        }
    }
    else if( pLeft && pLeft->IsNumber() )
    {
        nVal = pLeft->nVal;
        delete pLeft;
        pLeft = NULL;
        eType = SbxDOUBLE;
        eNodeType = SbxNUMVAL;
        switch( eTok )
        {
            case NEG:
                nVal = -nVal; break;
            case NOT: {
                
                bool err = false;
                if( nVal > SbxMAXLNG ) err = true, nVal = SbxMAXLNG;
                else if( nVal < SbxMINLNG ) err = true, nVal = SbxMINLNG;
                if( err )
                {
                    pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );
                    bError = true;
                }
                nVal = (double) ~((long) nVal);
                eType = SbxLONG;
                } break;
            default: break;
        }
    }
    if( eNodeType == SbxNUMVAL )
    {
        
        if( eType == SbxSINGLE || eType == SbxDOUBLE )
        {
            double x;
            if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG
            && !modf( nVal, &x ) )
                eType = SbxLONG;
        }
        if( eType == SbxLONG && nVal >= SbxMININT && nVal <= SbxMAXINT )
            eType = SbxINTEGER;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
