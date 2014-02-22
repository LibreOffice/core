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
#include "iosys.hxx"



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
        Error( SbERR_EXPECTED, "#" );
    return bRes;
}




void SbiParser::Print()
{
    bool bChan = Channel();

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



void SbiParser::Write()
{
    bool bChan = Channel();

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



void SbiParser::Line()
{
    
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




void SbiParser::LineInput()
{
    Channel( true );
    SbiExpression* pExpr = new SbiExpression( this, SbOPERAND );
    if( !pExpr->IsVariable() )
        Error( SbERR_VAR_EXPECTED );
    if( pExpr->GetType() != SbxVARIANT && pExpr->GetType() != SbxSTRING )
        Error( SbERR_CONVERSION );
    pExpr->Gen();
    aGen.Gen( _LINPUT );
    delete pExpr;
    aGen.Gen( _CHAN0 );     
}



void SbiParser::Input()
{
    aGen.Gen( _RESTART );
    Channel( true );
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
    aGen.Gen( _CHAN0 );
}



void SbiParser::Open()
{
    bInStatement = true;
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
        
        nMode &= ~(STREAM_READ | STREAM_WRITE);     
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
    
    SbiExpression* pChan = new SbiExpression( this );
    if( !pChan )
        Error( SbERR_SYNTAX );
    SbiExpression* pLen = NULL;
    if( Peek() == SYMBOL )
    {
        Next();
        if( aSym.equalsIgnoreAsciiCase("LEN") )
        {
            TestToken( EQ );
            pLen = new SbiExpression( this );
        }
    }
    if( !pLen ) pLen = new SbiExpression( this, 128, SbxINTEGER );
    
    
    
    
    pLen->Gen();
    if( pChan )
        pChan->Gen();
    aFileName.Gen();
    aGen.Gen( _OPEN, nMode, nFlags );
    delete pLen;
    delete pChan;
    bInStatement = false;
}



void SbiParser::Name()
{
    
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
