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
#ifndef _SFX_BINDINGS_HXX
#define _SFX_BINDINGS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include "rtl/strbuf.hxx"
#include <tools/link.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <vector>

//________________________________________________________________________________________________________________
//  some other includes
//________________________________________________________________________________________________________________

#include <sfx2/minarray.hxx>
#include <sfx2/viewfrm.hxx>

//________________________________________________________________________________________________________________
//  forwards, typedefs, declarations
//________________________________________________________________________________________________________________

class SystemWindow;
class SfxSlot;
class SfxSlotServer;
class SfxControllerItem;
class SfxStateCache;
class SfxItemSet;
class SfxDispatcher;
class SfxBindings;
class SfxBindings_Impl;
class Timer;
struct SfxFoundCache_Impl;
class SfxFoundCacheArr_Impl;
class SfxWorkWindow;
class SfxUnoControllerItem;

typedef std::vector<SfxUnoControllerItem*> SfxUnoControllerArr_Impl;

//________________________________________________________________________________________________________________
//  defines
//________________________________________________________________________________________________________________

#define SFX_CALLMODE_SLOT               0x00    // sync/async from Slot
#define SFX_CALLMODE_SYNCHRON           0x01    // synchronously in the same Stackframe
#define SFX_CALLMODE_ASYNCHRON          0x02    // asynchronously via AppEvent
#define SFX_CALLMODE_RECORD             0x04    // take into accont while recording
#define SFX_CALLMODE_API                0x08    // API call (silent)
#define SFX_CALLMODE_MODAL              0x10    // despite ModalMode

#define SFX_CALLMODE_STANDARD       SFX_CALLMODE_RECORD
typedef sal_uInt16 SfxCallMode;

enum SfxPopupAction
{
    SFX_POPUP_DELETE,
    SFX_POPUP_HIDE,
    SFX_POPUP_SHOW
};

//====================================================================
class SFX2_DLLPUBLIC SfxBindings: public SfxBroadcaster

/*  [Description]

    In each SFx application one instance of the SfxBindings-Class will
    exists from <SfxApplication::Init()> until <SfxApplication::Exit()>.
    This instance is automatically created and destroyed by SfxApplication.
    However these instances will be handled by the Macro <SFX_BINDINGS>
    or the associated <SfxViewFrame>.

    The SfxBindings manages all of its Slot-Ids bound by the registerd
    controllers and keeps a cache of the <Slot-Server> respectively.
    (it is what we call the combination of SfxShell instance and SfxSlot).
    In the SfxBindings it is stored, if and in this case which controllers
    that are dirty and which Slot-Server-Caches are dirty respectively.
    It summarizes status queries (calls to the status methods specified
    in the IDL) that are served by the same state methods, and handles
    the simulation of <Pseudo-Slots>.
*/

{
friend class SfxApplication;
friend class SfxShell;
friend class SfxBindings_Impl;

    SfxBindings_Impl*pImp;           // Data of the Bindings instance
    SfxDispatcher*   pDispatcher;    // Dispatcher, to be used
    sal_uInt16       nRegLevel;      // Lock-Level while Reconfig

private:
    SAL_DLLPRIVATE const SfxPoolItem*  Execute_Impl( sal_uInt16 nSlot, const SfxPoolItem **pArgs, sal_uInt16 nModi,
                                    SfxCallMode nCall, const SfxPoolItem **pInternalArgs, sal_Bool bGlobalOnly=sal_False);
    SAL_DLLPRIVATE void SetSubBindings_Impl( SfxBindings* );
    SAL_DLLPRIVATE void UpdateSlotServer_Impl(); // Update SlotServer
    SAL_DLLPRIVATE SfxItemSet* CreateSet_Impl( SfxStateCache* &pCache,
                                    const SfxSlot* &pRealSlot,
                                    const SfxSlotServer**,
                                    SfxFoundCacheArr_Impl& );
    SAL_DLLPRIVATE sal_uInt16 GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt = 0 );
    SAL_DLLPRIVATE void Update_Impl( SfxStateCache* pCache );
    SAL_DLLPRIVATE void UpdateControllers_Impl(
                            const SfxInterface* pIF,
                            const SfxFoundCache_Impl* pFound,
                            const SfxPoolItem *pItem,
                            SfxItemState eItemState );
    DECL_DLLPRIVATE_LINK( NextJob_Impl, Timer * );

public:
                     SfxBindings();
                     ~SfxBindings();

    void             HidePopups( bool bHide = true );
    SAL_DLLPRIVATE void HidePopupCtrls_Impl( bool bHide = true );

    void             SetDispatcher(SfxDispatcher *pDisp);

    void             Update( sal_uInt16 nId ); // For example, from  Menu::Activate
    void             Update();
    SAL_DLLPRIVATE void StartUpdate_Impl(sal_Bool bComplete=sal_False);
    void             Invalidate( sal_uInt16 nId );
    void             Invalidate( const sal_uInt16* pIds );
    void             InvalidateShell( const SfxShell &rSh, sal_Bool bDeep = sal_False );
    void             InvalidateAll( sal_Bool bWithMsg );
    void             SetState( const SfxItemSet &rSet );
    void             SetState( const SfxPoolItem &rItem );
    void             Invalidate( sal_uInt16 nId, sal_Bool bWithItem, sal_Bool bWithMsg=sal_False);
    void             Invalidate( sal_uInt16 nId, sal_Bool bWithMsg);
    sal_Bool         IsInUpdate() const;
    void             SetVisibleState( sal_uInt16 nId, sal_Bool bShow );

    sal_Bool         IsBound( sal_uInt16 nMsgId, sal_uInt16 nStartSearchAt = 0 );

    const SfxSlot*   GetSlot( sal_uInt16 nMsgId );
    SfxStateCache*   GetStateCache( sal_uInt16 nId, sal_uInt16 *pPos = 0 );
    SAL_DLLPRIVATE SfxStateCache* GetAnyStateCache_Impl( sal_uInt16 nId );
    SfxItemState     QueryState( sal_uInt16 nSID, SfxPoolItem* &rpState );

    const SfxPoolItem*  ExecuteSynchron( sal_uInt16 nSlot,
                                 const SfxPoolItem **pArgs = 0,
                                 sal_uInt16 nModi = 0,
                                 const SfxPoolItem **pInternalArgs = 0);
    sal_Bool         Execute( sal_uInt16 nSlot,
                                 const SfxPoolItem **pArgs = 0,
                                 sal_uInt16 nModi = 0,
                                 SfxCallMode nCall = SFX_CALLMODE_SLOT,
                                 const SfxPoolItem **pInternalArgs = 0);

    SAL_DLLPRIVATE void SetDispatchProvider_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & rFrame );
    SAL_DLLPRIVATE const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & GetDispatchProvider_Impl() const;
    void             SetActiveFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & rFrame );
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > GetActiveFrame() const;
                     // Reconfig
    int              IsInRegistrations() const;
    sal_uInt16           EnterRegistrations(const char *pFile = 0, int nLine = 0);
    void             LeaveRegistrations( sal_uInt16 nLevel = USHRT_MAX, const char *pFile = 0, int nLine = 0 );
    void             Register( SfxControllerItem& rBinding );
    void             Release( SfxControllerItem& rBinding );
    SystemWindow*    GetSystemWindow() const;
    SfxDispatcher*   GetDispatcher() const
                     { return pDispatcher; }
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > GetRecorder() const;
    com::sun::star::uno::Reference < com::sun::star::frame::XDispatch >
                    GetDispatch( const SfxSlot*, const com::sun::star::util::URL& aURL, sal_Bool bMasterCommand );
    SAL_DLLPRIVATE void ContextChanged_Impl();
    SAL_DLLPRIVATE void Execute_Impl( SfxRequest& rReq, const SfxSlot* pSlot, SfxShell* pShell );
    SAL_DLLPRIVATE void DeleteControllers_Impl();
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl()  { return pDispatcher; }
    SAL_DLLPRIVATE void ClearCache_Impl( sal_uInt16 nSlotId );
    SAL_DLLPRIVATE sal_Bool IsInUpdate_Impl() const{ return IsInUpdate(); }
    SAL_DLLPRIVATE void RegisterInternal_Impl( SfxControllerItem& rBinding );
    SAL_DLLPRIVATE void Register_Impl( SfxControllerItem& rBinding, sal_Bool );
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl() const;
    SAL_DLLPRIVATE void SetWorkWindow_Impl( SfxWorkWindow* );
    SAL_DLLPRIVATE SfxBindings* GetSubBindings_Impl( sal_Bool bTop = sal_False ) const;
    SAL_DLLPRIVATE void InvalidateUnoControllers_Impl();
    SAL_DLLPRIVATE void RegisterUnoController_Impl( SfxUnoControllerItem* );
    SAL_DLLPRIVATE void ReleaseUnoController_Impl( SfxUnoControllerItem* );
    SAL_DLLPRIVATE sal_Bool ExecuteCommand_Impl( const String& rCommand );
    SAL_DLLPRIVATE void SetRecorder_Impl( com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder >& );
    SAL_DLLPRIVATE void InvalidateSlotsInMap_Impl();
    SAL_DLLPRIVATE void AddSlotToInvalidateSlotsMap_Impl( sal_uInt16 nId );
};

#ifdef DBG_UTIL
#define ENTERREGISTRATIONS() EnterRegistrations(__FILE__, __LINE__)
#define LEAVEREGISTRATIONS() LeaveRegistrations(USHRT_MAX, __FILE__, __LINE__)
#define DENTERREGISTRATIONS() \
        EnterRegistrations( rtl::OStringBuffer(__FILE__).append('(').append(reinterpret_cast<sal_Int64>(this)).append(')').getStr(), __LINE__ )
#define DLEAVEREGISTRATIONS(  ) \
        LeaveRegistrations( USHRT_MAX, rtl::OStringBuffer(__FILE__).append('(').append(reinterpret_cast<sal_Int64>(this)).append(')').getStr(), __LINE__ )
#else
#define ENTERREGISTRATIONS() EnterRegistrations()
#define LEAVEREGISTRATIONS() LeaveRegistrations()
#define DENTERREGISTRATIONS() EnterRegistrations()
#define DLEAVEREGISTRATIONS() LeaveRegistrations()
#endif

//--------------------------------------------------------------------

inline int SfxBindings::IsInRegistrations() const

/*  [Description]

    Determines whether the <SfxContollerItems> SfxBindings instance is
    registerd or unregisted, i.e. <SfxBindings::EnterRegistrations()>
    calls that have not been closed by <SfxBindings::LeaveRegistrations()>.

    [Return value]

    int                 sal_True
                        The SfxBindings instance is currently in
                        Registration-Mode. No status updates .

    int sal_False
                        The SfxBindings instance is the normal mode.
                        Status updates can be done.
*/

{
    return 0 != nRegLevel;
}

//--------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
