/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_folders.h>

#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>

#include "updateprotocol.hxx"
#include "updatecheckconfig.hxx"

#include <com/sun/star/deployment/UpdateInformationEntry.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>


#include <rtl/ref.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/diagnose.h>

namespace container = css::container ;
namespace deployment = css::deployment ;
namespace uno = css::uno ;
namespace task = css::task ;
namespace xml = css::xml ;


static bool
getBootstrapData(
    uno::Sequence< OUString > & rRepositoryList,
    OUString & rGitID,
    OUString & rInstallSetID)
{
    rGitID = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}";
    rtl::Bootstrap::expandMacros( rGitID );
    if ( rGitID.isEmpty() )
        return false;

    rInstallSetID = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":UpdateID}";
    rtl::Bootstrap::expandMacros( rInstallSetID );
    if ( rInstallSetID.isEmpty() )
        return false;

    OUString aValue( u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":UpdateURL}"_ustr );
    rtl::Bootstrap::expandMacros( aValue );

    if( !aValue.isEmpty() )
    {
        rRepositoryList = { aValue };
    }

    return true;
}


// Returns 'true' if successfully connected to the update server
bool
checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    uno::Reference< uno::XComponentContext > const & rxContext,
    uno::Reference< task::XInteractionHandler > const & rxInteractionHandler,
    const uno::Reference< deployment::XUpdateInformationProvider >& rUpdateInfoProvider)
{
    OUString myArch;
    OUString myOS;

    rtl::Bootstrap::get(u"_OS"_ustr, myOS);
    rtl::Bootstrap::get(u"_ARCH"_ustr, myArch);

    uno::Sequence< OUString > aRepositoryList;
    OUString aGitID;
    OUString aInstallSetID;

    if( ! ( getBootstrapData(aRepositoryList, aGitID, aInstallSetID) && (aRepositoryList.getLength() > 0) ) )
        return false;

    return checkForUpdates( o_rUpdateInfo, rxContext, rxInteractionHandler, rUpdateInfoProvider,
            myOS, myArch,
            aRepositoryList, aGitID, aInstallSetID );
}

bool
checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    const uno::Reference< uno::XComponentContext > & rxContext,
    const uno::Reference< task::XInteractionHandler > & rxInteractionHandler,
    const uno::Reference< deployment::XUpdateInformationProvider >& rUpdateInfoProvider,
    std::u16string_view rOS,
    std::u16string_view rArch,
    const uno::Sequence< OUString > &rRepositoryList,
    std::u16string_view rGitID,
    const OUString &rInstallSetID )
{
    if( !rxContext.is() )
        throw uno::RuntimeException( u"checkForUpdates: empty component context"_ustr );

    OSL_ASSERT( rxContext->getServiceManager().is() );

    // XPath implementation
    uno::Reference< xml::xpath::XXPathAPI > xXPath = xml::xpath::XPathAPI::create(rxContext);

    xXPath->registerNS( u"inst"_ustr, u"http://update.libreoffice.org/description"_ustr );

    if( rxInteractionHandler.is() )
        rUpdateInfoProvider->setInteractionHandler(rxInteractionHandler);

    try
    {
        uno::Reference< container::XEnumeration > aUpdateInfoEnumeration =
            rUpdateInfoProvider->getUpdateInformationEnumeration( rRepositoryList, rInstallSetID );

        if ( !aUpdateInfoEnumeration.is() )
            return false; // something went wrong ..

        OUString aXPathExpression =
            OUString::Concat("/child::inst:description[inst:os=\'")+
             rOS +
            "\' and inst:arch=\'"+
             rArch +
            "\' and inst:gitid!=\'"+
             rGitID +
            "\']";


        while( aUpdateInfoEnumeration->hasMoreElements() )
        {
            deployment::UpdateInformationEntry aEntry;

            if( aUpdateInfoEnumeration->nextElement() >>= aEntry )
            {
                uno::Reference< xml::dom::XNode > xNode( aEntry.UpdateDocument );
                uno::Reference< xml::dom::XNodeList > xNodeList;
                try {
                    xNodeList = xXPath->selectNodeList(xNode, aXPathExpression
                        + "/inst:update/attribute::src");
                } catch (const css::xml::xpath::XPathException &) {
                    // ignore
                }

                sal_Int32 i, imax = xNodeList->getLength();
                for( i = 0; i < imax; ++i )
                {
                    uno::Reference< xml::dom::XNode > xNode2( xNodeList->item(i) );

                    if( xNode2.is() )
                    {
                        uno::Reference< xml::dom::XElement > xParent(xNode2->getParentNode(), uno::UNO_QUERY_THROW);
                        OUString aType = xParent->getAttribute(u"type"_ustr);
                        bool bIsDirect = !aType.equalsIgnoreAsciiCase("text/html");

                        o_rUpdateInfo.Sources.emplace_back(bIsDirect, xNode2->getNodeValue());
                    }
                }

                uno::Reference< xml::dom::XNode > xNode2;
                try {
                    xNode2 = xXPath->selectSingleNode(xNode, aXPathExpression
                        + "/inst:version/text()");
                } catch (const css::xml::xpath::XPathException &) {
                    // ignore
                }

                if( xNode2.is() )
                    o_rUpdateInfo.Version = xNode2->getNodeValue();

                try {
                    xNode2 = xXPath->selectSingleNode(xNode, aXPathExpression
                        + "/inst:buildid/text()");
                } catch (const css::xml::xpath::XPathException &) {
                    // ignore
                }

                if( xNode2.is() )
                    o_rUpdateInfo.BuildId = xNode2->getNodeValue();

                o_rUpdateInfo.Description = aEntry.Description;

                // Release Notes
                try {
                    xNodeList = xXPath->selectNodeList(xNode, aXPathExpression
                        + "/inst:relnote");
                } catch (const css::xml::xpath::XPathException &) {
                    // ignore
                }
                imax = xNodeList->getLength();
                for( i = 0; i < imax; ++i )
                {
                    uno::Reference< xml::dom::XElement > xRelNote(xNodeList->item(i), uno::UNO_QUERY);
                    if( xRelNote.is() )
                    {
                        sal_Int32 pos = xRelNote->getAttribute(u"pos"_ustr).toInt32();

                        ReleaseNote aRelNote(static_cast<sal_uInt8>(pos), xRelNote->getAttribute(u"src"_ustr));

                        if( xRelNote->hasAttribute(u"src2"_ustr) )
                        {
                            pos = xRelNote->getAttribute(u"pos2"_ustr).toInt32();
                            aRelNote.Pos2 = static_cast<sal_Int8>(pos);
                            aRelNote.URL2 = xRelNote->getAttribute(u"src2"_ustr);
                        }

                        o_rUpdateInfo.ReleaseNotes.push_back(aRelNote);
                    }
                }

                if( !o_rUpdateInfo.Sources.empty() )
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


bool storeExtensionUpdateInfos( const uno::Reference< uno::XComponentContext > & rxContext,
                                const uno::Sequence< uno::Sequence< OUString > > &rUpdateInfos )
{
    bool bNotify = false;

    if ( rUpdateInfos.hasElements() )
    {
        rtl::Reference< UpdateCheckConfig > aConfig = UpdateCheckConfig::get( rxContext );

        for ( sal_Int32 i = rUpdateInfos.getLength() - 1; i >= 0; i-- )
        {
            bNotify |= aConfig->storeExtensionVersion( rUpdateInfos[i][0], rUpdateInfos[i][1] );
        }
    }
    return bNotify;
}


// Returns 'true' if there are updates for any extension

bool checkForExtensionUpdates( const uno::Reference< uno::XComponentContext > & rxContext )
{
    uno::Sequence< uno::Sequence< OUString > > aUpdateList;

    uno::Reference< deployment::XPackageInformationProvider > xInfoProvider;
    try
    {
        uno::Any aValue( rxContext->getValueByName(
                u"/singletons/com.sun.star.deployment.PackageInformationProvider"_ustr ) );
        OSL_VERIFY( aValue >>= xInfoProvider );
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "checkForExtensionUpdates: could not create the PackageInformationProvider!" );
    }

    if ( !xInfoProvider.is() ) return false;

    aUpdateList = xInfoProvider->isUpdateAvailable( OUString() );
    bool bNotify = storeExtensionUpdateInfos( rxContext, aUpdateList );

    return bNotify;
}


// Returns 'true' if there are any pending updates for any extension (offline check)

bool checkForPendingUpdates( const uno::Reference< uno::XComponentContext > & rxContext )
{
    uno::Sequence< uno::Sequence< OUString > > aExtensionList;
    uno::Reference< deployment::XPackageInformationProvider > xInfoProvider;
    try
    {
        uno::Any aValue( rxContext->getValueByName(
                u"/singletons/com.sun.star.deployment.PackageInformationProvider"_ustr ) );
        OSL_VERIFY( aValue >>= xInfoProvider );
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "checkForExtensionUpdates: could not create the PackageInformationProvider!" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
