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

#include "dp_extbackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/extension-registry/2010"
#define NS_PREFIX "ext"
#define ROOT_ELEMENT_NAME "extension-backend-db"
#define KEY_ELEMENT_NAME "extension"

namespace dp_registry::backend::bundle {

ExtensionBackendDb::ExtensionBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):BackendDb(xContext, url)
{

}

OUString ExtensionBackendDb::getDbNSName()
{
    return EXTENSION_REG_NS;
}

OUString ExtensionBackendDb::getNSPrefix()
{
    return NS_PREFIX;
}

OUString ExtensionBackendDb::getRootElementName()
{
    return ROOT_ELEMENT_NAME;
}

OUString ExtensionBackendDb::getKeyElementName()
{
    return KEY_ELEMENT_NAME;
}

void ExtensionBackendDb::addEntry(OUString const & url, Data const & data)
{
    try{
        //reactive revoked entry if possible.
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> extensionNodeNode = writeKeyElement(url);
            writeVectorOfPair( data.items, u"extension-items", u"item",
                u"url", u"media-type", extensionNodeNode);
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

ExtensionBackendDb::Data ExtensionBackendDb::getEntry(std::u16string_view url)
{
    try
    {
        ExtensionBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);

        if (aNode.is())
        {
            retData.items =
                readVectorOfPair( aNode, u"extension-items", u"item",
                    u"url", u"media-type");
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

} // namespace dp_registry::backend::bundle

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
