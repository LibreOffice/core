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
#include "precompiled_idl.hxx"


#include <ctype.h>
#include <stdio.h>

#include <char.hxx>
#include <hash.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <tools/bigint.hxx>

ByteString SvToken::GetTokenAsString() const
{
    ByteString aStr;
    switch( nType )
    {
        case SVTOKEN_EMPTY:
            break;
        case SVTOKEN_COMMENT:
            aStr = aString;
            break;
        case SVTOKEN_INTEGER:
            aStr = ByteString::CreateFromInt64(nLong);
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
            aStr = cChar;
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
#ifdef DOS
    SetCharSet( CHARSET_ANSI );
#else
    SetCharSet( gsl_getSystemTextEncoding() );
#endif
    aStrTrue    = "TRUE";
    aStrFalse   = "FALSE";
    nLine       = nColumn = 0;
    nBufPos     = 0;
    nTabSize    = 4;
    pCurToken   = NULL;
    nMaxPos     = 0;
    c           = GetNextChar();
    FillTokenList();
}

SvTokenStream::SvTokenStream( const String & rFileName )
    : pInStream( new SvFileStream( rFileName, STREAM_STD_READ | STREAM_NOCREATE ) )
    , rInStream( *pInStream )
    , aFileName( rFileName )
    , aTokList( 0x8000, 0x8000 )
{
    InitCtor();
}

SvTokenStream::SvTokenStream( SvStream & rStream, const String & rFileName )
    : pInStream( NULL )
    , rInStream( rStream )
    , aFileName( rFileName )
    , aTokList( 0x8000, 0x8000 )
{
    InitCtor();
}

SvTokenStream::~SvTokenStream()
{
    delete pInStream;
    SvToken * pTok = aTokList.Last();
    while( pTok )
    {
        delete pTok;
        pTok = aTokList.Prev();
    }
}

void SvTokenStream::FillTokenList()
{
    SvToken * pToken = new SvToken();
    aTokList.Insert( pToken, LIST_APPEND );
    do
    {
        if( !MakeToken( *pToken ) )
        {
            SvToken * p = aTokList.Prev();
            *pToken = SvToken();
            if( p )
            {
                pToken->SetLine( p->GetLine() );
                pToken->SetColumn( p->GetColumn() );
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
            aTokList.Insert( pToken, LIST_APPEND );
        }
    }
    while( !pToken->IsEof() );
    pCurToken = aTokList.First();
}

void SvTokenStream::SetCharSet( CharSet nSet )
{
    nCharSet = nSet;

#ifdef DOS
    pCharTab = SvChar::GetTable( nSet, CHARSET_ANSI );
#else
    pCharTab = SvChar::GetTable( nSet, gsl_getSystemTextEncoding() );
#endif
}

int SvTokenStream::GetNextChar()
{
    int nChar;
    if( (int)aBufStr.Len() < nBufPos )
    {
        if( rInStream.ReadLine( aBufStr ) )
        {
            nLine++;
            nColumn = 0;
            nBufPos = 0;
        }
        else
        {
            aBufStr.Erase();
            nColumn = 0;
            nBufPos = 0;
            return '\0';
        }
    }
    nChar = aBufStr.GetChar( (USHORT)nBufPos++ );
    nColumn += nChar == '\t' ? nTabSize : 1;
    return nChar;
}

ULONG SvTokenStream::GetNumber()
{
    ULONG   l = 0;
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

BOOL SvTokenStream::MakeToken( SvToken & rToken )
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

    ULONG nLastLine     = nLine;
    ULONG nLastColumn   = nColumn;
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
                            return FALSE;
                    }
                    else
                        c = GetFastNextChar();
                }
                c = GetFastNextChar();
            }
            while( '/' != c && !IsEof() && ( SVSTREAM_OK == rInStream.GetError() ) );
            if( IsEof() || ( SVSTREAM_OK != rInStream.GetError() ) )
                return FALSE;
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
        ByteString          aStr;
        BOOL bDone = FALSE;
        while( !bDone && !IsEof() && c )
        {
            c = GetFastNextChar();
            if( '\0' == c )
            {
                // read strings beyond end of line
                aStr += '\n';
                c = GetNextChar();
                if( IsEof() )
                    return FALSE;
            }
            if( c == '"' )
            {
                c = GetFastNextChar();
                if( c == '"' )
                {
                    aStr += '"';
                    aStr += '"';
                }
                else
                    bDone = TRUE;
            }
            else if( c == '\\' )
            {
                aStr += '\\';
                c = GetFastNextChar();
                if( c )
                    aStr += (char)c;
            }
            else
                aStr += (char)c;
        }
        if( IsEof() || ( SVSTREAM_OK != rInStream.GetError() ) )
            return FALSE;
        char * pStr = (char *)aStr.GetBuffer();
        while( *pStr )
        {
            *pStr = pCharTab[ (unsigned char)*pStr ];
            pStr++;
        };
        rToken.nType   = SVTOKEN_STRING;
        rToken.aString = aStr;
    }
    else if( isdigit( c ) )
    {
        rToken.nType = SVTOKEN_INTEGER;
        rToken.nLong = GetNumber();

    }
    else if( isalpha (c) || (c == '_') )
    {
        ByteString aStr;

        while( isalnum( c ) || c == '_' )
        {
            aStr += (char)c;
            c = GetFastNextChar();
        }
        if( aStr.EqualsIgnoreCaseAscii( aStrTrue ) )
        {
            rToken.nType = SVTOKEN_BOOL;
            rToken.bBool = TRUE;
        }
        else if( aStr.EqualsIgnoreCaseAscii( aStrFalse ) )
        {
            rToken.nType = SVTOKEN_BOOL;
            rToken.bBool = FALSE;
        }
        else
        {
            UINT32 nHashId;
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
