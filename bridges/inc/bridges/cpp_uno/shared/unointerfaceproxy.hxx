/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unointerfaceproxy.hxx,v $
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
