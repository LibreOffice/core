/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_package.cxx,v $
 * $Revision: 1.34.16.2 $
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
#include "rtl/bootstrap.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "dp_misc.h"

#include "dp_helpbackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/help-registry/2010"
#define NS_PREFIX "help"
#define ROOT_ELEMENT_NAME "help-backend-db"

namespace dp_registry {
namespace backend {
namespace help {

HelpBackendDb::HelpBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):BackendDb(xContext, url)
{

}

OUString HelpBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString HelpBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString HelpBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

void HelpBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{

        const OUString sNameSpace = getDbNSName();
        const OUString sPrefix = getNSPrefix();
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

#if    OSL_DEBUG_LEVEL > 0
        //There must not be yet an entry with the same url
        OUString sExpression(
            sPrefix + OUSTR(":help[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XNode> _extensionNode =
            getXPathAPI()->selectSingleNode(root, sExpression);
        OSL_ASSERT(! _extensionNode.is());
#endif
        Reference<css::xml::dom::XElement> helpElement(
            doc->createElementNS(sNameSpace, sPrefix +  OUSTR(":help")));

        helpElement->setAttribute(OUSTR("url"), url);

        Reference<css::xml::dom::XNode> helpNode(
            helpElement, UNO_QUERY_THROW);
        root->appendChild(helpNode);

        Reference<css::xml::dom::XNode> dataNode(
            doc->createElementNS(sNameSpace, sPrefix + OUSTR(":data-url")),
            UNO_QUERY_THROW);
        helpNode->appendChild(dataNode);

        Reference<css::xml::dom::XNode> dataValue(
            doc->createTextNode(data.dataUrl), UNO_QUERY_THROW);
        dataNode->appendChild(dataValue);

//         writeSimpleList(
//             data.implementationNames,
//             OUSTR("implementation-names"),
//             OUSTR("name"),
//             componentNode);

//         writeVectorOfPair(
//             data.singletons,
//             OUSTR("singletons"),
//             OUSTR("item"),
//             OUSTR("key"),
//             OUSTR("value"),
//             componentNode);

        save();
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

void HelpBackendDb::removeEntry(::rtl::OUString const & url)
{
    OUString sExpression(
        OUSTR(NS_PREFIX) + OUSTR(":help[@url = \"") + url + OUSTR("\"]"));
    removeElement(sExpression);
}

::boost::optional<HelpBackendDb::Data>
HelpBackendDb::getEntry(::rtl::OUString const & url)
{
    try
    {
        const OUString sPrefix = getNSPrefix();
        HelpBackendDb::Data retData;
        const OUString sExpression(
            sPrefix + OUSTR(":help[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        //find the extension element that is to be removed
        Reference<css::xml::dom::XNode> aNode =
            xpathApi->selectSingleNode(root, sExpression);
        if (aNode.is())
        {
            const OUString sExprDataUrl(sPrefix + OUSTR(":data-url/text()"));

            Reference<css::xml::dom::XNode> dataUrlVal =
                xpathApi->selectSingleNode(aNode, sExprDataUrl);
            retData.dataUrl = dataUrlVal->getNodeValue();

//             retData.implementationNames =
//                 readList(
//                     aNode, OUSTR("reg:implementation-names"), OUSTR("reg:name"));

//             retData.singletons =
//                 readVectorOfPair(
//                     aNode, OUSTR("reg:singletons"), OUSTR("item"), OUSTR("key"),
//                     OUSTR("value"));

        }
        else
        {
            return ::boost::optional<Data>();
        }
        return ::boost::optional<Data>(retData);
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

::std::list<OUString> HelpBackendDb::getAllDataUrls()
{
    try
    {
        ::std::list<OUString> listRet;
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const OUString sPrefix = getNSPrefix();
        OUString sExpression(
            sPrefix + OUSTR(":help/") + sPrefix + OUSTR(":data-url/text()"));
        Reference<css::xml::dom::XNodeList> nodes =
            xpathApi->selectNodeList(root, sExpression);
        if (nodes.is())
        {
            sal_Int32 length = nodes->getLength();
            for (sal_Int32 i = 0; i < length; i++)
                listRet.push_back(nodes->item(i)->getNodeValue());
        }
        return listRet;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}


} // namespace help
} // namespace backend
} // namespace dp_registry

