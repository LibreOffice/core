/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bridge.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_BRIDGE_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_BRIDGE_HXX

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
        bool bExportCpp2Uno) SAL_THROW(());

    // Interface for Cpp/UnoInterfaceProxy:

    void acquire() SAL_THROW(());
    void release() SAL_THROW(());

    // Interface for individual CPP--UNO bridges:

    uno_ExtEnvironment * getCppEnv() { return pCppEnv; }
    uno_ExtEnvironment * getUnoEnv() { return pUnoEnv; }

    uno_Mapping * getCpp2Uno() { return &aCpp2Uno; }
    uno_Mapping * getUno2Cpp() { return &aUno2Cpp; }

private:
    Bridge(Bridge &); // not implemented
    void operator =(Bridge); // not implemented

    Bridge(
        uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
        bool bExportCpp2Uno_) SAL_THROW(());

    ~Bridge() SAL_THROW(());

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
