/*************************************************************************
 *
 *  $RCSfile: FilterConfigCache.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: sj $ $Date: 2001-08-07 13:03:10 $
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
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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

const char* FilterConfigCache::FilterConfigCacheEntry::ExternalPixelFilterNameList[] =
{
    "egi", "epn", "icd", "ipd", "ipx", "ipb", "epb", "epg",
    "epp", "ira", "era", "itg", "iti", "eti", "exp", NULL
};

sal_Bool FilterConfigCache::FilterConfigCacheEntry::IsValid()
{
    return sFilterName.Len() != 0;
}

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
            bIsInternalFilter = sal_True;
    }
    if ( !bIsInternalFilter )
    {
        for ( pPtr = ExternalPixelFilterNameList; *pPtr && ( bIsPixelFormat == sal_False ); pPtr++ )
        {
            if ( sFilterName.EqualsIgnoreCaseAscii( *pPtr ) )
                bIsPixelFormat = sal_True;
        }
        String aTemp( OUString::createFromAscii( SVLIBRARY( "?" ) ) );
        xub_StrLen nIndex = aTemp.Search( (sal_Unicode)'?' );
        aTemp.Replace( nIndex, 1, sFilterName );
        sFilterName = aTemp;
    }
    return sFilterName.Len() != 0;
}

String FilterConfigCache::FilterConfigCacheEntry::GetShortName()
{
    String aShortName;
    if ( lExtensionList.getLength() )
    {
        aShortName = lExtensionList[ 0 ];
        if ( aShortName.SearchAscii( "*.", 0 ) == 0 )
            aShortName.Erase( 0, 2 );
    }
    return aShortName;
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

sal_Bool FilterConfigCache::ImplAddFilterEntry( sal_Int32& nFlags,
                                                const Sequence< PropertyValue >& rFilterProperties,
                                                    const Reference< XNameAccess >& xTypeAccess )
{
    static OUString sExtensions         ( RTL_CONSTASCII_USTRINGPARAM( "Extensions" ) );
    static OUString sMediaType          ( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );

    static OUString sTrue               ( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

    sal_Bool bFilterEntryCreated = sal_False;

    try
    {
        FilterConfigCacheEntry aEntry;

        if ( nIndType >= 0 )
            rFilterProperties[ nIndType ].Value >>= aEntry.sType;
        if ( nIndUIName >= 0 )
            rFilterProperties[ nIndUIName ].Value >>= aEntry.sUIName;
        if ( nIndDocumentService >= 0 )
            rFilterProperties[ nIndDocumentService ].Value >>= aEntry.sDocumentService;
        if ( nIndFilterService >= 0 )
            rFilterProperties[ nIndFilterService ].Value >>= aEntry.sFilterService;
        if ( nIndFlags >= 0 )
            rFilterProperties[ nIndFlags ].Value >>= aEntry.nFlags;
        if ( nIndUserData >= 0 )
        {
            Sequence < OUString > lUserData;
            rFilterProperties[ nIndUserData ].Value >>= lUserData;
            if ( lUserData.getLength() == TOKEN_COUNT_FOR_OWN_FILTER )
            {
                aEntry.bHasDialog = lUserData[ TOKEN_INDEX_FOR_HASDIALOG ].equalsIgnoreAsciiCase( sTrue );
                aEntry.CreateFilterName( lUserData[ TOKEN_INDEX_FOR_FILTER ] );
            }
        }
        if ( nIndFileFormatVersion >= 0 )
            rFilterProperties[ nIndFileFormatVersion ].Value >>= aEntry.nFileFormatVersion;
        if ( nIndTemplateName >= 0 )
            rFilterProperties[ nIndTemplateName ].Value >>= aEntry.sTemplateName;

        if ( aEntry.IsValid() )
        {
            // trying to get the corresponding type for this filter
            if ( xTypeAccess->hasByName( aEntry.sType ) )
            {
                Any aTypePropertySet = xTypeAccess->getByName( aEntry.sType );
                Sequence< PropertyValue > lProperties;
                aTypePropertySet >>= lProperties;
                sal_Int32 j, nCount = lProperties.getLength();

                for ( j = 0; j < nCount; j++ )
                {
                    PropertyValue aPropValue( lProperties[ j ] );
                    if ( aPropValue.Name.equals( sExtensions ) )
                        aPropValue.Value >>= aEntry.lExtensionList;
                    else if ( aPropValue.Name.equals( sMediaType ) )
                        aPropValue.Value >>= aEntry.sMediaType;
                }
                // The first extension will be used
                // to generate our internal FilterType ( BMP, WMF ... )

                String aExtension( aEntry.GetShortName() );
                if ( aExtension.Len() == 3 )
                {
                    if ( aEntry.nFlags & 1 )
                        aImport.push_back( aEntry );
                    if ( aEntry.nFlags & 2 )
                        aExport.push_back( aEntry );
                    if ( aEntry.nFlags & 3 )
                        bFilterEntryCreated = sal_True;
                    nFlags = aEntry.nFlags;
                }
            }
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
    static OUString sType ( RTL_CONSTASCII_USTRINGPARAM( "Type" ) );

    // get global uno service manager
    Reference< XMultiServiceFactory > xSMGR = getProcessServiceFactory();

    // create type detection service
    Reference< XNameAccess > xTypeAccess( xSMGR->createInstance(
        OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ), UNO_QUERY );

    Reference< XNameAccess > xFilterAccess( xSMGR->createInstance(
        OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ), UNO_QUERY );
    Sequence< OUString > lAllFilter = xFilterAccess->getElementNames();
    sal_Int32 nAllFilterCount = lAllFilter.getLength();

    sal_Int32 i, j;

    for ( i = 0; i < nAllFilterCount; i++ )
    {
        OUString sInternalTypeName = lAllFilter[ i ];
        Any aTypePropertySet = xFilterAccess->getByName( sInternalTypeName );

        Sequence< PropertyValue > lProperties;
        aTypePropertySet >>= lProperties;
        if ( !bInitialized )
        {
            sal_Int32 i, nCount = lProperties.getLength();
            for ( i = 0; i < nCount; i++ )
            {
                PropertyValue aPropValue( lProperties[ i ] );
                if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "Type" ) ) ) )
                    nIndType = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ) ) ) )
                    nIndUIName = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentService" ) ) ) )
                    nIndDocumentService = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterService" ) ) ) )
                    nIndFilterService = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "Flags" ) ) ) )
                    nIndFlags = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "UserData" ) ) ) )
                    nIndUserData = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "FileFormatVersion" ) ) ) )
                    nIndFileFormatVersion = i;
                else if ( aPropValue.Name.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "TemplateName" ) ) ) )
                    nIndTemplateName = i;
            }
            bInitialized = sal_True;
        }
        if ( ImplIsOwnFilter( lProperties ) )
        {
            sal_Int32 nFlags = 0;
            if ( ImplAddFilterEntry( nFlags, lProperties, xTypeAccess ) )
            {
                if ( ( nFlags & 1 ) && ( nIndType >= 0 )  )  // import filter ?
                {
                    CacheVector::iterator aIter( aImport.end() - 1 );
                    for ( j = 0; j < nAllFilterCount; j++ )
                    {
                        if ( i != j )
                        {
                            OUString sInternalTypeName = lAllFilter[ j ];
                            Any aTypePropertySet = xFilterAccess->getByName( sInternalTypeName );
                            Sequence< PropertyValue > l2Properties;
                            aTypePropertySet >>= l2Properties;
                            sal_Int32 n2Flags;
                            l2Properties[ nIndFlags ].Value >>= n2Flags;
                            if ( n2Flags & 1 )
                            {
                                OUString s2Type;
                                l2Properties[ nIndType ].Value >>= s2Type;
                                if ( s2Type.equals( aIter->sType ) )
                                {
                                    sal_Int32 nIndex = sInternalTypeName.lastIndexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( ": " ) ) );
                                    if ( nIndex >= 0 )
                                        aIter->sFilterType = sInternalTypeName.copy( nIndex + 2 );
                                    else
                                        aIter->sFilterType = sInternalTypeName;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
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
    "png","2","epn",
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

        ByteString sFlags( *pPtr++ );
        aEntry.nFlags = sFlags.ToInt32();

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
        if ( aIter->sUIName.equalsIgnoreAsciiCase( rFormatName ) )
            break;
        aIter++;
    }
    return aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin();
}

sal_uInt16 FilterConfigCache::GetImportFormatNumberForMediaType( const String& rMediaType )
{
    CacheVector::iterator aIter( aImport.begin() );
    while ( aIter != aImport.end() )
    {
        if ( aIter->sMediaType.equalsIgnoreAsciiCase( rMediaType ) )
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
        if ( aIter->GetShortName().EqualsIgnoreCaseAscii( rShortName ) )
            break;
        aIter++;
    }
    return aIter == aImport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aImport.begin();
}

String FilterConfigCache::GetImportFormatName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    String aUIName;
    if ( aIter < aImport.end() )
        aUIName = aIter->sUIName;
    return aUIName;
}

String FilterConfigCache::GetImportFormatMediaType( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    String aMediaType;
    if ( aIter < aImport.end() )
        aMediaType = aIter->sMediaType;
    return aMediaType;
}

String FilterConfigCache::GetImportFormatShortName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    String aType;
    if ( aIter < aImport.end() )
        aType = aIter->GetShortName();
    aType.ToUpperAscii();
    return aType;
}

String FilterConfigCache::GetImportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    String aExtension;
    if ( aIter < aImport.end() )
    {
        if ( nEntry < aIter->lExtensionList.getLength() )
            aExtension = aIter->lExtensionList[ nEntry ];
    }
    return aExtension;
}

String FilterConfigCache::GetImportFilterType( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    String aType;
    if ( aIter < aImport.end() )
        aType = aIter->sType;
    return aType;
}

String FilterConfigCache::GetImportFilterTypeName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aImport.begin() + nFormat );
    String aFilterType;
    if ( aIter < aImport.end() )
        aFilterType = aIter->sFilterType;
    return aFilterType;
}

String FilterConfigCache::GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    String aWildcard( GetImportFormatExtension( nFormat, nEntry ) );
    if ( aWildcard.Len() )
        aWildcard.Insert( UniString::CreateFromAscii( "*.", 2 ), 0 );
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
        if ( aIter->sUIName.equalsIgnoreAsciiCase( rFormatName ) )
            break;
        aIter++;
    }
    return aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin();
}

sal_uInt16 FilterConfigCache::GetExportFormatNumberForMediaType( const String& rMediaType )
{
    CacheVector::iterator aIter( aExport.begin() );
    while ( aIter != aExport.end() )
    {
        if ( aIter->sMediaType.equalsIgnoreAsciiCase( rMediaType ) )
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
        if ( aIter->GetShortName().EqualsIgnoreCaseAscii( rShortName ) )
            break;
        aIter++;
    }
    return aIter == aExport.end() ? GRFILTER_FORMAT_NOTFOUND : aIter - aExport.begin();
}

String FilterConfigCache::GetExportFormatName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    String aUIName;
    if ( aIter < aExport.end() )
        aUIName = aIter->sUIName;
    return aUIName;
}

String FilterConfigCache::GetExportFormatMediaType( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    String aMediaType;
    if ( aIter < aExport.end() )
        aMediaType = aIter->sMediaType;
    return aMediaType;
}

String FilterConfigCache::GetExportFormatShortName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    String aType;
    if ( aIter < aExport.end() )
        aType = aIter->GetShortName();
    aType.ToUpperAscii();
    return aType;
}

String FilterConfigCache::GetExportFormatExtension( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    String aExtension;
    if ( aIter < aExport.end() )
    {
        if ( nEntry < aIter->lExtensionList.getLength() )
            aExtension = aIter->lExtensionList[ nEntry ];
    }
    return aExtension;
}

String FilterConfigCache::GetExportFilterTypeName( sal_uInt16 nFormat )
{
    CacheVector::iterator aIter( aExport.begin() + nFormat );
    String aFilterType;
    if ( aIter < aImport.end() )
        aFilterType = aIter->sFilterType;
    return aFilterType;
}

String FilterConfigCache::GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    String aWildcard( GetExportFormatExtension( nFormat, nEntry ) );
    if ( aWildcard.Len() )
        aWildcard.Insert( UniString::CreateFromAscii( "*.", 2 ), 0 );
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
