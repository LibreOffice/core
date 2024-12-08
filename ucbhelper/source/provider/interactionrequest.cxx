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

#include <ucbhelper/interactionrequest.hxx>

#include <rtl/ref.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <utility>

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
    explicit InteractionRequest_Impl( uno::Any aRequest )
    : m_aRequest(std::move( aRequest )) {}
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


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionAbort::select()
{
    recordSelection();
}


// InteractionRetry Implementation.


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionRetry::select()
{
    recordSelection();
}


// InteractionApprove Implementation.


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionApprove::select()
{
    recordSelection();
}


// InteractionDisapprove Implementation.


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionDisapprove::select()
{
    recordSelection();
}


// InteractionSupplyAuthentication Implementation.


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


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionReplaceExistingData::select()
{
    recordSelection();
}

// InteractionAuthFallback Implementation

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
