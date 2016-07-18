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

#ifndef INCLUDED_UCBHELPER_INTERACTIONREQUEST_HXX
#define INCLUDED_UCBHELPER_INTERACTIONREQUEST_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/ucb/XInteractionAuthFallback.hpp>
#include <com/sun/star/ucb/XInteractionReplaceExistingData.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp>
#include <rtl/ref.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/ucbhelperdllapi.h>
#include <memory>

namespace ucbhelper {

class InteractionContinuation;


struct InteractionRequest_Impl;

/**
  * This class implements the interface XInteractionRequest. Instances can
  * be passed directly to XInteractionHandler::handle(...). Each interaction
  * request contains an exception describing the error and a number of
  * interaction continuations describing the possible "answers" for the request.
  * After the request was passed to XInteractionHandler::handle(...) the method
  * getSelection() returns the continuation chosen by the interaction handler.
  *
  * The typical usage of this class would be:
  *
  * 1) Create exception object that shall be handled by the interaction handler.
  * 2) Create InteractionRequest, supply exception as ctor parameter
  * 3) Create continuations needed and add them to a sequence
  * 4) Supply the continuations to the InteractionRequest by calling
  *    setContinuations(...)
  *
  * This class can also be used as base class for more specialized requests,
  * like authentication requests.
  */
class UCBHELPER_DLLPUBLIC InteractionRequest : public cppu::OWeakObject,
                           public css::lang::XTypeProvider,
                           public css::task::XInteractionRequest
{
    std::unique_ptr<InteractionRequest_Impl> m_pImpl;

protected:
    void setRequest( const css::uno::Any & rRequest );

    InteractionRequest();
    virtual ~InteractionRequest();

public:
    /**
      * Constructor.
      *
      * @param rRequest is the exception describing the error.
      */
    InteractionRequest( const css::uno::Any & rRequest );

    /**
      * This method sets the continuations for the request.
      *
      * @param rContinuations contains the possible continuations.
      */
    void setContinuations(
        const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > & rContinuations );

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionRequest
    virtual css::uno::Any SAL_CALL
    getRequest()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL
    getContinuations()
        throw( css::uno::RuntimeException, std::exception ) override;

    // Non-interface methods.

    /**
      * After passing this request to XInteractionHandler::handle, this method
      * returns the continuation that was chosen by the interaction handler.
      *
      * @return the continuation chosen by an interaction handler or an empty
      *         reference, if the request was not (yet) handled.
      */
    rtl::Reference< InteractionContinuation > const & getSelection() const;

    /**
      * This method sets a continuation for the request. It also can be used
      * to reset the continuation set by a previous XInteractionHandler::handle
      * call in order to use this request object more than once.
      *
      * @param rxSelection is the interaction continuation to activate for
      *        the request or an empty reference in order to reset the
      *        current selection.
      */
    void
    setSelection(
        const rtl::Reference< InteractionContinuation > & rxSelection );
};


struct InteractionContinuation_Impl;

/**
  * This class is the base for implementations of the interface
  * XInteractionContinuation. Classes derived from this bas class work together
  * with class InteractionRequest.
  *
  * Derived classes must implement their XInteractionContinuation::select()
  * method the way that they simply call recordSelection() which is provided by
  * this class.
  */
class UCBHELPER_DLLPUBLIC InteractionContinuation : public cppu::OWeakObject
{
    std::unique_ptr<InteractionContinuation_Impl> m_pImpl;

protected:
    /**
      * This method marks this continuation as "selected" at the request it
      * belongs to.
      *
      * Derived classes must implement their XInteractionContinuation::select()
      * method the way that they call this method.
      */
    void recordSelection();
    virtual ~InteractionContinuation();

public:
    InteractionContinuation( InteractionRequest * pRequest );
};


/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionAbort. Instances of this class can be passed
  * along with an interaction request to indicate the possibility to abort
  * the operation that caused the request.
  */
class UCBHELPER_DLLPUBLIC InteractionAbort : public InteractionContinuation,
                         public css::lang::XTypeProvider,
                         public css::task::XInteractionAbort
{
public:
    InteractionAbort( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;
};


/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionRetry. Instances of this class can be passed
  * along with an interaction request to indicate the possibility to retry
  * the operation that caused the request.
  */
class UCBHELPER_DLLPUBLIC InteractionRetry : public InteractionContinuation,
                         public css::lang::XTypeProvider,
                         public css::task::XInteractionRetry
{
public:
    InteractionRetry( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;
};


/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionApprove. Instances of this class can be passed
  * along with an interaction request to indicate the possibility to approve
  * the request.
  */
class UCBHELPER_DLLPUBLIC InteractionApprove : public InteractionContinuation,
                           public css::lang::XTypeProvider,
                           public css::task::XInteractionApprove
{
public:
    InteractionApprove( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;
};


/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionDisapprove. Instances of this class can be passed
  * along with an interaction request to indicate the possibility to disapprove
  * the request.
  */
class UCBHELPER_DLLPUBLIC InteractionDisapprove : public InteractionContinuation,
                              public css::lang::XTypeProvider,
                              public css::task::XInteractionDisapprove
{
public:
    InteractionDisapprove( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;
};


/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionSupplyAuthentication. Instances of this class can be
  * passed along with an authentication interaction request to enable the
  * interaction handler to supply the missing authentication data.
  */
class UCBHELPER_DLLPUBLIC InteractionSupplyAuthentication :
                  public InteractionContinuation,
                  public css::lang::XTypeProvider,
                  public css::ucb::XInteractionSupplyAuthentication2
{
    css::uno::Sequence< css::ucb::RememberAuthentication >
                  m_aRememberPasswordModes;
    css::uno::Sequence< css::ucb::RememberAuthentication >
                  m_aRememberAccountModes;
    OUString m_aRealm;
    OUString m_aUserName;
    OUString m_aPassword;
    OUString m_aAccount;
    css::ucb::RememberAuthentication m_eRememberPasswordMode;
    css::ucb::RememberAuthentication m_eDefaultRememberPasswordMode;
    css::ucb::RememberAuthentication m_eRememberAccountMode;
    css::ucb::RememberAuthentication m_eDefaultRememberAccountMode;
    bool m_bCanSetRealm    : 1;
    bool m_bCanSetUserName : 1;
    bool m_bCanSetPassword : 1;
    bool m_bCanSetAccount  : 1;
    bool m_bCanUseSystemCredentials     : 1;
    bool m_bUseSystemCredentials        : 1;

public:
    /**
      * Constructor.
      *
      * Note: The remember-authentication stuff is interesting only for
      *       clients implementing own password storage functionality.
      *
      * @param rxRequest is the interaction request that owns this continuation.
      * @param bCanSetRealm indicates, whether the realm given with the
      *        authentication request is read-only.
      * @param bCanSetUserName indicates, whether the username given with the
      *        authentication request is read-only.
      * @param bCanSetPassword indicates, whether the password given with the
      *        authentication request is read-only.
      * @param bCanSetAccount indicates, whether the account given with the
      *        authentication request is read-only.
      * @param rRememberPasswordModes specifies the authentication-remember-
      *        modes for passwords supported by the requesting client.
      * @param eDefaultRememberPasswordMode specifies the default
      *        authentication-remember-mode for passwords preferred by the
      *        requesting client.
      * @param rRememberAccountModes specifies the authentication-remember-
      *        modes for accounts supported by the requesting client.
      * @param eDefaultRememberAccountMode specifies the default
      *        authentication-remember-mode for accounts preferred by the
      *        requesting client.
      * @param bCanUseSystemCredentials indicates whether issuer of the
      *        authentication request can obtain and use system credentials
      *        for authentication.
      *
      * @see css::ucb::AuthenticationRequest
      * @see css::ucb::RememberAuthentication
      */
    inline InteractionSupplyAuthentication(
                    InteractionRequest * pRequest,
                    bool bCanSetRealm,
                    bool bCanSetUserName,
                    bool bCanSetPassword,
                    bool bCanSetAccount,
                    const css::uno::Sequence< css::ucb::RememberAuthentication > & rRememberPasswordModes,
                    const css::ucb::RememberAuthentication eDefaultRememberPasswordMode,
                    const css::uno::Sequence< css::ucb::RememberAuthentication > & rRememberAccountModes,
                    const css::ucb::RememberAuthentication  eDefaultRememberAccountMode,
                    bool bCanUseSystemCredentials );

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionSupplyAuthentication
    virtual sal_Bool SAL_CALL
    canSetRealm()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setRealm( const OUString& Realm )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    canSetUserName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setUserName( const OUString& UserName )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    canSetPassword()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setPassword( const OUString& Password )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence<
                css::ucb::RememberAuthentication > SAL_CALL
    getRememberPasswordModes(
            css::ucb::RememberAuthentication& Default )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setRememberPassword( css::ucb::RememberAuthentication Remember )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    canSetAccount()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setAccount( const OUString& Account )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< css::ucb::RememberAuthentication > SAL_CALL
    getRememberAccountModes(
            css::ucb::RememberAuthentication& Default )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setRememberAccount( css::ucb::RememberAuthentication Remember )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionSupplyAuthentication2
    virtual sal_Bool SAL_CALL canUseSystemCredentials( sal_Bool& Default )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setUseSystemCredentials( sal_Bool UseSystemCredentials )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // Non-interface methods.

    /**
      * This method returns the realm that was supplied by the interaction
      * handler.
      *
      * @return the realm.
      */
    const OUString & getRealm()    const { return m_aRealm; }

    /**
      * This method returns the username that was supplied by the interaction
      * handler.
      *
      * @return the username.
      */
    const OUString & getUserName() const { return m_aUserName; }

    /**
      * This method returns the password that was supplied by the interaction
      * handler.
      *
      * @return the password.
      */
    const OUString & getPassword() const { return m_aPassword; }

    /**
      * This method returns the authentication remember-mode for the password
      * that was supplied by the interaction handler.
      *
      * @return the remember-mode for the password.
      */
    const css::ucb::RememberAuthentication &
    getRememberPasswordMode() const { return m_eRememberPasswordMode; }

    bool getUseSystemCredentials() const { return m_bUseSystemCredentials; }
};



inline InteractionSupplyAuthentication::InteractionSupplyAuthentication(
    InteractionRequest * pRequest,
    bool bCanSetRealm,
    bool bCanSetUserName,
    bool bCanSetPassword,
    bool bCanSetAccount,
    const css::uno::Sequence< css::ucb::RememberAuthentication > & rRememberPasswordModes,
    const css::ucb::RememberAuthentication eDefaultRememberPasswordMode,
    const css::uno::Sequence< css::ucb::RememberAuthentication > & rRememberAccountModes,
    const css::ucb::RememberAuthentication eDefaultRememberAccountMode,
    bool bCanUseSystemCredentials )
: InteractionContinuation( pRequest ),
  m_aRememberPasswordModes( rRememberPasswordModes ),
  m_aRememberAccountModes( rRememberAccountModes ),
  m_eRememberPasswordMode( eDefaultRememberPasswordMode ),
  m_eDefaultRememberPasswordMode( eDefaultRememberPasswordMode ),
  m_eRememberAccountMode( eDefaultRememberAccountMode ),
  m_eDefaultRememberAccountMode( eDefaultRememberAccountMode ),
  m_bCanSetRealm( bCanSetRealm ),
  m_bCanSetUserName( bCanSetUserName ),
  m_bCanSetPassword( bCanSetPassword ),
  m_bCanSetAccount( bCanSetAccount ),
  m_bCanUseSystemCredentials( bCanUseSystemCredentials ),
  m_bUseSystemCredentials( false )
{
}


/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionReplaceExistingData. Instances of this class can be
  * passed along with an interaction request to indicate the possibility to
  * replace existing data.
  */
class InteractionReplaceExistingData :
                  public InteractionContinuation,
                  public css::lang::XTypeProvider,
                  public css::ucb::XInteractionReplaceExistingData
{
public:
    InteractionReplaceExistingData( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;
};

class UCBHELPER_DLLPUBLIC InteractionAuthFallback:
                  public InteractionContinuation,
                  public css::ucb::XInteractionAuthFallback
{
    OUString m_aCode;

public:
    InteractionAuthFallback( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XAuthFallback
    virtual void SAL_CALL setCode( const OUString& code )
        throw (::css::uno::RuntimeException, ::std::exception) override;
    const OUString& SAL_CALL getCode()
        throw (::css::uno::RuntimeException, ::std::exception);


};


} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_INTERACTIONREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
