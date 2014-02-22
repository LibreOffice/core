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
#include <basic/sbx.hxx>
#include "expr.hxx"

/***************************************************************************
|*
|*      SbiExpression
|*
***************************************************************************/

SbiExpression::SbiExpression( SbiParser* p, SbiExprType t,
    SbiExprMode eMode, const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    pParser = p;
    bBased = bError = bByVal = bBracket = false;
    nParenLevel = 0;
    eCurExpr = t;
    m_eMode = eMode;
    pNext = NULL;
    pExpr = (t != SbSTDEXPR ) ? Term( pKeywordSymbolInfo ) : Boolean();
    if( t != SbSYMBOL )
    {
        pExpr->Optimize();
    }
    if( t == SbLVALUE && !pExpr->IsLvalue() )
    {
        p->Error( SbERR_LVALUE_EXPECTED );
    }
    if( t == SbOPERAND && !IsVariable() )
    {
        p->Error( SbERR_VAR_EXPECTED );
    }
}

SbiExpression::SbiExpression( SbiParser* p, double n, SbxDataType t )
{
    pParser = p;
    bBased = bError = bByVal = bBracket = false;
    nParenLevel = 0;
    eCurExpr = SbOPERAND;
    m_eMode = EXPRMODE_STANDARD;
    pNext = NULL;
    pExpr = new SbiExprNode( pParser, n, t );
    pExpr->Optimize();
}

SbiExpression::SbiExpression( SbiParser* p, const SbiSymDef& r, SbiExprList* pPar )
{
    pParser = p;
    bBased = bError = bByVal = bBracket = false;
    nParenLevel = 0;
    eCurExpr = SbOPERAND;
    m_eMode = EXPRMODE_STANDARD;
    pNext = NULL;
    pExpr = new SbiExprNode( pParser, r, SbxVARIANT, pPar );
}

SbiExpression::~SbiExpression()
{
    delete pExpr;
}










static bool DoParametersFollow( SbiParser* p, SbiExprType eCurExpr, SbiToken eTok )
{
    if( eTok == LPAREN )
    {
        return true;
    }
    
    if( !p->WhiteSpace() || eCurExpr != SbSYMBOL )
    {
        return false;
    }
    if ( eTok == NUMBER || eTok == MINUS || eTok == FIXSTRING ||
         eTok == SYMBOL || eTok == COMMA  || eTok == DOT || eTok == NOT || eTok == BYVAL )
    {
        return true;
    }
    else 
    {
        SbiTokenizer tokens( *(SbiTokenizer*)p );
        
        tokens.Next();
        if ( tokens.Peek() == ASSIGN )
        {
            return true;
        }
    }
    return false;
}



static SbiSymDef* AddSym ( SbiToken eTok, SbiSymPool& rPool, SbiExprType eCurExpr,
                           const OUString& rName, SbxDataType eType, SbiParameters* pPar )
{
    SbiSymDef* pDef;
    
    bool bHasType = ( eTok == EQ || eTok == DOT );
    if( ( !bHasType && eCurExpr == SbSYMBOL ) || pPar )
    {
        
        
        
        SbiSymPool* pPool = &rPool;
        if( pPool->GetScope() != SbPUBLIC )
        {
            pPool = &rPool.GetParser()->aPublics;
        }
        SbiProcDef* pProc = pPool->AddProc( rName );

        
        if( eCurExpr == SbSTDEXPR )
        {
            bHasType = true;
        }
        pDef = pProc;
        pDef->SetType( bHasType ? eType : SbxEMPTY );
        if( pPar )
        {
            
            sal_Int32 n = 1;
            for( short i = 0; i < pPar->GetSize(); i++ )
            {
                n += 1;
                OUStringBuffer aPar("PAR");
                aPar.append(n);
                pProc->GetParams().AddSym( aPar.makeStringAndClear() );
            }
        }
    }
    else
    {
        
        pDef = rPool.AddSym( rName );
        pDef->SetType( eType );
    }
    return pDef;
}



SbiExprNode* SbiExpression::Term( const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    if( pParser->Peek() == DOT )
    {
        SbiExprNode* pWithVar = pParser->GetWithVar();
        
        SbiSymDef* pDef = pWithVar ? pWithVar->GetRealVar() : NULL;
        SbiExprNode* pNd = NULL;
        if( !pDef )
        {
            pParser->Next();
        }
        else
        {
            pNd = ObjTerm( *pDef );
            if( pNd )
            {
                pNd->SetWithParent( pWithVar );
            }
        }
        if( !pNd )
        {
            pParser->Error( SbERR_UNEXPECTED, DOT );
            pNd = new SbiExprNode( pParser, 1.0, SbxDOUBLE );
        }
        return pNd;
    }

    SbiToken eTok = (pKeywordSymbolInfo == NULL) ? pParser->Next() : pKeywordSymbolInfo->m_eTok;
    
    pParser->LockColumn();
    OUString aSym( (pKeywordSymbolInfo == NULL) ? pParser->GetSym() : pKeywordSymbolInfo->m_aKeywordSymbol );
    SbxDataType eType = (pKeywordSymbolInfo == NULL) ? pParser->GetType() : pKeywordSymbolInfo->m_eSbxDataType;
    SbiParameters* pPar = NULL;
    SbiExprListVector* pvMoreParLcl = NULL;
    
    SbiToken eNextTok = pParser->Peek();
    
    
    
    if( eNextTok == ASSIGN )
    {
        pParser->UnlockColumn();
        return new SbiExprNode( pParser, aSym );
    }
    
    if( pParser->IsKwd( eTok ) )
    {
        if( pParser->IsCompatible() && eTok == INPUT )
        {
            eTok = SYMBOL;
        }
        else
        {
            pParser->Error( SbERR_SYNTAX );
            bError = true;
        }
    }

    if( DoParametersFollow( pParser, eCurExpr, eTok = eNextTok ) )
    {
        bool bStandaloneExpression = (m_eMode == EXPRMODE_STANDALONE);
        pPar = new SbiParameters( pParser, bStandaloneExpression );
        bError = bError || !pPar->IsValid();
        if( !bError )
            bBracket = pPar->IsBracket();
        eTok = pParser->Peek();

        
        while( eTok == LPAREN )
        {
            if( pvMoreParLcl == NULL )
            {
                pvMoreParLcl = new SbiExprListVector();
            }
            SbiParameters* pAddPar = new SbiParameters( pParser );
            pvMoreParLcl->push_back( pAddPar );
            bError = bError || !pAddPar->IsValid();
            eTok = pParser->Peek();
        }
    }
    
    
    
    bool bObj = ( ( eTok == DOT || eTok == EXCLAM )
                    && !pParser->WhiteSpace() );
    if( bObj )
    {
        bBracket = false;   
        if( eType == SbxVARIANT )
        {
            eType = SbxOBJECT;
        }
        else
        {
            
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = true;
        }
    }
    
    SbiSymDef* pDef = pParser->pPool->Find( aSym );
    if( !pDef )
    {
        
        
        
        pDef = pParser->CheckRTLForSym( aSym, eType );

        
        SbModule& rMod = pParser->aGen.GetModule();
        SbxArray* pModMethods = rMod.GetMethods();
        if( pModMethods->Find( aSym, SbxCLASS_DONTCARE ) )
        {
            pDef = NULL;
        }
    }
    if( !pDef )
    {
        if( bObj )
        {
            eType = SbxOBJECT;
        }
        pDef = AddSym( eTok, *pParser->pPool, eCurExpr, aSym, eType, pPar );
        
        
        
        if ( !bObj && pParser->pProc && pParser->pProc->IsStatic() )
        {
            pDef->SetStatic();
        }
    }
    else
    {

        SbiConstDef* pConst = pDef->GetConstDef();
        if( pConst )
        {
            delete pPar;
            delete pvMoreParLcl;
            if( pConst->GetType() == SbxSTRING )
            {
                return new SbiExprNode( pParser, pConst->GetString() );
            }
            else
            {
                return new SbiExprNode( pParser, pConst->GetValue(), pConst->GetType() );
            }
        }

        
        if( pDef->GetDims() )
        {
            if( pPar && pPar->GetSize() && pPar->GetSize() != pDef->GetDims() )
            {
                pParser->Error( SbERR_WRONG_DIMS );
            }
        }
        if( pDef->IsDefinedAs() )
        {
            SbxDataType eDefType = pDef->GetType();
            
            if( eType >= SbxINTEGER && eType <= SbxSTRING && eType != eDefType )
            {
                
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = true;
            }
            else if ( eType == SbxVARIANT )
            {
                
                
                
                eType = eDefType;
            }
        }
        
        
        
        if( eType != SbxVARIANT &&          
            eType != pDef->GetType() &&
            !pDef->GetProcDef() )
        {
            
            
            
            if( eType == SbxOBJECT && pDef->GetType() == SbxVARIANT )
            {
                pDef->SetType( SbxOBJECT );
            }
            else
            {
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = true;
            }
        }
    }
    SbiExprNode* pNd = new SbiExprNode( pParser, *pDef, eType );
    if( !pPar )
    {
        pPar = new SbiParameters( pParser,false,false );
    }
    pNd->aVar.pPar = pPar;
    pNd->aVar.pvMorePar = pvMoreParLcl;
    if( bObj )
    {
        
        if( pDef->GetType() == SbxVARIANT )
            pDef->SetType( SbxOBJECT );
        
        
        if( pDef->GetType() != SbxOBJECT && pDef->GetType() != SbxVARIANT )
        {
            
            if ( !pParser->IsVBASupportOn() )
            {
                pParser->Error( SbERR_BAD_DECLARATION, aSym );
                bError = true;
            }
        }
        if( !bError )
        {
            pNd->aVar.pNext = ObjTerm( *pDef );
        }
    }

    pParser->UnlockColumn();
    return pNd;
}




SbiExprNode* SbiExpression::ObjTerm( SbiSymDef& rObj )
{
    pParser->Next();
    SbiToken eTok = pParser->Next();
    if( eTok != SYMBOL && !pParser->IsKwd( eTok ) && !pParser->IsExtra( eTok ) )
    {
        
        
        if( eTok != MOD && eTok != NOT && eTok != AND && eTok != OR &&
            eTok != XOR && eTok != EQV && eTok != IMP && eTok != IS )
        {
            pParser->Error( SbERR_VAR_EXPECTED );
            bError = true;
        }
    }

    if( bError )
    {
        return NULL;
    }
    OUString aSym( pParser->GetSym() );
    SbxDataType eType = pParser->GetType();
    SbiParameters* pPar = NULL;
    SbiExprListVector* pvMoreParLcl = NULL;
    eTok = pParser->Peek();

    if( DoParametersFollow( pParser, eCurExpr, eTok ) )
    {
        bool bStandaloneExpression = false;
        pPar = new SbiParameters( pParser, bStandaloneExpression );
        bError = bError || !pPar->IsValid();
        eTok = pParser->Peek();

        
        while( eTok == LPAREN )
        {
            if( pvMoreParLcl == NULL )
            {
                pvMoreParLcl = new SbiExprListVector();
            }
            SbiParameters* pAddPar = new SbiParameters( pParser );
            pvMoreParLcl->push_back( pAddPar );
            bError = bError || !pPar->IsValid();
            eTok = pParser->Peek();
        }
    }
    bool bObj = ( ( eTok == DOT || eTok == EXCLAM ) && !pParser->WhiteSpace() );
    if( bObj )
    {
        if( eType == SbxVARIANT )
        {
            eType = SbxOBJECT;
        }
        else
        {
            
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = true;
        }
    }

    
    SbiSymPool& rPool = rObj.GetPool();
    rPool.SetScope( SbPUBLIC );
    SbiSymDef* pDef = rPool.Find( aSym );
    if( !pDef )
    {
        pDef = AddSym( eTok, rPool, eCurExpr, aSym, eType, pPar );
        pDef->SetType( eType );
    }

    SbiExprNode* pNd = new SbiExprNode( pParser, *pDef, eType );
    pNd->aVar.pPar = pPar;
    pNd->aVar.pvMorePar = pvMoreParLcl;
    if( bObj )
    {
        if( pDef->GetType() == SbxVARIANT )
        {
            pDef->SetType( SbxOBJECT );
        }
        if( pDef->GetType() != SbxOBJECT )
        {
            pParser->Error( SbERR_BAD_DECLARATION, aSym );
            bError = true;
        }
        if( !bError )
        {
            pNd->aVar.pNext = ObjTerm( *pDef );
            pNd->eType = eType;
        }
    }
    return pNd;
}









SbiExprNode* SbiExpression::Operand( bool bUsedForTypeOf )
{
    SbiExprNode *pRes;
    SbiToken eTok;

    
    switch( eTok = pParser->Peek() )
    {
    case SYMBOL:
        pRes = Term();
        
        if( !bUsedForTypeOf && pParser->IsVBASupportOn() && pParser->Peek() == IS )
        {
            eTok = pParser->Next();
            pRes = new SbiExprNode( pParser, pRes, eTok, Like() );
        }
        break;
    case DOT:   
        pRes = Term(); break;
    case NUMBER:
        pParser->Next();
        pRes = new SbiExprNode( pParser, pParser->GetDbl(), pParser->GetType() );
        break;
    case FIXSTRING:
        pParser->Next();
        pRes = new SbiExprNode( pParser, pParser->GetSym() ); break;
    case LPAREN:
        pParser->Next();
        if( nParenLevel == 0 && m_eMode == EXPRMODE_LPAREN_PENDING && pParser->Peek() == RPAREN )
        {
            m_eMode = EXPRMODE_EMPTY_PAREN;
            pRes = new SbiExprNode();   
            pParser->Next();
            break;
        }
        nParenLevel++;
        pRes = Boolean();
        if( pParser->Peek() != RPAREN )
        {
            
            if( nParenLevel == 1 && m_eMode == EXPRMODE_LPAREN_PENDING )
            {
                m_eMode = EXPRMODE_LPAREN_NOT_NEEDED;
            }
            else
            {
                pParser->Error( SbERR_BAD_BRACKETS );
            }
        }
        else
        {
            pParser->Next();
            if( nParenLevel == 1 && m_eMode == EXPRMODE_LPAREN_PENDING )
            {
                SbiToken eTokAfterRParen = pParser->Peek();
                if( eTokAfterRParen == EQ || eTokAfterRParen == LPAREN || eTokAfterRParen == DOT )
                {
                    m_eMode = EXPRMODE_ARRAY_OR_OBJECT;
                }
                else
                {
                    m_eMode = EXPRMODE_STANDARD;
                }
            }
        }
        nParenLevel--;
        break;
    default:
        
        if( pParser->IsKwd( eTok ) )
        {
            pRes = Term();
        }
        else
        {
            pParser->Next();
            pRes = new SbiExprNode( pParser, 1.0, SbxDOUBLE );
            pParser->Error( SbERR_UNEXPECTED, eTok );
        }
        break;
    }
    return pRes;
}

SbiExprNode* SbiExpression::Unary()
{
    SbiExprNode* pNd;
    SbiToken eTok = pParser->Peek();
    switch( eTok )
    {
        case MINUS:
            eTok = NEG;
            pParser->Next();
            pNd = new SbiExprNode( pParser, Unary(), eTok, NULL );
            break;
        case NOT:
            if( pParser->IsVBASupportOn() )
            {
                pNd = Operand();
            }
            else
            {
                pParser->Next();
                pNd = new SbiExprNode( pParser, Unary(), eTok, NULL );
            }
            break;
        case PLUS:
            pParser->Next();
            pNd = Unary();
            break;
        case TYPEOF:
        {
            pParser->Next();
            bool bUsedForTypeOf = true;
            SbiExprNode* pObjNode = Operand( bUsedForTypeOf );
            pParser->TestToken( IS );
            OUString aDummy;
            SbiSymDef* pTypeDef = new SbiSymDef( aDummy );
            pParser->TypeDecl( *pTypeDef, true );
            pNd = new SbiExprNode( pParser, pObjNode, pTypeDef->GetTypeId() );
            break;
        }
        case NEW:
        {
            pParser->Next();
            OUString aStr;
            SbiSymDef* pTypeDef = new SbiSymDef( aStr );
            pParser->TypeDecl( *pTypeDef, true );
            pNd = new SbiExprNode( pParser, pTypeDef->GetTypeId() );
            break;
        }
        default:
            pNd = Operand();
    }
    return pNd;
}

SbiExprNode* SbiExpression::Exp()
{
    SbiExprNode* pNd = Unary();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == EXPON )
        {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Unary() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::MulDiv()
{
    SbiExprNode* pNd = Exp();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != MUL && eTok != DIV )
            {
                break;
            }
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Exp() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::IntDiv()
{
    SbiExprNode* pNd = MulDiv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == IDIV )
        {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, MulDiv() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Mod()
{
    SbiExprNode* pNd = IntDiv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == MOD )
        {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, IntDiv() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::AddSub()
{
    SbiExprNode* pNd = Mod();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != PLUS && eTok != MINUS )
            {
                break;
            }
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Mod() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Cat()
{
    SbiExprNode* pNd = AddSub();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( eTok != CAT )
            {
                break;
            }
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, AddSub() );
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Comp()
{
    SbiExprNode* pNd = Cat();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        short nCount = 0;
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( m_eMode == EXPRMODE_ARRAY_OR_OBJECT )
            {
                break;
            }
            if( eTok != EQ && eTok != NE && eTok != LT &&
                eTok != GT && eTok != LE && eTok != GE )
            {
                break;
            }
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Cat() );
            nCount++;
        }
    }
    return pNd;
}


SbiExprNode* SbiExpression::VBA_Not()
{
    SbiExprNode* pNd = NULL;

    SbiToken eTok = pParser->Peek();
    if( eTok == NOT )
    {
        pParser->Next();
        pNd = new SbiExprNode( pParser, VBA_Not(), eTok, NULL );
    }
    else
    {
        pNd = Comp();
    }
    return pNd;
}

SbiExprNode* SbiExpression::Like()
{
    SbiExprNode* pNd = pParser->IsVBASupportOn() ? VBA_Not() : Comp();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        short nCount = 0;
        while( pParser->Peek() == LIKE )
        {
            SbiToken eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Comp() ), nCount++;
        }
        
        if( nCount > 1 && !pParser->IsVBASupportOn() )
        {
            pParser->Error( SbERR_SYNTAX );
            bError = true;
        }
    }
    return pNd;
}

SbiExprNode* SbiExpression::Boolean()
{
    SbiExprNode* pNd = Like();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        for( ;; )
        {
            SbiToken eTok = pParser->Peek();
            if( (eTok != AND) && (eTok != OR) &&
                (eTok != XOR) && (eTok != EQV) &&
                (eTok != IMP) && (eTok != IS) )
            {
                break;
            }
            eTok = pParser->Next();
            pNd = new SbiExprNode( pParser, pNd, eTok, Like() );
        }
    }
    return pNd;
}

/***************************************************************************
|*
|*      SbiConstExpression
|*
***************************************************************************/

SbiConstExpression::SbiConstExpression( SbiParser* p ) : SbiExpression( p )
{
    if( pExpr->IsConstant() )
    {
        eType = pExpr->GetType();
        if( pExpr->IsNumber() )
        {
            nVal = pExpr->nVal;
        }
        else
        {
            nVal = 0;
            aVal = pExpr->aStrVal;
        }
    }
    else
    {
        
        bool bIsBool = false;
        if( pExpr->eNodeType == SbxVARVAL )
        {
            SbiSymDef* pVarDef = pExpr->GetVar();

            bool bBoolVal = false;
            if( pVarDef->GetName().equalsIgnoreAsciiCase( "true" ) )
            {
                bIsBool = true;
                bBoolVal = true;
            }
            else if( pVarDef->GetName().equalsIgnoreAsciiCase( "false" ) )
            
            {
                bIsBool = true;
                bBoolVal = false;
            }

            if( bIsBool )
            {
                delete pExpr;
                pExpr = new SbiExprNode( pParser, (bBoolVal ? SbxTRUE : SbxFALSE), SbxINTEGER );
                eType = pExpr->GetType();
                nVal = pExpr->nVal;
            }
        }

        if( !bIsBool )
        {
            pParser->Error( SbERR_SYNTAX );
            eType = SbxDOUBLE;
            nVal = 0;
        }
    }
}

short SbiConstExpression::GetShortValue()
{
    if( eType == SbxSTRING )
    {
        SbxVariableRef refConv = new SbxVariable;
        refConv->PutString( aVal );
        return refConv->GetInteger();
    }
    else
    {
        double n = nVal;
        if( n > 0 )
        {
            n += .5;
        }
        else
        {
            n -= .5;
        }
        if( n > SbxMAXINT )
        {
            n = SbxMAXINT;
            pParser->Error( SbERR_OUT_OF_RANGE );
        }
        else if( n < SbxMININT )
        {
            n = SbxMININT;
            pParser->Error( SbERR_OUT_OF_RANGE );
        }

        return (short) n;
    }
}


/***************************************************************************
|*
|*      SbiExprList
|*
***************************************************************************/

SbiExprList::SbiExprList( SbiParser* p )
{
    pParser = p;
    pFirst = NULL;
    nExpr  =
    nDim   = 0;
    bError = false;
    bBracket = false;
}

SbiExprList::~SbiExprList()
{
    SbiExpression* p = pFirst;
    while( p )
    {
        SbiExpression* q = p->pNext;
        delete p;
        p = q;
    }
}


SbiExpression* SbiExprList::Get( short n )
{
    SbiExpression* p = pFirst;
    while( n-- && p )
    {
        p = p->pNext;
    }
    return p;
}

void SbiExprList::addExpression( SbiExpression* pExpr )
{
    if( !pFirst )
    {
        pFirst = pExpr;
        return;
    }

    SbiExpression* p = pFirst;
    while( p->pNext )
    {
        p = p->pNext;
    }
    p->pNext = pExpr;
}


/***************************************************************************
|*
|*      SbiParameters
|*
***************************************************************************/










SbiParameters::SbiParameters( SbiParser* p, bool bStandaloneExpression, bool bPar) :
    SbiExprList( p )
{
    if( !bPar )
    {
        return;
    }
    SbiExpression *pExpr;
    SbiToken eTok = pParser->Peek();

    bool bAssumeExprLParenMode = false;
    bool bAssumeArrayMode = false;
    if( eTok == LPAREN )
    {
        if( bStandaloneExpression )
        {
            bAssumeExprLParenMode = true;
        }
        else
        {
            bBracket = true;
            pParser->Next();
            eTok = pParser->Peek();
        }
    }


    if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
    {
        if( eTok == RPAREN )
        {
            pParser->Next();
        }
        return;
    }
    
    SbiExpression* pLast = NULL;
    OUString aName;
    while( !bError )
    {
        aName = "";
        
        if( eTok == COMMA )
        {
            pExpr = new SbiExpression( pParser, 0, SbxEMPTY );
        }
        
        else
        {
            bool bByVal = false;
            if( eTok == BYVAL )
            {
                bByVal = true;
                pParser->Next();
                eTok = pParser->Peek();
            }

            if( bAssumeExprLParenMode )
            {
                pExpr = new SbiExpression( pParser, SbSTDEXPR, EXPRMODE_LPAREN_PENDING );
                bAssumeExprLParenMode = false;

                SbiExprMode eModeAfter = pExpr->m_eMode;
                if( eModeAfter == EXPRMODE_LPAREN_NOT_NEEDED )
                {
                    bBracket = true;
                }
                else if( eModeAfter == EXPRMODE_ARRAY_OR_OBJECT )
                {
                    
                    
                    
                    bBracket = true;
                    bAssumeArrayMode = true;
                    eTok = NIL;
                }
                else if( eModeAfter == EXPRMODE_EMPTY_PAREN )
                {
                    bBracket = true;
                    delete pExpr;
                    if( bByVal )
                    {
                        pParser->Error( SbERR_LVALUE_EXPECTED );
                    }
                    return;
                }
            }
            else
            {
                pExpr = new SbiExpression( pParser );
            }
            if( bByVal && pExpr->IsLvalue() )
            {
                pExpr->SetByVal();
            }
            if( !bAssumeArrayMode )
            {
                if( pParser->Peek() == ASSIGN )
                {
                    
                    
                    aName = pExpr->GetString();
                    delete pExpr;
                    pParser->Next();
                    pExpr = new SbiExpression( pParser );
                }
                pExpr->GetName() = aName;
            }
        }
        pExpr->pNext = NULL;
        if( !pLast )
        {
            pFirst = pLast = pExpr;
        }
        else
        {
            pLast->pNext = pExpr, pLast = pExpr;
        }
        nExpr++;
        bError = bError || !pExpr->IsValid();

        if( bAssumeArrayMode )
        {
            break;
        }
        
        eTok = pParser->Peek();
        if( eTok != COMMA )
        {
            if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
            {
                break;
            }
            pParser->Error( bBracket ? SbERR_BAD_BRACKETS : SbERR_EXPECTED, COMMA );
            bError = true;
        }
        else
        {
            pParser->Next();
            eTok = pParser->Peek();
            if( ( bBracket && eTok == RPAREN ) || pParser->IsEoln( eTok ) )
            {
                break;
            }
        }
    }
    
    if( eTok == RPAREN )
    {
        pParser->Next();
        pParser->Peek();
        if( !bBracket )
        {
            pParser->Error( SbERR_BAD_BRACKETS );
            bError = true;
        }
    }
    nDim = nExpr;
}

/***************************************************************************
|*
|*      SbiDimList
|*
***************************************************************************/





SbiDimList::SbiDimList( SbiParser* p ) : SbiExprList( p )
{
    bConst = true;

    if( pParser->Next() != LPAREN )
    {
        pParser->Error( SbERR_EXPECTED, LPAREN );
        bError = true; return;
    }

    if( pParser->Peek() != RPAREN )
    {
        SbiExpression *pExpr1, *pExpr2, *pLast = NULL;
        SbiToken eTok;
        for( ;; )
        {
            pExpr1 = new SbiExpression( pParser );
            eTok = pParser->Next();
            if( eTok == TO )
            {
                pExpr2 = new SbiExpression( pParser );
                eTok = pParser->Next();
                bConst = bConst && pExpr1->IsIntConstant() && pExpr2->IsIntConstant();
                bError = bError || !pExpr1->IsValid() || !pExpr2->IsValid();
                pExpr1->pNext = pExpr2;
                if( !pLast )
                {
                    pFirst = pExpr1;
                }
                else
                {
                    pLast->pNext = pExpr1;
                }
                pLast = pExpr2;
                nExpr += 2;
            }
            else
            {
                pExpr1->SetBased();
                pExpr1->pNext = NULL;
                bConst = bConst && pExpr1->IsIntConstant();
                bError = bError || !pExpr1->IsValid();
                if( !pLast )
                {
                    pFirst = pLast = pExpr1;
                }
                else
                {
                    pLast->pNext = pExpr1, pLast = pExpr1;
                }
                nExpr++;
            }
            nDim++;
            if( eTok == RPAREN ) break;
            if( eTok != COMMA )
            {
                pParser->Error( SbERR_BAD_BRACKETS );
                pParser->Next();
                break;
            }
        }
    }
    else pParser->Next();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
