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
#ifdef SOLAR_JAVA
    OSL_ENSURE(pVm != 0 && nVersion >= JNI_VERSION_1_2 && pMainThreadEnv != 0,
               "bad parameter");
#endif
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
#ifndef SOLAR_JAVA
    return 0;
#else
    OSL_ENSURE(pAttached != 0, "bad parameter");
    JNIEnv * pEnv;
    jint n = m_pVm->GetEnv(reinterpret_cast< void ** >(&pEnv), m_nVersion);
    if (n != JNI_OK && n != JNI_EDETACHED) {
        OSL_FAIL("JNI: GetEnv failed");
    }
    if (pEnv == 0)
    {
        if (m_pVm->AttachCurrentThread(reinterpret_cast< void ** >(&pEnv), 0)
            != JNI_OK)
            return 0;
        *pAttached = true;
    }
    else
        *pAttached = false;
    return pEnv;
#endif
}

void VirtualMachine::detachThread() const
{
#ifdef SOLAR_JAVA
    if (m_pVm->DetachCurrentThread() != JNI_OK) {
        OSL_FAIL("JNI: DetachCurrentThread failed");
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
