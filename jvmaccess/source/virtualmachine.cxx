/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        OSL_ENSURE(false, "JNI: GetEnv failed");
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
        OSL_ENSURE(false, "JNI: DetachCurrentThread failed");
    }
#endif
}
