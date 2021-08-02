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
#include "dp_executablebackenddb.hxx"


using namespace ::com::sun::star::uno;

constexpr OUStringLiteral EXTENSION_REG_NS = u"http://openoffice.org/extensionmanager/executable-registry/2010";
constexpr OUStringLiteral NS_PREFIX = u"exe";
constexpr OUStringLiteral ROOT_ELEMENT_NAME = u"executable-backend-db";
constexpr OUStringLiteral ENTRY_NAME = u"executable";

namespace dp_registry::backend::executable {

ExecutableBackendDb::ExecutableBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):RegisteredDb(xContext, url)
{

}

OUString ExecutableBackendDb::getDbNSName()
{
    return EXTENSION_REG_NS;
}

OUString ExecutableBackendDb::getNSPrefix()
{
    return NS_PREFIX;
}

OUString ExecutableBackendDb::getRootElementName()
{
    return ROOT_ELEMENT_NAME;
}

OUString ExecutableBackendDb::getKeyElementName()
{
    return ENTRY_NAME;
}


} // namespace dp_registry::backend::executable

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
