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


#include <com/sun/star/uno/XComponentContext.hpp>
#include "dp_scriptbackenddb.hxx"


using namespace ::com::sun::star::uno;

constexpr OUStringLiteral EXTENSION_REG_NS = u"http://openoffice.org/extensionmanager/script-registry/2010";
constexpr OUStringLiteral NS_PREFIX = u"script";
constexpr OUStringLiteral ROOT_ELEMENT_NAME = u"script-backend-db";
constexpr OUStringLiteral KEY_ELEMENT_NAME = u"script";

namespace dp_registry::backend::script {

ScriptBackendDb::ScriptBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):RegisteredDb(xContext, url)
{

}

OUString ScriptBackendDb::getDbNSName()
{
    return EXTENSION_REG_NS;
}

OUString ScriptBackendDb::getNSPrefix()
{
    return NS_PREFIX;
}

OUString ScriptBackendDb::getRootElementName()
{
    return ROOT_ELEMENT_NAME;
}

OUString ScriptBackendDb::getKeyElementName()
{
    return KEY_ELEMENT_NAME;
}


} // namespace dp_registry::backend::script

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
