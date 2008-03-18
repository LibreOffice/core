/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateprotocol.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:17:30 $
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
#include "updatecheckconfig.hxx"

#ifndef _COM_SUN_STAR_DEPLOYMENT_UPDATEINFORMATINENTRY_HPP_
#include <com/sun/star/deployment/UpdateInformationEntry.hpp>
#endif
#ifndef  _COM_SUN_STAR_DEPLOYMENT_XPACKAGEINFORMATIONPROVIDER_HPP_
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#endif


#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>

#include <cppuhelper/implbase1.hxx>

namespace css = com::sun::star ;
namespace container = css::container ;
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
    if( !rtl::Bootstrap::get(UNISTRING("BRAND_BASE_DIR"), aPath) )
        return false;

    aPath += UNISTRING( "/program/" SAL_CONFIGFILE( "version" ) );

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
    UpdateInfo& o_rUpdateInfo,
    uno::Reference< uno::XComponentContext > const & rxContext,
    uno::Reference< task::XInteractionHandler > const & rxInteractionHandler,
    const uno::Reference< deployment::XUpdateInformationProvider >& rUpdateInfoProvider)
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

    // XPath implementation
    uno::Reference< xml::xpath::XXPathAPI > xXPath(
        rxContext->getServiceManager()->createInstanceWithContext( UNISTRING( "com.sun.star.xml.xpath.XPathAPI" ), rxContext ),
        uno::UNO_QUERY_THROW);

    xXPath->registerNS( UNISTRING("inst"), UNISTRING("http://installation.openoffice.org/description") );

    if( rxInteractionHandler.is() )
        rUpdateInfoProvider->setInteractionHandler(rxInteractionHandler);

    try
    {
        uno::Reference< container::XEnumeration > aUpdateInfoEnumeration =
            rUpdateInfoProvider->getUpdateInformationEnumeration( aRepositoryList, aInstallSetID );

        if ( !aUpdateInfoEnumeration.is() )
            return false; // something went wrong ..

        rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii("/child::inst:description[inst:os=\'");
        aBuffer.append( myOS );
        aBuffer.appendAscii("\' and inst:arch=\'");
        aBuffer.append( myArch );
        aBuffer.appendAscii("\' and inst:buildid>");
        aBuffer.append( aBuildID );
        aBuffer.appendAscii("]");

        rtl::OUString aXPathExpression = aBuffer.makeStringAndClear();

        while( aUpdateInfoEnumeration->hasMoreElements() )
        {
            deployment::UpdateInformationEntry aEntry;

            if( aUpdateInfoEnumeration->nextElement() >>= aEntry )
            {
                uno::Reference< xml::dom::XNode > xNode( aEntry.UpdateDocument.get() );
                uno::Reference< xml::dom::XNodeList > xNodeList =
                    xXPath->selectNodeList(xNode, aXPathExpression + UNISTRING("/inst:update/attribute::src"));

/*
                o_rUpdateInfo.Sources.push_back( DownloadSource(true,
                    UNISTRING("http://openoffice.bouncer.osuosl.org/?product=OpenOffice.org&os=solarissparcwjre&lang=en-US&version=2.2.1") ) );
*/

                sal_Int32 i, imax = xNodeList->getLength();
                for( i = 0; i < imax; ++i )
                {
                    uno::Reference< xml::dom::XNode > xNode2( xNodeList->item(i) );

                    if( xNode2.is() )
                    {
                        uno::Reference< xml::dom::XElement > xParent(xNode2->getParentNode(), uno::UNO_QUERY_THROW);
                        rtl::OUString aType = xParent->getAttribute(UNISTRING("type"));
                        bool bIsDirect = ( sal_False == aType.equalsIgnoreAsciiCaseAscii("text/html") );

                        o_rUpdateInfo.Sources.push_back( DownloadSource(bIsDirect, xNode2->getNodeValue()) );
                    }
                }

                uno::Reference< xml::dom::XNode > xNode2 =
                    xXPath->selectSingleNode(xNode, aXPathExpression + UNISTRING("/inst:version/text()"));

                if( xNode2.is() )
                    o_rUpdateInfo.Version = xNode2->getNodeValue();

                xNode2 = xXPath->selectSingleNode(xNode, aXPathExpression + UNISTRING("/inst:buildid/text()"));

                if( xNode2.is() )
                    o_rUpdateInfo.BuildId = xNode2->getNodeValue();

                o_rUpdateInfo.Description = aEntry.Description;

                // Release Notes
                xNodeList = xXPath->selectNodeList(xNode, aXPathExpression + UNISTRING("/inst:relnote"));
                imax = xNodeList->getLength();
                for( i = 0; i < imax; ++i )
                {
                    uno::Reference< xml::dom::XElement > xRelNote(xNodeList->item(i), uno::UNO_QUERY);
                    if( xRelNote.is() )
                    {
                        sal_Int32 pos = xRelNote->getAttribute(UNISTRING("pos")).toInt32();

                        ReleaseNote aRelNote((sal_uInt8) pos, xRelNote->getAttribute(UNISTRING("src")));

                        if( xRelNote->hasAttribute(UNISTRING("src2")) )
                        {
                            pos = xRelNote->getAttribute(UNISTRING("pos2")).toInt32();
                            aRelNote.Pos2 = (sal_Int8) pos;
                            aRelNote.URL2 = xRelNote->getAttribute(UNISTRING("src2"));
                        }

                        o_rUpdateInfo.ReleaseNotes.push_back(aRelNote);
                    }
                }
/*
                o_rUpdateInfo.ReleaseNotes.push_back(
                    ReleaseNote(1, UNISTRING("http://qa.openoffice.org/tests/online_update_test.html"))
                );
*/

                if( o_rUpdateInfo.Sources.size() > 0 )
                    return true;
            }
        }
    }
    catch( ... )
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
bool storeExtensionUpdateInfos( const uno::Reference< uno::XComponentContext > & rxContext,
                                const uno::Sequence< uno::Sequence< rtl::OUString > > &rUpdateInfos )
{
    if ( rUpdateInfos.hasElements() )
    {
        rtl::Reference< UpdateCheckConfig > aConfig = UpdateCheckConfig::get( rxContext );

        for ( sal_Int32 i = rUpdateInfos.getLength() - 1; i >= 0; i-- )
        {
            aConfig->storeExtensionVersion( rUpdateInfos[i][0], rUpdateInfos[i][1] );
        }
    }
    return rUpdateInfos.hasElements();
}

//------------------------------------------------------------------------------
// Returns 'true' if there are updates for any extension

bool checkForExtensionUpdates( const uno::Reference< uno::XComponentContext > & rxContext )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aUpdateList;

    uno::Reference< deployment::XPackageInformationProvider > xInfoProvider;
    try
    {
        uno::Any aValue( rxContext->getValueByName(
                UNISTRING( "/singletons/com.sun.star.deployment.PackageInformationProvider" ) ) );
        OSL_VERIFY( aValue >>= xInfoProvider );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "checkForExtensionUpdates: could not create the PackageInformationProvider!" );
    }

    if ( !xInfoProvider.is() ) return false;

    aUpdateList = xInfoProvider->isUpdateAvailable( ::rtl::OUString() );
    storeExtensionUpdateInfos( rxContext, aUpdateList );

    return aUpdateList.hasElements();
}

//------------------------------------------------------------------------------
// Returns 'true' if there are any pending updates for any extension (offline check)

bool checkForPendingUpdates( const uno::Reference< uno::XComponentContext > & rxContext )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aExtensionList;
    uno::Reference< deployment::XPackageInformationProvider > xInfoProvider;
    try
    {
        uno::Any aValue( rxContext->getValueByName(
                UNISTRING( "/singletons/com.sun.star.deployment.PackageInformationProvider" ) ) );
        OSL_VERIFY( aValue >>= xInfoProvider );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "checkForExtensionUpdates: could not create the PackageInformationProvider!" );
    }

    if ( !xInfoProvider.is() ) return false;

    bool bPendingUpdateFound = false;

    aExtensionList = xInfoProvider->getExtensionList();
    if ( aExtensionList.hasElements() )
    {
        rtl::Reference< UpdateCheckConfig > aConfig = UpdateCheckConfig::get( rxContext );

        for ( sal_Int32 i = aExtensionList.getLength() - 1; i >= 0; i-- )
        {
            bPendingUpdateFound = aConfig->checkExtensionVersion( aExtensionList[i][0], aExtensionList[i][1] );
            if ( bPendingUpdateFound )
                break;
        }
    }

    return bPendingUpdateFound;
}
