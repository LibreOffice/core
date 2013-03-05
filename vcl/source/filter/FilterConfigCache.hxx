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

#ifndef _FILTER_CONFIG_CACHE_HXX_
#define _FILTER_CONFIG_CACHE_HXX_

#include <tools/string.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <vector>

/** Cache to keep list of graphic filters + the filters themselves. */
class FilterConfigCache
{
    struct FilterConfigCacheEntry
    {
        OUString sInternalFilterName;
        OUString sType;
        ::com::sun::star::uno::Sequence< OUString > lExtensionList;
        OUString sUIName;
        OUString sDocumentService;
        OUString sFilterService;
        OUString sTemplateName;

        OUString sMediaType;
        OUString sFilterType;

        sal_Int32       nFlags;
        sal_Int32       nFileFormatVersion;

        // user data
        String          sFilterName;
        sal_Bool        bIsInternalFilter   : 1;
        sal_Bool        bIsPixelFormat      : 1;

        sal_Bool        CreateFilterName( const OUString& rUserDataEntry );
        String          GetShortName( );

        static const char* InternalPixelFilterNameList[];
        static const char* InternalVectorFilterNameList[];
        static const char* ExternalPixelFilterNameList[];
    };

    typedef std::vector< FilterConfigCacheEntry > CacheVector;

    CacheVector         aImport;
    CacheVector         aExport;
    sal_Bool            bUseConfig;

    static sal_Bool   bInitialized;
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
    sal_uInt16  GetImportFormatNumber( const String& rFormatName );
    sal_uInt16  GetImportFormatNumberForMediaType( const String& rMediaType );
    sal_uInt16  GetImportFormatNumberForShortName( const String& rShortName );
    sal_uInt16  GetImportFormatNumberForTypeName( const String& rType );
    sal_uInt16  GetImportFormatNumberForExtension( const String& rExt );
    String      GetImportFilterName( sal_uInt16 nFormat );
    String      GetImportFormatName( sal_uInt16 nFormat );
    String      GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0);
    String      GetImportFormatMediaType( sal_uInt16 nFormat );
    String      GetImportFormatShortName( sal_uInt16 nFormat );
    String      GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
    String      GetImportFilterType( sal_uInt16 nFormat );
    String      GetImportFilterTypeName( sal_uInt16 nFormat );

    sal_Bool    IsImportInternalFilter( sal_uInt16 nFormat );
    sal_Bool    IsImportPixelFormat( sal_uInt16 nFormat );

    sal_uInt16  GetExportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aExport.size()); };
    sal_uInt16  GetExportFormatNumber( const String& rFormatName );
    sal_uInt16  GetExportFormatNumberForMediaType( const String& rMediaType );
    sal_uInt16  GetExportFormatNumberForShortName( const String& rShortName );
    sal_uInt16  GetExportFormatNumberForTypeName( const String& rType );
    String      GetExportFilterName( sal_uInt16 nFormat );
    String      GetExportFormatName( sal_uInt16 nFormat );
    String      GetExportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
    String      GetExportFormatMediaType( sal_uInt16 nFormat );
    String      GetExportFormatShortName( sal_uInt16 nFormat );
    String      GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
    String      GetExportFilterTypeName( sal_uInt16 nFormat );
    String      GetExportInternalFilterName( sal_uInt16 nFormat );

    sal_Bool    IsExportInternalFilter( sal_uInt16 nFormat );
    sal_Bool    IsExportPixelFormat( sal_uInt16 nFormat );

                FilterConfigCache( sal_Bool bUseConfig );
                ~FilterConfigCache();
};

#endif  // _FILTER_CONFIG_CACHE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
