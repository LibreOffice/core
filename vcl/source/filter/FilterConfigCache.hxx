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

#ifndef INCLUDED_VCL_SOURCE_FILTER_FILTERCONFIGCACHE_HXX
#define INCLUDED_VCL_SOURCE_FILTER_FILTERCONFIGCACHE_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <rtl/ustring.hxx>
#include <vector>

/** Cache to keep list of graphic filters + the filters themselves. */
class FilterConfigCache
{
    struct FilterConfigCacheEntry
    {
        OUString sInternalFilterName;
        OUString sType;
        css::uno::Sequence< OUString > lExtensionList;
        OUString sUIName;
        OUString sExternalFilterName;

        OUString sMediaType;
        OUString sFilterType;

        sal_Int32       nFlags;

        // user data
        OUString        sFilterName;
        bool        bIsInternalFilter   : 1;
        bool        bIsPixelFormat      : 1;

        bool        CreateFilterName( const OUString& rUserDataEntry );
        OUString        GetShortName( );

        static const char* InternalPixelFilterNameList[];
        static const char* InternalVectorFilterNameList[];
        static const char* ExternalPixelFilterNameList[];
    };

    typedef std::vector< FilterConfigCacheEntry > CacheVector;

    CacheVector         aImport;
    CacheVector         aExport;
    bool            bUseConfig;

    static bool   bInitialized;
    static sal_Int32  nIndType;
    static sal_Int32  nIndUIName;
    static sal_Int32  nIndDocumentService;
    static sal_Int32  nIndFilterService;
    static sal_Int32  nIndFlags;
    static sal_Int32  nIndUserData;
    static sal_Int32  nIndFileFormatVersion;
    static sal_Int32  nIndTemplateName;

    static const char*  InternalFilterListForSvxLight[];

    void             ImplInit();
    void             ImplInitSmart();

public:

    sal_uInt16  GetImportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aImport.size()); };
    sal_uInt16  GetImportFormatNumber( const OUString& rFormatName );
    sal_uInt16  GetImportFormatNumberForShortName( const OUString& rShortName );
    sal_uInt16  GetImportFormatNumberForTypeName( const OUString& rType );
    sal_uInt16  GetImportFormatNumberForExtension( const OUString& rExt );
    OUString    GetImportFilterName( sal_uInt16 nFormat );
    OUString    GetImportFormatName( sal_uInt16 nFormat );
    OUString    GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0);
    OUString    GetImportFormatMediaType( sal_uInt16 nFormat );
    OUString    GetImportFormatShortName( sal_uInt16 nFormat );
    OUString    GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
    OUString    GetImportFilterType( sal_uInt16 nFormat );
    OUString    GetImportFilterTypeName( sal_uInt16 nFormat );
    OUString    GetExternalFilterName(sal_uInt16 nFormat, bool bExport);


    bool    IsImportInternalFilter( sal_uInt16 nFormat );

    sal_uInt16  GetExportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aExport.size()); };
    sal_uInt16  GetExportFormatNumber( const OUString& rFormatName );
    sal_uInt16  GetExportFormatNumberForMediaType( const OUString& rMediaType );
    sal_uInt16  GetExportFormatNumberForShortName( const OUString& rShortName );
    sal_uInt16  GetExportFormatNumberForTypeName( const OUString& rType );
    OUString    GetExportFilterName( sal_uInt16 nFormat );
    OUString    GetExportFormatName( sal_uInt16 nFormat );
    OUString    GetExportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
    OUString    GetExportFormatMediaType( sal_uInt16 nFormat );
    OUString    GetExportFormatShortName( sal_uInt16 nFormat );
    OUString    GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
    OUString    GetExportInternalFilterName( sal_uInt16 nFormat );

    bool    IsExportInternalFilter( sal_uInt16 nFormat );
    bool    IsExportPixelFormat( sal_uInt16 nFormat );

    explicit FilterConfigCache( bool bUseConfig );
                ~FilterConfigCache();
};

#endif // INCLUDED_VCL_SOURCE_FILTER_FILTERCONFIGCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
