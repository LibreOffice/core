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


#include <iodetect.hxx>
#include <boost/scoped_array.hpp>
#include <osl/endian.h>
#include <sot/storage.hxx>
#include <svtools/parhtml.hxx>
#include <tools/urlobj.hxx>

bool IsDocShellRegistered();

SwIoDetect aFilterDetect[] =
{
    SwIoDetect( FILTER_RTF,      STRING_LEN ),
    SwIoDetect( FILTER_BAS,      STRING_LEN ),
    SwIoDetect( sWW6,            STRING_LEN ),
    SwIoDetect( FILTER_WW8,      STRING_LEN ),
    SwIoDetect( sRtfWH,          STRING_LEN ),
    SwIoDetect( sHTML,           4 ),
    SwIoDetect( sWW1,            STRING_LEN ),
    SwIoDetect( sWW5,            STRING_LEN ),
    SwIoDetect( FILTER_XML,      4 ),
    SwIoDetect( FILTER_TEXT_DLG, 8 ),
    SwIoDetect( FILTER_TEXT,     4 )
};

const sal_Char* SwIoDetect::IsReader(const sal_Char* pHeader, sal_uLong nLen_,
    const String & /*rFileName*/, const String& /*rUserData*/) const
{
    // Filter erkennung
    struct W1_FIB
    {
        SVBT16 wIdent;      // 0x0 int magic number
        SVBT16 nFib;        // 0x2 FIB version written
        SVBT16 nProduct;    // 0x4 product version written by
        SVBT16 nlocale;     // 0x6 language stamp---localized version;
        SVBT16 pnNext;      // 0x8
        SVBT16 fFlags;

        sal_uInt16 nFibGet()    { return SVBT16ToShort(nFib); }
        sal_uInt16 wIdentGet()  { return SVBT16ToShort(wIdent); }
        sal_uInt16 fFlagsGet()  { return SVBT16ToShort(fFlags); }
        // SVBT16 fComplex :1;// 0004 when 1, file is in complex, fast-saved format.
        sal_Bool fComplexGet()  { return static_cast< sal_Bool >((fFlagsGet() >> 2) & 1); }
    };

    int bRet = sal_False;
    rtl::OString aName( pName );
    if ( sHTML == aName )
        bRet = HTMLParser::IsHTMLFormat( pHeader, sal_True, RTL_TEXTENCODING_DONTKNOW );
    else if ( FILTER_RTF == aName )
        bRet = 0 == strncmp( "{\\rtf", pHeader, 5 );
    else if ( sWW5 == aName )
    {
        W1_FIB *pW1Header = (W1_FIB*)pHeader;
        if (pW1Header->wIdentGet() == 0xA5DC && pW1Header->nFibGet() == 0x65)
            bRet = true; /*WW5*/
        else if (pW1Header->wIdentGet() == 0xA5DB && pW1Header->nFibGet() == 0x2D)
            bRet = true; /*WW2*/
    }
    else if ( sWW1 == aName )
    {
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA59C
                    && ((W1_FIB*)pHeader)->nFibGet() == 0x21)
                && ((W1_FIB*)pHeader)->fComplexGet() == 0);
    }
    else if ( FILTER_TEXT == aName )
        bRet = SwIoSystem::IsDetectableText(pHeader, nLen_);
    else if ( FILTER_TEXT_DLG == aName)
        bRet = SwIoSystem::IsDetectableText( pHeader, nLen_, 0, 0, 0, true);
    return bRet ? pName : 0;
}

const String SwIoSystem::GetSubStorageName( const SfxFilter& rFltr )
{
    /* bei den StorageFiltern noch den SubStorageNamen setzen */
    const rtl::OUString& rUserData = rFltr.GetUserData();
    if (rUserData == FILTER_XML ||
        rUserData == FILTER_XMLV ||
        rUserData == FILTER_XMLVW)
        return rtl::OUString("content.xml");
    if (rUserData == sWW6 || rUserData == FILTER_WW8)
        return rtl::OUString("WordDocument");
    return rtl::OUString();
}

const SfxFilter* SwIoSystem::GetFilterOfFormat(const String& rFmtNm,
    const SfxFilterContainer* pCnt)
{
    SfxFilterContainer aCntSw( rtl::OUString(sSWRITER) );
    SfxFilterContainer aCntSwWeb( rtl::OUString(sSWRITERWEB) );
    const SfxFilterContainer* pFltCnt = pCnt ? pCnt : ( IsDocShellRegistered() ? &aCntSw : &aCntSwWeb );

    do {
        if( pFltCnt )
        {
            SfxFilterMatcher aMatcher( pFltCnt->GetName() );
            SfxFilterMatcherIter aIter( aMatcher );
            const SfxFilter* pFilter = aIter.First();
            while ( pFilter )
            {
                if( pFilter->GetUserData().equals(rFmtNm) )
                    return pFilter;
                pFilter = aIter.Next();
            }
        }
        if( pCnt || pFltCnt == &aCntSwWeb )
            break;
        pFltCnt = &aCntSwWeb;
    } while( sal_True );
    return 0;
}

sal_Bool SwIoSystem::IsValidStgFilter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const SfxFilter& rFilter)
{
    sal_Bool bRet = sal_False;
    try
    {
        sal_uLong nStgFmtId = SotStorage::GetFormatID( rStg );
        bRet = rStg->isStreamElement( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")) );
        if ( bRet )
            bRet = ( nStgFmtId && ( rFilter.GetFormat() == nStgFmtId ) );
    }
    catch ( com::sun::star::uno::Exception& )
    {
    }

    return bRet;
}

sal_Bool SwIoSystem::IsValidStgFilter(SotStorage& rStg, const SfxFilter& rFilter)
{
    sal_uLong nStgFmtId = rStg.GetFormat();
    /*#i8409# We cannot trust the clipboard id anymore :-(*/
    if (rFilter.GetUserData() == FILTER_WW8 || rFilter.GetUserData() == sWW6)
        nStgFmtId = 0;

    sal_Bool bRet = SVSTREAM_OK == rStg.GetError() &&
        ( !nStgFmtId || rFilter.GetFormat() == nStgFmtId ) &&
        ( rStg.IsContained( SwIoSystem::GetSubStorageName( rFilter )) );
    if( bRet )
    {
        /* Bug 53445 - es gibt Excel Docs ohne ClipBoardId! */
        /* Bug 62703 - und auch WinWord Docs ohne ClipBoardId! */
        if (rFilter.GetUserData() == FILTER_WW8 || rFilter.GetUserData() == sWW6)
        {
            bRet = !((rStg.IsContained( rtl::OUString("0Table")) ||
                        rStg.IsContained( rtl::OUString("1Table"))) ^
                    (rFilter.GetUserData() == FILTER_WW8));
            if (bRet && !rFilter.IsAllowedAsTemplate())
            {
                SotStorageStreamRef xRef =
                    rStg.OpenSotStream(rtl::OUString("WordDocument"),
                            STREAM_STD_READ | STREAM_NOCREATE );
                xRef->Seek(10);
                sal_uInt8 nByte;
                *xRef >> nByte;
                bRet = !(nByte & 1);
            }
        }
    }
    return bRet;
}

void TerminateBuffer(sal_Char *pBuffer, sal_uLong nBytesRead, sal_uLong nBufferLen)
{
    OSL_ENSURE(nBytesRead <= nBufferLen - 2,
            "what you read must be less than the max + null termination");
    OSL_ENSURE(!(nBufferLen & 0x00000001), "nMaxReadBuf must be an even number");
    if (nBytesRead <= nBufferLen - 2)
    {
        pBuffer[nBytesRead] = '\0';
        pBuffer[nBytesRead+1] = '\0';
        if (nBytesRead & 0x00000001)
            pBuffer[nBytesRead+2] = '\0';
    }
}

/* Feststellen ob das File in dem entsprechenden Format vorliegt. */
/* Z.z werden nur unsere eigene Filter unterstuetzt               */
sal_Bool SwIoSystem::IsFileFilter( SfxMedium& rMedium, const String& rFmtName,
        const SfxFilter** ppFilter )
{
    sal_Bool bRet = sal_False;

    SfxFilterContainer aCntSw( rtl::OUString(sSWRITER) );
    SfxFilterContainer aCntSwWeb( rtl::OUString(sSWRITERWEB) );
    const SfxFilterContainer& rFltContainer = IsDocShellRegistered() ? aCntSw : aCntSwWeb;

    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStor;
    SotStorageRef xStg;
    if (rMedium.IsStorage())
        xStor = rMedium.GetStorage();
    else
    {
        SvStream* pStream = rMedium.GetInStream();
        if ( pStream && SotStorage::IsStorageFile(pStream) )
            xStg = new SotStorage( pStream, sal_False );
    }

    SfxFilterMatcher aMatcher( rFltContainer.GetName() );
    SfxFilterMatcherIter aIter( aMatcher );
    const SfxFilter* pFltr = aIter.First();
    while ( pFltr )
    {
        if( pFltr->GetUserData().equals(rFmtName) )
        {
            const rtl::OUString& rUserData = pFltr->GetUserData();
            if( 'C' == rUserData[0] )
            {
                if ( xStor.is() )
                    bRet = IsValidStgFilter( xStor, *pFltr );
                else if ( xStg.Is() )
                    bRet = xStg.Is() && IsValidStgFilter( *xStg, *pFltr );
                bRet = bRet && (pFltr->GetUserData().equals(rFmtName));
            }
            else if( !xStg.Is() && !xStor.is() )
            {
                SvStream* pStrm = rMedium.GetInStream();
                if( pStrm && !pStrm->GetError() )
                {
                    sal_Char aBuffer[4098];
                    const sal_uLong nMaxRead = sizeof(aBuffer) - 2;
                    sal_uLong nBytesRead = pStrm->Read(aBuffer, nMaxRead);
                    pStrm->Seek(STREAM_SEEK_TO_BEGIN);
                    TerminateBuffer(aBuffer, nBytesRead, sizeof(aBuffer));
                    for (sal_uInt16 i = 0; i < MAXFILTER; ++i)
                    {
                        if (aFilterDetect[i].IsFilter(rFmtName))
                        {
                            bRet = 0 != aFilterDetect[i].IsReader( aBuffer, nBytesRead,
                                    rMedium.GetPhysicalName(), rUserData );
                            break;
                        }
                    }
                }
            }

            if( bRet && ppFilter  )
                *ppFilter = pFltr;
        }

        pFltr = aIter.Next();
    }

    return bRet;
}

/* die Methode stellt fest, von welchem Typ der stream (File) ist.        */
/* Es wird versucht, eine dem Filter entsprechende Byte-Folge zu finden.  */
/* Wird kein entsprechender gefunden, wird zur Zeit der ASCII-Reader      */
/* returnt !! Der Returnwert ist der interne Filtername!                  */
/* rPrefFltName ist der interne Name des Filters, den der Benutzer im     */
/* Open-Dialog eingestellt hat.                                           */
const SfxFilter* SwIoSystem::GetFileFilter(const String& rFileName,
    const String& rPrefFltName, SfxMedium* pMedium)
{
    SfxFilterContainer aCntSw( rtl::OUString(sSWRITER) );
    SfxFilterContainer aCntSwWeb( rtl::OUString(sSWRITERWEB) );
    const SfxFilterContainer* pFCntnr = IsDocShellRegistered() ? &aCntSw : &aCntSwWeb;

    if( !pFCntnr )
        return 0;

    SfxFilterMatcher aMatcher( pFCntnr->GetName() );
    SfxFilterMatcherIter aIter( aMatcher );
    const SfxFilter* pFilter = aIter.First();
    if ( !pFilter )
        return 0;

    if( pMedium ? ( pMedium->IsStorage() || SotStorage::IsStorageFile( pMedium->GetInStream() ) ) : SotStorage::IsStorageFile( rFileName ) )
    {
        // package storage or OLEStorage based format
        SotStorageRef xStg;
        if (!pMedium )
        {
            INetURLObject aObj;
            aObj.SetSmartProtocol( INET_PROT_FILE );
            aObj.SetSmartURL( rFileName );
            pMedium = new SfxMedium( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ );
        }

        // templates should not get precedence over "normal" filters (#i35508, #i33168)
        const SfxFilter* pTemplateFilter = 0;
        const SfxFilter* pOldFilter = pFCntnr->GetFilter4FilterName( rPrefFltName );
        sal_Bool bLookForTemplate = pOldFilter && pOldFilter->IsOwnTemplateFormat();
        if ( pMedium->IsStorage() )
        {
            com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStor = pMedium->GetStorage();
            if ( xStor.is() )
            {
                while ( pFilter )
                {
                    if( 'C' == pFilter->GetUserData()[0] && IsValidStgFilter( xStor, *pFilter ) )
                    {
                        if ( pFilter->IsOwnTemplateFormat() && !bLookForTemplate )
                            // found template filter; maybe there's a "normal" one also
                            pTemplateFilter = pFilter;
                        else
                            return pFilter;
                    }

                    pFilter = aIter.Next();
                }

                // there's only a template filter that could be found
                if ( pTemplateFilter )
                    pFilter = pTemplateFilter;
            }
        }
        else
        {
            SvStream* pStream = pMedium->GetInStream();
            if ( pStream && SotStorage::IsStorageFile(pStream) )
                xStg = new SotStorage( pStream, sal_False );

            if( xStg.Is() && ( xStg->GetError() == SVSTREAM_OK ) )
            {
                while ( pFilter )
                {
                    if( 'C' == pFilter->GetUserData()[0] && IsValidStgFilter( *xStg, *pFilter ) )
                    {
                        if ( pFilter->IsOwnTemplateFormat() && !bLookForTemplate )
                            // found template filter; maybe there's a "normal" one also
                            pTemplateFilter = pFilter;
                        else
                            return pFilter;
                    }

                    pFilter = aIter.Next();
                }

                // there's only a template filter that could be found
                if ( pTemplateFilter )
                    pFilter = pTemplateFilter;

            }
        }

        return pFilter;
    }

    sal_Char aBuffer[4098];
    const sal_uLong nMaxRead = sizeof(aBuffer) - 2;
    sal_uLong nBytesRead = 0;
    if (pMedium)
    {
        SvStream* pIStrm = pMedium->GetInStream();
        if( !pIStrm || SVSTREAM_OK != pIStrm->GetError() )
            return 0;
        sal_uLong nCurrPos = pIStrm->Tell();
        nBytesRead = pIStrm->Read(aBuffer, nMaxRead);
        pIStrm->Seek( nCurrPos );
    }

    TerminateBuffer(aBuffer, nBytesRead, sizeof(aBuffer));


    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    /* suche nach dem bestimmten Filter, falls kein entsprechender        */
    /* gefunden wird, so wird der ASCII-Filter returnt.                   */
    /* Gibt es Filter ohne einen Identifizierungs-String, so werden diese */
    /* nie erkannt und es wird auch der ASCII-Filter returnt.             */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    {
        const SfxFilter* pFilterTmp = 0;
        const sal_Char* pNm;
        for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
        {
            String sEmptyUserData;
            pNm = aFilterDetect[n].IsReader(aBuffer, nBytesRead, rFileName, sEmptyUserData);
            pFilterTmp = pNm ? SwIoSystem::GetFilterOfFormat(rtl::OUString::createFromAscii(pNm), pFCntnr) : 0;
            if (pNm && pFilterTmp)
            {
                return pFilterTmp;
            }
        }
    }

    /* Ok, bis jetzt kein Filter gefunden, also befrage mal die */
    /* "WORD 4 WORD" Filter                                     */
    if( rFileName.Len() )
    {
        if( pMedium )
            pMedium->CloseInStream();

    }
    return SwIoSystem::GetTextFilter( aBuffer, nBytesRead);
}

bool SwIoSystem::IsDetectableText(const sal_Char* pBuf, sal_uLong &rLen,
    CharSet *pCharSet, bool *pSwap, LineEnd *pLineEnd, bool bEncodedFilter)
{
    bool bSwap = false;
    CharSet eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bool bLE = true;
    /*See if its a known unicode type*/
    if (rLen >= 2)
    {
        sal_uLong nHead=0;
        if (rLen > 2 && sal_uInt8(pBuf[0]) == 0xEF && sal_uInt8(pBuf[1]) == 0xBB &&
            sal_uInt8(pBuf[2]) == 0xBF)
        {
            eCharSet = RTL_TEXTENCODING_UTF8;
            nHead = 3;
        }
        else if (sal_uInt8(pBuf[0]) == 0xFE && sal_uInt8(pBuf[1]) == 0xFF)
        {
            eCharSet = RTL_TEXTENCODING_UCS2;
            bLE = false;
            nHead = 2;
        }
        else if (sal_uInt8(pBuf[1]) == 0xFE && sal_uInt8(pBuf[0]) == 0xFF)
        {
            eCharSet = RTL_TEXTENCODING_UCS2;
            nHead = 2;
        }
        pBuf+=nHead;
        rLen-=nHead;
    }

    bool bCR = false, bLF = false, bIsBareUnicode = false;

    if (eCharSet != RTL_TEXTENCODING_DONTKNOW)
    {
        boost::scoped_array<sal_Unicode> aWork(new sal_Unicode[rLen+1]);
        sal_Unicode *pNewBuf = aWork.get();
        sal_Size nNewLen;
        if (eCharSet != RTL_TEXTENCODING_UCS2)
        {
            nNewLen = rLen;
            rtl_TextToUnicodeConverter hConverter =
                rtl_createTextToUnicodeConverter(eCharSet);
            rtl_TextToUnicodeContext hContext =
                rtl_createTextToUnicodeContext(hConverter);

            sal_Size nCntBytes;
            sal_uInt32 nInfo;
            nNewLen = rtl_convertTextToUnicode( hConverter, hContext, pBuf,
                rLen, pNewBuf, nNewLen,
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                  RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                  RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT), &nInfo, &nCntBytes);

            rtl_destroyTextToUnicodeContext(hConverter, hContext);
            rtl_destroyTextToUnicodeConverter(hConverter);
        }
        else
        {
            nNewLen = rLen/2;
            memcpy(pNewBuf, pBuf, rLen);
#ifdef OSL_LITENDIAN
            bool bNativeLE = true;
#else
            bool bNativeLE = false;
#endif
            if (bLE != bNativeLE)
            {
                bSwap = true;
                sal_Char* pF = (sal_Char*)pNewBuf;
                sal_Char* pN = pF+1;
                for(xub_StrLen n = 0; n < nNewLen; ++n, pF+=2, pN+=2)
                {
                    sal_Char c = *pF;
                    *pF = *pN;
                    *pN = c;
                }
            }
        }

        for (sal_uLong nCnt = 0; nCnt < nNewLen; ++nCnt, ++pNewBuf)
        {
            switch (*pNewBuf)
            {
                case 0xA:
                    bLF = true;
                    break;
                case 0xD:
                    bCR = true;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        for( sal_uLong nCnt = 0; nCnt < rLen; ++nCnt, ++pBuf )
        {
            switch (*pBuf)
            {
                case 0x0:
                    if( nCnt + 1 < rLen && !*(pBuf+1) )
                        return 0;
                    bIsBareUnicode = true;
                    break;
                case 0xA:
                    bLF = true;
                    break;
                case 0xD:
                    bCR = true;
                    break;
                case 0xC:
                case 0x1A:
                case 0x9:
                    break;
                default:
                    break;
            }
        }
    }

    LineEnd eSysLE = GetSystemLineEnd();
    LineEnd eLineEnd;
    if (!bCR && !bLF)
        eLineEnd = eSysLE;
    else
        eLineEnd = bCR ? ( bLF ? LINEEND_CRLF : LINEEND_CR ) : LINEEND_LF;

    if (pCharSet)
        *pCharSet = eCharSet;
    if (pSwap)
        *pSwap = bSwap;
    if (pLineEnd)
        *pLineEnd = eLineEnd;

    return bEncodedFilter || (!bIsBareUnicode && eSysLE == eLineEnd);
}

const SfxFilter* SwIoSystem::GetTextFilter( const sal_Char* pBuf, sal_uLong nLen)
{
    bool bAuto = IsDetectableText(pBuf, nLen);
    const sal_Char* pNm = bAuto ? FILTER_TEXT : FILTER_TEXT_DLG;
    return SwIoSystem::GetFilterOfFormat( rtl::OUString::createFromAscii(pNm), 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
