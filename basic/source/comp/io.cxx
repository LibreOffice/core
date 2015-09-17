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

#include "parser.hxx"
#include "iosys.hxx"
#include <memory>

// test if there's an I/O channel

bool SbiParser::Channel( bool bAlways )
{
    bool bRes = false;
    Peek();
    if( IsHash() )
    {
        SbiExpression aExpr( this );
        while( Peek() == COMMA || Peek() == SEMICOLON )
            Next();
        aExpr.Gen();
        aGen.Gen( _CHANNEL );
        bRes = true;
    }
    else if( bAlways )
        Error( ERRCODE_BASIC_EXPECTED, "#" );
    return bRes;
}

// it's tried that at object variables the Default-
// Property is addressed for PRINT and WRITE

void SbiParser::Print()
{
    bool bChan = Channel();

    while( !bAbort )
    {
        if( !IsEoln( Peek() ) )
        {
            std::unique_ptr<SbiExpression> pExpr(new SbiExpression( this ));
            pExpr->Gen();
            pExpr.reset();
            Peek();
            aGen.Gen( eCurTok == COMMA ? _PRINTF : _BPRINT );
        }
        if( eCurTok == COMMA || eCurTok == SEMICOLON )
        {
            Next();
            if( IsEoln( Peek() ) ) break;
        }
        else
        {
            aGen.Gen( _PRCHAR, '\n' );
            break;
        }
    }
    if( bChan )
        aGen.Gen( _CHAN0 );
}

// WRITE #chan, expr, ...

void SbiParser::Write()
{
    bool bChan = Channel();

    while( !bAbort )
    {
        std::unique_ptr<SbiExpression> pExpr(new SbiExpression( this ));
        pExpr->Gen();
        pExpr.reset();
        aGen.Gen( _BWRITE );
        if( Peek() == COMMA )
        {
            aGen.Gen( _PRCHAR, ',' );
            Next();
            if( IsEoln( Peek() ) ) break;
        }
        else
        {
            aGen.Gen( _PRCHAR, '\n' );
            break;
        }
    }
    if( bChan )
        aGen.Gen( _CHAN0 );
}


// #i92642 Handle LINE keyword outside ::Next()
void SbiParser::Line()
{
    // #i92642: Special handling to allow name as symbol
    if( Peek() == INPUT )
    {
        Next();
        LineInput();
    }
    else
    {
        aGen.Statement();

        KeywordSymbolInfo aInfo;
        aInfo.m_aKeywordSymbol = "line";
        aInfo.m_eSbxDataType = GetType();
        aInfo.m_eTok = SYMBOL;

        Symbol( &aInfo );
    }
}


// LINE INPUT [prompt], var$

void SbiParser::LineInput()
{
    Channel( true );
    std::unique_ptr<SbiExpression> pExpr(new SbiExpression( this, SbOPERAND ));
    if( !pExpr->IsVariable() )
        Error( ERRCODE_BASIC_VAR_EXPECTED );
    if( pExpr->GetType() != SbxVARIANT && pExpr->GetType() != SbxSTRING )
        Error( ERRCODE_BASIC_CONVERSION );
    pExpr->Gen();
    aGen.Gen( _LINPUT );
    pExpr.reset();
    aGen.Gen( _CHAN0 );     // ResetChannel() not in StepLINPUT() anymore
}

// INPUT

void SbiParser::Input()
{
    aGen.Gen( _RESTART );
    Channel( true );
    std::unique_ptr<SbiExpression> pExpr(new SbiExpression( this, SbOPERAND ));
    while( !bAbort )
    {
        if( !pExpr->IsVariable() )
            Error( ERRCODE_BASIC_VAR_EXPECTED );
        pExpr->Gen();
        aGen.Gen( _INPUT );
        if( Peek() == COMMA )
        {
            Next();
            pExpr.reset(new SbiExpression( this, SbOPERAND ));
        }
        else break;
    }
    pExpr.reset();
    aGen.Gen( _CHAN0 );
}

// OPEN stringexpr FOR mode ACCESS access mode AS Channel [Len=n]

void SbiParser::Open()
{
    bInStatement = true;
    SbiExpression aFileName( this );
    SbiToken eTok;
    TestToken( FOR );
    StreamMode nMode = StreamMode::NONE;
    SbiStreamFlags nFlags = SbiStreamFlags::NONE;
    switch( Next() )
    {
        case INPUT:
            nMode = StreamMode::READ;  nFlags |= SbiStreamFlags::Input; break;
        case OUTPUT:
            nMode = StreamMode::WRITE | StreamMode::TRUNC; nFlags |= SbiStreamFlags::Output; break;
        case APPEND:
            nMode = StreamMode::WRITE; nFlags |= SbiStreamFlags::Append; break;
        case RANDOM:
            nMode = StreamMode::READ | StreamMode::WRITE; nFlags |= SbiStreamFlags::Random; break;
        case BINARY:
            nMode = StreamMode::READ | StreamMode::WRITE; nFlags |= SbiStreamFlags::Binary; break;
        default:
            Error( ERRCODE_BASIC_SYNTAX );
    }
    if( Peek() == ACCESS )
    {
        Next();
        eTok = Next();
        // influence only READ,WRITE-Flags in nMode
        nMode &= ~StreamMode(StreamMode::READ | StreamMode::WRITE);     // delete
        if( eTok == READ )
        {
            if( Peek() == WRITE )
            {
                Next();
                nMode |= (StreamMode::READ | StreamMode::WRITE);
            }
            else
                nMode |= StreamMode::READ;
        }
        else if( eTok == WRITE )
            nMode |= StreamMode::WRITE;
        else
            Error( ERRCODE_BASIC_SYNTAX );
    }
    switch( Peek() )
    {
#ifdef SHARED
#undef SHARED
#define tmpSHARED
#endif
        case SHARED:
            Next(); nMode |= StreamMode::SHARE_DENYNONE; break;
#ifdef tmpSHARED
#define SHARED
#undef tmpSHARED
#endif
        case LOCK:
            Next();
            eTok = Next();
            if( eTok == READ )
            {
                if( Peek() == WRITE ) Next(), nMode |= StreamMode::SHARE_DENYALL;
                else nMode |= StreamMode::SHARE_DENYREAD;
            }
            else if( eTok == WRITE )
                nMode |= StreamMode::SHARE_DENYWRITE;
            else
                Error( ERRCODE_BASIC_SYNTAX );
            break;
        default: break;
    }
    TestToken( AS );
    // channel number
    std::unique_ptr<SbiExpression> pChan(new SbiExpression( this ));
    if( !pChan )
        Error( ERRCODE_BASIC_SYNTAX );
    std::unique_ptr<SbiExpression> pLen;
    if( Peek() == SYMBOL )
    {
        Next();
        if( aSym.equalsIgnoreAsciiCase("LEN") )
        {
            TestToken( EQ );
            pLen.reset(new SbiExpression( this ));
        }
    }
    if( !pLen ) pLen.reset(new SbiExpression( this, 128, SbxINTEGER ));
    // the stack for the OPEN command looks as follows:
    // block length
    // channel number
    // file name
    pLen->Gen();
    if( pChan )
        pChan->Gen();
    aFileName.Gen();
    aGen.Gen( _OPEN, static_cast<sal_uInt32>(nMode), static_cast<sal_uInt32>(nFlags) );
    bInStatement = false;
}

// NAME file AS file

void SbiParser::Name()
{
    // #i92642: Special handling to allow name as symbol
    if( Peek() == EQ )
    {
        aGen.Statement();

        KeywordSymbolInfo aInfo;
        aInfo.m_aKeywordSymbol = "name";
        aInfo.m_eSbxDataType = GetType();
        aInfo.m_eTok = SYMBOL;

        Symbol( &aInfo );
        return;
    }
    SbiExpression aExpr1( this );
    TestToken( AS );
    SbiExpression aExpr2( this );
    aExpr1.Gen();
    aExpr2.Gen();
    aGen.Gen( _RENAME );
}

// CLOSE [n,...]

void SbiParser::Close()
{
    Peek();
    if( IsEoln( eCurTok ) )
        aGen.Gen( _CLOSE, 0 );
    else
    for( ;; )
    {
        SbiExpression aExpr( this );
        while( Peek() == COMMA || Peek() == SEMICOLON )
            Next();
        aExpr.Gen();
        aGen.Gen( _CHANNEL );
        aGen.Gen( _CLOSE, 1 );

        if( IsEoln( Peek() ) )
            break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
