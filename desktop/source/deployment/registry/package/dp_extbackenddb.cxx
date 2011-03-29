/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#define KEY_ELEMENT_NAME "extension"

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

OUString ExtensionBackendDb::getKeyElementName()
{
    return OUSTR(KEY_ELEMENT_NAME);
}

void ExtensionBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{
        //reactive revoked entry if possible.
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> extensionNodeNode = writeKeyElement(url);
            writeVectorOfPair(
                data.items,
                OUSTR("extension-items"),
                OUSTR("item"),
                OUSTR("url"),
                OUSTR("media-type"),
                extensionNodeNode);
            save();
        }
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

ExtensionBackendDb::Data ExtensionBackendDb::getEntry(::rtl::OUString const & url)
{
    try
    {
        ExtensionBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);

        if (aNode.is())
        {
            retData.items =
                readVectorOfPair(
                    aNode,
                    OUSTR("extension-items"),
                    OUSTR("item"),
                    OUSTR("url"),
                    OUSTR("media-type"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
