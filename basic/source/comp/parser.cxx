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

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include <com/sun/star/script/ModuleType.hpp>

struct SbiParseStack {              // "Stack" for statement-blocks
    SbiParseStack* pNext;           // Chain
    SbiExprNode* pWithVar;
    SbiToken eExitTok;
    sal_uInt32  nChain;                 // JUMP-Chain
};

struct SbiStatement {
    SbiToken eTok;
    void( SbiParser::*Func )();
    sal_Bool  bMain;                    // sal_True: OK outside the SUB
    sal_Bool  bSubr;                    // sal_True: OK inside the SUB
};

#define Y   sal_True
#define N   sal_False

static SbiStatement StmntTable [] = {
{ ATTRIBUTE, &SbiParser::Attribute, Y, Y, }, // ATTRIBUTE
{ CALL,     &SbiParser::Call,       N, Y, }, // CALL
{ CLOSE,    &SbiParser::Close,      N, Y, }, // CLOSE
{ _CONST_,  &SbiParser::Dim,        Y, Y, }, // CONST
{ DECLARE,  &SbiParser::Declare,    Y, N, }, // DECLARE
{ DEFBOOL,  &SbiParser::DefXXX,     Y, N, }, // DEFBOOL
{ DEFCUR,   &SbiParser::DefXXX,     Y, N, }, // DEFCUR
{ DEFDATE,  &SbiParser::DefXXX,     Y, N, }, // DEFDATE
{ DEFDBL,   &SbiParser::DefXXX,     Y, N, }, // DEFDBL
{ DEFERR,   &SbiParser::DefXXX,     Y, N, }, // DEFERR
{ DEFINT,   &SbiParser::DefXXX,     Y, N, }, // DEFINT
{ DEFLNG,   &SbiParser::DefXXX,     Y, N, }, // DEFLNG
{ DEFOBJ,   &SbiParser::DefXXX,     Y, N, }, // DEFOBJ
{ DEFSNG,   &SbiParser::DefXXX,     Y, N, }, // DEFSNG
{ DEFSTR,   &SbiParser::DefXXX,     Y, N, }, // DEFSTR
{ DEFVAR,   &SbiParser::DefXXX,     Y, N, }, // DEFVAR
{ DIM,      &SbiParser::Dim,        Y, Y, }, // DIM
{ DO,       &SbiParser::DoLoop,     N, Y, }, // DO
{ ELSE,     &SbiParser::NoIf,       N, Y, }, // ELSE
{ ELSEIF,   &SbiParser::NoIf,       N, Y, }, // ELSEIF
{ ENDIF,    &SbiParser::NoIf,       N, Y, }, // ENDIF
{ END,      &SbiParser::Stop,       N, Y, }, // END
{ ENUM,     &SbiParser::Enum,       Y, N, }, // TYPE
{ ERASE,    &SbiParser::Erase,      N, Y, }, // ERASE
{ _ERROR_,  &SbiParser::ErrorStmnt, N, Y, }, // ERROR
{ EXIT,     &SbiParser::Exit,       N, Y, }, // EXIT
{ FOR,      &SbiParser::For,        N, Y, }, // FOR
{ FUNCTION, &SbiParser::SubFunc,    Y, N, }, // FUNCTION
{ GOSUB,    &SbiParser::Goto,       N, Y, }, // GOSUB
{ GLOBAL,   &SbiParser::Dim,        Y, N, }, // GLOBAL
{ GOTO,     &SbiParser::Goto,       N, Y, }, // GOTO
{ IF,       &SbiParser::If,         N, Y, }, // IF
{ IMPLEMENTS, &SbiParser::Implements, Y, N, }, // IMPLEMENTS
{ INPUT,    &SbiParser::Input,      N, Y, }, // INPUT
{ LET,      &SbiParser::Assign,     N, Y, }, // LET
{ LINE,     &SbiParser::Line,       N, Y, }, // LINE, -> LINE INPUT (#i92642)
{ LINEINPUT,&SbiParser::LineInput,  N, Y, }, // LINE INPUT
{ LOOP,     &SbiParser::BadBlock,   N, Y, }, // LOOP
{ LSET,     &SbiParser::LSet,       N, Y, }, // LSET
{ NAME,     &SbiParser::Name,       N, Y, }, // NAME
{ NEXT,     &SbiParser::BadBlock,   N, Y, }, // NEXT
{ ON,       &SbiParser::On,         N, Y, }, // ON
{ OPEN,     &SbiParser::Open,       N, Y, }, // OPEN
{ OPTION,   &SbiParser::Option,     Y, N, }, // OPTION
{ PRINT,    &SbiParser::Print,      N, Y, }, // PRINT
{ PRIVATE,  &SbiParser::Dim,        Y, N, }, // PRIVATE
{ PROPERTY, &SbiParser::SubFunc,    Y, N, }, // FUNCTION
{ PUBLIC,   &SbiParser::Dim,        Y, N, }, // PUBLIC
{ REDIM,    &SbiParser::ReDim,      N, Y, }, // DIM
{ RESUME,   &SbiParser::Resume,     N, Y, }, // RESUME
{ RETURN,   &SbiParser::Return,     N, Y, }, // RETURN
{ RSET,     &SbiParser::RSet,       N, Y, }, // RSET
{ SELECT,   &SbiParser::Select,     N, Y, }, // SELECT
{ SET,      &SbiParser::Set,        N, Y, }, // SET
{ STATIC,   &SbiParser::Static,     Y, Y, }, // STATIC
{ STOP,     &SbiParser::Stop,       N, Y, }, // STOP
{ SUB,      &SbiParser::SubFunc,    Y, N, }, // SUB
{ TYPE,     &SbiParser::Type,       Y, N, }, // TYPE
{ UNTIL,    &SbiParser::BadBlock,   N, Y, }, // UNTIL
{ WHILE,    &SbiParser::While,      N, Y, }, // WHILE
{ WEND,     &SbiParser::BadBlock,   N, Y, }, // WEND
{ WITH,     &SbiParser::With,       N, Y, }, // WITH
{ WRITE,    &SbiParser::Write,      N, Y, }, // WRITE

{ NIL, NULL, N, N }
};


#ifdef _MSC_VER
// 'this' : used in base member initializer list
#pragma warning( disable: 4355 )
#endif

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
    bExplicit = false;
    bClassModule = ( pm->GetModuleType() == com::sun::star::script::ModuleType::CLASS );
    OSL_TRACE("Parser - %s, bClassModule %d", rtl::OUStringToOString( pm->GetName(), RTL_TEXTENCODING_UTF8 ).getStr(), bClassModule );
    pPool    = &aPublics;
    for( short i = 0; i < 26; i++ )
        eDefTypes[ i ] = SbxVARIANT;    // no explicit default type

    aPublics.SetParent( &aGlobals );
    aGlobals.SetParent( &aRtlSyms );


    nGblChain = aGen.Gen( _JUMP, 0 );

    rTypeArray = new SbxArray; // array for user defined types
    rEnumArray = new SbxArray; // array for Enum types
    bVBASupportOn = pm->IsVBACompat();
    if ( bVBASupportOn )
        EnableCompatibility();

}


// part of the runtime-library?
SbiSymDef* SbiParser::CheckRTLForSym( const String& rSym, SbxDataType eType )
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

// close global chain

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

    // #29955 service the for-loop level
    if( eTok == FOR )
        aGen.IncForLevel();
}

void SbiParser::CloseBlock()
{
    if( pStack )
    {
        SbiParseStack* p = pStack;

        // #29955 service the for-loop level
        if( p->eExitTok == FOR )
            aGen.DecForLevel();

        aGen.BackChain( p->nChain );
        pStack = p->pNext;
        pWithVar = p->pWithVar;
        delete p;
    }
}

// EXIT ...

void SbiParser::Exit()
{
    SbiToken eTok = Next();
    for( SbiParseStack* p = pStack; p; p = p->pNext )
    {
        SbiToken eExitTok = p->eExitTok;
        if( eTok == eExitTok ||
            (eTok == PROPERTY && (eExitTok == GET || eExitTok == LET) ) )   // #i109051
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
        bAbort = sal_True;
    }
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
        // AB #33133: If no sub has been created before,
        // the global chain must be closed here!
        // AB #40689: Due to the new static-handling there
        // can be another nGblChain, so ask for it before.
        if( bNewGblDefs && nGblChain == 0 )
            nGblChain = aGen.Gen( _JUMP, 0 );
        return false;
    }


    if( IsEoln( eCurTok ) )
    {
        Next(); return true;
    }

    if( !bSingleLineIf && MayBeLabel( sal_True ) )
    {
        // is a label
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

    // end of parsing?
    if( eCurTok == eEndTok ||
        ( bVBASupportOn &&      // #i109075
          (eCurTok == ENDFUNC || eCurTok == ENDPROPERTY || eCurTok == ENDSUB) &&
          (eEndTok == ENDFUNC || eEndTok == ENDPROPERTY || eEndTok == ENDSUB) ) )
    {
        Next();
        if( eCurTok != NIL )
            aGen.Statement();
        return false;
    }

    // comment?
    if( eCurTok == REM )
    {
        Next(); return true;
    }

        // In vba it's possible to do Error.foobar ( even if it results in
    // a runtime error
        if ( eCurTok == _ERROR_ && IsVBASupportOn() ) // we probably need to define a subset of keywords where this madness applies e.g. if ( IsVBASupportOn() && SymbolCanBeRedined( eCurTok ) )
        {
            SbiTokenizer tokens( *(SbiTokenizer*)this );
            tokens.Next();
            if ( tokens.Peek()  == DOT )
            {
                eCurTok = SYMBOL;
        ePush = eCurTok;
            }
    }
    // if there's a symbol, it's either a variable (LET)
    // or a SUB-procedure (CALL without brackets)
    // DOT for assignments in the WITH-block: .A=5
    if( eCurTok == SYMBOL || eCurTok == DOT )
    {
        if( !pProc )
            Error( SbERR_EXPECTED, SUB );
        else
        {
            // for correct line and column...
            Next();
            Push( eCurTok );
            aGen.Statement();
                Symbol();
        }
    }
    else
    {
        Next();

        // statement parsers

        SbiStatement* p;
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
                // AB #41606/#40689: Due to the new static-handling there
                // can be another nGblChain, so ask for it before.
                if( bNewGblDefs && nGblChain == 0 &&
                    ( eCurTok == SUB || eCurTok == FUNCTION || eCurTok == PROPERTY ) )
                {
                    nGblChain = aGen.Gen( _JUMP, 0 );
                    bNewGblDefs = false;
                }
                // statement-opcode at the beginning of a sub, too, please
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

    // test for the statement's end -
    // might also be an ELSE, as there must not neccessary be a : before the ELSE!

    if( !IsEos() )
    {
        Peek();
        if( !IsEos() && eCurTok != ELSE )
        {
            // if the parsing has been aborted, jump over to the ":"
            Error( SbERR_UNEXPECTED, eCurTok );
            while( !IsEos() ) Next();
        }
    }
    // The parser aborts at the end, the
    // next token has not been fetched yet!
    return true;
}


SbiExprNode* SbiParser::GetWithVar()
{
    if( pWithVar )
        return pWithVar;

    SbiParseStack* p = pStack;
    while( p )
    {
        // LoopVar can at the moment only be for with
        if( p->pWithVar )
            return p->pWithVar;
        p = p->pNext;
    }
    return NULL;
}


// assignment or subroutine call

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
        String aRtlName = pDef->GetName();
        if( aRtlName.EqualsIgnoreCaseAscii("Mid") )
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
            // so it must be an assignment!
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

// assignments of an object-variable

void SbiParser::Set()
{
    SbiExpression aLvalue( this, SbLVALUE );
    SbxDataType eType = aLvalue.GetType();
    if( eType != SbxOBJECT && eType != SbxEMPTY && eType != SbxVARIANT )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );

    SbiToken eTok = Peek();
    if( eTok == NEW )
    {
        Next();
        String aStr;
        SbiSymDef* pTypeDef = new SbiSymDef( aStr );
        TypeDecl( *pTypeDef, sal_True );

        aLvalue.Gen();
        aGen.Gen( _CREATE, pDef->GetId(), pTypeDef->GetTypeId() );
        aGen.Gen( _SETCLASS, pDef->GetTypeId() );
    }
    else
    {
        SbiExpression aExpr( this );
        aLvalue.Gen();
        aExpr.Gen();
        // Its a good idea to distinguish between
        // set someting = another &
        // someting = another
        // ( its necessary for vba objects where set is object
        // specific and also doesn't involve processing default params )
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

// JSM 07.10.95
void SbiParser::LSet()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxSTRING )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    aGen.Gen( _LSET );
}

// JSM 07.10.95
void SbiParser::RSet()
{
    SbiExpression aLvalue( this, SbLVALUE );
    if( aLvalue.GetType() != SbxSTRING )
        Error( SbERR_INVALID_OBJECT );
    TestToken( EQ );
    SbiSymDef* pDef = aLvalue.GetRealVar();
    if( pDef && pDef->GetConstDef() )
        Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
    SbiExpression aExpr( this );
    aLvalue.Gen();
    aExpr.Gen();
    aGen.Gen( _RSET );
}

// DEFINT, DEFLNG, DEFSNG, DEFDBL, DEFSTR and so on

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

// STOP/SYSTEM

void SbiParser::Stop()
{
    aGen.Gen( _STOP );
    Peek();     // #35694: only Peek(), so that EOL is recognized in Single-Line-If
}

// IMPLEMENTS

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

    String aImplementedIface = aSym;
    Next();
    if( Peek() == DOT )
    {
        rtl::OUString aDotStr( '.' );
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
    bCompatible = sal_True;
}

// OPTION

void SbiParser::Option()
{
    switch( Next() )
    {
        case EXPLICIT:
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
            String aString = SbiTokenizer::Symbol(Next());
            if( !aString.EqualsIgnoreCaseAscii("Module") )
                Error( SbERR_EXPECTED, "Module" );
            break;
        }
        case COMPARE:
        {
            SbiToken eTok = Next();
            if( eTok == BINARY )
                bText = false;
            else if( eTok == SYMBOL && GetSym().equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("text")) )
                bText = true;
            else
                Error( SbERR_EXPECTED, "Text/Binary" );
            break;
        }
        case COMPATIBLE:
            EnableCompatibility();
            break;

        case CLASSMODULE:
            bClassModule = true;
            aGen.GetModule().SetModuleType( com::sun::star::script::ModuleType::CLASS );
            break;
        case VBASUPPORT: // Option VBASupport used to override the module mode ( in fact this must reset the mode
            if( Next() == NUMBER )
            {
                if ( nVal == 1 || nVal == 0 )
                {
                    bVBASupportOn = ( nVal == 1 );
                    if ( bVBASupportOn )
                        EnableCompatibility();
                    // if the module setting is different
                    // reset it to what the Option tells us
                    if ( bVBASupportOn != aGen.GetModule().IsVBACompat() )
                        aGen.GetModule().SetVBACompat( bVBASupportOn );
                    break;
                }
            }
            Error( SbERR_EXPECTED, "0/1" );
            break;
        default:
            Error( SbERR_BAD_OPTION, eCurTok );
    }
}

void addStringConst( SbiSymPool& rPool, const char* pSym, const String& rStr )
{
    SbiConstDef* pConst = new SbiConstDef( rtl::OUString::createFromAscii( pSym ) );
    pConst->SetType( SbxSTRING );
    pConst->Set( rStr );
    rPool.Add( pConst );
}

inline void addStringConst( SbiSymPool& rPool, const char* pSym, const char* pStr )
{
    addStringConst( rPool, pSym, rtl::OUString::createFromAscii( pStr ) );
}

void SbiParser::AddConstants( void )
{
    // #113063 Create constant RTL symbols
    addStringConst( aPublics, "vbCr", "\x0D" );
    addStringConst( aPublics, "vbCrLf", "\x0D\x0A" );
    addStringConst( aPublics, "vbFormFeed", "\x0C" );
    addStringConst( aPublics, "vbLf", "\x0A" );
#if defined(UNX)
    addStringConst( aPublics, "vbNewLine", "\x0A" );
#else
    addStringConst( aPublics, "vbNewLine", "\x0D\x0A" );
#endif
    addStringConst( aPublics, "vbNullString", "" );
    addStringConst( aPublics, "vbTab", "\x09" );
    addStringConst( aPublics, "vbVerticalTab", "\x0B" );

    // Force length 1 and make char 0 afterwards
    String aNullCharStr( rtl::OUString(" ") );
    aNullCharStr.SetChar( 0, 0 );
    addStringConst( aPublics, "vbNullChar", aNullCharStr );
}

// ERROR n

void SbiParser::ErrorStmnt()
{
    SbiExpression aPar( this );
    aPar.Gen();
    aGen.Gen( _ERROR );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
