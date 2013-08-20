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

#include <stdio.h>
#include <svtools/svparser.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <rtl/textcvt.h>
#include <rtl/tencinfo.h>

// Struktur, um sich die akt. Daten zumerken
struct SvParser_Impl
{
    OUString          aToken;             // gescanntes Token
    sal_uLong           nFilePos;           // akt. Position im Stream
    sal_uLong           nlLineNr;           // akt. Zeilen Nummer
    sal_uLong           nlLinePos;          // akt. Spalten Nummer
    long            nTokenValue;        // zusaetzlicher Wert (RTF)
    sal_Bool            bTokenHasValue;     // indicates whether nTokenValue is valid
    int             nToken;             // akt. Token
    sal_Unicode     nNextCh;            // akt. Zeichen

    int             nSaveToken;         // das Token vom Continue

    rtl_TextToUnicodeConverter hConv;
    rtl_TextToUnicodeContext   hContext;

    SvParser_Impl() :
        nTokenValue(0), nToken(0), nSaveToken(0), hConv( 0 ), hContext( (rtl_TextToUnicodeContext)1 )
    {
    }

};



// Konstruktor
SvParser::SvParser( SvStream& rIn, sal_uInt8 nStackSize )
    : rInput( rIn )
    , nlLineNr( 1 )
    , nlLinePos( 1 )
    , pImplData( 0 )
    , nTokenValue( 0 )
    , bTokenHasValue( false )
    , eState( SVPAR_NOTSTARTED )
    , eSrcEnc( RTL_TEXTENCODING_DONTKNOW )
    , bDownloadingFile( sal_False )
    , nTokenStackSize( nStackSize )
    , nTokenStackPos( 0 )
{
    bUCS2BSrcEnc = bSwitchToUCS2 = sal_False;
    eState = SVPAR_NOTSTARTED;
    if( nTokenStackSize < 3 )
        nTokenStackSize = 3;
    pTokenStack = new TokenStackType[ nTokenStackSize ];
    pTokenStackPos = pTokenStack;
}

SvParser::~SvParser()
{
    if( pImplData && pImplData->hConv )
    {
        rtl_destroyTextToUnicodeContext( pImplData->hConv,
                                         pImplData->hContext );
        rtl_destroyTextToUnicodeConverter( pImplData->hConv );
    }

    delete pImplData;

    delete [] pTokenStack;
}

void SvParser::ClearTxtConvContext()
{
    if( pImplData && pImplData->hConv )
        rtl_resetTextToUnicodeContext( pImplData->hConv, pImplData->hContext );
}

void SvParser::SetSrcEncoding( rtl_TextEncoding eEnc )
{

    if( eEnc != eSrcEnc )
    {
        if( pImplData && pImplData->hConv )
        {
            rtl_destroyTextToUnicodeContext( pImplData->hConv,
                                             pImplData->hContext );
            rtl_destroyTextToUnicodeConverter( pImplData->hConv );
            pImplData->hConv = 0;
            pImplData->hContext = (rtl_TextToUnicodeContext )1;
        }

        if( rtl_isOctetTextEncoding(eEnc) ||
            RTL_TEXTENCODING_UCS2 == eEnc  )
        {
            eSrcEnc = eEnc;
            if( !pImplData )
                pImplData = new SvParser_Impl;
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
            DBG_ASSERT( !this,
                        "SvParser::SetSrcEncoding: invalid source encoding" );
            eSrcEnc = RTL_TEXTENCODING_DONTKNOW;
        }
    }
}

void SvParser::RereadLookahead()
{
    rInput.Seek(nNextChPos);
    nNextCh = GetNextChar();
}

sal_Unicode SvParser::GetNextChar()
{
    sal_Unicode c = 0U;

    // When reading muliple bytes, we don't have to care about the file
    // position when we run inti the pending state. The file position is
    // maintained by SaveState/RestoreState.
    sal_Bool bErr;
    if( bSwitchToUCS2 && 0 == rInput.Tell() )
    {
        sal_uChar c1, c2;
        sal_Bool bSeekBack = sal_True;

        rInput >> c1;
        bErr = rInput.IsEof() || rInput.GetError();
        if( !bErr )
        {
            if( 0xff == c1 || 0xfe == c1 )
            {
                rInput >> c2;
                bErr = rInput.IsEof() || rInput.GetError();
                if( !bErr )
                {
                    if( 0xfe == c1 && 0xff == c2 )
                    {
                        eSrcEnc = RTL_TEXTENCODING_UCS2;
                        bUCS2BSrcEnc = sal_True;
                        bSeekBack = sal_False;
                    }
                    else if( 0xff == c1 && 0xfe == c2 )
                    {
                        eSrcEnc = RTL_TEXTENCODING_UCS2;
                        bUCS2BSrcEnc = sal_False;
                        bSeekBack = sal_False;
                    }
                }
            }
        }
        if( bSeekBack )
            rInput.Seek( 0 );

        bSwitchToUCS2 = sal_False;
    }

    nNextChPos = rInput.Tell();

    if( RTL_TEXTENCODING_UCS2 == eSrcEnc )
    {
        sal_Unicode cUC = USHRT_MAX;
        sal_uChar c1, c2;

        rInput >> c1 >> c2;
        if( 2 == rInput.Tell() &&
            !(rInput.IsEof() || rInput.GetError()) &&
            ( (bUCS2BSrcEnc && 0xfe == c1 && 0xff == c2) ||
              (!bUCS2BSrcEnc && 0xff == c1 && 0xfe == c2) ) )
            rInput >> c1 >> c2;

        bErr = rInput.IsEof() || rInput.GetError();
        if( !bErr )
        {
            if( bUCS2BSrcEnc )
                cUC = (sal_Unicode(c1) << 8) | c2;
            else
                cUC = (sal_Unicode(c2) << 8) | c1;
        }

        if( !bErr )
        {
            c = cUC;
        }
    }
    else
    {
        sal_Size nChars = 0;
        do
        {
            sal_Char c1;    // signed, that's the text converter expects
            rInput >> c1;
            bErr = rInput.IsEof() || rInput.GetError();
            if( !bErr )
            {
                if (
                     RTL_TEXTENCODING_DONTKNOW == eSrcEnc ||
                     RTL_TEXTENCODING_SYMBOL == eSrcEnc
                   )
                {
                    // no convserion shall take place
                    c = (sal_Unicode)c1;
                    nChars = 1;
                }
                else
                {
                    DBG_ASSERT( pImplData && pImplData->hConv,
                                "no text converter!" );

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
                    if( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) != 0 )
                    {
                        // The conversion wasn't successful because we haven't
                        // read enough characters.
                        if( pImplData->hContext != (rtl_TextToUnicodeContext)1 )
                        {
                            while( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) != 0 )
                            {
                                rInput >> c1;
                                bErr = rInput.IsEof() || rInput.GetError();
                                if( bErr )
                                    break;

                                nChars = rtl_convertTextToUnicode(
                                            pImplData->hConv, pImplData->hContext,
                                            &c1, 1, &cUC, 1,
                                            RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR|
                                            RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR|
                                            RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                                            &nInfo, &nCvtBytes);
                            }
                            if( !bErr )
                            {
                                if( 1 == nChars && 0 == nInfo )
                                {
                                    c = cUC;
                                }
                                else if( 0 != nChars || 0 != nInfo )
                                {
                                    DBG_ASSERT( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) == 0,
                                        "source buffer is to small" );
                                    DBG_ASSERT( (nInfo&~(RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL)) == 0,
                                         "there is a conversion error" );
                                    DBG_ASSERT( 0 == nChars,
                                       "there is a converted character, but an error" );
                                    // There are still errors, but nothing we can
                                    // do
                                    c = (sal_Unicode)'?';
                                    nChars = 1;
                                }
                            }
                        }
                        else
                        {
                            sal_Char sBuffer[10];
                            sBuffer[0] = c1;
                            sal_uInt16 nLen = 1;
                            while( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) != 0 &&
                                    nLen < 10 )
                            {
                                rInput >> c1;
                                bErr = rInput.IsEof() || rInput.GetError();
                                if( bErr )
                                    break;

                                sBuffer[nLen++] = c1;
                                nChars = rtl_convertTextToUnicode(
                                            pImplData->hConv, 0, sBuffer, nLen, &cUC, 1,
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
                                    DBG_ASSERT( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) == 0,
                                        "source buffer is to small" );
                                    DBG_ASSERT( (nInfo&~(RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL)) == 0,
                                         "there is a conversion error" );
                                    DBG_ASSERT( 0 == nChars,
                                       "there is a converted character, but an error" );

                                    // There are still errors, so we use the first
                                    // character and restart after that.
                                    c = (sal_Unicode)sBuffer[0];
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
                        c = (sal_Unicode)c1;
                        nChars = 1;

                    }
                }
            }
        }
        while( 0 == nChars  && !bErr );
    }
    if( bErr )
    {
        if( ERRCODE_IO_PENDING == rInput.GetError() )
        {
            eState = SVPAR_PENDING;
            return c;
        }
        else
            return sal_Unicode(EOF);
    }

    if( c == '\n' )
    {
        IncLineNr();
        SetLinePos( 1L );
    }
    else
        IncLinePos();
    return c;
}

int SvParser::GetNextToken()
{
    int nRet = 0;

    if( !nTokenStackPos )
    {
        aToken = "";     // Token-Buffer loeschen
        nTokenValue = -1;   // Kennzeichen fuer kein Value gelesen
        bTokenHasValue = false;

        nRet = _GetNextToken();
        if( SVPAR_PENDING == eState )
            return nRet;
    }

    ++pTokenStackPos;
    if( pTokenStackPos == pTokenStack + nTokenStackSize )
        pTokenStackPos = pTokenStack;

    // vom Stack holen ??
    if( nTokenStackPos )
    {
        --nTokenStackPos;
        nTokenValue = pTokenStackPos->nTokenValue;
        bTokenHasValue = pTokenStackPos->bTokenHasValue;
        aToken = pTokenStackPos->sToken;
        nRet = pTokenStackPos->nTokenId;
    }
    // nein, dann das aktuelle auf den Stack
    else if( SVPAR_WORKING == eState )
    {
        pTokenStackPos->sToken = aToken;
        pTokenStackPos->nTokenValue = nTokenValue;
        pTokenStackPos->bTokenHasValue = bTokenHasValue;
        pTokenStackPos->nTokenId = nRet;
    }
    else if( SVPAR_ACCEPTED != eState && SVPAR_PENDING != eState )
        eState = SVPAR_ERROR;       // irgend ein Fehler

    return nRet;
}

int SvParser::SkipToken( short nCnt )       // n Tokens zurueck "skippen"
{
    pTokenStackPos = GetStackPtr( nCnt );
    short nTmp = nTokenStackPos - nCnt;
    if( nTmp < 0 )
        nTmp = 0;
    else if( nTmp > nTokenStackSize )
        nTmp = nTokenStackSize;
    nTokenStackPos = sal_uInt8(nTmp);

    // und die Werte zurueck
    aToken = pTokenStackPos->sToken;
    nTokenValue = pTokenStackPos->nTokenValue;
    bTokenHasValue = pTokenStackPos->bTokenHasValue;

    return pTokenStackPos->nTokenId;
}

SvParser::TokenStackType* SvParser::GetStackPtr( short nCnt )
{
    sal_uInt8 nAktPos = sal_uInt8(pTokenStackPos - pTokenStack );
    if( nCnt > 0 )
    {
        if( nCnt >= nTokenStackSize )
            nCnt = (nTokenStackSize-1);
        if( nAktPos + nCnt < nTokenStackSize )
            nAktPos = sal::static_int_cast< sal_uInt8 >(nAktPos + nCnt);
        else
            nAktPos = sal::static_int_cast< sal_uInt8 >(
                nAktPos + (nCnt - nTokenStackSize));
    }
    else if( nCnt < 0 )
    {
        if( -nCnt >= nTokenStackSize )
            nCnt = -nTokenStackSize+1;
        if( -nCnt <= nAktPos )
            nAktPos = sal::static_int_cast< sal_uInt8 >(nAktPos + nCnt);
        else
            nAktPos = sal::static_int_cast< sal_uInt8 >(
                nAktPos + (nCnt + nTokenStackSize));
    }
    return pTokenStack + nAktPos;
}

// wird fuer jedes Token gerufen, das in CallParser erkannt wird
void SvParser::NextToken( int )
{
}


// fuers asynchrone lesen aus dem SvStream

int SvParser::GetSaveToken() const
{
    return pImplData ? pImplData->nSaveToken : 0;
}

void SvParser::SaveState( int nToken )
{
    // aktuellen Status merken
    if( !pImplData )
    {
        pImplData = new SvParser_Impl;
        pImplData->nSaveToken = 0;
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

void SvParser::RestoreState()
{
    // alten Status wieder zurueck setzen
    if( pImplData )
    {
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
}

void SvParser::Continue( int )
{
}

void SvParser::BuildWhichTbl( std::vector<sal_uInt16> &rWhichMap,
                              sal_uInt16 *pWhichIds,
                              sal_uInt16 nWhichIds )
{
    sal_uInt16 aNewRange[2];

    for( sal_uInt16 nCnt = 0; nCnt < nWhichIds; ++nCnt, ++pWhichIds )
        if( *pWhichIds )
        {
            aNewRange[0] = aNewRange[1] = *pWhichIds;
            sal_Bool bIns = sal_True;

            // Position suchen
            for ( sal_uInt16 nOfs = 0; rWhichMap[nOfs]; nOfs += 2 )
            {
                if( *pWhichIds < rWhichMap[nOfs] - 1 )
                {
                    // neuen Range davor
                    rWhichMap.insert( rWhichMap.begin() + nOfs, aNewRange, aNewRange + 2 );
                    bIns = sal_False;
                    break;
                }
                else if( *pWhichIds == rWhichMap[nOfs] - 1 )
                {
                    // diesen Range nach unten erweitern
                    rWhichMap[nOfs] = *pWhichIds;
                    bIns = sal_False;
                    break;
                }
                else if( *pWhichIds == rWhichMap[nOfs+1] + 1 )
                {
                    if( rWhichMap[nOfs+2] != 0 && rWhichMap[nOfs+2] == *pWhichIds + 1 )
                    {
                        // mit dem naechsten Bereich mergen
                        rWhichMap[nOfs+1] = rWhichMap[nOfs+3];
                        rWhichMap.erase( rWhichMap.begin() + nOfs + 2,
                                rWhichMap.begin() + nOfs + 4 );
                    }
                    else
                        // diesen Range nach oben erweitern
                        rWhichMap[nOfs+1] = *pWhichIds;
                    bIns = sal_False;
                    break;
                }
            }

            // einen Range hinten anhaengen
            if( bIns )
            {
                rWhichMap.insert( rWhichMap.begin() + rWhichMap.size() - 1,
                        aNewRange, aNewRange + 2 );
            }
        }
}


IMPL_STATIC_LINK( SvParser, NewDataRead, void*, EMPTYARG )
{
    switch( pThis->eState )
    {
    case SVPAR_PENDING:
        // Wenn gerade ein File geladen wird duerfen wir nicht weiterlaufen,
        // sondern muessen den Aufruf ignorieren.
        if( pThis->IsDownloadingFile() )
            break;

        pThis->eState = SVPAR_WORKING;
        pThis->RestoreState();

        pThis->Continue( pThis->pImplData->nToken );

        if( ERRCODE_IO_PENDING == pThis->rInput.GetError() )
            pThis->rInput.ResetError();

        if( SVPAR_PENDING != pThis->eState )
            pThis->ReleaseRef();                    // ansonsten sind wir fertig!
        break;

    case SVPAR_WAITFORDATA:
        pThis->eState = SVPAR_WORKING;
        break;

    case SVPAR_NOTSTARTED:
    case SVPAR_WORKING:
        break;

    default:
        pThis->ReleaseRef();                    // ansonsten sind wir fertig!
        break;
    }

    return 0;
}

/*========================================================================
 *
 * SvKeyValueIterator.
 *
 *======================================================================*/

/*
 * SvKeyValueIterator.
 */
SvKeyValueIterator::SvKeyValueIterator (void)
    : m_pList (new SvKeyValueList_Impl),
      m_nPos  (0)
{
}

/*
 * ~SvKeyValueIterator.
 */
SvKeyValueIterator::~SvKeyValueIterator (void)
{
    delete m_pList;
}

/*
 * GetFirst.
 */
sal_Bool SvKeyValueIterator::GetFirst (SvKeyValue &rKeyVal)
{
    m_nPos = m_pList->size();
    return GetNext (rKeyVal);
}

/*
 * GetNext.
 */
sal_Bool SvKeyValueIterator::GetNext (SvKeyValue &rKeyVal)
{
    if (m_nPos > 0)
    {
        rKeyVal = (*m_pList)[--m_nPos];
        return sal_True;
    }
    else
    {
        // Nothing to do.
        return sal_False;
    }
}

/*
 * Append.
 */
void SvKeyValueIterator::Append (const SvKeyValue &rKeyVal)
{
    m_pList->push_back(new SvKeyValue(rKeyVal));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
