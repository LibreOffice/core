/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lex.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _LEX_HXX
#define _LEX_HXX

#include <hash.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

/******************** enum ***********************************************/
enum SVTOKEN_ENUM { SVTOKEN_EMPTY,      SVTOKEN_COMMENT,
                    SVTOKEN_INTEGER,    SVTOKEN_STRING,
                    SVTOKEN_BOOL,       SVTOKEN_IDENTIFIER,
                    SVTOKEN_CHAR,       SVTOKEN_RTTIBASE,
                    SVTOKEN_EOF,        SVTOKEN_HASHID };

/******************** class SvToken **************************************/
class BigInt;
class SvToken
{
friend class SvTokenStream;
    ULONG                   nLine, nColumn;
    SVTOKEN_ENUM            nType;
    ByteString                  aString;
    union
    {
        ULONG               nLong;
        BOOL                bBool;
        char                cChar;
//      SvRttiBase *        pComplexObj;
        SvStringHashEntry * pHash;
    };
public:
            SvToken();
            SvToken( const SvToken & rObj );
            SvToken( ULONG n );
            SvToken( SVTOKEN_ENUM nTypeP, BOOL b );
            SvToken( char c );
            SvToken( SVTOKEN_ENUM nTypeP, const ByteString & rStr );
//            SvToken( SvRttiBase * pComplexObj );
            SvToken( SVTOKEN_ENUM nTypeP );

    SvToken & operator = ( const SvToken & rObj );

    ByteString          GetTokenAsString() const;
    ByteString          Print() const;
    SVTOKEN_ENUM    GetType() const { return nType; }

    void        SetLine( ULONG nLineP )     { nLine = nLineP;       }
    ULONG       GetLine() const             { return nLine;         }

    void        SetColumn( ULONG nColumnP ) { nColumn = nColumnP;   }
    ULONG       GetColumn() const           { return nColumn;       }

    BOOL        IsEmpty() const     { return nType == SVTOKEN_EMPTY; }
    BOOL        IsComment() const   { return nType == SVTOKEN_COMMENT; }
    BOOL        IsInteger() const   { return nType == SVTOKEN_INTEGER; }
    BOOL        IsString() const    { return nType == SVTOKEN_STRING; }
    BOOL        IsBool() const      { return nType == SVTOKEN_BOOL; }
    BOOL        IsIdentifierHash() const
                { return nType == SVTOKEN_HASHID; }
    BOOL        IsIdentifier() const
                {
                    return nType == SVTOKEN_IDENTIFIER
                            || nType == SVTOKEN_HASHID;
                }
    BOOL        IsChar() const      { return nType == SVTOKEN_CHAR; }
    BOOL        IsRttiBase() const  { return nType == SVTOKEN_RTTIBASE; }
    BOOL        IsEof() const       { return nType == SVTOKEN_EOF; }

    const ByteString & GetString() const
                {
                    return IsIdentifierHash()
                        ? pHash->GetName()
                        : aString;
                }
    ULONG       GetNumber() const       { return nLong;         }
    BOOL        GetBool() const         { return bBool;         }
    char        GetChar() const         { return cChar;         }
//    SvRttiBase *GetObject() const       { return pComplexObj;   }

    void        SetHash( SvStringHashEntry * pHashP )
                { pHash = pHashP; nType = SVTOKEN_HASHID; }
    BOOL        HasHash() const
                { return nType == SVTOKEN_HASHID; }
    SvStringHashEntry * GetHash() const { return pHash; }
    BOOL        Is( SvStringHashEntry * pEntry ) const
                { return IsIdentifierHash() && pHash == pEntry; }
};

inline SvToken::SvToken()
    : nType( SVTOKEN_EMPTY ) {}

inline SvToken::SvToken( ULONG n )
    : nType( SVTOKEN_INTEGER ), nLong( n ) {}

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP, BOOL b )
    : nType( nTypeP ), bBool( b ) {}

inline SvToken::SvToken( char c )
    : nType( SVTOKEN_CHAR ), cChar( c ) {}

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP, const ByteString & rStr )
    : nType( nTypeP ), aString( rStr ) {}

/*
inline SvToken::SvToken( SvRttiBase * pObj )
    : nType( SVTOKEN_RTTIBASE ), pComplexObj( pObj )
        { pObj->AddRef(); }
*/

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP )
: nType( nTypeP ) {}

DECLARE_LIST( SvTokenList, SvToken * )

/******************** class SvTokenStream ********************************/
class SvTokenStream
{
    ULONG       nLine, nColumn;
    int         nBufPos;
    int         c;          // naechstes Zeichen
    CharSet     nCharSet;
    char *      pCharTab;   // Zeiger auf die Konverierungstabelle
    USHORT      nTabSize;   // Tabulator Laenge
    ByteString      aStrTrue;
    ByteString      aStrFalse;
    ULONG       nMaxPos;

    SvFileStream *  pInStream;
    SvStream &      rInStream;
    String          aFileName;
    SvTokenList     aTokList;
    SvToken *       pCurToken;

    void        InitCtor();

    ByteString          aBufStr;
    int             GetNextChar();
    int             GetFastNextChar()
                    {
                        return aBufStr.GetChar((USHORT)nBufPos++);
                    }

    void            FillTokenList();
    ULONG           GetNumber();
    BOOL            MakeToken( SvToken & );
    BOOL            IsEof() const { return rInStream.IsEof(); }
    void            SetMax()
                    {
                        ULONG n = Tell();
                        if( n > nMaxPos )
                            nMaxPos = n;
                    }
    void            CalcColumn()
                    {
                        // wenn Zeilenende berechnung sparen
                        if( 0 != c )
                        {
                            USHORT n = 0;
                            nColumn = 0;
                            while( n < nBufPos )
                                nColumn += aBufStr.GetChar(n++) == '\t' ? nTabSize : 1;
                        }
                    }
public:
                    SvTokenStream( const String & rFileName );
                    SvTokenStream( SvStream & rInStream, const String & rFileName );
                    ~SvTokenStream();

    static BOOL     GetHexValue( const ByteString & rStr, BigInt * pValue );
    const String &  GetFileName() const { return aFileName; }
    SvStream &      GetStream() { return rInStream; }

    void            SetCharSet( CharSet nSet );
    CharSet         GetCharSet() const { return nCharSet; }

    void            SetTabSize( USHORT nTabSizeP )
                    { nTabSize = nTabSizeP; }
    USHORT          GetTabSize() const { return nTabSize; }

    SvToken *       GetToken_PrevAll()
                    {
                        SvToken * pRetToken = pCurToken;
                        if( NULL == (pCurToken = aTokList.Prev()) )
                            // Current Zeiger nie Null
                            pCurToken = pRetToken;

                        return pRetToken;
                    }
    SvToken *       GetToken_NextAll()
                    {
                        SvToken * pRetToken = pCurToken;
                        if( NULL == (pCurToken = aTokList.Next()) )
                            // Current Zeiger nie Null
                            pCurToken = pRetToken;
                        SetMax();
                        return pRetToken;
                    }
    SvToken *       GetToken_Next()
                    {
                        // Kommentare werden initial entfernt
                        return GetToken_NextAll();
                    }
    SvToken *       GetToken() const { return pCurToken; }
    BOOL            Skip( char cStart, char cEnd, UINT32 * pBegin );
    BOOL            Read( char cChar )
                    {
                        if( pCurToken->IsChar()
                          && cChar == pCurToken->GetChar() )
                        {
                            GetToken_Next();
                            return TRUE;
                        }
                        else
                            return FALSE;
                    }
    void            ReadDelemiter()
                    {
                        if( pCurToken->IsChar()
                          && (';' == pCurToken->GetChar()
                                || ',' == pCurToken->GetChar()) )
                        {
                            GetToken_Next();
                        }
                    }

    UINT32          Tell() const
                    { return aTokList.GetCurPos(); }
    void            Seek( UINT32 nPos )
                    {
                        pCurToken = aTokList.Seek( nPos );
                        SetMax();
                    }
    void            SeekRel( INT32 nRelPos )
                    {
                        pCurToken = aTokList.Seek( Tell() + nRelPos );
                        SetMax();
                    }
    void            SeekEnd()
                    {
                        pCurToken = aTokList.Seek( nMaxPos );
                    }
};



#endif // _LEX_HXX

