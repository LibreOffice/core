/*************************************************************************
 *
 *  $RCSfile: docfilt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mba $ $Date: 2001-02-22 09:28:10 $
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

// INCLUDE ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
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

#pragma hdrstop

#include "docfac.hxx"
#include "docfilt.hxx"
#include "fltfnc.hxx"
#include "app.hxx"
#include "sfxresid.hxx"
#include "doc.hrc"
#include "sfxuno.hxx"

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
                       const String &rUsrDat ):
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
    pContainer( pContainerP )
{
    InitMembers_Impl();
    pContainer = pContainerP;
    aFilterName = rName;
    aMimeType = rMimeType;
}


SfxFilter::SfxFilter(  const char* pName, const String &rWildCard,
                       SfxFilterFlags nType,
                       const SfxFilterContainer* pContainerP )
    : lFormat(0),
      nFormatType(nType),
      aWildCard(rWildCard, ';'),
      nDocIcon(0),
      pContainer( pContainerP )
{
    aName = String::CreateFromAscii( pName );
    aFilterName = String::CreateFromAscii( pName );
    InitMembers_Impl();
    pContainer = pContainerP;
}

SfxFilter::SfxFilter(  const char* pName, const String &rWildCard,
                       SfxFilterFlags nType, const String &rTypeName,
                       const SfxFilterContainer* pContainerP )
    : lFormat(0),
      nFormatType(nType),
      aWildCard(rWildCard, ';'),
      aTypeName( rTypeName ),
      nDocIcon(0),
      pContainer( pContainerP )
{
    aName = String::CreateFromAscii( pName );
    aFilterName = String::CreateFromAscii( pName );
    InitMembers_Impl();
    pContainer = pContainerP;
}

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

    aName = pContainer->GetName();
    aName += DEFINE_CONST_UNICODE( ": " );
    aName += aFilterName;

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

SfxOfficeFilter::SfxOfficeFilter( const SfxFilterContainer* pContainer ) :
    SfxFilter(
        String( SfxResId( RID_OFFICEFILTER ) ),
        String( SfxResId( RID_OFFICEFILTER_WILDCARD ) ),
        SFX_FILTER_IMPORT | SFX_FILTER_REDIRECT | SFX_FILTER_NOTINCHOOSER,
        Exchange::RegisterFormatName( DEFINE_CONST_UNICODE("OfficeDocument") ),
        String( SfxResId( RID_OFFICEFILTER_MACTYPE ) ),
        String( SfxResId( RID_OFFICEFILTER_OS2TYPE ) ),
        0, String(), pContainer, String() )
{
}


