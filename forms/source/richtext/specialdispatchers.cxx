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

#include "specialdispatchers.hxx"
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <svx/svxids.hrc>
#include <editeng/scriptspaceitem.hxx>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    OSelectAllDispatcher::OSelectAllDispatcher( EditView& _rView, const URL&  _rURL )
        :ORichTextFeatureDispatcher( _rView, _rURL )
    {
    }


    OSelectAllDispatcher::~OSelectAllDispatcher( )
    {
        if ( !isDisposed() )
        {
            acquire();
            dispose();
        }
    }


    void SAL_CALL OSelectAllDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& /*_rArguments*/ )
        throw (RuntimeException,
               std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OSelectAllDispatcher::dispatch: invalid URL!" );
        (void)_rURL;

        checkDisposed();

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : nullptr;
        OSL_ENSURE( pEngine, "OSelectAllDispatcher::dispatch: no edit engine - but not yet disposed?" );
        if ( !pEngine )
            return;

        sal_Int32 nParagraphs = pEngine->GetParagraphCount();
        if ( nParagraphs )
        {
            sal_Int32 nLastParaNumber = nParagraphs - 1;
            sal_Int32 nParaLen = pEngine->GetTextLen( nLastParaNumber );
            getEditView()->SetSelection( ESelection( 0, 0, nLastParaNumber, nParaLen ) );
        }
    }


    FeatureStateEvent OSelectAllDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );
        aEvent.IsEnabled = sal_True;
        return aEvent;
    }

    OParagraphDirectionDispatcher::OParagraphDirectionDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL,
            IMultiAttributeDispatcher* _pMasterDispatcher )
        :OAttributeDispatcher( _rView, _nAttributeId, _rURL, _pMasterDispatcher )
    {
    }


    FeatureStateEvent OParagraphDirectionDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( OAttributeDispatcher::buildStatusEvent() );

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : nullptr;
        OSL_ENSURE( pEngine, "OParagraphDirectionDispatcher::dispatch: no edit engine - but not yet disposed?" );
        if ( pEngine && pEngine->IsVertical() )
            aEvent.IsEnabled = sal_False;

        return aEvent;
    }

    OTextDirectionDispatcher::OTextDirectionDispatcher( EditView& _rView, const URL& _rURL )
        :ORichTextFeatureDispatcher( _rView, _rURL )
    {
    }


    void SAL_CALL OTextDirectionDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& /*_rArguments*/ ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OTextDirectionDispatcher::dispatch: invalid URL!" );
        (void)_rURL;

        checkDisposed();

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : nullptr;
        OSL_ENSURE( pEngine, "OTextDirectionDispatcher::dispatch: no edit engine - but not yet disposed?" );
        if ( !pEngine )
            return;

        pEngine->SetVertical( !pEngine->IsVertical() );
    }


    FeatureStateEvent OTextDirectionDispatcher::buildStatusEvent() const
    {
        FeatureStateEvent aEvent( ORichTextFeatureDispatcher::buildStatusEvent() );

        EditEngine* pEngine = getEditView() ? getEditView()->GetEditEngine() : nullptr;
        OSL_ENSURE( pEngine, "OTextDirectionDispatcher::dispatch: no edit engine - but not yet disposed?" );

        aEvent.IsEnabled = sal_True;
        aEvent.State <<= pEngine && pEngine->IsVertical();

        return aEvent;
    }

    OAsianFontLayoutDispatcher::OAsianFontLayoutDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL, IMultiAttributeDispatcher* _pMasterDispatcher )
        :OParametrizedAttributeDispatcher( _rView, _nAttributeId, _rURL, _pMasterDispatcher )
    {
    }


    const SfxPoolItem* OAsianFontLayoutDispatcher::convertDispatchArgsToItem( const Sequence< PropertyValue >& _rArguments )
    {
        // look for the "Enable" parameter
        const PropertyValue* pLookup = _rArguments.getConstArray();
        const PropertyValue* pLookupEnd = _rArguments.getConstArray() + _rArguments.getLength();
        while ( pLookup != pLookupEnd )
        {
            if ( pLookup->Name == "Enable" )
                break;
            ++pLookup;
        }
        if ( pLookup != pLookupEnd )
        {
            bool bEnable = true;
            OSL_VERIFY( pLookup->Value >>= bEnable );
            if ( m_nAttributeId == SID_ATTR_PARA_SCRIPTSPACE )
                return new SvxScriptSpaceItem( bEnable, (WhichId)m_nAttributeId );
            return new SfxBoolItem( (WhichId)m_nAttributeId, bEnable );
        }

        OSL_FAIL( "OAsianFontLayoutDispatcher::convertDispatchArgsToItem: did not find the one and only argument!" );
        return nullptr;
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
