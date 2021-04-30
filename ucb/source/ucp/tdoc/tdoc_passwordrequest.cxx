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


#include <osl/mutex.hxx>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <ucbhelper/interactionrequest.hxx>

#include "tdoc_passwordrequest.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;

namespace tdoc_ucp
{
    namespace {

    class InteractionSupplyPassword :
                      public ucbhelper::InteractionContinuation,
                      public lang::XTypeProvider,
                      public task::XInteractionPassword
    {
    public:
        explicit InteractionSupplyPassword( ucbhelper::InteractionRequest * pRequest )
            : InteractionContinuation( pRequest ) {}

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType ) override;
        virtual void SAL_CALL acquire()
            noexcept override;
        virtual void SAL_CALL release()
            noexcept override;

        // XTypeProvider
        virtual uno::Sequence< uno::Type > SAL_CALL getTypes() override;
        virtual uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

        // XInteractionContinuation
        virtual void SAL_CALL select() override;

        // XInteractionPassword
        virtual void SAL_CALL setPassword( const OUString & aPasswd ) override;
        virtual OUString SAL_CALL getPassword() override;

    private:
        osl::Mutex m_aMutex;
        OUString m_aPassword;
    };

    }
} // namespace tdoc_ucp


// InteractionSupplyPassword Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionSupplyPassword::acquire()
    noexcept
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionSupplyPassword::release()
    noexcept
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionSupplyPassword::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< task::XInteractionPassword * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL
InteractionSupplyPassword::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionSupplyPassword::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<task::XInteractionPassword>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionSupplyPassword::select()
{
    recordSelection();
}


// XInteractionPassword methods.


// virtual
void SAL_CALL
InteractionSupplyPassword::setPassword( const OUString& aPasswd )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_aPassword = aPasswd;
}

// virtual
OUString SAL_CALL InteractionSupplyPassword::getPassword()
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_aPassword;
}


// DocumentPasswordRequest Implementation.


DocumentPasswordRequest::DocumentPasswordRequest(
    task::PasswordRequestMode eMode,
    const OUString & rDocumentName )
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
