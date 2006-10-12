/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FilterConfigCache.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:17:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _FILTER_CONFIG_CACHE_HXX_
#define _FILTER_CONFIG_CACHE_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

class SVT_DLLPUBLIC FilterConfigCache
{
        struct FilterConfigCacheEntry
        {
            ::rtl::OUString sInternalFilterName;
            ::rtl::OUString sType;
            ::com::sun::star::uno::Sequence< ::rtl::OUString > lExtensionList;
            ::rtl::OUString sUIName;
            ::rtl::OUString sDocumentService;
            ::rtl::OUString sFilterService;
            ::rtl::OUString sTemplateName;

            ::rtl::OUString sMediaType;
            ::rtl::OUString sFilterType;

            sal_Int32       nFlags;
            sal_Int32       nFileFormatVersion;

            // user data
            String          sFilterName;
            sal_Bool        bHasDialog          : 1;
            sal_Bool        bIsInternalFilter   : 1;
            sal_Bool        bIsPixelFormat      : 1;

            sal_Bool        IsValid();
            sal_Bool        CreateFilterName( const ::rtl::OUString& rUserDataEntry );
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

        SVT_DLLPRIVATE void             ImplInit();
        SVT_DLLPRIVATE void             ImplInitSmart();

    public :

        sal_uInt16  GetImportFormatCount() const
        { return sal::static_int_cast< sal_uInt16 >(aImport.size()); };
        sal_uInt16  GetImportFormatNumber( const String& rFormatName );
        sal_uInt16  GetImportFormatNumberForMediaType( const String& rMediaType );
        sal_uInt16  GetImportFormatNumberForShortName( const String& rShortName );
        sal_uInt16  GetImportFormatNumberForTypeName( const String& rType );
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
        sal_Bool    IsImportDialog( sal_uInt16 nFormat );

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
        sal_Bool    IsExportDialog( sal_uInt16 nFormat );

                    FilterConfigCache( sal_Bool bUseConfig );
                    ~FilterConfigCache();

};

#endif  // _FILTER_CONFIG_CACHE_HXX_

