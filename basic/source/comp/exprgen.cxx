/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sbcomp.hxx"
#include "expr.hxx"

// Transform table for token operators and opcodes

typedef struct {
        SbiToken  eTok;                 // Token
        SbiOpcode eOp;                  // Opcode
} OpTable;

static OpTable aOpTable [] = {
    { EXPON,_EXP },
    { MUL,  _MUL },
    { DIV,  _DIV },
    { IDIV, _IDIV },
    { MOD,  _MOD },
    { PLUS, _PLUS },
    { MINUS,_MINUS },
    { EQ,   _EQ },
    { NE,   _NE },
    { LE,   _LE },
    { GE,   _GE },
    { LT,   _LT },
    { GT,   _GT },
    { AND,  _AND },
    { OR,   _OR },
    { XOR,  _XOR },
    { EQV,  _EQV },
    { IMP,  _IMP },
    { NOT,  _NOT },
    { NEG,  _NEG },
    { CAT,  _CAT },
    { LIKE, _LIKE },
    { IS,   _IS },
    { NIL,  _NOP }};

// Output of an element
void SbiExprNode::Gen( RecursiveMode eRecMode )
{
    if( IsConstant() )
    {
        switch( GetType() )
        {
            case SbxEMPTY:   pGen->Gen( _EMPTY ); break;
            case SbxINTEGER: pGen->Gen( _CONST,  (short) nVal ); break;
            case SbxSTRING:
            {
                sal_uInt16 nStringId = pGen->GetParser()->aGblStrings.Add( aStrVal, sal_True );
                pGen->Gen( _SCONST, nStringId ); break;
            }
            default:
            {
                sal_uInt16 nStringId = pGen->GetParser()->aGblStrings.Add( nVal, eType );
                pGen->Gen( _NUMBER, nStringId );
            }
        }
    }
    else if( IsOperand() )
    {
        SbiExprNode* pWithParent_ = NULL;
        SbiOpcode eOp;
        if( aVar.pDef->GetScope() == SbPARAM )
        {
            eOp = _PARAM;
            if( 0 == aVar.pDef->GetPos() )
            {
                bool bTreatFunctionAsParam = true;
                if( eRecMode == FORCE_CALL )
                {
                    bTreatFunctionAsParam = false;
                }
                else if( eRecMode == UNDEFINED )
                {
                    if( aVar.pPar && aVar.pPar->IsBracket() )
                         bTreatFunctionAsParam = false;
                }
                if( !bTreatFunctionAsParam )
                    eOp = aVar.pDef->IsGlobal() ? _FIND_G : _FIND;
            }
        }
        // From 1995-12-17, special treatment for WITH
        else if( (pWithParent_ = GetWithParent()) != NULL )
        {
            eOp = _ELEM;            // .-Term in in WITH
        }
        else
        {
            eOp = ( aVar.pDef->GetScope() == SbRTL ) ? _RTL :
                (aVar.pDef->IsGlobal() ? _FIND_G : _FIND);
        }

        if( eOp == _FIND )
        {

            SbiProcDef* pProc = aVar.pDef->GetProcDef();
            if ( pGen->GetParser()->bClassModule )
                eOp = _FIND_CM;
            else if ( aVar.pDef->IsStatic() || (pProc && pProc->IsStatic()) )
            {
                eOp = _FIND_STATIC;
            }
        }
        for( SbiExprNode* p = this; p; p = p->aVar.pNext )
        {
            if( p == this && pWithParent_ != NULL )
                pWithParent_->Gen();
            p->GenElement( eOp );
            eOp = _ELEM;
        }
    }
    else if( IsTypeOf() )
    {
        pLeft->Gen();
        pGen->Gen( _TESTCLASS, nTypeStrId );
    }
    else if( IsNew() )
    {
        pGen->Gen( _CREATE, 0, nTypeStrId );
    }
    else
    {
        pLeft->Gen();
        if( pRight )
            pRight->Gen();
        for( OpTable* p = aOpTable; p->eTok != NIL; p++ )
        {
            if( p->eTok == eTok )
            {
                pGen->Gen( p->eOp ); break;
            }
        }
    }
}

// Output of an operand element

void SbiExprNode::GenElement( SbiOpcode eOp )
{
#ifdef DBG_UTIL
    if( (eOp < _RTL || eOp > _CALLC) && eOp != _FIND_G && eOp != _FIND_CM )
        pGen->GetParser()->Error( SbERR_INTERNAL_ERROR, "Opcode" );
#endif
    SbiSymDef* pDef = aVar.pDef;
    // The ID is either the position or the String-ID
    // If the bit Bit 0x8000 is set, the variable have
    // a parameter list.
    sal_uInt16 nId = ( eOp == _PARAM ) ? pDef->GetPos() : pDef->GetId();
    // Build a parameter list
    if( aVar.pPar && aVar.pPar->GetSize() )
    {
        nId |= 0x8000;
        aVar.pPar->Gen();
    }

    pGen->Gen( eOp, nId, sal::static_int_cast< sal_uInt16 >( GetType() ) );

    if( aVar.pvMorePar )
    {
        SbiExprListVector* pvMorePar = aVar.pvMorePar;
        SbiExprListVector::iterator it;
        for( it = pvMorePar->begin() ; it != pvMorePar->end() ; ++it )
        {
            SbiExprList* pExprList = *it;
            pExprList->Gen();
            pGen->Gen( _ARRAYACCESS );
        }
    }
}

// Create an Argv-Table
// The first element remain available for return value etc.
// See as well SbiProcDef::SbiProcDef() in symtbl.cxx

void SbiExprList::Gen()
{
    if( pFirst )
    {
        pParser->aGen.Gen( _ARGC );
        // From 1996-01-10: Type adjustment at DECLARE
        sal_uInt16 nCount = 1 /*, nParAnz = 0*/;
//      SbiSymPool* pPool = NULL;
        for( SbiExpression* pExpr = pFirst; pExpr; pExpr = pExpr->pNext,nCount++ )
        {
            pExpr->Gen();
            if( pExpr->GetName().Len() )
            {
                // named arg
                sal_uInt16 nSid = pParser->aGblStrings.Add( pExpr->GetName() );
                pParser->aGen.Gen( _ARGN, nSid );

                /* TODO: Check after Declare concept change
                // From 1996-01-10: Type adjustment at named -> search suitable parameter
                if( pProc )
                {
                    // For the present: trigger an error
                    pParser->Error( SbERR_NO_NAMED_ARGS );

                    // Later, if Named Args at DECLARE is posible
                    //for( sal_uInt16 i = 1 ; i < nParAnz ; i++ )
                    //{
                    //  SbiSymDef* pDef = pPool->Get( i );
                    //  const String& rName = pDef->GetName();
                    //  if( rName.Len() )
                    //  {
                    //      if( pExpr->GetName().ICompare( rName )
                    //          == COMPARE_EQUAL )
                    //      {
                    //          pParser->aGen.Gen( _ARGTYP, pDef->GetType() );
                    //          break;
                    //      }
                    //  }
                    //}
                }
                */
            }
            else
            {
                pParser->aGen.Gen( _ARGV );
            }
        }
    }
}

void SbiExpression::Gen( RecursiveMode eRecMode )
{
    // From 1995-12-17, special treatment for WITH
    // If pExpr == .-term in With, approximately Gen for Basis-Object
    pExpr->Gen( eRecMode );
    if( bByVal )
        pParser->aGen.Gen( _BYVAL );
    if( bBased )
    {
        sal_uInt16 uBase = pParser->nBase;
        if( pParser->IsCompatible() )
            uBase |= 0x8000;        // #109275 Flag compatiblity
        pParser->aGen.Gen( _BASED, uBase );
        pParser->aGen.Gen( _ARGV );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
