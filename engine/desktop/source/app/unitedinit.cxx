/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <osl/module.h>
#include <config_options.h>

#if !ENABLE_UNITEDLIBS
#error(this should only be compiled when in ENABLE_UNITEDLIBS)
#endif

#if defined(LINUX)

// This function runs automatically when the library is loaded.
// [-loplugin:external] [-loplugin:unreffun]
void __attribute__((constructor)) united_library_init(void)
{
    osl_setUnitedLibsActive();
}

#elif defined(_WIN32)

// on Windows, we would normally use a DllMain, but we already
// have one of those in sal/osl/w32/dllentry.cxx,
// so use a more complicated approach that hooks into the CRT.

void UnitedLibsInit()
{
    osl_setUnitedLibsActive();
}

// Register the function pointer into the CRT initialization segment
#pragma section(".CRT$XCU", read)
__declspec(allocate(".CRT$XCU")) void (*pMyLibraryInit)() = UnitedLibsInit;

#else

#error(no support for --enable-unitedlilbs on this platform yet)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
