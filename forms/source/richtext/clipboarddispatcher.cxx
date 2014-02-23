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

#include "clipboarddispatcher.hxx"
#include <editeng/editview.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <svtools/cliplistener.hxx>
#include <svtools/transfer.hxx>

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
    namespace
    {
        static URL createClipboardURL( OClipboardDispatcher::ClipboardFunc _eFunc )
        {
            URL aURL;
            switch ( _eFunc )
            {
            case OClipboardDispatcher::eCut:
                aURL.Complete = ".uno:Cut";
                break;
            case OClipboardDispatcher::eCopy:
                aURL.Complete = ".uno:Copy";
                break;
            case OClipboardDispatcher::ePaste:
                aURL.Complete = ".uno:Paste";
                break;
            }
            return aURL;
        }
    }

    //====================================================================
    //= OClipboardDispatcher
    //====================================================================

    OClipboardDispatcher::OClipboardDispatcher( EditView& _rView, ClipboardFunc _eFunc )
        :ORichTextFeatureDispatcher( _rView, createClipboardURL( _eFunc ) )
        ,m_eFunc( _eFunc )
        ,m_bLastKnownEnabled( sal_True )
    {
    }


    sal_Bool OClipboardDispatcher::implIsEnabled( ) const
    {
        sal_Bool bEnabled = sal_False;
        switch ( m_eFunc )
        {
        case eCut:
            bEnabled = !getEditView()->IsReadOnly() && getEditView()->HasSelection();
            break;

        case eCopy:
            bEnabled = getEditView()->HasSelection();
            break;

        case ePaste:
            bEnabled = !getEditView()->IsReadOnly();
            break;
        }
        return bEnabled;
    }


    FeatureStateEvent OClipboardDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );
        aEvent.IsEnabled = implIsEnabled();
        return aEvent;
    }


    void OClipboardDispatcher::invalidateFeatureState_Broadcast()
    {
        sal_Bool bEnabled = implIsEnabled();
        if ( m_bLastKnownEnabled == bEnabled )
            // nothing changed -> no notification
            return;
        m_bLastKnownEnabled = bEnabled;

        ORichTextFeatureDispatcher::invalidateFeatureState_Broadcast();
    }


    void SAL_CALL OClipboardDispatcher::dispatch( const URL& /*_rURL*/, const Sequence< PropertyValue >& /*Arguments*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !getEditView() )
            throw DisposedException();

        switch ( m_eFunc )
        {
        case eCut:
            getEditView()->Cut();
            break;

        case eCopy:
            getEditView()->Copy();
            break;

        case ePaste:
            getEditView()->Paste();
            break;
        }
    }

    //====================================================================
    //= OPasteClipboardDispatcher
    //====================================================================

    OPasteClipboardDispatcher::OPasteClipboardDispatcher( EditView& _rView )
        :OClipboardDispatcher( _rView, ePaste )
        ,m_pClipListener( NULL )
        ,m_bPastePossible( sal_False )
    {
        m_pClipListener = new TransferableClipboardListener( LINK( this, OPasteClipboardDispatcher, OnClipboardChanged ) );
        m_pClipListener->acquire();
        m_pClipListener->AddRemoveListener( _rView.GetWindow(), sal_True );

        // initial state
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( _rView.GetWindow() ) );
        m_bPastePossible = ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) || aDataHelper.HasFormat( SOT_FORMAT_RTF ) );
    }


    OPasteClipboardDispatcher::~OPasteClipboardDispatcher()
    {
        if ( !isDisposed() )
        {
            acquire();
            dispose();
        }
    }


    IMPL_LINK( OPasteClipboardDispatcher, OnClipboardChanged, TransferableDataHelper*, _pDataHelper )
    {
        OSL_ENSURE( _pDataHelper, "OPasteClipboardDispatcher::OnClipboardChanged: ooops!" );
        m_bPastePossible = _pDataHelper->HasFormat( SOT_FORMAT_STRING )
                        || _pDataHelper->HasFormat( SOT_FORMAT_RTF );

        invalidate();

        return 0L;
    }


    void OPasteClipboardDispatcher::disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify )
    {
        OSL_ENSURE( getEditView() && getEditView()->GetWindow(), "OPasteClipboardDispatcher::disposing: EditView should not (yet) be disfunctional here!" );
        if (m_pClipListener)
        {
            if (getEditView() && getEditView()->GetWindow())
                m_pClipListener->AddRemoveListener( getEditView()->GetWindow(), sal_False );

            m_pClipListener->release();
            m_pClipListener = NULL;
        }

        OClipboardDispatcher::disposing( _rClearBeforeNotify );
    }


    sal_Bool OPasteClipboardDispatcher::implIsEnabled( ) const
    {
        return m_bPastePossible && OClipboardDispatcher::implIsEnabled();
    }

//........................................................................
}   // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
