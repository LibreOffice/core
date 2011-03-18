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

#include "rtl/string.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "dp_misc.h"
#include "dp_executablebackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/executable-registry/2010"
#define NS_PREFIX "exe"
#define ROOT_ELEMENT_NAME "executable-backend-db"
#define ENTRY_NAME "executable"

namespace dp_registry {
namespace backend {
namespace executable {

ExecutableBackendDb::ExecutableBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):RegisteredDb(xContext, url)
{

}

OUString ExecutableBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString ExecutableBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString ExecutableBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

OUString ExecutableBackendDb::getKeyElementName()
{
    return OUSTR(ENTRY_NAME);
}


} // namespace executable
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
