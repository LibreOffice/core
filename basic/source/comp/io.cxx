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


#include <tools/stream.hxx>
#include "sbcomp.hxx"
#include "iosys.hxx"

// Test, ob ein I/O-Channel angegeben wurde

sal_Bool SbiParser::Channel( sal_Bool bAlways )
{
    sal_Bool bRes = sal_False;
    Peek();
    if( IsHash() )
    {
        SbiExpression aExpr( this );
        while( Peek() == COMMA || Peek() == SEMICOLON )
            Next();
        aExpr.Gen();
        aGen.Gen( _CHANNEL );
        bRes = sal_True;
    }
    else if( bAlways )
        Error( SbERR_EXPECTED, "#" );
    return bRes;
}

// Fuer PRINT und WRITE wird bei Objektvariablen versucht,
// die Default-Property anzusprechen.

// PRINT

void SbiParser::Print()
{
    sal_Bool bChan = Channel();
    // Die Ausdruecke zum Drucken:
    while( !bAbort )
    {
        if( !IsEoln( Peek() ) )
        {
            SbiExpression* pExpr = new SbiExpression( this );
            pExpr->Gen();
            delete pExpr;
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
    sal_Bool bChan = Channel();
    // Die Ausdruecke zum Drucken:
    while( !bAbort )
    {
        SbiExpression* pExpr = new SbiExpression( this );
        pExpr->Gen();
        delete pExpr;
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
        aInfo.m_aKeywordSymbol = String( RTL_CONSTASCII_USTRINGPARAM( "line" ) );
        aInfo.m_eSbxDataType = GetType();
        aInfo.m_eTok = SYMBOL;

        Symbol( &aInfo );
    }
}


// LINE INPUT [prompt], var$

void SbiParser::LineInput()
{
    Channel( sal_True );
    SbiExpression* pExpr = new SbiExpression( this, SbOPERAND );
    if( !pExpr->IsVariable() )
        Error( SbERR_VAR_EXPECTED );
    if( pExpr->GetType() != SbxVARIANT && pExpr->GetType() != SbxSTRING )
        Error( SbERR_CONVERSION );
    pExpr->Gen();
    aGen.Gen( _LINPUT );
    delete pExpr;
    aGen.Gen( _CHAN0 );     // ResetChannel() nicht mehr in StepLINPUT()
}

// INPUT

void SbiParser::Input()
{
    aGen.Gen( _RESTART );
    Channel( sal_True );
    SbiExpression* pExpr = new SbiExpression( this, SbOPERAND );
    while( !bAbort )
    {
        if( !pExpr->IsVariable() )
            Error( SbERR_VAR_EXPECTED );
        pExpr->Gen();
        aGen.Gen( _INPUT );
        if( Peek() == COMMA )
        {
            Next();
            delete pExpr;
            pExpr = new SbiExpression( this, SbOPERAND );
        }
        else break;
    }
    delete pExpr;
    aGen.Gen( _CHAN0 );     // ResetChannel() nicht mehr in StepINPUT()
}

// OPEN stringexpr FOR mode ACCCESS access mode AS Channel [Len=n]

void SbiParser::Open()
{
    SbiExpression aFileName( this );
    SbiToken eTok;
    TestToken( FOR );
    short nMode = 0;
    short nFlags = 0;
    switch( Next() )
    {
        case INPUT:
            nMode = STREAM_READ;  nFlags |= SBSTRM_INPUT; break;
        case OUTPUT:
            nMode = STREAM_WRITE | STREAM_TRUNC; nFlags |= SBSTRM_OUTPUT; break;
        case APPEND:
            nMode = STREAM_WRITE; nFlags |= SBSTRM_APPEND; break;
        case RANDOM:
            nMode = STREAM_READ | STREAM_WRITE; nFlags |= SBSTRM_RANDOM; break;
        case BINARY:
            nMode = STREAM_READ | STREAM_WRITE; nFlags |= SBSTRM_BINARY; break;
        default:
            Error( SbERR_SYNTAX );
    }
    if( Peek() == ACCESS )
    {
        Next();
        eTok = Next();
        // Nur STREAM_READ,STREAM_WRITE-Flags in nMode beeinflussen
        nMode &= ~(STREAM_READ | STREAM_WRITE);     // loeschen
        if( eTok == READ )
        {
            if( Peek() == WRITE )
            {
                Next();
                nMode |= (STREAM_READ | STREAM_WRITE);
            }
            else
                nMode |= STREAM_READ;
        }
        else if( eTok == WRITE )
            nMode |= STREAM_WRITE;
        else
            Error( SbERR_SYNTAX );
    }
    switch( Peek() )
    {
#ifdef SHARED
#undef SHARED
#define tmpSHARED
#endif
        case SHARED:
            Next(); nMode |= STREAM_SHARE_DENYNONE; break;
#ifdef tmpSHARED
#define SHARED
#undef tmpSHARED
#endif
        case LOCK:
            Next();
            eTok = Next();
            if( eTok == READ )
            {
                if( Peek() == WRITE ) Next(), nMode |= STREAM_SHARE_DENYALL;
                else nMode |= STREAM_SHARE_DENYREAD;
            }
            else if( eTok == WRITE )
                nMode |= STREAM_SHARE_DENYWRITE;
            else
                Error( SbERR_SYNTAX );
            break;
        default: break;
    }
    TestToken( AS );
    // Die Kanalnummer
    SbiExpression* pChan = new SbiExpression( this );
    if( !pChan )
        Error( SbERR_SYNTAX );
    SbiExpression* pLen = NULL;
    if( Peek() == SYMBOL )
    {
        Next();
        String aLen( aSym );
        if( aLen.EqualsIgnoreCaseAscii( "LEN" ) )
        {
            TestToken( EQ );
            pLen = new SbiExpression( this );
        }
    }
    if( !pLen ) pLen = new SbiExpression( this, 128, SbxINTEGER );
    // Der Stack fuer den OPEN-Befehl sieht wie folgt aus:
    // Blocklaenge
    // Kanalnummer
    // Dateiname
    pLen->Gen();
    if( pChan )
        pChan->Gen();
    aFileName.Gen();
    aGen.Gen( _OPEN, nMode, nFlags );
    delete pLen;
    delete pChan;
}

// NAME file AS file

void SbiParser::Name()
{
    // #i92642: Special handling to allow name as symbol
    if( Peek() == EQ )
    {
        aGen.Statement();

        KeywordSymbolInfo aInfo;
        aInfo.m_aKeywordSymbol = String( RTL_CONSTASCII_USTRINGPARAM( "name" ) );
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
