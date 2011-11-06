/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_UNOINTERFACEPROXY_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_UNOINTERFACEPROXY_HXX

#include "osl/interlck.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
} } } }

namespace bridges { namespace cpp_uno { namespace shared {

class Bridge;

extern "C" typedef void SAL_CALL FreeUnoInterfaceProxy(
    uno_ExtEnvironment * pEnv, void * pProxy);
FreeUnoInterfaceProxy freeUnoInterfaceProxy;

// private:
extern "C" typedef void SAL_CALL UnoInterfaceProxyDispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException);
UnoInterfaceProxyDispatch unoInterfaceProxyDispatch;
    // this function is not defined in the generic part, but instead has to be
    // defined individually for each CPP--UNO bridge

// private:
extern "C" typedef void SAL_CALL AcquireProxy(uno_Interface *);
AcquireProxy acquireProxy;

// private:
extern "C" typedef void SAL_CALL ReleaseProxy(uno_Interface *);
ReleaseProxy releaseProxy;

/**
 * A uno proxy wrapping a cpp interface.
 */
class UnoInterfaceProxy: public uno_Interface {
public:
    // Interface for Bridge:

    static UnoInterfaceProxy * create(
        Bridge * pBridge, com::sun::star::uno::XInterface * pCppI,
        typelib_InterfaceTypeDescription * pTypeDescr,
        rtl::OUString const & rOId) SAL_THROW(());

    // Interface for individual CPP--UNO bridges:

    Bridge * getBridge() { return pBridge; }
    com::sun::star::uno::XInterface * getCppI() { return pCppI; }

private:
    UnoInterfaceProxy(UnoInterfaceProxy &); // not implemented
    void operator =(UnoInterfaceProxy); // not implemented

    UnoInterfaceProxy(
        Bridge * pBridge_, com::sun::star::uno::XInterface * pCppI_,
        typelib_InterfaceTypeDescription * pTypeDescr_,
        rtl::OUString const & rOId_) SAL_THROW(());

    ~UnoInterfaceProxy();

    oslInterlockedCount nRef;
    Bridge * pBridge;

    // mapping information
    com::sun::star::uno::XInterface * pCppI; // wrapped interface
    typelib_InterfaceTypeDescription * pTypeDescr;
    rtl::OUString oid;

    friend void SAL_CALL freeUnoInterfaceProxy(
        uno_ExtEnvironment * pEnv, void * pProxy);

    friend void SAL_CALL unoInterfaceProxyDispatch(
        uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
        void * pReturn, void * pArgs[], uno_Any ** ppException);

    friend void SAL_CALL acquireProxy(uno_Interface * pUnoI);

    friend void SAL_CALL releaseProxy(uno_Interface * pUnoI);
};

} } }

#endif
