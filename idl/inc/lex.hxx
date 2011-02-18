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
    sal_uLong                   nLine, nColumn;
    SVTOKEN_ENUM            nType;
    ByteString                  aString;
    union
    {
        sal_uLong               nLong;
        sal_Bool                bBool;
        char                cChar;
//      SvRttiBase *        pComplexObj;
        SvStringHashEntry * pHash;
    };
public:
            SvToken();
            SvToken( const SvToken & rObj );
            SvToken( sal_uLong n );
            SvToken( SVTOKEN_ENUM nTypeP, sal_Bool b );
            SvToken( char c );
            SvToken( SVTOKEN_ENUM nTypeP, const ByteString & rStr );
//            SvToken( SvRttiBase * pComplexObj );
            SvToken( SVTOKEN_ENUM nTypeP );

    SvToken & operator = ( const SvToken & rObj );

    ByteString          GetTokenAsString() const;
    SVTOKEN_ENUM    GetType() const { return nType; }

    void        SetLine( sal_uLong nLineP )     { nLine = nLineP;       }
    sal_uLong       GetLine() const             { return nLine;         }

    void        SetColumn( sal_uLong nColumnP ) { nColumn = nColumnP;   }
    sal_uLong       GetColumn() const           { return nColumn;       }

    sal_Bool        IsEmpty() const     { return nType == SVTOKEN_EMPTY; }
    sal_Bool        IsComment() const   { return nType == SVTOKEN_COMMENT; }
    sal_Bool        IsInteger() const   { return nType == SVTOKEN_INTEGER; }
    sal_Bool        IsString() const    { return nType == SVTOKEN_STRING; }
    sal_Bool        IsBool() const      { return nType == SVTOKEN_BOOL; }
    sal_Bool        IsIdentifierHash() const
                { return nType == SVTOKEN_HASHID; }
    sal_Bool        IsIdentifier() const
                {
                    return nType == SVTOKEN_IDENTIFIER
                            || nType == SVTOKEN_HASHID;
                }
    sal_Bool        IsChar() const      { return nType == SVTOKEN_CHAR; }
    sal_Bool        IsRttiBase() const  { return nType == SVTOKEN_RTTIBASE; }
    sal_Bool        IsEof() const       { return nType == SVTOKEN_EOF; }

    const ByteString & GetString() const
                {
                    return IsIdentifierHash()
                        ? pHash->GetName()
                        : aString;
                }
    sal_uLong       GetNumber() const       { return nLong;         }
    sal_Bool        GetBool() const         { return bBool;         }
    char        GetChar() const         { return cChar;         }
//    SvRttiBase *GetObject() const       { return pComplexObj;   }

    void        SetHash( SvStringHashEntry * pHashP )
                { pHash = pHashP; nType = SVTOKEN_HASHID; }
    sal_Bool        HasHash() const
                { return nType == SVTOKEN_HASHID; }
    SvStringHashEntry * GetHash() const { return pHash; }
    sal_Bool        Is( SvStringHashEntry * pEntry ) const
                { return IsIdentifierHash() && pHash == pEntry; }
};

inline SvToken::SvToken()
    : nType( SVTOKEN_EMPTY ) {}

inline SvToken::SvToken( sal_uLong n )
    : nType( SVTOKEN_INTEGER ), nLong( n ) {}

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP, sal_Bool b )
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
    sal_uLong       nLine, nColumn;
    int         nBufPos;
    int         c;          // naechstes Zeichen
    CharSet     nCharSet;
    char *      pCharTab;   // Zeiger auf die Konverierungstabelle
    sal_uInt16      nTabSize;   // Tabulator Laenge
    ByteString      aStrTrue;
    ByteString      aStrFalse;
    sal_uLong       nMaxPos;

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
                        return aBufStr.GetChar((sal_uInt16)nBufPos++);
                    }

    void            FillTokenList();
    sal_uLong           GetNumber();
    sal_Bool            MakeToken( SvToken & );
    sal_Bool            IsEof() const { return rInStream.IsEof(); }
    void            SetMax()
                    {
                        sal_uLong n = Tell();
                        if( n > nMaxPos )
                            nMaxPos = n;
                    }
    void            CalcColumn()
                    {
                        // wenn Zeilenende berechnung sparen
                        if( 0 != c )
                        {
                            sal_uInt16 n = 0;
                            nColumn = 0;
                            while( n < nBufPos )
                                nColumn += aBufStr.GetChar(n++) == '\t' ? nTabSize : 1;
                        }
                    }
public:
                    SvTokenStream( const String & rFileName );
                    SvTokenStream( SvStream & rInStream, const String & rFileName );
                    ~SvTokenStream();

    const String &  GetFileName() const { return aFileName; }
    SvStream &      GetStream() { return rInStream; }

    void            SetCharSet( CharSet nSet );
    CharSet         GetCharSet() const { return nCharSet; }

    void            SetTabSize( sal_uInt16 nTabSizeP )
                    { nTabSize = nTabSizeP; }
    sal_uInt16          GetTabSize() const { return nTabSize; }

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
    sal_Bool            Read( char cChar )
                    {
                        if( pCurToken->IsChar()
                          && cChar == pCurToken->GetChar() )
                        {
                            GetToken_Next();
                            return sal_True;
                        }
                        else
                            return sal_False;
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

    sal_uInt32          Tell() const
                    { return aTokList.GetCurPos(); }
    void            Seek( sal_uInt32 nPos )
                    {
                        pCurToken = aTokList.Seek( nPos );
                        SetMax();
                    }
    void            SeekRel( sal_Int32 nRelPos )
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

