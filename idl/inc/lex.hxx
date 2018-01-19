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
#include "hash.hxx"
#include <tools/stream.hxx>
#include <vector>
#include <memory>

enum class SVTOKENTYPE { Empty,      Comment,
                         Integer,    String,
                         Bool,       Identifier,
                         Char,
                         EndOfFile,  HashId };

class SvToken
{
friend class SvTokenStream;
    sal_uLong               nLine, nColumn;
    SVTOKENTYPE             nType;
    OString                 aString;
    union
    {
        sal_uLong           nLong;
        bool                bBool;
        char                cChar;
        SvStringHashEntry * pHash;
    };
public:
            SvToken();
            SvToken( const SvToken & rObj ) = delete;

    SvToken & operator = ( const SvToken & rObj );

    OString     GetTokenAsString() const;

    void        SetLine( sal_uLong nLineP )     { nLine = nLineP;       }
    sal_uLong   GetLine() const             { return nLine;         }

    void        SetColumn( sal_uLong nColumnP ) { nColumn = nColumnP;   }
    sal_uLong   GetColumn() const           { return nColumn;       }

    bool        IsComment() const   { return nType == SVTOKENTYPE::Comment; }
    bool        IsInteger() const   { return nType == SVTOKENTYPE::Integer; }
    bool        IsString() const    { return nType == SVTOKENTYPE::String; }
    bool        IsBool() const      { return nType == SVTOKENTYPE::Bool; }
    bool        IsIdentifierHash() const
                { return nType == SVTOKENTYPE::HashId; }
    bool        IsIdentifier() const
                {
                    return nType == SVTOKENTYPE::Identifier
                            || nType == SVTOKENTYPE::HashId;
                }
    bool        IsChar() const      { return nType == SVTOKENTYPE::Char; }
    bool        IsEof() const       { return nType == SVTOKENTYPE::EndOfFile; }

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
                { pHash = pHashP; nType = SVTOKENTYPE::HashId; }
    bool        Is( SvStringHashEntry const * pEntry ) const
                { return IsIdentifierHash() && pHash == pEntry; }
};

inline SvToken::SvToken()
    : nLine(0)
    , nColumn(0)
    , nType( SVTOKENTYPE::Empty )
{
}

class SvTokenStream
{
    sal_uLong       nLine, nColumn;
    sal_Int32       nBufPos;
    char            c;          // next character
    static const sal_uInt16 nTabSize = 4;   // length of tabulator
    OString         aStrTrue;
    OString         aStrFalse;
    sal_uLong       nMaxPos;

    std::unique_ptr<SvFileStream>          pInStream;
    OUString                               aFileName;
    std::vector<std::unique_ptr<SvToken> > aTokList;
    std::vector<std::unique_ptr<SvToken> >::iterator pCurToken;

    OString         aBufStr;

    void            InitCtor();

    char            GetNextChar();
    char            GetFastNextChar()
                    {
                        return (nBufPos < aBufStr.getLength())
                            ? aBufStr[nBufPos++]
                            : '\0';
                    }

    void            FillTokenList();
    sal_uLong       GetNumber();
    bool            MakeToken( SvToken & );
    bool            IsEof() const { return pInStream->eof(); }
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
                            sal_Int32 n = 0;
                            nColumn = 0;
                            while( n < nBufPos )
                                nColumn += aBufStr[n++] == '\t' ? nTabSize : 1;
                        }
                    }
public:
                    SvTokenStream( const OUString & rFileName );
                    ~SvTokenStream();

    const OUString &  GetFileName() const { return aFileName; }
    SvStream &        GetStream() { return *pInStream; }

    SvToken& GetToken_PrevAll()
    {
        std::vector<std::unique_ptr<SvToken> >::iterator pRetToken = pCurToken;

        // current iterator always valid
        if(pCurToken != aTokList.begin())
            --pCurToken;

        return *(*pRetToken).get();
    }

    SvToken& GetToken_Next()
    {
        std::vector<std::unique_ptr<SvToken> >::iterator pRetToken = pCurToken++;

        if (pCurToken == aTokList.end())
            pCurToken = pRetToken;

        SetMax();

        return *(*pRetToken).get();
    }

    SvToken& GetToken() const { return *(*pCurToken).get(); }

    bool     ReadIf( char cChar )
    {
        if( GetToken().IsChar() && cChar == GetToken().GetChar() )
        {
            GetToken_Next();
            return true;
        }
        else
            return false;
    }

    void     ReadIfDelimiter()
    {
        if( GetToken().IsChar()
            && (';' == GetToken().GetChar()
                 || ',' == GetToken().GetChar()) )
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

    void SeekToMax()
    {
        pCurToken = aTokList.begin()+nMaxPos;
    }
};


#endif // INCLUDED_IDL_INC_LEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
