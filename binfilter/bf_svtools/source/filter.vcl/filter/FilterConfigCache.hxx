/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FILTER_CONFIG_CACHE_HXX_
#define _FILTER_CONFIG_CACHE_HXX_

#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include "bf_svtools/svtdllapi.h"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <vector>

namespace binfilter
{

class  FilterConfigCache
{
        struct FilterConfigCacheEntry
        {
            ::rtl::OUString sInternalFilterName;
            ::rtl::OUString sType;
            ::com::sun::star::uno::Sequence< ::rtl::OUString > lExtensionList;
            ::rtl::OUString	sUIName;
            ::rtl::OUString	sDocumentService;
            ::rtl::OUString sFilterService;
            ::rtl::OUString sTemplateName;

            ::rtl::OUString	sMediaType;
            ::rtl::OUString sFilterType;

            sal_Int32		nFlags;
            sal_Int32		nFileFormatVersion;

            // user data
            String			sFilterName;
            sal_Bool		bHasDialog			: 1;
            sal_Bool		bIsInternalFilter	: 1;
            sal_Bool		bIsPixelFormat		: 1;

            sal_Bool		CreateFilterName( const ::rtl::OUString& rUserDataEntry );
            String          GetShortName( );
                    
            static const char* InternalPixelFilterNameList[];
            static const char* InternalVectorFilterNameList[];
            static const char* ExternalPixelFilterNameList[];
        };

        typedef std::vector< FilterConfigCacheEntry > CacheVector;


        CacheVector			aImport;
        CacheVector			aExport;
        sal_Bool			bUseConfig;

        static sal_Bool   bInitialized;
        static sal_Int32  nIndType;
        static sal_Int32  nIndUIName;
        static sal_Int32  nIndDocumentService;
        static sal_Int32  nIndFilterService;
        static sal_Int32  nIndFlags;
        static sal_Int32  nIndUserData;
        static sal_Int32  nIndFileFormatVersion;
        static sal_Int32  nIndTemplateName;

        static const char*	InternalFilterListForSvxLight[];

        void				ImplInit();
        void				ImplInitSmart();
        
    public :

        sal_uInt16	GetImportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aImport.size()); };
        sal_uInt16	GetImportFormatNumber( const String& rFormatName );
        sal_uInt16	GetImportFormatNumberForMediaType( const String& rMediaType );
        sal_uInt16	GetImportFormatNumberForShortName( const String& rShortName );
        sal_uInt16	GetImportFormatNumberForTypeName( const String& rType );
        String		GetImportFilterName( sal_uInt16 nFormat );
        String		GetImportFormatName( sal_uInt16 nFormat );
        String		GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0);
        String		GetImportFormatMediaType( sal_uInt16 nFormat );
        String		GetImportFormatShortName( sal_uInt16 nFormat );
        String		GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
        String      GetImportFilterType( sal_uInt16 nFormat );
        String      GetImportFilterTypeName( sal_uInt16 nFormat );

        sal_Bool	IsImportInternalFilter( sal_uInt16 nFormat );
        sal_Bool	IsImportPixelFormat( sal_uInt16 nFormat );

        sal_uInt16	GetExportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aExport.size()); };
        sal_uInt16	GetExportFormatNumber( const String& rFormatName );
        sal_uInt16	GetExportFormatNumberForMediaType( const String& rMediaType );
        sal_uInt16	GetExportFormatNumberForShortName( const String& rShortName );
        sal_uInt16	GetExportFormatNumberForTypeName( const String& rType );
        String		GetExportFilterName( sal_uInt16 nFormat );
        String		GetExportFormatName( sal_uInt16 nFormat );
        String		GetExportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
        String		GetExportFormatMediaType( sal_uInt16 nFormat );
        String		GetExportFormatShortName( sal_uInt16 nFormat );
        String		GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );
        String      GetExportFilterTypeName( sal_uInt16 nFormat );

        sal_Bool	IsExportInternalFilter( sal_uInt16 nFormat );
        sal_Bool	IsExportPixelFormat( sal_uInt16 nFormat );

                    FilterConfigCache( sal_Bool bUseConfig );
                    ~FilterConfigCache();

};

}

#endif	// _FILTER_CONFIG_CACHE_HXX_

