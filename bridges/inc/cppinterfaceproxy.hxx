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

#pragma once

#include <sal/config.h>

#include <atomic>
#include <cstddef>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <typelib/typedescription.h>
#include <uno/dispatcher.h>
#include <uno/environment.h>
#include "vtablefactory.hxx"

namespace com::sun::star::uno { class XInterface; }

namespace bridges::cpp_uno::shared {

class Bridge;

extern "C" void freeCppInterfaceProxy(
    uno_ExtEnvironment * pEnv, void * pInterface);

/**
 * A cpp proxy wrapping a uno interface.
 */
class CppInterfaceProxy {
public:
    // Interface for Bridge:

    static com::sun::star::uno::XInterface * create(
        Bridge * pBridge, uno_Interface * pUnoI,
        typelib_InterfaceTypeDescription * pTypeDescr,
        OUString const & rOId);

    // Interface for individual CPP--UNO bridges:

    Bridge * getBridge() { return pBridge; }
    uno_Interface * getUnoI() { return pUnoI; }
    typelib_InterfaceTypeDescription * getTypeDescr() { return pTypeDescr; }
    const OUString& getOid() const { return oid; }

    // non virtual methods called on incoming vtable calls #1, #2
    void acquireProxy();
    void releaseProxy();

    static CppInterfaceProxy * castInterfaceToProxy(void * pInterface);

private:
    CppInterfaceProxy(CppInterfaceProxy const &) = delete;
    CppInterfaceProxy& operator =(const CppInterfaceProxy&) = delete;

    CppInterfaceProxy(
        Bridge * pBridge_, uno_Interface * pUnoI_,
        typelib_InterfaceTypeDescription * pTypeDescr_,
        OUString const & rOId_);

    ~CppInterfaceProxy();

    static com::sun::star::uno::XInterface * castProxyToInterface(
        CppInterfaceProxy * pProxy);

    std::atomic<std::size_t> nRef;
    Bridge * pBridge;

    // mapping information
    uno_Interface * pUnoI; // wrapped interface
    typelib_InterfaceTypeDescription * pTypeDescr;
    OUString oid;

    VtableFactory::Slot * vtables[1] = {};

    friend void freeCppInterfaceProxy(
        uno_ExtEnvironment * pEnv, void * pInterface);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
