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

#include "sal/config.h"

#include <boost/scoped_ptr.hpp>
#include <cppuhelper/compbase1.hxx>

#include <tools/debug.hxx>

#include <vcl/svapp.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <salsession.hxx>

#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XSessionManagerListener2.hpp>

#include <list>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;


SalSession::~SalSession()
{
}

class VCLSession:
    private osl::Mutex,
    public cppu::WeakComponentImplHelper1 < XSessionManagerClient >
{
    struct Listener
    {
        css::uno::Reference< XSessionManagerListener >      m_xListener;
        bool                                        m_bInteractionRequested;
        bool                                        m_bInteractionDone;
        bool                                        m_bSaveDone;

        Listener( const css::uno::Reference< XSessionManagerListener >& xListener )
                : m_xListener( xListener ),
                  m_bInteractionRequested( false ),
                  m_bInteractionDone( false ),
                  m_bSaveDone( false )
        {}
    };

    std::list< Listener >                           m_aListeners;
    boost::scoped_ptr< SalSession >                 m_pSession;
    bool                                            m_bInteractionRequested;
    bool                                            m_bInteractionGranted;
    bool                                            m_bInteractionDone;
    bool                                            m_bSaveDone;

    static void SalSessionEventProc( void* pData, SalSessionEvent* pEvent );

    virtual ~VCLSession() {}

    virtual void SAL_CALL addSessionManagerListener( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual void SAL_CALL removeSessionManagerListener( const css::uno::Reference< XSessionManagerListener>& xListener ) throw( RuntimeException );
    virtual void SAL_CALL queryInteraction( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual void SAL_CALL interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual void SAL_CALL saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual sal_Bool SAL_CALL cancelShutdown() throw( RuntimeException );

    void callSaveRequested( bool bShutdown, bool bCancelable );
    void callShutdownCancelled();
    void callInteractionGranted( bool bGranted );
    void callQuit();

public:
    VCLSession();
};

VCLSession::VCLSession()
        : cppu::WeakComponentImplHelper1< XSessionManagerClient >( *static_cast< osl::Mutex * >(this) ),
          m_pSession( ImplGetSVData()->mpDefInst->CreateSalSession() ),
          m_bInteractionRequested( false ),
          m_bInteractionGranted( false ),
          m_bInteractionDone( false ),
          m_bSaveDone( false )
{
    if( m_pSession )
        m_pSession->SetCallback( SalSessionEventProc, this );
}

void VCLSession::callSaveRequested( bool bShutdown, bool bCancelable )
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( *this );
        // reset listener states
        for( std::list< Listener >::iterator it = m_aListeners.begin();
             it != m_aListeners.end(); ++it )
        {
            it->m_bSaveDone = it->m_bInteractionRequested = it->m_bInteractionDone = false;
        }

        // copy listener list since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bSaveDone = false;
        m_bInteractionDone = false;
        // without session we assume UI is always possible,
        // so it was reqeusted and granted
        m_bInteractionRequested = m_bInteractionGranted = !m_pSession;

        // answer the session manager even if no listeners available anymore
        DBG_ASSERT( ! aListeners.empty(), "saveRequested but no listeners !" );
        if( aListeners.empty() )
        {
            if( m_pSession )
                m_pSession->saveDone();
            return;
        }
    }

    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
        it->m_xListener->doSave( bShutdown, bCancelable );
    Application::AcquireSolarMutex( nAcquireCount );
}

void VCLSession::callInteractionGranted( bool bInteractionGranted )
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( *this );
        // copy listener list since calling a listener may remove it.
        for( std::list< Listener >::const_iterator it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
            if( it->m_bInteractionRequested )
                aListeners.push_back( *it );

        m_bInteractionGranted = bInteractionGranted;

        // answer the session manager even if no listeners available anymore
        DBG_ASSERT( ! aListeners.empty(), "interactionGranted but no listeners !" );
        if( aListeners.empty() )
        {
            if( m_pSession )
                m_pSession->interactionDone();
            return;
        }
    }

    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
        it->m_xListener->approveInteraction( bInteractionGranted );

    Application::AcquireSolarMutex( nAcquireCount );
}

void VCLSession::callShutdownCancelled()
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( *this );
        // copy listener list since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bInteractionRequested = m_bInteractionDone = m_bInteractionGranted = false;
    }

    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
        it->m_xListener->shutdownCanceled();
    Application::AcquireSolarMutex( nAcquireCount );
}

void VCLSession::callQuit()
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( *this );
        // copy listener list since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bInteractionRequested = m_bInteractionDone = m_bInteractionGranted = false;
    }

    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
    {
        css::uno::Reference< XSessionManagerListener2 > xListener2( it->m_xListener, UNO_QUERY );
        if( xListener2.is() )
            xListener2->doQuit();
    }
    Application::AcquireSolarMutex( nAcquireCount );
}

void VCLSession::SalSessionEventProc( void* pData, SalSessionEvent* pEvent )
{
    VCLSession * pThis = static_cast< VCLSession * >( pData );
    switch( pEvent->m_eType )
    {
        case Interaction:
        {
            SalSessionInteractionEvent* pIEv = static_cast<SalSessionInteractionEvent*>(pEvent);
            pThis->callInteractionGranted( pIEv->m_bInteractionGranted );
        }
        break;
        case SaveRequest:
        {
            SalSessionSaveRequestEvent* pSEv = static_cast<SalSessionSaveRequestEvent*>(pEvent);
            pThis->callSaveRequested( pSEv->m_bShutdown, pSEv->m_bCancelable );
        }
        break;
        case ShutdownCancel:
            pThis->callShutdownCancelled();
            break;
        case Quit:
            pThis->callQuit();
            break;
    }
}

void SAL_CALL VCLSession::addSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException )
{
    osl::MutexGuard aGuard( *this );

    m_aListeners.push_back( Listener( xListener ) );
}

void SAL_CALL VCLSession::removeSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException )
{
    osl::MutexGuard aGuard( *this );

    std::list< Listener >::iterator it = m_aListeners.begin();
    while( it != m_aListeners.end() )
    {
        if( it->m_xListener == xListener )
        {
            it = m_aListeners.erase(it);
        }
        else
            ++it;
    }
}

void SAL_CALL VCLSession::queryInteraction( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException )
{
    if( m_bInteractionGranted )
    {
        if( m_bInteractionDone )
            xListener->approveInteraction( false );
        else
            xListener->approveInteraction( true );
        return;
    }

    osl::MutexGuard aGuard( *this );
    if( ! m_bInteractionRequested )
    {
        m_pSession->queryInteraction();
        m_bInteractionRequested = true;
    }
    for( std::list< Listener >::iterator it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
    {
        if( it->m_xListener == xListener )
        {
            it->m_bInteractionRequested = true;
            it->m_bInteractionDone      = false;
        }
    }
}

void SAL_CALL VCLSession::interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException )
{
    osl::MutexGuard aGuard( *this );
    int nRequested = 0, nDone = 0;
    for( std::list< Listener >::iterator it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
    {
        if( it->m_bInteractionRequested )
        {
            nRequested++;
            if( xListener == it->m_xListener )
                it->m_bInteractionDone = true;
        }
        if( it->m_bInteractionDone )
            nDone++;
    }
    if( nDone == nRequested && nDone > 0 )
    {
        m_bInteractionDone = true;
        if( m_pSession )
            m_pSession->interactionDone();
    }
}

void SAL_CALL VCLSession::saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException )
{
    osl::MutexGuard aGuard( *this );

    bool bSaveDone = true;
    for( std::list< Listener >::iterator it = m_aListeners.begin();
         it != m_aListeners.end(); ++it )
    {
        if( it->m_xListener == xListener )
            it->m_bSaveDone = true;
        if( ! it->m_bSaveDone )
            bSaveDone = false;
    }
    if( bSaveDone )
    {
        m_bSaveDone = true;
        if( m_pSession )
            m_pSession->saveDone();
    }
}

sal_Bool SAL_CALL VCLSession::cancelShutdown() throw( RuntimeException )
{
    return m_pSession && m_pSession->cancelShutdown();
}

// service implementation

OUString SAL_CALL vcl_session_getImplementationName()
{
    return OUString( "com.sun.star.frame.VCLSessionManagerClient" );
}

Sequence< OUString > SAL_CALL vcl_session_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = "com.sun.star.frame.SessionManagerClient";
    return aRet;
}

css::uno::Reference< XInterface > SAL_CALL vcl_session_createInstance( SAL_UNUSED_PARAMETER const css::uno::Reference< XMultiServiceFactory > & )
{
    return static_cast< cppu::OWeakObject * >(new VCLSession);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
