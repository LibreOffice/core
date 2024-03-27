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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>

#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <utility>
#include <vcl/svapp.hxx>

#include <factory.hxx>
#include <svdata.hxx>
#include <salinst.hxx>
#include <salsession.hxx>

#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XSessionManagerListener2.hpp>

#include <vector>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;

SalSession::~SalSession()
{
}

namespace {

class VCLSession:
    public comphelper::WeakComponentImplHelper < XSessionManagerClient, css::lang::XServiceInfo >
{
    struct Listener
    {
        css::uno::Reference< XSessionManagerListener >      m_xListener;
        bool                                        m_bInteractionRequested;
        bool                                        m_bInteractionDone;
        bool                                        m_bSaveDone;

        explicit Listener( css::uno::Reference< XSessionManagerListener > xListener )
                : m_xListener(std::move( xListener )),
                  m_bInteractionRequested( false ),
                  m_bInteractionDone( false ),
                  m_bSaveDone( false )
        {}
    };

    std::vector< Listener >                         m_aListeners;
    std::unique_ptr< SalSession >                   m_xSession;
    bool                                            m_bInteractionRequested;
    bool                                            m_bInteractionGranted;
    bool                                            m_bInteractionDone;
    bool                                            m_bSaveDone;

    static void SalSessionEventProc( void* pData, SalSessionEvent* pEvent );

    virtual ~VCLSession() override {}

    virtual void SAL_CALL addSessionManagerListener( const css::uno::Reference< XSessionManagerListener >& xListener ) override;
    virtual void SAL_CALL removeSessionManagerListener( const css::uno::Reference< XSessionManagerListener>& xListener ) override;
    virtual void SAL_CALL queryInteraction( const css::uno::Reference< XSessionManagerListener >& xListener ) override;
    virtual void SAL_CALL interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener ) override;
    virtual void SAL_CALL saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) override;
    virtual sal_Bool SAL_CALL cancelShutdown() override;

    OUString SAL_CALL getImplementationName() override {
        return "com.sun.star.frame.VCLSessionManagerClient";
    }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override {
        return {"com.sun.star.frame.SessionManagerClient"};
    }

    void disposing(std::unique_lock<std::mutex>& rGuard) override;

    void callSaveRequested( bool bShutdown );
    void callShutdownCancelled();
    void callInteractionGranted( bool bGranted );
    void callQuit();

public:
    VCLSession();
};

}

VCLSession::VCLSession()
        : m_xSession( ImplGetSVData()->mpDefInst->CreateSalSession() ),
          m_bInteractionRequested( false ),
          m_bInteractionGranted( false ),
          m_bInteractionDone( false ),
          m_bSaveDone( false )
{
    SAL_INFO("vcl.se", "VCLSession::VCLSession" );

    if (m_xSession)
        m_xSession->SetCallback( SalSessionEventProc, this );
}

void VCLSession::callSaveRequested( bool bShutdown )
{
    SAL_INFO("vcl.se", "VCLSession::callSaveRequested" );

    std::vector< Listener > aListeners;
    {
        std::unique_lock aGuard( m_aMutex );
        // reset listener states
        for (auto & listener : m_aListeners) {
            listener.m_bSaveDone = listener.m_bInteractionRequested = listener.m_bInteractionDone = false;
        }

        // copy listener vector since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bSaveDone = false;
        m_bInteractionDone = false;
        // without session we assume UI is always possible,
        // so it was requested and granted
        m_bInteractionRequested = m_bInteractionGranted = !m_xSession;

        // answer the session manager even if no listeners available anymore
        SAL_WARN_IF(  aListeners.empty(), "vcl.se", "saveRequested but no listeners !" );

        SAL_INFO("vcl.se.debug", "  aListeners.empty() = " << (aListeners.empty() ? "true" : "false") <<
                                 ", bShutdown = " << (bShutdown ? "true" : "false"));
        if( aListeners.empty() )
        {
            if (m_xSession)
                m_xSession->saveDone();
            return;
        }
    }

    SolarMutexReleaser aReleaser;
    for (auto const & listener: aListeners)
        listener.m_xListener->doSave( bShutdown, false/*bCancelable*/ );
}

void VCLSession::callInteractionGranted( bool bInteractionGranted )
{
    SAL_INFO("vcl.se", "VCLSession::callInteractionGranted" );

    std::vector< Listener > aListeners;
    {
        std::unique_lock aGuard( m_aMutex );
        // copy listener vector since calling a listener may remove it.
        for (auto const & listener: m_aListeners)
            if( listener.m_bInteractionRequested )
                aListeners.push_back( listener );

        m_bInteractionGranted = bInteractionGranted;

        // answer the session manager even if no listeners available anymore
        SAL_WARN_IF(  aListeners.empty(), "vcl.se", "interactionGranted but no listeners !" );

        SAL_INFO("vcl.se.debug", "  aListeners.empty() = " << (aListeners.empty() ? "true" : "false") <<
                                 ", bInteractionGranted = " << (bInteractionGranted ? "true" : "false"));
        if( aListeners.empty() )
        {
            if (m_xSession)
                m_xSession->interactionDone();
            return;
        }
    }

    SolarMutexReleaser aReleaser;
    for (auto const & listener: aListeners)
        listener.m_xListener->approveInteraction( bInteractionGranted );
}

void VCLSession::callShutdownCancelled()
{
    SAL_INFO("vcl.se", "VCLSession::callShutdownCancelled");

    std::vector< Listener > aListeners;
    {
        std::unique_lock aGuard( m_aMutex );
        // copy listener vector since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bInteractionRequested = m_bInteractionDone = m_bInteractionGranted = false;
    }

    SolarMutexReleaser aReleaser;
    for (auto const & listener: aListeners)
        listener.m_xListener->shutdownCanceled();
}

void VCLSession::callQuit()
{
    SAL_INFO("vcl.se", "VCLSession::callQuit");

    std::vector< Listener > aListeners;
    {
        std::unique_lock aGuard( m_aMutex );
        // copy listener vector since calling a listener may remove it.
        aListeners = m_aListeners;
        // set back interaction state
        m_bInteractionRequested = m_bInteractionDone = m_bInteractionGranted = false;
    }

    SolarMutexReleaser aReleaser;
    for (auto const & listener: aListeners)
    {
        css::uno::Reference< XSessionManagerListener2 > xListener2( listener.m_xListener, UNO_QUERY );
        if( xListener2.is() )
            xListener2->doQuit();
    }
}

void VCLSession::SalSessionEventProc( void* pData, SalSessionEvent* pEvent )
{
    SAL_INFO("vcl.se", "VCLSession::SalSessionEventProc");

    VCLSession * pThis = static_cast< VCLSession * >( pData );
    switch( pEvent->m_eType )
    {
        case Interaction:
        {
            SAL_INFO("vcl.se.debug", "  EventProcType = Interaction");
            SalSessionInteractionEvent* pIEv = static_cast<SalSessionInteractionEvent*>(pEvent);
            pThis->callInteractionGranted( pIEv->m_bInteractionGranted );
        }
        break;
        case SaveRequest:
        {
            SAL_INFO("vcl.se.debug", "  EventProcType = SaveRequest");
            SalSessionSaveRequestEvent* pSEv = static_cast<SalSessionSaveRequestEvent*>(pEvent);
            pThis->callSaveRequested( pSEv->m_bShutdown );
        }
        break;
        case ShutdownCancel:
            SAL_INFO("vcl.se.debug", "  EventProcType = ShutdownCancel");
            pThis->callShutdownCancelled();
            break;
        case Quit:
            SAL_INFO("vcl.se.debug", "  EventProcType = Quit");
            pThis->callQuit();
            break;
    }
}

void SAL_CALL VCLSession::addSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener )
{
    SAL_INFO("vcl.se", "VCLSession::addSessionManagerListener" );

    std::unique_lock aGuard( m_aMutex );

    SAL_INFO("vcl.se.debug", "  m_aListeners.size() = " << m_aListeners.size() );
    m_aListeners.emplace_back( xListener );
}

void SAL_CALL VCLSession::removeSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener )
{
    SAL_INFO("vcl.se", "VCLSession::removeSessionManagerListener" );

    std::unique_lock aGuard( m_aMutex );

    SAL_INFO("vcl.se.debug", "  m_aListeners.size() = " << m_aListeners.size() );

    std::erase_if(m_aListeners, [&](Listener& listener) {return xListener == listener.m_xListener;});
}

void SAL_CALL VCLSession::queryInteraction( const css::uno::Reference<XSessionManagerListener>& xListener )
{
    SAL_INFO("vcl.se", "VCLSession::queryInteraction");

    SAL_INFO("vcl.se.debug", "  m_bInteractionGranted = " << (m_bInteractionGranted ? "true" : "false") <<
                             ", m_bInteractionRequested = "<< (m_bInteractionRequested ? "true" : "false"));
    if( m_bInteractionGranted )
    {
        if( m_bInteractionDone )
            xListener->approveInteraction( false );
        else
            xListener->approveInteraction( true );
        return;
    }

    std::unique_lock aGuard( m_aMutex );
    if( ! m_bInteractionRequested )
    {
        if (m_xSession)
            m_xSession->queryInteraction();

        m_bInteractionRequested = true;
    }
    for (auto & listener: m_aListeners)
    {
        if( listener.m_xListener == xListener )
        {
            SAL_INFO("vcl.se.debug", "  listener.m_xListener == xListener");
            listener.m_bInteractionRequested = true;
            listener.m_bInteractionDone      = false;
        }
    }
}

void SAL_CALL VCLSession::interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener )
{
    SAL_INFO("vcl.se", "VCLSession::interactionDone");

    std::unique_lock aGuard( m_aMutex );
    int nRequested = 0, nDone = 0;
    for (auto & listener: m_aListeners)
    {
        if( listener.m_bInteractionRequested )
        {
            nRequested++;
            if( xListener == listener.m_xListener )
                listener.m_bInteractionDone = true;
        }
        if( listener.m_bInteractionDone )
            nDone++;
    }

    SAL_INFO("vcl.se.debug", "  nDone = " << nDone <<
                             ", nRequested =" << nRequested);
    if( nDone == nRequested && nDone > 0 )
    {
        m_bInteractionDone = true;
        if (m_xSession)
            m_xSession->interactionDone();
    }
}

void SAL_CALL VCLSession::saveDone( const css::uno::Reference< XSessionManagerListener >& xListener )
{
    SAL_INFO("vcl.se", "VCLSession::saveDone");

    std::unique_lock aGuard( m_aMutex );

    bool bSaveDone = true;
    for (auto & listener: m_aListeners)
    {
        if( listener.m_xListener == xListener )
            listener.m_bSaveDone = true;
        if( ! listener.m_bSaveDone )
            bSaveDone = false;
    }

    SAL_INFO("vcl.se.debug", "  bSaveDone = " << (bSaveDone ? "true" : "false"));

    if( bSaveDone && !m_bSaveDone )
    {
        m_bSaveDone = true;
        if (m_xSession)
            m_xSession->saveDone();
    }
}

sal_Bool SAL_CALL VCLSession::cancelShutdown()
{
    SAL_INFO("vcl.se", "VCLSession::cancelShutdown");

    return m_xSession && m_xSession->cancelShutdown();
}

void VCLSession::disposing(std::unique_lock<std::mutex>& rGuard) {
    SAL_INFO("vcl.se", "VCLSession::disposing");

    std::vector<Listener> vector;
    {
        vector.swap(m_aListeners);
    }
    css::lang::EventObject src(getXWeak());
    for (auto const & listener: vector) {
        rGuard.unlock();
        try {
            listener.m_xListener->disposing(src);
            SAL_INFO("vcl.se.debug", "  call Listener disposing");
        } catch (css::uno::RuntimeException &) {
            TOOLS_WARN_EXCEPTION("vcl.se", "ignoring");
        }
        rGuard.lock();
    }
}

// service implementation

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_frame_VCLSessionManagerClient_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new VCLSession);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
