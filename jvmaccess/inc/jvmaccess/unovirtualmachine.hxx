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
