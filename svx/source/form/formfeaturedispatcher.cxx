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
#include "precompiled_svx.hxx"

#include "formfeaturedispatcher.hxx"

#include <comphelper/namedvaluecollection.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form::runtime;

    //====================================================================
    //= OSingleFeatureDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OSingleFeatureDispatcher::OSingleFeatureDispatcher( const URL& _rFeatureURL, const sal_Int16 _nFormFeature,
            const Reference< XFormOperations >& _rxFormOperations, ::osl::Mutex& _rMutex )
        :m_rMutex( _rMutex )
        ,m_aStatusListeners( _rMutex )
        ,m_xFormOperations( _rxFormOperations )
        ,m_aFeatureURL( _rFeatureURL )
        ,m_nFormFeature( _nFormFeature )
        ,m_bLastKnownEnabled( sal_False )
        ,m_bDisposed( sal_False )
    {
    }

    //--------------------------------------------------------------------
    void OSingleFeatureDispatcher::dispose()
    {
        {
            ::osl::MutexGuard aGuard( m_rMutex );
            if ( m_bDisposed )
                return;
        }

        EventObject aDisposeEvent( *this );
        m_aStatusListeners.disposeAndClear( aDisposeEvent );

        {
            ::osl::MutexGuard aGuard( m_rMutex );
            m_bDisposed = sal_True;
        }
    }

    //--------------------------------------------------------------------
    void OSingleFeatureDispatcher::getUnoState( FeatureStateEvent& /* [out] */ _rState ) const
    {
        _rState.Source = *const_cast< OSingleFeatureDispatcher* >( this );

        FeatureState aState( m_xFormOperations->getState( m_nFormFeature ) );

        _rState.FeatureURL = m_aFeatureURL;
        _rState.IsEnabled = aState.Enabled;
        _rState.Requery = sal_False;
        _rState.State = aState.State;
    }

    //--------------------------------------------------------------------
    void OSingleFeatureDispatcher::updateAllListeners()
    {
        ::osl::ClearableMutexGuard aGuard( m_rMutex );

        FeatureStateEvent aUnoState;
        getUnoState( aUnoState );

        if ( ( m_aLastKnownState == aUnoState.State ) && ( m_bLastKnownEnabled == aUnoState.IsEnabled ) )
            return;

        m_aLastKnownState = aUnoState.State;
        m_bLastKnownEnabled = aUnoState.IsEnabled;

        notifyStatus( NULL, aGuard );
    }

    //--------------------------------------------------------------------
    void OSingleFeatureDispatcher::notifyStatus( const Reference< XStatusListener >& _rxListener, ::osl::ClearableMutexGuard& _rFreeForNotification )
    {
        FeatureStateEvent aUnoState;
        getUnoState( aUnoState );

        if ( _rxListener.is() )
        {
            try
            {
                _rFreeForNotification.clear();
                _rxListener->statusChanged( aUnoState );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OSingleFeatureDispatcher::notifyStatus: caught an exception!" );
            }
        }
        else
        {
            ::cppu::OInterfaceIteratorHelper aIter( m_aStatusListeners );
            _rFreeForNotification.clear();

            while ( aIter.hasMoreElements() )
            {
                try
                {
                    static_cast< XStatusListener* >( aIter.next() )->statusChanged( aUnoState );
                }
                catch( const DisposedException& )
                {
                    OSL_ENSURE( sal_False, "OSingleFeatureDispatcher::notifyStatus: caught a DisposedException - removing the listener!" );
                    aIter.remove( );
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "OSingleFeatureDispatcher::notifyStatus: caught a generic exception while notifying a single listener!" );
                }
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleFeatureDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_rMutex );
        checkAlive();

        OSL_ENSURE( _rURL.Complete == m_aFeatureURL.Complete, "OSingleFeatureDispatcher::dispatch: not responsible for this URL!" );
        (void)_rURL;

        if ( !m_xFormOperations->isEnabled( m_nFormFeature ) )
            return;

        // release our mutex before executing the command
        sal_Int16 nFormFeature( m_nFormFeature );
        Reference< XFormOperations > xFormOperations( m_xFormOperations );
        aGuard.clear();

        try
        {
            if ( !_rArguments.getLength() )
            {
                xFormOperations->execute( nFormFeature );
            }
            else
            {   // at the moment we only support one parameter
                ::comphelper::NamedValueCollection aArgs( _rArguments );
                xFormOperations->executeWithArguments( nFormFeature, aArgs.getNamedValues() );
            }
        }
        catch( const RuntimeException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleFeatureDispatcher::addStatusListener( const Reference< XStatusListener >& _rxControl, const URL& _rURL ) throw (RuntimeException)
    {
        (void)_rURL;
        OSL_ENSURE( _rURL.Complete == m_aFeatureURL.Complete, "OSingleFeatureDispatcher::addStatusListener: unexpected URL!" );
        OSL_ENSURE( _rxControl.is(), "OSingleFeatureDispatcher::addStatusListener: senseless call!" );
        if ( !_rxControl.is() )
            return;

        ::osl::ClearableMutexGuard aGuard( m_rMutex );
        if ( m_bDisposed )
        {
            EventObject aDisposeEvent( *this );
            aGuard.clear();
            _rxControl->disposing( aDisposeEvent );
            return;
        }

        m_aStatusListeners.addInterface( _rxControl );

        // initially update the status
        notifyStatus( _rxControl, aGuard );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleFeatureDispatcher::removeStatusListener( const Reference< XStatusListener >& _rxControl, const URL& _rURL ) throw (RuntimeException)
    {
        (void)_rURL;
        OSL_ENSURE( _rURL.Complete == m_aFeatureURL.Complete, "OSingleFeatureDispatcher::removeStatusListener: unexpected URL!" );
        OSL_ENSURE( _rxControl.is(), "OSingleFeatureDispatcher::removeStatusListener: senseless call!" );
        if ( !_rxControl.is() )
            return;

        ::osl::MutexGuard aGuard( m_rMutex );
        checkAlive();

        m_aStatusListeners.removeInterface( _rxControl );
    }

    //--------------------------------------------------------------------
    void OSingleFeatureDispatcher::checkAlive() const SAL_THROW((DisposedException))
    {
        if ( m_bDisposed )
            throw DisposedException( ::rtl::OUString(), *const_cast< OSingleFeatureDispatcher* >( this ) );
    }

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
