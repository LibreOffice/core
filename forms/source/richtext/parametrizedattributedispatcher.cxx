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

#include "parametrizedattributedispatcher.hxx"
#include <editeng/editview.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>

#include <svx/svxids.hrc>
#include <sfx2/sfxuno.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;

    OParametrizedAttributeDispatcher::OParametrizedAttributeDispatcher( EditView& _rView, AttributeId _nAttributeId, const URL& _rURL, IMultiAttributeDispatcher* _pMasterDispatcher )
        :OAttributeDispatcher( _rView, _nAttributeId, _rURL, _pMasterDispatcher )
    {
    }


    OParametrizedAttributeDispatcher::~OParametrizedAttributeDispatcher()
    {
        acquire();
        dispose();
    }


    namespace
    {
        static SfxSlotId lcl_normalizeLatinScriptSlotId( SfxSlotId _nSlotId )
        {
            switch ( _nSlotId )
            {
            case SID_ATTR_CHAR_LATIN_FONT:      return SID_ATTR_CHAR_FONT;
            case SID_ATTR_CHAR_LATIN_LANGUAGE:  return SID_ATTR_CHAR_LANGUAGE;
            case SID_ATTR_CHAR_LATIN_POSTURE:   return SID_ATTR_CHAR_POSTURE;
            case SID_ATTR_CHAR_LATIN_WEIGHT:    return SID_ATTR_CHAR_WEIGHT;
            case SID_ATTR_CHAR_LATIN_FONTHEIGHT:return SID_ATTR_CHAR_FONTHEIGHT;
            }
            return _nSlotId;
        }
    }


    void OParametrizedAttributeDispatcher::fillFeatureEventFromAttributeState( FeatureStateEvent& _rEvent, const AttributeState& _rState ) const
    {
        OSL_ENSURE( getEditView(), "OParametrizedAttributeDispatcher::notifyState: already disposed!" );
        if ( !getEditView() )
            return;

        SfxItemSet aEmptySet( const_cast< EditView* >( getEditView() )->GetEmptyItemSet() );
        Sequence< PropertyValue > aUnoStateDescription;
        if ( _rState.getItem() )
        {
            aEmptySet.Put( *_rState.getItem() );
            SfxSlotId nSlotId = aEmptySet.GetPool()->GetSlotId( _rState.getItem()->Which() );
            TransformItems( nSlotId, aEmptySet, aUnoStateDescription );
            _rEvent.State <<= aUnoStateDescription;
        }
        else
            OAttributeDispatcher::fillFeatureEventFromAttributeState( _rEvent, _rState );
    }


    const SfxPoolItem* OParametrizedAttributeDispatcher::convertDispatchArgsToItem( const Sequence< PropertyValue >& _rArguments )
    {
        // get the real slot id. This may differ from our attribute id: for instance, both
        // SID_ATTR_CHAR_HEIGHT and SID_ATTR_CHAR_LATIN_HEIGHT are mapped to the same which id
        SfxSlotId nSlotId = lcl_normalizeLatinScriptSlotId( (SfxSlotId)m_nAttributeId );

        SfxAllItemSet aParameterSet( getEditView()->GetEmptyItemSet() );
        TransformParameters( nSlotId, _rArguments, aParameterSet );

        const SfxPoolItem* pArgument = nullptr;
        if ( aParameterSet.Count() )
        {
            OSL_ENSURE( aParameterSet.Count() == 1, "OParametrizedAttributeDispatcher::convertDispatchArgsToItem: Arguments which form more than 1 item? How this?" );
            WhichId nAttributeWhich = aParameterSet.GetPool()->GetWhich( nSlotId );
            pArgument = aParameterSet.GetItem( nAttributeWhich );
            OSL_ENSURE( pArgument, "OParametrizedAttributeDispatcher::convertDispatchArgsToItem: suspicious: there were arguments, but they're not for my slot!" );
        }

        return pArgument;
    }


    void SAL_CALL OParametrizedAttributeDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArguments ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( _rURL.Complete == getFeatureURL().Complete, "OParametrizedAttributeDispatcher::dispatch: invalid URL!" );
        (void)_rURL;
        if ( m_pMasterDispatcher )
        {
            const SfxPoolItem* pConvertedArgument = convertDispatchArgsToItem( _rArguments );
            m_pMasterDispatcher->executeAttribute( m_nAttributeId, pConvertedArgument );
        }
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
