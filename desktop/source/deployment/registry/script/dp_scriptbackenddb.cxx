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
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocument.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "dp_misc.h"
#include "dp_scriptbackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/script-registry/2010"
#define NS_PREFIX "script"
#define ROOT_ELEMENT_NAME "script-backend-db"
#define KEY_ELEMENT_NAME "script"

namespace dp_registry {
namespace backend {
namespace script {

ScriptBackendDb::ScriptBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):RegisteredDb(xContext, url)
{

}

OUString ScriptBackendDb::getDbNSName()
{
    return OUString(EXTENSION_REG_NS);
}

OUString ScriptBackendDb::getNSPrefix()
{
    return OUString(NS_PREFIX);
}

OUString ScriptBackendDb::getRootElementName()
{
    return OUString(ROOT_ELEMENT_NAME);
}

OUString ScriptBackendDb::getKeyElementName()
{
    return OUString(KEY_ELEMENT_NAME);
}



} // namespace executable
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
