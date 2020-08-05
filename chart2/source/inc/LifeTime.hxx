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
#pragma once

#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <cppuhelper/interfacecontainer.h>
#include "charttoolsdllapi.hxx"

namespace com::sun::star::lang { class XComponent; }
namespace com::sun::star::util { class CloseVetoException; }
namespace com::sun::star::util { class XCloseListener; }
namespace com::sun::star::util { class XCloseable; }

namespace apphelper
{

class OOO_DLLPUBLIC_CHARTTOOLS LifeTimeManager
{
friend class LifeTimeGuard;
protected:
    mutable ::osl::Mutex                    m_aAccessMutex;
public:
    LifeTimeManager( css::lang::XComponent* pComponent );
    virtual ~LifeTimeManager();

    bool        impl_isDisposed( bool bAssert=true );
    /// @throws css::uno::RuntimeException
    bool    dispose();

public:
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aListenerContainer;

protected:
    SAL_DLLPRIVATE virtual bool    impl_canStartApiCall();
    SAL_DLLPRIVATE virtual void        impl_apiCallCountReachedNull(){}

    SAL_DLLPRIVATE void        impl_registerApiCall(bool bLongLastingCall);
    SAL_DLLPRIVATE void        impl_unregisterApiCall(bool bLongLastingCall);

protected:
    css::lang::XComponent*     m_pComponent;

    ::osl::Condition        m_aNoAccessCountCondition;
    sal_Int32               m_nAccessCount;

    bool volatile       m_bDisposed;
    bool volatile       m_bInDispose;

    ::osl::Condition        m_aNoLongLastingCallCountCondition;
    sal_Int32               m_nLongLastingCallCount;
};

class CloseableLifeTimeManager final : public LifeTimeManager
{
    css::util::XCloseable*         m_pCloseable;

    ::osl::Condition    m_aEndTryClosingCondition;
    bool volatile       m_bClosed;
    bool volatile       m_bInTryClose;
    //the ownership between model and controller is not clear at first
    //each controller might consider him as owner of the model first
    //at start the model is not considered as owner of itself
    bool volatile       m_bOwnership;

public:
    CloseableLifeTimeManager( css::util::XCloseable* pCloseable
        , css::lang::XComponent* pComponent );
    virtual ~CloseableLifeTimeManager() override;

    bool    impl_isDisposedOrClosed( bool bAssert=true );
/// @throws css::uno::Exception
    bool    g_close_startTryClose(bool bDeliverOwnership);
/// @throws css::util::CloseVetoException
    void    g_close_isNeedToCancelLongLastingCalls( bool bDeliverOwnership, css::util::CloseVetoException const & ex );
    void    g_close_endTryClose(bool bDeliverOwnership );
    void    g_close_endTryClose_doClose();
/// @throws css::uno::RuntimeException
    void    g_addCloseListener( const css::uno::Reference< css::util::XCloseListener > & xListener );

private:
    virtual bool    impl_canStartApiCall() override;
    virtual void    impl_apiCallCountReachedNull() override;

    void        impl_setOwnership( bool bDeliverOwnership, bool bMyVeto );
    void        impl_doClose();
};

/*
Use this Guard in your ApiCalls to protect access on resources
which will be released in dispose.
It's guaranteed that the release of resources only starts if your
guarded call has finished.
! It's only partly guaranteed that this resources will not change during the call.
See the example for details.

This class is to be used as described in the example.

If this guard is used in all api calls of an XCloseable object
it's guaranteed that the closeable will close itself after finishing the last call
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
        //but then you need to protect access to these members always like this
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

}

*/

class LifeTimeGuard
{

public:
    LifeTimeGuard( LifeTimeManager& rManager )
        : m_guard( rManager.m_aAccessMutex )
        , m_rManager(rManager)
        , m_bCallRegistered(false)
        , m_bLongLastingCallRegistered(false)
    {

    }
    bool startApiCall(bool bLongLastingCall=false);
    ~LifeTimeGuard();
    void clear() { m_guard.clear(); }

private:
    osl::ClearableMutexGuard m_guard;
    LifeTimeManager&    m_rManager;
    bool            m_bCallRegistered;
    bool            m_bLongLastingCallRegistered;

private:
    LifeTimeGuard( const LifeTimeGuard& ) = delete;
    LifeTimeGuard& operator= ( const LifeTimeGuard& ) = delete;
};

template<class T>
class NegativeGuard final
{
    T * m_pT;
public:

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
