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

#include <sfx2/sfxstatuslistener.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include "itemdel.hxx"
#include <svl/visitem.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/msgpool.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SfxStatusListener::SfxStatusListener( const Reference< XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const OUString& rCommand ) :
    m_nSlotID( nSlotId ),
    m_xDispatchProvider( rDispatchProvider )
{
    m_aCommand.Complete = rCommand;
    Reference< XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( m_aCommand );
    if ( rDispatchProvider.is() )
        m_xDispatch = rDispatchProvider->queryDispatch( m_aCommand, OUString(), 0 );
}

SfxStatusListener::~SfxStatusListener()
{
}

// old sfx controller item C++ API
void SfxStatusListener::StateChanged( sal_uInt16, SfxItemState, const SfxPoolItem* )
{
    // must be implemented by sub class
}

void SfxStatusListener::UnBind()
{
    if ( m_xDispatch.is() )
    {
        Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
        m_xDispatch.clear();
    }
}

void SfxStatusListener::ReBind()
{
    Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    if ( m_xDispatch.is() )
        m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
    if ( m_xDispatchProvider.is() )
    {
        try
        {
            m_xDispatch = m_xDispatchProvider->queryDispatch( m_aCommand, OUString(), 0 );
            if ( m_xDispatch.is() )
                m_xDispatch->addStatusListener( aStatusListener, m_aCommand );
        }
        catch( Exception& )
        {
        }
    }
}

// new UNO API
void SAL_CALL SfxStatusListener::dispose() throw( css::uno::RuntimeException, std::exception )
{
    if ( m_xDispatch.is() && !m_aCommand.Complete.isEmpty() )
    {
        try
        {
            Reference< XStatusListener > aStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
            m_xDispatch->removeStatusListener( aStatusListener, m_aCommand );
        }
        catch ( Exception& )
        {
        }
    }

    m_xDispatch.clear();
    m_xDispatchProvider.clear();
}

void SAL_CALL SfxStatusListener::addEventListener( const Reference< XEventListener >& )
throw ( RuntimeException, std::exception )
{
    // do nothing - this is a wrapper class which does not support listeners
}

void SAL_CALL SfxStatusListener::removeEventListener( const Reference< XEventListener >& )
throw ( RuntimeException, std::exception )
{
    // do nothing - this is a wrapper class which does not support listeners
}

void SAL_CALL SfxStatusListener::disposing( const EventObject& Source )
throw( RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    if ( Source.Source == Reference< XInterface >( m_xDispatch, UNO_QUERY ))
        m_xDispatch.clear();
    else if ( Source.Source == Reference< XInterface >( m_xDispatchProvider, UNO_QUERY ))
        m_xDispatchProvider.clear();
}

void SAL_CALL SfxStatusListener::statusChanged( const FeatureStateEvent& rEvent)
throw( RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    SfxViewFrame* pViewFrame = nullptr;
    if ( m_xDispatch.is() )
    {
        Reference< XUnoTunnel > xTunnel( m_xDispatch, UNO_QUERY );
        SfxOfficeDispatch* pDisp = nullptr;
        if ( xTunnel.is() )
        {
            sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
            pDisp = reinterpret_cast< SfxOfficeDispatch* >(sal::static_int_cast< sal_IntPtr >( nImplementation ));
        }

        if ( pDisp )
            pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
    }

    SfxSlotPool& rPool = SfxSlotPool::GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetSlot( m_nSlotID );

    SfxItemState eState = SfxItemState::DISABLED;
    SfxPoolItem* pItem = nullptr;
    if ( rEvent.IsEnabled )
    {
        eState = SfxItemState::DEFAULT;
        css::uno::Type pType = rEvent.State.getValueType();

        if ( pType == ::cppu::UnoType<void>::get() )
        {
            pItem = new SfxVoidItem( m_nSlotID );
            eState = SfxItemState::UNKNOWN;
        }
        else if ( pType == cppu::UnoType< bool >::get() )
        {
            bool bTemp = false;
            rEvent.State >>= bTemp ;
            pItem = new SfxBoolItem( m_nSlotID, bTemp );
        }
        else if ( pType == cppu::UnoType< ::cppu::UnoUnsignedShortType >::get() )
        {
            sal_uInt16 nTemp = 0;
            rEvent.State >>= nTemp ;
            pItem = new SfxUInt16Item( m_nSlotID, nTemp );
        }
        else if ( pType == cppu::UnoType<sal_uInt32>::get() )
        {
            sal_uInt32 nTemp = 0;
            rEvent.State >>= nTemp ;
            pItem = new SfxUInt32Item( m_nSlotID, nTemp );
        }
        else if ( pType == cppu::UnoType<OUString>::get() )
        {
            OUString sTemp ;
            rEvent.State >>= sTemp ;
            pItem = new SfxStringItem( m_nSlotID, sTemp );
        }
        else if ( pType == cppu::UnoType< css::frame::status::ItemStatus >::get() )
        {
            ItemStatus aItemStatus;
            rEvent.State >>= aItemStatus;
            eState = (SfxItemState) aItemStatus.State;
            pItem = new SfxVoidItem( m_nSlotID );
        }
        else if ( pType == cppu::UnoType< css::frame::status::Visibility >::get() )
        {
            Visibility aVisibilityStatus;
            rEvent.State >>= aVisibilityStatus;
            pItem = new SfxVisibilityItem( m_nSlotID, aVisibilityStatus.bVisible );
        }
        else
        {
            if ( pSlot )
                pItem = pSlot->GetType()->CreateItem();
            if ( pItem )
            {
                pItem->SetWhich( m_nSlotID );
                pItem->PutValue( rEvent.State, 0 );
            }
            else
                pItem = new SfxVoidItem( m_nSlotID );
        }
    }

    StateChanged( m_nSlotID, eState, pItem );
    delete pItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
