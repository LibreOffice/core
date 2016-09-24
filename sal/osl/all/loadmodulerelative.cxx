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

#include "sal/config.h"

#include <cstddef>

#include "sal/log.hxx"
#include "osl/module.h"
#include "osl/module.hxx"
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
    rtl::OUString base;
    if (!osl::Module::getUrlFromAddress(baseModule, base)) {
        SAL_INFO("sal.osl","osl::Module::getUrlFromAddress failed");
        return nullptr;
    }
    rtl::OUString abs;
    try {
        abs = rtl::Uri::convertRelToAbs(base, relativePath);
    } catch (const rtl::MalformedUriException & e) {
        (void) e; // avoid warnings
        SAL_INFO("sal.osl",
            "rtl::MalformedUriException <" << e.getMessage() << ">");
            //TODO: let some OSL_TRACE variant take care of text conversion?
        return nullptr;
    }
    return osl_loadModule(abs.pData, mode);
}

#endif // !DISABLE_DYNLOADING

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
