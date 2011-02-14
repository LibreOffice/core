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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <math.h>

#include <rtl/math.hxx>
#include "sbcomp.hxx"
#include "expr.hxx"

//////////////////////////////////////////////////////////////////////////

SbiExprNode::SbiExprNode( void )
{
    pLeft = NULL;
    pRight = NULL;
    eNodeType = SbxDUMMY;
}

SbiExprNode::SbiExprNode( SbiParser* p, SbiExprNode* l, SbiToken t, SbiExprNode* r )
{
    BaseInit( p );

    pLeft     = l;
    pRight    = r;
    eTok      = t;
    nVal      = 0;
    eType     = SbxVARIANT;     // Nodes sind immer Variant
    eNodeType = SbxNODE;
    bComposite= sal_True;
}

SbiExprNode::SbiExprNode( SbiParser* p, double n, SbxDataType t )
{
    BaseInit( p );

    eType     = t;
    eNodeType = SbxNUMVAL;
    nVal      = n;
}

SbiExprNode::SbiExprNode( SbiParser* p, const String& rVal )
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

    // Funktionsergebnisse sind nie starr
    bComposite= sal_Bool( aVar.pDef->GetProcDef() != NULL );
}

// #120061 TypeOf
SbiExprNode::SbiExprNode( SbiParser* p, SbiExprNode* l, sal_uInt16 nId )
{
    BaseInit( p );

    pLeft      = l;
    eType      = SbxBOOL;
    eNodeType  = SbxTYPEOF;
    nTypeStrId = nId;
}

// new <type>
SbiExprNode::SbiExprNode( SbiParser* p, sal_uInt16 nId )
{
    BaseInit( p );

    eType     = SbxOBJECT;
    eNodeType = SbxNEW;
    nTypeStrId = nId;
}

// AB: 17.12.95, Hilfsfunktion fuer Ctor fuer einheitliche Initialisierung
void SbiExprNode::BaseInit( SbiParser* p )
{
    pGen = &p->aGen;
    eTok = NIL;
    pLeft       = NULL;
    pRight      = NULL;
    pWithParent = NULL;
    bComposite  = sal_False;
    bError      = sal_False;
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

// AB: 18.12.95
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

// Diese Methode setzt den Typ um, falls er in den Integer-Bereich hineinpasst

sal_Bool SbiExprNode::IsIntConst()
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
                return sal_True;
            }
        }
    }
    return sal_False;
}

sal_Bool SbiExprNode::IsNumber()
{
    return sal_Bool( eNodeType == SbxNUMVAL );
}

sal_Bool SbiExprNode::IsString()
{
    return sal_Bool( eNodeType == SbxSTRVAL );
}

sal_Bool SbiExprNode::IsVariable()
{
    return sal_Bool( eNodeType == SbxVARVAL );
}

sal_Bool SbiExprNode::IsLvalue()
{
    return IsVariable();
}

// Ermitteln der Tiefe eines Baumes

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


// Abgleich eines Baumes:
// 1. Constant Folding
// 2. Typabgleich
// 3. Umwandlung der Operanden in Strings
// 4. Hochziehen der Composite- und Error-Bits

void SbiExprNode::Optimize()
{
    FoldConstants();
    CollectBits();
}

// Hochziehen der Composite- und Fehlerbits

void SbiExprNode::CollectBits()
{
    if( pLeft )
    {
        pLeft->CollectBits();
        bError |= pLeft->bError;
        bComposite |= pLeft->bComposite;
    }
    if( pRight )
    {
        pRight->CollectBits();
        bError |= pRight->bError;
        bComposite |= pRight->bComposite;
    }
}

// Kann ein Zweig umgeformt werden, wird sal_True zurueckgeliefert. In diesem
// Fall ist das Ergebnis im linken Zweig.

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
                // CAT verbindet auch zwei Zahlen miteinander!
                eType = SbxSTRING;
            if( pLeft->eType == SbxSTRING )
                // Kein Type Mismatch!
                eType = SbxSTRING;
            if( eType == SbxSTRING )
            {
                String rl( pLeft->GetString() );
                String rr( pRight->GetString() );
                delete pLeft; pLeft = NULL;
                delete pRight; pRight = NULL;
                bComposite = sal_False;
                if( eTok == PLUS || eTok == CAT )
                {
                    eTok = CAT;
                    // Verkettung:
                    aStrVal = rl;
                    aStrVal += rr;
                    eType = SbxSTRING;
                    eNodeType = SbxSTRVAL;
                }
                else
                {
                    eType = SbxDOUBLE;
                    eNodeType = SbxNUMVAL;
                    StringCompare eRes = rr.CompareTo( rl );
                    switch( eTok )
                    {
                        case EQ:
                            nVal = ( eRes == COMPARE_EQUAL ) ? SbxTRUE : SbxFALSE;
                            break;
                        case NE:
                            nVal = ( eRes != COMPARE_EQUAL ) ? SbxTRUE : SbxFALSE;
                            break;
                        case LT:
                            nVal = ( eRes == COMPARE_LESS ) ? SbxTRUE : SbxFALSE;
                            break;
                        case GT:
                            nVal = ( eRes == COMPARE_GREATER ) ? SbxTRUE : SbxFALSE;
                            break;
                        case LE:
                            nVal = ( eRes != COMPARE_GREATER ) ? SbxTRUE : SbxFALSE;
                            break;
                        case GE:
                            nVal = ( eRes != COMPARE_LESS ) ? SbxTRUE : SbxFALSE;
                            break;
                        default:
                            pGen->GetParser()->Error( SbERR_CONVERSION );
                            bError = sal_True;
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
                    // Integer-Operationen
                    sal_Bool err = sal_False;
                    if( nl > SbxMAXLNG ) err = sal_True, nl = SbxMAXLNG;
                    else
                    if( nl < SbxMINLNG ) err = sal_True, nl = SbxMINLNG;
                    if( nr > SbxMAXLNG ) err = sal_True, nr = SbxMAXLNG;
                    else
                    if( nr < SbxMINLNG ) err = sal_True, nr = SbxMINLNG;
                    ll = (long) nl; lr = (long) nr;
                    llMod = (long) (nl < 0 ? nl - 0.5 : nl + 0.5);
                    lrMod = (long) (nr < 0 ? nr - 0.5 : nr + 0.5);
                    if( err )
                    {
                        pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );
                        bError = sal_True;
                    }
                }
                sal_Bool bBothInt = sal_Bool( pLeft->eType < SbxSINGLE
                                   && pRight->eType < SbxSINGLE );
                delete pLeft; pLeft = NULL;
                delete pRight; pRight = NULL;
                nVal = 0;
                eType = SbxDOUBLE;
                eNodeType = SbxNUMVAL;
                bComposite = sal_False;
                sal_Bool bCheckType = sal_False;
                switch( eTok )
                {
                    case EXPON:
                        nVal = pow( nl, nr ); break;
                    case MUL:
                        bCheckType = sal_True;
                        nVal = nl * nr; break;
                    case DIV:
                        if( !nr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = sal_True;
                        } else nVal = nl / nr;
                        break;
                    case PLUS:
                        bCheckType = sal_True;
                        nVal = nl + nr; break;
                    case MINUS:
                        bCheckType = sal_True;
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
                            bError = sal_True;
                        } else nVal = ll / lr;
                        eType = SbxLONG; break;
                    case MOD:
                        if( !lr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = sal_True;
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

                // Den Datentyp wiederherstellen, um Rundungsfehler
                // zu killen
                if( bCheckType && bBothInt
                 && nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                {
                    // NK-Stellen weg
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
        bComposite = sal_False;
        switch( eTok )
        {
            case NEG:
                nVal = -nVal; break;
            case NOT: {
                // Integer-Operation!
                sal_Bool err = sal_False;
                if( nVal > SbxMAXLNG ) err = sal_True, nVal = SbxMAXLNG;
                else
                if( nVal < SbxMINLNG ) err = sal_True, nVal = SbxMINLNG;
                if( err )
                {
                    pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );
                    bError = sal_True;
                }
                nVal = (double) ~((long) nVal);
                eType = SbxLONG;
                } break;
            default: break;
        }
    }
    if( eNodeType == SbxNUMVAL )
    {
        // Evtl auf INTEGER falten (wg. besserem Opcode)?
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


