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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "featuredispatcher.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;

    //====================================================================
    //= ORichTextFeatureDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    ORichTextFeatureDispatcher::ORichTextFeatureDispatcher( EditView& _rView, const URL&  _rURL )
        :m_aFeatureURL( _rURL )
        ,m_aStatusListeners( m_aMutex )
        ,m_pEditView( &_rView )
        ,m_bDisposed( false )
    {
    }

    //--------------------------------------------------------------------
    ORichTextFeatureDispatcher::~ORichTextFeatureDispatcher( )
    {
        if ( !m_bDisposed )
        {
            acquire();
            dispose();
        }
    }

    //--------------------------------------------------------------------
    void ORichTextFeatureDispatcher::dispose()
    {
        EventObject aEvent( *this );
        m_aStatusListeners.disposeAndClear( aEvent );

        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        m_bDisposed = true;
        disposing( aGuard );
    }

    //--------------------------------------------------------------------
    void ORichTextFeatureDispatcher::disposing( ::osl::ClearableMutexGuard& /*_rClearBeforeNotify*/ )
    {
        m_pEditView = NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ORichTextFeatureDispatcher::addStatusListener( const Reference< XStatusListener >& _rxControl, const URL& _rURL ) throw (RuntimeException)
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

    //--------------------------------------------------------------------
    void SAL_CALL ORichTextFeatureDispatcher::removeStatusListener( const Reference< XStatusListener >& _rxControl, const URL& /*_rURL*/ ) throw (RuntimeException)
    {
        m_aStatusListeners.removeInterface( _rxControl );
    }

    //--------------------------------------------------------------------
    void ORichTextFeatureDispatcher::invalidate()
    {
        invalidateFeatureState_Broadcast();
    }

    //--------------------------------------------------------------------
    FeatureStateEvent ORichTextFeatureDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent;
        aEvent.IsEnabled = sal_False;
        aEvent.Source = *const_cast< ORichTextFeatureDispatcher* >( this );
        aEvent.FeatureURL = getFeatureURL();
        aEvent.Requery = sal_False;
        return aEvent;
    }

    //--------------------------------------------------------------------
    void ORichTextFeatureDispatcher::invalidateFeatureState_Broadcast()
    {
        FeatureStateEvent aEvent( buildStatusEvent() );
        ::cppu::OInterfaceIteratorHelper aIter( getStatusListeners() );
        while ( aIter.hasMoreElements() )
            doNotify( static_cast< XStatusListener* >( aIter.next() ), aEvent );
    }

    //--------------------------------------------------------------------
    void ORichTextFeatureDispatcher::newStatusListener( const Reference< XStatusListener >& _rxListener )
    {
        doNotify( _rxListener, buildStatusEvent() );
    }

    //--------------------------------------------------------------------
    void ORichTextFeatureDispatcher::doNotify( const Reference< XStatusListener >& _rxListener, const FeatureStateEvent& _rEvent ) const SAL_THROW(())
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

//........................................................................
}   // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
