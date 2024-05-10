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

#ifdef __sun
#include <ctime>
#endif

#include <sot/exchange.hxx>
#include <sot/storage.hxx>
#include <comphelper/fileformat.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/objsh.hxx>
#include <utility>

using namespace ::com::sun::star;

SfxFilter::SfxFilter( OUString aProvider, OUString aFilterName ) :
    maFilterName(std::move(aFilterName)),
    maProvider(std::move(aProvider)),
    nFormatType(SfxFilterFlags::NONE),
    nVersion(0),
    lFormat(SotClipboardFormatId::NONE),
    mbEnabled(true)
{
}

SfxFilter::SfxFilter( OUString aName,
                      std::u16string_view rWildCard,
                      SfxFilterFlags nType,
                      SotClipboardFormatId lFmt,
                      OUString aTypNm,
                      OUString _aMimeType,
                      OUString aUsrDat,
                      OUString _aServiceName,
                      bool bEnabled ):
    aWildCard(rWildCard, ';'),
    aTypeName(std::move(aTypNm)),
    aUserData(std::move(aUsrDat)),
    aServiceName(std::move(_aServiceName)),
    aMimeType(std::move(_aMimeType)),
    maFilterName(std::move(aName)),
    aUIName(maFilterName),
    nFormatType(nType),
    nVersion(SOFFICE_FILEFORMAT_50),
    lFormat(lFmt),
    mbEnabled(bEnabled)
{
    const OUString aExts = GetWildcard().getGlob();
    sal_Int32 nLen{ aExts.getLength() };
    if (nLen<=0)
        return;

    // truncate to first empty extension
    if (aExts[0]==';')
    {
        aWildCard.setGlob(u"");
        return;
    }
    const sal_Int32 nIdx{ aExts.indexOf(";;") };
    if (nIdx>0)
        nLen = nIdx;
    else if (aExts[nLen-1]==';')
        --nLen;
    if (nLen<aExts.getLength())
        aWildCard.setGlob(aExts.subView(0, nLen));
}

SfxFilter::~SfxFilter()
{
}

OUString SfxFilter::GetDefaultExtension() const
{
    return GetWildcard().getGlob().getToken(0, ';');
}


OUString SfxFilter::GetSuffixes() const
{
    OUString aRet = GetWildcard().getGlob();
    aRet = aRet.replaceAll( "*.", "" );
    aRet = aRet.replaceAll( ";", "," );
    return aRet;
}

std::shared_ptr<const SfxFilter> SfxFilter::GetDefaultFilter( std::u16string_view rName )
{
    return SfxFilterContainer::GetDefaultFilter_Impl( rName );
}

std::shared_ptr<const SfxFilter> SfxFilter::GetDefaultFilterFromFactory( const OUString& rFact )
{
    return GetDefaultFilter( SfxObjectShell::GetServiceNameFromFactory( rFact ) );
}

std::shared_ptr<const SfxFilter> SfxFilter::GetFilterByName( const OUString& rName )
{
    SfxFilterMatcher aMatch;
    return aMatch.GetFilter4FilterName( rName, SfxFilterFlags::NONE, SfxFilterFlags::NONE );
}

OUString SfxFilter::GetTypeFromStorage( const SotStorage& rStg )
{
    const char* pType=nullptr;
    if ( rStg.IsStream( u"WordDocument"_ustr ) )
    {
        if ( rStg.IsStream( u"0Table"_ustr ) || rStg.IsStream( u"1Table"_ustr ) )
            pType = "writer_MS_Word_97";
        else
            pType = "writer_MS_Word_95";
    }
    else if ( rStg.IsStream( u"Book"_ustr ) )
    {
        pType = "calc_MS_Excel_95";
    }
    else if ( rStg.IsStream( u"Workbook"_ustr ) )
    {
        pType = "calc_MS_Excel_97";
    }
    else if ( rStg.IsStream( u"PowerPoint Document"_ustr ) )
    {
        pType = "impress_MS_PowerPoint_97";
    }
    else if ( rStg.IsStream( u"Equation Native"_ustr ) )
    {
        pType = "math_MathType_3x";
    }
    else
    {
        SotClipboardFormatId nClipId = const_cast<SotStorage&>(rStg).GetFormat();
        if ( nClipId != SotClipboardFormatId::NONE )
        {
            std::shared_ptr<const SfxFilter> pFilter = SfxFilterMatcher().GetFilter4ClipBoardId( nClipId );
            if ( pFilter )
                return pFilter->GetTypeName();
        }
    }

    return pType ? OUString::createFromAscii(pType) : OUString();
}

OUString SfxFilter::GetTypeFromStorage(
    const uno::Reference<embed::XStorage>& xStorage )
{
    SfxFilterMatcher aMatcher;

    css::uno::Reference< css::beans::XPropertySet > xProps( xStorage, css::uno::UNO_QUERY );
    if ( xProps.is() )
    {
        OUString aMediaType;
        xProps->getPropertyValue(u"MediaType"_ustr) >>= aMediaType;
        if ( !aMediaType.isEmpty() )
        {
            css::datatransfer::DataFlavor aDataFlavor;
            aDataFlavor.MimeType = aMediaType;
            SotClipboardFormatId nClipId = SotExchange::GetFormat( aDataFlavor );
            if ( nClipId != SotClipboardFormatId::NONE )
            {
                SfxFilterFlags const nMust = SfxFilterFlags::IMPORT;
                // template filters shouldn't be detected if not explicitly asked for
                SfxFilterFlags const nDont = SFX_FILTER_NOTINSTALLED | SfxFilterFlags::TEMPLATEPATH;

                // get filter from storage MediaType
                std::shared_ptr<const SfxFilter> pFilter = aMatcher.GetFilter4ClipBoardId( nClipId, nMust, nDont );
                if ( !pFilter )
                    // template filter is asked for , but there isn't one; so at least the "normal" format should be detected
                    // or storage *is* a template, but bTemplate is not set
                    pFilter = aMatcher.GetFilter4ClipBoardId( nClipId );

                if ( pFilter )
                {
                    return pFilter->GetTypeName();
                }
            }
        }
    }

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
