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

#include "jvmaccess/virtualmachine.hxx"

#include "osl/diagnose.h"

using jvmaccess::VirtualMachine;

VirtualMachine::AttachGuard::CreationException::CreationException()
{}

VirtualMachine::AttachGuard::CreationException::CreationException(
    CreationException const &)
{}

VirtualMachine::AttachGuard::CreationException::~CreationException()
{}

VirtualMachine::AttachGuard::CreationException &
VirtualMachine::AttachGuard::CreationException::operator =(
    CreationException const &)
{
    return *this;
}

VirtualMachine::AttachGuard::AttachGuard(
    rtl::Reference< VirtualMachine > const & rMachine):
    m_xMachine(rMachine)
{
    OSL_ENSURE(m_xMachine.is(), "bad parameter");
    m_pEnvironment = m_xMachine->attachThread(&m_bDetach);
    if (m_pEnvironment == 0)
        throw CreationException();
}

VirtualMachine::AttachGuard::~AttachGuard()
{
    if (m_bDetach)
        m_xMachine->detachThread();
}

VirtualMachine::VirtualMachine(JavaVM * pVm, int nVersion, bool bDestroy,
                               JNIEnv * pMainThreadEnv):
    m_pVm(pVm), m_nVersion(nVersion), m_bDestroy(bDestroy)
{
    (void) pMainThreadEnv; // avoid warnings
    OSL_ENSURE(pVm != 0 && nVersion >= JNI_VERSION_1_2 && pMainThreadEnv != 0,
               "bad parameter");
}

VirtualMachine::~VirtualMachine()
{
    if (m_bDestroy)
    {
        // Do not destroy the VM.  Under Java 1.3, the AWT event loop thread is
        // not a daemon thread and is never terminated, so that calling
        // DestroyJavaVM (waiting for all non-daemon threads to terminate) hangs
        // forever.
/*
        jint n = m_pVm->DestroyJavaVM();
        OSL_ENSURE(n == JNI_OK, "JNI: DestroyJavaVM failed");
*/
    }
}

JNIEnv * VirtualMachine::attachThread(bool * pAttached) const
{
    OSL_ENSURE(pAttached != 0, "bad parameter");
    JNIEnv * pEnv;
    jint n = m_pVm->GetEnv(reinterpret_cast< void ** >(&pEnv), m_nVersion);
    if (n != JNI_OK && n != JNI_EDETACHED) {
        OSL_FAIL("JNI: GetEnv failed");
    }
    if (pEnv == 0)
    {
        if (m_pVm->AttachCurrentThread
            (
#ifndef ANDROID
             reinterpret_cast< void ** >(&pEnv),
#else
             // The Android <jni.h> has AttachCurrentThread() taking a
             // JNIEnv** and not void **
             &pEnv,
#endif
             0)
            != JNI_OK)
            return 0;
        *pAttached = true;
    }
    else
        *pAttached = false;
    return pEnv;
}

void VirtualMachine::detachThread() const
{
    if (m_pVm->DetachCurrentThread() != JNI_OK) {
        OSL_FAIL("JNI: DetachCurrentThread failed");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
