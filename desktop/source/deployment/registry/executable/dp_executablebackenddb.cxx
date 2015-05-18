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
#include <com/sun/star/uno/XComponentContext.hpp>
#include "dp_misc.h"
#include "dp_executablebackenddb.hxx"


using namespace ::com::sun::star::uno;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/executable-registry/2010"
#define NS_PREFIX "exe"
#define ROOT_ELEMENT_NAME "executable-backend-db"
#define ENTRY_NAME "executable"

namespace dp_registry {
namespace backend {
namespace executable {

ExecutableBackendDb::ExecutableBackendDb(
    Reference<XComponentContext> const &  xContext,
    OUString const & url):RegisteredDb(xContext, url)
{

}

OUString ExecutableBackendDb::getDbNSName()
{
    return OUString(EXTENSION_REG_NS);
}

OUString ExecutableBackendDb::getNSPrefix()
{
    return OUString(NS_PREFIX);
}

OUString ExecutableBackendDb::getRootElementName()
{
    return OUString(ROOT_ELEMENT_NAME);
}

OUString ExecutableBackendDb::getKeyElementName()
{
    return OUString(ENTRY_NAME);
}


}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
