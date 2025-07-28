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
#include <hash.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <rtl/strbuf.hxx>
#include<rtl/character.hxx>

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

void SvTokenStream::InitCtor()
{
    aStrTrue = "TRUE"_ostr;
    aStrFalse = "FALSE"_ostr;
    nLine       = nColumn = 0;
    nBufPos     = 0;
    nMaxPos     = 0;
    c           = GetNextChar();
    FillTokenList();
}

SvTokenStream::SvTokenStream( const OUString & rFileName )
    : pInStream( new SvFileStream( rFileName, StreamMode::STD_READ ) )
    , aFileName( rFileName )
{
    InitCtor();
}

SvTokenStream::~SvTokenStream()
{
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
#if defined __GNUC__ && !defined __clang__ && __GNUC__ >= 12 && __GNUC__ <= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
                *pToken = SvToken();
#if defined __GNUC__ && !defined __clang__ && __GNUC__ >= 12 && __GNUC__ <= 16
#pragma GCC diagnostic pop
#endif
                std::vector<std::unique_ptr<SvToken> >::const_iterator it = aTokList.begin();

                pToken->SetLine((*it)->GetLine());
                pToken->SetColumn((*it)->GetColumn());
            }
            break;
        }
        else if( pToken->IsComment() )
        {
#if defined __GNUC__ && !defined __clang__ && __GNUC__ >= 12 && __GNUC__ <= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
            *pToken = SvToken();
#if defined __GNUC__ && !defined __clang__ && __GNUC__ >= 12 && __GNUC__ <= 16
#pragma GCC diagnostic pop
#endif
        }
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

char SvTokenStream::GetNextChar()
{
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
            aBufStr.setLength(0);
            nColumn = 0;
            nBufPos = 0;
            return '\0';
        }
    }
    char nChar = aBufStr[nBufPos++];
    nColumn += nChar == '\t' ? nTabSize : 1;
    return nChar;
}

sal_uInt64 SvTokenStream::GetNumber()
{
    sal_uInt64   l = 0;
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
        while( rtl::isAsciiHexDigit( static_cast<unsigned char>(c) ) )
        {
            if( rtl::isAsciiDigit( static_cast<unsigned char>(c) ) )
                l = l * nLog + (c - '0');
            else
                l = l * nLog
                    + (rtl::toAsciiUpperCase( static_cast<unsigned char>(c) )
                       - 'A' + 10 );
            c = GetFastNextChar();
        }
    }
    else
    {
        while( rtl::isAsciiDigit( static_cast<unsigned char>(c) ) || 'x' == c )
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
        while( rtl::isAsciiWhiteSpace( static_cast<unsigned char>(c) )
               || 26 == c )
        {
            c = GetFastNextChar();
            nColumn += c == '\t' ? nTabSize : 1;
        }
    }
    while( 0 == c && !IsEof() && ( ERRCODE_NONE == pInStream->GetError() ) );

    sal_uInt64 nLastLine     = nLine;
    sal_uInt64 nLastColumn   = nColumn;
    // comment
    if( '/' == c )
    {
        // time optimization, no comments
        char c1 = c;
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
            while( '/' != c && !IsEof() && ( ERRCODE_NONE == pInStream->GetError() ) );
            if( IsEof() || ( ERRCODE_NONE != pInStream->GetError() ) )
                return false;
            c = GetNextChar();
            rToken.nType = SVTOKENTYPE::Comment;
            CalcColumn();
        }
        else
        {
            rToken.nType = SVTOKENTYPE::Char;
            rToken.cChar = c1;
        }
    }
    else if( c == '"' )
    {
        OStringBuffer aStr(128);
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
                aStr.append(c);
        }
        if( IsEof() || ( ERRCODE_NONE != pInStream->GetError() ) )
            return false;
        rToken.nType   = SVTOKENTYPE::String;
        rToken.aString = aStr.makeStringAndClear();
    }
    else if( rtl::isAsciiDigit( static_cast<unsigned char>(c) ) )
    {
        rToken.nType = SVTOKENTYPE::Integer;
        rToken.nLong = GetNumber();

    }
    else if( rtl::isAsciiAlpha (static_cast<unsigned char>(c)) || (c == '_') )
    {
        OStringBuffer aBuf(64);
        while( rtl::isAsciiAlphanumeric( static_cast<unsigned char>(c) )
               || c == '_' || c == ':')
        {
            aBuf.append(c);
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
        rToken.cChar = c;
        c = GetFastNextChar();
    }
    rToken.SetLine( nLastLine );
    rToken.SetColumn( nLastColumn );
    return pInStream->GetError() == ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
