/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/svapp.hxx>
#include <vcl/svdata.hxx>
#include <vcl/salinst.hxx>
#include <vcl/salsession.hxx>
#include <cppuhelper/compbase1.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XSessionManagerListener2.hpp>

#include <list>

namespace {

namespace css = com::sun::star;

}

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace rtl;

SalSession::~SalSession()
{
}

class VCLSession : public cppu::WeakComponentImplHelper1 < XSessionManagerClient >
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
    SalSession*                                     m_pSession;
    osl::Mutex                                      m_aMutex;
    bool                                            m_bInteractionRequested;
    bool                                            m_bInteractionGranted;
    bool                                            m_bInteractionDone;
    bool                                            m_bSaveDone;

    static void SalSessionEventProc( SalSessionEvent* pEvent );
    static VCLSession* pOneInstance;

    void callSaveRequested( bool bShutdown, bool bCancelable );
    void callShutdownCancelled();
    void callInteractionGranted( bool bGranted );
    void callQuit();
public:
    VCLSession();
    virtual ~VCLSession();

    virtual void SAL_CALL addSessionManagerListener( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual void SAL_CALL removeSessionManagerListener( const css::uno::Reference< XSessionManagerListener>& xListener ) throw( RuntimeException );
    virtual void SAL_CALL queryInteraction( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual void SAL_CALL interactionDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual void SAL_CALL saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException );
    virtual sal_Bool SAL_CALL cancelShutdown() throw( RuntimeException );
};

VCLSession* VCLSession::pOneInstance = NULL;

VCLSession::VCLSession()
        : cppu::WeakComponentImplHelper1< XSessionManagerClient >( m_aMutex ),
          m_bInteractionRequested( false ),
          m_bInteractionGranted( false ),
          m_bInteractionDone( false ),
          m_bSaveDone( false )
{
    DBG_ASSERT( pOneInstance == 0, "One instance  of VCLSession only !" );
    pOneInstance = this;
    m_pSession = ImplGetSVData()->mpDefInst->CreateSalSession();
    if( m_pSession )
        m_pSession->SetCallback( SalSessionEventProc );
}

VCLSession::~VCLSession()
{
    DBG_ASSERT( pOneInstance == this, "Another instance of VCLSession in destructor !" );
    pOneInstance = NULL;
    delete m_pSession;
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
        m_bInteractionRequested = m_bInteractionGranted = m_pSession ? false : true;

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
        osl::MutexGuard aGuard( m_aMutex );
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
        osl::MutexGuard aGuard( m_aMutex );
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

void VCLSession::SalSessionEventProc( SalSessionEvent* pEvent )
{
    switch( pEvent->m_eType )
    {
        case Interaction:
        {
            SalSessionInteractionEvent* pIEv = static_cast<SalSessionInteractionEvent*>(pEvent);
            pOneInstance->callInteractionGranted( pIEv->m_bInteractionGranted );
        }
        break;
        case SaveRequest:
        {
            SalSessionSaveRequestEvent* pSEv = static_cast<SalSessionSaveRequestEvent*>(pEvent);
            pOneInstance->callSaveRequested( pSEv->m_bShutdown, pSEv->m_bCancelable );
        }
        break;
        case ShutdownCancel:
            pOneInstance->callShutdownCancelled();
            break;
        case Quit:
            pOneInstance->callQuit();
            break;
    }
}

void SAL_CALL VCLSession::addSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aListeners.push_back( Listener( xListener ) );
}

void SAL_CALL VCLSession::removeSessionManagerListener( const css::uno::Reference<XSessionManagerListener>& xListener ) throw( RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    std::list< Listener >::iterator it = m_aListeners.begin();
    while( it != m_aListeners.end() )
    {
        if( it->m_xListener == xListener )
        {
            m_aListeners.erase( it );
            it = m_aListeners.begin();
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

    osl::MutexGuard aGuard( m_aMutex );
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
        if( m_pSession )
            m_pSession->interactionDone();
    }
}

void SAL_CALL VCLSession::saveDone( const css::uno::Reference< XSessionManagerListener >& xListener ) throw( RuntimeException )
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
        if( m_pSession )
            m_pSession->saveDone();
    }
}

sal_Bool SAL_CALL VCLSession::cancelShutdown() throw( RuntimeException )
{
    return m_pSession ? (sal_Bool)m_pSession->cancelShutdown() : sal_False;
}

// service implementation

OUString SAL_CALL vcl_session_getImplementationName()
{
    static OUString aImplementationName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.VCLSessionManagerClient" ) );
    return aImplementationName;
}

Sequence< rtl::OUString > SAL_CALL vcl_session_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.frame.SessionManagerClient");
    return aRet;
}

css::uno::Reference< XInterface > SAL_CALL vcl_session_createInstance( const css::uno::Reference< XMultiServiceFactory > & /*xMultiServiceFactory*/ )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->xSMClient.is() )
        pSVData->xSMClient = new VCLSession();

    return css::uno::Reference< XInterface >(pSVData->xSMClient, UNO_QUERY );
}
