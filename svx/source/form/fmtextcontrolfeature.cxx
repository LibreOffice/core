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

#include "fmtextcontrolfeature.hxx"

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;

    //====================================================================
    //= FmTextControlFeature
    //====================================================================
    //--------------------------------------------------------------------
    FmTextControlFeature::FmTextControlFeature( const Reference< XDispatch >& _rxDispatcher, const URL& _rFeatureURL, SfxSlotId _nSlotId, ISlotInvalidator* _pInvalidator )
        :m_xDispatcher    ( _rxDispatcher )
        ,m_aFeatureURL    ( _rFeatureURL  )
        ,m_nSlotId        ( _nSlotId      )
        ,m_pInvalidator   ( _pInvalidator )
        ,m_bFeatureEnabled( false         )
    {
        OSL_ENSURE( _rxDispatcher.is(), "FmTextControlFeature::FmTextControlFeature: invalid dispatcher!"  );
        OSL_ENSURE( m_nSlotId,          "FmTextControlFeature::FmTextControlFeature: invalid slot id!"     );
        OSL_ENSURE( m_pInvalidator,     "FmTextControlFeature::FmTextControlFeature: invalid invalidator!" );

        osl_atomic_increment( &m_refCount );
        try
        {
            m_xDispatcher->addStatusListener( this, m_aFeatureURL );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmTextControlFeature::FmTextControlFeature: caught an exception!" );
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    FmTextControlFeature::~FmTextControlFeature( )
    {
    }

    //--------------------------------------------------------------------
    void FmTextControlFeature::dispatch() const SAL_THROW(())
    {
        dispatch( Sequence< PropertyValue >( ) );
    }

    //--------------------------------------------------------------------
    void FmTextControlFeature::dispatch( const Sequence< PropertyValue >& _rArgs ) const SAL_THROW(())
    {
        try
        {
            if ( m_xDispatcher.is() )
                m_xDispatcher->dispatch( m_aFeatureURL, _rArgs );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmTextControlFeature::dispatch: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmTextControlFeature::statusChanged( const FeatureStateEvent& _rState ) throw (RuntimeException)
    {
        m_aFeatureState   = _rState.State;
        m_bFeatureEnabled = _rState.IsEnabled;

        if ( m_pInvalidator )
            m_pInvalidator->Invalidate( m_nSlotId );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmTextControlFeature::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
    {
        // nothing to do
    }

    //--------------------------------------------------------------------
    void FmTextControlFeature::dispose() SAL_THROW(())
    {
        try
        {
            m_xDispatcher->removeStatusListener( this, m_aFeatureURL );
            m_xDispatcher.clear();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmTextControlFeature::dispose: caught an exception!" );
        }
    }

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
