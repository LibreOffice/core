/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtextcontrolfeature.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:10:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX
#include "fmtextcontrolfeature.hxx"
#endif

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

