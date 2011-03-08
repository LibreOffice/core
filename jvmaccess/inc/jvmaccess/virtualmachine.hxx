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

#if !defined INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
#define INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX

#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#ifdef SOLAR_JAVA
#include "jni.h"
#else
struct JNIEnv;
struct JavaVM;
typedef int jint;
typedef void * jobject;
#endif

namespace jvmaccess {

/** An encapsulating wrapper around a Java virtual machine.
 */
class VirtualMachine: public salhelper::SimpleReferenceObject
{
public:
    /** A helper to attach a thread to a Java virtual machine.

        @descr
        Upon construction of a guard the current thread is attached to the
        virtual machine, and upon destruction of the guard the thread is
        detached again.  For any one thread, multiple instances of this class
        may be used in a stack-like fashion (care is taken to only really
        detach the thread from the virtual machine upon destruction of the guard
        at the bottom of the stack).
     */
    class AttachGuard
    {
    public:
        /** An exception indicating failure to create an AttachGuard.
         */
        class CreationException
        {
        public:
            CreationException();

            CreationException(CreationException const &);

            virtual ~CreationException();

            CreationException & operator =(CreationException const &);
        };

        /** Attach the current thread to a virtual machine.

            @param rMachine
            The virtual machine to attach to.  Must not be a null reference.

            @exception CreationException
            Thrown in case attaching fails (due to a JNI problem).
         */
        explicit AttachGuard(rtl::Reference< VirtualMachine > const & rMachine);

        /** Detach the current thread from the virtual machine again.
         */
        ~AttachGuard();

        /** Get a JNI environment pointer for the current thread.

            @return
            A valid JNI environment pointer.  Will never be null.
         */
        inline JNIEnv * getEnvironment() const { return m_pEnvironment; }

    private:
        AttachGuard(AttachGuard &); // not implemented
        void operator =(AttachGuard); // not implemented

        rtl::Reference< VirtualMachine > m_xMachine;
        JNIEnv * m_pEnvironment;
        bool m_bDetach;
    };

    /** Create a wrapper around a Java virtual machine.

        @param pVm
        A JNI pointer to virtual machine.  Must not be null.

        @param nVersion
        The JNI version of the virtual machine pointed to by pVm.  Must be at
        least JNI_VERSION_1_2.  This parameter should be of type jint, not int,
        but at least on some platforms the definition of jint changed from
        JDK 1.3 (long) to JDK 1.4 (int), so that the mangled C++ name of the
        constructor would depend on the JDK version used at compile time.

        @param bDestroy
        Whether to destroy the virtual machine when destructing the wrapper
        (i.e., whether the wrapper owns the virtual machine pointed to by pVm).

        @param pMainThreadEnv
        A valid JNI environment pointer for the current thread; must not be
        null.  The current thread must be "initially attached" to the virtual
        machine while this constructor is being called (i.e., it must be the
        thread that has called JNI_CreateJavaVM in case the virtual machine has
        been started via the JNI Invocation API, and it must not already have
        called DetachCurrentThread; or it must be executing native code called
        from a "primordial" virtual machine).  This environment pointer was
        formerly used to obtain a reference to the thread's current context
        class loader (java.lang.Thread.getCurrentClassLoader; if later a native
        thread was attached to the virtual machine, that thread's context class
        loader would be null, so the AttachGuard first of all set it to the
        saved value; this feature has been removed again for performance reasons
        and because the default context class loader is often not useful, so
        that code relying on a context class loader has to set one explicitly,
        anyway).  This parameter is currently unused (but may be used again in
        the future).
     */
    VirtualMachine(JavaVM * pVm, int nVersion, bool bDestroy,
                   JNIEnv * pMainThreadEnv);

private:
    VirtualMachine(VirtualMachine &); // not implemented
    void operator =(VirtualMachine); // not implemented

    virtual ~VirtualMachine();

    JNIEnv * attachThread(bool * pAttached) const;

    void detachThread() const;

    JavaVM * m_pVm;
    jint m_nVersion;
    bool m_bDestroy;

    friend class AttachGuard; // to access attachThread, detachThread
};

}

#endif // INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
