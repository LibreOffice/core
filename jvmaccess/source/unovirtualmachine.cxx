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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
