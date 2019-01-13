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


#include <memory>
#include <parser.hxx>
#include <basic/sbx.hxx>
#include <expr.hxx>

SbiExpression::SbiExpression( SbiParser* p, SbiExprType t,
    SbiExprMode eMode, const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    pParser = p;
    bBased = bError = bByVal = bBracket = false;
    nParenLevel = 0;
    eCurExpr = t;
    m_eMode = eMode;
    pExpr = (t != SbSTDEXPR ) ? Term( pKeywordSymbolInfo ) : Boolean();
    if( t != SbSYMBOL )
    {
        pExpr->Optimize(pParser);
    }
    if( t == SbLVALUE && !pExpr->IsLvalue() )
    {
        p->Error( ERRCODE_BASIC_LVALUE_EXPECTED );
    }
    if( t == SbOPERAND && !IsVariable() )
    {
        p->Error( ERRCODE_BASIC_VAR_EXPECTED );
    }
}

SbiExpression::SbiExpression( SbiParser* p, double n, SbxDataType t )
{
    pParser = p;
    bBased = bError = bByVal = bBracket = false;
    nParenLevel = 0;
    eCurExpr = SbOPERAND;
    m_eMode = EXPRMODE_STANDARD;
    pExpr = std::make_unique<SbiExprNode>( n, t );
    pExpr->Optimize(pParser);
}

SbiExpression::SbiExpression( SbiParser* p, const SbiSymDef& r, SbiExprListPtr pPar )
{
    pParser = p;
    bBased = bError = bByVal = bBracket = false;
    nParenLevel = 0;
    eCurExpr = SbOPERAND;
    m_eMode = EXPRMODE_STANDARD;
    pExpr = std::make_unique<SbiExprNode>( r, SbxVARIANT, std::move(pPar) );
}

SbiExpression::~SbiExpression() { }

// reading in a complete identifier
// an identifier has the following form:
// name[(Parameter)][.Name[(parameter)]]...
// structure elements are coupled via the element pNext,
// so that they're not in the tree.

// Are there parameters without brackets following? This may be a number,
// a string, a symbol or also a comma (if the 1st parameter is missing)

static bool DoParametersFollow( SbiParser* p, SbiExprType eCurExpr, SbiToken eTok )
{
    if( eTok == LPAREN )
    {
        return true;
    }
    // but only if similar to CALL!
    if( !p->WhiteSpace() || eCurExpr != SbSYMBOL )
    {
        return false;
    }
    if ( eTok == NUMBER || eTok == MINUS || eTok == FIXSTRING ||
         eTok == SYMBOL || eTok == COMMA  || eTok == DOT || eTok == NOT || eTok == BYVAL )
    {
        return true;
    }
    else // check for default params with reserved names ( e.g. names of tokens )
    {
        SbiTokenizer tokens( *static_cast<SbiTokenizer*>(p) );
        // Urk the Next() / Peek() semantics are... weird
        tokens.Next();
        if ( tokens.Peek() == ASSIGN )
        {
            return true;
        }
    }
    return false;
}

// definition of a new symbol

static SbiSymDef* AddSym ( SbiToken eTok, SbiSymPool& rPool, SbiExprType eCurExpr,
                           const OUString& rName, SbxDataType eType, SbiExprList* pPar )
{
    SbiSymDef* pDef;
    // A= is not a procedure
    bool bHasType = ( eTok == EQ || eTok == DOT );
    if( ( !bHasType && eCurExpr == SbSYMBOL ) || pPar )
    {
        // so this is a procedure
        // the correct pool should be found out, as
        // procs must always get into a public pool
        SbiSymPool* pPool = &rPool;
        if( pPool->GetScope() != SbPUBLIC )
        {
            pPool = &rPool.GetParser()->aPublics;
        }
        SbiProcDef* pProc = pPool->AddProc( rName );

        // special treatment for Colls like Documents(1)
        if( eCurExpr == SbSTDEXPR )
        {
            bHasType = true;
        }
        pDef = pProc;
        pDef->SetType( bHasType ? eType : SbxEMPTY );
        if( pPar )
        {
            // generate dummy parameters
            for( sal_Int32 n = 1; n <= pPar->GetSize(); n++ )
            {
                OUString aPar = "PAR" + OUString::number( n );
                pProc->GetParams().AddSym( aPar );
            }
        }
    }
    else
    {
        // or a normal symbol
        pDef = rPool.AddSym( rName );
        pDef->SetType( eType );
    }
    return pDef;
}

// currently even keywords are allowed (because of Dflt properties of the same name)

std::unique_ptr<SbiExprNode> SbiExpression::Term( const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    if( pParser->Peek() == DOT )
    {
        SbiExprNode* pWithVar = pParser->GetWithVar();
        // #26608: get to the node-chain's end to pass the correct object
        SbiSymDef* pDef = pWithVar ? pWithVar->GetRealVar() : nullptr;
        std::unique_ptr<SbiExprNode> pNd;
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
            pParser->Error( ERRCODE_BASIC_UNEXPECTED, DOT );
            pNd = std::make_unique<SbiExprNode>( 1.0, SbxDOUBLE );
        }
        return pNd;
    }

    SbiToken eTok = (pKeywordSymbolInfo == nullptr) ? pParser->Next() : SYMBOL;
    // memorize the parsing's begin
    pParser->LockColumn();
    OUString aSym( (pKeywordSymbolInfo == nullptr) ? pParser->GetSym() : pKeywordSymbolInfo->m_aKeywordSymbol );
    SbxDataType eType = (pKeywordSymbolInfo == nullptr) ? pParser->GetType() : pKeywordSymbolInfo->m_eSbxDataType;
    SbiExprListPtr pPar;
    std::unique_ptr<SbiExprListVector> pvMoreParLcl;
    // are there parameters following?
    SbiToken eNextTok = pParser->Peek();
    // is it a known parameter?
    // create a string constant then, which will be recognized
    // in the SbiParameters-ctor and is continued to be handled
    if( eNextTok == ASSIGN )
    {
        pParser->UnlockColumn();
        return std::make_unique<SbiExprNode>( aSym );
    }
    // no keywords allowed from here on!
    if( SbiTokenizer::IsKwd( eTok )
        && (!pParser->IsCompatible() || eTok != INPUT) )
    {
        pParser->Error( ERRCODE_BASIC_SYNTAX );
        bError = true;
    }

    if( DoParametersFollow( pParser, eCurExpr, eTok = eNextTok ) )
    {
        bool bStandaloneExpression = (m_eMode == EXPRMODE_STANDALONE);
        pPar = SbiExprList::ParseParameters( pParser, bStandaloneExpression );
        bError = bError || !pPar->IsValid();
        if( !bError )
            bBracket = pPar->IsBracket();
        eTok = pParser->Peek();

        // i75443 check for additional sets of parameters
        while( eTok == LPAREN )
        {
            if( pvMoreParLcl == nullptr )
            {
                pvMoreParLcl.reset(new SbiExprListVector);
            }
            SbiExprListPtr pAddPar = SbiExprList::ParseParameters( pParser );
            bError = bError || !pAddPar->IsValid();
            pvMoreParLcl->push_back( std::move(pAddPar) );
            eTok = pParser->Peek();
        }
    }
    // It might be an object part, if . or ! is following.
    // In case of . the variable must already be defined;
    // it's an object, if pDef is NULL after the search.
    bool bObj = ( ( eTok == DOT || eTok == EXCLAM )
                    && !pParser->WhiteSpace() );
    if( bObj )
    {
        bBracket = false;   // Now the bracket for the first term is obsolete
        if( eType == SbxVARIANT )
        {
            eType = SbxOBJECT;
        }
        else
        {
            // Name%. really does not work!
            pParser->Error( ERRCODE_BASIC_BAD_DECLARATION, aSym );
            bError = true;
        }
    }
    // Search:
    SbiSymDef* pDef = pParser->pPool->Find( aSym );
    if( !pDef )
    {
        // Part of the Runtime-Library?
        // from 31.3.1996: swapped out to parser-method
        // (is also needed in SbiParser::DefVar() in DIM.CXX)
        pDef = pParser->CheckRTLForSym( aSym, eType );

        // #i109184: Check if symbol is or later will be defined inside module
        SbModule& rMod = pParser->aGen.GetModule();
        if( rMod.FindMethod( aSym, SbxClassType::DontCare ) )
        {
            pDef = nullptr;
        }
    }
    if( !pDef )
    {
        if( bObj )
        {
            eType = SbxOBJECT;
        }
        pDef = AddSym( eTok, *pParser->pPool, eCurExpr, aSym, eType, pPar.get() );
        // Looks like this is a local ( but undefined variable )
        // if it is in a static procedure then make this Symbol
        // static
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
            pPar = nullptr;
            pvMoreParLcl.reset();
            if( pConst->GetType() == SbxSTRING )
            {
                return std::make_unique<SbiExprNode>( pConst->GetString() );
            }
            else
            {
                return std::make_unique<SbiExprNode>( pConst->GetValue(), pConst->GetType() );
            }
        }

        // 0 parameters come up to ()
        if( pDef->GetDims() )
        {
            if( pPar && pPar->GetSize() && pPar->GetSize() != pDef->GetDims() )
            {
                pParser->Error( ERRCODE_BASIC_WRONG_DIMS );
            }
        }
        if( pDef->IsDefinedAs() )
        {
            SbxDataType eDefType = pDef->GetType();
            // #119187 Only error if types conflict
            if( eType >= SbxINTEGER && eType <= SbxSTRING && eType != eDefType )
            {
                // How? Define with AS first and take a Suffix then?
                pParser->Error( ERRCODE_BASIC_BAD_DECLARATION, aSym );
                bError = true;
            }
            else if ( eType == SbxVARIANT )
            {
                // if there's nothing named, take the type of the entry,
                // but only if the var hasn't been defined with AS XXX
                // so that we catch n% = 5 : print n
                eType = eDefType;
            }
        }
        // checking type of variables:
        // is there named anything different in the scanner?
        // That's OK for methods!
        if( eType != SbxVARIANT &&          // Variant takes everything
            eType != pDef->GetType() &&
            !pDef->GetProcDef() )
        {
            // maybe pDef describes an object that so far has only been
            // recognized as SbxVARIANT - then change type of pDef
            // from 16.12.95 (similar cases possible perhaps?!?)
            if( eType == SbxOBJECT && pDef->GetType() == SbxVARIANT )
            {
                pDef->SetType( SbxOBJECT );
            }
            else
            {
                pParser->Error( ERRCODE_BASIC_BAD_DECLARATION, aSym );
                bError = true;
            }
        }
    }
    std::unique_ptr<SbiExprNode> pNd(new SbiExprNode( *pDef, eType ));
    if( !pPar )
    {
        pPar = SbiExprList::ParseParameters( pParser,false,false );
    }
    pNd->aVar.pPar = pPar.release();
    pNd->aVar.pvMorePar = pvMoreParLcl.release();
    if( bObj )
    {
        // from 8.1.95: Object may also be of the type SbxVARIANT
        if( pDef->GetType() == SbxVARIANT )
            pDef->SetType( SbxOBJECT );
        // if we scan something with point,
        // the type must be SbxOBJECT
        if( pDef->GetType() != SbxOBJECT && pDef->GetType() != SbxVARIANT )
        {
            // defer error until runtime if in vba mode
            if ( !pParser->IsVBASupportOn() )
            {
                pParser->Error( ERRCODE_BASIC_BAD_DECLARATION, aSym );
                bError = true;
            }
        }
        if( !bError )
        {
            pNd->aVar.pNext = ObjTerm( *pDef ).release();
        }
    }

    pParser->UnlockColumn();
    return pNd;
}

// construction of an object term. A term of this kind is part
// of an expression that begins with an object variable.

std::unique_ptr<SbiExprNode> SbiExpression::ObjTerm( SbiSymDef& rObj )
{
    pParser->Next();
    SbiToken eTok = pParser->Next();
    if( eTok != SYMBOL && !SbiTokenizer::IsKwd( eTok ) && !SbiTokenizer::IsExtra( eTok ) )
    {
        // #66745 Some operators can also be allowed
        // as identifiers, important for StarOne
        if( eTok != MOD && eTok != NOT && eTok != AND && eTok != OR &&
            eTok != XOR && eTok != EQV && eTok != IMP && eTok != IS )
        {
            pParser->Error( ERRCODE_BASIC_VAR_EXPECTED );
            bError = true;
        }
    }

    if( bError )
    {
        return nullptr;
    }
    OUString aSym( pParser->GetSym() );
    SbxDataType eType = pParser->GetType();
    SbiExprListPtr pPar;
    SbiExprListVector* pvMoreParLcl = nullptr;
    eTok = pParser->Peek();

    if( DoParametersFollow( pParser, eCurExpr, eTok ) )
    {
        pPar = SbiExprList::ParseParameters( pParser, false/*bStandaloneExpression*/ );
        bError = bError || !pPar->IsValid();
        eTok = pParser->Peek();

        // i109624 check for additional sets of parameters
        while( eTok == LPAREN )
        {
            if( pvMoreParLcl == nullptr )
            {
                pvMoreParLcl = new SbiExprListVector;
            }
            SbiExprListPtr pAddPar = SbiExprList::ParseParameters( pParser );
            bError = bError || !pPar->IsValid();
            pvMoreParLcl->push_back( std::move(pAddPar) );
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
            // Name%. does really not work!
            pParser->Error( ERRCODE_BASIC_BAD_DECLARATION, aSym );
            bError = true;
        }
    }

    // an object's symbol pool is always PUBLIC
    SbiSymPool& rPool = rObj.GetPool();
    rPool.SetScope( SbPUBLIC );
    SbiSymDef* pDef = rPool.Find( aSym );
    if( !pDef )
    {
        pDef = AddSym( eTok, rPool, eCurExpr, aSym, eType, pPar.get() );
        pDef->SetType( eType );
    }

    std::unique_ptr<SbiExprNode> pNd(new SbiExprNode( *pDef, eType ));
    pNd->aVar.pPar = pPar.release();
    pNd->aVar.pvMorePar = pvMoreParLcl;
    if( bObj )
    {
        if( pDef->GetType() == SbxVARIANT )
        {
            pDef->SetType( SbxOBJECT );
        }
        if( pDef->GetType() != SbxOBJECT )
        {
            pParser->Error( ERRCODE_BASIC_BAD_DECLARATION, aSym );
            bError = true;
        }
        if( !bError )
        {
            pNd->aVar.pNext = ObjTerm( *pDef ).release();
            pNd->eType = eType;
        }
    }
    return pNd;
}

// an operand can be:
//      constant
//      scalar variable
//      structure elements
//      array elements
//      functions
//      bracketed expressions

std::unique_ptr<SbiExprNode> SbiExpression::Operand( bool bUsedForTypeOf )
{
    std::unique_ptr<SbiExprNode> pRes;

    // test operand:
    switch( SbiToken eTok = pParser->Peek() )
    {
    case SYMBOL:
        pRes = Term();
        // process something like "IF Not r Is Nothing Then .."
        if( !bUsedForTypeOf && pParser->IsVBASupportOn() && pParser->Peek() == IS )
        {
            eTok = pParser->Next();
            pRes = std::make_unique<SbiExprNode>( std::move(pRes), eTok, Like() );
        }
        break;
    case DOT:   // .with
        pRes = Term(); break;
    case NUMBER:
        pParser->Next();
        pRes = std::make_unique<SbiExprNode>( pParser->GetDbl(), pParser->GetType() );
        break;
    case FIXSTRING:
        pParser->Next();
        pRes = std::make_unique<SbiExprNode>( pParser->GetSym() ); break;
    case LPAREN:
        pParser->Next();
        if( nParenLevel == 0 && m_eMode == EXPRMODE_LPAREN_PENDING && pParser->Peek() == RPAREN )
        {
            m_eMode = EXPRMODE_EMPTY_PAREN;
            pRes = std::make_unique<SbiExprNode>();   // Dummy node
            pParser->Next();
            break;
        }
        nParenLevel++;
        pRes = Boolean();
        if( pParser->Peek() != RPAREN )
        {
            // If there was a LPARAM, it does not belong to the expression
            if( nParenLevel == 1 && m_eMode == EXPRMODE_LPAREN_PENDING )
            {
                m_eMode = EXPRMODE_LPAREN_NOT_NEEDED;
            }
            else
            {
                pParser->Error( ERRCODE_BASIC_BAD_BRACKETS );
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
        // keywords here are OK at the moment!
        if( SbiTokenizer::IsKwd( eTok ) )
        {
            pRes = Term();
        }
        else
        {
            pParser->Next();
            pRes = std::make_unique<SbiExprNode>( 1.0, SbxDOUBLE );
            pParser->Error( ERRCODE_BASIC_UNEXPECTED, eTok );
        }
        break;
    }
    return pRes;
}

std::unique_ptr<SbiExprNode> SbiExpression::Unary()
{
    std::unique_ptr<SbiExprNode> pNd;
    SbiToken eTok = pParser->Peek();
    switch( eTok )
    {
        case MINUS:
            eTok = NEG;
            pParser->Next();
            pNd = std::make_unique<SbiExprNode>( Unary(), eTok, nullptr );
            break;
        case NOT:
            if( pParser->IsVBASupportOn() )
            {
                pNd = Operand();
            }
            else
            {
                pParser->Next();
                pNd = std::make_unique<SbiExprNode>( Unary(), eTok, nullptr );
            }
            break;
        case PLUS:
            pParser->Next();
            pNd = Unary();
            break;
        case TYPEOF:
        {
            pParser->Next();
            std::unique_ptr<SbiExprNode> pObjNode = Operand( true/*bUsedForTypeOf*/ );
            pParser->TestToken( IS );
            SbiSymDef* pTypeDef = new SbiSymDef( OUString() );
            pParser->TypeDecl( *pTypeDef, true );
            pNd = std::make_unique<SbiExprNode>( std::move(pObjNode), pTypeDef->GetTypeId() );
            break;
        }
        case NEW:
        {
            pParser->Next();
            SbiSymDef* pTypeDef = new SbiSymDef( OUString() );
            pParser->TypeDecl( *pTypeDef, true );
            pNd = std::make_unique<SbiExprNode>( pTypeDef->GetTypeId() );
            break;
        }
        default:
            pNd = Operand();
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::Exp()
{
    std::unique_ptr<SbiExprNode> pNd = Unary();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == EXPON )
        {
            SbiToken eTok = pParser->Next();
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, Unary() );
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::MulDiv()
{
    std::unique_ptr<SbiExprNode> pNd = Exp();
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
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, Exp() );
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::IntDiv()
{
    std::unique_ptr<SbiExprNode> pNd = MulDiv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == IDIV )
        {
            SbiToken eTok = pParser->Next();
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, MulDiv() );
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::Mod()
{
    std::unique_ptr<SbiExprNode> pNd = IntDiv();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        while( pParser->Peek() == MOD )
        {
            SbiToken eTok = pParser->Next();
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, IntDiv() );
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::AddSub()
{
    std::unique_ptr<SbiExprNode> pNd = Mod();
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
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, Mod() );
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::Cat()
{
    std::unique_ptr<SbiExprNode> pNd = AddSub();
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
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, AddSub() );
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::Comp()
{
    std::unique_ptr<SbiExprNode> pNd = Cat();
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
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, Cat() );
            nCount++;
        }
    }
    return pNd;
}


std::unique_ptr<SbiExprNode> SbiExpression::VBA_Not()
{
    std::unique_ptr<SbiExprNode> pNd;

    SbiToken eTok = pParser->Peek();
    if( eTok == NOT )
    {
        pParser->Next();
        pNd = std::make_unique<SbiExprNode>( VBA_Not(), eTok, nullptr );
    }
    else
    {
        pNd = Comp();
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::Like()
{
    std::unique_ptr<SbiExprNode> pNd = pParser->IsVBASupportOn() ? VBA_Not() : Comp();
    if( m_eMode != EXPRMODE_EMPTY_PAREN )
    {
        short nCount = 0;
        while( pParser->Peek() == LIKE )
        {
            SbiToken eTok = pParser->Next();
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, Comp() );
            nCount++;
        }
        // multiple operands in a row does not work
        if( nCount > 1 && !pParser->IsVBASupportOn() )
        {
            pParser->Error( ERRCODE_BASIC_SYNTAX );
            bError = true;
        }
    }
    return pNd;
}

std::unique_ptr<SbiExprNode> SbiExpression::Boolean()
{
    std::unique_ptr<SbiExprNode> pNd = Like();
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
            pNd = std::make_unique<SbiExprNode>( std::move(pNd), eTok, Like() );
        }
    }
    return pNd;
}

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
        // #40204 special treatment for sal_Bool-constants
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
            //else if( pVarDef->GetName().ICompare( "false" ) == COMPARE_EQUAL )
            {
                bIsBool = true;
                bBoolVal = false;
            }

            if( bIsBool )
            {
                pExpr = std::make_unique<SbiExprNode>( (bBoolVal ? SbxTRUE : SbxFALSE), SbxINTEGER );
                eType = pExpr->GetType();
                nVal = pExpr->nVal;
            }
        }

        if( !bIsBool )
        {
            pParser->Error( ERRCODE_BASIC_SYNTAX );
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
            pParser->Error( ERRCODE_BASIC_OUT_OF_RANGE );
        }
        else if( n < SbxMININT )
        {
            n = SbxMININT;
            pParser->Error( ERRCODE_BASIC_OUT_OF_RANGE );
        }

        return static_cast<short>(n);
    }
}


SbiExprList::SbiExprList( )
{
    nDim   = 0;
    bError = false;
    bBracket = false;
}

SbiExprList::~SbiExprList() {}

SbiExpression* SbiExprList::Get( size_t n )
{
    return aData[n].get();
}

void SbiExprList::addExpression( std::unique_ptr<SbiExpression>&& pExpr )
{
    aData.push_back(std::move(pExpr));
}

// the parameter list is completely parsed
// "procedurename()" is OK
// it's a function without parameters then
// i. e. you give an array as procedure parameter

// #i79918/#i80532: bConst has never been set to true
// -> reused as bStandaloneExpression
//SbiParameters::SbiParameters( SbiParser* p, sal_Bool bConst, sal_Bool bPar) :
SbiExprListPtr SbiExprList::ParseParameters( SbiParser* pParser, bool bStandaloneExpression, bool bPar)
{
    auto pExprList = std::make_unique<SbiExprList>();
    if( !bPar )
    {
        return pExprList;
    }

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
            pExprList->bBracket = true;
            pParser->Next();
            eTok = pParser->Peek();
        }
    }


    if( ( pExprList->bBracket && eTok == RPAREN ) || SbiTokenizer::IsEoln( eTok ) )
    {
        if( eTok == RPAREN )
        {
            pParser->Next();
        }
        return pExprList;
    }
    // read in parameter table and lay down in correct order!
    while( !pExprList->bError )
    {
        std::unique_ptr<SbiExpression> pExpr;
        // missing argument
        if( eTok == COMMA )
        {
            pExpr = std::make_unique<SbiExpression>( pParser, 0, SbxEMPTY );
        }
        // named arguments: either .name= or name:=
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
                pExpr = std::make_unique<SbiExpression>( pParser, SbSTDEXPR, EXPRMODE_LPAREN_PENDING );
                bAssumeExprLParenMode = false;

                SbiExprMode eModeAfter = pExpr->m_eMode;
                if( eModeAfter == EXPRMODE_LPAREN_NOT_NEEDED )
                {
                    pExprList->bBracket = true;
                }
                else if( eModeAfter == EXPRMODE_ARRAY_OR_OBJECT )
                {
                    // Expression "looks" like an array assignment
                    // a(...)[(...)] = ? or a(...).b(...)
                    // RPAREN is already parsed
                    pExprList->bBracket = true;
                    bAssumeArrayMode = true;
                    eTok = NIL;
                }
                else if( eModeAfter == EXPRMODE_EMPTY_PAREN )
                {
                    pExprList->bBracket = true;
                    return pExprList;
                }
            }
            else
            {
                pExpr = std::make_unique<SbiExpression>( pParser );
            }
            if( bByVal && pExpr->IsLvalue() )
            {
                pExpr->SetByVal();
            }
            if( !bAssumeArrayMode )
            {
                OUString aName;
                if( pParser->Peek() == ASSIGN )
                {
                    // VBA mode: name:=
                    // SbiExpression::Term() has made as string out of it
                    aName = pExpr->GetString();
                    pParser->Next();
                    pExpr = std::make_unique<SbiExpression>( pParser );
                }
                pExpr->GetName() = aName;
            }
        }
        pExprList->bError = pExprList->bError || !pExpr->IsValid();
        pExprList->aData.push_back(std::move(pExpr));
        if( bAssumeArrayMode )
        {
            break;
        }
        // next element?
        eTok = pParser->Peek();
        if( eTok != COMMA )
        {
            if( ( pExprList->bBracket && eTok == RPAREN ) || SbiTokenizer::IsEoln( eTok ) )
            {
                break;
            }
            pParser->Error( pExprList->bBracket ? ERRCODE_BASIC_BAD_BRACKETS : ERRCODE_BASIC_EXPECTED, COMMA );
            pExprList->bError = true;
        }
        else
        {
            pParser->Next();
            eTok = pParser->Peek();
            if( ( pExprList->bBracket && eTok == RPAREN ) || SbiTokenizer::IsEoln( eTok ) )
            {
                break;
            }
        }
    }
    // closing bracket
    if( eTok == RPAREN )
    {
        pParser->Next();
        pParser->Peek();
        if( !pExprList->bBracket )
        {
            pParser->Error( ERRCODE_BASIC_BAD_BRACKETS );
            pExprList->bError = true;
        }
    }
    pExprList->nDim = pExprList->GetSize();
    return pExprList;
}

// A list of array dimensions is parsed.

SbiExprListPtr SbiExprList::ParseDimList( SbiParser* pParser )
{
    auto pExprList = std::make_unique<SbiExprList>();

    if( pParser->Next() != LPAREN )
    {
        pParser->Error( ERRCODE_BASIC_EXPECTED, LPAREN );
        pExprList->bError = true; return pExprList;
    }

    if( pParser->Peek() != RPAREN )
    {
        SbiToken eTok;
        for( ;; )
        {
            auto pExpr1 = std::make_unique<SbiExpression>( pParser );
            eTok = pParser->Next();
            if( eTok == TO )
            {
                auto pExpr2 = std::make_unique<SbiExpression>( pParser );
                pExpr1->ConvertToIntConstIfPossible();
                pExpr2->ConvertToIntConstIfPossible();
                eTok = pParser->Next();
                pExprList->bError = pExprList->bError || !pExpr1->IsValid() || !pExpr2->IsValid();
                pExprList->aData.push_back(std::move(pExpr1));
                pExprList->aData.push_back(std::move(pExpr2));
            }
            else
            {
                pExpr1->SetBased();
                pExpr1->ConvertToIntConstIfPossible();
                pExprList->bError = pExprList->bError || !pExpr1->IsValid();
                pExprList->aData.push_back(std::move(pExpr1));
            }
            pExprList->nDim++;
            if( eTok == RPAREN ) break;
            if( eTok != COMMA )
            {
                pParser->Error( ERRCODE_BASIC_BAD_BRACKETS );
                pParser->Next();
                break;
            }
        }
    }
    else pParser->Next();
    return pExprList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
