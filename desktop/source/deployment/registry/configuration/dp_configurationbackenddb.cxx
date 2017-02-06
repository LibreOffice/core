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

#include "dp_configurationbackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/configuration-registry/2010"
#define NS_PREFIX "conf"
#define ROOT_ELEMENT_NAME "configuration-backend-db"
#define KEY_ELEMENT_NAME "configuration"

namespace dp_registry {
namespace backend {
namespace configuration {

ConfigurationBackendDb::ConfigurationBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):BackendDb(xContext, url)
{

}

OUString ConfigurationBackendDb::getDbNSName()
{
    return OUString(EXTENSION_REG_NS);
}

OUString ConfigurationBackendDb::getNSPrefix()
{
    return OUString(NS_PREFIX);
}

OUString ConfigurationBackendDb::getRootElementName()
{
    return OUString(ROOT_ELEMENT_NAME);
}

OUString ConfigurationBackendDb::getKeyElementName()
{
    return OUString(KEY_ELEMENT_NAME);
}


void ConfigurationBackendDb::addEntry(OUString const & url, Data const & data)
{
    try{
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> helpNode
                = writeKeyElement(url);

            writeSimpleElement("data-url", data.dataUrl, helpNode);
            writeSimpleElement("ini-entry", data.iniEntry, helpNode);
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
            "Extension Manager: failed to write data entry in configuration backend db: " +
            m_urlDb, nullptr, exc);
    }
}


::boost::optional<ConfigurationBackendDb::Data>
ConfigurationBackendDb::getEntry(OUString const & url)
{
    try
    {
        ConfigurationBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);
        if (aNode.is())
        {
            retData.dataUrl = readSimpleElement("data-url", aNode);
            retData.iniEntry = readSimpleElement("ini-entry", aNode);
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
            "Extension Manager: failed to read data entry in configuration backend db: " +
            m_urlDb, nullptr, exc);
    }
}

::std::list<OUString> ConfigurationBackendDb::getAllDataUrls()
{
    try
    {
        ::std::list<OUString> listRet;
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const OUString sPrefix = getNSPrefix();
        OUString sExpression(
            sPrefix + ":configuration/" + sPrefix + ":data-url/text()");
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
    catch ( const css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(const css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            "Extension Manager: failed to read data entry in configuration backend db: " +
            m_urlDb, nullptr, exc);
    }
}

} // namespace configuration
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
