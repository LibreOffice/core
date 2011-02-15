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
#ifndef _SFX_BINDINGS_HXX
#define _SFX_BINDINGS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>

//________________________________________________________________________________________________________________
//  some other includes
//________________________________________________________________________________________________________________

#include <sfx2/minarray.hxx>
#include <sfx2/viewfrm.hxx>

//________________________________________________________________________________________________________________
//  forwards, typedefs, declarations
//________________________________________________________________________________________________________________

class SfxConfigManager;
class SystemWindow;
class SfxArg;
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
typedef SfxUnoControllerItem* SfxUnoControllerItemPtr;
SV_DECL_PTRARR( SfxUnoControllerArr_Impl, SfxUnoControllerItemPtr, 20, 20 )

//________________________________________________________________________________________________________________
//  defines
//________________________________________________________________________________________________________________

#define SFX_CALLMODE_SLOT           0x00    // sync/async vom Slot
#define SFX_CALLMODE_SYNCHRON       0x01    // synchron im selben Stackframe
#define SFX_CALLMODE_ASYNCHRON      0x02    // asynchron per AppEvent
#define SFX_CALLMODE_RECORD         0x04    // beim Recorden ber"ucksichtigen
#define SFX_CALLMODE_API            0x08    // Call von der API (silent)
#define SFX_CALLMODE_MODAL          0x10    // trotz ModalMode

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

/*  [Beschreibung]

    In jeder SFx-Applikation existiert "uber die Laufzeit von vor
    <SfxApplication::Init()> bis nach <SfxApplication::Exit()> eine Instanz
    der Klasse SfxBindings. Sie wird von der SfxApplication automatisch
    angelegt und zerst"ort. Instanzen werden aber i.d.R. "uber das
    Makro <SFX_BINDINGS> oder den zugeh"origen <SfxViewFrame> besorgt
    werden.

    Die SfxBindings verwalten alle in den an ihr angemeldeten Controllern
    gebundenen Slot-Ids und cachen die jeweiligen <Slot-Server>
    (so nenne wir die Kombination aus SfxShell-Instanz und SfxSlot).
    In den SfxBindings ist gespeichert, ob und welche Controller dirty
    sind sowie welche Slot-Server-Caches jeweils dirty sind. Sie fa"st
    Status-Anfragen (Aufrufe der in der IDL genannten Status-Methoden)
    zusammen, die von derselben Status-Methode bedient werden, und sorgt
    f"ur die Simulation der <Pseudo-Slots>.
*/

{
friend class SfxApplication;
friend class SfxShell;
friend class SfxBindings_Impl;

    SfxBindings_Impl*pImp;          // Daten der Bindings-Instanz
    SfxDispatcher*   pDispatcher;   // zu verwendender Dispatcher
    sal_uInt16       nRegLevel;      // Lock-Level waehrend Reconfig

//#if 0 // _SOLAR__PRIVATE
private:
    SAL_DLLPRIVATE const SfxPoolItem*  Execute_Impl( sal_uInt16 nSlot, const SfxPoolItem **pArgs, sal_uInt16 nModi,
                                    SfxCallMode nCall, const SfxPoolItem **pInternalArgs, sal_Bool bGlobalOnly=sal_False);
    SAL_DLLPRIVATE void SetSubBindings_Impl( SfxBindings* );
    SAL_DLLPRIVATE void UpdateSlotServer_Impl(); // SlotServer aktualisieren
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
//#endif

public:
                     SfxBindings();
                     ~SfxBindings();

    void             HidePopups( bool bHide = true );
    SAL_DLLPRIVATE void HidePopupCtrls_Impl( int bHide = sal_True );

    void             SetDispatcher(SfxDispatcher *pDisp);

    void             Update( sal_uInt16 nId ); // z.B. aus Menu::Activate
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
//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void ContextChanged_Impl();
    SAL_DLLPRIVATE void Execute_Impl( SfxRequest& rReq, const SfxSlot* pSlot, SfxShell* pShell );
    SAL_DLLPRIVATE void DeleteControllers_Impl();
    SAL_DLLPRIVATE SfxPopupAction GetPopupAction_Impl() const;
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
    SAL_DLLPRIVATE void ExecuteGlobal_Impl( sal_uInt16 nId );
    SAL_DLLPRIVATE void InvalidateSlotsInMap_Impl();
    SAL_DLLPRIVATE void AddSlotToInvalidateSlotsMap_Impl( sal_uInt16 nId );
//#endif
};

#ifdef DBG_UTIL
#define ENTERREGISTRATIONS() EnterRegistrations(__FILE__, __LINE__)
#define LEAVEREGISTRATIONS() LeaveRegistrations(USHRT_MAX, __FILE__, __LINE__)
#define DENTERREGISTRATIONS(  ) \
        EnterRegistrations( (ByteString(__FILE__).Append('(').Append(ByteString::CreateFromInt64((sal_uIntPtr)this).Append(')'))).GetBufferAccess(), __LINE__ )
#define DLEAVEREGISTRATIONS(  ) \
        LeaveRegistrations( USHRT_MAX, (ByteString(__FILE__).Append('(').Append(ByteString::CreateFromInt64((sal_uIntPtr)this).Append(')'))).GetBufferAccess(), __LINE__ )
#else
#define ENTERREGISTRATIONS() EnterRegistrations()
#define LEAVEREGISTRATIONS() LeaveRegistrations()
#define DENTERREGISTRATIONS(  ) EnterRegistrations()
#define DLEAVEREGISTRATIONS(  ) LeaveRegistrations()
#endif

//--------------------------------------------------------------------

inline int SfxBindings::IsInRegistrations() const

/*  [Beschreibung]

    Stellt fest, ob an der SfxBindings Instanz gerade <SfxContollerItems>
    an- oder abgemeldet werden, also noch <SfxBindings::EnterRegistrations()>
    Aufrufe nicht mit <SfxBindings::EnterRegistrations()> geschlo"sen wurden.

    [R"uckgabewert]

    int                 sal_True
                        Die SfxBindings Instanz ist gerade im Registrierungs-
                        Modus. Es erfolgen also keine Status-Updates.

                        sal_False
                        Die SfxBindings Instanz ist gerade im normalen
                        Modus. Es k"oennen also Status-Updates erfolgen.
*/

{
    return 0 != nRegLevel;
}

//--------------------------------------------------------------------

#endif

