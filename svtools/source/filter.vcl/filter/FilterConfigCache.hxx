/*************************************************************************
 *
 *  $RCSfile: FilterConfigCache.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-28 15:17:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _FILTER_CONFIG_CACHE_HXX_
#define _FILTER_CONFIG_CACHE_HXX_
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
#include <vector>

class FilterConfigCache
{
        struct FilterConfigCacheEntry
        {
            ::rtl::OUString sType;
            ::rtl::OUString sExtension;
            ::rtl::OUString sUIName;
            ::rtl::OUString sDocumentService;
            ::rtl::OUString sFilterService;
            ::rtl::OUString sTemplateName;

            ::rtl::OUString sMediaType;

            sal_Int32       nFlags;
            sal_Int32       nFileFormatVersion;

            // user data
            String          sFilterName;
            sal_Bool        bHasDialog          : 1;
            sal_Bool        bIsInternalFilter   : 1;
            sal_Bool        bIsPixelFormat      : 1;

            sal_Bool        IsValid();
            sal_Bool        CreateFilterName( const ::rtl::OUString& rUserDataEntry );

            static const char* InternalPixelFilterNameList[];
            static const char* InternalVectorFilterNameList[];
        };

        typedef std::vector< FilterConfigCacheEntry > CacheVector;

        CacheVector         aImport;
        CacheVector         aExport;

        sal_Bool            ImplIsOwnFilter( const ::com::sun::star::uno::Sequence<
                                                ::com::sun::star::beans::PropertyValue >& rFilterProperties );
        sal_Bool            ImplAddFilterEntry( const ::com::sun::star::uno::Sequence<
                                                ::com::sun::star::beans::PropertyValue >& rFilterProperties,
                                                    const ::com::sun::star::uno::Reference<
                                                        ::com::sun::star::container::XNameAccess >& xTypeAccess );

        void                ImplInit();

    public :

        sal_uInt16  GetImportFormatCount() const { return aImport.size(); };
        sal_uInt16  GetImportFormatNumber( const String& rFormatName );
        sal_uInt16  GetImportFormatNumberForMediaType( const String& rMediaType );
        sal_uInt16  GetImportFormatNumberForShortName( const String& rShortName );
        String      GetImportFilterName( sal_uInt16 nFormat );
        String      GetImportFormatName( sal_uInt16 nFormat );
        String      GetImportFormatExtension( sal_uInt16 nFormat );
        String      GetImportFormatMediaType( sal_uInt16 nFormat );
        String      GetImportFormatShortName( sal_uInt16 nFormat );
        String      GetImportWildcard( sal_uInt16 nFormat );

        sal_Bool    IsImportInternalFilter( sal_uInt16 nFormat );
        sal_Bool    IsImportPixelFormat( sal_uInt16 nFormat );
        sal_Bool    IsImportDialog( sal_uInt16 nFormat );

        sal_uInt16  GetExportFormatCount() const { return aExport.size(); };
        sal_uInt16  GetExportFormatNumber( const String& rFormatName );
        sal_uInt16  GetExportFormatNumberForMediaType( const String& rMediaType );
        sal_uInt16  GetExportFormatNumberForShortName( const String& rShortName );
        String      GetExportFilterName( sal_uInt16 nFormat );
        String      GetExportFormatName( sal_uInt16 nFormat );
        String      GetExportFormatExtension( sal_uInt16 nFormat );
        String      GetExportFormatMediaType( sal_uInt16 nFormat );
        String      GetExportFormatShortName( sal_uInt16 nFormat );
        String      GetExportWildcard( sal_uInt16 nFormat );

        sal_Bool    IsExportInternalFilter( sal_uInt16 nFormat );
        sal_Bool    IsExportPixelFormat( sal_uInt16 nFormat );
        sal_Bool    IsExportDialog( sal_uInt16 nFormat );

                    FilterConfigCache();
                    ~FilterConfigCache();

};

#endif  // _FILTER_CONFIG_CACHE_HXX_

