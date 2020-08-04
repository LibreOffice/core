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

#include <sal/types.h>
#include <typelib/typedescription.h>
#include <uno/environment.h>
#include <uno/mapping.h>

namespace bridges::cpp_uno::shared {

// private:
extern "C" void freeMapping(uno_Mapping *);

// private:
extern "C" void acquireMapping(uno_Mapping *);

// private:
extern "C" void releaseMapping(uno_Mapping *);

// private:
extern "C" void cpp2unoMapping(
    uno_Mapping *, void **, void *, typelib_InterfaceTypeDescription *);

// private:
extern "C" void uno2cppMapping(
    uno_Mapping *, void **, void *, typelib_InterfaceTypeDescription *);

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
    Bridge(Bridge const &) = delete;
    Bridge& operator =(const Bridge&) = delete;

    Bridge(
        uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
        bool bExportCpp2Uno_);

    ~Bridge();

    struct Mapping: public uno_Mapping {
        Bridge * pBridge;
    };

    std::atomic<std::size_t> nRef;

    uno_ExtEnvironment * pCppEnv;
    uno_ExtEnvironment * pUnoEnv;

    Mapping aCpp2Uno;
    Mapping aUno2Cpp;

    bool bExportCpp2Uno;

    friend void freeMapping(uno_Mapping * pMapping);

    friend void acquireMapping(uno_Mapping * pMapping);

    friend void releaseMapping(uno_Mapping * pMapping);

    friend void cpp2unoMapping(
        uno_Mapping * pMapping, void ** ppUnoI, void * pCppI,
        typelib_InterfaceTypeDescription * pTypeDescr);

    friend void uno2cppMapping(
        uno_Mapping * pMapping, void ** ppCppI, void * pUnoI,
        typelib_InterfaceTypeDescription * pTypeDescr);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
