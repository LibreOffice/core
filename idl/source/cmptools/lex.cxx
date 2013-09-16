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

#include <ctype.h>
#include <stdio.h>

#include <hash.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <rtl/strbuf.hxx>

OString SvToken::GetTokenAsString() const
{
    OString aStr;
    switch( nType )
    {
        case SVTOKEN_EMPTY:
            break;
        case SVTOKEN_COMMENT:
            aStr = aString;
            break;
        case SVTOKEN_INTEGER:
            aStr = OString::number(nLong);
            break;
        case SVTOKEN_STRING:
            aStr = aString;
            break;
        case SVTOKEN_BOOL:
            aStr = bBool ? "TRUE" : "FALSE";
            break;
        case SVTOKEN_IDENTIFIER:
            aStr = aString;
            break;
        case SVTOKEN_CHAR:
            aStr = OString(cChar);
            break;
        case SVTOKEN_RTTIBASE:
            aStr = "RTTIBASE";
            break;
        case SVTOKEN_EOF:
        case SVTOKEN_HASHID:
            break;
    }

    return aStr;
}

SvToken::SvToken( const SvToken & rObj )
{
    nLine = rObj.nLine;
    nColumn = rObj.nColumn;
    nType = rObj.nType;
    aString = rObj.aString;
    nLong = rObj.nLong;
}

SvToken & SvToken::operator = ( const SvToken & rObj )
{
    if( this != &rObj )
    {
        nLine = rObj.nLine;
        nColumn = rObj.nColumn;
        nType = rObj.nType;
        aString = rObj.aString;
        nLong = rObj.nLong;
    }
    return *this;
}

void SvTokenStream::InitCtor()
{
    aStrTrue = OString(RTL_CONSTASCII_STRINGPARAM("TRUE"));
    aStrFalse = OString(RTL_CONSTASCII_STRINGPARAM("FALSE"));
    nLine       = nColumn = 0;
    nBufPos     = 0;
    nTabSize    = 4;
    nMaxPos     = 0;
    c           = GetNextChar();
    FillTokenList();
}

SvTokenStream::SvTokenStream( const OUString & rFileName )
    : pInStream( new SvFileStream( rFileName, STREAM_STD_READ | STREAM_NOCREATE ) )
    , rInStream( *pInStream )
    , aFileName( rFileName )
{
    InitCtor();
}

SvTokenStream::SvTokenStream( SvStream & rStream, const OUString & rFileName )
    : pInStream( NULL )
    , rInStream( rStream )
    , aFileName( rFileName )
{
    InitCtor();
}

SvTokenStream::~SvTokenStream()
{
    delete pInStream;
}

void SvTokenStream::FillTokenList()
{
    SvToken * pToken = new SvToken();
    aTokList.push_back(pToken);
    do
    {
        if( !MakeToken( *pToken ) )
        {
            if (!aTokList.empty())
            {
                *pToken = SvToken();
                boost::ptr_vector<SvToken>::const_iterator it = aTokList.begin();

                pToken->SetLine(it->GetLine());
                pToken->SetColumn(it->GetColumn());
            }
            break;
        }
        else if( pToken->IsComment() )
            *pToken = SvToken();
        else if( pToken->IsEof() )
            break;
        else
        {
            pToken = new SvToken();
            aTokList.push_back(pToken);
        }
    }
    while( !pToken->IsEof() );
    pCurToken = aTokList.begin();
}

int SvTokenStream::GetNextChar()
{
    int nChar;
    if( aBufStr.getLength() < nBufPos )
    {
        if( rInStream.ReadLine( aBufStr ) )
        {
            nLine++;
            nColumn = 0;
            nBufPos = 0;
        }
        else
        {
            aBufStr = OString();
            nColumn = 0;
            nBufPos = 0;
            return '\0';
        }
    }
    nChar = aBufStr[nBufPos++];
    nColumn += nChar == '\t' ? nTabSize : 1;
    return nChar;
}

sal_uLong SvTokenStream::GetNumber()
{
    sal_uLong   l = 0;
    short   nLog = 10;

    if( '0' == c )
    {
        c = GetFastNextChar();
        if( 'x' == c )
        {
            nLog = 16;
            c = GetFastNextChar();
        }
    };

    if( nLog == 16 )
    {
        while( isxdigit( c ) )
        {
            if( isdigit( c ) )
                l = l * nLog + (c - '0');
            else
                l = l * nLog + (toupper( c ) - 'A' + 10 );
            c = GetFastNextChar();
        }
    }
    else
    {
        while( isdigit( c ) || 'x' == c )
        {
            l = l * nLog + (c - '0');
            c = GetFastNextChar();
        }
    }

    return( l );
}

sal_Bool SvTokenStream::MakeToken( SvToken & rToken )
{
    do
    {
        if( 0 == c )
            c = GetNextChar();
        // skip whitespace
        while( isspace( c ) || 26 == c )
        {
            c = GetFastNextChar();
            nColumn += c == '\t' ? nTabSize : 1;
        }
    }
    while( 0 == c && !IsEof() && ( SVSTREAM_OK == rInStream.GetError() ) );

    sal_uLong nLastLine     = nLine;
    sal_uLong nLastColumn   = nColumn;
    // comment
    if( '/' == c )
    {
        // time optimization, no comments
        int c1 = c;
        c = GetFastNextChar();
        if( '/' == c )
        {
            while( '\0' != c )
            {
                c = GetFastNextChar();
            }
            c = GetNextChar();
            rToken.nType    = SVTOKEN_COMMENT;
        }
        else if( '*' == c )
        {
            c = GetFastNextChar();
            do
            {
                while( '*' != c )
                {
                    if( '\0' == c )
                    {
                        c = GetNextChar();
                        if( IsEof() )
                            return sal_False;
                    }
                    else
                        c = GetFastNextChar();
                }
                c = GetFastNextChar();
            }
            while( '/' != c && !IsEof() && ( SVSTREAM_OK == rInStream.GetError() ) );
            if( IsEof() || ( SVSTREAM_OK != rInStream.GetError() ) )
                return sal_False;
            c = GetNextChar();
            rToken.nType = SVTOKEN_COMMENT;
            CalcColumn();
        }
        else
        {
            rToken.nType = SVTOKEN_CHAR;
            rToken.cChar = (char)c1;
        }
    }
    else if( c == '"' )
    {
        OStringBuffer aStr;
        sal_Bool bDone = sal_False;
        while( !bDone && !IsEof() && c )
        {
            c = GetFastNextChar();
            if( '\0' == c )
            {
                // read strings beyond end of line
                aStr.append('\n');
                c = GetNextChar();
                if( IsEof() )
                    return sal_False;
            }
            if( c == '"' )
            {
                c = GetFastNextChar();
                if( c == '"' )
                {
                    aStr.append('"');
                    aStr.append('"');
                }
                else
                    bDone = sal_True;
            }
            else if( c == '\\' )
            {
                aStr.append('\\');
                c = GetFastNextChar();
                if( c )
                    aStr.append(static_cast<char>(c));
            }
            else
                aStr.append(static_cast<char>(c));
        }
        if( IsEof() || ( SVSTREAM_OK != rInStream.GetError() ) )
            return sal_False;
        rToken.nType   = SVTOKEN_STRING;
        rToken.aString = aStr.makeStringAndClear();
    }
    else if( isdigit( c ) )
    {
        rToken.nType = SVTOKEN_INTEGER;
        rToken.nLong = GetNumber();

    }
    else if( isalpha (c) || (c == '_') )
    {
        OStringBuffer aBuf;
        while( isalnum( c ) || c == '_' )
        {
            aBuf.append(static_cast<char>(c));
            c = GetFastNextChar();
        }
        OString aStr = aBuf.makeStringAndClear();
        if( aStr.equalsIgnoreAsciiCase( aStrTrue ) )
        {
            rToken.nType = SVTOKEN_BOOL;
            rToken.bBool = sal_True;
        }
        else if( aStr.equalsIgnoreAsciiCase( aStrFalse ) )
        {
            rToken.nType = SVTOKEN_BOOL;
            rToken.bBool = sal_False;
        }
        else
        {
            sal_uInt32 nHashId;
            if( IDLAPP->pHashTable->Test( aStr, &nHashId ) )
                rToken.SetHash( IDLAPP->pHashTable->Get( nHashId ) );
            else
            {
                rToken.nType   = SVTOKEN_IDENTIFIER;
                rToken.aString = aStr;
            }
        }
    }
    else if( IsEof() )
    {
        rToken.nType = SVTOKEN_EOF;
    }
    else
    {
        rToken.nType = SVTOKEN_CHAR;
        rToken.cChar = (char)c;
        c = GetFastNextChar();
    }
    rToken.SetLine( nLastLine );
    rToken.SetColumn( nLastColumn );
    return rInStream.GetError() == SVSTREAM_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
