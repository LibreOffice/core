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


#include "codegen.hxx"
#include "expr.hxx"
#include "parser.hxx"

// Transform table for token operators and opcodes

typedef struct {
        SbiToken  eTok;                 // Token
        SbiOpcode eOp;                  // Opcode
} OpTable;

static const OpTable aOpTable [] = {
    { EXPON,EXP_ },
    { MUL,  MUL_ },
    { DIV,  DIV_ },
    { IDIV, IDIV_ },
    { MOD,  MOD_ },
    { PLUS, PLUS_ },
    { MINUS,MINUS_ },
    { EQ,   EQ_ },
    { NE,   NE_ },
    { LE,   LE_ },
    { GE,   GE_ },
    { LT,   LT_ },
    { GT,   GT_ },
    { AND,  AND_ },
    { OR,   OR_ },
    { XOR,  XOR_ },
    { EQV,  EQV_ },
    { IMP,  IMP_ },
    { NOT,  NOT_ },
    { NEG,  NEG_ },
    { CAT,  CAT_ },
    { LIKE, LIKE_ },
    { IS,   IS_ },
    { NIL,  NOP_ }};

// Output of an element
void SbiExprNode::Gen( SbiCodeGen& rGen, RecursiveMode eRecMode )
{
    sal_uInt16 nStringId;

    if( IsConstant() )
    {
        switch( GetType() )
        {
        case SbxEMPTY:
            rGen.Gen( EMPTY_ );
            break;
        case SbxINTEGER:
            rGen.Gen( CONST_,  (short) nVal );
            break;
        case SbxSTRING:
            nStringId = rGen.GetParser()->aGblStrings.Add( aStrVal );
            rGen.Gen( SCONST_, nStringId );
            break;
        default:
            nStringId = rGen.GetParser()->aGblStrings.Add( nVal, eType );
            rGen.Gen( NUMBER_, nStringId );
            break;
        }
    }
    else if( IsOperand() )
    {
        SbiExprNode* pWithParent_ = nullptr;
        SbiOpcode eOp;
        if( aVar.pDef->GetScope() == SbPARAM )
        {
            eOp = PARAM_;
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
                    {
                         bTreatFunctionAsParam = false;
                    }
                }
                if( !bTreatFunctionAsParam )
                {
                    eOp = aVar.pDef->IsGlobal() ? FIND_G_ : FIND_;
                }
            }
        }
        // special treatment for WITH
        else if( (pWithParent_ = GetWithParent()) != nullptr )
        {
            eOp = ELEM_;            // .-Term in WITH
        }
        else
        {
            eOp = ( aVar.pDef->GetScope() == SbRTL ) ? RTL_ :
                (aVar.pDef->IsGlobal() ? FIND_G_ : FIND_);
        }

        if( eOp == FIND_ )
        {

            SbiProcDef* pProc = aVar.pDef->GetProcDef();
            if ( rGen.GetParser()->bClassModule )
            {
                eOp = FIND_CM_;
            }
            else if ( aVar.pDef->IsStatic() || (pProc && pProc->IsStatic()) )
            {
                eOp = FIND_STATIC_;
            }
        }
        for( SbiExprNode* p = this; p; p = p->aVar.pNext )
        {
            if( p == this && pWithParent_ != nullptr )
            {
                pWithParent_->Gen(rGen);
            }
            p->GenElement( rGen, eOp );
            eOp = ELEM_;
        }
    }
    else if( IsTypeOf() )
    {
        pLeft->Gen(rGen);
        rGen.Gen( TESTCLASS_, nTypeStrId );
    }
    else if( IsNew() )
    {
        rGen.Gen( CREATE_, 0, nTypeStrId );
    }
    else
    {
        pLeft->Gen(rGen);
        if( pRight )
        {
            pRight->Gen(rGen);
        }
        for( const OpTable* p = aOpTable; p->eTok != NIL; p++ )
        {
            if( p->eTok == eTok )
            {
                rGen.Gen( p->eOp ); break;
            }
        }
    }
}

// Output of an operand element

void SbiExprNode::GenElement( SbiCodeGen& rGen, SbiOpcode eOp )
{
#ifdef DBG_UTIL
    if ((eOp < RTL_ || eOp > CALLC_) && eOp != FIND_G_ && eOp != FIND_CM_ && eOp != FIND_STATIC_)
        rGen.GetParser()->Error( ERRCODE_BASIC_INTERNAL_ERROR, "Opcode" );
#endif
    SbiSymDef* pDef = aVar.pDef;
    // The ID is either the position or the String-ID
    // If the bit Bit 0x8000 is set, the variable have
    // a parameter list.
    sal_uInt16 nId = ( eOp == PARAM_ ) ? pDef->GetPos() : pDef->GetId();
    // Build a parameter list
    if( aVar.pPar && aVar.pPar->GetSize() )
    {
        nId |= 0x8000;
        aVar.pPar->Gen(rGen);
    }

    rGen.Gen( eOp, nId, sal::static_int_cast< sal_uInt16 >( GetType() ) );

    if( aVar.pvMorePar )
    {
        for( auto& pExprList: *aVar.pvMorePar )
        {
            pExprList->Gen(rGen);
            rGen.Gen( ARRAYACCESS_ );
        }
    }
}

// Create an Argv-Table
// The first element remain available for return value etc.
// See as well SbiProcDef::SbiProcDef() in symtbl.cxx

void SbiExprList::Gen(SbiCodeGen& rGen)
{
    if( !aData.empty() )
    {
        rGen.Gen( ARGC_ );
        // Type adjustment at DECLARE
        sal_uInt16 nCount = 1;

        for( auto& pExpr: aData )
        {
            pExpr->Gen();
            if( !pExpr->GetName().isEmpty() )
            {
                // named arg
                sal_uInt16 nSid = rGen.GetParser()->aGblStrings.Add( pExpr->GetName() );
                rGen.Gen( ARGN_, nSid );

                /* TODO: Check after Declare concept change
                // From 1996-01-10: Type adjustment at named -> search suitable parameter
                if( pProc )
                {
                    // For the present: trigger an error
                    pParser->Error( ERRCODE_BASIC_NO_NAMED_ARGS );

                    // Later, if Named Args at DECLARE is possible
                    //for( sal_uInt16 i = 1 ; i < nParAnz ; i++ )
                    //{
                    //  SbiSymDef* pDef = pPool->Get( i );
                    //  const String& rName = pDef->GetName();
                    //  if( rName.Len() )
                    //  {
                    //      if( pExpr->GetName().ICompare( rName )
                    //          == COMPARE_EQUAL )
                    //      {
                    //          pParser->aGen.Gen( ARGTYP_, pDef->GetType() );
                    //          break;
                    //      }
                    //  }
                    //}
                }
                */
            }
            else
            {
                rGen.Gen( ARGV_ );
            }
            nCount++;
        }
    }
}

void SbiExpression::Gen( RecursiveMode eRecMode )
{
    // special treatment for WITH
    // If pExpr == .-term in With, approximately Gen for Basis-Object
    pExpr->Gen( pParser->aGen, eRecMode );
    if( bByVal )
    {
        pParser->aGen.Gen( BYVAL_ );
    }
    if( bBased )
    {
        sal_uInt16 uBase = pParser->nBase;
        if( pParser->IsCompatible() )
        {
            uBase |= 0x8000;        // #109275 Flag compatibility
        }
        pParser->aGen.Gen( BASED_, uBase );
        pParser->aGen.Gen( ARGV_ );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
