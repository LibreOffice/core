/*************************************************************************
 *
 *  $RCSfile: exprnode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2000-10-10 13:02:15 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _NTSDK      // wg. HUGE_VAL MH
#define HUGE_VAL    HUGE
#include <math.h>

#include "sbcomp.hxx"
#pragma hdrstop
#include "expr.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCEXPR, SBCOMP_CODE  )

//////////////////////////////////////////////////////////////////////////

SbiExprNode::SbiExprNode( SbiParser* p, SbiExprNode* l, SbiToken t, SbiExprNode* r )
{
    BaseInit( p );

    pLeft     = l;
    pRight    = r;
    eTok      = t;
    nVal      = 0;
    eType     = SbxVARIANT;     // Nodes sind immer Variant
    eNodeType = SbxNODE;
    bComposite= TRUE;
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
    nStringId = p->aGblStrings.Add( rVal, TRUE );
}

SbiExprNode::SbiExprNode( SbiParser* p, const SbiSymDef& r, SbxDataType t, SbiExprList* l )
{
    BaseInit( p );

    eType     = ( t == SbxVARIANT ) ? r.GetType() : t;
    eNodeType = SbxVARVAL;
    aVar.pDef = (SbiSymDef*) &r;
    aVar.pPar = l;
    aVar.pNext= NULL;

    // Funktionsergebnisse sind nie starr
    bComposite= BOOL( aVar.pDef->GetProcDef() != NULL );
}

// AB: 17.12.95, Hilfsfunktion fuer Ctor fuer einheitliche Initialisierung
void SbiExprNode::BaseInit( SbiParser* p )
{
    pGen = &p->aGen;
    eTok = NIL;
    pLeft       = NULL;
    pRight      = NULL;
    pWithParent = NULL;
    bComposite  = FALSE;
    bError      = FALSE;
}

SbiExprNode::~SbiExprNode()
{
    delete pLeft;
    delete pRight;
    if( IsVariable() )
    {
        delete aVar.pPar;
        delete aVar.pNext;
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

BOOL SbiExprNode::IsOperand()
{
    return BOOL( eNodeType != SbxNODE );
}

BOOL SbiExprNode::IsConstant()
{
    return BOOL( eNodeType == SbxSTRVAL || eNodeType == SbxNUMVAL );
}

// Diese Methode setzt den Typ um, falls er in den Integer-Bereich hineinpasst

BOOL SbiExprNode::IsIntConst()
{
    if( eNodeType == SbxNUMVAL )
    {
        if( eType >= SbxINTEGER && eType <= SbxDOUBLE )
        {
#if defined(MAC) && !defined(__powerc)
            long double n;
#else
            double n;
#endif
            if( nVal >= SbxMININT && nVal <= SbxMAXINT && modf( nVal, &n ) == 0 )
            {
                nVal = (double) (short) nVal;
                eType = SbxINTEGER;
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL SbiExprNode::IsNumber()
{
    return BOOL( eNodeType == SbxNUMVAL );
}

BOOL SbiExprNode::IsString()
{
    return BOOL( eNodeType == SbxSTRVAL );
}

BOOL SbiExprNode::IsVariable()
{
    return BOOL( eNodeType == SbxVARVAL );
}

BOOL SbiExprNode::IsLvalue()
{
    return IsVariable();
}

// Ermitteln der Tiefe eines Baumes

short SbiExprNode::GetDepth()
{
    if( IsOperand() ) return 0;
    else
    {
        SbiExprNode* p = (SbiExprNode*) this;
        short d1 = pLeft->GetDepth();
        short d2 = pRight->GetDepth();
        return( (d1 < d2 ) ? d2 : d1 ) + 1;
    }
}

const String& SbiExprNode::GetString()
{
    USHORT n = ( eType == SbxSTRING ) ? nStringId : 0;
    return pGen->GetParser()->aGblStrings.Find( n );
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

// Kann ein Zweig umgeformt werden, wird TRUE zurueckgeliefert. In diesem
// Fall ist das Ergebnis im linken Zweig.

void SbiExprNode::FoldConstants()
{
    if( IsOperand() ) return;
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
                eType = SbxDOUBLE;
                eNodeType = SbxNUMVAL;
                bComposite = FALSE;
                StringCompare eRes = rr.CompareTo( rl );
                //StringCompare eRes = rl.Compare( rr );
                String s;
                switch( eTok )
                {
                    case PLUS:
                    case CAT:
                        eTok = CAT;
                        // Verkettung:
                        s = rl;
                        s += rr;
                        nStringId = pGen->GetParser()->aGblStrings.Add( s, TRUE );
                        eType = SbxSTRING;
                        eNodeType = SbxSTRVAL;
                        break;
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
                        bError = TRUE;
                }
            }
            else
            {
                double nl = pLeft->nVal;
                double nr = pRight->nVal;
                long ll, lr;
                if( ( eTok >= AND && eTok <= EQV )
                   || eTok == IDIV || eTok == MOD )
                {
                    // Integer-Operationen
                    BOOL err = FALSE;
                    if( nl > SbxMAXLNG ) err = TRUE, nl = SbxMAXLNG;
                    else
                    if( nl < SbxMINLNG ) err = TRUE, nl = SbxMINLNG;
                    if( nr > SbxMAXLNG ) err = TRUE, nr = SbxMAXLNG;
                    else
                    if( nr < SbxMINLNG ) err = TRUE, nr = SbxMINLNG;
                    ll = (long) nl; lr = (long) nr;
                    if( err )
                    {
                        pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );
                        bError = TRUE;
                    }
                }
                BOOL bBothInt = BOOL( pLeft->eType < SbxSINGLE
                                   && pRight->eType < SbxSINGLE );
                delete pLeft; pLeft = NULL;
                delete pRight; pRight = NULL;
                nVal = 0;
                eType = SbxDOUBLE;
                eNodeType = SbxNUMVAL;
                bComposite = FALSE;
                BOOL bCheckType = FALSE;
                switch( eTok )
                {
                    case EXPON:
                        nVal = pow( nl, nr ); break;
                    case MUL:
                        bCheckType = TRUE;
                        nVal = nl * nr; break;
                    case DIV:
                        if( !nr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = TRUE;
                        } else nVal = nl / nr;
                        break;
                    case PLUS:
                        bCheckType = TRUE;
                        nVal = nl + nr; break;
                    case MINUS:
                        bCheckType = TRUE;
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
                            bError = TRUE;
                        } else nVal = ll / lr;
                        eType = SbxLONG; break;
                    case MOD:
                        if( !lr )
                        {
                            pGen->GetParser()->Error( SbERR_ZERODIV ); nVal = HUGE_VAL;
                            bError = TRUE;
                        } else nVal = ll % lr;
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
                }
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
    else if( pLeft->IsNumber() )
    {
        nVal = pLeft->nVal;
        delete pLeft;
        pLeft = NULL;
        eType = SbxDOUBLE;
        eNodeType = SbxNUMVAL;
        bComposite = FALSE;
        switch( eTok )
        {
            case NEG:
                nVal = -nVal; break;
            case NOT: {
                // Integer-Operation!
                BOOL err = FALSE;
                if( nVal > SbxMAXLNG ) err = TRUE, nVal = SbxMAXLNG;
                else
                if( nVal < SbxMINLNG ) err = TRUE, nVal = SbxMINLNG;
                if( err )
                {
                    pGen->GetParser()->Error( SbERR_MATH_OVERFLOW );
                    bError = TRUE;
                }
                nVal = (double) ~((long) nVal);
                eType = SbxLONG;
                } break;
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


