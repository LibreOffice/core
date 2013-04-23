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

#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#define _UCBHELPER_INTERATIONREQUEST_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/ucb/XInteractionReplaceExistingData.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp>
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#include <rtl/ref.hxx>
#include <cppuhelper/weak.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper {

class InteractionContinuation;

//============================================================================
struct InteractionRequest_Impl;

/**
  * This class implements the interface XInteractionRequest. Instances can
  * be passed directly to XInteractionHandler::handle(...). Each interaction
  * request contains an exception describing the error and a number of
  * interaction continuations describing the possible "answers" for the request.
  * After the request was passed to XInteractionHandler::handle(...) the method
  * getSelection() returns the continuation choosen by the interaction handler.
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
                           public com::sun::star::lang::XTypeProvider,
                           public com::sun::star::task::XInteractionRequest
{
    InteractionRequest_Impl * m_pImpl;

protected:
    void setRequest( const com::sun::star::uno::Any & rRequest );

    InteractionRequest();
    virtual ~InteractionRequest();

public:
    /**
      * Constructor.
      *
      * @param rRequest is the exception describing the error.
      */
    InteractionRequest( const com::sun::star::uno::Any & rRequest );

    /**
      * This method sets the continuations for the request.
      *
      * @param rContinuations contains the possible continuations.
      */
    void setContinuations(
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > &
                    rContinuations );

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionRequest
    virtual com::sun::star::uno::Any SAL_CALL
    getRequest()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence<
                com::sun::star::uno::Reference<
                    com::sun::star::task::XInteractionContinuation > > SAL_CALL
    getContinuations()
        throw( com::sun::star::uno::RuntimeException );

    // Non-interface methods.

    /**
      * After passing this request to XInteractionHandler::handle, this method
      * returns the continuation that was choosen by the interaction handler.
      *
      * @return the continuation choosen by an interaction handler or an empty
      *         reference, if the request was not (yet) handled.
      */
    rtl::Reference< InteractionContinuation > getSelection() const;

    /**
      * This method sets a continuation for the request. It also can be used
      * to reset the continuation set by a previous XInteractionHandler::handle
      * call in order to use this request object more then once.
      *
      * @param rxSelection is the interaction continuation to activate for
      *        the request or an empty reference in order to reset the
      *        current selection.
      */
    void
    setSelection(
        const rtl::Reference< InteractionContinuation > & rxSelection );
};

//============================================================================
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
    InteractionContinuation_Impl * m_pImpl;

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

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionAbort. Instances of this class can be passed
  * along with an interaction request to indicate the possiblity to abort
  * the operation that caused the request.
  */
class UCBHELPER_DLLPUBLIC InteractionAbort : public InteractionContinuation,
                         public com::sun::star::lang::XTypeProvider,
                         public com::sun::star::task::XInteractionAbort
{
public:
    InteractionAbort( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );
};

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionRetry. Instances of this class can be passed
  * along with an interaction request to indicate the possiblity to retry
  * the operation that caused the request.
  */
class UCBHELPER_DLLPUBLIC InteractionRetry : public InteractionContinuation,
                         public com::sun::star::lang::XTypeProvider,
                         public com::sun::star::task::XInteractionRetry
{
public:
    InteractionRetry( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );
};

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionApprove. Instances of this class can be passed
  * along with an interaction request to indicate the possiblity to approve
  * the request.
  */
class UCBHELPER_DLLPUBLIC InteractionApprove : public InteractionContinuation,
                           public com::sun::star::lang::XTypeProvider,
                           public com::sun::star::task::XInteractionApprove
{
public:
    InteractionApprove( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );
};

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionDisapprove. Instances of this class can be passed
  * along with an interaction request to indicate the possiblity to disapprove
  * the request.
  */
class UCBHELPER_DLLPUBLIC InteractionDisapprove : public InteractionContinuation,
                              public com::sun::star::lang::XTypeProvider,
                              public com::sun::star::task::XInteractionDisapprove
{
public:
    InteractionDisapprove( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );
};

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionSupplyAuthentication. Instances of this class can be
  * passed along with an authentication interaction request to enable the
  * interaction handler to supply the missing authentication data.
  */
class UCBHELPER_DLLPUBLIC InteractionSupplyAuthentication :
                  public InteractionContinuation,
                  public com::sun::star::lang::XTypeProvider,
                  public com::sun::star::ucb::XInteractionSupplyAuthentication2
{
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication >
                  m_aRememberPasswordModes;
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication >
                  m_aRememberAccountModes;
    OUString m_aRealm;
    OUString m_aUserName;
    OUString m_aPassword;
    OUString m_aAccount;
    com::sun::star::ucb::RememberAuthentication m_eRememberPasswordMode;
    com::sun::star::ucb::RememberAuthentication m_eDefaultRememberPasswordMode;
    com::sun::star::ucb::RememberAuthentication m_eRememberAccountMode;
    com::sun::star::ucb::RememberAuthentication m_eDefaultRememberAccountMode;
    unsigned m_bCanSetRealm    : 1;
    unsigned m_bCanSetUserName : 1;
    unsigned m_bCanSetPassword : 1;
    unsigned m_bCanSetAccount  : 1;
    unsigned m_bCanUseSystemCredentials     : 1;
    unsigned m_bDefaultUseSystemCredentials : 1;
    unsigned m_bUseSystemCredentials        : 1;

public:
    /**
      * Constructor.
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
      *
      * @see com::sun::star::ucb::AuthenticationRequest
      */
    inline InteractionSupplyAuthentication(
                    InteractionRequest * pRequest,
                    sal_Bool bCanSetRealm,
                    sal_Bool bCanSetUserName,
                    sal_Bool bCanSetPassword,
                    sal_Bool bCanSetAccount);
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
      *        authetication request can obtain and use system credentials
      *        for authentication.
      * @param bDefaultUseSystemCredentials specifies the default system
      *        credentials usage preferred by the requesting client
      *
      * @see com::sun::star::ucb::AuthenticationRequest
      * @see com::sun::star::ucb::RememberAuthentication
      */
    inline InteractionSupplyAuthentication(
                    InteractionRequest * pRequest,
                    sal_Bool bCanSetRealm,
                    sal_Bool bCanSetUserName,
                    sal_Bool bCanSetPassword,
                    sal_Bool bCanSetAccount,
                    const com::sun::star::uno::Sequence<
                        com::sun::star::ucb::RememberAuthentication > &
                            rRememberPasswordModes,
                    const com::sun::star::ucb::RememberAuthentication
                        eDefaultRememberPasswordMode,
                    const com::sun::star::uno::Sequence<
                        com::sun::star::ucb::RememberAuthentication > &
                            rRememberAccountModes,
                    const com::sun::star::ucb::RememberAuthentication
                        eDefaultRememberAccountMode,
                    sal_Bool bCanUseSystemCredentials,
                    sal_Bool bDefaultUseSystemCredentials );

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionSupplyAuthentication
    virtual sal_Bool SAL_CALL
    canSetRealm()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setRealm( const OUString& Realm )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    canSetUserName()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setUserName( const OUString& UserName )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    canSetPassword()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setPassword( const OUString& Password )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::RememberAuthentication > SAL_CALL
    getRememberPasswordModes(
            com::sun::star::ucb::RememberAuthentication& Default )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setRememberPassword( com::sun::star::ucb::RememberAuthentication Remember )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    canSetAccount()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setAccount( const OUString& Account )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::RememberAuthentication > SAL_CALL
    getRememberAccountModes(
            com::sun::star::ucb::RememberAuthentication& Default )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setRememberAccount( com::sun::star::ucb::RememberAuthentication Remember )
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionSupplyAuthentication2
    virtual ::sal_Bool SAL_CALL canUseSystemCredentials( ::sal_Bool& Default )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUseSystemCredentials( ::sal_Bool UseSystemCredentials )
        throw ( ::com::sun::star::uno::RuntimeException );

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
      * This method returns the account that was supplied by the interaction
      * handler.
      *
      * @return the account.
      */
    const OUString & getAccount()  const { return m_aAccount; }

    /**
      * This method returns the authentication remember-mode for the password
      * that was supplied by the interaction handler.
      *
      * @return the remember-mode for the password.
      */
    const com::sun::star::ucb::RememberAuthentication &
    getRememberPasswordMode() const { return m_eRememberPasswordMode; }

    /**
      * This method returns the authentication remember-mode for the account
      * that was supplied by the interaction handler.
      *
      * @return the remember-mode for the account.
      */
    const com::sun::star::ucb::RememberAuthentication &
    getRememberAccountMode() const { return m_eRememberAccountMode; }

    sal_Bool getUseSystemCredentials() const { return m_bUseSystemCredentials; }
};

//============================================================================
inline InteractionSupplyAuthentication::InteractionSupplyAuthentication(
                    InteractionRequest * pRequest,
                    sal_Bool bCanSetRealm,
                    sal_Bool bCanSetUserName,
                    sal_Bool bCanSetPassword,
                    sal_Bool bCanSetAccount )
: InteractionContinuation( pRequest ),
  m_aRememberPasswordModes( com::sun::star::uno::Sequence<
                com::sun::star::ucb::RememberAuthentication >( 1 ) ),
  m_aRememberAccountModes( com::sun::star::uno::Sequence<
                com::sun::star::ucb::RememberAuthentication >( 1 ) ),
  m_eRememberPasswordMode( com::sun::star::ucb::RememberAuthentication_NO ),
  m_eDefaultRememberPasswordMode(
                com::sun::star::ucb::RememberAuthentication_NO ),
  m_eRememberAccountMode( com::sun::star::ucb::RememberAuthentication_NO ),
  m_eDefaultRememberAccountMode(
                com::sun::star::ucb::RememberAuthentication_NO ),
  m_bCanSetRealm( bCanSetRealm ),
  m_bCanSetUserName( bCanSetUserName ),
  m_bCanSetPassword( bCanSetPassword ),
  m_bCanSetAccount( bCanSetAccount ),
  m_bCanUseSystemCredentials( sal_False ),
  m_bDefaultUseSystemCredentials( sal_False ),
  m_bUseSystemCredentials( sal_False )
{
    m_aRememberPasswordModes[ 0 ]
        = com::sun::star::ucb::RememberAuthentication_NO;
    m_aRememberAccountModes [ 0 ]
        = com::sun::star::ucb::RememberAuthentication_NO;
}

//============================================================================
inline InteractionSupplyAuthentication::InteractionSupplyAuthentication(
    InteractionRequest * pRequest,
    sal_Bool bCanSetRealm,
    sal_Bool bCanSetUserName,
    sal_Bool bCanSetPassword,
    sal_Bool bCanSetAccount,
    const com::sun::star::uno::Sequence<
        com::sun::star::ucb::RememberAuthentication > & rRememberPasswordModes,
    const com::sun::star::ucb::RememberAuthentication
        eDefaultRememberPasswordMode,
    const com::sun::star::uno::Sequence<
        com::sun::star::ucb::RememberAuthentication > & rRememberAccountModes,
    const com::sun::star::ucb::RememberAuthentication
        eDefaultRememberAccountMode,
    sal_Bool bCanUseSystemCredentials,
    sal_Bool bDefaultUseSystemCredentials )
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
  m_bDefaultUseSystemCredentials( bDefaultUseSystemCredentials ),
  m_bUseSystemCredentials( bDefaultUseSystemCredentials & bCanUseSystemCredentials )
{
}

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionSupplyName. Instances of this class can be passed
  * along with an interaction request to indicate the possiblity to
  * supply a new name.
  */
class InteractionSupplyName : public InteractionContinuation,
                              public com::sun::star::lang::XTypeProvider,
                              public com::sun::star::ucb::XInteractionSupplyName
{
    OUString m_aName;

public:
    InteractionSupplyName( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionSupplyName
    virtual void SAL_CALL setName( const OUString& Name )
        throw ( com::sun::star::uno::RuntimeException );

    // Non-interface methods.

    /**
      * This method returns the name that was supplied by the interaction
      * handler.
      *
      * @return the name.
      */
    const OUString & getName() const { return m_aName; }
};

//============================================================================
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionReplaceExistingData. Instances of this class can be
  * passed along with an interaction request to indicate the possiblity to
  * replace existing data.
  */
class InteractionReplaceExistingData :
                  public InteractionContinuation,
                  public com::sun::star::lang::XTypeProvider,
                  public com::sun::star::ucb::XInteractionReplaceExistingData
{
public:
    InteractionReplaceExistingData( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException );
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_INTERATIONREQUEST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
