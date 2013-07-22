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

#include <sfx2/querystatus.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svtools/itemdel.hxx>
#include <svl/visitem.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

class SfxQueryStatus_Impl : public ::cppu::WeakImplHelper1< css::frame::XStatusListener >
{
    public:

        SfxQueryStatus_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const OUString& aCommand );
        virtual ~SfxQueryStatus_Impl();

        // Query method
        SfxItemState QueryState( SfxPoolItem*& pPoolItem );

        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

    private:
        SfxQueryStatus_Impl( const SfxQueryStatus& );
        SfxQueryStatus_Impl();
        SfxQueryStatus_Impl& operator=( const SfxQueryStatus& );

        sal_Bool                                                                   m_bQueryInProgress;
        SfxItemState                                                               m_eState;
        SfxPoolItem*                                                               m_pItem;
        sal_uInt16                                                                     m_nSlotID;
        osl::Condition                                                             m_aCondition;
        ::com::sun::star::util::URL                                                m_aCommand;
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >         m_xDispatch;
};

SfxQueryStatus_Impl::SfxQueryStatus_Impl( const Reference< XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const OUString& rCommand ) :
    m_bQueryInProgress( sal_False ),
    m_eState( SFX_ITEM_DISABLED ),
    m_pItem( 0 ),
    m_nSlotID( nSlotId )
{
    m_aCommand.Complete = rCommand;
    Reference< XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( m_aCommand );
    if ( rDispatchProvider.is() )
        m_xDispatch = rDispatchProvider->queryDispatch( m_aCommand, OUString(), 0 );
    m_aCondition.reset();
}

SfxQueryStatus_Impl::~SfxQueryStatus_Impl()
{
}

void SAL_CALL SfxQueryStatus_Impl::disposing( const EventObject& )
throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    m_xDispatch.clear();
}

void SAL_CALL SfxQueryStatus_Impl::statusChanged( const FeatureStateEvent& rEvent)
throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    m_pItem  = NULL;
    m_eState = SFX_ITEM_DISABLED;

    if ( rEvent.IsEnabled )
    {
        m_eState = SFX_ITEM_AVAILABLE;
        ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

        if ( pType == ::getBooleanCppuType() )
        {
            sal_Bool bTemp = false;
            rEvent.State >>= bTemp ;
            m_pItem = new SfxBoolItem( m_nSlotID, bTemp );
        }
        else if ( pType == ::getCppuType((const sal_uInt16*)0) )
        {
            sal_uInt16 nTemp = 0;
            rEvent.State >>= nTemp ;
            m_pItem = new SfxUInt16Item( m_nSlotID, nTemp );
        }
        else if ( pType == ::getCppuType((const sal_uInt32*)0) )
        {
            sal_uInt32 nTemp = 0;
            rEvent.State >>= nTemp ;
            m_pItem = new SfxUInt32Item( m_nSlotID, nTemp );
        }
        else if ( pType == ::getCppuType((const OUString*)0) )
        {
            OUString sTemp ;
            rEvent.State >>= sTemp ;
            m_pItem = new SfxStringItem( m_nSlotID, sTemp );
        }
        else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
        {
            ItemStatus aItemStatus;
            rEvent.State >>= aItemStatus;
            m_eState = aItemStatus.State;
            m_pItem = new SfxVoidItem( m_nSlotID );
        }
        else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::Visibility*)0) )
        {
            Visibility aVisibilityStatus;
            rEvent.State >>= aVisibilityStatus;
            m_pItem = new SfxVisibilityItem( m_nSlotID, aVisibilityStatus.bVisible );
        }
        else
        {
            m_eState = SFX_ITEM_UNKNOWN;
            m_pItem  = new SfxVoidItem( m_nSlotID );
        }
    }

    if ( m_pItem )
        DeleteItemOnIdle( m_pItem );

    try
    {
        m_aCondition.set();
        m_xDispatch->removeStatusListener( Reference< XStatusListener >( static_cast< cppu::OWeakObject* >( this ), UNO_QUERY ),
                                           m_aCommand );
    }
    catch ( Exception& )
    {
    }
}

// Query method
SfxItemState SfxQueryStatus_Impl::QueryState( SfxPoolItem*& rpPoolItem )
{
    SolarMutexGuard aGuard;
    if ( !m_bQueryInProgress )
    {
        m_pItem  = NULL;
        m_eState = SFX_ITEM_DISABLED;

        if ( m_xDispatch.is() )
        {
            try
            {
                m_aCondition.reset();
                m_bQueryInProgress = sal_True;
                m_xDispatch->addStatusListener( Reference< XStatusListener >( static_cast< cppu::OWeakObject* >( this ), UNO_QUERY ),
                                                m_aCommand );
            }
            catch ( Exception& )
            {
                m_aCondition.set();
            }
        }
        else
            m_aCondition.set();
    }

    m_aCondition.wait();

    m_bQueryInProgress = sal_False;
    rpPoolItem = m_pItem;
    return m_eState;
}

//*************************************************************************

SfxQueryStatus::SfxQueryStatus( const Reference< XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const OUString& rCommand )
{
    m_pSfxQueryStatusImpl = new SfxQueryStatus_Impl( rDispatchProvider, nSlotId, rCommand );
    m_xStatusListener     = Reference< XStatusListener >(
                                static_cast< cppu::OWeakObject* >( m_pSfxQueryStatusImpl ),
                                UNO_QUERY );
}

SfxQueryStatus::~SfxQueryStatus()
{
}

SfxItemState SfxQueryStatus::QueryState( SfxPoolItem*& rpPoolItem )
{
    SolarMutexGuard aGuard;
    return m_pSfxQueryStatusImpl->QueryState( rpPoolItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
