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

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>

#include <tools/debug.hxx>

#include <vcl/svapp.hxx>

#include <factory.hxx>
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
    private cppu::BaseMutex,
    public cppu::WeakComponentImplHelper < XSessionManagerClient >
{
    struct Listener
    {
        css::uno::Reference< XSessionManagerListener >      m_xListener;
        bool                                        m_bInteractionRequested;
        bool                                        m_bInteractionDone;
        bool                                        m_bSaveDone;

        explicit Listener( const css::uno::Reference< XSessionManagerListener >& xListener )
                : m_xListener( xListener ),
                  m_bInteractionRequested( false ),
                  m_bInteractionDone( false ),
                  m_bSaveDone( false )
        {}
    };

    std::list< Listener >                           m_aListeners;
    std::unique_ptr< SalSession >                   m_xSession;
    bool                                            m_bInteractionRequested;
    bool                                            m_bInteractionGranted;
    bool                                            m_bInteractionDone;
    bool                                            m_bSaveDone;

    static void SalSessionEventProc( void* pData, SalSessionEvent* pEvent );

    virtual ~VCLSession() {}

    virtual void SAL_CALL addSessionManagerListener( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL removeSessionManagerListener( const css::uno::Reference< XSessionManagerListener>& xListener ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL queryInteraction( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL cancelShutdown() throw( RuntimeException, std::exception ) SAL_OVERRIDE;

    void callSaveRequested( bool bShutdown, bool bCancelable );
    void callShutdownCancelled();
    void callInteractionGranted( bool bGranted );
    void callQuit();

public:
    VCLSession();
};

VCLSession::VCLSession()
        : cppu::WeakComponentImplHelper< XSessionManagerClient >( m_aMutex ),
          m_xSession( ImplGetSVData()->mpDefInst->CreateSalSession() ),
          m_bInteractionRequested( false ),
          m_bInteractionGranted( false ),
          m_bInteractionDone( false ),
          m_bSaveDone( false )
{
    if (m_xSession)
        m_xSession->SetCallback( SalSessionEventProc, this );
}

void VCLSession::callSaveRequested( bool bShutdown, bool bCancelable )
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
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
        m_bInteractionRequested = m_bInteractionGranted = !m_xSession;

        // answer the session manager even if no listeners available anymore
        DBG_ASSERT( ! aListeners.empty(), "saveRequested but no listeners !" );
        if( aListeners.empty() )
        {
            if (m_xSession)
                m_xSession->saveDone();
            return;
        }
    }

    SolarMutexReleaser aReleaser;
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
        it->m_xListener->doSave( bShutdown, bCancelable );
}

void VCLSession::callInteractionGranted( bool bInteractionGranted )
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        // copy listener list since calling a listener may remove it.
        for( std::list< Listener >::const_iterator it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
            if( it->m_bInteractionRequested )
                aListeners.push_back( *it );

        m_bInteractionGranted = bInteractionGranted;

        // answer the session manager even if no listeners available anymore
        DBG_ASSERT( ! aListeners.empty(), "interactionGranted but no listeners !" );
        if( aListeners.empty() )
        {
            if (m_xSession)
                m_xSession->interactionDone();
            return;
        }
    }

    SolarMutexReleaser aReleaser;
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
        it->m_xListener->approveInteraction( bInteractionGranted );
}

void VCLSession::callShutdownCancelled()
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        // copy listener list since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bInteractionRequested = m_bInteractionDone = m_bInteractionGranted = false;
    }

    SolarMutexReleaser aReleaser;
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
        it->m_xListener->shutdownCanceled();
}

void VCLSession::callQuit()
{
    std::list< Listener > aListeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        // copy listener list since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bInteractionRequested = m_bInteractionDone = m_bInteractionGranted = false;
    }

    SolarMutexReleaser aReleaser;
    for( std::list< Listener >::const_iterator it = aListeners.begin(); it != aListeners.end(); ++it )
    {
        css::uno::Reference< XSessionManagerListener2 > xListener2( it->m_xListener, UNO_QUERY );
        if( xListener2.is() )
            xListener2->doQuit();
    }
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

void SAL_CALL VCLSession::addSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aListeners.push_back( Listener( xListener ) );
}

void SAL_CALL VCLSession::removeSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

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

void SAL_CALL VCLSession::queryInteraction( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException, std::exception )
{
    if( m_bInteractionGranted )
    {
        if( m_bInteractionDone )
            xListener->approveInteraction( false );
        else
            xListener->approveInteraction( true );
        return;
    }

    osl::MutexGuard aGuard( m_aMutex );
    if( ! m_bInteractionRequested )
    {
        m_xSession->queryInteraction();
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

void SAL_CALL VCLSession::interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );
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
        if (m_xSession)
            m_xSession->interactionDone();
    }
}

void SAL_CALL VCLSession::saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

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
        if (m_xSession)
            m_xSession->saveDone();
    }
}

sal_Bool SAL_CALL VCLSession::cancelShutdown() throw( RuntimeException, std::exception )
{
    return m_xSession && m_xSession->cancelShutdown();
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
