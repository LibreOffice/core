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


#include <rtl/instance.hxx>
#include <osl/diagnose.h>
#include <uno/dispatcher.hxx>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>
#include <cppuhelper/detail/XExceptionThrower.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <cppuhelper/exc_hlp.hxx>

#ifdef IOS
#include <ios/ios.hxx>
#endif

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

using cppuhelper::detail::XExceptionThrower;


struct ExceptionThrower : public uno_Interface, XExceptionThrower
{
    ExceptionThrower();

    virtual ~ExceptionThrower() {}

    static Type const & getCppuType()
    {
        return cppu::UnoType<XExceptionThrower>::get();
    }

    // XInterface
    virtual Any SAL_CALL queryInterface( Type const & type ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XExceptionThrower
    virtual void SAL_CALL throwException( Any const & exc ) override;
    virtual void SAL_CALL rethrowException() override;
};

extern "C"
{


void ExceptionThrower_acquire_release_nop(
    SAL_UNUSED_PARAMETER uno_Interface * )
{}


void ExceptionThrower_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberType,
    void * pReturn, void * pArgs [], uno_Any ** ppException )
{
    OSL_ASSERT( pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD );

    switch (reinterpret_cast< typelib_InterfaceMemberTypeDescription * >(
                const_cast< typelib_TypeDescription * >( pMemberType ) )->
            nPosition)
    {
    case 0: // queryInterface()
    {
        Type const & rType_demanded =
            *static_cast< Type const * >( pArgs[ 0 ] );
        if (rType_demanded.equals( cppu::UnoType<XInterface>::get() ) ||
            rType_demanded.equals( ExceptionThrower::getCppuType() ))
        {
            typelib_TypeDescription * pTD = nullptr;
            TYPELIB_DANGER_GET( &pTD, rType_demanded.getTypeLibType() );
            uno_any_construct(
                static_cast< uno_Any * >( pReturn ), &pUnoI, pTD, nullptr );
            TYPELIB_DANGER_RELEASE( pTD );
        }
        else
        {
            uno_any_construct(
                static_cast< uno_Any * >( pReturn ), nullptr, nullptr, nullptr );
        }
        *ppException = nullptr;
        break;
    }
    case 1: // acquire()
    case 2: // release()
        *ppException = nullptr;
        break;
    case 3: // throwException()
    {
        uno_Any * pAny = static_cast< uno_Any * >( pArgs[ 0 ] );
        OSL_ASSERT( pAny->pType->eTypeClass == typelib_TypeClass_EXCEPTION );
        uno_type_any_construct( *ppException, pAny->pData, pAny->pType, nullptr );
        break;
    }
    default:
    {
        OSL_ASSERT( false );
        RuntimeException exc( "not implemented!" );
        uno_type_any_construct(
            *ppException, &exc, cppu::UnoType<decltype(exc)>::get().getTypeLibType(), nullptr );
        break;
    }
    }
}

} // extern "C"


Any ExceptionThrower::queryInterface( Type const & type )
{
    if (type.equals( cppu::UnoType<XInterface>::get() ) ||
        type.equals( ExceptionThrower::getCppuType() ))
    {
        XExceptionThrower * that = static_cast< XExceptionThrower * >( this );
        return Any( &that, type );
    }
    return Any();
}


void ExceptionThrower::acquire() throw ()
{
}

void ExceptionThrower::release() throw ()
{
}


void ExceptionThrower::throwException( Any const & exc )
{
    OSL_FAIL( "unexpected!" );
    cppu::throwException( exc );
}


void ExceptionThrower::rethrowException()
{
    throw;
}


ExceptionThrower::ExceptionThrower()
{
    uno_Interface::acquire = ExceptionThrower_acquire_release_nop;
    uno_Interface::release = ExceptionThrower_acquire_release_nop;
    uno_Interface::pDispatcher = ExceptionThrower_dispatch;
}

class theExceptionThrower : public rtl::Static<ExceptionThrower, theExceptionThrower> {};

} // anonymous namespace


namespace cppu
{


void SAL_CALL throwException( Any const & exc )
{
    if (exc.getValueTypeClass() != TypeClass_EXCEPTION)
    {
        throw RuntimeException(
            "no UNO exception given "
            "(must be derived from com::sun::star::uno::Exception)!" );
    }

#ifdef IOS
    lo_ios_throwException(exc);
#else
    Mapping uno2cpp(Environment(UNO_LB_UNO), Environment::getCurrent());
    if (! uno2cpp.is())
    {
        throw RuntimeException(
            "cannot get binary UNO to C++ mapping!" );
    }

    Reference< XExceptionThrower > xThrower;
    uno2cpp.mapInterface(
        reinterpret_cast< void ** >( &xThrower ),
        static_cast< uno_Interface * >( &theExceptionThrower::get() ),
        ExceptionThrower::getCppuType() );
    OSL_ASSERT( xThrower.is() );
    xThrower->throwException( exc );
#endif
}


Any SAL_CALL getCaughtException()
{
    Mapping cpp2uno(Environment::getCurrent(), Environment(UNO_LB_UNO));
    if (! cpp2uno.is())
    {
        throw RuntimeException(
            "cannot get C++ to binary UNO mapping!" );
    }
    Mapping uno2cpp(Environment(UNO_LB_UNO), Environment::getCurrent());
    if (! uno2cpp.is())
    {
        throw RuntimeException(
            "cannot get binary UNO to C++ mapping!" );
    }

    typelib_TypeDescription * pTD = nullptr;
    TYPELIB_DANGER_GET(
        &pTD, ExceptionThrower::getCppuType().getTypeLibType() );

    UnoInterfaceReference unoI;
    cpp2uno.mapInterface(
        reinterpret_cast< void ** >( &unoI.m_pUnoI ),
        static_cast< XExceptionThrower * >( &theExceptionThrower::get() ), pTD );
    OSL_ASSERT( unoI.is() );

    typelib_TypeDescription * pMemberTD = nullptr;
    TYPELIB_DANGER_GET(
        &pMemberTD,
        reinterpret_cast< typelib_InterfaceTypeDescription * >( pTD )->
        ppMembers[ 1 ] /* rethrowException() */ );

    uno_Any exc_mem;
    uno_Any * exc = &exc_mem;
    unoI.dispatch( pMemberTD, nullptr, nullptr, &exc );

    TYPELIB_DANGER_RELEASE( pMemberTD );
    TYPELIB_DANGER_RELEASE( pTD );

    if (exc == nullptr)
    {
        throw RuntimeException( "rethrowing C++ exception failed!" );
    }

    Any ret;
    uno_any_destruct( &ret, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_type_any_constructAndConvert(
        &ret, exc->pData, exc->pType, uno2cpp.get() );
    uno_any_destruct( exc, nullptr );
    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
