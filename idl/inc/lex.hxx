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

#ifndef INCLUDED_IDL_INC_LEX_HXX
#define INCLUDED_IDL_INC_LEX_HXX

#include <sal/types.h>
#include <hash.hxx>
#include <tools/stream.hxx>
#include <vector>
#include <memory>

enum SVTOKEN_ENUM { SVTOKEN_EMPTY,      SVTOKEN_COMMENT,
                    SVTOKEN_INTEGER,    SVTOKEN_STRING,
                    SVTOKEN_BOOL,       SVTOKEN_IDENTIFIER,
                    SVTOKEN_CHAR,       SVTOKEN_RTTIBASE,
                    SVTOKEN_EOF,        SVTOKEN_HASHID };

class SvToken
{
friend class SvTokenStream;
    sal_uLong               nLine, nColumn;
    SVTOKEN_ENUM            nType;
    OString            aString;
    union
    {
        sal_uLong           nLong;
        bool                bBool;
        char                cChar;
        SvStringHashEntry * pHash;
    };
public:
            SvToken();
            SvToken( const SvToken & rObj );
            SvToken( sal_uLong n );
            SvToken( SVTOKEN_ENUM nTypeP, bool b );
            SvToken( char c );
            SvToken( SVTOKEN_ENUM nTypeP, const OString& rStr );
            SvToken( SVTOKEN_ENUM nTypeP );

    SvToken & operator = ( const SvToken & rObj );

    OString     GetTokenAsString() const;

    void        SetLine( sal_uLong nLineP )     { nLine = nLineP;       }
    sal_uLong   GetLine() const             { return nLine;         }

    void        SetColumn( sal_uLong nColumnP ) { nColumn = nColumnP;   }
    sal_uLong   GetColumn() const           { return nColumn;       }

    bool        IsEmpty() const     { return nType == SVTOKEN_EMPTY; }
    bool        IsComment() const   { return nType == SVTOKEN_COMMENT; }
    bool        IsInteger() const   { return nType == SVTOKEN_INTEGER; }
    bool        IsString() const    { return nType == SVTOKEN_STRING; }
    bool        IsBool() const      { return nType == SVTOKEN_BOOL; }
    bool        IsIdentifierHash() const
                { return nType == SVTOKEN_HASHID; }
    bool        IsIdentifier() const
                {
                    return nType == SVTOKEN_IDENTIFIER
                            || nType == SVTOKEN_HASHID;
                }
    bool        IsChar() const      { return nType == SVTOKEN_CHAR; }
    bool        IsEof() const       { return nType == SVTOKEN_EOF; }

    const OString& GetString() const
                {
                    return IsIdentifierHash()
                        ? pHash->GetName()
                        : aString;
                }
    sal_uLong   GetNumber() const       { return nLong;         }
    bool        GetBool() const         { return bBool;         }
    char        GetChar() const         { return cChar;         }

    void        SetHash( SvStringHashEntry * pHashP )
                { pHash = pHashP; nType = SVTOKEN_HASHID; }
    bool        HasHash() const
                { return nType == SVTOKEN_HASHID; }
    bool        Is( SvStringHashEntry * pEntry ) const
                { return IsIdentifierHash() && pHash == pEntry; }
};

inline SvToken::SvToken()
    : nLine(0)
    , nColumn(0)
    , nType( SVTOKEN_EMPTY )
{
}

inline SvToken::SvToken( sal_uLong n )
    : nType( SVTOKEN_INTEGER ), nLong( n ) {}

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP, bool b )
    : nType( nTypeP ), bBool( b ) {}

inline SvToken::SvToken( char c )
    : nType( SVTOKEN_CHAR ), cChar( c ) {}

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP, const OString& rStr )
    : nType( nTypeP ), aString( rStr ) {}

inline SvToken::SvToken( SVTOKEN_ENUM nTypeP )
: nType( nTypeP ) {}

class SvTokenStream
{
    sal_uLong       nLine, nColumn;
    int         nBufPos;
    int         c;          // next character
    sal_uInt16      nTabSize;   // length of tabulator
    OString    aStrTrue;
    OString    aStrFalse;
    sal_uLong       nMaxPos;

    SvFileStream *  pInStream;
    SvStream &      rInStream;
    OUString        aFileName;
    std::vector<std::unique_ptr<SvToken> > aTokList;
    std::vector<std::unique_ptr<SvToken> >::iterator pCurToken;

    OString aBufStr;

    void        InitCtor();

    int             GetNextChar();
    int             GetFastNextChar()
                    {
                        return (nBufPos < aBufStr.getLength())
                            ? aBufStr[nBufPos++]
                            : '\0';
                    }

    void            FillTokenList();
    sal_uLong       GetNumber();
    bool            MakeToken( SvToken & );
    bool            IsEof() const { return rInStream.IsEof(); }
    void            SetMax()
                    {
                        sal_uLong n = Tell();
                        if( n > nMaxPos )
                            nMaxPos = n;
                    }
    void            CalcColumn()
                    {
                        // if end of line spare calculation
                        if( 0 != c )
                        {
                            sal_uInt16 n = 0;
                            nColumn = 0;
                            while( n < nBufPos )
                                nColumn += aBufStr[n++] == '\t' ? nTabSize : 1;
                        }
                    }
public:
                    SvTokenStream( const OUString & rFileName );
                    SvTokenStream( SvStream & rInStream, const OUString & rFileName );
                    ~SvTokenStream();

    const OUString &  GetFileName() const { return aFileName; }
    SvStream &        GetStream() { return rInStream; }

    SvToken* GetToken_PrevAll()
    {
        std::vector<std::unique_ptr<SvToken> >::iterator pRetToken = pCurToken;

        // current iterator always valid
        if(pCurToken != aTokList.begin())
            --pCurToken;

        return (*pRetToken).get();
    }

    SvToken* GetToken_NextAll()
    {
        std::vector<std::unique_ptr<SvToken> >::iterator pRetToken = pCurToken++;

        if (pCurToken == aTokList.end())
            pCurToken = pRetToken;

        SetMax();

        return (*pRetToken).get();
    }

    SvToken* GetToken_Next()
    {
        // comments get removed initially
        return GetToken_NextAll();
    }

    SvToken& GetToken() const { return *(*pCurToken).get(); }

    bool     Read( char cChar )
                    {
                        if( (*pCurToken)->IsChar()
                          && cChar == (*pCurToken)->GetChar() )
                        {
                            GetToken_Next();
                            return true;
                        }
                        else
                            return false;
                    }

    void     ReadDelemiter()
                    {
                        if( (*pCurToken)->IsChar()
                          && (';' == (*pCurToken)->GetChar()
                                || ',' == (*pCurToken)->GetChar()) )
                        {
                            GetToken_Next();
                        }
                    }

    sal_uInt32 Tell() const { return pCurToken-aTokList.begin(); }

    void Seek( sal_uInt32 nPos )
    {
        pCurToken = aTokList.begin() + nPos;
        SetMax();
    }

    void SeekEnd()
    {
        pCurToken = aTokList.begin()+nMaxPos;
    }
};



#endif // INCLUDED_IDL_INC_LEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
