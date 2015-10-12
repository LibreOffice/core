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

#ifndef INCLUDED_JVMACCESS_UNOVIRTUALMACHINE_HXX
#define INCLUDED_JVMACCESS_UNOVIRTUALMACHINE_HXX

#include <jvmaccess/jvmaccessdllapi.h>
#include <sal/config.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>

namespace jvmaccess {

class VirtualMachine;

/** An encapsulating wrapper around a Java virtual machine and an appropriate
    UNO class loader.
 */
class JVMACCESS_DLLPUBLIC UnoVirtualMachine: public salhelper::SimpleReferenceObject {
public:
    /** An exception indicating failure to create a UnoVirtualMachine.
     */
    class JVMACCESS_DLLPUBLIC CreationException
    {
    public:
        CreationException();

        CreationException(CreationException const &);

        virtual ~CreationException();

        CreationException & operator =(CreationException const &);
    };

    /** Create a wrapper around a Java virtual machine and an appropriate UNO
        class loader.

        @param virtualMachine
        A Java virtual machine wrapper.  Must not be null.

        @param classLoader
        A local or global JNI reference, relative to the given virtualMachine,
        to an appropriate UNO class loader instance.  Must not be null.  This
        parameter should be of type jobject, not void *, but the exact
        definition of jobject is different for different JDK versions, so that
        the mangled C++ name of the constructor would depend on the JDK version
        used at compile time.

        @exception CreationException
        Thrown in case creation fails (due to a JNI problem).
     */
    UnoVirtualMachine(
        rtl::Reference< jvmaccess::VirtualMachine > const & virtualMachine,
        void * classLoader);

    /** Get the Java virtual machine wrapper.

        @return
        The Java virtual machine wrapper.  Will never be null.
     */
    rtl::Reference< jvmaccess::VirtualMachine > getVirtualMachine() const { return m_virtualMachine;}

    /** Get the UNO class loader.

        @return
        A global JNI reference to the UNO class loader.  (The JNI reference must
        not be deleted by client code.)  Will never be null.  This should be of
        type jobject, not void *, but the exact definition of jobject is
        different for different JDK versions, so that the mangled C++ name of
        the function would depend on the JDK version used at compile time.
     */
    void * getClassLoader() const { return m_classLoader;}

private:
    UnoVirtualMachine(UnoVirtualMachine &) = delete;
    void operator =(UnoVirtualMachine &) = delete;

    virtual ~UnoVirtualMachine();

    rtl::Reference< jvmaccess::VirtualMachine > m_virtualMachine;
    void * m_classLoader;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
