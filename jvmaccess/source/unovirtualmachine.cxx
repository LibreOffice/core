/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unovirtualmachine.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:23:03 $
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

#include "sal/config.h"

#include "jvmaccess/unovirtualmachine.hxx"

#include "osl/diagnose.h"

#include "jvmaccess/virtualmachine.hxx"

#if defined SOLAR_JAVA
#include "jni.h"
#endif

namespace jvmaccess {

UnoVirtualMachine::CreationException::CreationException() {}

UnoVirtualMachine::CreationException::CreationException(
    CreationException const &)
{}

UnoVirtualMachine::CreationException::~CreationException() {}

UnoVirtualMachine::CreationException &
UnoVirtualMachine::CreationException::operator =(CreationException const &) {
    return *this;
}

UnoVirtualMachine::UnoVirtualMachine(
    rtl::Reference< jvmaccess::VirtualMachine > const & virtualMachine,
    void * classLoader):
    m_virtualMachine(virtualMachine),
    m_classLoader(0)
{
#if defined SOLAR_JAVA
    try {
        m_classLoader =
            jvmaccess::VirtualMachine::AttachGuard(m_virtualMachine).
            getEnvironment()->NewGlobalRef(static_cast< jobject >(classLoader));
    } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {}
#endif
    if (m_classLoader == 0) {
        throw CreationException();
    }
}

rtl::Reference< jvmaccess::VirtualMachine >
UnoVirtualMachine::getVirtualMachine() const {
    return m_virtualMachine;
}

void * UnoVirtualMachine::getClassLoader() const {
    return m_classLoader;
}

UnoVirtualMachine::~UnoVirtualMachine() {
#if defined SOLAR_JAVA
    try {
        jvmaccess::VirtualMachine::AttachGuard(m_virtualMachine).
            getEnvironment()->DeleteGlobalRef(
                static_cast< jobject >(m_classLoader));
    } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {
        OSL_TRACE(
            "jvmaccess::UnoVirtualMachine::~UnoVirtualMachine:"
            " jvmaccess::VirtualMachine::AttachGuard::CreationException" );
    }
#endif
}

}
