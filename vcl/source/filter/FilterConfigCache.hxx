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

#include <rtl/ustring.hxx>
#include <vector>

/** Cache to keep list of graphic filters + the filters themselves. */
class FilterConfigCache
{
    struct FilterConfigCacheEntry
    {
        OUString sInternalFilterName;
        OUString sType;
        std::vector< OUString > lExtensionList;
        OUString sUIName;

        OUString sMediaType;
        OUString sFilterType;

        sal_Int32       nFlags;

        // user data
        OUString        sFilterName;
        bool        bIsPixelFormat      : 1;

        void            CreateFilterName( const OUString& rUserDataEntry );
        OUString        GetShortName( );

        static const char* InternalPixelFilterNameList[];
        static const char* InternalVectorFilterNameList[];
    };


    std::vector< FilterConfigCacheEntry >         aImport;
    std::vector< FilterConfigCacheEntry >         aExport;

    static const char*  InternalFilterListForSvxLight[];

    void             ImplInit();
    void             ImplInitSmart();

public:

    sal_uInt16  GetImportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aImport.size()); };
    sal_uInt16  GetImportFormatNumber( std::u16string_view rFormatName );
    sal_uInt16  GetImportFormatNumberForShortName( std::u16string_view rShortName );
    sal_uInt16  GetImportFormatNumberForTypeName( std::u16string_view rType );
    sal_uInt16  GetImportFormatNumberForExtension( std::u16string_view rExt );
    OUString    GetImportFilterName( sal_uInt16 nFormat );
    OUString    GetImportFormatName( sal_uInt16 nFormat );
    OUString    GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0);
    OUString    GetImportFormatMediaType( sal_uInt16 nFormat );
    OUString    GetImportFormatShortName( sal_uInt16 nFormat );
    OUString    GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
    OUString    GetImportFilterType( sal_uInt16 nFormat );
    OUString    GetImportFilterTypeName( sal_uInt16 nFormat );


    bool    IsImportInternalFilter( sal_uInt16 nFormat );

    sal_uInt16  GetExportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aExport.size()); };
    sal_uInt16  GetExportFormatNumber( std::u16string_view rFormatName );
    sal_uInt16  GetExportFormatNumberForMediaType( std::u16string_view rMediaType );
    sal_uInt16  GetExportFormatNumberForShortName( std::u16string_view rShortName );
    sal_uInt16  GetExportFormatNumberForTypeName( std::u16string_view rType );
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
