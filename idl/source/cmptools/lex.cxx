/*************************************************************************
 *
 *  $RCSfile: lex.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <ctype.h>
#include <stdio.h>

#include <char.hxx>
#include <hash.hxx>
#include <lex.hxx>
#include <globals.hxx>

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#pragma hdrstop

/****************** SvToken **********************************************/
/*************************************************************************
|*
|*    SvToken::Print()
|*
|*    Beschreibung
|*
*************************************************************************/
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
            aStr = nLong;
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
            aStr = "RTTIBASE";//(ULONG)pComplexObj;
            break;
        case SVTOKEN_EOF:
            break;
    }

    return aStr;
}

/*************************************************************************
|*
|*    SvToken::Print()
|*
|*    Beschreibung
|*
*************************************************************************/
ByteString SvToken::Print() const
{
    ByteString aStr;
    aStr += "Line = ";
    aStr += nLine;
    aStr += ", Column = ";
    aStr += nColumn;
    aStr += ", ";
    switch( nType )
    {
        case SVTOKEN_EMPTY:
            aStr += "Empty";
            break;
        case SVTOKEN_COMMENT:
            aStr += "Comment = ";
            break;
        case SVTOKEN_INTEGER:
            aStr += "Integer = ";
            break;
        case SVTOKEN_STRING:
            aStr += "ByteString = ";
            break;
        case SVTOKEN_BOOL:
            aStr += "Bool = ";
            break;
        case SVTOKEN_IDENTIFIER:
            aStr += "Identifier = ";
            break;
        case SVTOKEN_CHAR:
            aStr += "char = ";
            break;
        case SVTOKEN_RTTIBASE:
            aStr += "SvRttiBase = ";
            break;
        case SVTOKEN_EOF:
            aStr += "end of file";
            break;
    }
    aStr += GetTokenAsString();

    return aStr;
}

/*************************************************************************
|*
|*    SvToken::SvToken()
|*
|*    Beschreibung
|*
*************************************************************************/
SvToken::SvToken( const SvToken & rObj )
{
    nLine = rObj.nLine;
    nColumn = rObj.nColumn;
    nType = rObj.nType;
    aString = rObj.aString;
/*
    if( SVTOKEN_RTTIBASE = nType )
    {
        pComplexObj = rObj.pComplexObj;
        pComplexObj->AddRef();
    }
    else
*/
        nLong = rObj.nLong;
}

/*************************************************************************
|*
|*    SvToken::operator = ()
|*
|*    Beschreibung
|*
*************************************************************************/
SvToken & SvToken::operator = ( const SvToken & rObj )
{
    if( this != &rObj )
    {
/*
        if( SVTOKEN_RTTIBASE = nType )
            pComplexObj->ReleaseRef();
*/
        nLine = rObj.nLine;
        nColumn = rObj.nColumn;
        nType = rObj.nType;
        aString = rObj.aString;
/*
        if( SVTOKEN_RTTIBASE = nType )
        {
            pComplexObj = rObj.pComplexObj;
            pComplexObj->AddRef();
        }
        else
*/
            nLong = rObj.nLong;
    }
    return *this;
}

/****************** SvTokenStream ****************************************/
/*************************************************************************
|*    SvTokenStream::InitCtor()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvTokenStream::SvTokenStream()
|*
|*    Beschreibung
*************************************************************************/
SvTokenStream::SvTokenStream( const String & rFileName )
    : aFileName( rFileName )
    , pInStream( new SvFileStream( rFileName, STREAM_STD_READ | STREAM_NOCREATE ) )
    , rInStream( *pInStream )
    , aTokList( 0x8000, 0x8000 )
{
    InitCtor();
}

/*************************************************************************
|*    SvTokenStream::SvTokenStream()
|*
|*    Beschreibung
*************************************************************************/
SvTokenStream::SvTokenStream( SvStream & rStream, const String & rFileName )
    : aFileName( rFileName )
    , pInStream( NULL )
    , rInStream( rStream )
    , aTokList( 0x8000, 0x8000 )
{
    InitCtor();
}

/*************************************************************************
|*    SvTokenStream::~SvTokenStream()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvTokenStream::GetHexValue()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvTokenStream::GetHexValue( const ByteString & rStr, BigInt * pValue )
{
    short   nLog = 16;

    *pValue = 0;
    char * pStr = (char *)rStr.GetBuffer();
    while( isxdigit( *pStr ) )
    {
        if( isdigit( *pStr ) )
            *pValue = *pValue * BigInt( nLog ) + BigInt(*pStr - '0');
        else
            *pValue = *pValue * BigInt( nLog ) + BigInt(toupper( *pStr ) - 'A' + 10 );
        pStr++;
    }
    return '\0' == *pStr;
}


/*************************************************************************
|*    SvTokenStream::FillTokenList()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvTokenStream::Skip()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvTokenStream::Skip( char cStart, char cEnd, UINT32 * pBegin )
{
    SvToken * pTok = GetToken_Next();
    while( !pTok->IsEof()
           && (!pTok->IsChar() || cStart != pTok->GetChar()) )
         pTok = GetToken_Next();

    if( pBegin )
        *pBegin = Tell() -1;
    UINT32 nContextCount = 1;

    while( !pTok->IsEof() && nContextCount != 0 )
    {
        pTok = GetToken_Next();
        if( pTok->IsChar() )
        {
            if( cEnd == pTok->GetChar() )
                nContextCount--;
            if( cStart == pTok->GetChar() )
                nContextCount++;
        }
    }
    pTok = GetToken();
    if( cEnd == '}' && pTok->IsChar() && pTok->GetChar() == ';' )
        // siehe aerger rsc, }; ausgemerzt
        pTok = GetToken_Next();
    return nContextCount == 0;
}

/*************************************************************************
|*    SvTokenStream::Replace()
|*
|*    Beschreibung
|*    Invariante        Range immer gueltig
*************************************************************************/
void SvTokenStream::Replace( const Range & rRange, SvToken * pNewTok )
{
    // Robuster SeekCursor
    ULONG nSeekPos = aTokList.GetCurPos();
    if( nSeekPos >= (ULONG)rRange.Min() )
        if( nSeekPos <= (ULONG)rRange.Max() )
            nSeekPos = rRange.Min();
        else
            nSeekPos -= rRange.Len();

    long nLen = rRange.Len();
    aTokList.Seek( (ULONG)rRange.Min() );
    while( nLen-- )
        delete aTokList.Remove();

    Seek( nSeekPos );
}

/*************************************************************************
|*    SvTokenStrem::SetCharSet()
|*
|*    Beschreibung
*************************************************************************/
void SvTokenStream::SetCharSet( CharSet nSet )
{
    nCharSet = nSet;

#ifdef DOS
    pCharTab = SvChar::GetTable( nSet, CHARSET_ANSI );
#else
    pCharTab = SvChar::GetTable( nSet, gsl_getSystemTextEncoding() );
#endif
}

/*************************************************************************
|*    SvTokeStream::GetNextChar()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvTokenStrem::GetNumber()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvTokenStream::MakeToken()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvTokenStream::MakeToken( SvToken & rToken )
{
    int             c1;
    USHORT          i;

    do
    {
        if( 0 == c )
            c = GetNextChar();
        // Leerzeichen ueberlesen
        while( isspace( c ) || 26 == c )
        {
            c = GetFastNextChar();
            nColumn += c == '\t' ? nTabSize : 1;
        }
    }
    while( 0 == c && !IsEof() && ( SVSTREAM_OK == rInStream.GetError() ) );

    ULONG nLastLine     = nLine;
    ULONG nLastColumn   = nColumn;
    // Kommentar
    if( '/' == c )
    {
        // Zeit Optimierung, keine Kommentare
        //ByteString aComment( (char)c );
        c1 = c;
        c = GetFastNextChar();
        if( '/' == c )
        {
            while( '\0' != c )
            {
                //aComment += (char)c;
                c = GetFastNextChar();
            }
            c = GetNextChar();
            rToken.nType    = SVTOKEN_COMMENT;
            //rToken.aString    = aComment;
        }
        else if( '*' == c )
        {
            //aComment += (char)c;
            c = GetFastNextChar();
            do
            {
                //aComment += (char)c;
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
                    //aComment += (char)c;
                }
                c = GetFastNextChar();
            }
            while( '/' != c && !IsEof() && ( SVSTREAM_OK == rInStream.GetError() ) );
            if( IsEof() || ( SVSTREAM_OK != rInStream.GetError() ) )
                return FALSE;
            //aComment += (char)c;
            c = GetNextChar();
            rToken.nType = SVTOKEN_COMMENT;
            //rToken.aString = aComment;
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
        i = 0;
        BOOL bDone = FALSE;
        while( !bDone && !IsEof() && c )
        {
            c = GetFastNextChar();
            if( '\0' == c )
            {
                // Strings auch "uber das Zeilenende hinauslesen
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

