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

#include "EventThread.hxx"
#include <comphelper/guarding.hxx>
#include <tools/debug.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <memory>

namespace frm
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

OComponentEventThread::OComponentEventThread( ::cppu::OComponentHelper* pCompImpl ) :
    m_xComp( pCompImpl )
{

    osl_atomic_increment(&m_refCount);

    // and add us at the Control
    {
        Reference<XEventListener> xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->addEventListener( xEvtLstnr );
    }

    osl_atomic_decrement(&m_refCount);
}

OComponentEventThread::~OComponentEventThread()
{

    DBG_ASSERT( m_aEvents.empty(),
        "OComponentEventThread::~OComponentEventThread: Didn't call dispose?" );

    impl_clearEventQueue();
}

Any SAL_CALL OComponentEventThread::queryInterface(const Type& _rType)
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
    m_aEvents.clear();
    m_aControls.clear();
    m_aFlags.clear();
}

void OComponentEventThread::disposing( const EventObject& evt )
{
    if( evt.Source == static_cast<XWeak*>(m_xComp.get()) )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // Remove EventListener
        Reference<XEventListener>  xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->removeEventListener( xEvtLstnr );

        // Clear EventQueue
        impl_clearEventQueue();

        // Free the Control and set pCompImpl to 0,
        // so that the thread knows, that it should terminate.
        m_xComp.clear();

        // Wake up the thread and terminate
        m_aCond.set();
        terminate();
    }
}

void OComponentEventThread::addEvent( std::unique_ptr<EventObject> _pEvt )
{
    Reference<XControl>  xTmp;
    addEvent( std::move(_pEvt), xTmp );
}

void OComponentEventThread::addEvent( std::unique_ptr<EventObject> _pEvt,
                                   const Reference<XControl>& rControl,
                                   bool bFlag )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Put data into the queue
    m_aEvents.push_back( std::move( _pEvt ) );

    Reference<XWeak>        xWeakControl(rControl, UNO_QUERY);
    Reference<XAdapter> xControlAdapter = xWeakControl.is() ? xWeakControl->queryAdapter() : Reference<XAdapter>();
    m_aControls.push_back( xControlAdapter );

    m_aFlags.push_back( bFlag );

    // Wake up thread
    m_aCond.set();
}

void SAL_CALL OComponentEventThread::onTerminated()
{
    OComponentEventThread_TBASE::onTerminated();

    release( );
}

void OComponentEventThread::run()
{
    osl_setThreadName("frm::OComponentEventThread");

    acquire( );

    // Hold on to ourselves, so that we're not deleted if a dispose is called at some point in time
    css::uno::Reference<css::uno::XInterface> xThis(static_cast<XWeak*>(this));

    do
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        while( !m_aEvents.empty() )
        {
            // Get the Control and hold on to it so that it cannot be deleted during actionPerformed
            rtl::Reference<::cppu::OComponentHelper> xComp = m_xComp;

            ThreadEvents::iterator firstEvent( m_aEvents.begin() );
            std::unique_ptr<EventObject> pEvt = std::move(*firstEvent);
            m_aEvents.erase( firstEvent );

            ThreadObjects::iterator firstControl( m_aControls.begin() );
            Reference<XAdapter> xControlAdapter = *firstControl;
            m_aControls.erase( firstControl );

            auto firstFlag( m_aFlags.begin() );
            bool bFlag = *firstFlag;
            m_aFlags.erase( firstFlag );

            {
                MutexRelease aReleaseOnce(m_aMutex);
                // Because a queryHardRef can throw an Exception, it should not be called when
                // the mutex is locked.
                Reference<XControl>  xControl;
                if ( xControlAdapter.is() )
                    xControl.set(
                        xControlAdapter->queryAdapted(), css::uno::UNO_QUERY);

                if( xComp.is() )
                    processEvent( xComp.get(), pEvt.get(), xControl, bFlag );
            }
        }

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
    while( true );
}


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
