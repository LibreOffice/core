/*************************************************************************
 *
 *  $RCSfile: virtualmachine.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 11:40:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    OSL_ENSURE(pVm != 0 && nVersion >= JNI_VERSION_1_2 && pMainThreadEnv != 0,
               "bad parameter");
    acquireInitialContextClassLoader(pMainThreadEnv);
}

VirtualMachine::~VirtualMachine()
{
    releaseInitialContextClassLoader();
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

void VirtualMachine::acquireInitialContextClassLoader(JNIEnv * pEnv)
{
    jclass aClass = pEnv->FindClass("java/lang/Thread");
    jmethodID aMethod1 = pEnv->GetStaticMethodID(aClass, "currentThread",
                                                 "()Ljava/lang/Thread;");
    jobject aThread = pEnv->CallStaticObjectMethod(aClass, aMethod1);
    jmethodID aMethod2 = pEnv->GetMethodID(aClass, "getContextClassLoader",
                                           "()Ljava/lang/ClassLoader;");
    jobject aClassLoader = pEnv->CallObjectMethod(aThread, aMethod2);
    OSL_ENSURE(!pEnv->ExceptionCheck(), "JNI: exception occured");
    pEnv->ExceptionClear();
    if (aClassLoader == 0)
        m_aInitialContextClassLoader = 0;
    else
    {
        m_aInitialContextClassLoader = pEnv->NewGlobalRef(aClassLoader);
        OSL_ENSURE(m_aInitialContextClassLoader != 0,
                   "JNI: NewGlobalRef failed");
    }
}

void VirtualMachine::releaseInitialContextClassLoader() const
{
    if (m_aInitialContextClassLoader != 0)
    {
        JNIEnv * pEnv;
        jint n = m_pVm->AttachCurrentThread(reinterpret_cast< void ** >(&pEnv),
                                            0);
        OSL_ENSURE(n == JNI_OK, "JNI: AttachCurrentThread failed");
        if (n == JNI_OK)
        {
            pEnv->DeleteGlobalRef(m_aInitialContextClassLoader);
            n = m_pVm->DetachCurrentThread();
            OSL_ENSURE(n == JNI_OK, "JNI: DetachCurrentThread failed");
        }
    }
}

JNIEnv * VirtualMachine::attachThread(bool * pAttached) const
{
    OSL_ENSURE(pAttached != 0, "bad parameter");
    JNIEnv * pEnv;
    jint n = m_pVm->GetEnv(reinterpret_cast< void ** >(&pEnv), m_nVersion);
    OSL_ENSURE(n == JNI_OK || n == JNI_EDETACHED, "JNI: GetEnv failed");
    if (pEnv == 0)
    {
        if (m_pVm->AttachCurrentThread(reinterpret_cast< void ** >(&pEnv), 0)
            != JNI_OK)
            return 0;
        if (m_aInitialContextClassLoader != 0)
        {
            jclass aClass = pEnv->FindClass("java/lang/Thread");
            jmethodID aMethod1 = pEnv->GetStaticMethodID(
                aClass, "currentThread", "()Ljava/lang/Thread;");
            jobject aThread = pEnv->CallStaticObjectMethod(aClass, aMethod1);
            jmethodID aMethod2 = pEnv->GetMethodID(
                aClass, "setContextClassLoader", "(Ljava/lang/ClassLoader;)V");
            pEnv->CallVoidMethod(aThread, aMethod2,
                                 m_aInitialContextClassLoader);
            OSL_ENSURE(!pEnv->ExceptionCheck(), "JNI: exception occured");
            pEnv->ExceptionClear();
        }
        *pAttached = true;
    }
    else
        *pAttached = false;
    return pEnv;
}

void VirtualMachine::detachThread() const
{
    jint n = m_pVm->DetachCurrentThread();
    OSL_ENSURE(n == JNI_OK, "JNI: DetachCurrentThread failed");
}
