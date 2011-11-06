/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "fmtextcontrolfeature.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

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

        osl_incrementInterlockedCount( &m_refCount );
        try
        {
            m_xDispatcher->addStatusListener( this, m_aFeatureURL );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FmTextControlFeature::FmTextControlFeature: caught an exception!" );
        }
        osl_decrementInterlockedCount( &m_refCount );
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
            OSL_ENSURE( sal_False, "FmTextControlFeature::dispatch: caught an exception!" );
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
            OSL_ENSURE( sal_False, "FmTextControlFeature::dispose: caught an exception!" );
        }
    }

//........................................................................
}   // namespace svx
//........................................................................

