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

#ifndef INCLUDED_BRIDGES_INC_BRIDGES_CPP_UNO_SHARED_BRIDGE_HXX
#define INCLUDED_BRIDGES_INC_BRIDGES_CPP_UNO_SHARED_BRIDGE_HXX

#include "osl/interlck.h"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/environment.h"
#include "uno/mapping.h"

namespace bridges { namespace cpp_uno { namespace shared {

// private:
extern "C" typedef void SAL_CALL FreeMapping(uno_Mapping *);
FreeMapping freeMapping;

// private:
extern "C"
typedef void SAL_CALL AcquireMapping(uno_Mapping *);
AcquireMapping acquireMapping;

// private:
extern "C"
typedef void SAL_CALL ReleaseMapping(uno_Mapping *);
ReleaseMapping releaseMapping;

// private:
extern "C" typedef void SAL_CALL Cpp2unoMapping(
    uno_Mapping *, void **, void *, typelib_InterfaceTypeDescription *);
Cpp2unoMapping cpp2unoMapping;

// private:
extern "C" typedef void SAL_CALL Uno2cppMapping(
    uno_Mapping *, void **, void *, typelib_InterfaceTypeDescription *);
Uno2cppMapping uno2cppMapping;

/**
 * Holding environments and mappings.
 */
class Bridge {
public:
    // Interface for generic/component.cxx:

    static uno_Mapping * createMapping(
        uno_ExtEnvironment * pCppEnv, uno_ExtEnvironment * pUnoEnv,
        bool bExportCpp2Uno);

    // Interface for Cpp/UnoInterfaceProxy:

    void acquire();
    void release();

    // Interface for individual CPP--UNO bridges:

    uno_ExtEnvironment * getCppEnv() { return pCppEnv; }
    uno_ExtEnvironment * getUnoEnv() { return pUnoEnv; }

    uno_Mapping * getCpp2Uno() { return &aCpp2Uno; }
    uno_Mapping * getUno2Cpp() { return &aUno2Cpp; }

private:
    Bridge(Bridge &) = delete;
    void operator =(const Bridge&) = delete;

    Bridge(
        uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
        bool bExportCpp2Uno_);

    ~Bridge();

    struct Mapping: public uno_Mapping {
        Bridge * pBridge;
    };

    oslInterlockedCount nRef;

    uno_ExtEnvironment * pCppEnv;
    uno_ExtEnvironment * pUnoEnv;

    Mapping aCpp2Uno;
    Mapping aUno2Cpp;

    bool bExportCpp2Uno;

    friend void SAL_CALL freeMapping(uno_Mapping * pMapping);

    friend void SAL_CALL acquireMapping(uno_Mapping * pMapping);

    friend void SAL_CALL releaseMapping(uno_Mapping * pMapping);

    friend void SAL_CALL cpp2unoMapping(
        uno_Mapping * pMapping, void ** ppUnoI, void * pCppI,
        typelib_InterfaceTypeDescription * pTypeDescr);

    friend void SAL_CALL uno2cppMapping(
        uno_Mapping * pMapping, void ** ppCppI, void * pUnoI,
        typelib_InterfaceTypeDescription * pTypeDescr);
};

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
