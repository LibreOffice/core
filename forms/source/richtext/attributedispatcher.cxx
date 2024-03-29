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

#include "attributedispatcher.hxx"

#include <editeng/editview.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;

    OAttributeDispatcher::OAttributeDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL,
            IMultiAttributeDispatcher* _pMasterDispatcher )
        :ORichTextFeatureDispatcher( _rView, _rURL )
        ,m_pMasterDispatcher( _pMasterDispatcher )
        ,m_nAttributeId( _nAttributeId )
    {
        OSL_ENSURE( m_pMasterDispatcher, "OAttributeDispatcher::OAttributeDispatcher: invalid master dispatcher!" );
    }


    OAttributeDispatcher::~OAttributeDispatcher( )
    {
        acquire();
        dispose();
    }


    void OAttributeDispatcher::disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify )
    {
        m_pMasterDispatcher = nullptr;
        ORichTextFeatureDispatcher::disposing( _rClearBeforeNotify );
    }


    void OAttributeDispatcher::fillFeatureEventFromAttributeState( FeatureStateEvent& _rEvent, const AttributeState& _rState ) const
    {
        if ( _rState.eSimpleState == eChecked )
            _rEvent.State <<= true;
        else if ( _rState.eSimpleState == eUnchecked )
            _rEvent.State <<= false;
    }


    FeatureStateEvent OAttributeDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );
        aEvent.IsEnabled = getEditView() && !getEditView()->IsReadOnly();

        AttributeState aState;
        if ( m_pMasterDispatcher )
            aState = m_pMasterDispatcher->getState( m_nAttributeId );

        fillFeatureEventFromAttributeState( aEvent, aState );

        return aEvent;
    }


    void SAL_CALL OAttributeDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArguments )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        checkDisposed();

        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OAttributeDispatcher::dispatch: invalid URL!" );
        SAL_WARN_IF( _rArguments.hasElements(), "forms.richtext",
                "OAttributeDispatcher::dispatch: found arguments, but can't handle arguments at all"
                " (URL: " << _rURL.Complete << ")");

        if ( m_pMasterDispatcher )
            m_pMasterDispatcher->executeAttribute( m_nAttributeId, nullptr );
    }


    void OAttributeDispatcher::onAttributeStateChanged( AttributeId _nAttributeId )
    {
        OSL_ENSURE( _nAttributeId == m_nAttributeId, "OAttributeDispatcher::onAttributeStateChanged: wrong attribute!" );

        FeatureStateEvent aEvent( buildStatusEvent() );
        ::comphelper::OInterfaceIteratorHelper3 aIter( getStatusListeners() );
        while ( aIter.hasMoreElements() )
            doNotify( aIter.next(), aEvent );
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
