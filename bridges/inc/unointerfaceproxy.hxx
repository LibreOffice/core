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

namespace com::sun::star::uno { class XInterface; }

namespace bridges::cpp_uno::shared {

class Bridge;

extern "C" void freeUnoInterfaceProxy(
    uno_ExtEnvironment * pEnv, void * pProxy);

// private:
extern "C" void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException);
    // this function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge

// private:
extern "C" void acquireProxy(uno_Interface *);

// private:
extern "C" void releaseProxy(uno_Interface *);

/**
 * A uno proxy wrapping a cpp interface.
 */
class UnoInterfaceProxy: public uno_Interface {
public:
    // Interface for Bridge:

    static UnoInterfaceProxy * create(
        Bridge * pBridge, com::sun::star::uno::XInterface * pCppI,
        typelib_InterfaceTypeDescription * pTypeDescr,
        OUString const & rOId);

    // Interface for individual CPP--UNO bridges:

    Bridge * getBridge() { return pBridge; }
    com::sun::star::uno::XInterface * getCppI() { return pCppI; }

private:
    UnoInterfaceProxy(UnoInterfaceProxy const &) = delete;
    UnoInterfaceProxy& operator =(const UnoInterfaceProxy&) = delete;

    UnoInterfaceProxy(
        Bridge * pBridge_, com::sun::star::uno::XInterface * pCppI_,
        typelib_InterfaceTypeDescription * pTypeDescr_,
        OUString const & rOId_);

    ~UnoInterfaceProxy();

    std::atomic<std::size_t> nRef;
    Bridge * pBridge;

    // mapping information
    com::sun::star::uno::XInterface * pCppI; // wrapped interface
    typelib_InterfaceTypeDescription * pTypeDescr;
    OUString oid;

    friend void freeUnoInterfaceProxy(
        uno_ExtEnvironment * pEnv, void * pProxy);

    friend void unoInterfaceProxyDispatch(
        uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
        void * pReturn, void * pArgs[], uno_Any ** ppException);

    friend void acquireProxy(uno_Interface * pUnoI);

    friend void releaseProxy(uno_Interface * pUnoI);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
