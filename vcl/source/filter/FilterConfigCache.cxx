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

#include "FilterConfigCache.hxx"

#include <o3tl/safeint.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/configmgr.hxx>
#include <tools/svlibrary.h>
#include <com/sun/star/uno/Any.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace ::com::sun::star::lang          ;   // XMultiServiceFactory
using namespace ::com::sun::star::container     ;   // XNameAccess
using namespace ::com::sun::star::uno           ;   // Reference
using namespace ::com::sun::star::beans         ;   // PropertyValue
using namespace ::com::sun::star::configuration ;

const char* FilterConfigCache::FilterConfigCacheEntry::InternalPixelFilterNameList[] =
{
    IMP_BMP, IMP_GIF, IMP_PNG, IMP_JPEG, IMP_XBM, IMP_XPM,
    EXP_BMP, EXP_JPEG, EXP_PNG, IMP_MOV, IMP_TIFF, EXP_TIFF,
    IMP_TGA, nullptr
};

const char* FilterConfigCache::FilterConfigCacheEntry::InternalVectorFilterNameList[] =
{
    IMP_SVMETAFILE, IMP_WMF, IMP_EMF, IMP_SVG, IMP_PDF,
    EXP_SVMETAFILE, EXP_WMF, EXP_EMF, EXP_SVG, EXP_PDF, nullptr
};

const char* FilterConfigCache::FilterConfigCacheEntry::ExternalPixelFilterNameList[] =
{
    "egi", "icd", "ipd", "ipx", "ipb", "epb", "epg",
    "epp", "ira", "era", "exp", nullptr
};

void FilterConfigCache::FilterConfigCacheEntry::CreateFilterName( const OUString& rUserDataEntry )
{
    bIsPixelFormat = bIsInternalFilter = false;
    sFilterName = rUserDataEntry;
    const char** pPtr;
    for ( pPtr = InternalPixelFilterNameList; *pPtr && !bIsInternalFilter; pPtr++ )
    {
        if ( sFilterName.equalsIgnoreAsciiCaseAscii( *pPtr ) )
        {
            bIsInternalFilter = true;
            bIsPixelFormat = true;
        }
    }
    for ( pPtr = InternalVectorFilterNameList; *pPtr && !bIsInternalFilter; pPtr++ )
    {
        if ( sFilterName.equalsIgnoreAsciiCaseAscii( *pPtr ) )
            bIsInternalFilter = true;
    }
    if ( !bIsInternalFilter )
    {
        for ( pPtr = ExternalPixelFilterNameList; *pPtr && !bIsPixelFormat; pPtr++ )
        {
            if ( sFilterName.equalsIgnoreAsciiCaseAscii( *pPtr ) )
                bIsPixelFormat = true;
        }
        sExternalFilterName = sFilterName;
        sFilterName = SVLIBRARY("gie");
    }
}

OUString FilterConfigCache::FilterConfigCacheEntry::GetShortName()
{
    OUString aShortName;
    if ( !lExtensionList.empty() )
    {
        aShortName = lExtensionList[ 0 ];
        if ( aShortName.startsWith( "*." ) )
            aShortName = aShortName.replaceAt( 0, 2, "" );
    }
    return aShortName;
}

/** helper to open the configuration root of the underlying
    config package

    @param  sPackage
            specify, which config package should be opened.
            Must be one of "types" or "filters"

    @return A valid object if open was successful. The access on opened
            data will be readonly. It returns NULL in case open failed.

    @throws It let pass RuntimeExceptions only.
 */
static Reference< XInterface > openConfig(const char* sPackage)
{
    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< XInterface >           xCfg;
    try
    {
        // get access to config API (not to file!)
        Reference< XMultiServiceFactory > xConfigProvider = theDefaultProvider::get( xContext );

        Sequence< Any > lParams(1);
        PropertyValue   aParam    ;

        // define cfg path for open
        aParam.Name = "nodepath";
        if (rtl_str_compareIgnoreAsciiCase(sPackage, "types") == 0)
            aParam.Value <<= OUString( "/org.openoffice.TypeDetection.Types/Types" );
        if (rtl_str_compareIgnoreAsciiCase(sPackage, "filters") == 0)
            aParam.Value <<= OUString( "/org.openoffice.TypeDetection.GraphicFilter/Filters" );
        lParams[0] <<= aParam;

        // get access to file
        xCfg = xConfigProvider->createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", lParams);
    }
    catch(const RuntimeException&)
        { throw; }
    catch(const Exception&)
        { xCfg.clear(); }

    return xCfg;
}

void FilterConfigCache::ImplInit()
{
    OUString const STYPE                ( "Type"                );
    OUString const SUINAME              ( "UIName"              );
    OUString const SFLAGS               ( "Flags"               );
    OUString const SMEDIATYPE           ( "MediaType"           );
    OUString const SEXTENSIONS          ( "Extensions"          );
    OUString const SFORMATNAME          ( "FormatName"          );
    OUString const SREALFILTERNAME      ( "RealFilterName"      );

    // get access to config
    Reference< XNameAccess > xTypeAccess  ( openConfig("types"  ), UNO_QUERY );
    Reference< XNameAccess > xFilterAccess( openConfig("filters"), UNO_QUERY );

    if ( !(xTypeAccess.is() && xFilterAccess.is()) )
        return;

    const Sequence< OUString > lAllFilter = xFilterAccess->getElementNames();

    for ( const OUString& sInternalFilterName : lAllFilter )
    {
        Reference< XPropertySet > xFilterSet;
        xFilterAccess->getByName( sInternalFilterName ) >>= xFilterSet;
        if (!xFilterSet.is())
            continue;

        FilterConfigCacheEntry aEntry;

        aEntry.sInternalFilterName = sInternalFilterName;
        xFilterSet->getPropertyValue(STYPE) >>= aEntry.sType;
        xFilterSet->getPropertyValue(SUINAME) >>= aEntry.sUIName;
        xFilterSet->getPropertyValue(SREALFILTERNAME) >>= aEntry.sFilterType;
        Sequence< OUString > lFlags;
        xFilterSet->getPropertyValue(SFLAGS) >>= lFlags;
        if (lFlags.getLength()!=1 || lFlags[0].isEmpty())
            continue;
        if (lFlags[0].equalsIgnoreAsciiCase("import"))
            aEntry.nFlags = 1;
        else if (lFlags[0].equalsIgnoreAsciiCase("export"))
            aEntry.nFlags = 2;

        OUString sFormatName;
        xFilterSet->getPropertyValue(SFORMATNAME) >>= sFormatName;
        aEntry.CreateFilterName( sFormatName );

        Reference< XPropertySet > xTypeSet;
        xTypeAccess->getByName( aEntry.sType ) >>= xTypeSet;
        if (!xTypeSet.is())
            continue;

        xTypeSet->getPropertyValue(SMEDIATYPE) >>= aEntry.sMediaType;
        css::uno::Sequence<OUString> tmp;
        if (xTypeSet->getPropertyValue(SEXTENSIONS) >>= tmp)
            aEntry.lExtensionList = comphelper::sequenceToContainer<std::vector<OUString>>(tmp);

        // The first extension will be used
        // to generate our internal FilterType ( BMP, WMF ... )
        OUString aExtension( aEntry.GetShortName() );
        if (aExtension.getLength() != 3)
            continue;

        if ( aEntry.nFlags & 1 )
            aImport.push_back( aEntry );
        if ( aEntry.nFlags & 2 )
            aExport.push_back( aEntry );

        // bFilterEntryCreated!?
        if (!( aEntry.nFlags & 3 ))
            continue; //? Entry was already inserted ... but following code will be suppressed?!
    }
};

const char* FilterConfigCache::InternalFilterListForSvxLight[] =
{
    "bmp","1","SVBMP",
    "bmp","2","SVBMP",
    "dxf","1","idx",
    "eps","1","ips",
    "eps","2","eps",
    "gif","1","SVIGIF",
    "gif","2","egi",
    "jpg","1","SVIJPEG",
    "jpg","2","SVEJPEG",
    "mov","1","SVMOV",
    "mov","2","SVMOV",
    "met","1","ime",
    "png","1","SVIPNG",
    "png","2","SVEPNG",
    "pct","1","ipt",
    "pct","2","ept",
    "pcd","1","icd",
    "psd","1","ipd",
    "pcx","1","ipx",
    "pbm","1","ipb",
    "pgm","1","ipb",
    "ppm","1","ipb",
    "ras","1","ira",
    "ras","2","era",
    "svm","1","SVMETAFILE",
    "svm","2","SVMETAFILE",
    "tga","1","SVTGA",
    "tif","1","SVTIFF",
    "tif","2","SVTIFF",
    "emf","1","SVEMF",
    "emf","2","SVEMF",
    "wmf","1","SVWMF",
    "wmf","2","SVWMF",
    "xbm","1","SVIXBM",
    "xpm","1","SVIXPM",
    "xpm","2","exp",
    "svg","1","SVISVG",
    "svg","2","SVESVG",
    nullptr
};

void FilterConfigCache::ImplInitSmart()
{
    const char** pPtr;
    for ( pPtr = InternalFilterListForSvxLight; *pPtr; pPtr++ )
    {
        FilterConfigCacheEntry  aEntry;

        OUString    sExtension( OUString::createFromAscii( *pPtr++ ) );

        aEntry.lExtensionList.push_back(sExtension);

        aEntry.sType = sExtension;
        aEntry.sUIName = sExtension;

        OString sFlags( *pPtr++ );
        aEntry.nFlags = sFlags.toInt32();

        OUString    sUserData( OUString::createFromAscii( *pPtr ) );
        aEntry.CreateFilterName( sUserData );

        if ( aEntry.nFlags & 1 )
            aImport.push_back( aEntry );
        if ( aEntry.nFlags & 2 )
            aExport.push_back( aEntry );
    }
}

FilterConfigCache::FilterConfigCache( bool bConfig )
{
    if (bConfig)
        bConfig = !utl::ConfigManager::IsFuzzing();
    if (bConfig)
        ImplInit();
    else
        ImplInitSmart();
}

FilterConfigCache::~FilterConfigCache()
{
}

OUString FilterConfigCache::GetImportFilterName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sFilterName;
    return OUString();
}

sal_uInt16 FilterConfigCache::GetImportFormatNumber( std::u16string_view rFormatName )
{
    sal_uInt16 nPos = 0;
    for (auto const& elem : aImport)
    {
        if ( elem.sUIName.equalsIgnoreAsciiCase( rFormatName ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

/// get the index of the filter that matches this extension
sal_uInt16 FilterConfigCache::GetImportFormatNumberForExtension( std::u16string_view rExt )
{
    sal_uInt16 nPos = 0;
    for (auto const& elem : aImport)
    {
        for ( OUString const & s : elem.lExtensionList )
        {
            if ( s.equalsIgnoreAsciiCase( rExt ) )
                return nPos;
        }
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForShortName( std::u16string_view rShortName )
{
    sal_uInt16 nPos = 0;
    for (auto & elem : aImport)
    {
        if ( elem.GetShortName().equalsIgnoreAsciiCase( rShortName ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForTypeName( std::u16string_view rType )
{
    sal_uInt16 nPos = 0;
    for (auto const& elem : aImport)
    {
        if ( elem.sType.equalsIgnoreAsciiCase( rType ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

OUString FilterConfigCache::GetImportFormatName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sUIName;
    return OUString();
}

OUString FilterConfigCache::GetImportFormatMediaType( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sMediaType;
    return OUString();
}

OUString FilterConfigCache::GetImportFormatShortName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].GetShortName();
    return OUString();
}

OUString FilterConfigCache::GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    if ( (nFormat < aImport.size()) && (o3tl::make_unsigned(nEntry) < aImport[ nFormat ].lExtensionList.size()) )
        return aImport[ nFormat ].lExtensionList[ nEntry ];
    return OUString();
}

OUString FilterConfigCache::GetImportFilterType( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sType;
    return OUString();
}

OUString FilterConfigCache::GetImportFilterTypeName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sFilterType;
    return OUString();
}

OUString FilterConfigCache::GetExternalFilterName(sal_uInt16 nFormat, bool bExport)
{
    if (bExport)
    {
        if (nFormat < aExport.size())
            return aExport[nFormat].sExternalFilterName;
    }
    else
    {
        if (nFormat < aImport.size())
            return aImport[nFormat].sExternalFilterName;
    }
    return OUString();
}

OUString FilterConfigCache::GetImportWildcard(sal_uInt16 nFormat, sal_Int32 nEntry)
{
    OUString aWildcard( GetImportFormatExtension( nFormat, nEntry ) );
    if ( !aWildcard.isEmpty() )
        aWildcard = aWildcard.replaceAt( 0, 0, "*." );
    return aWildcard;
}

bool FilterConfigCache::IsImportInternalFilter( sal_uInt16 nFormat )
{
    return (nFormat < aImport.size()) && aImport[ nFormat ].bIsInternalFilter;
}

OUString FilterConfigCache::GetExportFilterName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sFilterName;
    return OUString();
}

sal_uInt16 FilterConfigCache::GetExportFormatNumber(std::u16string_view rFormatName)
{
    sal_uInt16 nPos = 0;
    for (auto const& elem : aExport)
    {
        if ( elem.sUIName.equalsIgnoreAsciiCase( rFormatName ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForMediaType( std::u16string_view rMediaType )
{
    sal_uInt16 nPos = 0;
    for (auto const& elem : aExport)
    {
        if ( elem.sMediaType.equalsIgnoreAsciiCase( rMediaType ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForShortName( std::u16string_view rShortName )
{
    sal_uInt16 nPos = 0;
    for (auto & elem : aExport)
    {
        if ( elem.GetShortName().equalsIgnoreAsciiCase( rShortName ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForTypeName( std::u16string_view rType )
{
    sal_uInt16 nPos = 0;
    for (auto const& elem : aExport)
    {
        if ( elem.sType.equalsIgnoreAsciiCase( rType ) )
            return nPos;
        ++nPos;
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

OUString FilterConfigCache::GetExportFormatName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sUIName;
    return OUString();
}

OUString FilterConfigCache::GetExportFormatMediaType( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sMediaType;
    return OUString();
}

OUString FilterConfigCache::GetExportFormatShortName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].GetShortName();
    return OUString();
}

OUString FilterConfigCache::GetExportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    if ( (nFormat < aExport.size()) && (o3tl::make_unsigned(nEntry) < aExport[ nFormat ].lExtensionList.size()) )
        return aExport[ nFormat ].lExtensionList[ nEntry ];
    return OUString();
}

OUString FilterConfigCache::GetExportInternalFilterName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sInternalFilterName;
    return OUString();
}

OUString FilterConfigCache::GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    OUString aWildcard( GetExportFormatExtension( nFormat, nEntry ) );
    if ( !aWildcard.isEmpty() )
        aWildcard = aWildcard.replaceAt( 0, 0, "*." );
    return aWildcard;
}

bool FilterConfigCache::IsExportInternalFilter( sal_uInt16 nFormat )
{
    return (nFormat < aExport.size()) && aExport[ nFormat ].bIsInternalFilter;
}

bool FilterConfigCache::IsExportPixelFormat( sal_uInt16 nFormat )
{
    return (nFormat < aExport.size()) && aExport[ nFormat ].bIsPixelFormat;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
