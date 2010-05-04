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
#include "rtl/bootstrap.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "dp_misc.h"

#include "dp_extbackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/extension-registry/2010"
#define ROOT_ELEMENT_NAME "extension-backend-db"

// /extension-backend-db/extension
#define EXTENSION_ELEMENT "extension"

// /extension-backend-db/extension/extension-items
#define EXTENSION_ITEMS "extension-items"

// /extension-backend-db/extension/extension-items/item
#define EXTENSION_ITEMS_ITEM "item"

// /extension-backend-db/extension/extension-items/item/url
#define ITEM_URL "url"

// /extension-backend-db/extension/extension-items/item/media-type
#define ITEM_MEDIA_TYP "media-type"


namespace dp_registry {
namespace backend {
namespace bundle {

ExtensionBackendDb::ExtensionBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):BackendDb(xContext, url)
{

}

OUString ExtensionBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString ExtensionBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

void ExtensionBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{

        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

#if    OSL_DEBUG_LEVEL > 0
        //There must not be yet an entry with the same url
        OUString sExpression(
            OUSTR("reg:extension[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XNode> _extensionNode =
            getXPathAPI()->selectSingleNode(root, sExpression);
        OSL_ASSERT(! _extensionNode.is());
#endif
        // <extension url="file:///...">
        Reference<css::xml::dom::XElement> extensionNode(
            doc->createElement(OUSTR("extension")));

        extensionNode->setAttribute(OUSTR("url"), url);

        Reference<css::xml::dom::XNode> extensionNodeNode(
            extensionNode, css::uno::UNO_QUERY_THROW);
        root->appendChild(extensionNodeNode);

        // <identifier>...</identifier>
        Reference<css::xml::dom::XNode> identifierNode(
            doc->createElement(OUSTR("identifier")), UNO_QUERY_THROW);
        extensionNodeNode->appendChild(identifierNode);

        Reference<css::xml::dom::XNode> identifierValue(
            doc->createTextNode(data.identifier), UNO_QUERY_THROW);
        identifierNode->appendChild(identifierValue);


        writeVectorOfPair(
            data.items,
            OUSTR("extension-items"),
            OUSTR("item"),
            OUSTR("url"),
            OUSTR("media-type"),
            extensionNodeNode);
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

void ExtensionBackendDb::removeEntry(::rtl::OUString const & url)
{
    OUString sExpression(
        OUSTR("reg:extension[@url = \"") + url + OUSTR("\"]"));
    removeElement(sExpression);
}

ExtensionBackendDb::Data ExtensionBackendDb::getEntry(::rtl::OUString const & url)
{
    ExtensionBackendDb::Data retData;
    const OUString sExpression(
        OUSTR("reg:extension[@url = \"") + url + OUSTR("\"]"));
    Reference<css::xml::dom::XDocument> doc = getDocument();
    Reference<css::xml::dom::XNode> root = doc->getFirstChild();

    Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        //find the extension element that is to be removed
    Reference<css::xml::dom::XNode> aNode =
        xpathApi->selectSingleNode(root, sExpression);
    OSL_ASSERT(aNode.is());

    const OUString sExprIdentifier(OUSTR("reg:identifier/text()"));

    Reference<css::xml::dom::XNode> idValueNode =
        xpathApi->selectSingleNode(aNode, sExprIdentifier);
    retData.identifier = idValueNode->getNodeValue();

    retData.items =
        readVectorOfPair(
            aNode,
            OUSTR("reg:extension-items"),
            OUSTR("reg:item"), OUSTR("reg:url"), OUSTR("reg:media-type"));
    return retData;
}

} // namespace bundle
} // namespace backend
} // namespace dp_registry

