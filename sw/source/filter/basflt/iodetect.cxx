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
    SwIoDetect( sWW5 ),
    SwIoDetect( FILTER_XML ),
    SwIoDetect( FILTER_TEXT_DLG ),
    SwIoDetect( FILTER_TEXT )
};

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

const SfxFilter* SwIoSystem::GetFilterOfFormat(const OUString& rFormatNm,
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
                if( pFilter->GetUserData().equals(rFormatNm) )
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
        SotClipboardFormatId nStgFormatId = SotStorage::GetFormatID( rStg );
        bRet = rStg->isStreamElement( OUString("content.xml") );
        if ( bRet )
            bRet = ( nStgFormatId != SotClipboardFormatId::NONE && ( rFilter.GetFormat() == nStgFormatId ) );
    }
    catch (const com::sun::star::uno::Exception& )
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

    bool bRet = SVSTREAM_OK == rStg.GetError() &&
        ( nStgFormatId == SotClipboardFormatId::NONE || rFilter.GetFormat() == nStgFormatId ) &&
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
                tools::SvRef<SotStorageStream> xRef =
                    rStg.OpenSotStream(OUString("WordDocument"),
                            STREAM_STD_READ | StreamMode::NOCREATE );
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
        tools::SvRef<SotStorage> xStg;
        INetURLObject aObj;
        aObj.SetSmartProtocol( INetProtocol::File );
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
                    pFilter = pTemplateFilter;

            }
        }

        return pFilter;
    }

    return SwIoSystem::GetFilterOfFormat(FILTER_TEXT, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
