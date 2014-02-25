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
#include <osl/mutex.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <ucbhelper/interactionrequest.hxx>

using namespace com::sun::star;
using namespace ucbhelper;




// InteractionRequest Implementation.




namespace ucbhelper
{

struct InteractionRequest_Impl
{
    rtl::Reference< InteractionContinuation > m_xSelection;
    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > m_aContinuations;

    InteractionRequest_Impl() {}
    InteractionRequest_Impl( const uno::Any & rRequest )
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
    delete m_pImpl;
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


rtl::Reference< InteractionContinuation >
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
    throw ( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionRequest * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



// XTypeProvider methods.



// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionRequest::getImplementationId()
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionRequest::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< task::XInteractionRequest > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionRequest methods.



// virtual
uno::Any SAL_CALL InteractionRequest::getRequest()
    throw( uno::RuntimeException )
{
    return m_pImpl->m_aRequest;
}


// virtual
uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL
InteractionRequest::getContinuations()
    throw( uno::RuntimeException )
{
    return m_pImpl->m_aContinuations;
}




// InteractionContinuation Implementation.




namespace ucbhelper
{

struct InteractionContinuation_Impl
{
    InteractionRequest * m_pRequest;

    InteractionContinuation_Impl( InteractionRequest * pRequest )
    : m_pRequest( pRequest ) {}
};

}


InteractionContinuation::InteractionContinuation(
                        InteractionRequest * pRequest )
: m_pImpl( new InteractionContinuation_Impl( pRequest ) )
{
}


// virtual
InteractionContinuation::~InteractionContinuation()
{
    delete m_pImpl;
}


void InteractionContinuation::recordSelection()
{
    m_pImpl->m_pRequest->setSelection( this );
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
    throw ( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionAbort::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< task::XInteractionAbort > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionAbort::select()
    throw( uno::RuntimeException )
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
    throw ( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionRetry::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< task::XInteractionRetry > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionRetry::select()
    throw( uno::RuntimeException )
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
    throw ( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionApprove::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< task::XInteractionApprove > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionApprove::select()
    throw( uno::RuntimeException )
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
    throw ( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionDisapprove::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< task::XInteractionDisapprove > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionDisapprove::select()
    throw( uno::RuntimeException )
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
    throw ( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionSupplyAuthentication::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference<
                        ucb::XInteractionSupplyAuthentication2 > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionSupplyAuthentication::select()
    throw( uno::RuntimeException )
{
    recordSelection();
}



// XInteractionSupplyAuthentication methods.



// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetRealm()
    throw( uno::RuntimeException )
{
    return m_bCanSetRealm;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setRealm( const OUString& Realm )
    throw( uno::RuntimeException )
{
    OSL_ENSURE( m_bCanSetPassword,
        "InteractionSupplyAuthentication::setRealm - Not supported!" );

    if ( m_bCanSetRealm )
        m_aRealm = Realm;
}


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetUserName()
    throw( uno::RuntimeException )
{
    return m_bCanSetUserName;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setUserName( const OUString& UserName )
    throw( uno::RuntimeException )
{
    OSL_ENSURE( m_bCanSetUserName,
        "InteractionSupplyAuthentication::setUserName - Not supported!" );

    if ( m_bCanSetUserName )
        m_aUserName = UserName;
}


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetPassword()
    throw( uno::RuntimeException )
{
    return m_bCanSetPassword;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setPassword( const OUString& Password )
    throw( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    Default = m_eDefaultRememberPasswordMode;
    return m_aRememberPasswordModes;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setRememberPassword(
                                    ucb::RememberAuthentication Remember )
    throw( uno::RuntimeException )
{
    m_eRememberPasswordMode = Remember;
}


// virtual
sal_Bool SAL_CALL
InteractionSupplyAuthentication::canSetAccount()
    throw( uno::RuntimeException )
{
    return m_bCanSetAccount;
}


// virtual
void SAL_CALL
InteractionSupplyAuthentication::setAccount( const OUString& Account )
    throw( uno::RuntimeException )
{
    OSL_ENSURE( m_bCanSetAccount,
        "InteractionSupplyAuthentication::setAccount - Not supported!" );

    if ( m_bCanSetAccount )
        m_aAccount = Account;
}


// virtual
uno::Sequence< ucb::RememberAuthentication > SAL_CALL
InteractionSupplyAuthentication::getRememberAccountModes(
                                    ucb::RememberAuthentication& Default )
    throw( uno::RuntimeException )
{
    Default = m_eDefaultRememberAccountMode;
    return m_aRememberAccountModes;
}


// virtual
void SAL_CALL InteractionSupplyAuthentication::setRememberAccount(
                                    ucb::RememberAuthentication Remember )
    throw( uno::RuntimeException )
{
    m_eRememberAccountMode = Remember;
}



// XInteractionSupplyAuthentication2 methods.



// virtual
::sal_Bool SAL_CALL
InteractionSupplyAuthentication::canUseSystemCredentials(
        ::sal_Bool& Default )
    throw ( uno::RuntimeException )
{
    Default = m_bDefaultUseSystemCredentials;
    return m_bCanUseSystemCredentials;
}


// virtual
void SAL_CALL InteractionSupplyAuthentication::setUseSystemCredentials(
        ::sal_Bool UseSystemCredentials )
    throw ( uno::RuntimeException )
{
    if ( m_bCanUseSystemCredentials )
        m_bUseSystemCredentials = UseSystemCredentials;
}





// InteractionSupplyName Implementation.






// XInterface methods.



// virtual
void SAL_CALL InteractionSupplyName::acquire()
    throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL InteractionSupplyName::release()
    throw()
{
    OWeakObject::release();
}


// virtual
uno::Any SAL_CALL
InteractionSupplyName::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< ucb::XInteractionSupplyName * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}



// XTypeProvider methods.



// virtual
uno::Sequence< sal_Int8 > SAL_CALL InteractionSupplyName::getImplementationId()
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionSupplyName::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference< ucb::XInteractionSupplyName > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionSupplyName::select()
    throw( uno::RuntimeException )
{
    recordSelection();
}



// XInteractionSupplyName methods.



// virtual
void SAL_CALL
InteractionSupplyName::setName( const OUString& Name )
    throw( uno::RuntimeException )
{
    m_aName = Name;
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
    throw ( uno::RuntimeException )
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
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( false );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}


// virtual
uno::Sequence< uno::Type > SAL_CALL InteractionReplaceExistingData::getTypes()
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast<
                    uno::Reference< lang::XTypeProvider > * >( 0 ) ),
                getCppuType( static_cast<
                    uno::Reference<
                        ucb::XInteractionReplaceExistingData > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XInteractionContinuation methods.



// virtual
void SAL_CALL InteractionReplaceExistingData::select()
    throw( uno::RuntimeException )
{
    recordSelection();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
