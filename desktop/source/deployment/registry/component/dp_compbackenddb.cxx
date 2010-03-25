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

#include "dp_compbackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/component-registry/2010"
#define NS_PREFIX "comp"
#define ROOT_ELEMENT_NAME "component-backend-db"

namespace dp_registry {
namespace backend {
namespace component {

ComponentBackendDb::ComponentBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):BackendDb(xContext, url)
{

}

OUString ComponentBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString ComponentBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString ComponentBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

void ComponentBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{
        const OUString sNameSpace = getDbNSName();
        const OUString sPrefix = getNSPrefix();
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

#if    OSL_DEBUG_LEVEL > 0
        //There must not be yet an entry with the same url
        OUString sExpression(
            sPrefix + OUSTR(":component[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XNode> _extensionNode =
            getXPathAPI()->selectSingleNode(root, sExpression);
        OSL_ASSERT(! _extensionNode.is());
#endif
        Reference<css::xml::dom::XElement> componentElement(
            doc->createElementNS(sNameSpace, sPrefix + OUSTR(":component")));

        componentElement->setAttribute(OUSTR("url"), url);

        Reference<css::xml::dom::XNode> componentNode(
            componentElement, UNO_QUERY_THROW);

        root->appendChild(componentNode);

        Reference<css::xml::dom::XNode> javaTypeLibNode(
            doc->createElementNS(sNameSpace, sPrefix + OUSTR(":java-type-library")), UNO_QUERY_THROW);

        componentNode->appendChild(javaTypeLibNode);

        Reference<css::xml::dom::XNode> javaTypeLibValueNode(
            doc->createTextNode(OUString::valueOf((sal_Bool) data.javaTypeLibrary)),
            UNO_QUERY_THROW);
        javaTypeLibNode->appendChild(javaTypeLibValueNode);

        writeSimpleList(
            data.implementationNames,
            sPrefix + OUSTR(":implementation-names"),
            sPrefix + OUSTR(":name"),
            componentNode);

        writeVectorOfPair(
            data.singletons,
            sPrefix + OUSTR(":singletons"),
            sPrefix + OUSTR(":item"),
            sPrefix + OUSTR(":key"),
            sPrefix + OUSTR(":value"),
            componentNode);

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

void ComponentBackendDb::removeEntry(::rtl::OUString const & url)
{
    OUString sExpression(
        OUSTR(NS_PREFIX) + OUSTR(":component[@url = \"") + url + OUSTR("\"]"));
    removeElement(sExpression);
}

ComponentBackendDb::Data ComponentBackendDb::getEntry(::rtl::OUString const & url)
{
    try
    {
        const OUString sPrefix = getNSPrefix();
        ComponentBackendDb::Data retData;
        const OUString sExpression(
            sPrefix + OUSTR(":component[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        //find the extension element that is to be removed
        Reference<css::xml::dom::XNode> aNode =
            xpathApi->selectSingleNode(root, sExpression);
        if (aNode.is())
        {
            const OUString sExprJavaTypeLib(sPrefix + OUSTR(":java-type-library/text()"));

            Reference<css::xml::dom::XNode> idValueNode =
                xpathApi->selectSingleNode(aNode, sExprJavaTypeLib);
            retData.javaTypeLibrary =
                idValueNode->getNodeValue().equals(OUSTR("true")) ? true : false;

            retData.implementationNames =
                readList(
                    aNode,
                    sPrefix + OUSTR(":implementation-names"),
                    sPrefix + OUSTR(":name"));

            retData.singletons =
                readVectorOfPair(
                    aNode,
                    sPrefix + OUSTR(":singletons"),
                    sPrefix + OUSTR(":item"),
                    sPrefix + OUSTR(":key"),
                    sPrefix + OUSTR(":value"));
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

