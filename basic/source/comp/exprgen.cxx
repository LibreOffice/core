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


#include <codegen.hxx>
#include <expr.hxx>
#include <parser.hxx>

// Transform table for token operators and opcodes

struct OpTable {
        SbiToken  eTok;                 // Token
        SbiOpcode eOp;                  // Opcode
};

static const OpTable aOpTable [] = {
    { EXPON,SbiOpcode::EXP_ },
    { MUL,  SbiOpcode::MUL_ },
    { DIV,  SbiOpcode::DIV_ },
    { IDIV, SbiOpcode::IDIV_ },
    { MOD,  SbiOpcode::MOD_ },
    { PLUS, SbiOpcode::PLUS_ },
    { MINUS,SbiOpcode::MINUS_ },
    { EQ,   SbiOpcode::EQ_ },
    { NE,   SbiOpcode::NE_ },
    { LE,   SbiOpcode::LE_ },
    { GE,   SbiOpcode::GE_ },
    { LT,   SbiOpcode::LT_ },
    { GT,   SbiOpcode::GT_ },
    { AND,  SbiOpcode::AND_ },
    { OR,   SbiOpcode::OR_ },
    { XOR,  SbiOpcode::XOR_ },
    { EQV,  SbiOpcode::EQV_ },
    { IMP,  SbiOpcode::IMP_ },
    { NOT,  SbiOpcode::NOT_ },
    { NEG,  SbiOpcode::NEG_ },
    { CAT,  SbiOpcode::CAT_ },
    { LIKE, SbiOpcode::LIKE_ },
    { IS,   SbiOpcode::IS_ },
    { NIL,  SbiOpcode::NOP_ }};

// Output of an element
void SbiExprNode::Gen( SbiCodeGen& rGen, RecursiveMode eRecMode )
{
    sal_uInt16 nStringId;

    if( IsConstant() )
    {
        switch( GetType() )
        {
        case SbxEMPTY:
            rGen.Gen( SbiOpcode::EMPTY_ );
            break;
        case SbxINTEGER:
            rGen.Gen( SbiOpcode::CONST_,  static_cast<short>(nVal) );
            break;
        case SbxSTRING:
            nStringId = rGen.GetParser()->aGblStrings.Add( aStrVal );
            rGen.Gen( SbiOpcode::SCONST_, nStringId );
            break;
        default:
            nStringId = rGen.GetParser()->aGblStrings.Add( nVal, eType );
            rGen.Gen( SbiOpcode::NUMBER_, nStringId );
            break;
        }
    }
    else if( IsOperand() )
    {
        SbiExprNode* pWithParent_ = nullptr;
        SbiOpcode eOp;
        if( aVar.pDef->GetScope() == SbPARAM )
        {
            eOp = SbiOpcode::PARAM_;
            if( aVar.pDef->GetPos() == 0 )
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
                    eOp = aVar.pDef->IsGlobal() ? SbiOpcode::FIND_G_ : SbiOpcode::FIND_;
                }
            }
        }
        // special treatment for WITH
        else if( (pWithParent_ = pWithParent) != nullptr )
        {
            eOp = SbiOpcode::ELEM_;            // .-Term in WITH
        }
        else
        {
            eOp = ( aVar.pDef->GetScope() == SbRTL ) ? SbiOpcode::RTL_ :
                (aVar.pDef->IsGlobal() ? SbiOpcode::FIND_G_ : SbiOpcode::FIND_);
        }

        if( eOp == SbiOpcode::FIND_ )
        {

            SbiProcDef* pProc = aVar.pDef->GetProcDef();
            if ( rGen.GetParser()->bClassModule )
            {
                eOp = SbiOpcode::FIND_CM_;
            }
            else if ( aVar.pDef->IsStatic() || (pProc && pProc->IsStatic()) )
            {
                eOp = SbiOpcode::FIND_STATIC_;
            }
        }
        for( SbiExprNode* p = this; p; p = p->aVar.pNext )
        {
            if( p == this && pWithParent_ != nullptr )
            {
                pWithParent_->Gen(rGen);
            }
            p->GenElement( rGen, eOp );
            eOp = SbiOpcode::ELEM_;
        }
    }
    else if( eNodeType == SbxTYPEOF )
    {
        pLeft->Gen(rGen);
        rGen.Gen( SbiOpcode::TESTCLASS_, nTypeStrId );
    }
    else if( eNodeType == SbxNEW )
    {
        rGen.Gen( SbiOpcode::CREATE_, 0, nTypeStrId );
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
    if ((eOp < SbiOpcode::RTL_ || eOp > SbiOpcode::CALLC_) && eOp != SbiOpcode::FIND_G_ && eOp != SbiOpcode::FIND_CM_ && eOp != SbiOpcode::FIND_STATIC_)
        rGen.GetParser()->Error( ERRCODE_BASIC_INTERNAL_ERROR, "Opcode" );
#endif
    SbiSymDef* pDef = aVar.pDef;
    // The ID is either the position or the String-ID
    // If the bit Bit 0x8000 is set, the variable have
    // a parameter list.
    sal_uInt16 nId = ( eOp == SbiOpcode::PARAM_ ) ? pDef->GetPos() : pDef->GetId();
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
            rGen.Gen( SbiOpcode::ARRAYACCESS_ );
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
        rGen.Gen( SbiOpcode::ARGC_ );
        // Type adjustment at DECLARE

        for( auto& pExpr: aData )
        {
            pExpr->Gen();
            if( !pExpr->GetName().isEmpty() )
            {
                // named arg
                sal_uInt16 nSid = rGen.GetParser()->aGblStrings.Add( pExpr->GetName() );
                rGen.Gen( SbiOpcode::ARGN_, nSid );

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
                rGen.Gen( SbiOpcode::ARGV_ );
            }
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
        pParser->aGen.Gen( SbiOpcode::BYVAL_ );
    }
    if( bBased )
    {
        sal_uInt16 uBase = pParser->nBase;
        if( pParser->IsCompatible() )
        {
            uBase |= 0x8000;        // #109275 Flag compatibility
        }
        pParser->aGen.Gen( SbiOpcode::BASED_, uBase );
        pParser->aGen.Gen( SbiOpcode::ARGV_ );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
