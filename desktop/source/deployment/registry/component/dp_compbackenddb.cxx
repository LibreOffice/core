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

OUString ComponentBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

void ComponentBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{

        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

#if    OSL_DEBUG_LEVEL > 0
        //There must not be yet an entry with the same url
        OUString sExpression(
            OUSTR("reg:component[@url = \"") + url + OUSTR("\"]"));
        Reference<css::xml::dom::XNode> _extensionNode =
            getXPathAPI()->selectSingleNode(root, sExpression);
        OSL_ASSERT(! _extensionNode.is());
#endif
        Reference<css::xml::dom::XElement> componentElement(
            doc->createElement(OUSTR("component")));

        componentElement->setAttribute(OUSTR("url"), url);

        Reference<css::xml::dom::XNode> componentNode(
            componentElement, UNO_QUERY_THROW);

        root->appendChild(componentNode);

        Reference<css::xml::dom::XNode> javaTypeLibNode(
            doc->createElement(OUSTR("java-type-library")), UNO_QUERY_THROW);

        componentNode->appendChild(javaTypeLibNode);

        Reference<css::xml::dom::XNode> javaTypeLibValueNode(
            doc->createTextNode(OUString::valueOf((sal_Bool) data.javaTypeLibrary)),
            UNO_QUERY_THROW);
        javaTypeLibNode->appendChild(javaTypeLibValueNode);

        writeSimpleList(
            data.implementationNames,
            OUSTR("implementation-names"),
            OUSTR("name"),
            componentNode);

        writeVectorOfPair(
            data.singletons,
            OUSTR("singletons"),
            OUSTR("item"),
            OUSTR("key"),
            OUSTR("value"),
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
        OUSTR("reg:component[@url = \"") + url + OUSTR("\"]"));
    removeElement(sExpression);
}



} // namespace bundle
} // namespace backend
} // namespace dp_registry

