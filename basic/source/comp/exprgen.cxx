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


#include "sbcomp.hxx"
#include "expr.hxx"



typedef struct {
        SbiToken  eTok;                 
        SbiOpcode eOp;                  
} OpTable;

static const OpTable aOpTable [] = {
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


void SbiExprNode::Gen( RecursiveMode eRecMode )
{
    sal_uInt16 nStringId;

    if( IsConstant() )
    {
        switch( GetType() )
        {
        case SbxEMPTY:
            pGen->Gen( _EMPTY );
            break;
        case SbxINTEGER:
            pGen->Gen( _CONST,  (short) nVal );
            break;
        case SbxSTRING:
            nStringId = pGen->GetParser()->aGblStrings.Add( aStrVal, true );
            pGen->Gen( _SCONST, nStringId );
            break;
        default:
            nStringId = pGen->GetParser()->aGblStrings.Add( nVal, eType );
            pGen->Gen( _NUMBER, nStringId );
            break;
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
                    {
                         bTreatFunctionAsParam = false;
                    }
                }
                if( !bTreatFunctionAsParam )
                {
                    eOp = aVar.pDef->IsGlobal() ? _FIND_G : _FIND;
                }
            }
        }
        
        else if( (pWithParent_ = GetWithParent()) != NULL )
        {
            eOp = _ELEM;            
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
            {
                eOp = _FIND_CM;
            }
            else if ( aVar.pDef->IsStatic() || (pProc && pProc->IsStatic()) )
            {
                eOp = _FIND_STATIC;
            }
        }
        for( SbiExprNode* p = this; p; p = p->aVar.pNext )
        {
            if( p == this && pWithParent_ != NULL )
            {
                pWithParent_->Gen();
            }
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
        {
            pRight->Gen();
        }
        for( const OpTable* p = aOpTable; p->eTok != NIL; p++ )
        {
            if( p->eTok == eTok )
            {
                pGen->Gen( p->eOp ); break;
            }
        }
    }
}



void SbiExprNode::GenElement( SbiOpcode eOp )
{
#ifdef DBG_UTIL
    if ((eOp < _RTL || eOp > _CALLC) && eOp != _FIND_G && eOp != _FIND_CM && eOp != _FIND_STATIC)
        pGen->GetParser()->Error( SbERR_INTERNAL_ERROR, "Opcode" );
#endif
    SbiSymDef* pDef = aVar.pDef;
    
    
    
    sal_uInt16 nId = ( eOp == _PARAM ) ? pDef->GetPos() : pDef->GetId();
    
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





void SbiExprList::Gen()
{
    if( pFirst )
    {
        pParser->aGen.Gen( _ARGC );
        
        sal_uInt16 nCount = 1;

        for( SbiExpression* pExpr = pFirst; pExpr; pExpr = pExpr->pNext,nCount++ )
        {
            pExpr->Gen();
            if( !pExpr->GetName().isEmpty() )
            {
                
                sal_uInt16 nSid = pParser->aGblStrings.Add( pExpr->GetName() );
                pParser->aGen.Gen( _ARGN, nSid );

                /* TODO: Check after Declare concept change
                
                if( pProc )
                {
                    
                    pParser->Error( SbERR_NO_NAMED_ARGS );

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
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
    
    
    pExpr->Gen( eRecMode );
    if( bByVal )
    {
        pParser->aGen.Gen( _BYVAL );
    }
    if( bBased )
    {
        sal_uInt16 uBase = pParser->nBase;
        if( pParser->IsCompatible() )
        {
            uBase |= 0x8000;        
        }
        pParser->aGen.Gen( _BASED, uBase );
        pParser->aGen.Gen( _ARGV );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
