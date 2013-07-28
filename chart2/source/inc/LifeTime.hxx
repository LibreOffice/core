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
#ifndef _LIFETIME_HXX
#define _LIFETIME_HXX

#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/weakref.hxx>
#include "charttoolsdllapi.hxx"

namespace apphelper
{

class LifeTimeGuard;
class LifeTimeManager
{
friend class LifeTimeGuard;
protected:
    mutable ::osl::Mutex                    m_aAccessMutex;
public:
OOO_DLLPUBLIC_CHARTTOOLS    LifeTimeManager( ::com::sun::star::lang::XComponent* pComponent, sal_Bool bLongLastingCallsCancelable = sal_False );
OOO_DLLPUBLIC_CHARTTOOLS    virtual ~LifeTimeManager();

OOO_DLLPUBLIC_CHARTTOOLS    bool        impl_isDisposed( bool bAssert=true );
OOO_DLLPUBLIC_CHARTTOOLS    sal_Bool    dispose() throw(::com::sun::star::uno::RuntimeException);

public:
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aListenerContainer;

protected:
    virtual sal_Bool    impl_canStartApiCall();
    virtual void        impl_apiCallCountReachedNull(){}

    void        impl_registerApiCall(sal_Bool bLongLastingCall);
    void        impl_unregisterApiCall(sal_Bool bLongLastingCall);

    void        impl_init();

protected:
    ::com::sun::star::lang::XComponent*     m_pComponent;

    ::osl::Condition        m_aNoAccessCountCondition;
    sal_Int32 volatile      m_nAccessCount;

    sal_Bool volatile       m_bDisposed;
    sal_Bool volatile       m_bInDispose;

    sal_Bool                m_bLongLastingCallsCancelable;
    ::osl::Condition        m_aNoLongLastingCallCountCondition;
    sal_Int32 volatile      m_nLongLastingCallCount;
};

class CloseableLifeTimeManager : public LifeTimeManager
{
protected:
    ::com::sun::star::util::XCloseable*         m_pCloseable;

    ::osl::Condition        m_aEndTryClosingCondition;
    sal_Bool volatile       m_bClosed;
    sal_Bool volatile       m_bInTryClose;
    //the ownership between model and controller is not clear at first
    //each controller might consider him as owner of the model first
    //at start the model is not considered as owner of itself
    sal_Bool volatile       m_bOwnership;
    //with a XCloseable::close call and during XCloseListener::queryClosing
    //the ownership can be regulated more explicit,
    //if so the ownership is considered to be well known
    sal_Bool volatile       m_bOwnershipIsWellKnown;

public:
OOO_DLLPUBLIC_CHARTTOOLS    CloseableLifeTimeManager( ::com::sun::star::util::XCloseable* pCloseable
        , ::com::sun::star::lang::XComponent* pComponent
        , sal_Bool bLongLastingCallsCancelable = sal_False );
OOO_DLLPUBLIC_CHARTTOOLS    virtual ~CloseableLifeTimeManager();

OOO_DLLPUBLIC_CHARTTOOLS    bool        impl_isDisposedOrClosed( bool bAssert=true );
OOO_DLLPUBLIC_CHARTTOOLS    sal_Bool    g_close_startTryClose(sal_Bool bDeliverOwnership)
                    throw ( ::com::sun::star::uno::Exception );
OOO_DLLPUBLIC_CHARTTOOLS    sal_Bool    g_close_isNeedToCancelLongLastingCalls( sal_Bool bDeliverOwnership, ::com::sun::star::util::CloseVetoException& ex )
                    throw ( ::com::sun::star::util::CloseVetoException );
OOO_DLLPUBLIC_CHARTTOOLS    void        g_close_endTryClose(sal_Bool bDeliverOwnership, sal_Bool bMyVeto );
OOO_DLLPUBLIC_CHARTTOOLS    void        g_close_endTryClose_doClose();
OOO_DLLPUBLIC_CHARTTOOLS    sal_Bool    g_addCloseListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::util::XCloseListener > & xListener )
                    throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual sal_Bool    impl_canStartApiCall();
    virtual void        impl_apiCallCountReachedNull();

    void        impl_setOwnership( sal_Bool bDeliverOwnership, sal_Bool bMyVeto );
    sal_Bool    impl_shouldCloseAtNextChance();
    void        impl_doClose();

    void        impl_init()
    {
        m_bClosed = sal_False;
        m_bInTryClose = sal_False;
        m_bOwnership = sal_False;
        m_bOwnershipIsWellKnown = sal_False;
        m_aEndTryClosingCondition.set();
    }
};

/*
Use this Guard in your apicalls to protect access on resources
which will be released in dispose.
It's guarantied, that the release of resources only starts if your
guarded call has finished.
! It's only partly guaranteed that this resources will not change during the call.
See the example for details.

This class is to be used as described in the example.

If this guard is used in all api calls of an XCloseable object
it's guarantied, that the closeable will close itself after finishing the last call
if it should do so.

  ::ApiCall
{
    //hold no mutex!!!
    LifeTimeGuard aLifeTimeGuard(m_aLifeTimeManager);

    //mutex is acquired; call is not registered

    if(!aLifeTimeGuard.startApiCall())
        return ; //behave as passive as possible, if disposed or closed

    //mutex is acquired, call is registered
    {
        //you might access some private members here
        //but than you need to protect access to these members always like this
        //never call to the outside here
    }

    aLifeTimeGuard.clear(); //!!!

      //Mutex is released, the running call is still registered
    //this call will finish before the 'release-section' in dispose is allowed to start

    {
        //you might access some private members here guarded with your own mutex
        //but release your mutex at the end of this block
    }

    //you can call to the outside (without holding the mutex) without becoming disposed

    //End of method -> ~LifeTimeGuard
    //-> call is unregistered
    //-> this object might be disposed now
}

your XComponent::dispose method has to be implemented in the following way:

    ::dispose()
{
    //hold no mutex!!!
    if( !m_aLifeTimeManager.dispose() )
        return;

    //--release all resources and references
    //...
}

*/

class OOO_DLLPUBLIC_CHARTTOOLS LifeTimeGuard
{

public:
    LifeTimeGuard( LifeTimeManager& rManager )
        : m_guard( rManager.m_aAccessMutex )
        , m_rManager(rManager)
        , m_bCallRegistered(sal_False)
        , m_bLongLastingCallRegistered(sal_False)
    {

    }
    sal_Bool startApiCall(sal_Bool bLongLastingCall=sal_False);
    ~LifeTimeGuard();
    void clear() { m_guard.clear(); }

private:
    osl::ClearableMutexGuard m_guard;
    LifeTimeManager&    m_rManager;
    sal_Bool            m_bCallRegistered;
    sal_Bool            m_bLongLastingCallRegistered;

private:
    // these make no sense
    LifeTimeGuard( ::osl::Mutex& rMutex );
    LifeTimeGuard( const LifeTimeGuard& );
    LifeTimeGuard& operator= ( const LifeTimeGuard& );
};

template<class T>
class NegativeGuard
{
protected:
    T * m_pT;
public:

    NegativeGuard(T * pT) : m_pT(pT)
    {
        m_pT->release();
    }

    NegativeGuard(T & t) : m_pT(&t)
    {
        m_pT->release();
    }

    ~NegativeGuard()
    {
        m_pT->acquire();
    }
};

}//end namespace apphelper
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
