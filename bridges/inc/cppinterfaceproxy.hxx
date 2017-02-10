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

#ifndef INCLUDED_BRIDGES_INC_CPPINTERFACEPROXY_HXX
#define INCLUDED_BRIDGES_INC_CPPINTERFACEPROXY_HXX

#include "osl/interlck.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"
#include "vtablefactory.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
} } } }

#if !defined __GNUG__
void dso_init();
void dso_exit();
#endif

namespace bridges { namespace cpp_uno { namespace shared {

class Bridge;

extern "C" typedef void SAL_CALL FreeCppInterfaceProxy(
    uno_ExtEnvironment * pEnv, void * pInterface);
FreeCppInterfaceProxy freeCppInterfaceProxy;

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
    const OUString& getOid() { return oid; }

    // non virtual methods called on incoming vtable calls #1, #2
    void acquireProxy();
    void releaseProxy();

    static CppInterfaceProxy * castInterfaceToProxy(void * pInterface);

private:
    CppInterfaceProxy(CppInterfaceProxy &) = delete;
    void operator =(const CppInterfaceProxy&) = delete;

    CppInterfaceProxy(
        Bridge * pBridge_, uno_Interface * pUnoI_,
        typelib_InterfaceTypeDescription * pTypeDescr_,
        OUString const & rOId_);

    ~CppInterfaceProxy();

    static com::sun::star::uno::XInterface * castProxyToInterface(
        CppInterfaceProxy * pProxy);

    oslInterlockedCount nRef;
    Bridge * pBridge;

    // mapping information
    uno_Interface * pUnoI; // wrapped interface
    typelib_InterfaceTypeDescription * pTypeDescr;
    OUString oid;

    VtableFactory::Slot * vtables[1];

    friend void SAL_CALL freeCppInterfaceProxy(
        uno_ExtEnvironment * pEnv, void * pInterface);
};

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
