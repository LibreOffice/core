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


#include "rtl/string.h"
#include "rtl/bootstrap.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "dp_misc.h"

#include "dp_compbackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/component-registry/2010"
#define NS_PREFIX "comp"
#define ROOT_ELEMENT_NAME "component-backend-db"
#define KEY_ELEMENT_NAME "component"

namespace dp_registry {
namespace backend {
namespace component {

ComponentBackendDb::ComponentBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):BackendDb(xContext, url)
{

}

OUString ComponentBackendDb::getDbNSName()
{
    return OUString(EXTENSION_REG_NS);
}

OUString ComponentBackendDb::getNSPrefix()
{
    return OUString(NS_PREFIX);
}

OUString ComponentBackendDb::getRootElementName()
{
    return OUString(ROOT_ELEMENT_NAME);
}

OUString ComponentBackendDb::getKeyElementName()
{
    return OUString(KEY_ELEMENT_NAME);
}

void ComponentBackendDb::addEntry(OUString const & url, Data const & data)
{
    try{
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> componentNode = writeKeyElement(url);
            writeSimpleElement("java-type-library",
                               OUString::boolean((sal_Bool) data.javaTypeLibrary),
                               componentNode);

            writeSimpleList(
                data.implementationNames,
                "implementation-names",
                "name",
                componentNode);

            writeVectorOfPair(
                data.singletons,
                "singletons",
                "item",
                "key",
                "value",
                componentNode);

            save();
        }
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry in backend db: " +
            m_urlDb, 0, exc);
    }
}

ComponentBackendDb::Data ComponentBackendDb::getEntry(OUString const & url)
{
    try
    {
        ComponentBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);
        if (aNode.is())
        {
            bool bJava = (readSimpleElement("java-type-library", aNode) ==
                "true") ? true : false;
            retData.javaTypeLibrary = bJava;

            retData.implementationNames =
                readList( aNode, "implementation-names", "name");

            retData.singletons =
                readVectorOfPair( aNode, "singletons", "item", "key", "value");
        }
        return retData;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in backend db: " +
            m_urlDb, 0, exc);
    }
}


} // namespace bundle
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
