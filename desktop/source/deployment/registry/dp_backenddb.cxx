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


#include <rtl/string.h>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/file.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include "dp_ucb.h"
#include "dp_misc.h"
#include <ucbhelper/content.hxx>
#include <xmlscript/xml_helper.hxx>
#include "dp_backenddb.hxx"


using namespace ::com::sun::star::uno;


namespace dp_registry {
namespace backend {

BackendDb::BackendDb(
    Reference<css::uno::XComponentContext> const &  xContext,
    OUString const & url):
    m_xContext(xContext)
{
    m_urlDb = dp_misc::expandUnoRcUrl(url);
}

void BackendDb::save()
{
    const Reference<css::io::XActiveDataSource> xDataSource(m_doc,css::uno::UNO_QUERY_THROW);
    std::vector<sal_Int8> bytes;
    xDataSource->setOutputStream(::xmlscript::createOutputStream(&bytes));
    const Reference<css::io::XActiveDataControl> xDataControl(m_doc,css::uno::UNO_QUERY_THROW);
    xDataControl->start();

    const Reference<css::io::XInputStream> xData(
        ::xmlscript::createInputStream(bytes));
    ::ucbhelper::Content ucbDb(m_urlDb, nullptr, m_xContext);
    ucbDb.writeStream(xData, true /*replace existing*/);
}

css::uno::Reference<css::xml::dom::XDocument> const & BackendDb::getDocument()
{
    if (!m_doc.is())
    {
        const Reference<css::xml::dom::XDocumentBuilder> xDocBuilder(
            css::xml::dom::DocumentBuilder::create(m_xContext) );

        ::osl::DirectoryItem item;
        ::osl::File::RC err = ::osl::DirectoryItem::get(m_urlDb, item);
        if (err == ::osl::File::E_None)
        {
            ::ucbhelper::Content descContent(
                m_urlDb, css::uno::Reference<css::ucb::XCommandEnvironment>(),
                m_xContext);
            Reference<css::io::XInputStream> xIn = descContent.openStream();
            m_doc = xDocBuilder->parse(xIn);
        }
        else if (err == ::osl::File::E_NOENT)
        {
            //Create a new document and insert some basic stuff
            m_doc = xDocBuilder->newDocument();
            const Reference<css::xml::dom::XElement> rootNode =
                m_doc->createElementNS(getDbNSName(), getNSPrefix() +
                                       ":" + getRootElementName());

            m_doc->appendChild(Reference<css::xml::dom::XNode>(
                                   rootNode, UNO_QUERY_THROW));
            save();
        }
        else
            throw css::uno::RuntimeException(
                "Extension manager could not access database file:"
                + m_urlDb, nullptr);

        if (!m_doc.is())
            throw css::uno::RuntimeException(
                "Extension manager could not get root node of data base file: "
                      + m_urlDb, nullptr);
    }

    return m_doc;
}

Reference<css::xml::xpath::XXPathAPI> const & BackendDb::getXPathAPI()
{
    if (!m_xpathApi.is())
    {
        m_xpathApi = css::xml::xpath::XPathAPI::create( m_xContext );

        m_xpathApi->registerNS( getNSPrefix(), getDbNSName() );
    }

    return m_xpathApi;
}

void BackendDb::removeElement(OUString const & sXPathExpression)
{
    try
    {
        const Reference<css::xml::dom::XDocument> doc = getDocument();
        const Reference<css::xml::dom::XNode> root = doc->getFirstChild();
        const Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        //find the extension element that is to be removed
        const Reference<css::xml::dom::XNode> aNode =
            xpathApi->selectSingleNode(root, sXPathExpression);

        if (aNode.is())
        {
            root->removeChild(aNode);
            save();
        }

#if    OSL_DEBUG_LEVEL > 0
        //There must not be any other entry with the same url
        const Reference<css::xml::dom::XNode> nextNode =
            xpathApi->selectSingleNode(root, sXPathExpression);
        OSL_ASSERT(! nextNode.is());
#endif
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

void BackendDb::removeEntry(OUString const & url)
{
    const OUString sKeyElement = getKeyElementName();
    const OUString sPrefix = getNSPrefix();
    OUStringBuffer sExpression(500);
    sExpression.append(sPrefix);
    sExpression.append(":");
    sExpression.append(sKeyElement);
    sExpression.append("[@url = \"");
    sExpression.append(url);
    sExpression.append("\"]");

    removeElement(sExpression.makeStringAndClear());
}

void BackendDb::revokeEntry(OUString const & url)
{
    try
    {
        Reference<css::xml::dom::XElement> entry(getKeyElement(url), UNO_QUERY);
        if (entry.is())
        {
            entry->setAttribute("revoked", "true");
            save();
        }
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to revoke data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

bool BackendDb::activateEntry(OUString const & url)
{
    try
    {
        bool ret = false;
        Reference<css::xml::dom::XElement> entry(getKeyElement(url), UNO_QUERY);
        if (entry.is())
        {
            //no attribute "active" means it is active, that is, registered.
            entry->removeAttribute("revoked");
            save();
            ret = true;
        }
        return ret;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to revoke data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

bool BackendDb::hasActiveEntry(OUString const & url)
{
    try
    {
        bool ret = false;
        Reference<css::xml::dom::XElement> entry(getKeyElement(url), UNO_QUERY);
        if (entry.is())
        {
            OUString sActive = entry->getAttribute("revoked");
            if (!(sActive == "true"))
                ret = true;
        }
        return ret;

    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to determine an active entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

Reference<css::xml::dom::XNode> BackendDb::getKeyElement(
    OUString const & url)
{
    try
    {
        const OUString sPrefix = getNSPrefix();
        const OUString sKeyElement = getKeyElementName();
        OUStringBuffer sExpression(500);
        sExpression.append(sPrefix);
        sExpression.append(":");
        sExpression.append(sKeyElement);
        sExpression.append("[@url = \"");
        sExpression.append(url);
        sExpression.append("\"]");

        const Reference<css::xml::dom::XDocument> doc = getDocument();
        const Reference<css::xml::dom::XNode> root = doc->getFirstChild();
        const Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        return xpathApi->selectSingleNode(root, sExpression.makeStringAndClear());
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read key element in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

//Only writes the data if there is at least one entry
void BackendDb::writeVectorOfPair(
    ::std::vector< ::std::pair< OUString, OUString > > const & vecPairs,
    OUString const & sVectorTagName,
    OUString const & sPairTagName,
    OUString const & sFirstTagName,
    OUString const & sSecondTagName,
    css::uno::Reference<css::xml::dom::XNode> const & xParent)
{
    try{
        if (vecPairs.empty())
            return;
        const OUString sNameSpace = getDbNSName();
        OSL_ASSERT(!sNameSpace.isEmpty());
        const OUString sPrefix(getNSPrefix() + ":");
        const Reference<css::xml::dom::XDocument> doc = getDocument();
        const Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        const Reference<css::xml::dom::XElement> vectorNode(
            doc->createElementNS(sNameSpace, sPrefix + sVectorTagName));

        xParent->appendChild(
            Reference<css::xml::dom::XNode>(
                vectorNode, css::uno::UNO_QUERY_THROW));
        typedef ::std::vector< ::std::pair< OUString, OUString > >::const_iterator CIT;
        for (CIT i = vecPairs.begin(); i != vecPairs.end(); ++i)
        {
            const Reference<css::xml::dom::XElement> pairNode(
                doc->createElementNS(sNameSpace, sPrefix + sPairTagName));

            vectorNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    pairNode, css::uno::UNO_QUERY_THROW));

            const Reference<css::xml::dom::XElement> firstNode(
                doc->createElementNS(sNameSpace, sPrefix + sFirstTagName));

            pairNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    firstNode, css::uno::UNO_QUERY_THROW));

            const Reference<css::xml::dom::XText> firstTextNode(
                doc->createTextNode( i->first));

            firstNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    firstTextNode, css::uno::UNO_QUERY_THROW));

            const Reference<css::xml::dom::XElement> secondNode(
                doc->createElementNS(sNameSpace, sPrefix + sSecondTagName));

            pairNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    secondNode, css::uno::UNO_QUERY_THROW));

            const Reference<css::xml::dom::XText> secondTextNode(
                doc->createTextNode( i->second));

            secondNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    secondTextNode, css::uno::UNO_QUERY_THROW));
        }
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

::std::vector< ::std::pair< OUString, OUString > >
BackendDb::readVectorOfPair(
    Reference<css::xml::dom::XNode> const & parent,
    OUString const & sListTagName,
    OUString const & sPairTagName,
    OUString const & sFirstTagName,
    OUString const & sSecondTagName)
{
    try
    {
        OSL_ASSERT(parent.is());
        const OUString sPrefix(getNSPrefix() + ":");
        const Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const OUString sExprPairs(
            sPrefix + sListTagName + "/" + sPrefix + sPairTagName);
        const Reference<css::xml::dom::XNodeList> listPairs =
            xpathApi->selectNodeList(parent, sExprPairs);

        ::std::vector< ::std::pair< OUString, OUString > > retVector;
        sal_Int32 length = listPairs->getLength();
        for (sal_Int32 i = 0; i < length; i++)
        {
            const Reference<css::xml::dom::XNode> aPair = listPairs->item(i);
            const OUString sExprFirst(sPrefix + sFirstTagName + "/text()");
            const Reference<css::xml::dom::XNode> first =
                xpathApi->selectSingleNode(aPair, sExprFirst);

            const OUString sExprSecond(sPrefix + sSecondTagName + "/text()");
            const Reference<css::xml::dom::XNode> second =
                xpathApi->selectSingleNode(aPair, sExprSecond);
            OSL_ASSERT(first.is() && second.is());

            retVector.push_back(::std::make_pair(
                                    first->getNodeValue(), second->getNodeValue()));
        }
        return retVector;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

//Only writes the data if there is at least one entry
void BackendDb::writeSimpleList(
    ::std::list< OUString> const & list,
    OUString const & sListTagName,
    OUString const & sMemberTagName,
    Reference<css::xml::dom::XNode> const & xParent)
{
    try
    {
        if (list.empty())
            return;
        const OUString sNameSpace = getDbNSName();
        const OUString sPrefix(getNSPrefix() + ":");
        const Reference<css::xml::dom::XDocument> doc = getDocument();

        const Reference<css::xml::dom::XElement> listNode(
            doc->createElementNS(sNameSpace, sPrefix + sListTagName));

        xParent->appendChild(
            Reference<css::xml::dom::XNode>(
                listNode, css::uno::UNO_QUERY_THROW));

        typedef ::std::list<OUString>::const_iterator ITC_ITEMS;
        for (ITC_ITEMS i = list.begin(); i != list.end(); ++i)
        {
            const Reference<css::xml::dom::XNode> memberNode(
                doc->createElementNS(sNameSpace, sPrefix + sMemberTagName), css::uno::UNO_QUERY_THROW);

            listNode->appendChild(memberNode);

            const Reference<css::xml::dom::XNode> textNode(
                doc->createTextNode( *i), css::uno::UNO_QUERY_THROW);

            memberNode->appendChild(textNode);
        }
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

//Writes only the element if is has a value.
//The prefix is automatically added to the element name
void BackendDb::writeSimpleElement(
    OUString const & sElementName, OUString const & value,
    Reference<css::xml::dom::XNode> const & xParent)
{
    try
    {
        if (value.isEmpty())
            return;
        const OUString sPrefix = getNSPrefix();
        const Reference<css::xml::dom::XDocument> doc = getDocument();
        const OUString sNameSpace = getDbNSName();
        const Reference<css::xml::dom::XNode> dataNode(
            doc->createElementNS(sNameSpace, sPrefix + ":" + sElementName),
            UNO_QUERY_THROW);
        xParent->appendChild(dataNode);

        const Reference<css::xml::dom::XNode> dataValue(
            doc->createTextNode(value), UNO_QUERY_THROW);
        dataNode->appendChild(dataValue);
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry(writeSimpleElement) in backend db: " +
            m_urlDb, nullptr, exc);
    }

}

/** The key elements have an url attribute and are always children of the root
    element.
*/
Reference<css::xml::dom::XNode> BackendDb::writeKeyElement(
    OUString const & url)
{
    try
    {
        const OUString sNameSpace = getDbNSName();
        const OUString sPrefix = getNSPrefix();
        const OUString sElementName = getKeyElementName();
        const Reference<css::xml::dom::XDocument> doc = getDocument();
        const Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        //Check if there are an entry with the same url. This can be the case if the
        //the status of an XPackage is ambiguous. In this case a call to activateExtension
        //(dp_extensionmanager.cxx), will register the package again. See also
        //Package::processPackage_impl in dp_backend.cxx.
        //A package can become
        //invalid after its successful registration, for example if a second extension with
        //the same service is installed.
        const OUString sExpression(
            sPrefix + ":" + sElementName + "[@url = \"" + url + "\"]");
        const Reference<css::xml::dom::XNode> existingNode =
            getXPathAPI()->selectSingleNode(root, sExpression);
        if (existingNode.is())
        {
            OSL_ASSERT(false);
            //replace the existing entry.
            removeEntry(url);
        }

        const Reference<css::xml::dom::XElement> keyElement(
            doc->createElementNS(sNameSpace, sPrefix +  ":" + sElementName));

        keyElement->setAttribute("url", url);

        const Reference<css::xml::dom::XNode> keyNode(
            keyElement, UNO_QUERY_THROW);
        root->appendChild(keyNode);
        return keyNode;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write key element in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

OUString BackendDb::readSimpleElement(
    OUString const & sElementName, Reference<css::xml::dom::XNode> const & xParent)
{
    try
    {
        const OUString sPrefix = getNSPrefix();
        const OUString sExpr(sPrefix + ":" + sElementName + "/text()");
        const Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const Reference<css::xml::dom::XNode> val =
            xpathApi->selectSingleNode(xParent, sExpr);
        if (val.is())
            return val->getNodeValue();
        return OUString();
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data (readSimpleElement) in backend db: " +
            m_urlDb, nullptr, exc);
    }
}


::std::list< OUString> BackendDb::readList(
    Reference<css::xml::dom::XNode> const & parent,
    OUString const & sListTagName,
    OUString const & sMemberTagName)
{
    try
    {
        OSL_ASSERT(parent.is());
        const OUString sPrefix(getNSPrefix() + ":");
        const Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const OUString sExprList(
            sPrefix + sListTagName + "/" + sPrefix + sMemberTagName + "/text()");
        const Reference<css::xml::dom::XNodeList> list =
            xpathApi->selectNodeList(parent, sExprList);

        ::std::list<OUString > retList;
        sal_Int32 length = list->getLength();
        for (sal_Int32 i = 0; i < length; i++)
        {
            const Reference<css::xml::dom::XNode> member = list->item(i);
            retList.push_back(member->getNodeValue());
        }
        return retList;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

::std::list<OUString> BackendDb::getOneChildFromAllEntries(
    OUString const & name)
{
    try
    {
        ::std::list<OUString> listRet;
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const OUString sPrefix = getNSPrefix();
        const OUString sKeyElement = getKeyElementName();
        OUStringBuffer buf(512);
        buf.append(sPrefix);
        buf.append(":");
        buf.append(sKeyElement);
        buf.append("/");
        buf.append(sPrefix);
        buf.append(":");
        buf.append(name);
        buf.append("/text()");

        Reference<css::xml::dom::XNodeList> nodes =
            xpathApi->selectNodeList(root, buf.makeStringAndClear());
        if (nodes.is())
        {
            sal_Int32 length = nodes->getLength();
            for (sal_Int32 i = 0; i < length; i++)
                listRet.push_back(nodes->item(i)->getNodeValue());
        }
        return listRet;
    }
    catch ( const css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}


RegisteredDb::RegisteredDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):BackendDb(xContext, url)
{
}

void RegisteredDb::addEntry(OUString const & url)
{
    try{
        if (!activateEntry(url))
        {
            const OUString sNameSpace = getDbNSName();
            const OUString sPrefix = getNSPrefix();
            const OUString sEntry = getKeyElementName();

            Reference<css::xml::dom::XDocument> doc = getDocument();
            Reference<css::xml::dom::XNode> root = doc->getFirstChild();

#if    OSL_DEBUG_LEVEL > 0
            //There must not be yet an entry with the same url
            OUString sExpression(
                sPrefix + ":" + sEntry + "[@url = \"" + url + "\"]");
            Reference<css::xml::dom::XNode> _extensionNode =
                getXPathAPI()->selectSingleNode(root, sExpression);
            OSL_ASSERT(! _extensionNode.is());
#endif
            Reference<css::xml::dom::XElement> helpElement(
                doc->createElementNS(sNameSpace, sPrefix +  ":" + sEntry));

            helpElement->setAttribute("url", url);

            Reference<css::xml::dom::XNode> helpNode(
                helpElement, UNO_QUERY_THROW);
            root->appendChild(helpNode);

            save();
        }
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}

} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
