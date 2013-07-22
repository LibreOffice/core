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
#ifndef _SFXSTATCACH_HXX
#define _SFXSTATCACH_HXX

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase1.hxx>

#include <sfx2/bindings.hxx>

#include "slotserv.hxx"

class SfxControllerItem;
class SfxDispatcher;
class BindDispatch_Impl :   public ::cppu::WeakImplHelper1< css::frame::XStatusListener >
{
friend class SfxStateCache;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          xDisp;
    ::com::sun::star::util::URL                     aURL;
    ::com::sun::star::frame::FeatureStateEvent      aStatus;
    SfxStateCache*          pCache;
    const SfxSlot*          pSlot;

public:
                            BindDispatch_Impl(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > & rDisp,
                                const ::com::sun::star::util::URL& rURL,
                                SfxStateCache* pStateCache, const SfxSlot* pSlot );

    virtual void SAL_CALL           statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

    void                    Release();
    const ::com::sun::star::frame::FeatureStateEvent& GetStatus() const;
    void                    Dispatch( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aProps, sal_Bool bForceSynchron = sal_False );
};

class SfxStateCache
{
friend class BindDispatch_Impl;
    BindDispatch_Impl*      pDispatch;
    sal_uInt16              nId;           // Slot-Id
    SfxControllerItem*      pInternalController;
    com::sun::star::uno::Reference < com::sun::star::frame::XDispatch > xMyDispatch;
    SfxControllerItem*      pController;    // Pointer to first bound Controller (interlinked with each other)
    SfxSlotServer           aSlotServ;      // SlotServer, SlotPtr = 0 -> not on Stack
    SfxPoolItem*            pLastItem;      // Last sent Item, never -1
    SfxItemState            eLastState;     // Last sent State
    sal_Bool                bCtrlDirty:1;   // Update Controller?
    sal_Bool                bSlotDirty:1;   // Present Funktion, must be updated
    sal_Bool                bItemVisible:1; // item visibility
    sal_Bool                bItemDirty;     // Validity of pLastItem

private:
                            SfxStateCache( const SfxStateCache& rOrig ); // inward
    void                    SetState_Impl( SfxItemState, const SfxPoolItem*, sal_Bool bMaybeDirty=sal_False );

public:
                            SfxStateCache( sal_uInt16 nFuncId );
                            ~SfxStateCache();

    sal_uInt16                  GetId() const;

    const SfxSlotServer*    GetSlotServer( SfxDispatcher &rDispat, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & xProv );
    const SfxSlotServer*    GetSlotServer( SfxDispatcher &rDispat )
                            { return GetSlotServer( rDispat, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > () ); }
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          GetDispatch() const;
    void                    Dispatch( const SfxItemSet* pSet, sal_Bool bForceSynchron = sal_False );
    sal_Bool                    IsControllerDirty() const
                            { return bCtrlDirty ? sal_True : sal_False; }
    SfxPoolItem*            GetItem() const { return pLastItem; }
    void                    ClearCache();

    void                    SetState( SfxItemState, const SfxPoolItem*, sal_Bool bMaybeDirty=sal_False );
    void                    SetCachedState(sal_Bool bAlways = sal_False);
    void                    DeleteFloatingWindows();
    void                    Invalidate( sal_Bool bWithSlot );
    void                    SetVisibleState( sal_Bool bShow=sal_True );

    SfxControllerItem*      ChangeItemLink( SfxControllerItem* pNewBinding );
    SfxControllerItem*      GetItemLink() const;
    void                    SetInternalController( SfxControllerItem* pCtrl )
                            { DBG_ASSERT( !pInternalController, "Only one internal controller allowed!" ); pInternalController = pCtrl; }
    void                    ReleaseInternalController() { pInternalController = 0; }
    SfxControllerItem*      GetInternalController() const { return pInternalController; }
    com::sun::star::uno::Reference < com::sun::star::frame::XDispatch >
                            GetInternalDispatch() const
                            { return xMyDispatch; }
    void                    SetInternalDispatch( const com::sun::star::uno::Reference < com::sun::star::frame::XDispatch >& rDisp )
                            { xMyDispatch = rDisp; }
};

//--------------------------------------------------------------------

// clears Cached-Item

inline void SfxStateCache::ClearCache()
{
    bItemDirty = sal_True;
}

//--------------------------------------------------------------------

// registeres a item representing this function

inline SfxControllerItem* SfxStateCache::ChangeItemLink( SfxControllerItem* pNewBinding )
{
    SfxControllerItem* pOldBinding = pController;
    pController = pNewBinding;
    if ( pNewBinding )
    {
        bCtrlDirty = sal_True;
        bItemDirty = sal_True;
    }
    return pOldBinding;
}
//--------------------------------------------------------------------

// returns the func binding which becomes called on spreading states

inline SfxControllerItem* SfxStateCache::GetItemLink() const
{
    return pController;
}
//--------------------------------------------------------------------

inline sal_uInt16 SfxStateCache::GetId() const
{
    return nId;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
