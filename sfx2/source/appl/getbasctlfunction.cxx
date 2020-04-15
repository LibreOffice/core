/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_features.h>
#include <config_options.h>
#include <osl/module.h>
#include <osl/module.hxx>
#include <tools/svlibrary.h>

#include "getbasctlfunction.hxx"

#if HAVE_FEATURE_SCRIPTING
#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

oslGenericFunction sfx2::getBasctlFunction(char const* name)
{
    osl::Module aMod;

    // load basctl module
    auto const ok = aMod.loadRelative(
        &thisModule,
#if ENABLE_MERGELIBS
        SVLIBRARY("merged")
#else
        SVLIBRARY("basctl")
#endif
        );
    assert(ok);
    (void) ok;

    // get symbol
    auto pSymbol = aMod.getFunctionSymbol(name);
    assert(pSymbol);
    aMod.release();

    return pSymbol;
}

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
