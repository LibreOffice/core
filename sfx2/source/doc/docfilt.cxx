/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docfilt.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:39:13 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef SOLARIS
// HACK: prevent conflict between STLPORT and Workshop headers on Solaris 8
#include <ctime>
#endif

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINMODE_HPP_
#include <com/sun/star/plugin/PluginMode.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINDESCRIPTION_HPP_
#include <com/sun/star/plugin/PluginDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINEXCEPTION_HPP_
#include <com/sun/star/plugin/PluginException.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINVARIABLE_HPP_
#include <com/sun/star/plugin/PluginVariable.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGIN_HPP_
#include <com/sun/star/plugin/XPlugin.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINMANAGER_HPP_
#include <com/sun/star/plugin/XPluginManager.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINCONTEXT_HPP_
#include <com/sun/star/plugin/XPluginContext.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#pragma hdrstop

#include "docfac.hxx"
#include "docfilt.hxx"
#include "fltfnc.hxx"
#include "app.hxx"
#include "sfxresid.hxx"
#include "doc.hrc"
#include "sfxuno.hxx"

using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxFilter);

SfxFilter::SfxFilter(  const String &rName,
                       const String &rWildCard,
                       SfxFilterFlags nType,
                       sal_uInt32 lFmt,
                       const String &rMacTyp,
                       const String &rTypNm,
                       sal_uInt16 nIcon,
                       const String &rMimeType,
                       const SfxFilterContainer* pContainerP,
                       const String &rUsrDat,
                       const String &rServiceName ):
    lFormat(lFmt),
    nFormatType(nType),
    aWildCard(rWildCard, ';'),
    aMacType(rMacTyp),
    aTypeName(rTypNm),
    nDocIcon(nIcon),
    aUserData(rUsrDat),
    aName( rName ),
    aMimeType( rMimeType ),
    aFilterName( rName ),
    aServiceName( rServiceName )
//  ,pContainer( pContainerP )
{
    InitMembers_Impl();
}

/*
SfxFilter::SfxFilter(  const char* pName, const String &rWildCard,
                       SfxFilterFlags nType,
                       const SfxFilterContainer* pContainerP )
    : lFormat(0),
      nFormatType(nType),
      aWildCard(rWildCard, ';'),
      nDocIcon(0)
//    ,pContainer( pContainerP )
{
    aName = String::CreateFromAscii( pName );
    aFilterName = String::CreateFromAscii( pName );
    InitMembers_Impl();
}

SfxFilter::SfxFilter(  const char* pName, const String &rWildCard,
                       SfxFilterFlags nType, const String &rTypeName,
                       const SfxFilterContainer* pContainerP )
    : lFormat(0),
      nFormatType(nType),
      aWildCard(rWildCard, ';'),
      aTypeName( rTypeName ),
      nDocIcon(0)
//    ,pContainer( pContainerP )
{
    aName = String::CreateFromAscii( pName );
    aFilterName = String::CreateFromAscii( pName );
    InitMembers_Impl();
}
*/
void SfxFilter::InitMembers_Impl()
{
    String aExts = GetWildcard()();
    String aShort, aLong;
    String aRet;
    sal_uInt16 nMaxLength =
#if defined( WIN ) || defined( OS2 )
        3
#else
        USHRT_MAX
#endif
        ;
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
    bPlugDataSearched = 0;
    pPlugData = 0;
    aUIName = aFilterName;
}

SfxFilter::~SfxFilter()
{
    delete pPlugData;
}

const String&   SfxFilter::GetTypeName() const
{
    return
#ifdef MAC
    aMacType;
#else
    aTypeName;
#endif
}

const ::com::sun::star::plugin::PluginDescription* SfxFilter::GetPlugData()
{
    if( !bPlugDataSearched )
    {
        bPlugDataSearched = sal_True;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference< ::com::sun::star::plugin::XPluginManager >
                xPlugMgr( xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.plugin.PluginManager" ) ), ::com::sun::star::uno::UNO_QUERY );
        DBG_ASSERT( xPlugMgr.is(), "### cannot create instance com.sun.star.plugin.PluginManager!" );
        if( xPlugMgr.is() )
        {
            ::com::sun::star::uno::Sequence < ::com::sun::star::plugin::PluginDescription > aDescr = xPlugMgr->getPluginDescriptions();
            const ::com::sun::star::plugin::PluginDescription *pArr = aDescr.getConstArray();
            sal_uInt16 n;
            for (n=0; n<aDescr.getLength(); n++ )
            {
                const ::com::sun::star::plugin::PluginDescription& rData = pArr[n];
                String aTest = rData.Description;
                aTest += DEFINE_CONST_UNICODE( " (PlugIn)" );
                if( aTest == GetFilterName() )
                    break;
            }

            const ::com::sun::star::plugin::PluginDescription& rData = pArr[n];
            ::com::sun::star::plugin::PluginDescription *pPlug = new ::com::sun::star::plugin::PluginDescription;
            pPlug->PluginName = rData.PluginName;
            pPlug->Mimetype = rData.Mimetype;
            pPlug->Extension = rData.Extension;
            pPlug->Description = rData.Description;

            pPlugData = pPlug;
        }
    }

    return pPlugData;
}

sal_Bool SfxFilter::IsFirstPlugin() const
{
    const ::com::sun::star::plugin::PluginDescription* pData = GetPlugData();
    if( pData )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference< ::com::sun::star::plugin::XPluginManager >
            xPlugMgr( xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.plugin.PluginManager" ) ), ::com::sun::star::uno::UNO_QUERY );
        DBG_ASSERT( xPlugMgr.is(), "### cannot create instance com.sun.star.plugin.PluginManager!" );
        if( xPlugMgr.is() )
        {
            ::com::sun::star::uno::Sequence < ::com::sun::star::plugin::PluginDescription > aDescr = xPlugMgr->getPluginDescriptions();
            String aPlug;
            const ::com::sun::star::plugin::PluginDescription *pArr = aDescr.getConstArray();
            String aTest;
            for ( sal_uInt16 n=0; n<aDescr.getLength(); n++ )
            {
                if( pArr[n].Description == pData->Description )
                    return sal_True;
                else if( pArr[n].PluginName == pData->PluginName )
                        return sal_False;
            }
        }
    }
    return sal_False;
}


String SfxFilter::GetDefaultExtension() const
{
    return GetWildcard()().GetToken( 0, ';' );
}


String SfxFilter::GetSuffixes() const
{
    String aRet = GetWildcard()();
    while( aRet.SearchAndReplaceAscii( "*.", String() ) != STRING_NOTFOUND );
    while( aRet.SearchAndReplace( ';', ',' ) != STRING_NOTFOUND );
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
            const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( nClipId );
            if ( pFilter )
                return pFilter->GetTypeName();
        }
    }

    return pType ? String::CreateFromAscii(pType) : String();
}

String SfxFilter::GetTypeFromStorage( const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xStorage, BOOL bTemplate,
                                        String* pFilterName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
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
                    pFilter = rMatcher.GetFilter4FilterName( aName, nMust, nDont );

                if ( !pFilter || pFilter->GetFormat() != nClipId )
                {
                    // get filter from storage MediaType
                    pFilter = rMatcher.GetFilter4ClipBoardId( nClipId, nMust, nDont );
                    if ( !pFilter && bTemplate )
                        // template filter is asked for , but there isn't one; so at least the "normal" format should be detected
                        pFilter = rMatcher.GetFilter4ClipBoardId( nClipId );
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
            *pFilterName = rMatcher.GetFilter4EA( aRet )->GetName();
    }

    return aRet;
}
