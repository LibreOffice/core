/*************************************************************************
 *
 *  $RCSfile: svparser.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:39:43 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include <stdio.h>
#include "svparser.hxx"
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#define _SVSTDARR_USHORTS
#include <svstdarr.hxx>
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#define SVPAR_CSM_

#define SVPAR_CSM_ANSI      0x0001U
#define SVPAR_CSM_UTF8      0x0002U
#define SVPAR_CSM_UCS2B     0x0004U
#define SVPAR_CSM_UCS2L     0x0008U
#define SVPAR_CSM_SWITCH    0x8000U

// Struktur, um sich die akt. Daten zumerken
struct SvParser_Impl
{
    String          aToken;             // gescanntes Token
    ULONG           nFilePos;           // akt. Position im Stream
    ULONG           nlLineNr;           // akt. Zeilen Nummer
    ULONG           nlLinePos;          // akt. Spalten Nummer
    long            nTokenValue;        // zusaetzlicher Wert (RTF)
    int             nToken;             // akt. Token
    sal_Unicode     nNextCh;            // akt. Zeichen

    int             nSaveToken;         // das Token vom Continue

    rtl_TextToUnicodeConverter hConv;
    rtl_TextToUnicodeContext   hContext;

#ifndef PRODUCT
    SvFileStream aOut;
#endif

    SvParser_Impl() :
        nSaveToken(0), hConv( 0 ), hContext( (rtl_TextToUnicodeContext)1 )
    {
    }

};



// Konstruktor
SvParser::SvParser( SvStream& rIn, BYTE nStackSize )
    : rInput( rIn ), eState( SVPAR_NOTSTARTED ),
    nlLineNr( 1 ), nlLinePos( 1 ),
    nTokenStackSize( nStackSize ), nTokenStackPos( 0 ),
    nTokenValue( 0 ),
    pImplData( 0 ),
    bDownloadingFile( FALSE ),
    eSrcEnc( RTL_TEXTENCODING_DONTKNOW )
{
    bUCS2BSrcEnc = bSwitchToUCS2 = FALSE;
    eState = SVPAR_NOTSTARTED;
    if( nTokenStackSize < 3 )
        nTokenStackSize = 3;
    pTokenStack = new TokenStackType[ nTokenStackSize ];
    pTokenStackPos = pTokenStack;

#ifndef PRODUCT

    // wenn die Datei schon existiert, dann Anhaengen:
    if( !pImplData )
        pImplData = new SvParser_Impl;
    pImplData->aOut.Open( String::CreateFromAscii( "\\parser.dmp" ),
                          STREAM_STD_WRITE | STREAM_NOCREATE );
    if( pImplData->aOut.GetError() || !pImplData->aOut.IsOpen() )
        pImplData->aOut.Close();
    else
    {
        pImplData->aOut.Seek( STREAM_SEEK_TO_END );
        pImplData->aOut << "\x0c\n\n >>>>>>>>>>>>>>> Dump Start <<<<<<<<<<<<<<<\n";
    }
#endif
}

SvParser::~SvParser()
{
#ifndef PRODUCT
    if( pImplData->aOut.IsOpen() )
        pImplData->aOut << "\n\n >>>>>>>>>>>>>>> Dump Ende <<<<<<<<<<<<<<<\n";
    pImplData->aOut.Close();
#endif

    if( pImplData && pImplData->hConv )
    {
        rtl_destroyTextToUnicodeContext( pImplData->hConv,
                                         pImplData->hContext );
        rtl_destroyTextToUnicodeConverter( pImplData->hConv );
    }

    delete pImplData;

#ifdef MPW
    // der MPW-Compiler ruft sonst keine Dtoren!
    for (int n = 0; n < nTokenStackSize; ++n )
        (pTokenStack+n)->TokenStackType::~TokenStackType();
    delete (void*) pTokenStack;
#else
    __DELETE(nTokenStackSize) pTokenStack;
#endif
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
    BOOL bErr;
    if( bSwitchToUCS2 && 0 == rInput.Tell() )
    {
        sal_uChar c1, c2;
        BOOL bSeekBack = TRUE;

        rInput >> c1;
        if( !(bErr = (rInput.IsEof() || rInput.GetError())) )
        {
            if( 0xff == c1 || 0xfe == c1 )
            {
                rInput >> c2;
                if( !(bErr = (rInput.IsEof() || rInput.GetError())) )
                {
                    if( 0xfe == c1 && 0xff == c2 )
                    {
                        eSrcEnc = RTL_TEXTENCODING_UCS2;
                        bUCS2BSrcEnc = TRUE;
                        bSeekBack = FALSE;
                    }
                    else if( 0xff == c1 && 0xfe == c2 )
                    {
                        eSrcEnc = RTL_TEXTENCODING_UCS2;
                        bUCS2BSrcEnc = FALSE;
                        bSeekBack = FALSE;
                    }
                }
            }
        }
        if( bSeekBack )
            rInput.Seek( 0 );

        bSwitchToUCS2 = FALSE;
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

        if( !(bErr = (rInput.IsEof() || rInput.GetError())) )
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
            if( !(bErr = (rInput.IsEof() || rInput.GetError())) )
            {
                if( RTL_TEXTENCODING_DONTKNOW == eSrcEnc )
                {
                    // no convserion shall take place
                    c = (sal_Unicode)c1;
                    nChars = 1;
                }
                else
                {
                    DBG_ASSERT( pImplData && pImplData->hConv,
                                "no text converter!" )

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
                        // The conversion wasn't successfull because we haven't
                        // read enough characters.
                        if( pImplData->hContext != (rtl_TextToUnicodeContext)1 )
                        {
                            while( (nInfo&RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) != 0 )
                            {
                                rInput >> c1;
                                if( (bErr = (rInput.IsEof() || rInput.GetError())) )
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
                                if( (bErr = (rInput.IsEof() || rInput.GetError())) )
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
                        // The conversion was successfull
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

#ifndef PRODUCT
    if( pImplData->aOut.IsOpen() )
        pImplData->aOut << ByteString::ConvertFromUnicode( c,
                                                RTL_TEXTENCODING_MS_1251 );
#endif

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
        aToken.Erase();     // Token-Buffer loeschen
        nTokenValue = -1;   // Kennzeichen fuer kein Value gelesen

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
        aToken = pTokenStackPos->sToken;
        nRet = pTokenStackPos->nTokenId;
    }
    // nein, dann das aktuelle auf den Stack
    else if( SVPAR_WORKING == eState )
    {
        pTokenStackPos->sToken = aToken;
        pTokenStackPos->nTokenValue = nTokenValue;
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
    nTokenStackPos = BYTE(nTmp);

    // und die Werte zurueck
    aToken = pTokenStackPos->sToken;
    nTokenValue = pTokenStackPos->nTokenValue;

    return pTokenStackPos->nTokenId;
}

SvParser::TokenStackType* SvParser::GetStackPtr( short nCnt )
{
    BYTE nAktPos = BYTE(pTokenStackPos - pTokenStack );
    if( nCnt > 0 )
    {
        if( nCnt >= nTokenStackSize )
            nCnt = (nTokenStackSize-1);
        if( nAktPos + nCnt < nTokenStackSize )
            nAktPos += BYTE(nCnt);
        else
            nAktPos += nCnt - nTokenStackSize;
    }
    else if( nCnt < 0 )
    {
        if( -nCnt >= nTokenStackSize )
            nCnt = -nTokenStackSize+1;
        if( -nCnt <= nAktPos )
            nAktPos += BYTE(nCnt);
        else
            nAktPos += nCnt + nTokenStackSize;
    }
    return pTokenStack + nAktPos;
}

// wird fuer jedes Token gerufen, das in CallParser erkannt wird
void SvParser::NextToken( int nToken )
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
        nNextCh = pImplData->nNextCh;

        pImplData->nSaveToken = pImplData->nToken;

        rInput.Seek( pImplData->nFilePos );
    }
}

void SvParser::Continue( int nToken )
{
}

void SvParser::BuildWhichTbl( SvUShorts &rWhichMap,
                              USHORT *pWhichIds,
                              USHORT nWhichIds )
{
    USHORT aNewRange[2];

    for( USHORT nCnt = 0; nCnt < nWhichIds; ++nCnt, ++pWhichIds )
        if( *pWhichIds )
        {
            aNewRange[0] = aNewRange[1] = *pWhichIds;
            BOOL bIns = TRUE;

            // Position suchen
            for ( USHORT nOfs = 0; rWhichMap[nOfs]; nOfs += 2 )
            {
                if( *pWhichIds < rWhichMap[nOfs] - 1 )
                {
                    // neuen Range davor
                    rWhichMap.Insert( aNewRange, 2, nOfs );
                    bIns = FALSE;
                    break;
                }
                else if( *pWhichIds == rWhichMap[nOfs] - 1 )
                {
                    // diesen Range nach unten erweitern
                    rWhichMap[nOfs] = *pWhichIds;
                    bIns = FALSE;
                    break;
                }
                else if( *pWhichIds == rWhichMap[nOfs+1] + 1 )
                {
                    if( rWhichMap[nOfs+2] != 0 && rWhichMap[nOfs+2] == *pWhichIds + 1 )
                    {
                        // mit dem naechsten Bereich mergen
                        rWhichMap[nOfs+1] = rWhichMap[nOfs+3];
                        rWhichMap.Remove( nOfs+2, 2 );
                    }
                    else
                        // diesen Range nach oben erweitern
                        rWhichMap[nOfs+1] = *pWhichIds;
                    bIns = FALSE;
                    break;
                }
            }

            // einen Range hinten anhaengen
            if( bIns )
                rWhichMap.Insert( aNewRange, 2, rWhichMap.Count()-1 );
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

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
