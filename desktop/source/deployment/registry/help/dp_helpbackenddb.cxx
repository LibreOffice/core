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
#include <rtl/bootstrap.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include "dp_misc.h"

#include "dp_helpbackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/help-registry/2010"
#define NS_PREFIX "help"
#define ROOT_ELEMENT_NAME "help-backend-db"
#define KEY_ELEMENT_NAME "help"

namespace dp_registry {
namespace backend {
namespace help {

HelpBackendDb::HelpBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):BackendDb(xContext, url)
{

}

OUString HelpBackendDb::getDbNSName()
{
    return OUString(EXTENSION_REG_NS);
}

OUString HelpBackendDb::getNSPrefix()
{
    return OUString(NS_PREFIX);
}

OUString HelpBackendDb::getRootElementName()
{
    return OUString(ROOT_ELEMENT_NAME);
}

OUString HelpBackendDb::getKeyElementName()
{
    return OUString(KEY_ELEMENT_NAME);
}


void HelpBackendDb::addEntry(OUString const & url, Data const & data)
{
    try{
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> helpNode
                = writeKeyElement(url);

            writeSimpleElement("data-url", data.dataUrl, helpNode);
            save();
        }
    }
    catch ( const css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to write data entry in help backend db: " + m_urlDb, nullptr, exc);
    }
}


::boost::optional<HelpBackendDb::Data>
HelpBackendDb::getEntry(OUString const & url)
{
    try
    {
        HelpBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);
        if (aNode.is())
        {
            retData.dataUrl = readSimpleElement("data-url", aNode);
        }
        else
        {
            return ::boost::optional<Data>();
        }
        return ::boost::optional<Data>(retData);
    }
    catch ( const css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in help backend db: " + m_urlDb, nullptr, exc);
    }
}

::std::list<OUString> HelpBackendDb::getAllDataUrls()
{
    return getOneChildFromAllEntries("data-url");
}

} // namespace help
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
