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
#include <cppuhelper/weak.hxx>

#ifndef _SFXBINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXMSGSERV_HXX
#include "slotserv.hxx"
#endif

#include <sfx2/sfxuno.hxx>

class SfxControllerItem;
class SfxDispatcher;
class BindDispatch_Impl :   public ::com::sun::star::frame::XStatusListener ,
                            public ::com::sun::star::lang::XTypeProvider    ,
                            public ::cppu::OWeakObject
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

    SFX_DECL_XINTERFACE_XTYPEPROVIDER

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
    SfxControllerItem*      pController;   // Ptr auf 1. gebundenen Controller (untereinander verkettet)
    SfxSlotServer           aSlotServ;     // SlotServer, SlotPtr = 0 -> Nicht auf Stack
    SfxPoolItem*            pLastItem;     // zuletzt verschicktes Item, nie -1
    SfxItemState            eLastState;    // zuletzt verschickter State
    sal_Bool                bCtrlDirty:1;  // Controller aktualisiert?
    sal_Bool                bSlotDirty:1;  // Funktion gfs. vorhanden, muss aktualisiert werden
    sal_Bool                bItemVisible:1;// item visibility
    sal_Bool                bItemDirty;    // G"ultigkeit von pLastItem

private:
                            SfxStateCache( const SfxStateCache& rOrig ); // n.i.
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

// checks wheather this function is in the stack of active SfxObjectInterface

//inline sal_Bool SfxStateCache::IsCallable( SfxDispatcher &rDispat )
//{
//  return GetSlotServer(rDispat) != 0;
//}
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
