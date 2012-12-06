/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cstdlib>

#include "sal/types.h"

// Stubs for removed functionality, to be killed when we bump sal SONAME

extern "C" {

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL osl_acquireSemaphore(void *) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL osl_createSemaphore(sal_uInt32) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL osl_destroySemaphore(void *) {
    std::abort();
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL osl_releaseSemaphore(void *) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL osl_tryToAcquireSemaphore(void *) {
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
