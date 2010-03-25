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
#include "precompiled_desktop.hxx"

#include "rtl/string.h"
#include "rtl/strbuf.hxx"
#include "rtl/bootstrap.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "osl/file.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "com/sun/star/io/XActiveDataSource.hpp"
#include "com/sun/star/io/XActiveDataControl.hpp"
#include "dp_ucb.h"
#include "dp_misc.h"
#include "ucbhelper/content.hxx"
#include "xmlscript/xml_helper.hxx"
#include "dp_backenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;


namespace dp_registry {
namespace backend {

BackendDb::BackendDb(
    Reference<css::uno::XComponentContext> const &  xContext,
    ::rtl::OUString const & url):
    m_xContext(xContext)
{
    m_urlDb = dp_misc::expandUnoRcUrl(url);
}

void BackendDb::save()
{
    Reference<css::io::XActiveDataSource> xDataSource(m_doc,css::uno::UNO_QUERY_THROW);
    ::rtl::ByteSequence bytes;
    xDataSource->setOutputStream(::xmlscript::createOutputStream(&bytes));
    Reference<css::io::XActiveDataControl> xDataControl(m_doc,css::uno::UNO_QUERY_THROW);
    xDataControl->start();

    Reference<css::io::XInputStream> xData(
        ::xmlscript::createInputStream(bytes));
    ::ucbhelper::Content ucbDb(m_urlDb, 0);
    ucbDb.writeStream(xData, true /*replace existing*/);
}

css::uno::Reference<css::xml::dom::XDocument> BackendDb::getDocument()
{
    if (!m_doc.is())
    {
        Reference<css::xml::dom::XDocumentBuilder> xDocBuilder(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.xml.dom.DocumentBuilder"),
                m_xContext ), css::uno::UNO_QUERY);
        if (!xDocBuilder.is())
            throw css::uno::RuntimeException(
                OUSTR(" Could not create service com.sun.star.xml.dom.DocumentBuilder"), 0);

        ::osl::DirectoryItem item;
        ::osl::File::RC err = ::osl::DirectoryItem::get(m_urlDb, item);
        if (err == ::osl::File::E_None)
        {
            m_doc = xDocBuilder->parseURI(m_urlDb);
        }
        else if (err == ::osl::File::E_NOENT)
        {
            //Create a new document and insert some basic stuff
            m_doc = xDocBuilder->newDocument();
            Reference<css::xml::dom::XElement> rootNode =
                m_doc->createElementNS(getDbNSName(), getNSPrefix() +
                                       OUSTR(":") + getRootElementName());
//             rootNode->setAttribute(
//                 OUSTR("xmlns"), getDbNSName());
            m_doc->appendChild(Reference<css::xml::dom::XNode>(
                                   rootNode, UNO_QUERY_THROW));
            save();
        }
        else
            throw css::uno::RuntimeException(
                OUSTR("Extension manager could not access database file:" )
                + m_urlDb, 0);

        if (!m_doc.is())
            throw css::uno::RuntimeException(
                OUSTR("Extension manager could not get root node of data base file: ")
                      + m_urlDb, 0);
    }

    return m_doc;
}

Reference<css::xml::xpath::XXPathAPI> BackendDb::getXPathAPI()
{
    if (!m_xpathApi.is())
    {
        m_xpathApi = Reference< css::xml::xpath::XXPathAPI >(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.xml.xpath.XPathAPI"),
                m_xContext), css::uno::UNO_QUERY);

        if (!m_xpathApi.is())
            throw css::uno::RuntimeException(
                OUSTR(" Could not create service com.sun.star.xml.xpath.XPathAPI"), 0);

        m_xpathApi->registerNS(
            getNSPrefix(), getDbNSName());
    }

    return m_xpathApi;
}

void BackendDb::removeElement(::rtl::OUString const & sXPathExpression)
{
    try
    {
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();
        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        //find the extension element that is to be removed
        Reference<css::xml::dom::XNode> aNode =
            xpathApi->selectSingleNode(root, sXPathExpression);
        OSL_ASSERT(aNode.is());
        if (aNode.is())
        {
            root->removeChild(aNode);
            save();
        }

#if    OSL_DEBUG_LEVEL > 0
        //There must not be any other entry with the same url
        Reference<css::xml::dom::XNode> nextNode =
            xpathApi->selectSingleNode(root, sXPathExpression);
        OSL_ASSERT(! nextNode.is());
#endif
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in backend db: ") +
            m_urlDb, 0, exc);
    }

}

void BackendDb::writeVectorOfPair(
    ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > > const & vecPairs,
    OUString const & sVectorTagName,
    OUString const & sPairTagName,
    OUString const & sFirstTagName,
    OUString const & sSecondTagName,
    css::uno::Reference<css::xml::dom::XNode> const & xParent)
{
    try{
        const OUString sNameSpace = getDbNSName();
        OSL_ASSERT(sNameSpace.getLength());
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::dom::XElement> vectorNode(
            doc->createElementNS(sNameSpace, sVectorTagName));

        xParent->appendChild(
            Reference<css::xml::dom::XNode>(
                vectorNode, css::uno::UNO_QUERY_THROW));
        typedef ::std::vector< ::std::pair< OUString, OUString > >::const_iterator CIT;
        for (CIT i = vecPairs.begin(); i != vecPairs.end(); i++)
        {
            Reference<css::xml::dom::XElement> pairNode(
                doc->createElementNS(sNameSpace, sPairTagName));

            vectorNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    pairNode, css::uno::UNO_QUERY_THROW));

            Reference<css::xml::dom::XElement> firstNode(
                doc->createElementNS(sNameSpace, sFirstTagName));

            pairNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    firstNode, css::uno::UNO_QUERY_THROW));

            Reference<css::xml::dom::XText> firstTextNode(
                doc->createTextNode( i->first));

            firstNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    firstTextNode, css::uno::UNO_QUERY_THROW));

            Reference<css::xml::dom::XElement> secondNode(
                doc->createElementNS(sNameSpace, sSecondTagName));

            pairNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    secondNode, css::uno::UNO_QUERY_THROW));

            Reference<css::xml::dom::XText> secondTextNode(
                doc->createTextNode( i->second));

            secondNode->appendChild(
                Reference<css::xml::dom::XNode>(
                    secondTextNode, css::uno::UNO_QUERY_THROW));
        }
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in backend db: ") +
            m_urlDb, 0, exc);
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
        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        OUString sExprPairs(
            sListTagName + OUSTR("/") + sPairTagName);
        Reference<css::xml::dom::XNodeList> listPairs =
            xpathApi->selectNodeList(parent, sExprPairs);

        ::std::vector< ::std::pair< OUString, OUString > > retVector;
        sal_Int32 length = listPairs->getLength();
        for (sal_Int32 i = 0; i < length; i++)
        {
            Reference<css::xml::dom::XNode> aPair = listPairs->item(i);
            OUString sExprFirst(sFirstTagName + OUSTR("/text()"));
            Reference<css::xml::dom::XNode> first =
                xpathApi->selectSingleNode(aPair, sExprFirst);

            OUString sExprSecond(sSecondTagName + OUSTR("/text()"));
            Reference<css::xml::dom::XNode> second =
                xpathApi->selectSingleNode(aPair, sExprSecond);
            OSL_ASSERT(first.is() && second.is());

            retVector.push_back(::std::make_pair(
                                    first->getNodeValue(), second->getNodeValue()));
        }
        return retVector;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

void BackendDb::writeSimpleList(
    ::std::list< ::rtl::OUString> const & list,
    OUString const & sListTagName,
    OUString const & sMemberTagName,
    Reference<css::xml::dom::XNode> const & xParent)
{
    try
    {
        const OUString sNameSpace = getDbNSName();
        Reference<css::xml::dom::XDocument> doc = getDocument();

        Reference<css::xml::dom::XElement> listNode(
            doc->createElementNS(sNameSpace, sListTagName));

        xParent->appendChild(
            Reference<css::xml::dom::XNode>(
                listNode, css::uno::UNO_QUERY_THROW));

        typedef ::std::list<OUString>::const_iterator ITC_ITEMS;
        for (ITC_ITEMS i = list.begin(); i != list.end(); i++)
        {
            Reference<css::xml::dom::XNode> memberNode(
                doc->createElementNS(sNameSpace, sMemberTagName), css::uno::UNO_QUERY_THROW);

            listNode->appendChild(memberNode);

            Reference<css::xml::dom::XNode> textNode(
                doc->createTextNode( *i), css::uno::UNO_QUERY_THROW);

            memberNode->appendChild(textNode);
        }
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in backend db: ") +
            m_urlDb, 0, exc);
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
        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        OUString sExprList(
            sListTagName + OUSTR("/") + sMemberTagName + OUSTR("/text()"));
        Reference<css::xml::dom::XNodeList> list =
            xpathApi->selectNodeList(parent, sExprList);

        ::std::list<OUString > retList;
        sal_Int32 length = list->getLength();
        for (sal_Int32 i = 0; i < length; i++)
        {
            Reference<css::xml::dom::XNode> member = list->item(i);
            retList.push_back(member->getNodeValue());
        }
        return retList;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}



} // namespace backend
} // namespace dp_registry

