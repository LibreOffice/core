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


#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "dp_compbackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/component-registry/2010"
#define NS_PREFIX "comp"
#define ROOT_ELEMENT_NAME "component-backend-db"
#define KEY_ELEMENT_NAME "component"

namespace dp_registry::backend::component {

ComponentBackendDb::ComponentBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):BackendDb(xContext, url)
{

}

OUString ComponentBackendDb::getDbNSName()
{
    return EXTENSION_REG_NS;
}

OUString ComponentBackendDb::getNSPrefix()
{
    return NS_PREFIX;
}

OUString ComponentBackendDb::getRootElementName()
{
    return ROOT_ELEMENT_NAME;
}

OUString ComponentBackendDb::getKeyElementName()
{
    return KEY_ELEMENT_NAME;
}

void ComponentBackendDb::addEntry(OUString const & url, Data const & data)
{
    try{
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> componentNode = writeKeyElement(url);
            writeSimpleElement(u"java-type-library",
                               OUString::boolean(data.javaTypeLibrary),
                               componentNode);

            writeSimpleList(
                data.implementationNames,
                u"implementation-names",
                u"name",
                componentNode);

            writeVectorOfPair(
                data.singletons,
                u"singletons",
                u"item",
                u"key",
                u"value",
                componentNode);

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

ComponentBackendDb::Data ComponentBackendDb::getEntry(std::u16string_view url)
{
    try
    {
        ComponentBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);
        if (aNode.is())
        {
            bool bJava = readSimpleElement(u"java-type-library", aNode) == "true";
            retData.javaTypeLibrary = bJava;

            retData.implementationNames =
                readList( aNode, u"implementation-names", u"name");

            retData.singletons =
                readVectorOfPair( aNode, u"singletons", u"item", u"key", u"value");
        }
        return retData;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in backend db: " +
            m_urlDb, nullptr, exc);
    }
}


} // namespace dp_registry::backend::component

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
