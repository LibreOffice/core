/*************************************************************************
 *
 *  $RCSfile: interactionrequest.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-16 14:54:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#define _UCBHELPER_INTERATIONREQUEST_HXX

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONREPLACEEXISTINGDATA_HPP_
#include <com/sun/star/ucb/XInteractionReplaceExistingData.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYAUTHENTICATION_HPP_
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYNAME_HPP_
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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
class InteractionRequest : public cppu::OWeakObject,
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
class InteractionContinuation : public cppu::OWeakObject
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
class InteractionAbort : public InteractionContinuation,
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
class InteractionRetry : public InteractionContinuation,
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
class InteractionApprove : public InteractionContinuation,
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
class InteractionDisapprove : public InteractionContinuation,
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
class InteractionSupplyAuthentication :
                  public InteractionContinuation,
                  public com::sun::star::lang::XTypeProvider,
                  public com::sun::star::ucb::XInteractionSupplyAuthentication
{
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication >
                  m_aRememberPasswordModes;
    com::sun::star::uno::Sequence< com::sun::star::ucb::RememberAuthentication >
                  m_aRememberAccountModes;
    rtl::OUString m_aRealm;
    rtl::OUString m_aUserName;
    rtl::OUString m_aPassword;
    rtl::OUString m_aAccount;
    com::sun::star::ucb::RememberAuthentication m_eRememberPasswordMode;
    com::sun::star::ucb::RememberAuthentication m_eDefaultRememberPasswordMode;
    com::sun::star::ucb::RememberAuthentication m_eRememberAccountMode;
    com::sun::star::ucb::RememberAuthentication m_eDefaultRememberAccountMode;
    unsigned m_bCanSetRealm    : 1;
    unsigned m_bCanSetUserName : 1;
    unsigned m_bCanSetPassword : 1;
    unsigned m_bCanSetAccount  : 1;

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
                    sal_Bool bCanSetAccount );
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
                        eDefaultRememberAccountMode );

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
    setRealm( const rtl::OUString& Realm )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    canSetUserName()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setUserName( const rtl::OUString& UserName )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    canSetPassword()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setPassword( const rtl::OUString& Password )
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
    setAccount( const rtl::OUString& Account )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::RememberAuthentication > SAL_CALL
    getRememberAccountModes(
            com::sun::star::ucb::RememberAuthentication& Default )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setRememberAccount( com::sun::star::ucb::RememberAuthentication Remember )
        throw( com::sun::star::uno::RuntimeException );

    // Non-interface methods.

    /**
      * This method returns the realm that was supplied by the interaction
      * handler.
      *
      * @return the realm.
      */
    const rtl::OUString & getRealm()    const { return m_aRealm; }

    /**
      * This method returns the username that was supplied by the interaction
      * handler.
      *
      * @return the username.
      */
    const rtl::OUString & getUserName() const { return m_aUserName; }

    /**
      * This method returns the password that was supplied by the interaction
      * handler.
      *
      * @return the password.
      */
    const rtl::OUString & getPassword() const { return m_aPassword; }

    /**
      * This method returns the account that was supplied by the interaction
      * handler.
      *
      * @return the account.
      */
    const rtl::OUString & getAccount()  const { return m_aAccount; }

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
  m_eDefaultRememberPasswordMode(
                com::sun::star::ucb::RememberAuthentication_NO ),
  m_aRememberAccountModes( com::sun::star::uno::Sequence<
                com::sun::star::ucb::RememberAuthentication >( 1 ) ),
  m_eDefaultRememberAccountMode(
                com::sun::star::ucb::RememberAuthentication_NO ),
  m_eRememberPasswordMode( com::sun::star::ucb::RememberAuthentication_NO ),
  m_eRememberAccountMode( com::sun::star::ucb::RememberAuthentication_NO ),
  m_bCanSetRealm( bCanSetRealm ),
  m_bCanSetUserName( bCanSetUserName ),
  m_bCanSetPassword( bCanSetPassword ),
  m_bCanSetAccount( bCanSetAccount )
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
        eDefaultRememberAccountMode )
: InteractionContinuation( pRequest ),
  m_aRememberPasswordModes( rRememberPasswordModes ),
  m_eDefaultRememberPasswordMode( eDefaultRememberPasswordMode ),
  m_aRememberAccountModes( rRememberAccountModes ),
  m_eDefaultRememberAccountMode( eDefaultRememberAccountMode ),
  m_eRememberPasswordMode( eDefaultRememberPasswordMode ),
  m_eRememberAccountMode( eDefaultRememberAccountMode ),
  m_bCanSetRealm( bCanSetRealm ),
  m_bCanSetUserName( bCanSetUserName ),
  m_bCanSetPassword( bCanSetPassword ),
  m_bCanSetAccount( bCanSetAccount )
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
    rtl::OUString m_aName;

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
    virtual void SAL_CALL setName( const ::rtl::OUString& Name )
        throw ( com::sun::star::uno::RuntimeException );

    // Non-interface methods.

    /**
      * This method returns the name that was supplied by the interaction
      * handler.
      *
      * @return the name.
      */
    const rtl::OUString & getName() const { return m_aName; }
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
