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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#ifdef SOLARIS
// HACK: prevent conflict between STLPORT and Workshop headers on Solaris 8
#include <ctime>
#endif

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers
#include <sot/exchange.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include "fltfnc.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/objsh.hxx>

using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxFilter)

SfxFilter::SfxFilter(  const String &rName,
                       const String &rWildCard,
                       SfxFilterFlags nType,
                       sal_uInt32 lFmt,
                       const String &rTypNm,
                       sal_uInt16 nIcon,
                       const String &rMimeType,
                       const String &rUsrDat,
                       const String &rServiceName ):
    aWildCard(rWildCard, ';'),
    lFormat(lFmt),
    aTypeName(rTypNm),
    aUserData(rUsrDat),
    nFormatType(nType),
    nDocIcon(nIcon),
    aServiceName( rServiceName ),
    aMimeType( rMimeType ),
    aFilterName( rName )
{
    String aExts = GetWildcard()();
    String aShort, aLong;
    String aRet;
    sal_uInt16 nMaxLength = USHRT_MAX;
    String aTest;
    sal_uInt16 nPos = 0;
    while( ( aRet = aExts.GetToken( nPos++, ';' ) ).Len() )
    {
        aTest = aRet;
        aTest.SearchAndReplace( DEFINE_CONST_UNICODE( "*." ), String() );
        if( aTest.Len() <= nMaxLength )
        {
            if( aShort.Len() ) aShort += ';';
            aShort += aRet;
        }
        else
        {
            if( aLong.Len() ) aLong += ';';
            aLong += aRet;
        }
    }
    if( aShort.Len() && aLong.Len() )
    {
        aShort += ';';
        aShort += aLong;
    }
    aWildCard = aShort;

    nVersion = SOFFICE_FILEFORMAT_50;
    aUIName = aFilterName;
}

SfxFilter::~SfxFilter()
{
}

String SfxFilter::GetDefaultExtension() const
{
    return GetWildcard()().GetToken( 0, ';' );
}

String SfxFilter::GetSuffixes() const
{
    String aRet = GetWildcard()();
    while( aRet.SearchAndReplaceAscii( "*.", String() ) != STRING_NOTFOUND ) ;
    while( aRet.SearchAndReplace( ';', ',' ) != STRING_NOTFOUND ) ;
    return aRet;
}

const SfxFilter* SfxFilter::GetDefaultFilter( const String& rName )
{
    return SfxFilterContainer::GetDefaultFilter_Impl( rName );
}

const SfxFilter* SfxFilter::GetDefaultFilterFromFactory( const String& rFact )
{
    return GetDefaultFilter( SfxObjectShell::GetServiceNameFromFactory( rFact ) );
}

const SfxFilter* SfxFilter::GetFilterByName( const String& rName )
{
    SfxFilterMatcher aMatch;
    return aMatch.GetFilter4FilterName( rName, 0, 0 );
}

String SfxFilter::GetTypeFromStorage( const SotStorage& rStg )
{
    const char* pType=0;
    if ( rStg.IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "WordDocument" ) ) ) )
    {
        if ( rStg.IsStream( String::CreateFromAscii("0Table" ) ) || rStg.IsStream( String::CreateFromAscii("1Table" ) ) )
            pType = "writer_MS_Word_97";
        else
            pType = "writer_MS_Word_95";
    }
    else if ( rStg.IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Book" ) ) ) )
    {
        pType = "calc_MS_Excel_95";
    }
    else if ( rStg.IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Workbook" ) ) ) )
    {
        pType = "calc_MS_Excel_97";
    }
    else if ( rStg.IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PowerPoint Document" ) ) ) )
    {
        pType = "impress_MS_PowerPoint_97";
    }
    else if ( rStg.IsStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Equation Native" ) ) ) )
    {
        pType = "math_MathType_3x";
    }
    else
    {
        sal_Int32 nClipId = ((SotStorage&)rStg).GetFormat();
        if ( nClipId )
        {
            const SfxFilter* pFilter = SfxFilterMatcher().GetFilter4ClipBoardId( nClipId );
            if ( pFilter )
                return pFilter->GetTypeName();
        }
    }

    return pType ? String::CreateFromAscii(pType) : String();
}

String SfxFilter::GetTypeFromStorage( const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xStorage, sal_Bool bTemplate,
                                        String* pFilterName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    SfxFilterMatcher aMatcher;
    const char* pType=0;
    String aName;
    if ( pFilterName )
    {
        aName = *pFilterName;
        pFilterName->Erase();
    }

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xProps( xStorage, com::sun::star::uno::UNO_QUERY );
    if ( xProps.is() )
    {
        ::rtl::OUString aMediaType;
        xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) ) >>= aMediaType;
        if ( aMediaType.getLength() )
        {
            ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
            aDataFlavor.MimeType = aMediaType;
            sal_uInt32 nClipId = SotExchange::GetFormat( aDataFlavor );
            if ( nClipId )
            {
                SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;
                if ( bTemplate )
                    // template filter was preselected, try to verify
                    nMust |= SFX_FILTER_TEMPLATEPATH;
                else
                    // template filters shouldn't be detected if not explicitly asked for
                    nDont |= SFX_FILTER_TEMPLATEPATH;

                const SfxFilter* pFilter = 0;
                if ( aName.Len() )
                    // get preselected Filter if it matches the desired filter flags
                    pFilter = aMatcher.GetFilter4FilterName( aName, nMust, nDont );

                if ( !pFilter || pFilter->GetFormat() != nClipId )
                {
                    // get filter from storage MediaType
                    pFilter = aMatcher.GetFilter4ClipBoardId( nClipId, nMust, nDont );
                    if ( !pFilter )
                        // template filter is asked for , but there isn't one; so at least the "normal" format should be detected
                        // or storage *is* a template, but bTemplate is not set
                        pFilter = aMatcher.GetFilter4ClipBoardId( nClipId );
                }

                if ( pFilter )
                {
                    if ( pFilterName )
                        *pFilterName = pFilter->GetName();
                    return pFilter->GetTypeName();
                }
            }
        }
    }

    //TODO: do it without SfxFilter
    //TODO/LATER: don't yield FilterName, should be done in FWK!
    String aRet;
    if ( pType )
    {
        aRet = String::CreateFromAscii(pType);
        if ( pFilterName )
            *pFilterName = aMatcher.GetFilter4EA( aRet )->GetName();
    }

    return aRet;
}
