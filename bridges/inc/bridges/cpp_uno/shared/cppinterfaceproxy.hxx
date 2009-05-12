/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cppinterfaceproxy.hxx,v $
 * $Revision: 1.6 $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_CPPINTERFACEPROXY_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_CPPINTERFACEPROXY_HXX

#include "osl/interlck.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
} } } }

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
        rtl::OUString const & rOId) SAL_THROW(());

    // Interface for individual CPP--UNO bridges:

    Bridge * getBridge() { return pBridge; }
    uno_Interface * getUnoI() { return pUnoI; }
    typelib_InterfaceTypeDescription * getTypeDescr() { return pTypeDescr; }
    rtl::OUString getOid() { return oid; }

    // non virtual methods called on incoming vtable calls #1, #2
    void acquireProxy() SAL_THROW(());
    void releaseProxy() SAL_THROW(());

    static CppInterfaceProxy * castInterfaceToProxy(void * pInterface);

private:
    CppInterfaceProxy(CppInterfaceProxy &); // not implemented
    void operator =(CppInterfaceProxy); // not implemented

    CppInterfaceProxy(
        Bridge * pBridge_, uno_Interface * pUnoI_,
        typelib_InterfaceTypeDescription * pTypeDescr_,
        rtl::OUString const & rOId_) SAL_THROW(());

    ~CppInterfaceProxy();

    static com::sun::star::uno::XInterface * castProxyToInterface(
        CppInterfaceProxy * pProxy);

    oslInterlockedCount nRef;
    Bridge * pBridge;

    // mapping information
    uno_Interface * pUnoI; // wrapped interface
    typelib_InterfaceTypeDescription * pTypeDescr;
    rtl::OUString oid;

    VtableFactory::Slot * vtables[1];

    friend void SAL_CALL freeCppInterfaceProxy(
        uno_ExtEnvironment * pEnv, void * pInterface);
};

} } }

#endif
