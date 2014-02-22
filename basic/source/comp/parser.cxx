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

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include <com/sun/star/script/ModuleType.hpp>
#include <svtools/miscopt.hxx>

struct SbiParseStack {              
    SbiParseStack* pNext;           
    SbiExprNode* pWithVar;
    SbiToken eExitTok;
    sal_uInt32  nChain;                 
};

struct SbiStatement {
    SbiToken eTok;
    void( SbiParser::*Func )();
    bool  bMain;                    
    bool  bSubr;                    
};

#define Y   true
#define N   false

static const SbiStatement StmntTable [] = {
{ ATTRIBUTE, &SbiParser::Attribute, Y, Y, }, 
{ CALL,     &SbiParser::Call,       N, Y, }, 
{ CLOSE,    &SbiParser::Close,      N, Y, }, 
{ _CONST_,  &SbiParser::Dim,        Y, Y, }, 
{ DECLARE,  &SbiParser::Declare,    Y, N, }, 
{ DEFBOOL,  &SbiParser::DefXXX,     Y, N, }, 
{ DEFCUR,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFDATE,  &SbiParser::DefXXX,     Y, N, }, 
{ DEFDBL,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFERR,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFINT,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFLNG,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFOBJ,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFSNG,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFSTR,   &SbiParser::DefXXX,     Y, N, }, 
{ DEFVAR,   &SbiParser::DefXXX,     Y, N, }, 
{ DIM,      &SbiParser::Dim,        Y, Y, }, 
{ DO,       &SbiParser::DoLoop,     N, Y, }, 
{ ELSE,     &SbiParser::NoIf,       N, Y, }, 
{ ELSEIF,   &SbiParser::NoIf,       N, Y, }, 
{ ENDIF,    &SbiParser::NoIf,       N, Y, }, 
{ END,      &SbiParser::Stop,       N, Y, }, 
{ ENUM,     &SbiParser::Enum,       Y, N, }, 
{ ERASE,    &SbiParser::Erase,      N, Y, }, 
{ _ERROR_,  &SbiParser::ErrorStmnt, N, Y, }, 
{ EXIT,     &SbiParser::Exit,       N, Y, }, 
{ FOR,      &SbiParser::For,        N, Y, }, 
{ FUNCTION, &SbiParser::SubFunc,    Y, N, }, 
{ GOSUB,    &SbiParser::Goto,       N, Y, }, 
{ GLOBAL,   &SbiParser::Dim,        Y, N, }, 
{ GOTO,     &SbiParser::Goto,       N, Y, }, 
{ IF,       &SbiParser::If,         N, Y, }, 
{ IMPLEMENTS, &SbiParser::Implements, Y, N, }, 
{ INPUT,    &SbiParser::Input,      N, Y, }, 
{ LET,      &SbiParser::Assign,     N, Y, }, 
{ LINE,     &SbiParser::Line,       N, Y, }, 
{ LINEINPUT,&SbiParser::LineInput,  N, Y, }, 
{ LOOP,     &SbiParser::BadBlock,   N, Y, }, 
{ LSET,     &SbiParser::LSet,       N, Y, }, 
{ NAME,     &SbiParser::Name,       N, Y, }, 
{ NEXT,     &SbiParser::BadBlock,   N, Y, }, 
{ ON,       &SbiParser::On,         N, Y, }, 
{ OPEN,     &SbiParser::Open,       N, Y, }, 
{ OPTION,   &SbiParser::Option,     Y, N, }, 
{ PRINT,    &SbiParser::Print,      N, Y, }, 
{ PRIVATE,  &SbiParser::Dim,        Y, N, }, 
{ PROPERTY, &SbiParser::SubFunc,    Y, N, }, 
{ PUBLIC,   &SbiParser::Dim,        Y, N, }, 
{ REDIM,    &SbiParser::ReDim,      N, Y, }, 
{ RESUME,   &SbiParser::Resume,     N, Y, }, 
{ RETURN,   &SbiParser::Return,     N, Y, }, 
{ RSET,     &SbiParser::RSet,       N, Y, }, 
{ SELECT,   &SbiParser::Select,     N, Y, }, 
{ SET,      &SbiParser::Set,        N, Y, }, 
{ STATIC,   &SbiParser::Static,     Y, Y, }, 
{ STOP,     &SbiParser::Stop,       N, Y, }, 
{ SUB,      &SbiParser::SubFunc,    Y, N, }, 
{ TYPE,     &SbiParser::Type,       Y, N, }, 
{ UNTIL,    &SbiParser::BadBlock,   N, Y, }, 
{ WHILE,    &SbiParser::While,      N, Y, }, 
{ WEND,     &SbiParser::BadBlock,   N, Y, }, 
{ WITH,     &SbiParser::With,       N, Y, }, 
{ WRITE,    &SbiParser::Write,      N, Y, }, 

{ NIL, NULL, N, N }
};


SbiParser::SbiParser( StarBASIC* pb, SbModule* pm )
        : SbiTokenizer( pm->GetSource32(), pb ),
          aGblStrings( this ),
          aLclStrings( this ),
          aGlobals( aGblStrings, SbGLOBAL ),
          aPublics( aGblStrings, SbPUBLIC ),
          aRtlSyms( aGblStrings, SbRTL ),
          aGen( *pm, this, 1024 )
{
    pBasic   = pb;
    eCurExpr = SbSYMBOL;
    eEndTok  = NIL;
    pProc    = NULL;
    pStack   = NULL;
    pWithVar = NULL;
    nBase    = 0;
    bText    =
    bGblDefs =
    bNewGblDefs =
    bSingleLineIf =
    bCodeCompleting =
    bExplicit = false;
    bClassModule = ( pm->GetModuleType() == com::sun::star::script::ModuleType::CLASS );
    OSL_TRACE("Parser - %s, bClassModule %d", OUStringToOString( pm->GetName(), RTL_TEXTENCODING_UTF8 ).getStr(), bClassModule );
    pPool    = &aPublics;
    for( short i = 0; i < 26; i++ )
        eDefTypes[ i ] = SbxVARIANT;    

    aPublics.SetParent( &aGlobals );
    aGlobals.SetParent( &aRtlSyms );


    nGblChain = aGen.Gen( _JUMP, 0 );

    rTypeArray = new SbxArray; 
    rEnumArray = new SbxArray; 
    bVBASupportOn = pm->IsVBACompat();
    if ( bVBASupportOn )
        EnableCompatibility();

}


SbiSymDef* SbiParser::CheckRTLForSym( const OUString& rSym, SbxDataType eType )
{
    SbxVariable* pVar = GetBasic()->GetRtl()->Find( rSym, SbxCLASS_DONTCARE );
    SbiSymDef* pDef = NULL;
    if( pVar )
    {
        if( pVar->IsA( TYPE(SbxMethod) ) )
        {
            SbiProcDef* pProc_ = aRtlSyms.AddProc( rSym );
            pProc_->SetType( pVar->GetType() );
            pDef = pProc_;
        }
        else
        {
            pDef = aRtlSyms.AddSym( rSym );
            pDef->SetType( eType );
        }
    }
    return pDef;
}



bool SbiParser::HasGlobalCode()
{
    if( bGblDefs && nGblChain )
    {
        aGen.BackChain( nGblChain );
        aGen.Gen( _LEAVE );
        nGblChain = 0;
    }
    return bGblDefs;
}

void SbiParser::OpenBlock( SbiToken eTok, SbiExprNode* pVar )
{
    SbiParseStack* p = new SbiParseStack;
    p->eExitTok = eTok;
    p->nChain   = 0;
    p->pWithVar = pWithVar;
    p->pNext    = pStack;
    pStack      = p;
    pWithVar    = pVar;

    
    if( eTok == FOR )
        aGen.IncForLevel();
}

void SbiParser::CloseBlock()
{
    if( pStack )
    {
        SbiParseStack* p = pStack;

        
        if( p->eExitTok == FOR )
            aGen.DecForLevel();

        aGen.BackChain( p->nChain );
        pStack = p->pNext;
        pWithVar = p->pWithVar;
        delete p;
    }
}



void SbiParser::Exit()
{
    SbiToken eTok = Next();
    for( SbiParseStack* p = pStack; p; p = p->pNext )
    {
        SbiToken eExitTok = p->eExitTok;
        if( eTok == eExitTok ||
            (eTok == PROPERTY && (eExitTok == GET || eExitTok == LET) ) )   
        {
            p->nChain = aGen.Gen( _JUMP, p->nChain );
            return;
        }
    }
    if( pStack )
        Error( SbERR_EXPECTED, pStack->eExitTok );
    else
        Error( SbERR_BAD_EXIT );
}

bool SbiParser::TestSymbol( bool bKwdOk )
{
    Peek();
    if( eCurTok == SYMBOL || ( bKwdOk && IsKwd( eCurTok ) ) )
    {
        Next(); return true;
    }
    Error( SbERR_SYMBOL_EXPECTED );
    return false;
}



bool SbiParser::TestToken( SbiToken t )
{
    if( Peek() == t )
    {
        Next(); return true;
    }
    else
    {
        Error( SbERR_EXPECTED, t );
        return false;
    }
}



bool SbiParser::TestComma()
{
    SbiToken eTok = Peek();
    if( IsEoln( eTok ) )
    {
        Next();
        return false;
    }
    else if( eTok != COMMA )
    {
        Error( SbERR_EXPECTED, COMMA );
        return false;
    }
    Next();
    return true;
}



void SbiParser::TestEoln()
{
    if( !IsEoln( Next() ) )
    {
        Error( SbERR_EXPECTED, EOLN );
        while( !IsEoln( Next() ) ) {}
    }
}



void SbiParser::StmntBlock( SbiToken eEnd )
{
    SbiToken xe = eEndTok;
    eEndTok = eEnd;
    while( !bAbort && Parse() ) {}
    eEndTok = xe;
    if( IsEof() )
    {
        Error( SbERR_BAD_BLOCK, eEnd );
        bAbort = true;
    }
}

void SbiParser::SetCodeCompleting( const bool& b )
{
    bCodeCompleting = b;
}

bool SbiParser::IsCodeCompleting() const
{
    return bCodeCompleting;
}

bool SbiParser::Parse()
{
    if( bAbort ) return false;

    EnableErrors();

    bErrorIsSymbol = false;
    Peek();
    bErrorIsSymbol = true;

    if( IsEof() )
    {
        
        
        
        
        if( bNewGblDefs && nGblChain == 0 )
            nGblChain = aGen.Gen( _JUMP, 0 );
        return false;
    }


    if( IsEoln( eCurTok ) )
    {
        Next(); return true;
    }

    if( !bSingleLineIf && MayBeLabel( true ) )
    {
        
        if( !pProc )
            Error( SbERR_NOT_IN_MAIN, aSym );
        else
            pProc->GetLabels().Define( aSym );
        Next(); Peek();

        if( IsEoln( eCurTok ) )
        {
            Next(); return true;
        }
    }

    
    if( eCurTok == eEndTok ||
        ( bVBASupportOn &&      
          (eCurTok == ENDFUNC || eCurTok == ENDPROPERTY || eCurTok == ENDSUB) &&
          (eEndTok == ENDFUNC || eEndTok == ENDPROPERTY || eEndTok == ENDSUB) ) )
    {
        Next();
        if( eCurTok != NIL )
            aGen.Statement();
        return false;
    }

    
    if( eCurTok == REM )
    {
        Next(); return true;
    }

        
    
        if ( eCurTok == _ERROR_ && IsVBASupportOn() ) 
        {
            SbiTokenizer tokens( *(SbiTokenizer*)this );
            tokens.Next();
            if ( tokens.Peek()  == DOT )
            {
                eCurTok = SYMBOL;
        ePush = eCurTok;
            }
    }
    
    
    
    if( eCurTok == SYMBOL || eCurTok == DOT )
    {
        if( !pProc )
            Error( SbERR_EXPECTED, SUB );
        else
        {
            
            Next();
            Push( eCurTok );
            aGen.Statement();
                Symbol();
        }
    }
    else
    {
        Next();

        

        const SbiStatement* p;
        for( p = StmntTable; p->eTok != NIL; p++ )
            if( p->eTok == eCurTok )
                break;
        if( p->eTok != NIL )
        {
            if( !pProc && !p->bMain )
                Error( SbERR_NOT_IN_MAIN, eCurTok );
            else if( pProc && !p->bSubr )
                Error( SbERR_NOT_IN_SUBR, eCurTok );
            else
            {
                
                
                if( bNewGblDefs && nGblChain == 0 &&
                    ( eCurTok == SUB || eCurTok == FUNCTION || eCurTok == PROPERTY ) )
                {
                    nGblChain = aGen.Gen( _JUMP, 0 );
                    bNewGblDefs = false;
                }
                
                if( ( p->bSubr && (eCurTok != STATIC || Peek() == SUB || Peek() == FUNCTION ) ) ||
                        eCurTok == SUB || eCurTok == FUNCTION )
                    aGen.Statement();
                (this->*( p->Func ) )();
                SbxError nSbxErr = SbxBase::GetError();
                if( nSbxErr )
                    SbxBase::ResetError(), Error( (SbError)nSbxErr );
            }
        }
        else
            Error( SbERR_UNEXPECTED, eCurTok );
    }

    
    

    if( !IsEos() )
    {
        Peek();
        if( !IsEos() && eCurTok != ELSE )
        {
            
            Error( SbERR_UNEXPECTED, eCurTok );
            while( !IsEos() ) Next();
        }
    }
    
    
    return true;
}


SbiExprNode* SbiParser::GetWithVar()
{
    if( pWithVar )
        return pWithVar;

    SbiParseStack* p = pStack;
    while( p )
    {
        
        if( p->pWithVar )
            return p->pWithVar;
        p = p->pNext;
    }
    return NULL;
}




void SbiParser::Symbol( const KeywordSymbolInfo* pKeywordSymbolInfo )
{
    SbiExprMode eMode = bVBASupportOn ? EXPRMODE_STANDALONE : EXPRMODE_STANDARD;
    SbiExpression aVar( this, SbSYMBOL, eMode, pKeywordSymbolInfo );

    bool bEQ = ( Peek() == EQ );
    if( !bEQ && bVBASupportOn && aVar.IsBracket() )
        Error( SbERR_EXPECTED, "=" );

    RecursiveMode eRecMode = ( bEQ ? PREVENT_CALL : FORCE_CALL );
    bool bSpecialMidHandling = false;
    SbiSymDef* pDef = aVar.GetRealVar();
    if( bEQ && pDef && pDef->GetScope() == SbRTL )
    {
        OUString aRtlName = pDef->GetName();
        if( aRtlName.equalsIgnoreAsciiCase("Mid") )
        {
            SbiExprNode* pExprNode = aVar.GetExprNode();
            if( pExprNode && pExprNode->GetNodeType() == SbxVARVAL )
            {
                SbiExprList* pPar = pExprNode->GetParameters();
                short nParCount = pPar ? pPar->GetSize() : 0;
                if( nParCount == 2 || nParCount == 3 )
                {
                    if( nParCount == 2 )
                        pPar->addExpression( new SbiExpression( this, -1, SbxLONG ) );

                    TestToken( EQ );
                    pPar->addExpression( new SbiExpression( this ) );

                    bSpecialMidHandling = true;
                }
            }
        }
    }
    aVar.Gen( eRecMode );
    if( !bSpecialMidHandling )
    {
        if( !bEQ )
        {
            aGen.Gen( _GET );
        }
        else
        {
            
            if( !aVar.IsLvalue() )
                Error( SbERR_LVALUE_EXPECTED );
            TestToken( EQ );
            SbiExpression aExpr( this );
            aExpr.Gen();
            SbiOpcode eOp = _PUT;
            if( pDef )
            {
                if( pDef->GetConstDef() )
                    Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
                if( pDef->GetType() == SbxOBJECT )
                {
                    eOp = _SET;
                    if( pDef->GetTypeId() )
                    {
                        aGen.Gen( _SETCLASS, pDef->GetTypeId() );
                        return;
                    }
                }
            }
            aGen.Gen( eOp );
        }
    }
}


void SbiParser::Assign()
{
    SbiExpression aLvalue( this, SbLVALUE );
    TestToken( EQ );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    sal_uInt16 nLen = 0;
    SbiSymDef* pDef = aLvalue.GetRealVar();
    {
        if( pDef->GetConstDef() )
            Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
        nLen = aLvalue.GetRealVar()->GetLen();
    }
    if( nLen )
        aGen.Gen( _PAD, nLen );
    aGen.Gen( _PUT );
}



void SbiParser::Set()
{
    SbiExpression aLvalue( this, SbLVALUE );
    SbxDataType eType = aLvalue.GetType();
    if( eType != SbxOBJECT && eType != SbxEMPTY && eType != SbxVARIANT )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );

    SbiToken eTok = Peek();
    if( eTok == NEW )
    {
        Next();
        OUString aStr;
        SbiSymDef* pTypeDef = new SbiSymDef( aStr );
        TypeDecl( *pTypeDef, true );

        aLvalue.Gen();
        aGen.Gen( _CREATE, pDef->GetId(), pTypeDef->GetTypeId() );
        aGen.Gen( _SETCLASS, pDef->GetTypeId() );
    }
    else
    {
        SbiExpression aExpr( this );
        aLvalue.Gen();
        aExpr.Gen();
        
        
        
        
        
        if( pDef->GetTypeId() )
        {
            if ( bVBASupportOn )
                aGen.Gen( _VBASETCLASS, pDef->GetTypeId() );
            else
                aGen.Gen( _SETCLASS, pDef->GetTypeId() );
        }
        else
        {
            if ( bVBASupportOn )
                aGen.Gen( _VBASET );
            else
                aGen.Gen( _SET );
        }
    }
}


void SbiParser::LSet()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxSTRING )
    {
        Error( SbERR_INVALID_OBJECT );
    }
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
    {
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    }
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    aGen.Gen( _LSET );
}


void SbiParser::RSet()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxSTRING )
    {
        Error( SbERR_INVALID_OBJECT );
    }
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    aGen.Gen( _RSET );
}



void SbiParser::DefXXX()
{
    sal_Unicode ch1, ch2;
    SbxDataType t = SbxDataType( eCurTok - DEFINT + SbxINTEGER );

    while( !bAbort )
    {
        if( Next() != SYMBOL ) break;
        ch1 = aSym.toAsciiUpperCase()[0];
        ch2 = 0;
        if( Peek() == MINUS )
        {
            Next();
            if( Next() != SYMBOL ) Error( SbERR_SYMBOL_EXPECTED );
            else
            {
                ch2 = aSym.toAsciiUpperCase()[0];
                if( ch2 < ch1 ) Error( SbERR_SYNTAX ), ch2 = 0;
            }
        }
        if (!ch2) ch2 = ch1;
        ch1 -= 'A'; ch2 -= 'A';
        for (; ch1 <= ch2; ch1++) eDefTypes[ ch1 ] = t;
        if( !TestComma() ) break;
    }
}



void SbiParser::Stop()
{
    aGen.Gen( _STOP );
    Peek();     
}



void SbiParser::Implements()
{
    if( !bClassModule )
    {
        Error( SbERR_UNEXPECTED, IMPLEMENTS );
        return;
    }

    Peek();
    if( eCurTok != SYMBOL )
    {
        Error( SbERR_SYMBOL_EXPECTED );
        return;
    }

    OUString aImplementedIface = aSym;
    Next();
    if( Peek() == DOT )
    {
        OUString aDotStr( '.' );
        while( Peek() == DOT )
        {
            aImplementedIface += aDotStr;
            Next();
            SbiToken ePeekTok = Peek();
            if( ePeekTok == SYMBOL || IsKwd( ePeekTok ) )
            {
                Next();
                aImplementedIface += aSym;
            }
            else
            {
                Next();
                Error( SbERR_SYMBOL_EXPECTED );
                break;
            }
        }
    }
    aIfaceVector.push_back( aImplementedIface );
}

void SbiParser::EnableCompatibility()
{
    if( !bCompatible )
        AddConstants();
    bCompatible = true;
}



void SbiParser::Option()
{
    switch( Next() )
    {
        case BASIC_EXPLICIT:
            bExplicit = true; break;
        case BASE:
            if( Next() == NUMBER )
            {
                if( nVal == 0 || nVal == 1 )
                {
                    nBase = (short) nVal;
                    break;
                }
            }
            Error( SbERR_EXPECTED, "0/1" );
            break;
        case PRIVATE:
        {
            OUString aString = SbiTokenizer::Symbol(Next());
            if( !aString.equalsIgnoreAsciiCase("Module") )
            {
                Error( SbERR_EXPECTED, "Module" );
            }
            break;
        }
        case COMPARE:
        {
            SbiToken eTok = Next();
            if( eTok == BINARY )
            {
                bText = false;
            }
            else if( eTok == SYMBOL && GetSym().equalsIgnoreAsciiCase("text") )
            {
                bText = true;
            }
            else
            {
                Error( SbERR_EXPECTED, "Text/Binary" );
            }
            break;
        }
        case COMPATIBLE:
            EnableCompatibility();
            break;

        case CLASSMODULE:
            bClassModule = true;
            aGen.GetModule().SetModuleType( com::sun::star::script::ModuleType::CLASS );
            break;
        case VBASUPPORT: 
            if( Next() == NUMBER )
            {
                if ( nVal == 1 || nVal == 0 )
                {
                    bVBASupportOn = ( nVal == 1 );
                    if ( bVBASupportOn )
                    {
                        EnableCompatibility();
                    }
                    
                    
                    if ( bVBASupportOn != aGen.GetModule().IsVBACompat() )
                    {
                        aGen.GetModule().SetVBACompat( bVBASupportOn );
                    }
                    break;
                }
            }
            Error( SbERR_EXPECTED, "0/1" );
            break;
        default:
            Error( SbERR_BAD_OPTION, eCurTok );
    }
}

void addStringConst( SbiSymPool& rPool, const char* pSym, const OUString& rStr )
{
    SbiConstDef* pConst = new SbiConstDef( OUString::createFromAscii( pSym ) );
    pConst->SetType( SbxSTRING );
    pConst->Set( rStr );
    rPool.Add( pConst );
}

inline void addStringConst( SbiSymPool& rPool, const char* pSym, const char* pStr )
{
    addStringConst( rPool, pSym, OUString::createFromAscii( pStr ) );
}

void SbiParser::AddConstants( void )
{
    
    addStringConst( aPublics, "vbCr", "\x0D" );
    addStringConst( aPublics, "vbCrLf", "\x0D\x0A" );
    addStringConst( aPublics, "vbFormFeed", "\x0C" );
    addStringConst( aPublics, "vbLf", "\x0A" );
#ifndef WNT
    addStringConst( aPublics, "vbNewLine", "\x0D\x0A" );
#else
    addStringConst( aPublics, "vbNewLine", "\x0A" );
#endif
    addStringConst( aPublics, "vbNullString", "" );
    addStringConst( aPublics, "vbTab", "\x09" );
    addStringConst( aPublics, "vbVerticalTab", "\x0B" );

    
    OUString aNullCharStr((sal_Unicode)0);
    addStringConst( aPublics, "vbNullChar", aNullCharStr );
}



void SbiParser::ErrorStmnt()
{
    SbiExpression aPar( this );
    aPar.Gen();
    aGen.Gen( _ERROR );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
