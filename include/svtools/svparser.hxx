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

#ifndef INCLUDED_SVTOOLS_SVPARSER_HXX
#define INCLUDED_SVTOOLS_SVPARSER_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <tools/ref.hxx>
#include <tools/solar.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>

struct SvParser_Impl;
class SvStream;

enum class SvParserState
{
    Accepted = 0,
    NotStarted,
    Working,
    Pending,
    Error
};

class SVT_DLLPUBLIC SvParser : public SvRefBase
{
    DECL_LINK( NewDataRead, LinkParamNone*, void );

protected:
    SvStream&           rInput;
    OUString            aToken;             // scanned token
    sal_uLong           nlLineNr;           // current line number
    sal_uLong           nlLinePos;          // current column number

    std::unique_ptr<SvParser_Impl> pImplData; // internal data
    long                nTokenValue;        // additional value (RTF)
    bool                bTokenHasValue;     // indicates whether nTokenValue is valid
    SvParserState       eState;             // status also in derived classes

    rtl_TextEncoding    eSrcEnc;        // Source encoding

    sal_uLong nNextChPos;
    sal_uInt32 nNextCh;                // current character codepoint in UTF32 for the "lex"


    bool                bUCS2BSrcEnc : 1;   // or as big-endian UCS2
    bool                bSwitchToUCS2 : 1;  // switching is allowed

    bool                bRTF_InTextRead : 1;  // only for RTF-Parser!!!

    struct TokenStackType
    {
        OUString    sToken;
        long        nTokenValue;
        bool        bTokenHasValue;
        int         nTokenId;

        TokenStackType()
            : nTokenValue(0)
            , bTokenHasValue(false)
            , nTokenId(0)
        {
        }
    };

    // methods for Token stack
    int SkipToken( short nCnt = -1 );       // "skip" n Tokens back
    TokenStackType* GetStackPtr( short nCnt );

    // scan the next token:
    //  work off Token stack and call GetNextToken_() if necessary.
    //  That one is responsible for the recognition of new Tokens.
    int GetNextToken();
    virtual int GetNextToken_() = 0;

    // is called for each Token that is recognized in CallParser
    virtual void NextToken( int nToken );

    // at times of SvRefBase derivation, not everybody may delete
    virtual ~SvParser() override;

    void ClearTxtConvContext();

private:
    TokenStackType* pTokenStack;
    TokenStackType *pTokenStackPos;
    sal_uInt8 nTokenStackSize, nTokenStackPos;

public:
    SvParser( SvStream& rIn, sal_uInt8 nStackSize = 3 );

    virtual  SvParserState CallParser() = 0; // calling of the parser

    SvParserState GetStatus() const  { return eState; }  // StatusInfo

    sal_uLong    GetLineNr() const       { return nlLineNr; }
    sal_uLong    GetLinePos() const      { return nlLinePos; }
    void         IncLineNr()             { ++nlLineNr; }
    sal_uLong    IncLinePos()            { return ++nlLinePos; }
    inline void         SetLineNr( sal_uLong nlNum );           // inline bottom
    inline void         SetLinePos( sal_uLong nlPos );          // inline bottom

    sal_uInt32 GetNextChar();   // Return next Unicode codepoint in UTF32.
    void RereadLookahead();

    bool IsParserWorking() const { return SvParserState::Working == eState; }

    Link<LinkParamNone*,void> GetAsynchCallLink() const
        { return LINK( const_cast<SvParser*>(this), SvParser, NewDataRead ); }

    // for asynchronous reading from the SvStream
    void SaveState( int nToken );
    void RestoreState();
    virtual void Continue( int nToken );

    // Set/get source encoding. The UCS2BEncoding flag is valid if source
    // encoding is UCS2. It specifies a big endian encoding.
    void SetSrcEncoding( rtl_TextEncoding eSrcEnc );
    rtl_TextEncoding GetSrcEncoding() const { return eSrcEnc; }

    // May the character set be switched to UCS/2, if a BOM
    // is in the first two characters of the stream?
    void SetSwitchToUCS2( bool bSet ) { bSwitchToUCS2 = bSet; }
    bool IsSwitchToUCS2() const { return bSwitchToUCS2; }

    // how many bytes a character consists of
    inline sal_uInt16 GetCharSize() const;

    int GetSaveToken() const;

    // build a Which-Map 'rWhichMap' from an array of WhichIds
    // 'pWhichIds'. It has the length 'nWhichIds'.
    // The WhichMap is not deleted.
    static void BuildWhichTable( std::vector<sal_uInt16> &rWhichMap,
                               sal_uInt16 *pWhichIds,
                               sal_uInt16 nWhichIds );
};


inline void SvParser::SetLineNr( sal_uLong nlNum )
{   nlLineNr = nlNum; }

inline void SvParser::SetLinePos( sal_uLong nlPos )
{   nlLinePos = nlPos; }

inline sal_uInt16 SvParser::GetCharSize() const
{
    return (RTL_TEXTENCODING_UCS2 == eSrcEnc) ? 2 : 1;
}


/*========================================================================
 *
 * SvKeyValue.
 *
 *======================================================================*/

class SvKeyValue
{
    /** Representation.
    */
    OUString m_aKey;
    OUString m_aValue;

public:
    /** Construction.
    */
    SvKeyValue()
    {}

    SvKeyValue (const OUString &rKey, const OUString &rValue)
        : m_aKey (rKey), m_aValue (rValue)
    {}

    SvKeyValue (const SvKeyValue &rOther)
        : m_aKey (rOther.m_aKey), m_aValue (rOther.m_aValue)
    {}

    /** Assignment.
    */
    SvKeyValue& operator= (SvKeyValue &rOther)
    {
        m_aKey   = rOther.m_aKey;
        m_aValue = rOther.m_aValue;
        return *this;
    }

    /** Operation.
    */
    const OUString& GetKey() const { return m_aKey; }
    const OUString& GetValue() const { return m_aValue; }
};

/*========================================================================
 *
 * SvKeyValueIterator.
 *
 *======================================================================*/

class SVT_DLLPUBLIC SvKeyValueIterator : public SvRefBase
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    /** Construction/Destruction.
    */
    SvKeyValueIterator();
    virtual ~SvKeyValueIterator() override;
    SvKeyValueIterator(const SvKeyValueIterator&) = delete;
    SvKeyValueIterator& operator=( const SvKeyValueIterator& ) = delete;

    /** Operation.
    */
    virtual bool GetFirst (SvKeyValue &rKeyVal);
    virtual bool GetNext  (SvKeyValue &rKeyVal);
    virtual void Append   (const SvKeyValue &rKeyVal);
};

typedef tools::SvRef<SvKeyValueIterator> SvKeyValueIteratorRef;

#endif // INCLUDED_SVTOOLS_SVPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
