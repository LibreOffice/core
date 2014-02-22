/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "EventThread.hxx"
#include <comphelper/guarding.hxx>
#include <tools/debug.hxx>


namespace frm
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

OComponentEventThread::OComponentEventThread( ::cppu::OComponentHelper* pCompImpl ) :
    m_pCompImpl( pCompImpl )
{

    increment(m_refCount);

    
    {
        InterfaceRef xIFace(static_cast<XWeak*>(pCompImpl));
        query_interface(xIFace, m_xComp);
    }

    
    {
        Reference<XEventListener> xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->addEventListener( xEvtLstnr );
    }

    decrement(m_refCount);
}

OComponentEventThread::~OComponentEventThread()
{

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

        
        Reference<XEventListener>  xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->removeEventListener( xEvtLstnr );

        
        impl_clearEventQueue();

        
        
        m_xComp = 0;
        m_pCompImpl = 0;

        
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

    
    m_aEvents.push_back( cloneEvent( _pEvt ) );

    Reference<XWeak>        xWeakControl(rControl, UNO_QUERY);
    Reference<XAdapter> xControlAdapter = xWeakControl.is() ? xWeakControl->queryAdapter() : Reference<XAdapter>();
    m_aControls.push_back( xControlAdapter );

    m_aFlags.push_back( bFlag );

    
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

    
    InterfaceRef xThis(static_cast<XWeak*>(this));

    do
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        while( m_aEvents.size() > 0 )
        {
            
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
                
                
                Reference<XControl>  xControl;
                if ( xControlAdapter.is() )
                    query_interface(xControlAdapter->queryAdapted(), xControl);

                if( xComp.is() )
                    processEvent( pCompImpl, pEvt, xControl, bFlag );
            }

            delete pEvt;
        };

        
        
        if( !m_xComp.is() )
            return;

        
        m_aCond.reset();
        {
            MutexRelease aReleaseOnce(m_aMutex);
            
            m_aCond.wait();
        }
    }
    while( true );
}


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
