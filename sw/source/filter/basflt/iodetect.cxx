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
#include <memory>
#include <osl/endian.h>
#include <rtl/ref.hxx>
#include <sot/storage.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <unicode/ucsdet.h>

using namespace ::com::sun::star;

static bool IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriterInstalled();
}

SwIoDetect aFilterDetect[] =
{
    SwIoDetect( FILTER_RTF ),
    SwIoDetect( FILTER_BAS ),
    SwIoDetect( sWW6 ),
    SwIoDetect( FILTER_WW8 ),
    SwIoDetect( sRtfWH ),
    SwIoDetect( sHTML ),
    SwIoDetect( sWW5 ),
    SwIoDetect( FILTER_XML ),
    SwIoDetect( FILTER_TEXT_DLG ),
    SwIoDetect( FILTER_TEXT ),
    SwIoDetect( FILTER_DOCX ),
    SwIoDetect( FILTER_MD ),
};

OUString SwIoSystem::GetSubStorageName( const SfxFilter& rFltr )
{
    // for StorageFilters also set the SubStorageName
    const OUString& rUserData = rFltr.GetUserData();
    if (rUserData == FILTER_XML ||
        rUserData == FILTER_XMLV ||
        rUserData == FILTER_XMLVW)
        return u"content.xml"_ustr;
    if (rUserData == sWW6 || rUserData == FILTER_WW8)
        return u"WordDocument"_ustr;
    return OUString();
}

std::shared_ptr<const SfxFilter> SwIoSystem::GetFilterOfFormat(std::u16string_view rFormatNm,
    const SfxFilterContainer* pCnt)
{
    SfxFilterContainer aCntSw( sSWRITER );
    SfxFilterContainer aCntSwWeb( sSWRITERWEB );
    const SfxFilterContainer* pFltCnt = pCnt ? pCnt : ( IsDocShellRegistered() ? &aCntSw : &aCntSwWeb );

    do {
        if( pFltCnt )
        {
            SfxFilterMatcher aMatcher( pFltCnt->GetName() );
            SfxFilterMatcherIter aIter( aMatcher );
            std::shared_ptr<const SfxFilter> pFilter = aIter.First();
            while ( pFilter )
            {
                if( pFilter->GetUserData() == rFormatNm )
                    return pFilter;
                pFilter = aIter.Next();
            }
        }
        if( pCnt || pFltCnt == &aCntSwWeb )
            break;
        pFltCnt = &aCntSwWeb;
    } while( true );
    return nullptr;
}

bool SwIoSystem::IsValidStgFilter( const css::uno::Reference < css::embed::XStorage >& rStg, const SfxFilter& rFilter)
{
    bool bRet = false;
    try
    {
        SotClipboardFormatId nStgFormatId = SotStorage::GetFormatID( rStg );
        bRet = rStg->isStreamElement( u"content.xml"_ustr );
        if ( bRet )
            bRet = ( nStgFormatId != SotClipboardFormatId::NONE && ( rFilter.GetFormat() == nStgFormatId ) );
    }
    catch (const css::uno::Exception& )
    {
    }

    return bRet;
}

bool SwIoSystem::IsValidStgFilter(SotStorage& rStg, const SfxFilter& rFilter)
{
    SotClipboardFormatId nStgFormatId = rStg.GetFormat();
    /*#i8409# We cannot trust the clipboard id anymore :-(*/
    if (rFilter.GetUserData() == FILTER_WW8 || rFilter.GetUserData() == sWW6)
        nStgFormatId = SotClipboardFormatId::NONE;

    bool bRet = ERRCODE_NONE == rStg.GetError() &&
        ( nStgFormatId == SotClipboardFormatId::NONE || rFilter.GetFormat() == nStgFormatId ) &&
        ( rStg.IsContained( SwIoSystem::GetSubStorageName( rFilter )) );
    if( bRet )
    {
        /* Bug 53445 - there are Excel Docs w/o ClipBoardId! */
        /* Bug 62703 - and also WinWord Docs w/o ClipBoardId! */
        if (rFilter.GetUserData() == FILTER_WW8 || rFilter.GetUserData() == sWW6)
        {
            bRet = (rStg.IsContained(u"0Table"_ustr)
                    || rStg.IsContained(u"1Table"_ustr))
                == (rFilter.GetUserData() == FILTER_WW8);
            if (bRet && !rFilter.IsAllowedAsTemplate())
            {
                rtl::Reference<SotStorageStream> xRef =
                    rStg.OpenSotStream(u"WordDocument"_ustr,
                            StreamMode::STD_READ );
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
std::shared_ptr<const SfxFilter> SwIoSystem::GetFileFilter(const OUString& rFileName)
{
    SfxFilterContainer aCntSw( sSWRITER );
    SfxFilterContainer aCntSwWeb( sSWRITERWEB );
    const SfxFilterContainer* pFCntnr = IsDocShellRegistered() ? &aCntSw : &aCntSwWeb;

    SfxFilterMatcher aMatcher( pFCntnr->GetName() );
    SfxFilterMatcherIter aIter( aMatcher );
    std::shared_ptr<const SfxFilter> pFilter = aIter.First();
    if ( !pFilter )
        return nullptr;

    if (SotStorage::IsStorageFile(rFileName))
    {
        // package storage or OLEStorage based format
        rtl::Reference<SotStorage> xStg;
        INetURLObject aObj;
        aObj.SetSmartProtocol( INetProtocol::File );
        aObj.SetSmartURL( rFileName );
        SfxMedium aMedium(aObj.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::STD_READ);

        // templates should not get precedence over "normal" filters (#i35508, #i33168)
        std::shared_ptr<const SfxFilter> pTemplateFilter;
        if (aMedium.IsStorage())
        {
            uno::Reference<embed::XStorage> const xStor = aMedium.GetStorage();
            if ( xStor.is() )
            {
                while ( pFilter )
                {
                    if (pFilter->GetUserData().startsWith("C") && IsValidStgFilter(xStor, *pFilter ))
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
                    pFilter = std::move(pTemplateFilter);
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

            if( xStg.is() && ( xStg->GetError() == ERRCODE_NONE ) )
            {
                while ( pFilter )
                {
                    if (pFilter->GetUserData().startsWith("C") && IsValidStgFilter(*xStg, *pFilter))
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
                    pFilter = std::move(pTemplateFilter);

            }
        }

        return pFilter;
    }

    return SwIoSystem::GetFilterOfFormat(FILTER_TEXT);
}

bool SwIoSystem::IsDetectableText(SvStream& rStream, sal_uLong nMaxBuf, LineEnd* pLineEnd,
                                  bool* pBom)
{
    const auto nOrigPos = rStream.Tell();
    rStream.DetectEncoding(nMaxBuf);
    const auto nNewPos = rStream.Tell();
    nMaxBuf -= nNewPos - nOrigPos;

    const rtl_TextEncoding eCharSet = rStream.GetStreamEncoding();
    bool bCR = false, bLF = false, bIsBareUnicode = false;

    if (eCharSet != RTL_TEXTENCODING_DONTKNOW)
    {
        OUString aCRLFBuffer;
        if (eCharSet != RTL_TEXTENCODING_UCS2)
        {
            aCRLFBuffer = read_uInt8s_ToOUString(rStream, nMaxBuf, eCharSet);
        }
        else
        {
            aCRLFBuffer = read_uInt16s_ToOUString(rStream, nMaxBuf / 2);
        }
        rStream.Seek(nNewPos);

        for (sal_Int32 i = 0; i < aCRLFBuffer.getLength(); ++i)
        {
            switch (aCRLFBuffer[i])
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
        OString aCRLFBuffer = read_uInt8s_ToOString(rStream, nMaxBuf);
        rStream.Seek(nNewPos);
        for (sal_Int32 i = 0; i < aCRLFBuffer.getLength(); ++i)
        {
            switch (aCRLFBuffer[i])
            {
                case 0x0:
                    if (i + 1 < aCRLFBuffer.getLength() && !aCRLFBuffer[i + 1])
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

    if (pLineEnd)
        *pLineEnd = eLineEnd;
    if (pBom)
        *pBom = nNewPos != nOrigPos;

    return !bIsBareUnicode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
