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

#include <hash.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <rtl/strbuf.hxx>

OString SvToken::GetTokenAsString() const
{
    OString aStr;
    switch( nType )
    {
        case SVTOKENTYPE::Empty:
            break;
        case SVTOKENTYPE::Comment:
            aStr = aString;
            break;
        case SVTOKENTYPE::Integer:
            aStr = OString::number(nLong);
            break;
        case SVTOKENTYPE::String:
            aStr = aString;
            break;
        case SVTOKENTYPE::Bool:
            aStr = bBool ? "TRUE" : "FALSE";
            break;
        case SVTOKENTYPE::Identifier:
            aStr = aString;
            break;
        case SVTOKENTYPE::Char:
            aStr = OString(cChar);
            break;
        case SVTOKENTYPE::EndOfFile:
        case SVTOKENTYPE::HashId:
            break;
    }

    return aStr;
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
    aStrTrue = OString("TRUE");
    aStrFalse = OString("FALSE");
    nLine       = nColumn = 0;
    nBufPos     = 0;
    nTabSize    = 4;
    nMaxPos     = 0;
    c           = GetNextChar();
    FillTokenList();
}

SvTokenStream::SvTokenStream( const OUString & rFileName )
    : pInStream( new SvFileStream( rFileName, STREAM_STD_READ | StreamMode::NOCREATE ) )
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
    aTokList.push_back(std::unique_ptr<SvToken>(pToken));
    do
    {
        if( !MakeToken( *pToken ) )
        {
            if (!aTokList.empty())
            {
                *pToken = SvToken();
                std::vector<std::unique_ptr<SvToken> >::const_iterator it = aTokList.begin();

                pToken->SetLine((*it)->GetLine());
                pToken->SetColumn((*it)->GetColumn());
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
            aTokList.push_back(std::unique_ptr<SvToken>(pToken));
        }
    }
    while( !pToken->IsEof() );
    pCurToken = aTokList.begin();
}

int SvTokenStream::GetNextChar()
{
    int nChar;
    while (aBufStr.getLength() <= nBufPos)
    {
        if (pInStream->ReadLine(aBufStr))
        {
            nLine++;
            nColumn = 0;
            nBufPos = 0;
        }
        else
        {
            aBufStr.clear();
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
    }

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

    return l;
}

bool SvTokenStream::MakeToken( SvToken & rToken )
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
    while( 0 == c && !IsEof() && ( SVSTREAM_OK == pInStream->GetError() ) );

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
            rToken.nType    = SVTOKENTYPE::Comment;
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
                            return false;
                    }
                    else
                        c = GetFastNextChar();
                }
                c = GetFastNextChar();
            }
            while( '/' != c && !IsEof() && ( SVSTREAM_OK == pInStream->GetError() ) );
            if( IsEof() || ( SVSTREAM_OK != pInStream->GetError() ) )
                return false;
            c = GetNextChar();
            rToken.nType = SVTOKENTYPE::Comment;
            CalcColumn();
        }
        else
        {
            rToken.nType = SVTOKENTYPE::Char;
            rToken.cChar = (char)c1;
        }
    }
    else if( c == '"' )
    {
        OStringBuffer aStr;
        bool bDone = false;
        while( !bDone && !IsEof() && c )
        {
            c = GetFastNextChar();
            if( '\0' == c )
            {
                // read strings beyond end of line
                aStr.append('\n');
                c = GetNextChar();
                if( IsEof() )
                    return false;
            }
            if( c == '"' )
            {
                c = GetFastNextChar();
                bDone = true;
            }
            else
                aStr.append(static_cast<char>(c));
        }
        if( IsEof() || ( SVSTREAM_OK != pInStream->GetError() ) )
            return false;
        rToken.nType   = SVTOKENTYPE::String;
        rToken.aString = aStr.makeStringAndClear();
    }
    else if( isdigit( c ) )
    {
        rToken.nType = SVTOKENTYPE::Integer;
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
            rToken.nType = SVTOKENTYPE::Bool;
            rToken.bBool = true;
        }
        else if( aStr.equalsIgnoreAsciiCase( aStrFalse ) )
        {
            rToken.nType = SVTOKENTYPE::Bool;
            rToken.bBool = false;
        }
        else
        {
            sal_uInt32 nHashId;
            if( GetIdlApp().pHashTable->Test( aStr, &nHashId ) )
                rToken.SetHash( GetIdlApp().pHashTable->Get( nHashId ) );
            else
            {
                rToken.nType   = SVTOKENTYPE::Identifier;
                rToken.aString = aStr;
            }
        }
    }
    else if( IsEof() )
    {
        rToken.nType = SVTOKENTYPE::EndOfFile;
    }
    else
    {
        rToken.nType = SVTOKENTYPE::Char;
        rToken.cChar = (char)c;
        c = GetFastNextChar();
    }
    rToken.SetLine( nLastLine );
    rToken.SetColumn( nLastColumn );
    return pInStream->GetError() == SVSTREAM_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
