/*************************************************************************
 *
 *  $RCSfile: attributedispatcher.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:10:38 $
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

#ifndef FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX
#include "attributedispatcher.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= OAttributeDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    OAttributeDispatcher::OAttributeDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL,
            IMultiAttributeDispatcher* _pMasterDispatcher )
        :ORichTextFeatureDispatcher( _rView, _rURL )
        ,m_nAttributeId( _nAttributeId )
        ,m_pMasterDispatcher( _pMasterDispatcher )
    {
        OSL_ENSURE( m_pMasterDispatcher, "OAttributeDispatcher::OAttributeDispatcher: invalid master dispatcher!" );
    }

    //--------------------------------------------------------------------
    OAttributeDispatcher::~OAttributeDispatcher( )
    {
        acquire();
        dispose();
    }

    //--------------------------------------------------------------------
    void OAttributeDispatcher::disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify )
    {
        m_pMasterDispatcher = NULL;
        ORichTextFeatureDispatcher::disposing( _rClearBeforeNotify );
    }

    //--------------------------------------------------------------------
    void OAttributeDispatcher::fillFeatureEventFromAttributeState( FeatureStateEvent& _rEvent, const AttributeState& _rState ) const
    {
        if ( _rState.eSimpleState == eChecked )
            _rEvent.State <<= (sal_Bool)sal_True;
        else if ( _rState.eSimpleState == eUnchecked )
            _rEvent.State <<= (sal_Bool)sal_False;
    }

    //--------------------------------------------------------------------
    FeatureStateEvent OAttributeDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );
        aEvent.IsEnabled = getEditView() ? !getEditView()->IsReadOnly() : sal_False;

        AttributeState aState;
        if ( m_pMasterDispatcher )
            aState = m_pMasterDispatcher->getState( m_nAttributeId );

        fillFeatureEventFromAttributeState( aEvent, aState );

        return aEvent;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAttributeDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OAttributeDispatcher::dispatch: invalid URL!" );

        checkDisposed();

#if OSL_DEBUG_LEVEL > 0
        if ( _rArguments.getLength() )
        {
            ::rtl::OString sMessage( "OAttributeDispatcher::dispatch: found arguments, but can't handle arguments at all" );
            sMessage += "\n (URL: ";
            sMessage += ::rtl::OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
            sMessage += ")";
            DBG_ERROR( sMessage.getStr() );
        }
#endif

        if ( m_pMasterDispatcher )
            m_pMasterDispatcher->executeAttribute( m_nAttributeId, NULL );
    }

    //--------------------------------------------------------------------
    void OAttributeDispatcher::onAttributeStateChanged( AttributeId _nAttributeId, const AttributeState& _rState )
    {
        OSL_ENSURE( _nAttributeId == m_nAttributeId, "OAttributeDispatcher::onAttributeStateChanged: wrong attribute!" );

        FeatureStateEvent aEvent( buildStatusEvent() );
        ::cppu::OInterfaceIteratorHelper aIter( getStatusListeners() );
        while ( aIter.hasMoreElements() )
            doNotify( static_cast< XStatusListener* >( aIter.next() ), aEvent );
    }

//........................................................................
}   // namespace frm
//........................................................................
