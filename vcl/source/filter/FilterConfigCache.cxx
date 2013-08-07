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

#include <vcl/graphicfilter.hxx>
#include <com/sun/star/uno/Any.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
    IMP_BMP, IMP_GIF, IMP_PNG,IMP_JPEG, IMP_XBM, IMP_XPM,
    EXP_BMP, EXP_JPEG, EXP_PNG, NULL
};

const char* FilterConfigCache::FilterConfigCacheEntry::InternalVectorFilterNameList[] =
{
    IMP_SVMETAFILE, IMP_WMF, IMP_EMF, IMP_SVSGF, IMP_SVSGV, IMP_SVG,
    EXP_SVMETAFILE, EXP_WMF, EXP_EMF, EXP_SVG, NULL
};

const char* FilterConfigCache::FilterConfigCacheEntry::ExternalPixelFilterNameList[] =
{
    "egi", "icd", "ipd", "ipx", "ipb", "epb", "epg",
    "epp", "ira", "era", "itg", "iti", "eti", "exp", NULL
};

sal_Bool FilterConfigCache::bInitialized = sal_False;
sal_Int32 FilterConfigCache::nIndType = -1;
sal_Int32 FilterConfigCache::nIndUIName = -1;
sal_Int32 FilterConfigCache::nIndDocumentService = -1;
sal_Int32 FilterConfigCache::nIndFilterService = -1;
sal_Int32 FilterConfigCache::nIndFlags = -1;
sal_Int32 FilterConfigCache::nIndUserData = -1;
sal_Int32 FilterConfigCache::nIndFileFormatVersion = -1;
sal_Int32 FilterConfigCache::nIndTemplateName = -1;

sal_Bool FilterConfigCache::FilterConfigCacheEntry::CreateFilterName( const OUString& rUserDataEntry )
{
    bIsPixelFormat = bIsInternalFilter = sal_False;
    sFilterName = rUserDataEntry;
    const char** pPtr;
    for ( pPtr = InternalPixelFilterNameList; *pPtr && ( bIsInternalFilter == sal_False ); pPtr++ )
    {
        if ( sFilterName.equalsIgnoreAsciiCase( OUString(*pPtr, strlen(*pPtr), RTL_TEXTENCODING_ASCII_US) ) )
        {
            bIsInternalFilter = sal_True;
            bIsPixelFormat = sal_True;
        }
    }
    for ( pPtr = InternalVectorFilterNameList; *pPtr && ( bIsInternalFilter == sal_False ); pPtr++ )
    {
        if ( sFilterName.equalsIgnoreAsciiCase( OUString(*pPtr, strlen(*pPtr), RTL_TEXTENCODING_ASCII_US) ) )
            bIsInternalFilter = sal_True;
    }
    if ( !bIsInternalFilter )
    {
        for ( pPtr = ExternalPixelFilterNameList; *pPtr && ( bIsPixelFormat == sal_False ); pPtr++ )
        {
            if ( sFilterName.equalsIgnoreAsciiCase( OUString(*pPtr, strlen(*pPtr), RTL_TEXTENCODING_ASCII_US) ) )
                bIsPixelFormat = sal_True;
        }
        OUString sTemp(SVLIBRARY("?"));
        sal_Int32 nIndex = sTemp.indexOf(static_cast<sal_Unicode>('?'));
        sFilterName = sTemp.replaceAt(nIndex, 1, sFilterName);
    }
    return ! sFilterName.isEmpty();
}

OUString FilterConfigCache::FilterConfigCacheEntry::GetShortName()
{
    OUString aShortName;
    if ( lExtensionList.getLength() )
    {
        aShortName = lExtensionList[ 0 ];
        if ( aShortName.indexOf( "*." ) == 0 )
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
Reference< XInterface > openConfig(const char* sPackage)
    throw(RuntimeException)
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
        aParam.Name = OUString( "nodepath" );
        if (rtl_str_compareIgnoreAsciiCase(sPackage, "types") == 0)
            aParam.Value <<= OUString( "/org.openoffice.TypeDetection.Types/Types" );
        if (rtl_str_compareIgnoreAsciiCase(sPackage, "filters") == 0)
            aParam.Value <<= OUString( "/org.openoffice.TypeDetection.GraphicFilter/Filters" );
        lParams[0] = makeAny(aParam);

        // get access to file
        xCfg = xConfigProvider->createInstanceWithArguments(
            OUString( "com.sun.star.configuration.ConfigurationAccess" ), lParams);
    }
    catch(const RuntimeException&)
        { throw; }
    catch(const Exception&)
        { xCfg.clear(); }

    return xCfg;
}

void FilterConfigCache::ImplInit()
{
    OUString STYPE                ( "Type"                );
    OUString SUINAME              ( "UIName"              );
    OUString SFLAGS               ( "Flags"               );
    OUString SMEDIATYPE           ( "MediaType"           );
    OUString SEXTENSIONS          ( "Extensions"          );
    OUString SFORMATNAME          ( "FormatName"          );
    OUString SREALFILTERNAME      ( "RealFilterName"      );

    // get access to config
    Reference< XNameAccess > xTypeAccess  ( openConfig("types"  ), UNO_QUERY );
    Reference< XNameAccess > xFilterAccess( openConfig("filters"), UNO_QUERY );

    if ( xTypeAccess.is() && xFilterAccess.is() )
    {
        Sequence< OUString > lAllFilter = xFilterAccess->getElementNames();
        sal_Int32 nAllFilterCount = lAllFilter.getLength();

        for ( sal_Int32 i = 0; i < nAllFilterCount; i++ )
        {
            OUString sInternalFilterName = lAllFilter[ i ];
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
            xTypeSet->getPropertyValue(SEXTENSIONS) >>= aEntry.lExtensionList;

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
                continue; //? Entry was already inserted ... but following code will be supressed?!
        }
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
    "sgv","1","SVSGV",
    "sgf","1","SVSGF",
    "met","1","ime",
    "met","2","eme",
    "png","1","SVIPNG",
    "png","2","SVEPNG",
    "pct","1","ipt",
    "pct","2","ept",
    "pcd","1","icd",
    "psd","1","ipd",
    "pcx","1","ipx",
    "pbm","1","ipb",
    "pbm","2","epb",
    "pgm","1","ipb",
    "pgm","2","epg",
    "ppm","1","ipb",
    "ppm","2","epp",
    "ras","1","ira",
    "ras","2","era",
    "svm","1","SVMETAFILE",
    "svm","2","SVMETAFILE",
    "tga","1","itg",
    "tif","1","iti",
    "tif","2","eti",
    "emf","1","SVEMF",
    "emf","2","SVEMF",
    "wmf","1","SVWMF",
    "wmf","2","SVWMF",
    "xbm","1","SVIXBM",
    "xpm","1","SVIXPM",
    "xpm","2","exp",
    "svg","1","SVISVG",
    "svg","2","SVESVG",
    NULL
};

void FilterConfigCache::ImplInitSmart()
{
    const char** pPtr;
    for ( pPtr = InternalFilterListForSvxLight; *pPtr; pPtr++ )
    {
        FilterConfigCacheEntry  aEntry;

        OUString    sExtension( OUString::createFromAscii( *pPtr++ ) );

        aEntry.lExtensionList.realloc( 1 );
        aEntry.lExtensionList[ 0 ] = sExtension;

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

// ------------------------------------------------------------------------

FilterConfigCache::FilterConfigCache( sal_Bool bConfig ) :
    bUseConfig ( bConfig )
{
    if ( bUseConfig )
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
    return OUString("");
}

sal_uInt16 FilterConfigCache::GetImportFormatNumber( const OUString& rFormatName )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aImport.begin(), aEnd = aImport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->sUIName.equalsIgnoreAsciiCase( rFormatName ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin());
}

/// get the index of the filter that matches this extension
sal_uInt16 FilterConfigCache::GetImportFormatNumberForExtension( const OUString& rExt )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aImport.begin(), aEnd = aImport.end(); aIter != aEnd; ++aIter)
    {
        for ( sal_Int32 i = 0; i < aIter->lExtensionList.getLength(); i++ )
        {
            if ( aIter->lExtensionList[i].equalsIgnoreAsciiCase( rExt ) )
                return sal::static_int_cast< sal_uInt16 >( aIter - aImport.begin() );
        }
    }
    return GRFILTER_FORMAT_NOTFOUND;
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForMediaType( const OUString& rMediaType )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aImport.begin(), aEnd = aImport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->sMediaType.equalsIgnoreAsciiCase( rMediaType ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin());
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForShortName( const OUString& rShortName )
{
    CacheVector::const_iterator aEnd;
    CacheVector::iterator aIter;
    for (aIter = aImport.begin(), aEnd = aImport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->GetShortName().equalsIgnoreAsciiCase( rShortName ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin());
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForTypeName( const OUString& rType )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aImport.begin(), aEnd = aImport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->sType.equalsIgnoreAsciiCase( rType ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin());
}

OUString FilterConfigCache::GetImportFormatName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sUIName;
    return OUString("");
}

OUString FilterConfigCache::GetImportFormatMediaType( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sMediaType;
    return OUString("");
}

OUString FilterConfigCache::GetImportFormatShortName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].GetShortName();
    return OUString("");
}

OUString FilterConfigCache::GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    if ( (nFormat < aImport.size()) && (nEntry < aImport[ nFormat ].lExtensionList.getLength()) )
        return aImport[ nFormat ].lExtensionList[ nEntry ];
    return OUString("");
}

OUString FilterConfigCache::GetImportFilterType( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sType;
    return OUString("");
}

OUString FilterConfigCache::GetImportFilterTypeName( sal_uInt16 nFormat )
{
    if( nFormat < aImport.size() )
        return aImport[ nFormat ].sFilterType;
    return OUString("");
}

OUString FilterConfigCache::GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    OUString aWildcard( GetImportFormatExtension( nFormat, nEntry ) );
    if ( !aWildcard.isEmpty() )
        aWildcard = aWildcard.replaceAt( 0, 0, "*." );
    return aWildcard;
}

sal_Bool FilterConfigCache::IsImportInternalFilter( sal_uInt16 nFormat )
{
    return (nFormat < aImport.size()) && aImport[ nFormat ].bIsInternalFilter;
}

sal_Bool FilterConfigCache::IsImportPixelFormat( sal_uInt16 nFormat )
{
    return (nFormat < aImport.size()) && aImport[ nFormat ].bIsPixelFormat;
}

// ------------------------------------------------------------------------

OUString FilterConfigCache::GetExportFilterName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sFilterName;
    return OUString("");
}

sal_uInt16 FilterConfigCache::GetExportFormatNumber( const OUString& rFormatName )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aExport.begin(), aEnd = aExport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->sUIName.equalsIgnoreAsciiCase( rFormatName ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin());
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForMediaType( const OUString& rMediaType )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aExport.begin(), aEnd = aExport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->sMediaType.equalsIgnoreAsciiCase( rMediaType ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin());
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForShortName( const OUString& rShortName )
{
    CacheVector::const_iterator aEnd;
    CacheVector::iterator aIter;
    for (aIter = aExport.begin(), aEnd = aExport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->GetShortName().equalsIgnoreAsciiCase( rShortName ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin());
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForTypeName( const OUString& rType )
{
    CacheVector::const_iterator aIter, aEnd;
    for (aIter = aExport.begin(), aEnd = aExport.end(); aIter != aEnd; ++aIter)
    {
        if ( aIter->sType.equalsIgnoreAsciiCase( rType ) )
            break;
    }
    return sal::static_int_cast< sal_uInt16 >(aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin());
}

OUString FilterConfigCache::GetExportFormatName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sUIName;
    return OUString("");
}

OUString FilterConfigCache::GetExportFormatMediaType( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sMediaType;
    return OUString("");
}

OUString FilterConfigCache::GetExportFormatShortName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].GetShortName();
    return OUString("");
}

OUString FilterConfigCache::GetExportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    if ( (nFormat < aExport.size()) && (nEntry < aExport[ nFormat ].lExtensionList.getLength()) )
        return aExport[ nFormat ].lExtensionList[ nEntry ];
    return OUString("");
}

OUString FilterConfigCache::GetExportFilterTypeName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sFilterType;
    return OUString("");
}

OUString FilterConfigCache::GetExportInternalFilterName( sal_uInt16 nFormat )
{
    if( nFormat < aExport.size() )
        return aExport[ nFormat ].sInternalFilterName;
    return OUString("");
}

OUString FilterConfigCache::GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    OUString aWildcard( GetExportFormatExtension( nFormat, nEntry ) );
    if ( !aWildcard.isEmpty() )
        aWildcard = aWildcard.replaceAt( 0, 0, "*." );
    return aWildcard;
}

sal_Bool FilterConfigCache::IsExportInternalFilter( sal_uInt16 nFormat )
{
    return (nFormat < aExport.size()) && aExport[ nFormat ].bIsInternalFilter;
}

sal_Bool FilterConfigCache::IsExportPixelFormat( sal_uInt16 nFormat )
{
    return (nFormat < aExport.size()) && aExport[ nFormat ].bIsPixelFormat;
}

// ------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
