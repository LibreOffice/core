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
#ifndef INCLUDED_SFX2_SOURCE_INC_STATCACH_HXX
#define INCLUDED_SFX2_SOURCE_INC_STATCACH_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/debug.hxx>

#include <sfx2/bindings.hxx>

#include "slotserv.hxx"

class SfxControllerItem;
class SfxDispatcher;
class BindDispatch_Impl :   public ::cppu::WeakImplHelper< css::frame::XStatusListener >
{
friend class SfxStateCache;
    css::uno::Reference< css::frame::XDispatch >   xDisp;
    css::util::URL const               aURL;
    css::frame::FeatureStateEvent      aStatus;
    SfxStateCache*          pCache;
    const SfxSlot*          pSlot;

public:
                            BindDispatch_Impl(
                                const css::uno::Reference< css::frame::XDispatch > & rDisp,
                                const css::util::URL& rURL,
                                SfxStateCache* pStateCache, const SfxSlot* pSlot );

    virtual void SAL_CALL   statusChanged( const css::frame::FeatureStateEvent& Event ) override;
    virtual void SAL_CALL   disposing( const css::lang::EventObject& Source ) override;

    const css::frame::FeatureStateEvent& GetStatus() const { return aStatus;}
    sal_Int16               Dispatch( const css::uno::Sequence < css::beans::PropertyValue >& aProps, bool bForceSynchron );
    void                    Release();
};

class SfxStateCache
{
friend class BindDispatch_Impl;
    rtl::Reference<BindDispatch_Impl>
                            mxDispatch;
    sal_uInt16 const        nId;           // Slot-Id
    SfxControllerItem*      pInternalController;
    css::uno::Reference < css::frame::XDispatch > xMyDispatch;
    SfxControllerItem*      pController;    // Pointer to first bound Controller (interlinked with each other)
    SfxSlotServer           aSlotServ;      // SlotServer, SlotPtr = 0 -> not on Stack
    SfxPoolItem*            pLastItem;      // Last sent Item, never -1
    SfxItemState            eLastState;     // Last sent State
    bool                    bCtrlDirty:1;   // Update Controller?
    bool                    bSlotDirty:1;   // Present Function, must be updated
    bool                    bItemVisible:1; // item visibility
    bool                    bItemDirty;     // Validity of pLastItem

private:
                            SfxStateCache( const SfxStateCache& rOrig ) = delete;
    void                    SetState_Impl( SfxItemState, const SfxPoolItem*, bool bMaybeDirty );

public:
                            SfxStateCache( sal_uInt16 nFuncId );
                            ~SfxStateCache();

    sal_uInt16                  GetId() const;

    const SfxSlotServer*    GetSlotServer( SfxDispatcher &rDispat, const css::uno::Reference< css::frame::XDispatchProvider > & xProv );
    const SfxSlotServer*    GetSlotServer( SfxDispatcher &rDispat )
                            { return GetSlotServer( rDispat, css::uno::Reference< css::frame::XDispatchProvider > () ); }
    css::uno::Reference< css::frame::XDispatch >          GetDispatch() const;
    sal_Int16               Dispatch( const SfxItemSet* pSet, bool bForceSynchron );
    bool                    IsControllerDirty() const
                            { return bCtrlDirty; }
    void                    ClearCache();

    void                    SetState( SfxItemState, const SfxPoolItem*, bool bMaybeDirty=false );
    void                    SetCachedState(bool bAlways);
    void                    Invalidate( bool bWithSlot );
    void                    SetVisibleState( bool bShow );

    SfxControllerItem*      ChangeItemLink( SfxControllerItem* pNewBinding );
    SfxControllerItem*      GetItemLink() const;
    void                    SetInternalController( SfxControllerItem* pCtrl )
                            { DBG_ASSERT( !pInternalController, "Only one internal controller allowed!" ); pInternalController = pCtrl; }
    void                    ReleaseInternalController() { pInternalController = nullptr; }
    SfxControllerItem*      GetInternalController() const { return pInternalController; }
    const css::uno::Reference < css::frame::XDispatch >&
                            GetInternalDispatch() const
                            { return xMyDispatch; }
    void                    SetInternalDispatch( const css::uno::Reference < css::frame::XDispatch >& rDisp )
                            { xMyDispatch = rDisp; }
};


// clears Cached-Item

inline void SfxStateCache::ClearCache()
{
    bItemDirty = true;
}


// registers a item representing this function

inline SfxControllerItem* SfxStateCache::ChangeItemLink( SfxControllerItem* pNewBinding )
{
    SfxControllerItem* pOldBinding = pController;
    pController = pNewBinding;
    if ( pNewBinding )
    {
        bCtrlDirty = true;
        bItemDirty = true;
    }
    return pOldBinding;
}


// returns the func binding which becomes called on spreading states

inline SfxControllerItem* SfxStateCache::GetItemLink() const
{
    return pController;
}


inline sal_uInt16 SfxStateCache::GetId() const
{
    return nId;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
