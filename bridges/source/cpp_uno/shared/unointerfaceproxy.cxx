/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unointerfaceproxy.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:35:28 $
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

#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"

#include "bridges/cpp_uno/shared/bridge.hxx"

#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"

using bridges::cpp_uno::shared::UnoInterfaceProxy;

UnoInterfaceProxy * UnoInterfaceProxy::create(
    bridges::cpp_uno::shared::Bridge * pBridge,
    com::sun::star::uno::XInterface * pCppI,
    typelib_InterfaceTypeDescription * pTypeDescr,
    rtl::OUString const & rOId) SAL_THROW(())
{
    return new UnoInterfaceProxy(pBridge, pCppI, pTypeDescr, rOId);
}

void UnoInterfaceProxy::free(uno_ExtEnvironment * pEnv, void * pProxy)
    SAL_THROW(())
{
    UnoInterfaceProxy * pThis =
        static_cast< UnoInterfaceProxy * >(
            reinterpret_cast< uno_Interface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pBridge->getUnoEnv() );

    (*pThis->pBridge->getCppEnv()->revokeInterface)(
        pThis->pBridge->getCppEnv(), pThis->pCppI );
    pThis->pCppI->release();
    ::typelib_typedescription_release(
        (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}

UnoInterfaceProxy::UnoInterfaceProxy(
    bridges::cpp_uno::shared::Bridge * pBridge_,
    com::sun::star::uno::XInterface * pCppI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, rtl::OUString const & rOId_)
    SAL_THROW(())
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pCppI( pCppI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    pBridge->acquire();
    ::typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    if (! ((typelib_TypeDescription *)pTypeDescr)->bComplete)
        ::typelib_typedescription_complete(
            (typelib_TypeDescription **)&pTypeDescr );
    OSL_ENSURE(
        ((typelib_TypeDescription *)pTypeDescr)->bComplete,
        "### type is incomplete!" );
    pCppI->acquire();
    (*pBridge->getCppEnv()->registerInterface)(
        pBridge->getCppEnv(), reinterpret_cast< void ** >( &pCppI ), oid.pData,
        pTypeDescr );

    // uno_Interface
    acquire = acquireProxy;
    release = releaseProxy;
    pDispatcher = dispatch;
}

UnoInterfaceProxy::~UnoInterfaceProxy()
{}

void UnoInterfaceProxy::acquireProxy(uno_Interface * pUnoI) SAL_THROW(())
{
    if (1 == osl_incrementInterlockedCount(
            & static_cast< UnoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // rebirth of proxy zombie
        // register at uno env
#if OSL_DEBUG_LEVEL > 1
        void * pThis = pUnoI;
#endif
        (*static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv()->
         registerProxyInterface)(
             static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv(),
             reinterpret_cast< void ** >( &pUnoI ),
             free,
             static_cast< UnoInterfaceProxy * >( pUnoI )->oid.pData,
             static_cast< UnoInterfaceProxy * >( pUnoI )->pTypeDescr );
#if OSL_DEBUG_LEVEL > 1
        OSL_ASSERT( pThis == pUnoI );
#endif
    }
}

void UnoInterfaceProxy::releaseProxy(uno_Interface * pUnoI) SAL_THROW(())
{
    if (! osl_decrementInterlockedCount(
            & static_cast< UnoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // revoke from uno env on last release
        (*static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv()->
         revokeInterface)(
             static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv(),
             pUnoI );
    }
}
