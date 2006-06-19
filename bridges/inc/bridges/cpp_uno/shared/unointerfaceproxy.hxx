/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unointerfaceproxy.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:38:24 $
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
