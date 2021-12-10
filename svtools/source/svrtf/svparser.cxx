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

#include <svtools/svparser.hxx>
#include <svtools/htmltokn.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <rtl/textcvt.h>
#include <rtl/tencinfo.h>
#include <rtl/character.hxx>
#include <sal/log.hxx>
#include <unicode/ucsdet.h>

#include <vector>
#include <climits>

// structure to store the actual data
template<typename T>
struct SvParser_Impl
{
    OUString        aToken;             // gescanntes Token
    sal_uInt64      nFilePos;           // actual position in stream
    sal_uInt32      nlLineNr;           // actual line number
    sal_uInt32      nlLinePos;          // actual column number
    tools::Long            nTokenValue;        // extra value (RTF)
    bool            bTokenHasValue;     // indicates whether nTokenValue is valid
    T               nToken;             // actual Token
    sal_uInt32      nNextCh;            // actual character
    T               nSaveToken;         // the token from Continue

    rtl_TextToUnicodeConverter hConv;
    rtl_TextToUnicodeContext   hContext;

    SvParser_Impl()
        : nFilePos(0)
        , nlLineNr(0)
        , nlLinePos(0)
        , nTokenValue(0)
        , bTokenHasValue(false)
        , nToken(static_cast<T>(0))
        , nNextCh(0)
        , nSaveToken(static_cast<T>(0))
        , hConv( nullptr )
        , hContext( reinterpret_cast<rtl_TextToUnicodeContext>(1) )
    {
    }

};


template<typename T>
SvParser<T>::TokenStackType::TokenStackType()
  : nTokenValue(0)
    , bTokenHasValue(false)
    , nTokenId(static_cast<T>(0))
{
}

// Constructor
template<typename T>
SvParser<T>::SvParser( SvStream& rIn, sal_uInt8 nStackSize )
    : rInput( rIn )
    , nlLineNr( 1 )
    , nlLinePos( 1 )
    , pImplData( nullptr )
    , m_nTokenIndex(0)
    , nTokenValue( 0 )
    , bTokenHasValue( false )
    , eState( SvParserState::NotStarted )
    , eSrcEnc( RTL_TEXTENCODING_DONTKNOW )
    , nNextChPos(0)
    , nNextCh(0)
    , bSwitchToUCS2(false)
    , bRTF_InTextRead(false)
    , nTokenStackSize( nStackSize )
    , nTokenStackPos( 0 )
{
    eState = SvParserState::NotStarted;
    if( nTokenStackSize < 3 )
        nTokenStackSize = 3;
    pTokenStack.reset(new TokenStackType[ nTokenStackSize ]);
    pTokenStackPos = pTokenStack.get();
}

template<typename T>
SvParser<T>::~SvParser()
{
    if( pImplData && pImplData->hConv )
    {
        rtl_destroyTextToUnicodeContext( pImplData->hConv,
                                         pImplData->hContext );
        rtl_destroyTextToUnicodeConverter( pImplData->hConv );
    }

    pTokenStack.reset();
}

template<typename T> SvParserState SvParser<T>::GetStatus() const { return eState; }
template<typename T> sal_uInt32 SvParser<T>::GetLineNr() const       { return nlLineNr; }
template<typename T> sal_uInt32 SvParser<T>::GetLinePos() const      { return nlLinePos; }
template<typename T> void       SvParser<T>::IncLineNr()             { ++nlLineNr; }
template<typename T> sal_uInt32 SvParser<T>::IncLinePos()            { return ++nlLinePos; }
template<typename T> void       SvParser<T>::SetLineNr( sal_uInt32 nlNum ) { nlLineNr = nlNum; }
template<typename T> void       SvParser<T>::SetLinePos( sal_uInt32 nlPos ) {   nlLinePos = nlPos; }
template<typename T> bool       SvParser<T>::IsParserWorking() const { return SvParserState::Working == eState; }
template<typename T> rtl_TextEncoding SvParser<T>::GetSrcEncoding() const { return eSrcEnc; }
template<typename T> void       SvParser<T>::SetSwitchToUCS2( bool bSet ) { bSwitchToUCS2 = bSet; }
template<typename T> bool       SvParser<T>::IsSwitchToUCS2() const { return bSwitchToUCS2; }
template<typename T> sal_uInt16 SvParser<T>::GetCharSize() const { return (RTL_TEXTENCODING_UCS2 == eSrcEnc) ? 2 : 1; }
template<typename T> Link<LinkParamNone*,void> SvParser<T>::GetAsynchCallLink() const
{
    return LINK( const_cast<SvParser*>(this), SvParser, NewDataRead );
}

template<typename T>
void SvParser<T>::ClearTxtConvContext()
{
    if( pImplData && pImplData->hConv )
        rtl_resetTextToUnicodeContext( pImplData->hConv, pImplData->hContext );
}

template<typename T>
void SvParser<T>::SetSrcEncoding( rtl_TextEncoding eEnc )
{
    if( eEnc == eSrcEnc )
        return;

    if( pImplData && pImplData->hConv )
    {
        rtl_destroyTextToUnicodeContext( pImplData->hConv,
                                         pImplData->hContext );
        rtl_destroyTextToUnicodeConverter( pImplData->hConv );
        pImplData->hConv = nullptr;
        pImplData->hContext = reinterpret_cast<rtl_TextToUnicodeContext>(1);
    }

    if( rtl_isOctetTextEncoding(eEnc) ||
        RTL_TEXTENCODING_UCS2 == eEnc  )
    {
        eSrcEnc = eEnc;
        if( !pImplData )
            pImplData.reset(new SvParser_Impl<T>);
        pImplData->hConv = rtl_createTextToUnicodeConverter( eSrcEnc );
        DBG_ASSERT( pImplData->hConv,
                    "SvParser::SetSrcEncoding: no converter for source encoding" );
        if( !pImplData->hConv )
            eSrcEnc = RTL_TEXTENCODING_DONTKNOW;
        else
            pImplData->hContext =
                rtl_createTextToUnicodeContext( pImplData->hConv );
    }
    else
    {
        SAL_WARN( "svtools",
                    "SvParser::SetSrcEncoding: invalid source encoding" );
        eSrcEnc = RTL_TEXTENCODING_DONTKNOW;
    }
}

template<typename T>
void SvParser<T>::RereadLookahead()
{
    rInput.Seek(nNextChPos);
    nNextCh = GetNextChar();
}

template<typename T>
sal_uInt32 SvParser<T>::GetNextChar()
{
    sal_uInt32 c = 0U;

    // When reading multiple bytes, we don't have to care about the file
    // position when we run into the pending state. The file position is
    // maintained by SaveState/RestoreState.
    if( bSwitchToUCS2 && 0 == rInput.Tell() )
    {
        rInput.StartReadingUnicodeText(RTL_TEXTENCODING_DONTKNOW);
        if (rInput.good())
        {
            sal_uInt64 nPos = rInput.Tell();
            if (nPos == 2)
                SetSrcEncoding(RTL_TEXTENCODING_UCS2);
            else if (nPos == 3)
                SetSrcEncoding(RTL_TEXTENCODING_UTF8);
            else // Try to detect encoding without BOM
            {
                std::vector<char> buf(65535); // Arbitrarily chosen 64KiB buffer
                const size_t nSize = rInput.ReadBytes(buf.data(), buf.size());
                // Do not fail when the data size is less than max buffer size
                rInput.ResetError();
                rInput.Seek(0);
                if (nSize > 0)
                {
                    UErrorCode uerr = U_ZERO_ERROR;
                    UCharsetDetector* ucd = ucsdet_open(&uerr);
                    ucsdet_setText(ucd, buf.data(), nSize, &uerr);
                    if (const UCharsetMatch* match = ucsdet_detect(ucd, &uerr))
                    {
                        const char* pEncodingName = ucsdet_getName(match, &uerr);

                        if (U_SUCCESS(uerr))
                        {
                            if (strcmp("UTF-8", pEncodingName) == 0)
                            {
                                SetSrcEncoding(RTL_TEXTENCODING_UTF8);
                            }
                            else if (strcmp("UTF-16BE", pEncodingName) == 0)
                            {
                                SetSrcEncoding(RTL_TEXTENCODING_UCS2);
                                rInput.SetEndian(SvStreamEndian::BIG);
                            }
                            else if (strcmp("UTF-16LE", pEncodingName) == 0)
                            {
                                SetSrcEncoding(RTL_TEXTENCODING_UCS2);
                                rInput.SetEndian(SvStreamEndian::LITTLE);
                            }
                        }
                    }

                    ucsdet_close(ucd);
                }
            }
        }
        bSwitchToUCS2 = false;
    }

    bool bErr;
    nNextChPos = rInput.Tell();

    if( RTL_TEXTENCODING_UCS2 == eSrcEnc )
    {
        sal_Unicode cUC;
        rInput.ReadUtf16(cUC);
        bErr = !rInput.good();
        if( !bErr )
            c = cUC;
    }
    else
    {
        sal_Size nChars = 0;
        do
        {
            char c1;    // signed, that's the text converter expects
            rInput.ReadChar( c1 );
            bErr = !rInput.good();
            if( !bErr )
            {
                if (
                     RTL_TEXTENCODING_DONTKNOW == eSrcEnc ||
                     RTL_TEXTENCODING_SYMBOL == eSrcEnc
                   )
                {
                    // no conversion shall take place
                    c = reinterpret_cast<unsigned char&>( c1 );
                    nChars = 1;
                }
                else
                {
                    assert(pImplData && pImplData->hConv && "no text converter!");

                    sal_Unicode cUC;
                    sal_uInt32 nInfo = 0;
                    sal_Size nCvtBytes;
                    nChars = rtl_convertTextToUnicode(
                                pImplData->hConv, pImplData->hContext,
                                &c1, 1, &cUC, 1,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR|
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR|
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                                &nInfo, &nCvtBytes);
                    if( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL) != 0 )
                    {
                        // The conversion wasn't successful because we haven't
                        // read enough characters.
                        if( pImplData->hContext != reinterpret_cast<rtl_TextToUnicodeContext>(1) )
                        {
                            sal_Unicode sCh[2];
                            while( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL) != 0 )
                            {
                                rInput.ReadChar( c1 );
                                bErr = !rInput.good();
                                if( bErr )
                                    break;

                                nChars = rtl_convertTextToUnicode(
                                            pImplData->hConv, pImplData->hContext,
                                            &c1, 1, sCh , 2,
                                            RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR|
                                            RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR|
                                            RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                                            &nInfo, &nCvtBytes);
                            }
                            if( !bErr )
                            {
                                if( 1 == nChars && 0 == nInfo )
                                {
                                    c = sal_uInt32( sCh[0] );
                                }
                                else if( 2 == nChars && 0 == nInfo )
                                {
                                    c = rtl::combineSurrogates( sCh[0], sCh[1] );
                                }
                                else if( 0 != nChars || 0 != nInfo )
                                {
                                    DBG_ASSERT( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL) == 0,
                                        "source buffer is too small" );
                                    DBG_ASSERT( (nInfo&~(RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL)) == 0,
                                         "there is a conversion error" );
                                    DBG_ASSERT( 0 == nChars,
                                       "there is a converted character, but an error" );
                                    // There are still errors, but nothing we can
                                    // do
                                    c = '?';
                                    nChars = 1;
                                }
                            }
                        }
                        else
                        {
                            char sBuffer[10];
                            sBuffer[0] = c1;
                            sal_uInt16 nLen = 1;
                            while( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL) != 0 &&
                                    nLen < 10 )
                            {
                                rInput.ReadChar( c1 );
                                bErr = !rInput.good();
                                if( bErr )
                                    break;

                                sBuffer[nLen++] = c1;
                                nChars = rtl_convertTextToUnicode(
                                            pImplData->hConv, nullptr, sBuffer, nLen, &cUC, 1,
                                            RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR|
                                            RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR|
                                            RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                                            &nInfo, &nCvtBytes);
                            }
                            if( !bErr )
                            {
                                if( 1 == nChars && 0 == nInfo )
                                {
                                    DBG_ASSERT( nCvtBytes == nLen,
                                                "no all bytes have been converted!" );
                                    c = cUC;
                                }
                                else
                                {
                                    DBG_ASSERT( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL) == 0,
                                        "source buffer is too small" );
                                    DBG_ASSERT( (nInfo&~(RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL)) == 0,
                                         "there is a conversion error" );
                                    DBG_ASSERT( 0 == nChars,
                                       "there is a converted character, but an error" );

                                    // There are still errors, so we use the first
                                    // character and restart after that.
                                    c = reinterpret_cast<unsigned char&>( sBuffer[0] );
                                    rInput.SeekRel( -(nLen-1) );
                                    nChars = 1;
                                }
                            }
                        }
                    }
                    else if( 1 == nChars && 0 == nInfo )
                    {
                        // The conversion was successful
                        DBG_ASSERT( nCvtBytes == 1,
                                    "no all bytes have been converted!" );
                        c = cUC;
                    }
                    else if( 0 != nChars || 0 != nInfo )
                    {
                        DBG_ASSERT( 0 == nChars,
                                "there is a converted character, but an error" );
                        DBG_ASSERT( 0 != nInfo,
                                "there is no converted character and no error" );
                        // #73398#: If the character could not be converted,
                        // because a conversion is not available, do no conversion at all.
                        c = reinterpret_cast<unsigned char&>( c1 );
                        nChars = 1;

                    }
                }
            }
        }
        while( 0 == nChars  && !bErr );
    }

    if ( ! rtl::isUnicodeScalarValue( c ) )
        c = '?' ;

    if( bErr )
    {
        if( ERRCODE_IO_PENDING == rInput.GetError() )
        {
            eState = SvParserState::Pending;
            return c;
        }
        else
            return sal_Unicode(EOF);
    }

    if( c == '\n' )
    {
        IncLineNr();
        SetLinePos( 1 );
    }
    else
        IncLinePos();

    return c;
}

template<typename T>
T SvParser<T>::GetNextToken()
{
    T nRet = static_cast<T>(0);

    if( !nTokenStackPos )
    {
        aToken.clear();     // empty token buffer
        nTokenValue = -1;   // marker for no value read
        bTokenHasValue = false;

        nRet = GetNextToken_();
        if( SvParserState::Pending == eState )
            return nRet;
    }

    ++pTokenStackPos;
    if( pTokenStackPos == pTokenStack.get() + nTokenStackSize )
        pTokenStackPos = pTokenStack.get();

    // pop from stack ??
    if( nTokenStackPos )
    {
        --nTokenStackPos;
        nTokenValue = pTokenStackPos->nTokenValue;
        bTokenHasValue = pTokenStackPos->bTokenHasValue;
        aToken = pTokenStackPos->sToken;
        nRet = pTokenStackPos->nTokenId;
        ++m_nTokenIndex;
    }
    // no, now push actual value on stack
    else if( SvParserState::Working == eState )
    {
        pTokenStackPos->sToken = aToken;
        pTokenStackPos->nTokenValue = nTokenValue;
        pTokenStackPos->bTokenHasValue = bTokenHasValue;
        pTokenStackPos->nTokenId = nRet;
        ++m_nTokenIndex;
    }
    else if( SvParserState::Accepted != eState && SvParserState::Pending != eState )
        eState = SvParserState::Error;       // an error occurred

    return nRet;
}

template<typename T>
T SvParser<T>::SkipToken( short nCnt )       // "skip" n Tokens backward
{
    pTokenStackPos = GetStackPtr( nCnt );
    short nTmp = nTokenStackPos - nCnt;
    if( nTmp < 0 )
        nTmp = 0;
    else if( nTmp > nTokenStackSize )
        nTmp = nTokenStackSize;
    nTokenStackPos = sal_uInt8(nTmp);

    m_nTokenIndex -= nTmp;

    // restore values
    aToken = pTokenStackPos->sToken;
    nTokenValue = pTokenStackPos->nTokenValue;
    bTokenHasValue = pTokenStackPos->bTokenHasValue;

    return pTokenStackPos->nTokenId;
}

template<typename T>
typename SvParser<T>::TokenStackType* SvParser<T>::GetStackPtr( short nCnt )
{
    sal_uInt8 nCurrentPos = sal_uInt8(pTokenStackPos - pTokenStack.get());
    if( nCnt > 0 )
    {
        if( nCnt >= nTokenStackSize )
            nCnt = (nTokenStackSize-1);
        if( nCurrentPos + nCnt < nTokenStackSize )
            nCurrentPos = sal::static_int_cast< sal_uInt8 >(nCurrentPos + nCnt);
        else
            nCurrentPos = sal::static_int_cast< sal_uInt8 >(
                nCurrentPos + (nCnt - nTokenStackSize));
    }
    else if( nCnt < 0 )
    {
        if( -nCnt >= nTokenStackSize )
            nCnt = -nTokenStackSize+1;
        if( -nCnt <= nCurrentPos )
            nCurrentPos = sal::static_int_cast< sal_uInt8 >(nCurrentPos + nCnt);
        else
            nCurrentPos = sal::static_int_cast< sal_uInt8 >(
                nCurrentPos + (nCnt + nTokenStackSize));
    }
    return pTokenStack.get() + nCurrentPos;
}

// to read asynchronous from SvStream

template<typename T>
T SvParser<T>::GetSaveToken() const
{
    return pImplData ? pImplData->nSaveToken : static_cast<T>(0);
}

template<typename T>
void SvParser<T>::SaveState( T nToken )
{
    // save actual status
    if( !pImplData )
    {
        pImplData.reset(new SvParser_Impl<T>);
        pImplData->nSaveToken = static_cast<T>(0);
    }

    pImplData->nFilePos = rInput.Tell();
    pImplData->nToken = nToken;

    pImplData->aToken = aToken;
    pImplData->nlLineNr = nlLineNr;
    pImplData->nlLinePos = nlLinePos;
    pImplData->nTokenValue= nTokenValue;
    pImplData->bTokenHasValue = bTokenHasValue;
    pImplData->nNextCh = nNextCh;
}

template<typename T>
void SvParser<T>::RestoreState()
{
    // restore old status
    if( !pImplData )
        return;

    if( ERRCODE_IO_PENDING == rInput.GetError() )
        rInput.ResetError();
    aToken = pImplData->aToken;
    nlLineNr = pImplData->nlLineNr;
    nlLinePos = pImplData->nlLinePos;
    nTokenValue= pImplData->nTokenValue;
    bTokenHasValue=pImplData->bTokenHasValue;
    nNextCh = pImplData->nNextCh;

    pImplData->nSaveToken = pImplData->nToken;

    rInput.Seek( pImplData->nFilePos );
}

template<typename T>
void SvParser<T>::Continue( T )
{
}


// expanded out version of
//   IMPL_LINK_NOARG( SvParser, NewDataRead, LinkParamNone*, void )
// since it can't cope with template methods
template<typename T>
void SvParser<T>::LinkStubNewDataRead(void * instance, LinkParamNone* data) {
    return static_cast<SvParser<T> *>(instance)->NewDataRead(data);
}
template<typename T>
void SvParser<T>::NewDataRead(SAL_UNUSED_PARAMETER LinkParamNone*)
{
    switch( eState )
    {
    case SvParserState::Pending:
        eState = SvParserState::Working;
        RestoreState();

        Continue( pImplData->nToken );

        if( ERRCODE_IO_PENDING == rInput.GetError() )
            rInput.ResetError();

        if( SvParserState::Pending != eState )
            ReleaseRef();                    // ready otherwise!
        break;

    case SvParserState::NotStarted:
    case SvParserState::Working:
        break;

    default:
        ReleaseRef();                    // ready otherwise!
        break;
    }
}

template class SVT_DLLPUBLIC SvParser<int>;
template class SVT_DLLPUBLIC SvParser<HtmlTokenId>;

/*========================================================================
 *
 * SvKeyValueIterator.
 *
 *======================================================================*/

typedef std::vector<SvKeyValue> SvKeyValueList_Impl;

struct SvKeyValueIterator::Impl
{
    SvKeyValueList_Impl maList;
    sal_uInt16 mnPos;

    Impl() : mnPos(0) {}
};

SvKeyValueIterator::SvKeyValueIterator() : mpImpl(new Impl) {}

SvKeyValueIterator::~SvKeyValueIterator() = default;

bool SvKeyValueIterator::GetFirst (SvKeyValue &rKeyVal)
{
    mpImpl->mnPos = mpImpl->maList.size();
    return GetNext (rKeyVal);
}

bool SvKeyValueIterator::GetNext (SvKeyValue &rKeyVal)
{
    if (mpImpl->mnPos > 0)
    {
        rKeyVal = mpImpl->maList[--mpImpl->mnPos];
        return true;
    }
    else
    {
        // Nothing to do.
        return false;
    }
}

void SvKeyValueIterator::Append (const SvKeyValue &rKeyVal)
{
    mpImpl->maList.push_back(rKeyVal);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
