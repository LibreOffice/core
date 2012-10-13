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


#include "sal/config.h"

#include <cstddef>

#include "osl/diagnose.h"
#include "osl/module.h"
#include "osl/module.hxx"
#include "osl/thread.h"
#include "rtl/malformeduriexception.hxx"
#include "rtl/uri.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

extern "C" {

#ifndef DISABLE_DYNLOADING

oslModule SAL_CALL osl_loadModuleRelative(
    oslGenericFunction const baseModule, rtl_uString * const relativePath,
    sal_Int32 const mode)
{
    ::rtl::OUString base;
    if (!::osl::Module::getUrlFromAddress(baseModule, base)) {
        OSL_TRACE("osl::Module::getUrlFromAddress failed");
        return NULL;
    }
    ::rtl::OUString abs;
    try {
        abs = ::rtl::Uri::convertRelToAbs(base, relativePath);
    } catch (const ::rtl::MalformedUriException & e) {
        (void) e; // avoid warnings
        OSL_TRACE(
            "rtl::MalformedUriException <%s>",
            rtl::OUStringToOString(e.getMessage(), osl_getThreadTextEncoding()).
                getStr());
            //TODO: let some OSL_TRACE variant take care of text conversion?
        return NULL;
    }
    return ::osl_loadModule(abs.pData, mode);
}

#endif // !DISABLE_DYNLOADING

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
