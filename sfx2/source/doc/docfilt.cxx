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

#ifdef SOLARIS
#include <ctime>
#endif

#include <string>
#include <sot/exchange.hxx>
#include <sot/storage.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/objsh.hxx>

using namespace ::com::sun::star;

SfxFilter::SfxFilter( const OUString& rProvider, const OUString &rFilterName ) :
    maFilterName(rFilterName),
    maProvider(rProvider),
    nFormatType(SfxFilterFlags::NONE),
    nVersion(0),
    lFormat(SotClipboardFormatId::NONE),
    nDocIcon(0)
{
}

SfxFilter::SfxFilter( const OUString &rName,
                      const OUString &rWildCard,
                      SfxFilterFlags nType,
                      SotClipboardFormatId lFmt,
                      const OUString &rTypNm,
                      sal_uInt16 nIcon,
                      const OUString &rMimeType,
                      const OUString &rUsrDat,
                      const OUString &rServiceName ):
    aWildCard(rWildCard, ';'),
    aTypeName(rTypNm),
    aUserData(rUsrDat),
    aServiceName(rServiceName),
    aMimeType(rMimeType),
    maFilterName(rName),
    aUIName(maFilterName),
    nFormatType(nType),
    nVersion(SOFFICE_FILEFORMAT_50),
    lFormat(lFmt),
    nDocIcon(nIcon)
{
    OUString aExts = GetWildcard().getGlob();
    OUString aShort, aLong;
    OUString aRet;
    sal_uInt16 nMaxLength = USHRT_MAX;
    OUString aTest;
    sal_uInt16 nPos = 0;
    while (!(aRet = aExts.getToken(nPos++, ';')).isEmpty() )
    {
        aTest = aRet;
        aTest = aTest.replaceFirst( "*." , "" );
        if( aTest.getLength() <= nMaxLength )
        {
            if (!aShort.isEmpty())
                aShort += ";";
            aShort += aRet;
        }
        else
        {
            if (!aLong.isEmpty())
                aLong += ";";
            aLong += aRet;
        }
    }
    if (!aShort.isEmpty() && !aLong.isEmpty())
    {
        aShort += ";";
        aShort += aLong;
    }
    aWildCard.setGlob(aShort);
}

SfxFilter::~SfxFilter()
{
}

OUString SfxFilter::GetDefaultExtension() const
{
    return GetWildcard().getGlob().getToken(0, ';');
}


void SfxFilter::SetURLPattern( const OUString& rStr )
{
    aPattern = rStr.toAsciiLowerCase();
}

OUString SfxFilter::GetSuffixes() const
{
    OUString aRet = GetWildcard().getGlob();
    aRet = aRet.replaceAll( "*.", "" );
    aRet = aRet.replaceAll( ";", "," );
    return aRet;
}

const SfxFilter* SfxFilter::GetDefaultFilter( const OUString& rName )
{
    return SfxFilterContainer::GetDefaultFilter_Impl( rName );
}

const SfxFilter* SfxFilter::GetDefaultFilterFromFactory( const OUString& rFact )
{
    return GetDefaultFilter( SfxObjectShell::GetServiceNameFromFactory( rFact ) );
}

const SfxFilter* SfxFilter::GetFilterByName( const OUString& rName )
{
    SfxFilterMatcher aMatch;
    return aMatch.GetFilter4FilterName( rName, SfxFilterFlags::NONE, SfxFilterFlags::NONE );
}

OUString SfxFilter::GetTypeFromStorage( const SotStorage& rStg )
{
    const char* pType=nullptr;
    if ( rStg.IsStream( "WordDocument" ) )
    {
        if ( rStg.IsStream( "0Table" ) || rStg.IsStream( "1Table" ) )
            pType = "writer_MS_Word_97";
        else
            pType = "writer_MS_Word_95";
    }
    else if ( rStg.IsStream( "Book" ) )
    {
        pType = "calc_MS_Excel_95";
    }
    else if ( rStg.IsStream( "Workbook" ) )
    {
        pType = "calc_MS_Excel_97";
    }
    else if ( rStg.IsStream( "PowerPoint Document" ) )
    {
        pType = "impress_MS_PowerPoint_97";
    }
    else if ( rStg.IsStream( "Equation Native" ) )
    {
        pType = "math_MathType_3x";
    }
    else
    {
        SotClipboardFormatId nClipId = ((SotStorage&)rStg).GetFormat();
        if ( nClipId != SotClipboardFormatId::NONE )
        {
            const SfxFilter* pFilter = SfxFilterMatcher().GetFilter4ClipBoardId( nClipId );
            if ( pFilter )
                return pFilter->GetTypeName();
        }
    }

    return pType ? OUString::createFromAscii(pType) : OUString();
}

OUString SfxFilter::GetTypeFromStorage(
    const uno::Reference<embed::XStorage>& xStorage, bool bTemplate, OUString* pFilterName )
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    SfxFilterMatcher aMatcher;
    OUString aName;
    if ( pFilterName )
    {
        aName = *pFilterName;
        pFilterName->clear();
    }

    css::uno::Reference< css::beans::XPropertySet > xProps( xStorage, css::uno::UNO_QUERY );
    if ( xProps.is() )
    {
        OUString aMediaType;
        xProps->getPropertyValue("MediaType") >>= aMediaType;
        if ( !aMediaType.isEmpty() )
        {
            css::datatransfer::DataFlavor aDataFlavor;
            aDataFlavor.MimeType = aMediaType;
            SotClipboardFormatId nClipId = SotExchange::GetFormat( aDataFlavor );
            if ( nClipId != SotClipboardFormatId::NONE )
            {
                SfxFilterFlags nMust = SfxFilterFlags::IMPORT, nDont = SFX_FILTER_NOTINSTALLED;
                if ( bTemplate )
                    // template filter was preselected, try to verify
                    nMust |= SfxFilterFlags::TEMPLATEPATH;
                else
                    // template filters shouldn't be detected if not explicitly asked for
                    nDont |= SfxFilterFlags::TEMPLATEPATH;

                const SfxFilter* pFilter = nullptr;
                if (!aName.isEmpty())
                    // get preselected Filter if it matches the desired filter flags
                    pFilter = aMatcher.GetFilter4FilterName( aName, nMust, nDont );

                if ( !pFilter || pFilter->GetFormat() != nClipId )
                {
                    // get filter from storage MediaType
                    pFilter = aMatcher.GetFilter4ClipBoardId( nClipId, nMust, nDont );
                    if ( !pFilter )
                        // template filter is asked for , but there isn't one; so at least the "normal" format should be detected
                        // or storage *is* a template, but bTemplate is not set
                        pFilter = aMatcher.GetFilter4ClipBoardId( nClipId );
                }

                if ( pFilter )
                {
                    if ( pFilterName )
                        *pFilterName = pFilter->GetName();
                    return pFilter->GetTypeName();
                }
            }
        }
    }

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
