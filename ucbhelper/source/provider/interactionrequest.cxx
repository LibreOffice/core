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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <ucbhelper/interactionrequest.hxx>

#include <rtl/ref.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace com::sun::star;
using namespace ucbhelper;


// InteractionRequest Implementation.


namespace ucbhelper
{

struct InteractionRequest_Impl
{
    rtl::Reference< InteractionContinuation > m_xSelection;
    css::uno::Any m_aRequest;
    css::uno::Sequence<
        css::uno::Reference<
            css::task::XInteractionContinuation > > m_aContinuations;

    InteractionRequest_Impl() {}
    explicit InteractionRequest_Impl( const uno::Any & rRequest )
    : m_aRequest( rRequest ) {}
};

}


InteractionRequest::InteractionRequest()
: m_pImpl( new InteractionRequest_Impl )
{
}


InteractionRequest::InteractionRequest( const uno::Any & rRequest )
: m_pImpl( new InteractionRequest_Impl( rRequest ) )
{
}


// virtual
InteractionRequest::~InteractionRequest()
{
}


void InteractionRequest::setRequest( const uno::Any & rRequest )
{
    m_pImpl->m_aRequest = rRequest;
}


void InteractionRequest::setContinuations(
                const uno::Sequence< uno::Reference<
                    task::XInteractionContinuation > > & rContinuations )
{
    m_pImpl->m_aContinuations = rContinuations;
}


rtl::Reference< InteractionContinuation > const &
InteractionRequest::getSelection() const
{
    return m_pImpl->m_xSelection;
}


void InteractionRequest::setSelection(
                const rtl::Reference< InteractionContinuation > & rxSelection )
{
    m_pImpl->m_xSelection = rxSelection;
}


// XInterface methods.


// virtual
void SAL_CALL InteractionRequest::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionRequest::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionRequest::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionRequest * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionRequest::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionRequest::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<task::XInteractionRequest>::get() );

    return s_aCollection.getTypes();
}


// XInteractionRequest methods.


// virtual
uno::Any SAL_CALL InteractionRequest::getRequest()
{
    return m_pImpl->m_aRequest;
}


// virtual
uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL
InteractionRequest::getContinuations()
{
    return m_pImpl->m_aContinuations;
}


// InteractionContinuation Implementation.


InteractionContinuation::InteractionContinuation(
                        InteractionRequest * pRequest )
: m_pRequest( pRequest )
{
}


// virtual
InteractionContinuation::~InteractionContinuation()
{
}


void InteractionContinuation::recordSelection()
{
    m_pRequest->setSelection( this );
}


// InteractionAbort Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionAbort::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionAbort::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionAbort::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< task::XInteractionAbort * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionAbort::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionAbort::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<task::XInteractionAbort>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionAbort::select()
{
    recordSelection();
}


// InteractionRetry Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionRetry::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionRetry::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionRetry::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< task::XInteractionRetry * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionRetry::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionRetry::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<task::XInteractionRetry>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionRetry::select()
{
    recordSelection();
}


// InteractionApprove Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionApprove::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionApprove::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionApprove::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< task::XInteractionApprove * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionApprove::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionApprove::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<task::XInteractionApprove>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionApprove::select()
{
    recordSelection();
}


// InteractionDisapprove Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionDisapprove::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionDisapprove::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionDisapprove::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< task::XInteractionDisapprove * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionDisapprove::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionDisapprove::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<task::XInteractionDisapprove>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionDisapprove::select()
{
    recordSelection();
}


// InteractionSupplyAuthentication Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionSupplyAuthentication::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionSupplyAuthentication::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionSupplyAuthentication::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
            static_cast< lang::XTypeProvider * >( this ),
            static_cast< task::XInteractionContinuation * >( this ),
            static_cast< ucb::XInteractionSupplyAuthentication * >( this ),
            static_cast< ucb::XInteractionSupplyAuthentication2 * >( this ));

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL
InteractionSupplyAuthentication::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionSupplyAuthentication::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<ucb::XInteractionSupplyAuthentication2>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionSupplyAuthentication::select()
{
    recordSelection();
}


// XInteractionSupplyAuthentication methods.


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetRealm()
{
    return m_bCanSetRealm;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setRealm( const OUString& Realm )
{
    OSL_ENSURE( m_bCanSetPassword,
        "InteractionSupplyAuthentication::setRealm - Not supported!" );

    if ( m_bCanSetRealm )
        m_aRealm = Realm;
}


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetUserName()
{
    return m_bCanSetUserName;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setUserName( const OUString& UserName )
{
    OSL_ENSURE( m_bCanSetUserName,
        "InteractionSupplyAuthentication::setUserName - Not supported!" );

    if ( m_bCanSetUserName )
        m_aUserName = UserName;
}


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetPassword()
{
    return m_bCanSetPassword;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setPassword( const OUString& Password )
{
    OSL_ENSURE( m_bCanSetPassword,
        "InteractionSupplyAuthentication::setPassword - Not supported!" );

    if ( m_bCanSetPassword )
        m_aPassword = Password;
}


// virtual
uno::Sequence< ucb::RememberAuthentication > SAL_CALL
InteractionSupplyAuthentication::getRememberPasswordModes(
                                    ucb::RememberAuthentication& Default )
{
    Default = m_eDefaultRememberPasswordMode;
    return m_aRememberPasswordModes;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setRememberPassword(
                                    ucb::RememberAuthentication Remember )
{
    m_eRememberPasswordMode = Remember;
}


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetAccount()
{
    return m_bCanSetAccount;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setAccount( const OUString& /*Account*/ )
{
    OSL_ENSURE( m_bCanSetAccount,
        "InteractionSupplyAuthentication::setAccount - Not supported!" );
}


// virtual
uno::Sequence< ucb::RememberAuthentication > SAL_CALL
InteractionSupplyAuthentication::getRememberAccountModes(
                                    ucb::RememberAuthentication& Default )
{
    Default = m_eDefaultRememberAccountMode;
    return m_aRememberAccountModes;
}


// virtual
void SAL_CALL InteractionSupplyAuthentication::setRememberAccount(
                                    ucb::RememberAuthentication )
{
}


// XInteractionSupplyAuthentication2 methods.


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canUseSystemCredentials(
        sal_Bool& Default )
{
    Default = false;
    return m_bCanUseSystemCredentials;
}


// virtual
void SAL_CALL InteractionSupplyAuthentication::setUseSystemCredentials(
        sal_Bool UseSystemCredentials )
{
    if ( m_bCanUseSystemCredentials )
        m_bUseSystemCredentials = UseSystemCredentials;
}


// InteractionReplaceExistingData Implementation.


// XInterface methods.


// virtual
void SAL_CALL InteractionReplaceExistingData::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionReplaceExistingData::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionReplaceExistingData::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< ucb::XInteractionReplaceExistingData * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}


// XTypeProvider methods.


// virtual
uno::Sequence< sal_Int8 > SAL_CALL
InteractionReplaceExistingData::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionReplaceExistingData::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<ucb::XInteractionReplaceExistingData>::get() );

    return s_aCollection.getTypes();
}


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionReplaceExistingData::select()
{
    recordSelection();
}

// InteractionAuthFallback Implementation

// XInterface methods.

// virtual
void SAL_CALL InteractionAuthFallback::acquire()
    throw()
{
    OWeakObject::acquire();
}

// virtual
void SAL_CALL InteractionAuthFallback::release()
    throw()
{
    OWeakObject::release();
}

// virtual
uno::Any SAL_CALL
InteractionAuthFallback::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
            static_cast< task::XInteractionContinuation * >( this ),
            static_cast< ucb::XInteractionAuthFallback * >( this ));

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}

// XInteractionContinuation methods.

// virtual
void SAL_CALL InteractionAuthFallback::select()
{
    recordSelection();
}

// XInteractionAuthFallback methods

// virtual
void SAL_CALL InteractionAuthFallback::setCode( const OUString& code )
{
    m_aCode = code;
}

const OUString& InteractionAuthFallback::getCode() const
{
    return m_aCode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
