/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unovirtualmachine.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:22:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_JVMACCESS_UNOVIRTUALMACHINE_HXX
#define INCLUDED_JVMACCESS_UNOVIRTUALMACHINE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include "salhelper/simplereferenceobject.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif

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
