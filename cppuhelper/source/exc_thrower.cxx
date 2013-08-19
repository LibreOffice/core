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


#include "rtl/instance.hxx"
#include "osl/diagnose.h"
#include "osl/doublecheckedlocking.h"
#include "osl/mutex.hxx"
#include "uno/dispatcher.hxx"
#include "uno/mapping.hxx"
#include "cppuhelper/detail/XExceptionThrower.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"

#include "cppuhelper/exc_hlp.hxx"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

using cppuhelper::detail::XExceptionThrower;

//==============================================================================
struct ExceptionThrower : public uno_Interface, XExceptionThrower
{
    inline ExceptionThrower();

    virtual ~ExceptionThrower() {}

    static inline Type const & getCppuType()
    {
        return ::getCppuType(
            reinterpret_cast< Reference< XExceptionThrower > const * >(0) );
    }

    // XInterface
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XExceptionThrower
    virtual void SAL_CALL throwException( Any const & exc ) throw (Exception);
    virtual void SAL_CALL rethrowException() throw (Exception);
};

extern "C"
{

//------------------------------------------------------------------------------
static void SAL_CALL ExceptionThrower_acquire_release_nop(
    SAL_UNUSED_PARAMETER uno_Interface * )
{}

//------------------------------------------------------------------------------
static void SAL_CALL ExceptionThrower_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberType,
    void * pReturn, void * pArgs [], uno_Any ** ppException )
{
    OSL_ASSERT( pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD );

    switch (reinterpret_cast< typelib_InterfaceMemberTypeDescription * >(
                const_cast< typelib_TypeDescription * >( pMemberType ) )->
            nPosition)
    {
    case 0: // queryInterace()
    {
        Type const & rType_demanded =
            *reinterpret_cast< Type const * >( pArgs[ 0 ] );
        if (rType_demanded.equals(
                ::getCppuType( reinterpret_cast<
                               Reference< XInterface > const * >(0) ) ) ||
            rType_demanded.equals( ExceptionThrower::getCppuType() ))
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, rType_demanded.getTypeLibType() );
            uno_any_construct(
                reinterpret_cast< uno_Any * >( pReturn ), &pUnoI, pTD, 0 );
            TYPELIB_DANGER_RELEASE( pTD );
        }
        else
        {
            uno_any_construct(
                reinterpret_cast< uno_Any * >( pReturn ), 0, 0, 0 );
        }
        *ppException = 0;
        break;
    }
    case 1: // acquire()
    case 2: // release()
        *ppException = 0;
        break;
    case 3: // throwException()
    {
        uno_Any * pAny = reinterpret_cast< uno_Any * >( pArgs[ 0 ] );
        OSL_ASSERT( pAny->pType->eTypeClass == typelib_TypeClass_EXCEPTION );
        uno_type_any_construct( *ppException, pAny->pData, pAny->pType, 0 );
        break;
    }
    default:
    {
        OSL_ASSERT( 0 );
        RuntimeException exc(
            "not implemented!", Reference< XInterface >() );
        uno_type_any_construct(
            *ppException, &exc, ::getCppuType( &exc ).getTypeLibType(), 0 );
        break;
    }
    }
}

} // extern "C"

//______________________________________________________________________________
Any ExceptionThrower::queryInterface( Type const & type )
    throw (RuntimeException)
{
    if (type.equals( ::getCppuType( reinterpret_cast<
                                    Reference< XInterface > const * >(0) ) ) ||
        type.equals( ExceptionThrower::getCppuType() ))
    {
        XExceptionThrower * that = static_cast< XExceptionThrower * >( this );
        return Any( &that, type );
    }
    return Any();
}

//______________________________________________________________________________
void ExceptionThrower::acquire() throw ()
{
}
//______________________________________________________________________________
void ExceptionThrower::release() throw ()
{
}

//______________________________________________________________________________
void ExceptionThrower::throwException( Any const & exc ) throw (Exception)
{
    OSL_FAIL( "unexpected!" );
    throwException( exc );
}

//______________________________________________________________________________
void ExceptionThrower::rethrowException() throw (Exception)
{
    throw;
}

//______________________________________________________________________________
inline ExceptionThrower::ExceptionThrower()
{
    uno_Interface::acquire = ExceptionThrower_acquire_release_nop;
    uno_Interface::release = ExceptionThrower_acquire_release_nop;
    uno_Interface::pDispatcher = ExceptionThrower_dispatch;
}

class theExceptionThrower : public rtl::Static<ExceptionThrower, theExceptionThrower> {};

} // anonymous namespace


namespace cppu
{

//==============================================================================
void SAL_CALL throwException( Any const & exc ) SAL_THROW( (Exception) )
{
    if (exc.getValueTypeClass() != TypeClass_EXCEPTION)
    {
        throw RuntimeException(
            "no UNO exception given "
            "(must be derived from com::sun::star::uno::Exception)!",
            Reference< XInterface >() );
    }

    Mapping uno2cpp(Environment(UNO_LB_UNO), Environment::getCurrent());
    if (! uno2cpp.is())
    {
        throw RuntimeException(
            "cannot get binary UNO to C++ mapping!",
            Reference< XInterface >() );
    }

    Reference< XExceptionThrower > xThrower;
    uno2cpp.mapInterface(
        reinterpret_cast< void ** >( &xThrower ),
        static_cast< uno_Interface * >( &theExceptionThrower::get() ),
        ExceptionThrower::getCppuType() );
    OSL_ASSERT( xThrower.is() );
    xThrower->throwException( exc );
}

//==============================================================================
Any SAL_CALL getCaughtException()
{
    Mapping cpp2uno(Environment::getCurrent(), Environment(UNO_LB_UNO));
    if (! cpp2uno.is())
    {
        throw RuntimeException(
            "cannot get C++ to binary UNO mapping!",
            Reference< XInterface >() );
    }
    Mapping uno2cpp(Environment(UNO_LB_UNO), Environment::getCurrent());
    if (! uno2cpp.is())
    {
        throw RuntimeException(
            "cannot get binary UNO to C++ mapping!",
            Reference< XInterface >() );
    }

    typelib_TypeDescription * pTD = 0;
    TYPELIB_DANGER_GET(
        &pTD, ExceptionThrower::getCppuType().getTypeLibType() );

    UnoInterfaceReference unoI;
    cpp2uno.mapInterface(
        reinterpret_cast< void ** >( &unoI.m_pUnoI ),
        static_cast< XExceptionThrower * >( &theExceptionThrower::get() ), pTD );
    OSL_ASSERT( unoI.is() );

    typelib_TypeDescription * pMemberTD = 0;
    TYPELIB_DANGER_GET(
        &pMemberTD,
        reinterpret_cast< typelib_InterfaceTypeDescription * >( pTD )->
        ppMembers[ 1 ] /* rethrowException() */ );

    uno_Any exc_mem;
    uno_Any * exc = &exc_mem;
    unoI.dispatch( pMemberTD, 0, 0, &exc );

    TYPELIB_DANGER_RELEASE( pMemberTD );
    TYPELIB_DANGER_RELEASE( pTD );

    if (exc == 0)
    {
        throw RuntimeException(
            "rethrowing C++ exception failed!",
            Reference< XInterface >() );
    }

    Any ret;
    uno_any_destruct( &ret, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_type_any_constructAndConvert(
        &ret, exc->pData, exc->pType, uno2cpp.get() );
    uno_any_destruct( exc, 0 );
    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
