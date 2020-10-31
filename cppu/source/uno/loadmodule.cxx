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


#include <sal/config.h>

#include <cassert>

#include <osl/module.h>
#include <osl/module.hxx>
#include <rtl/malformeduriexception.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include "loadmodule.hxx"

namespace cppu::detail {

#ifndef DISABLE_DYNLOADING

bool loadModule(osl::Module& rModule, OUString const & name) {
    static OUString base = [] {
            OUString url;
            if (!osl::Module::getUrlFromAddress(
                    reinterpret_cast<oslGenericFunction>(&loadModule), url))
            {
                SAL_WARN("cppu", "osl::Module::getUrlFromAddress failed");
                return OUString();
            }
            assert(!url.isEmpty());
            return url;
        }();
    if (base.isEmpty()) {
        SAL_INFO("cppu", "osl::Module::getUrlFromAddress had failed");
        return false;
    }
    OUString b =
#if defined SAL_DLLPREFIX
        SAL_DLLPREFIX +
#endif
        name +
        SAL_DLLEXTENSION;
    try {
        b = rtl::Uri::convertRelToAbs(base, b);
    } catch (rtl::MalformedUriException & e) {
        SAL_INFO("cppu", "rtl::MalformedUriException <" << e.getMessage() << ">");
        return false;
    }
    return rModule.load(
        b,
        SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY);
}

#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
