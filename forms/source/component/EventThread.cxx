/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "EventThread.hxx"
#include <comphelper/guarding.hxx>
#include <tools/debug.hxx>

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

DBG_NAME( OComponentEventThread )
OComponentEventThread::OComponentEventThread( ::cppu::OComponentHelper* pCompImpl ) :
    m_pCompImpl( pCompImpl )
{
    DBG_CTOR( OComponentEventThread, NULL );

    increment(m_refCount);

    // Hold a reference of the Control
    {
        InterfaceRef xIFace(static_cast<XWeak*>(pCompImpl));
        query_interface(xIFace, m_xComp);
    }

    // and add us at the Control
    {
        Reference<XEventListener> xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->addEventListener( xEvtLstnr );
    }

    decrement(m_refCount);
}

OComponentEventThread::~OComponentEventThread()
{
    DBG_DTOR( OComponentEventThread, NULL );

    DBG_ASSERT( m_aEvents.empty(),
        "OComponentEventThread::~OComponentEventThread: Didn't call dispose?" );

    impl_clearEventQueue();
}

Any SAL_CALL OComponentEventThread::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aReturn;

    aReturn = OWeakObject::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast<XEventListener*>(this)
        );

    return aReturn;
}

void OComponentEventThread::impl_clearEventQueue()
{
    while ( m_aEvents.size() )
    {
        delete *m_aEvents.begin();
        m_aEvents.erase( m_aEvents.begin() );
    }
    m_aControls.erase( m_aControls.begin(), m_aControls.end() );
    m_aFlags.erase( m_aFlags.begin(), m_aFlags.end() );
}

void OComponentEventThread::disposing( const EventObject& evt ) throw ( ::com::sun::star::uno::RuntimeException)
{
    if( evt.Source == m_xComp )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // Remove EventListener
        Reference<XEventListener>  xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->removeEventListener( xEvtLstnr );

        // Clear EventQueue
        impl_clearEventQueue();

        // Free the Control and set pCompImpl to 0,
        // so that the thread knows, that it should terminate.
        m_xComp = 0;
        m_pCompImpl = 0;

        // Wake up the thread and terminate
        m_aCond.set();
        terminate();
    }
}

void OComponentEventThread::addEvent( const EventObject* _pEvt, sal_Bool bFlag )
{
    Reference<XControl>  xTmp;
    addEvent( _pEvt, xTmp, bFlag );
}

void OComponentEventThread::addEvent( const EventObject* _pEvt,
                                   const Reference<XControl>& rControl,
                                   sal_Bool bFlag )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Put data into the queue
    m_aEvents.push_back( cloneEvent( _pEvt ) );

    Reference<XWeak>        xWeakControl(rControl, UNO_QUERY);
    Reference<XAdapter> xControlAdapter = xWeakControl.is() ? xWeakControl->queryAdapter() : Reference<XAdapter>();
    m_aControls.push_back( xControlAdapter );

    m_aFlags.push_back( bFlag );

    // Wake up thread
    m_aCond.set();
}

void OComponentEventThread::implStarted( )
{
    acquire( );
}

void OComponentEventThread::implTerminated( )
{
    release( );
}

void SAL_CALL OComponentEventThread::kill()
{
    OComponentEventThread_TBASE::terminate();
    OComponentEventThread_TBASE::join();

    implTerminated( );
}

void SAL_CALL OComponentEventThread::onTerminated()
{
    OComponentEventThread_TBASE::onTerminated();

    implTerminated( );
}

void OComponentEventThread::run()
{
    implStarted( );

    // Hold on to ourselves, so that we're not deleted if a dispose is called at some point in time
    InterfaceRef xThis(static_cast<XWeak*>(this));

    do
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        while( m_aEvents.size() > 0 )
        {
            // Get the Control and hold on to it so that it cannot be deleted during actionPerformed
            Reference<XComponent>  xComp = m_xComp;
            ::cppu::OComponentHelper *pCompImpl = m_pCompImpl;

            ThreadEvents::iterator firstEvent( m_aEvents.begin() );
            EventObject* pEvt = *firstEvent;
            m_aEvents.erase( firstEvent );

            ThreadObjects::iterator firstControl( m_aControls.begin() );
            Reference<XAdapter> xControlAdapter = *firstControl;
            m_aControls.erase( firstControl );

            ThreadBools::iterator firstFlag( m_aFlags.begin() );
            sal_Bool bFlag = *firstFlag;
            m_aFlags.erase( firstFlag );

            {
                MutexRelease aReleaseOnce(m_aMutex);
                // Because a queryHardRef can throw an Exception, it shoudln't be called when
                // the mutex is locked.
                Reference<XControl>  xControl;
                if ( xControlAdapter.is() )
                    query_interface(xControlAdapter->queryAdapted(), xControl);

                if( xComp.is() )
                    processEvent( pCompImpl, pEvt, xControl, bFlag );
            }

            delete pEvt;
        };

        // After a Dispose, we do not know the Control anymore.
        // Thus, we must not wait either.
        if( !m_xComp.is() )
            return;

        // Reset waiting condition
        m_aCond.reset();
        {
            MutexRelease aReleaseOnce(m_aMutex);
            // And wait ... if, in the meantime, an Event came in after all
            m_aCond.wait();
        }
    }
    while( sal_True );
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
