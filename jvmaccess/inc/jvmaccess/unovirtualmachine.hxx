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

#ifndef INCLUDED_JVMACCESS_UNOVIRTUALMACHINE_HXX
#define INCLUDED_JVMACCESS_UNOVIRTUALMACHINE_HXX

#include "sal/config.h"
#include "salhelper/simplereferenceobject.hxx"
#include "rtl/ref.hxx"

namespace jvmaccess {

class VirtualMachine;

/** An encapsulating wrapper around a Java virtual machine and an appropriate
    UNO class loader.
 */
class UnoVirtualMachine: public salhelper::SimpleReferenceObject {
public:
    /** An exception indicating failure to create a UnoVirtualMachine.
     */
    class CreationException
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
    rtl::Reference< jvmaccess::VirtualMachine > getVirtualMachine() const;

    /** Get the UNO class loader.

        @return
        A global JNI reference to the UNO class loader.  (The JNI reference must
        not be deleted by client code.)  Will never be null.  This should be of
        type jobject, not void *, but the exact definition of jobject is
        different for different JDK versions, so that the mangled C++ name of
        the function would depend on the JDK version used at compile time.
     */
    void * getClassLoader() const;

private:
    UnoVirtualMachine(UnoVirtualMachine &); // not defined
    void operator =(UnoVirtualMachine &); // not defined

    virtual ~UnoVirtualMachine();

    rtl::Reference< jvmaccess::VirtualMachine > m_virtualMachine;
    void * m_classLoader;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
