/*************************************************************************
 *
 *  $RCSfile: FilterConfigCache.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2001-02-22 11:26:12 $
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
#include "FilterConfigCache.hxx"
#endif
#ifndef _FILTER_HXX
#include <filter.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#define TOKEN_COUNT_FOR_OWN_FILTER      3
#define TOKEN_INDEX_FOR_IDENT           0
#define TOKEN_INDEX_FOR_FILTER          1
#define TOKEN_INDEX_FOR_HASDIALOG       2

using namespace ::com::sun::star::lang          ;   // XMultiServiceFactory, getProcessServiceFactory()
using namespace ::com::sun::star::container     ;   // XNameAccess
using namespace ::com::sun::star::uno           ;   // Reference
using namespace ::com::sun::star::beans         ;   // PropertyValue
using namespace ::utl                           ;   // getProcessServiceFactory();
using namespace ::rtl                           ;

const char* FilterConfigCache::FilterConfigCacheEntry::InternalPixelFilterNameList[] =
{
    IMP_BMP, IMP_GIF, IMP_PNG,IMP_JPEG, IMP_XBM, IMP_XPM,
    EXP_BMP, EXP_JPEG, NULL
};

const char* FilterConfigCache::FilterConfigCacheEntry::InternalVectorFilterNameList[] =
{
    IMP_SVMETAFILE, IMP_WMF, IMP_EMF, IMP_SVSGF, IMP_SVSGV,
    EXP_SVMETAFILE, EXP_WMF, EXP_EMF, EXP_SVG, NULL
};

sal_Bool FilterConfigCache::FilterConfigCacheEntry::IsValid()
{
    return sFilterName.Len() != 0;
}

sal_Bool FilterConfigCache::FilterConfigCacheEntry::CreateFilterName( const OUString& rUserDataEntry )
{
    bIsInternalFilter = sal_False;
    sFilterName = String( rUserDataEntry );
    const char** pPtr;
    for ( pPtr = InternalPixelFilterNameList; *pPtr && ( bIsInternalFilter == sal_False ); pPtr++ )
    {
        if ( sFilterName.EqualsIgnoreCaseAscii( *pPtr ) )
        {
            bIsInternalFilter = sal_True;
            bIsPixelFormat = sal_True;
        }
    }
    for ( pPtr = InternalVectorFilterNameList; *pPtr && ( bIsInternalFilter == sal_False ); pPtr++ )
    {
        if ( sFilterName.EqualsIgnoreCaseAscii( *pPtr ) )
        {
            bIsInternalFilter = sal_True;
            bIsPixelFormat = sal_False;
        }
    }
    if ( !bIsInternalFilter )
    {
        String aTemp( OUString::createFromAscii( SVLIBRARY( "?" ) ) );
        xub_StrLen nIndex = aTemp.Search( (sal_Unicode)'?' );
        aTemp.Replace( nIndex, 1, sFilterName );
        sFilterName = aTemp;
    }
    return sFilterName.Len() != 0;
}

FilterConfigCache::FilterConfigCache()
{
    ImplInit();
}

FilterConfigCache::~FilterConfigCache()
{

}

sal_Bool FilterConfigCache::ImplIsOwnFilter( const Sequence< PropertyValue >& rFilterProperties )
{
    static OUString sUserData   ( RTL_CONSTASCII_USTRINGPARAM( "UserData" ) );
    static OUString sMagic      ( RTL_CONSTASCII_USTRINGPARAM( "OO" ) );

    sal_Bool bIsOwn = sal_False;
    sal_Int32 i, nCount = rFilterProperties.getLength();
    for ( i = 0; i < nCount; i++ )
    {
        if ( rFilterProperties[ i ].Name.equals( sUserData ) )
        {
            Sequence < OUString > lUserData;
            rFilterProperties[ i ].Value >>= lUserData;
            if ( lUserData.getLength() == TOKEN_COUNT_FOR_OWN_FILTER )
            {
                if ( lUserData[ TOKEN_INDEX_FOR_IDENT ].equals( sMagic ) )
                    bIsOwn = sal_True;
            }
            break;
        }
    }
    return bIsOwn;
}

sal_Bool FilterConfigCache::ImplAddFilterEntry( const Sequence< PropertyValue >& rFilterProperties )
{
    static OUString sType               ( RTL_CONSTASCII_USTRINGPARAM( "Type" ) );
    static OUString sUIName             ( RTL_CONSTASCII_USTRINGPARAM( "UIName" ) );
    static OUString sDocumentService    ( RTL_CONSTASCII_USTRINGPARAM( "DocumentService" ) );
    static OUString sFilterService      ( RTL_CONSTASCII_USTRINGPARAM( "FilterService" ) );
    static OUString sFlags              ( RTL_CONSTASCII_USTRINGPARAM( "Flags" ) );
    static OUString sUserData           ( RTL_CONSTASCII_USTRINGPARAM( "UserData" ) );
    static OUString sFileFormatVersion  ( RTL_CONSTASCII_USTRINGPARAM( "FileFormatVersion" ) );
    static OUString sTemplateName       ( RTL_CONSTASCII_USTRINGPARAM( "TemplateName" ) );

    static OUString sTrue               ( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

    sal_Bool bFilterEntryCreated = sal_False;
    FilterConfigCacheEntry  aEntry;

    try
    {
        sal_Int32 i, nCount = rFilterProperties.getLength();
        for ( i = 0; i < nCount; i++ )
        {
            PropertyValue aPropValue( rFilterProperties[ i ] );
            if ( aPropValue.Name.equals( sType ) )
                aPropValue.Value >>= aEntry.sType;
            else if ( aPropValue.Name.equals( sUIName ) )
                aPropValue.Value >>= aEntry.sUIName;
            else if ( aPropValue.Name.equals( sDocumentService ) )
                aPropValue.Value >>= aEntry.sDocumentService;
            else if ( aPropValue.Name.equals( sFilterService ) )
                aPropValue.Value >>= aEntry.sFilterService;
            else if ( aPropValue.Name.equals( sFlags ) )
                aPropValue.Value >>= aEntry.nFlags;
            else if ( aPropValue.Name.equals( sUserData ) )
            {
                Sequence < OUString > lUserData;
                aPropValue.Value >>= lUserData;
                if ( lUserData.getLength() == TOKEN_COUNT_FOR_OWN_FILTER )
                {
                    aEntry.bHasDialog = lUserData[ TOKEN_INDEX_FOR_HASDIALOG ].equalsIgnoreCase( sTrue );
                    aEntry.CreateFilterName( lUserData[ TOKEN_INDEX_FOR_FILTER ] );
                }
            }
            else if ( aPropValue.Name.equals( sFileFormatVersion ) )
                aPropValue.Value >>= aEntry.nFileFormatVersion;
            else if ( aPropValue.Name.equals( sTemplateName ) )
                aPropValue.Value >>= aEntry.sTemplateName;
        }
        if ( aEntry.IsValid() )
        {
            if ( aEntry.nFlags & 1 )
                aImport.push_back( aEntry );
            if ( aEntry.nFlags & 2 )
                aExport.push_back( aEntry );
            if ( aEntry.nFlags & 3 )
                bFilterEntryCreated = sal_True;
        }
    }
    catch ( ::com::sun::star::uno::Exception& )
    {
        DBG_ERROR( "FilterConfigCache::FilterConfigEntry::ImplAddFilterEntry : exception while reading the filter properties" );
    }
    return bFilterEntryCreated;
}

void FilterConfigCache::ImplInit()
{
    // get global uno service manager
    Reference< XMultiServiceFactory > xSMGR = getProcessServiceFactory();

    // create type detection service
    Reference< XNameAccess > xTypeAccess( xSMGR->createInstance(
        OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ), UNO_QUERY );
    Sequence< OUString > lAllTypes = xTypeAccess->getElementNames();
    sal_Int32 nAllTypesCount = lAllTypes.getLength();

    Reference< XNameAccess > xFilterAccess( xSMGR->createInstance(
        OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ), UNO_QUERY );
    Sequence< OUString > lAllFilter = xFilterAccess->getElementNames();
    sal_Int32 nAllFilterCount = lAllFilter.getLength();

    sal_Int32 i;
/*
    for ( i = 0; i < nAllTypesCount; i++ )
    {
        OUString sInternalTypeName = lAllTypes[ i ];
        Any aTypePropertySet = xTypeAccess->getByName( sInternalTypeName );

        Sequence< PropertyValue > lProperties;
        aTypePropertySet >>= lProperties;
        sal_Int32 j, nCount = lProperties.getLength();
        for ( j = 0; j < nCount; j++ )
        {
            PropertyValue aPropValue( lProperties[ j ] );
            OUString aName( aPropValue.Name );
        }
    }
*/
/*
    [0].Name    =   "UIName"
    [0].Value   =   localized name of filter as [string]

    [1].Name    =   "MediaType"
    [1].Value   =   mime type as [string]

    [2].Name    =   "ClipboardFormat"
    [2].Value   =   name of clipboard format as [string]

    [3].Name    =   "URLPattern"
    [3].Value   =   list of matching URL pattern to specify type; as [stringlist]

    [4].Name    =   "Extensions"
    [4].Value   =   list of matching file extensions to specify type; as [stringlist]

    [5].Name    =   "DocumentIconID"
    [5].Value   =   ID of a suitable document icon as [integer]
*/

    for ( i =  0; i < nAllFilterCount; i++ )
    {
        OUString sInternalTypeName = lAllFilter[ i ];
        Any aTypePropertySet = xFilterAccess->getByName( sInternalTypeName );

        Sequence< PropertyValue > lProperties;
        aTypePropertySet >>= lProperties;

        if ( ImplIsOwnFilter( lProperties ) )
            ImplAddFilterEntry( lProperties );
    }
};

// ------------------------------------------------------------------------

String FilterConfigCache::GetImportFilterName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    return ( aIter < aImport.end() ) ? aIter->sFilterName : String();
}

sal_uInt16 FilterConfigCache::GetImportFormatNumber( const String& rFormatName )
{
    CacheVector::iterator aIter( aImport.begin() );
    while ( aIter != aImport.end() )
    {
        if ( aIter->sUIName.equalsIgnoreCase( rFormatName ) )
            break;
        aIter++;
    }
    return aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin();
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForShortName( const String& rShortName )
{
    CacheVector::iterator aIter( aImport.begin() );
    while ( aIter != aImport.end() )
    {
        if ( aIter->sType.equalsIgnoreCase( rShortName ) )
            break;
        aIter++;
    }
    return aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin();
}

String FilterConfigCache::GetImportFormatName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    return ( aIter < aImport.end() ) ? aIter->sUIName : String();
}

String FilterConfigCache::GetImportFormatType( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    return ( aIter < aImport.end() ) ? aIter->sType : String();
}

String FilterConfigCache::GetImportFormatExtension( sal_uInt16 nFormat )
{
    return GetImportFormatType( nFormat );
}

String FilterConfigCache::GetImportFormatShortName( sal_uInt16 nFormat )
{
    return GetImportFormatType( nFormat );
}

String FilterConfigCache::GetImportWildcard( sal_uInt16 nFormat )
{
    String aWildcard( UniString::CreateFromAscii( "*.", 2 ) );
    aWildcard.Append( GetImportFormatType( nFormat ) );
    return aWildcard;
}

sal_Bool FilterConfigCache::IsImportInternalFilter( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    return ( aIter < aImport.end() ) ? aIter->bIsInternalFilter != 0 : sal_False;
}

sal_Bool FilterConfigCache::IsImportPixelFormat( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    return ( aIter < aImport.end() ) ? aIter->bIsPixelFormat != 0 : sal_False;
}

sal_Bool FilterConfigCache::IsImportDialog( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    return ( aIter < aImport.end() ) ? aIter->bHasDialog != 0: sal_False;
}

// ------------------------------------------------------------------------

String FilterConfigCache::GetExportFilterName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    return ( aIter < aExport.end() ) ? aIter->sFilterName : String();
}

sal_uInt16 FilterConfigCache::GetExportFormatNumber( const String& rFormatName )
{
    CacheVector::iterator aIter( aExport.begin() );
    while ( aIter != aExport.end() )
    {
        if ( aIter->sUIName.equalsIgnoreCase( rFormatName ) )
            break;
        aIter++;
    }
    return aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin();
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForShortName( const String& rShortName )
{
    CacheVector::iterator aIter( aExport.begin() );
    while ( aIter != aExport.end() )
    {
        if ( aIter->sType.equalsIgnoreCase( rShortName ) )
            break;
        aIter++;
    }
    return aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin();
}

String FilterConfigCache::GetExportFormatName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    return ( aIter < aExport.end() ) ? aIter->sUIName : String();
}

String FilterConfigCache::GetExportFormatType( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    return ( aIter < aExport.end() ) ? aIter->sType : String();
}

String FilterConfigCache::GetExportFormatExtension( sal_uInt16 nFormat )
{
    return GetExportFormatType( nFormat );
}

String FilterConfigCache::GetExportFormatShortName( sal_uInt16 nFormat )
{
    return GetExportFormatType( nFormat );
}

String FilterConfigCache::GetExportWildcard( sal_uInt16 nFormat )
{
    String aWildcard( UniString::CreateFromAscii( "*.", 2 ) );
    aWildcard.Append( GetExportFormatType( nFormat ) );
    return aWildcard;
}

sal_Bool FilterConfigCache::IsExportInternalFilter( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    return ( aIter < aExport.end() ) ? aIter->bIsInternalFilter != 0 : sal_False;
}

sal_Bool FilterConfigCache::IsExportPixelFormat( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    return ( aIter < aExport.end() ) ? aIter->bIsPixelFormat != 0 : sal_False;
}

sal_Bool FilterConfigCache::IsExportDialog( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    return ( aIter < aExport.end() ) ? aIter->bHasDialog != 0: sal_False;
}

// ------------------------------------------------------------------------
