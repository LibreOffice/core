/*************************************************************************
 *
 *  $RCSfile: virtualmachine.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:41:13 $
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

#if !defined INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
#define INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX

#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "jni.h"

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
            The virtual machine to attach to.

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
        from a "primordial" virtual machine).  This environment pointer is used
        to obtain a reference to the thread's current context class loader
        (java.lang.Thread.getCurrentClassLoader).  If later a native thread is
        attached to the virtual machine, that thread's context class loader
        would be null, so the AttachGuard first of all sets it to the saved
        value.  In a nutshell, this means that all native threads attached to
        the virtual machine use the context class loader of the "initial Java
        thread."
     */
    VirtualMachine(JavaVM * pVm, int nVersion, bool bDestroy,
                   JNIEnv * pMainThreadEnv);

private:
    VirtualMachine(VirtualMachine &); // not implemented
    void operator =(VirtualMachine); // not implemented

    virtual ~VirtualMachine();

    void acquireInitialContextClassLoader(JNIEnv * pEnv);

    void releaseInitialContextClassLoader() const;

    JNIEnv * attachThread(bool * pAttached) const;

    void detachThread() const;

    JavaVM * m_pVm;
    jint m_nVersion;
    bool m_bDestroy;
    jobject m_aInitialContextClassLoader;

    friend class AttachGuard; // to access attachThread, detachThread
};

}

#endif // INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
