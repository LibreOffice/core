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

#include "featuredispatcher.hxx"


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;

    ORichTextFeatureDispatcher::ORichTextFeatureDispatcher( EditView& _rView, const URL&  _rURL )
        :m_aFeatureURL( _rURL )
        ,m_aStatusListeners( m_aMutex )
        ,m_pEditView( &_rView )
        ,m_bDisposed( false )
    {
    }


    ORichTextFeatureDispatcher::~ORichTextFeatureDispatcher( )
    {
        if ( !m_bDisposed )
        {
            acquire();
            dispose();
        }
    }


    void ORichTextFeatureDispatcher::dispose()
    {
        EventObject aEvent( *this );
        m_aStatusListeners.disposeAndClear( aEvent );

        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        m_bDisposed = true;
        disposing( aGuard );
    }


    void ORichTextFeatureDispatcher::disposing( ::osl::ClearableMutexGuard& /*_rClearBeforeNotify*/ )
    {
        m_pEditView = nullptr;
    }


    void SAL_CALL ORichTextFeatureDispatcher::addStatusListener( const Reference< XStatusListener >& _rxControl, const URL& _rURL ) throw (RuntimeException, std::exception)
    {
        OSL_ENSURE( !m_bDisposed, "ORichTextFeatureDispatcher::addStatusListener: already disposed!" );
        if ( m_bDisposed )
            throw DisposedException();

        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "ORichTextFeatureDispatcher::addStatusListener: invalid URL!" );
        if ( _rURL.Complete == getFeatureURL().Complete )
            if ( _rxControl.is() )
            {
                m_aStatusListeners.addInterface( _rxControl );
                newStatusListener( _rxControl );
            }
    }


    void SAL_CALL ORichTextFeatureDispatcher::removeStatusListener( const Reference< XStatusListener >& _rxControl, const URL& /*_rURL*/ ) throw (RuntimeException, std::exception)
    {
        m_aStatusListeners.removeInterface( _rxControl );
    }


    void ORichTextFeatureDispatcher::invalidate()
    {
        invalidateFeatureState_Broadcast();
    }


    FeatureStateEvent ORichTextFeatureDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent;
        aEvent.IsEnabled = sal_False;
        aEvent.Source = *const_cast< ORichTextFeatureDispatcher* >( this );
        aEvent.FeatureURL = getFeatureURL();
        aEvent.Requery = sal_False;
        return aEvent;
    }


    void ORichTextFeatureDispatcher::invalidateFeatureState_Broadcast()
    {
        FeatureStateEvent aEvent( buildStatusEvent() );
        ::cppu::OInterfaceIteratorHelper aIter( getStatusListeners() );
        while ( aIter.hasMoreElements() )
            doNotify( static_cast< XStatusListener* >( aIter.next() ), aEvent );
    }


    void ORichTextFeatureDispatcher::newStatusListener( const Reference< XStatusListener >& _rxListener )
    {
        doNotify( _rxListener, buildStatusEvent() );
    }


    void ORichTextFeatureDispatcher::doNotify( const Reference< XStatusListener >& _rxListener, const FeatureStateEvent& _rEvent )
    {
        OSL_PRECOND( _rxListener.is(), "ORichTextFeatureDispatcher::doNotify: invalid listener!" );
        if ( _rxListener.is() )
        {
            try
            {
                _rxListener->statusChanged( _rEvent );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "ORichTextFeatureDispatcher::doNotify: caught an exception!" );
            }
        }
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
