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


#include <formfeaturedispatcher.hxx>

#include <comphelper/namedvaluecollection.hxx>
#include <tools/diagnose_ex.h>


namespace svx
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form::runtime;

    OSingleFeatureDispatcher::OSingleFeatureDispatcher( const URL& _rFeatureURL, const sal_Int16 _nFormFeature,
            const Reference< XFormOperations >& _rxFormOperations, ::osl::Mutex& _rMutex )
        :m_rMutex( _rMutex )
        ,m_aStatusListeners( _rMutex )
        ,m_xFormOperations( _rxFormOperations )
        ,m_aFeatureURL( _rFeatureURL )
        ,m_nFormFeature( _nFormFeature )
        ,m_bLastKnownEnabled( false )
    {
    }


    void OSingleFeatureDispatcher::getUnoState( FeatureStateEvent& /* [out] */ _rState ) const
    {
        _rState.Source = *const_cast< OSingleFeatureDispatcher* >( this );

        FeatureState aState( m_xFormOperations->getState( m_nFormFeature ) );

        _rState.FeatureURL = m_aFeatureURL;
        _rState.IsEnabled = aState.Enabled;
        _rState.Requery = false;
        _rState.State = aState.State;
    }


    void OSingleFeatureDispatcher::updateAllListeners()
    {
        ::osl::ClearableMutexGuard aGuard( m_rMutex );

        FeatureStateEvent aUnoState;
        getUnoState( aUnoState );

        if ( ( m_aLastKnownState == aUnoState.State ) && ( m_bLastKnownEnabled == bool(aUnoState.IsEnabled) ) )
            return;

        m_aLastKnownState = aUnoState.State;
        m_bLastKnownEnabled = aUnoState.IsEnabled;

        notifyStatus( nullptr, aGuard );
    }


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
                TOOLS_WARN_EXCEPTION( "svx", "OSingleFeatureDispatcher::notifyStatus" );
            }
        }
        else
        {
            ::comphelper::OInterfaceIteratorHelper2 aIter( m_aStatusListeners );
            _rFreeForNotification.clear();

            while ( aIter.hasMoreElements() )
            {
                try
                {
                    static_cast< XStatusListener* >( aIter.next() )->statusChanged( aUnoState );
                }
                catch( const DisposedException& )
                {
                    TOOLS_WARN_EXCEPTION("svx.form",
                                         "caught a DisposedException - removing the listener!");
                    aIter.remove( );
                }
                catch( const Exception& )
                {
                    TOOLS_WARN_EXCEPTION(
                        "svx.form",
                        "caught a generic exception while notifying a single listener!");
                }
            }
        }
    }


    void SAL_CALL OSingleFeatureDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArguments )
    {
        ::osl::ClearableMutexGuard aGuard( m_rMutex );

        OSL_ENSURE( _rURL.Complete == m_aFeatureURL.Complete, "OSingleFeatureDispatcher::dispatch: not responsible for this URL!" );

        if ( !m_xFormOperations->isEnabled( m_nFormFeature ) )
            return;

        // release our mutex before executing the command
        sal_Int16 nFormFeature( m_nFormFeature );
        Reference< XFormOperations > xFormOperations( m_xFormOperations );
        aGuard.clear();

        try
        {
            if ( !_rArguments.hasElements() )
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
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }


    void SAL_CALL OSingleFeatureDispatcher::addStatusListener( const Reference< XStatusListener >& _rxControl, const URL& _rURL )
    {
        OSL_ENSURE( _rURL.Complete == m_aFeatureURL.Complete, "OSingleFeatureDispatcher::addStatusListener: unexpected URL!" );
        OSL_ENSURE( _rxControl.is(), "OSingleFeatureDispatcher::addStatusListener: senseless call!" );
        if ( !_rxControl.is() )
            return;

        ::osl::ClearableMutexGuard aGuard( m_rMutex );

        m_aStatusListeners.addInterface( _rxControl );

        // initially update the status
        notifyStatus( _rxControl, aGuard );
    }


    void SAL_CALL OSingleFeatureDispatcher::removeStatusListener( const Reference< XStatusListener >& _rxControl, const URL& _rURL )
    {
        OSL_ENSURE( _rURL.Complete == m_aFeatureURL.Complete, "OSingleFeatureDispatcher::removeStatusListener: unexpected URL!" );
        OSL_ENSURE( _rxControl.is(), "OSingleFeatureDispatcher::removeStatusListener: senseless call!" );
        if ( !_rxControl.is() )
            return;

        ::osl::MutexGuard aGuard( m_rMutex );

        m_aStatusListeners.removeInterface( _rxControl );
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
