/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateprotocol.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 16:00:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _COM_SUN_STAR_XML_XPATH_XXPATHAPI_HPP_
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#endif

#include "updateprotocol.hxx"

#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>

#include <cppuhelper/implbase1.hxx>

namespace css = com::sun::star ;
namespace deployment = css::deployment ;
namespace lang = css::lang ;
namespace uno = css::uno ;
namespace task = css::task ;
namespace xml = css::xml ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

//------------------------------------------------------------------------------

static bool
getBootstrapData(
    uno::Sequence< ::rtl::OUString > & rRepositoryList,
    ::rtl::OUString & rBuildID,
    ::rtl::OUString & rInstallSetID)
{
    rtl::OUString aPath, aPath2;
    if( osl_getExecutableFile(&aPath.pData) != osl_Process_E_None )
        return false;

    sal_uInt32 lastIndex = aPath.lastIndexOf('/');
    if ( lastIndex > 0 )
    {
        aPath = aPath.copy( 0, lastIndex+1 );
        aPath += UNISTRING( SAL_CONFIGFILE( "version" ) );
    }

    rtl::Bootstrap aVersionFile(aPath);
    aVersionFile.getFrom(UNISTRING("ProductBuildid"), rBuildID, rtl::OUString());
    aVersionFile.getFrom(UNISTRING("UpdateID"), rInstallSetID, rtl::OUString());

    rtl::OUString aValue;
    aVersionFile.getFrom(UNISTRING("UpdateURL"), aValue, rtl::OUString());
    if( aValue.getLength() > 0 )
    {
        rRepositoryList.realloc(1);
        rRepositoryList[0] = aValue;
    }

    return true;
}

//------------------------------------------------------------------------------

// Returns 'true' if successfully connected to the update server
bool
checkForUpdates(
    uno::Reference< uno::XComponentContext > const & rxContext,
    uno::Reference< task::XInteractionHandler > const & rxInteractionHandler,
    rtl::OUString& rUpdateURL, rtl::OUString& rVersionFound,
    uno::Reference< deployment::XUpdateInformationProvider >& rUpdateInfoProvider)
{
    OSL_TRACE("checking for updates ..\n");

    ::rtl::OUString myArch;
    ::rtl::OUString myOS;

    rtl::Bootstrap::get(UNISTRING("_OS"), myOS);
    rtl::Bootstrap::get(UNISTRING("_ARCH"), myArch);

    uno::Sequence< ::rtl::OUString > aRepositoryList;
    ::rtl::OUString aBuildID;
    ::rtl::OUString aInstallSetID;

    if( ! ( getBootstrapData(aRepositoryList, aBuildID, aInstallSetID) && (aRepositoryList.getLength() > 0) ) )
        return false;

    if( !rxContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "checkForUpdates: empty component context" ), uno::Reference< uno::XInterface >() );

    OSL_ASSERT( rxContext->getServiceManager().is() );

    // the update information provider
    rUpdateInfoProvider =  deployment::UpdateInformationProvider::create( rxContext );

    // XPath implementation
    uno::Reference< xml::xpath::XXPathAPI > xXPath(
        rxContext->getServiceManager()->createInstanceWithContext( UNISTRING( "com.sun.star.xml.xpath.XPathAPI" ), rxContext ),
        uno::UNO_QUERY_THROW);

    xXPath->registerNS( UNISTRING("inst"), UNISTRING("http://installation.openoffice.org/description") );

    if( rxInteractionHandler.is() )
        rUpdateInfoProvider->setInteractionHandler(rxInteractionHandler);

    try
    {
        uno::Sequence< uno::Reference< xml::dom::XElement > > aUpdateInfoList =
            rUpdateInfoProvider->getUpdateInformation( aRepositoryList, aInstallSetID );

        rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii("/child::inst:description[inst:os=\'");
        aBuffer.append( myOS );
        aBuffer.appendAscii("\' and inst:arch=\'");
        aBuffer.append( myArch );
        aBuffer.appendAscii("\' and inst:buildid>");
        aBuffer.append( aBuildID );
        aBuffer.appendAscii("]");

        rtl::OUString aXPathExpression = aBuffer.makeStringAndClear();

        sal_Int32 nmax = aUpdateInfoList.getLength();
        for( sal_Int32 n = 0; n < nmax; ++n )
        {
            uno::Reference< xml::dom::XNode > xNode( aUpdateInfoList[n].get() );
            uno::Reference< xml::dom::XNodeList > xNodeList =
                xXPath->selectNodeList(xNode, aXPathExpression + UNISTRING("/inst:update/attribute::src"));

            sal_Int32 imax = xNodeList->getLength();
            for( sal_Int32 i = 0; i < imax; ++i )
            {
                uno::Reference< xml::dom::XNode > xNode2( xNodeList->item(i) );

                if( xNode2.is() && rUpdateURL.getLength() == 0 )
                    rUpdateURL = xNode2->getNodeValue();
            }

            uno::Reference< xml::dom::XNode > xNode2 =
                xXPath->selectSingleNode(xNode, aXPathExpression + UNISTRING("/inst:buildid/text()"));

            if( xNode2.is() )
                rVersionFound = xNode2->getNodeValue();

            if( rUpdateURL.getLength() > 0 )
                return true;
        }
    }
    catch( ... )
    {
        return false;
    }

    return true;
}
