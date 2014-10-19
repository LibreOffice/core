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

#include <iodetect.hxx>
#include <boost/scoped_array.hpp>
#include <osl/endian.h>
#include <sot/storage.hxx>
#include <svtools/parhtml.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>


using namespace ::com::sun::star;

static bool IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriter();
}

SwIoDetect aFilterDetect[] =
{
    SwIoDetect( FILTER_RTF ),
    SwIoDetect( FILTER_BAS ),
    SwIoDetect( sWW6 ),
    SwIoDetect( FILTER_WW8 ),
    SwIoDetect( sRtfWH ),
    SwIoDetect( sHTML ),
    SwIoDetect( sWW1 ),
    SwIoDetect( sWW5 ),
    SwIoDetect( FILTER_XML ),
    SwIoDetect( FILTER_TEXT_DLG ),
    SwIoDetect( FILTER_TEXT )
};

OUString SwIoDetect::IsReader(const sal_Char* pHeader, sal_uLong nLen_) const
{
    // Filter recognition
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
        bool fComplexGet()      { return static_cast< bool >((fFlagsGet() >> 2) & 1); }
    };

    bool bRet = false;
    if ( sHTML == sName )
        bRet = HTMLParser::IsHTMLFormat( pHeader, true, RTL_TEXTENCODING_DONTKNOW );
    else if ( FILTER_RTF == sName )
        bRet = 0 == strncmp( "{\\rtf", pHeader, 5 );
    else if ( sWW5 == sName )
    {
        W1_FIB *pW1Header = (W1_FIB*)pHeader;
        if (pW1Header->wIdentGet() == 0xA5DC && pW1Header->nFibGet() == 0x65)
            bRet = true; /*WW5*/
        else if (pW1Header->wIdentGet() == 0xA5DB && pW1Header->nFibGet() == 0x2D)
            bRet = true; /*WW2*/
    }
    else if ( sWW1 == sName )
    {
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA59C
                    && ((W1_FIB*)pHeader)->nFibGet() == 0x21)
                && ((W1_FIB*)pHeader)->fComplexGet() == false);
    }
    else if ( FILTER_TEXT == sName )
        bRet = SwIoSystem::IsDetectableText(pHeader, nLen_);
    else if ( FILTER_TEXT_DLG == sName)
        bRet = SwIoSystem::IsDetectableText( pHeader, nLen_, 0, 0, 0, true);
    return bRet ? sName : OUString();
}

const OUString SwIoSystem::GetSubStorageName( const SfxFilter& rFltr )
{
    // for StorageFilters also set the SubStorageName
    const OUString& rUserData = rFltr.GetUserData();
    if (rUserData == FILTER_XML ||
        rUserData == FILTER_XMLV ||
        rUserData == FILTER_XMLVW)
        return OUString("content.xml");
    if (rUserData == sWW6 || rUserData == FILTER_WW8)
        return OUString("WordDocument");
    return OUString();
}

const SfxFilter* SwIoSystem::GetFilterOfFormat(const OUString& rFmtNm,
    const SfxFilterContainer* pCnt)
{
    SfxFilterContainer aCntSw( OUString(sSWRITER) );
    SfxFilterContainer aCntSwWeb( OUString(sSWRITERWEB) );
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
    } while( true );
    return 0;
}

bool SwIoSystem::IsValidStgFilter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const SfxFilter& rFilter)
{
    bool bRet = false;
    try
    {
        sal_uLong nStgFmtId = SotStorage::GetFormatID( rStg );
        bRet = rStg->isStreamElement( OUString("content.xml") );
        if ( bRet )
            bRet = ( nStgFmtId && ( rFilter.GetFormat() == nStgFmtId ) );
    }
    catch (const com::sun::star::uno::Exception& )
    {
    }

    return bRet;
}

bool SwIoSystem::IsValidStgFilter(SotStorage& rStg, const SfxFilter& rFilter)
{
    sal_uLong nStgFmtId = rStg.GetFormat();
    /*#i8409# We cannot trust the clipboard id anymore :-(*/
    if (rFilter.GetUserData() == FILTER_WW8 || rFilter.GetUserData() == sWW6)
        nStgFmtId = 0;

    bool bRet = SVSTREAM_OK == rStg.GetError() &&
        ( !nStgFmtId || rFilter.GetFormat() == nStgFmtId ) &&
        ( rStg.IsContained( SwIoSystem::GetSubStorageName( rFilter )) );
    if( bRet )
    {
        /* Bug 53445 - there are Excel Docs w/o ClipBoardId! */
        /* Bug 62703 - and also WinWord Docs w/o ClipBoardId! */
        if (rFilter.GetUserData() == FILTER_WW8 || rFilter.GetUserData() == sWW6)
        {
            bRet = (rStg.IsContained(OUString("0Table"))
                    || rStg.IsContained(OUString("1Table")))
                == (rFilter.GetUserData() == FILTER_WW8);
            if (bRet && !rFilter.IsAllowedAsTemplate())
            {
                SotStorageStreamRef xRef =
                    rStg.OpenSotStream(OUString("WordDocument"),
                            STREAM_STD_READ | STREAM_NOCREATE );
                xRef->Seek(10);
                sal_uInt8 nByte;
                xRef->ReadUChar( nByte );
                bRet = !(nByte & 1);
            }
        }
    }
    return bRet;
}

// Check the type of the stream (file) by searching for corresponding set of bytes.
// If no known type is found, return ASCII for now!
// Returns the internal FilterName.
const SfxFilter* SwIoSystem::GetFileFilter(const OUString& rFileName)
{
    SfxFilterContainer aCntSw( OUString(sSWRITER) );
    SfxFilterContainer aCntSwWeb( OUString(sSWRITERWEB) );
    const SfxFilterContainer* pFCntnr = IsDocShellRegistered() ? &aCntSw : &aCntSwWeb;

    SfxFilterMatcher aMatcher( pFCntnr->GetName() );
    SfxFilterMatcherIter aIter( aMatcher );
    const SfxFilter* pFilter = aIter.First();
    if ( !pFilter )
        return 0;

    if (SotStorage::IsStorageFile(rFileName))
    {
        // package storage or OLEStorage based format
        SotStorageRef xStg;
        INetURLObject aObj;
        aObj.SetSmartProtocol( INET_PROT_FILE );
        aObj.SetSmartURL( rFileName );
        SfxMedium aMedium(aObj.GetMainURL(INetURLObject::NO_DECODE), STREAM_STD_READ);

        // templates should not get precedence over "normal" filters (#i35508, #i33168)
        const SfxFilter* pTemplateFilter = 0;
        if (aMedium.IsStorage())
        {
            uno::Reference<embed::XStorage> const xStor = aMedium.GetStorage();
            if ( xStor.is() )
            {
                while ( pFilter )
                {
                    if( (!pFilter->GetUserData().isEmpty()) && 'C' == pFilter->GetUserData()[0] && IsValidStgFilter( xStor, *pFilter ) )
                    {
                        if (pFilter->IsOwnTemplateFormat())
                        {
                            // found template filter; maybe there's a "normal" one also
                            pTemplateFilter = pFilter;
                        }
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
            try
            {
                SvStream *const pStream = aMedium.GetInStream();
                if ( pStream && SotStorage::IsStorageFile(pStream) )
                    xStg = new SotStorage( pStream, false );
            }
            catch (const css::ucb::ContentCreationException &)
            {
            }

            if( xStg.Is() && ( xStg->GetError() == SVSTREAM_OK ) )
            {
                while ( pFilter )
                {
                    if( 'C' == pFilter->GetUserData()[0] && IsValidStgFilter( *xStg, *pFilter ) )
                    {
                        if (pFilter->IsOwnTemplateFormat())
                        {
                            // found template filter; maybe there's a "normal" one also
                            pTemplateFilter = pFilter;
                        }
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

    return SwIoSystem::GetFilterOfFormat(OUString::createFromAscii(FILTER_TEXT), 0);
}

bool SwIoSystem::IsDetectableText(const sal_Char* pBuf, sal_uLong &rLen,
    rtl_TextEncoding *pCharSet, bool *pSwap, LineEnd *pLineEnd, bool bEncodedFilter)
{
    bool bSwap = false;
    rtl_TextEncoding eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bool bLE = true;
    /*See if it's a known unicode type*/
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
                for(sal_uLong n = 0; n < nNewLen; ++n, pF+=2, pN+=2 )
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
                        return false;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
