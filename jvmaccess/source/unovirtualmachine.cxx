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

#include "sal/config.h"

#include "jvmaccess/unovirtualmachine.hxx"

#include "jvmaccess/virtualmachine.hxx"
#include "sal/log.hxx"

#include "jni.h"

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
    try {
        m_classLoader =
            jvmaccess::VirtualMachine::AttachGuard(m_virtualMachine).
            getEnvironment()->NewGlobalRef(static_cast< jobject >(classLoader));
    } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {}
    if (m_classLoader == 0) {
        throw CreationException();
    }
}

rtl::Reference< jvmaccess::VirtualMachine >
UnoVirtualMachine::getVirtualMachine() const {
    return m_virtualMachine;
}

UnoVirtualMachine::~UnoVirtualMachine() {
    try {
        jvmaccess::VirtualMachine::AttachGuard(m_virtualMachine).
            getEnvironment()->DeleteGlobalRef(
                static_cast< jobject >(m_classLoader));
    } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {
        SAL_INFO(
            "jvmaccess",
            "ignored"
                " jvmaccess::VirtualMachine::AttachGuard::CreationException");
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
