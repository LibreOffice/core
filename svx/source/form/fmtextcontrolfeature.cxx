/*************************************************************************
 *
 *  $RCSfile: fmtextcontrolfeature.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 15:47:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    void SAL_CALL FmTextControlFeature::disposing( const EventObject& Source ) throw (RuntimeException)
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

