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

#include "osl/module.h"
#include "osl/time.h"
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

SAL_DLLPUBLIC_EXPORT sal_Int32 SAL_CALL rtl_addUnloadingListener(
    void (SAL_CALL *)(void *), void *)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT sal_Int32 SAL_CALL rtl_compareMemory(
    void const *, void const *, sal_Size)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_copyMemory(
    void *, void const *, sal_Size)
{
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_fillMemory(void *, sal_Size, sal_uInt8) {
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL rtl_findInMemory(
    void const *, sal_uInt8, sal_Size)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_moveMemory(
    void *, void const *, sal_Size)
{
    std::abort();
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL rtl_registerModuleForUnloading(oslModule)
{
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_removeUnloadingListener(sal_Int32) {
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_unloadUnusedModules(TimeValue *) {
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_unregisterModuleForUnloading(oslModule) {
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL rtl_zeroMemory(void *, sal_Size) {
    std::abort();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
