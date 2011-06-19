/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include "osl/mutex.hxx"

#include "com/sun/star/lang/XTypeProvider.hpp"
#include "com/sun/star/task/DocumentPasswordRequest.hpp"

#include "cppuhelper/typeprovider.hxx"
#include "ucbhelper/interactionrequest.hxx"

#include "tdoc_passwordrequest.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;

namespace tdoc_ucp
{
    class InteractionSupplyPassword :
                      public ucbhelper::InteractionContinuation,
                      public lang::XTypeProvider,
                      public task::XInteractionPassword
    {
    public:
        InteractionSupplyPassword( ucbhelper::InteractionRequest * pRequest )
        : InteractionContinuation( pRequest ) {}

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType )
            throw ( uno::RuntimeException );
        virtual void SAL_CALL acquire()
            throw ();
        virtual void SAL_CALL release()
            throw ();

        // XTypeProvider
        virtual uno::Sequence< uno::Type > SAL_CALL getTypes()
            throw ( uno::RuntimeException );
        virtual uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
            throw ( uno::RuntimeException );

        // XInteractionContinuation
        virtual void SAL_CALL select()
            throw ( uno::RuntimeException );

        // XInteractionPassword
        virtual void SAL_CALL setPassword( const rtl::OUString & aPasswd )
            throw ( uno::RuntimeException );
        virtual rtl::OUString SAL_CALL getPassword()
            throw ( uno::RuntimeException );

    private:
        osl::Mutex m_aMutex;
        rtl::OUString m_aPassword;
    };
} // namespace tdoc_ucp

//=========================================================================
//=========================================================================
//
// InteractionSupplyPassword Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL InteractionSupplyPassword::acquire()
    throw()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL InteractionSupplyPassword::release()
    throw()
{
    OWeakObject::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL
InteractionSupplyPassword::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< task::XInteractionPassword * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

// virtual
uno::Sequence< sal_Int8 > SAL_CALL
InteractionSupplyPassword::getImplementationId()
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId * pId = 0;
    if ( !pId )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static cppu::OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
}

//=========================================================================
// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionSupplyPassword::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection * pCollection = 0;
    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< task::XInteractionPassword > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

//=========================================================================
//
// XInteractionContinuation methods.
//
//=========================================================================

// virtual
void SAL_CALL InteractionSupplyPassword::select()
    throw( uno::RuntimeException )
{
    recordSelection();
}

//=========================================================================
//
// XInteractionPassword methods.
//
//=========================================================================

// virtual
void SAL_CALL
InteractionSupplyPassword::setPassword( const ::rtl::OUString& aPasswd )
    throw ( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_aPassword = aPasswd;
}

// virtual
rtl::OUString SAL_CALL InteractionSupplyPassword::getPassword()
    throw ( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_aPassword;
}

//=========================================================================
//=========================================================================
//
// DocumentPasswordRequest Implementation.
//
//=========================================================================
//=========================================================================

DocumentPasswordRequest::DocumentPasswordRequest(
    task::PasswordRequestMode eMode,
    const rtl::OUString & rDocumentName )
{
    // Fill request...
    task::DocumentPasswordRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.Mode           = eMode;
    aRequest.Name           = rDocumentName;

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    uno::Sequence<
        uno::Reference< task::XInteractionContinuation > > aContinuations( 3 );
    aContinuations[ 0 ] = new ucbhelper::InteractionAbort( this );
    aContinuations[ 1 ] = new ucbhelper::InteractionRetry( this );
    aContinuations[ 2 ] = new InteractionSupplyPassword( this );

    setContinuations( aContinuations );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
