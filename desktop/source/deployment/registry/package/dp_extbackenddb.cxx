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
#define NS_PREFIX "ext"
#define ROOT_ELEMENT_NAME "extension-backend-db"

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

OUString ExtensionBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString ExtensionBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

void ExtensionBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{

        const OUString sNameSpace = getDbNSName();
        const OUString sPrefix = getNSPrefix();
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

#if    OSL_DEBUG_LEVEL > 0
        //There must not be yet an entry with the same url
        OUString sExpression(
            sPrefix + OUSTR(":extension[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XNode> _extensionNode =
            getXPathAPI()->selectSingleNode(root, sExpression);
        OSL_ASSERT(! _extensionNode.is());
#endif
        // <extension url="file:///...">
        Reference<css::xml::dom::XElement> extensionNode(
            doc->createElementNS(sNameSpace,
                                 sPrefix + OUSTR(":extension")));

        extensionNode->setAttribute(OUSTR("url"), url);

        Reference<css::xml::dom::XNode> extensionNodeNode(
            extensionNode, css::uno::UNO_QUERY_THROW);
        root->appendChild(extensionNodeNode);

        writeVectorOfPair(
            data.items,
            sPrefix + OUSTR(":extension-items"),
            sPrefix + OUSTR(":item"),
            sPrefix + OUSTR(":url"),
            sPrefix + OUSTR(":media-type"),
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
        OUSTR(NS_PREFIX) + OUSTR(":extension[@url = \"") + url + OUSTR("\"]"));
    removeElement(sExpression);
}

ExtensionBackendDb::Data ExtensionBackendDb::getEntry(::rtl::OUString const & url)
{
    try
    {
        const OUString sPrefix = getNSPrefix();
        ExtensionBackendDb::Data retData;
        const OUString sExpression(
            sPrefix + OUSTR(":extension[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();

        Reference<css::xml::dom::XNode> aNode =
            xpathApi->selectSingleNode(root, sExpression);
        if (aNode.is())
        {
            retData.items =
                readVectorOfPair(
                    aNode,
                    sPrefix + OUSTR(":extension-items"),
                    sPrefix + OUSTR(":item"),
                    sPrefix + OUSTR(":url"),
                    sPrefix + OUSTR(":media-type"));
        }
        return retData;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

} // namespace bundle
} // namespace backend
} // namespace dp_registry

