/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cppinterfaceproxy.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-27 09:47:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
