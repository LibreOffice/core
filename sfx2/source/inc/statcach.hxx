/*************************************************************************
 *
 *  $RCSfile: statcach.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SFXSTATCACH_HXX
#define _SFXSTATCACH_HXX

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _SFXBINDINGS_HXX
#include "bindings.hxx"
#endif

#ifndef _SFXMSGSERV_HXX
#include "slotserv.hxx"
#endif

#include "sfxuno.hxx"

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

public:
                            BindDispatch_Impl(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > & rDisp,
                                const ::com::sun::star::util::URL& rURL,
                                SfxStateCache* pStateCache );

    SFX_DECL_XINTERFACE_XTYPEPROVIDER

    virtual void SAL_CALL           statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

    void                    Release();
    const ::com::sun::star::frame::FeatureStateEvent& GetStatus() const;
    void                    Dispatch( sal_Bool bForceSynchron = sal_False );
};

class SfxStateCache
{
friend class BindDispatch_Impl;
    BindDispatch_Impl*      pDispatch;
    sal_uInt16                  nId;           // Slot-Id
    SfxControllerItem*      pController;   // Ptr auf 1. gebundenen Controller (untereinander verkettet)
    SfxSlotServer           aSlotServ;     // SlotServer, SlotPtr = 0 -> Nicht auf Stack
    SfxPoolItem*            pLastItem;     // zuletzt verschicktes Item, nie -1
    SfxItemState            eLastState;    // zuletzt verschickter State
    sal_Bool                    bCtrlDirty:1;  // Controller aktualisiert?
    sal_Bool                    bSlotDirty:1;  // Funktion gfs. vorhanden, muss aktualisiert werden
    sal_Bool                    bItemDirty;    // G"ultigkeit von pLastItem

private:
                            SfxStateCache( const SfxStateCache& rOrig ); // n.i.
    void                    SetState_Impl( SfxItemState, const SfxPoolItem * );
    sal_Bool                    UpdateDispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & xProv );

public:
                            SfxStateCache( sal_uInt16 nFuncId );
                            ~SfxStateCache();

    sal_uInt16                  GetId() const;

    const SfxSlotServer*    GetSlotServer( SfxDispatcher &rDispat, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & xProv );
    const SfxSlotServer*    GetSlotServer( SfxDispatcher &rDispat )
                            { return GetSlotServer( rDispat, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > () ); }
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >          GetDispatch() const;
    void                    Dispatch( sal_Bool bForceSynchron = sal_False );
    sal_Bool                    IsControllerDirty() const
                            { return bCtrlDirty ? sal_True : sal_False; }
    SfxPoolItem*            GetItem() const { return pLastItem; }
    void                    ClearCache();

    void                    SetState( SfxItemState, const SfxPoolItem * );
    void                    SetCachedState();
    void                    DeleteFloatingWindows();
    void                    Invalidate( sal_Bool bWithSlot );

    SfxControllerItem*      ChangeItemLink( SfxControllerItem* pNewBinding );
    SfxControllerItem*      GetItemLink() const;
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
