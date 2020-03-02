/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <osl/mutex.hxx>

static osl::Mutex aLocks[2];

// These two functions are used by dtoa.c to synchronize in multithread environment

extern "C" void ACQUIRE_DTOA_LOCK(unsigned int n)
{
    if (n < 2)
        aLocks[n].acquire();
}

extern "C" void FREE_DTOA_LOCK(unsigned int n)
{
    if (n < 2)
        aLocks[n].release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
