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

#include "dp_descriptioninfoset.hxx"

#include "dp_resource.h"
#include <sal/config.h>

#include <comphelper/sequence.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <boost/optional.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <ucbhelper/content.hxx>

namespace {

using css::uno::Reference;

class EmptyNodeList:
    public cppu::WeakImplHelper<css::xml::dom::XNodeList>
{
public:
    EmptyNodeList();

    EmptyNodeList(const EmptyNodeList&) = delete;
    const EmptyNodeList& operator=(const EmptyNodeList&) = delete;

    virtual ::sal_Int32 SAL_CALL getLength() override;

    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
    item(::sal_Int32 index) override;
};

EmptyNodeList::EmptyNodeList() {}

::sal_Int32 EmptyNodeList::getLength() {
    return 0;
}

css::uno::Reference< css::xml::dom::XNode > EmptyNodeList::item(::sal_Int32)
{
    throw css::uno::RuntimeException("bad EmptyNodeList com.sun.star.xml.dom.XNodeList.item call",
        static_cast< ::cppu::OWeakObject * >(this));
}

OUString getNodeValue(
    css::uno::Reference< css::xml::dom::XNode > const & node)
{
    OSL_ASSERT(node.is());
    try {
        return node->getNodeValue();
    } catch (const css::xml::dom::DOMException & e) {
        throw css::uno::RuntimeException(
            "com.sun.star.xml.dom.DOMException: " + e.Message);
    }
}

/**The class uses the UCB to access the description.xml file in an
   extension. The UCB must have been initialized already. It also
   requires that the extension has already be unzipped to a particular
   location.
 */
class ExtensionDescription
{
public:
    /**throws an exception if the description.xml is not
        available, cannot be read, does not contain the expected data,
        or any other error occurred. Therefore it should only be used with
        new extensions.

        Throws css::uno::RuntimeException,
        css::deployment::DeploymentException,
        dp_registry::backend::bundle::NoDescriptionException.
     */
    ExtensionDescription(
        const css::uno::Reference<css::uno::XComponentContext>& xContext,
        const OUString& installDir,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv);

    const css::uno::Reference<css::xml::dom::XNode>& getRootElement() const
    {
        return m_xRoot;
    }

private:
    css::uno::Reference<css::xml::dom::XNode> m_xRoot;
};

class NoDescriptionException
{
};

class FileDoesNotExistFilter
    : public ::cppu::WeakImplHelper< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler >

{
    bool m_bExist;
    css::uno::Reference< css::ucb::XCommandEnvironment > m_xCommandEnv;

public:
    explicit FileDoesNotExistFilter(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv);

    bool exist() { return m_bExist;}
    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() override;
    virtual css::uno::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() override;

    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest ) override;
};

ExtensionDescription::ExtensionDescription(
    const Reference<css::uno::XComponentContext>& xContext,
    const OUString& installDir,
    const Reference< css::ucb::XCommandEnvironment >& xCmdEnv)
{
    try {
        //may throw css::ucb::ContentCreationException
        //If there is no description.xml then ucb will start an interaction which
        //brings up a dialog.We want to prevent this. Therefore we wrap the xCmdEnv
        //and filter the respective exception out.
        OUString sDescriptionUri(installDir + "/description.xml");
        Reference<css::ucb::XCommandEnvironment> xFilter =
            static_cast<css::ucb::XCommandEnvironment*>(
                new FileDoesNotExistFilter(xCmdEnv));
        ::ucbhelper::Content descContent(sDescriptionUri, xFilter, xContext);

        //throws an css::uno::Exception if the file is not available
        Reference<css::io::XInputStream> xIn;
        try
        {   //throws com.sun.star.ucb.InteractiveIOException
            xIn = descContent.openStream();
        }
        catch ( const css::uno::Exception& )
        {
            if ( ! static_cast<FileDoesNotExistFilter*>(xFilter.get())->exist())
                throw NoDescriptionException();
            throw;
        }
        if (!xIn.is())
        {
            throw css::uno::Exception(
                "Could not get XInputStream for description.xml of extension " +
                sDescriptionUri, nullptr);
        }

        //get root node of description.xml
        Reference<css::xml::dom::XDocumentBuilder> xDocBuilder(
            css::xml::dom::DocumentBuilder::create(xContext) );

        if (!xDocBuilder->isNamespaceAware())
        {
            throw css::uno::Exception(
                "Service com.sun.star.xml.dom.DocumentBuilder is not namespace aware.", nullptr);
        }

        Reference<css::xml::dom::XDocument> xDoc = xDocBuilder->parse(xIn);
        if (!xDoc.is())
        {
            throw css::uno::Exception(sDescriptionUri + " contains data which cannot be parsed. ", nullptr);
        }

        //check for proper root element and namespace
        Reference<css::xml::dom::XElement> xRoot = xDoc->getDocumentElement();
        if (!xRoot.is())
        {
            throw css::uno::Exception(
                sDescriptionUri + " contains no root element.", nullptr);
        }

        if ( ! (xRoot->getTagName() == "description"))
        {
            throw css::uno::Exception(
                sDescriptionUri + " does not contain the root element <description>.", nullptr);
        }

        m_xRoot.set(xRoot, css::uno::UNO_QUERY_THROW);
        OUString nsDescription = xRoot->getNamespaceURI();

        //check if this namespace is supported
        if ( ! (nsDescription == "http://openoffice.org/extensions/description/2006"))
        {
            throw css::uno::Exception(sDescriptionUri + " contains a root element with an unsupported namespace. ", nullptr);
        }
    } catch (const css::uno::RuntimeException &) {
        throw;
    } catch (const css::deployment::DeploymentException &) {
        throw;
    } catch (const css::uno::Exception & e) {
        css::uno::Any a(cppu::getCaughtException());
        throw css::deployment::DeploymentException(
            e.Message, Reference< css::uno::XInterface >(), a);
    }
}

FileDoesNotExistFilter::FileDoesNotExistFilter(
    const Reference< css::ucb::XCommandEnvironment >& xCmdEnv):
    m_bExist(true), m_xCommandEnv(xCmdEnv)
{}

    // XCommandEnvironment
Reference<css::task::XInteractionHandler >
    FileDoesNotExistFilter::getInteractionHandler()
{
    return static_cast<css::task::XInteractionHandler*>(this);
}

Reference<css::ucb::XProgressHandler >
    FileDoesNotExistFilter::getProgressHandler()
{
    return m_xCommandEnv.is()
        ? m_xCommandEnv->getProgressHandler()
        : Reference<css::ucb::XProgressHandler>();
}

// XInteractionHandler
//If the interaction was caused by a non-existing file which is specified in the ctor
//of FileDoesNotExistFilter, then we do nothing
void  FileDoesNotExistFilter::handle(
        Reference<css::task::XInteractionRequest > const & xRequest )
{
    css::uno::Any request( xRequest->getRequest() );

    css::ucb::InteractiveIOException ioexc;
    if ((request>>= ioexc)
        && (ioexc.Code == css::ucb::IOErrorCode_NOT_EXISTING
            || ioexc.Code == css::ucb::IOErrorCode_NOT_EXISTING_PATH))
    {
        m_bExist = false;
        return;
    }
    Reference<css::task::XInteractionHandler> xInteraction;
    if (m_xCommandEnv.is()) {
        xInteraction = m_xCommandEnv->getInteractionHandler();
    }
    if (xInteraction.is()) {
        xInteraction->handle(xRequest);
    }
}

}

namespace dp_misc {

DescriptionInfoset getDescriptionInfoset(OUString const & sExtensionFolderURL)
{
    Reference< css::xml::dom::XNode > root;
    Reference<css::uno::XComponentContext> context(
        comphelper::getProcessComponentContext());
    try {
        root =
            ExtensionDescription(
                context, sExtensionFolderURL,
                Reference< css::ucb::XCommandEnvironment >()).
            getRootElement();
    } catch (const NoDescriptionException &) {
    } catch (const css::deployment::DeploymentException & e) {
        throw css::uno::RuntimeException(
             "com.sun.star.deployment.DeploymentException: " + e.Message, nullptr);
    }
    return DescriptionInfoset(context, root);
}

DescriptionInfoset::DescriptionInfoset(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    css::uno::Reference< css::xml::dom::XNode > const & element):
    m_context(context),
    m_element(element)
{
    if (m_element.is()) {
        m_xpath = css::xml::xpath::XPathAPI::create(context);
        m_xpath->registerNS("desc", element->getNamespaceURI());
        m_xpath->registerNS("xlink", "http://www.w3.org/1999/xlink");
    }
}

DescriptionInfoset::~DescriptionInfoset() {}

::boost::optional< OUString > DescriptionInfoset::getIdentifier() const {
    return getOptionalValue("desc:identifier/@value");
}

OUString DescriptionInfoset::getNodeValueFromExpression(OUString const & expression) const
{
    css::uno::Reference< css::xml::dom::XNode > n;
    if (m_element.is()) {
        try {
            n = m_xpath->selectSingleNode(m_element, expression);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
    }
    return n.is() ? getNodeValue(n) : OUString();
}

void DescriptionInfoset::checkBlacklist() const
{
    if (m_element.is()) {
        boost::optional< OUString > id(getIdentifier());
        if (!id)
            return; // nothing to check
        OUString currentversion(getVersion());
        if (currentversion.getLength() == 0)
            return;  // nothing to check

        css::uno::Sequence< css::uno::Any > args = css::uno::Sequence< css::uno::Any >(1);
        css::beans::PropertyValue prop;
        prop.Name = "nodepath";
        prop.Value <<= OUString("/org.openoffice.Office.ExtensionDependencies/Extensions");
        args[0] <<= prop;

        css::uno::Reference< css::container::XNameAccess > blacklist(
            (css::configuration::theDefaultProvider::get(m_context)
             ->createInstanceWithArguments(
                 "com.sun.star.configuration.ConfigurationAccess", args)),
            css::uno::UNO_QUERY_THROW);

        // check first if a blacklist entry is available
        if (blacklist.is() && blacklist->hasByName(*id)) {
            css::uno::Reference< css::beans::XPropertySet > extProps(
                blacklist->getByName(*id), css::uno::UNO_QUERY_THROW);

            css::uno::Any anyValue = extProps->getPropertyValue("Versions");

            css::uno::Sequence< OUString > blversions;
            anyValue >>= blversions;

            // check if the current version requires further dependency checks from the blacklist
            if (checkBlacklistVersion(currentversion, blversions)) {
                anyValue = extProps->getPropertyValue("Dependencies");
                OUString udeps;
                anyValue >>= udeps;

                if (udeps.getLength() == 0)
                    return; // nothing todo

                OString xmlDependencies = OUStringToOString(udeps, RTL_TEXTENCODING_UNICODE);

                css::uno::Reference< css::xml::dom::XDocumentBuilder> docbuilder(
                    m_context->getServiceManager()->createInstanceWithContext("com.sun.star.xml.dom.DocumentBuilder", m_context),
                    css::uno::UNO_QUERY_THROW);

                css::uno::Sequence< sal_Int8 > byteSeq(reinterpret_cast<const sal_Int8*>(xmlDependencies.getStr()), xmlDependencies.getLength());

                css::uno::Reference< css::io::XInputStream> inputstream( css::io::SequenceInputStream::createStreamFromSequence(m_context, byteSeq),
                                                                         css::uno::UNO_QUERY_THROW);

                css::uno::Reference< css::xml::dom::XDocument > xDocument(docbuilder->parse(inputstream));
                css::uno::Reference< css::xml::dom::XElement > xElement(xDocument->getDocumentElement());
                css::uno::Reference< css::xml::dom::XNodeList > xDeps(xElement->getChildNodes());
                sal_Int32 nLen = xDeps->getLength();

                // get the parent xml document  of current description info for the import
                css::uno::Reference< css::xml::dom::XDocument > xCurrentDescInfo(m_element->getOwnerDocument());

                // get dependency node of current description info to merge the new dependencies from the blacklist
                css::uno::Reference< css::xml::dom::XNode > xCurrentDeps(
                    m_xpath->selectSingleNode(m_element, "desc:dependencies"));

                // if no dependency node exists, create a new one in the current description info
                if (!xCurrentDeps.is()) {
                    css::uno::Reference< css::xml::dom::XNode > xNewDepNode(
                        xCurrentDescInfo->createElementNS(
                            "http://openoffice.org/extensions/description/2006",
                            "dependencies"), css::uno::UNO_QUERY_THROW);
                    m_element->appendChild(xNewDepNode);
                    xCurrentDeps = m_xpath->selectSingleNode(m_element, "desc:dependencies");
                }

                for (sal_Int32 i=0; i<nLen; i++) {
                    css::uno::Reference< css::xml::dom::XNode > xNode(xDeps->item(i));
                    css::uno::Reference< css::xml::dom::XElement > xDep(xNode, css::uno::UNO_QUERY);
                    if (xDep.is()) {
                        // found valid blacklist dependency, import the node first and append it to the existing dependency node
                        css::uno::Reference< css::xml::dom::XNode > importedNode = xCurrentDescInfo->importNode(xNode, true);
                        xCurrentDeps->appendChild(importedNode);
                    }
                }
            }
        }
    }
}

bool DescriptionInfoset::checkBlacklistVersion(
    const OUString& currentversion,
    css::uno::Sequence< OUString > const & versions)
{
    sal_Int32 nLen = versions.getLength();
    for (sal_Int32 i=0; i<nLen; i++) {
        if (currentversion == versions[i])
            return true;
    }

    return false;
}

OUString DescriptionInfoset::getVersion() const
{
    return getNodeValueFromExpression( "desc:version/@value" );
}

css::uno::Sequence< OUString > DescriptionInfoset::getSupportedPlatforms() const
{
    //When there is no description.xml then we assume that we support all platforms
    if (! m_element.is())
    {
        return { OUString("all") };
    }

    //Check if the <platform> element was provided. If not the default is "all" platforms
    css::uno::Reference< css::xml::dom::XNode > nodePlatform(
        m_xpath->selectSingleNode(m_element, "desc:platform"));
    if (!nodePlatform.is())
    {
        return { OUString("all") };
    }

    //There is a platform element.
    const OUString value = getNodeValueFromExpression("desc:platform/@value");
    //parse the string, it can contained multiple strings separated by commas
    ::std::vector< OUString> vec;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = value.getToken( 0, ',', nIndex );
        aToken = aToken.trim();
        if (!aToken.isEmpty())
            vec.push_back(aToken);

    }
    while (nIndex >= 0);

    return comphelper::containerToSequence(vec);
}

css::uno::Reference< css::xml::dom::XNodeList >
DescriptionInfoset::getDependencies() const {
    if (m_element.is()) {
        try {
            // check the extension blacklist first and expand the dependencies if applicable
            checkBlacklist();

            return m_xpath->selectNodeList(m_element, "desc:dependencies/*");
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
    }
    return new EmptyNodeList;
}

css::uno::Sequence< OUString >
DescriptionInfoset::getUpdateInformationUrls() const {
    return getUrls("desc:update-information/desc:src/@xlink:href");
}

css::uno::Sequence< OUString >
DescriptionInfoset::getUpdateDownloadUrls() const
{
    return getUrls("desc:update-download/desc:src/@xlink:href");
}

OUString DescriptionInfoset::getIconURL( bool bHighContrast ) const
{
    css::uno::Sequence< OUString > aStrList = getUrls( "desc:icon/desc:default/@xlink:href" );
    css::uno::Sequence< OUString > aStrListHC = getUrls( "desc:icon/desc:high-contrast/@xlink:href" );

    if ( bHighContrast && aStrListHC.hasElements() && !aStrListHC[0].isEmpty() )
        return aStrListHC[0];

    if ( aStrList.hasElements() && !aStrList[0].isEmpty() )
        return aStrList[0];

    return OUString();
}

::boost::optional< OUString > DescriptionInfoset::getLocalizedUpdateWebsiteURL()
    const
{
    bool bParentExists = false;
    const OUString sURL (getLocalizedHREFAttrFromChild("/desc:description/desc:update-website", &bParentExists ));

    if (!sURL.isEmpty())
        return ::boost::optional< OUString >(sURL);
    else
        return bParentExists ? ::boost::optional< OUString >(OUString()) :
            ::boost::optional< OUString >();
}

::boost::optional< OUString > DescriptionInfoset::getOptionalValue(
    OUString const & expression) const
{
    css::uno::Reference< css::xml::dom::XNode > n;
    if (m_element.is()) {
        try {
            n = m_xpath->selectSingleNode(m_element, expression);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
    }
    return n.is()
        ? ::boost::optional< OUString >(getNodeValue(n))
        : ::boost::optional< OUString >();
}

css::uno::Sequence< OUString > DescriptionInfoset::getUrls(
    OUString const & expression) const
{
    css::uno::Reference< css::xml::dom::XNodeList > ns;
    if (m_element.is()) {
        try {
            ns = m_xpath->selectNodeList(m_element, expression);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
    }
    css::uno::Sequence< OUString > urls(ns.is() ? ns->getLength() : 0);
    for (::sal_Int32 i = 0; i < urls.getLength(); ++i) {
        urls[i] = getNodeValue(ns->item(i));
    }
    return urls;
}

::std::pair< OUString, OUString > DescriptionInfoset::getLocalizedPublisherNameAndURL() const
{
    css::uno::Reference< css::xml::dom::XNode > node =
        getLocalizedChild("desc:publisher");

    OUString sPublisherName;
    OUString sURL;
    if (node.is())
    {
        const OUString exp1("text()");
        css::uno::Reference< css::xml::dom::XNode > xPathName;
        try {
            xPathName = m_xpath->selectSingleNode(node, exp1);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
        OSL_ASSERT(xPathName.is());
        if (xPathName.is())
            sPublisherName = xPathName->getNodeValue();

        const OUString exp2("@xlink:href");
        css::uno::Reference< css::xml::dom::XNode > xURL;
        try {
            xURL = m_xpath->selectSingleNode(node, exp2);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
        OSL_ASSERT(xURL.is());
        if (xURL.is())
           sURL = xURL->getNodeValue();
    }
    return ::std::make_pair(sPublisherName, sURL);
}

OUString DescriptionInfoset::getLocalizedReleaseNotesURL() const
{
    return getLocalizedHREFAttrFromChild("/desc:description/desc:release-notes", nullptr);
}

OUString DescriptionInfoset::getLocalizedDisplayName() const
{
    css::uno::Reference< css::xml::dom::XNode > node =
        getLocalizedChild("desc:display-name");
    if (node.is())
    {
        const OUString exp("text()");
        css::uno::Reference< css::xml::dom::XNode > xtext;
        try {
            xtext = m_xpath->selectSingleNode(node, exp);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
        if (xtext.is())
            return xtext->getNodeValue();
    }
    return OUString();
}

OUString DescriptionInfoset::getLocalizedLicenseURL() const
{
    return getLocalizedHREFAttrFromChild("/desc:description/desc:registration/desc:simple-license", nullptr);

}

::boost::optional<SimpleLicenseAttributes>
DescriptionInfoset::getSimpleLicenseAttributes() const
{
    //Check if the node exist
    css::uno::Reference< css::xml::dom::XNode > n;
    if (m_element.is()) {
        try {
            n = m_xpath->selectSingleNode(m_element, "/desc:description/desc:registration/desc:simple-license/@accept-by");
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
        if (n.is())
        {
            SimpleLicenseAttributes attributes;
            attributes.acceptBy =
                getNodeValueFromExpression("/desc:description/desc:registration/desc:simple-license/@accept-by");

            ::boost::optional< OUString > suppressOnUpdate = getOptionalValue("/desc:description/desc:registration/desc:simple-license/@suppress-on-update");
            if (suppressOnUpdate)
                attributes.suppressOnUpdate = (*suppressOnUpdate).trim().equalsIgnoreAsciiCase("true");
            else
                attributes.suppressOnUpdate = false;

            ::boost::optional< OUString > suppressIfRequired = getOptionalValue("/desc:description/desc:registration/desc:simple-license/@suppress-if-required");
            if (suppressIfRequired)
                attributes.suppressIfRequired = (*suppressIfRequired).trim().equalsIgnoreAsciiCase("true");
            else
                attributes.suppressIfRequired = false;

            return ::boost::optional<SimpleLicenseAttributes>(attributes);
        }
    }
    return ::boost::optional<SimpleLicenseAttributes>();
}

OUString DescriptionInfoset::getLocalizedDescriptionURL() const
{
    return getLocalizedHREFAttrFromChild("/desc:description/desc:extension-description", nullptr);
}

css::uno::Reference< css::xml::dom::XNode >
DescriptionInfoset::getLocalizedChild( const OUString & sParent) const
{
    if ( ! m_element.is() || sParent.isEmpty())
        return css::uno::Reference< css::xml::dom::XNode > ();

    css::uno::Reference< css::xml::dom::XNode > xParent;
    try {
        xParent = m_xpath->selectSingleNode(m_element, sParent);
    } catch (const css::xml::xpath::XPathException &) {
        // ignore
    }
    css::uno::Reference<css::xml::dom::XNode> nodeMatch;
    if (xParent.is())
    {
        nodeMatch = matchLanguageTag(xParent, getOfficeLanguageTag().getBcp47());

        //office: en-DE, en, en-DE-altmark
        if (! nodeMatch.is())
        {
            // Already tried full tag, continue with first fallback.
            const ::std::vector< OUString > aFallbacks( getOfficeLanguageTag().getFallbackStrings( false));
            for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
            {
                nodeMatch = matchLanguageTag(xParent, *it);
                if (nodeMatch.is())
                    break;
            }
            if (! nodeMatch.is())
                nodeMatch = getChildWithDefaultLocale(xParent);
        }
    }

    return nodeMatch;
}

css::uno::Reference<css::xml::dom::XNode>
DescriptionInfoset::matchLanguageTag(
    css::uno::Reference< css::xml::dom::XNode > const & xParent, OUString const & rTag) const
{
    OSL_ASSERT(xParent.is());
    css::uno::Reference<css::xml::dom::XNode> nodeMatch;

    //first try exact match for lang
    const OUString exp1("*[@lang=\"" + rTag + "\"]");
    try {
        nodeMatch = m_xpath->selectSingleNode(xParent, exp1);
    } catch (const css::xml::xpath::XPathException &) {
        // ignore
    }

    //try to match in strings that also have a country and/or variant, for
    //example en  matches in en-US-montana, en-US, en-montana
    if (!nodeMatch.is())
    {
        const OUString exp2(
            "*[starts-with(@lang,\"" + rTag + "-\")]");
        try {
            nodeMatch = m_xpath->selectSingleNode(xParent, exp2);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
    }
    return nodeMatch;
}

css::uno::Reference<css::xml::dom::XNode>
DescriptionInfoset::getChildWithDefaultLocale(css::uno::Reference< css::xml::dom::XNode >
                                    const & xParent) const
{
    OSL_ASSERT(xParent.is());
    if ( xParent->getNodeName() == "simple-license" )
    {
        css::uno::Reference<css::xml::dom::XNode> nodeDefault;
        try {
            nodeDefault = m_xpath->selectSingleNode(xParent, "@default-license-id");
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
        if (nodeDefault.is())
        {
            //The old way
            const OUString exp1("desc:license-text[@license-id = \""
                + nodeDefault->getNodeValue()
                + "\"]");
            try {
                return m_xpath->selectSingleNode(xParent, exp1);
            } catch (const css::xml::xpath::XPathException &) {
                // ignore
            }
        }
    }

    const OUString exp2("*[1]");
    try {
        return m_xpath->selectSingleNode(xParent, exp2);
    } catch (const css::xml::xpath::XPathException &) {
        // ignore
        return nullptr;
    }
}

OUString DescriptionInfoset::getLocalizedHREFAttrFromChild(
    OUString const & sXPathParent, bool * out_bParentExists)
    const
{
    css::uno::Reference< css::xml::dom::XNode > node =
        getLocalizedChild(sXPathParent);

    OUString sURL;
    if (node.is())
    {
        if (out_bParentExists)
            *out_bParentExists = true;
        const OUString exp("@xlink:href");
        css::uno::Reference< css::xml::dom::XNode > xURL;
        try {
            xURL = m_xpath->selectSingleNode(node, exp);
        } catch (const css::xml::xpath::XPathException &) {
            // ignore
        }
        OSL_ASSERT(xURL.is());
        if (xURL.is())
            sURL = xURL->getNodeValue();
    }
    else
    {
        if (out_bParentExists)
            *out_bParentExists = false;
    }
    return sURL;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
