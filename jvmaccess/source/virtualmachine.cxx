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

#include <jvmaccess/virtualmachine.hxx>
#include <sal/log.hxx>

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
    assert(m_xMachine.is());
    m_pEnvironment = m_xMachine->attachThread(&m_bDetach);
    if (m_pEnvironment == nullptr)
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
    assert(pVm != nullptr);
    assert(nVersion >= JNI_VERSION_1_2);
    assert(pMainThreadEnv);
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
        SAL_WARN_IF(n != JNI_OK, "jvmaccess", "JNI: DestroyJavaVM failed");
*/
    }
}

JNIEnv * VirtualMachine::attachThread(bool * pAttached) const
{
    assert(pAttached != nullptr && "bad parameter");
    JNIEnv * pEnv;
    jint n = m_pVm->GetEnv(reinterpret_cast< void ** >(&pEnv), m_nVersion);
    SAL_WARN_IF(
        n != JNI_OK && n != JNI_EDETACHED, "jvmaccess", "JNI: GetEnv failed");
    if (pEnv == nullptr)
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
             nullptr)
            != JNI_OK)
            return nullptr;
        *pAttached = true;
    }
    else
        *pAttached = false;
    return pEnv;
}

void VirtualMachine::detachThread() const
{
    jint n = m_pVm->DetachCurrentThread();
    SAL_WARN_IF(n != JNI_OK, "jvmaccess", "JNI: DetachCurrentThread failed");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
