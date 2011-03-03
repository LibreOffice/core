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
#include "attributedispatcher.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <editeng/editview.hxx>

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
        ,m_pMasterDispatcher( _pMasterDispatcher )
        ,m_nAttributeId( _nAttributeId )
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

        checkDisposed();

        (void)_rURL;
        (void)_rArguments;

        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OAttributeDispatcher::dispatch: invalid URL!" );
#if OSL_DEBUG_LEVEL > 0
        if ( _rArguments.getLength() )
        {
            ::rtl::OString sMessage( "OAttributeDispatcher::dispatch: found arguments, but can't handle arguments at all" );
            sMessage += "\n (URL: ";
            sMessage += ::rtl::OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
            sMessage += ")";
            OSL_FAIL( sMessage.getStr() );
        }
#endif

        if ( m_pMasterDispatcher )
            m_pMasterDispatcher->executeAttribute( m_nAttributeId, NULL );
    }

    //--------------------------------------------------------------------
    void OAttributeDispatcher::onAttributeStateChanged( AttributeId _nAttributeId, const AttributeState& /*_rState*/ )
    {
        OSL_ENSURE( _nAttributeId == m_nAttributeId, "OAttributeDispatcher::onAttributeStateChanged: wrong attribute!" );
        (void)_nAttributeId;

        FeatureStateEvent aEvent( buildStatusEvent() );
        ::cppu::OInterfaceIteratorHelper aIter( getStatusListeners() );
        while ( aIter.hasMoreElements() )
            doNotify( static_cast< XStatusListener* >( aIter.next() ), aEvent );
    }

//........................................................................
}   // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
