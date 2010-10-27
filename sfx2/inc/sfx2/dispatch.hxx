/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SFXDISPATCH_HXX
#define _SFXDISPATCH_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"

#include <stdarg.h>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>     // SvUShorts

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>

class SfxSlotServer;
class SfxShell;
class SfxRequest;
class SfxShellStack_Impl;
class SfxHintPoster;
class SfxViewFrame;
class SfxBindings;
class SfxItemSet;
class SfxPopupMenuManager;
class SfxModule;
struct SfxDispatcher_Impl;
struct SfxPlugInInfo_Impl;

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XDispatch;
            }
        }
    }
}

//=========================================================================

#define SFX_SHELL_POP_UNTIL     4
#define SFX_SHELL_POP_DELETE    2
#define SFX_SHELL_PUSH          1

//=========================================================================

typedef SfxPoolItem* SfxPoolItemPtr;
SV_DECL_PTRARR_DEL( SfxItemPtrArray, SfxPoolItemPtr, 4, 4 )

// fuer  shell.cxx
typedef SfxItemPtrArray SfxItemArray_Impl;

class SfxExecuteItem : public SfxItemPtrArray, public SfxPoolItem
{
    USHORT nSlot;
    SfxCallMode eCall;
    USHORT nModifier;
public:
    USHORT                   GetSlot() const { return nSlot; }
    USHORT                   GetModifier() const { return nModifier; }
    void                     SetModifier( USHORT nModifierP ) { nModifier = nModifierP; }
    SfxCallMode              GetCallMode() const { return eCall; }
    void                     SetCallMode( SfxCallMode eMode ) { eCall = eMode; }
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
                             SfxExecuteItem(
                                 USHORT nWhich, USHORT nSlot, SfxCallMode eMode,
                                 const SfxPoolItem *pArg1, ... );
                             SfxExecuteItem(
                                 USHORT nWhich, USHORT nSlot, SfxCallMode eMode );
                             SfxExecuteItem( const SfxExecuteItem& );
};

//=========================================================================

class SFX2_DLLPUBLIC SfxDispatcher
{
    SfxDispatcher_Impl*             pImp;
    BOOL                            bFlushed;

private:
    // auf temporaer ausgewerteten Todos suchen
    SAL_DLLPRIVATE BOOL CheckVirtualStack( const SfxShell& rShell, BOOL bDeep );

#ifndef _SFX_HXX

friend class SfxApplication;
friend class SfxViewFrame;

    DECL_DLLPRIVATE_LINK( EventHdl_Impl, void * );
    DECL_DLLPRIVATE_LINK( PostMsgHandler, SfxRequest * );

    SAL_DLLPRIVATE int Call_Impl( SfxShell& rShell, const SfxSlot &rSlot, SfxRequest &rReq, BOOL bRecord );
    SAL_DLLPRIVATE sal_uInt32 _Update_Impl( BOOL,BOOL,BOOL,SfxWorkWindow*);
    SAL_DLLPRIVATE void CollectTools_Impl(SfxWorkWindow*);

protected:
friend class SfxBindings;
friend class SfxStateCache;
friend class SfxPopupMenuManager;
friend class SfxHelp;
                        // Fuer die Bindings: Finden einer Message; Level fuer
                        // erneuten Zugriff
    SAL_DLLPRIVATE BOOL _TryIntercept_Impl( USHORT nId, SfxSlotServer &rServer, BOOL bModal );
    BOOL                _FindServer( USHORT nId, SfxSlotServer &rServer, BOOL bModal );
    BOOL                _FillState( const SfxSlotServer &rServer,
                                    SfxItemSet &rState, const SfxSlot *pRealSlot );
    const SfxPoolItem*  _Execute( const SfxSlotServer &rServer );
    void                _Execute( SfxShell &rShell, const SfxSlot &rSlot,
                                  SfxRequest &rReq,
                                  SfxCallMode eCall = SFX_CALLMODE_STANDARD);
    const SfxPoolItem*  _Execute( USHORT nSlot, SfxCallMode eCall,
                                  va_list pArgs, const SfxPoolItem *pArg1 );

#endif
protected:
    void FlushImpl();

public:
                        SfxDispatcher( SfxDispatcher* pParent );
                        SfxDispatcher( SfxViewFrame *pFrame = 0 );

    SAL_DLLPRIVATE void Construct_Impl( SfxDispatcher* pParent );

    virtual             ~SfxDispatcher();

    const SfxPoolItem*  Execute( const SfxExecuteItem& rItem );
    virtual USHORT      ExecuteFunction( USHORT nSID, SfxPoolItem** ppArgs=0, USHORT nMode=0 );
    USHORT              ExecuteFunction( USHORT nSID, const SfxItemSet& rArgs , USHORT nMode=0 );

    virtual void        SetExecuteMode( USHORT );

    const SfxPoolItem*  Execute( USHORT nSlot,
                                 SfxCallMode nCall = SFX_CALLMODE_SLOT,
                                 const SfxPoolItem **pArgs = 0,
                                 USHORT nModi = 0,
                                 const SfxPoolItem **pInternalArgs = 0);

    const SfxPoolItem*  Execute( USHORT nSlot,
                                 SfxCallMode nCall,
                                 SfxItemSet* pArgs,
                                 SfxItemSet* pInternalArgs,
                                 USHORT nModi = 0);

    const SfxPoolItem*  Execute( USHORT nSlot,
                                 SfxCallMode nCall,
                                 const SfxPoolItem *pArg1, ... );

    const SfxPoolItem*  Execute( USHORT nSlot,
                                 SfxCallMode nCall,
                                 const SfxItemSet &rArgs );

    const SfxPoolItem*  Execute( USHORT nSlot,
                                 SfxCallMode nCall,
                                 USHORT nModi,
                                 const SfxItemSet &rArgs );

    USHORT              GetSlotId( const String& rCommand );
    const SfxSlot*      GetSlot( const String& rCommand );

    BOOL                IsActive( const SfxShell& rShell );
    BOOL                IsOnTop( const SfxShell& rShell );
    USHORT              GetShellLevel( const SfxShell &rShell );
    SfxBindings*        GetBindings() const;

    void                Push( SfxShell& rShell );
    void                Pop( SfxShell& rShell, USHORT nMode = 0 );

    SfxShell*           GetShell(USHORT nIdx) const;
    SfxViewFrame*       GetFrame() const;
    SfxModule*      GetModule() const;
    // caller has to clean up the Manager on his own
    static SfxPopupMenuManager* Popup( sal_uInt16 nConfigId,Window *pWin, const Point *pPos );

    void                ExecutePopup( const ResId &rId,
                              Window *pWin = 0, const Point *pPosPixel = 0 );
    static void         ExecutePopup( USHORT nConfigId = 0,
                              Window *pWin = 0, const Point *pPosPixel = 0 );
    static void         ExecutePopup( USHORT nConfigId,
                                      Window *pWin, const Point *pPosPixel,
                                      const SfxPoolItem *pArg1, ...  );

    void                EnterAction( const String& rName );
    void                LeaveAction();

    BOOL                IsAppDispatcher() const;
    BOOL                IsFlushed() const;
    void                Flush();
    void                Lock( BOOL bLock );
    BOOL                IsLocked( USHORT nSID = 0 ) const;
    void                SetSlotFilter( BOOL bEnable = FALSE,
                                       USHORT nCount = 0, const USHORT *pSIDs = 0 );

    void                HideUI( BOOL bHide = TRUE );
    void                ShowObjectBar(USHORT nId, SfxShell *pShell=0) const;
    sal_uInt32          GetObjectBarId( USHORT nPos ) const;

    SfxItemState        QueryState( USHORT nSID, const SfxPoolItem* &rpState );
    SfxItemState        QueryState( USHORT nSID, ::com::sun::star::uno::Any& rAny );

    BOOL                IsAllowed( USHORT nSlot ) const;
    ::com::sun::star::frame::XDispatch*          GetDispatchInterface( const String& );
    void                SetDisableFlags( sal_uInt32 nFlags );
    sal_uInt32              GetDisableFlags() const;

    SAL_DLLPRIVATE BOOL HasSlot_Impl( USHORT );
    SAL_DLLPRIVATE void SetMenu_Impl();
    SAL_DLLPRIVATE long Update_Impl( BOOL bForce = FALSE ); // ObjectBars etc.
    SAL_DLLPRIVATE BOOL IsUpdated_Impl() const;
    SAL_DLLPRIVATE void DebugOutput_Impl() const;
    SAL_DLLPRIVATE void ResetObjectBars_Impl();
    SAL_DLLPRIVATE int GetShellAndSlot_Impl( USHORT nSlot, SfxShell **ppShell, const SfxSlot **ppSlot,
                                              BOOL bOwnShellsOnly, BOOL bModal, BOOL bRealSlot=TRUE );
    SAL_DLLPRIVATE void LockUI_Impl( BOOL bLock = TRUE );
    SAL_DLLPRIVATE void SetReadOnly_Impl( BOOL  bOn );
    SAL_DLLPRIVATE BOOL GetReadOnly_Impl() const;
    SAL_DLLPRIVATE BOOL IsSlotEnabledByFilter_Impl( USHORT nSID ) const;
    SAL_DLLPRIVATE void SetQuietMode_Impl( BOOL bOn );
    SAL_DLLPRIVATE void SetModalMode_Impl( BOOL bOn );
    SAL_DLLPRIVATE BOOL IsReadOnlyShell_Impl( USHORT nShell ) const;
    SAL_DLLPRIVATE void RemoveShell_Impl( SfxShell& rShell );
    SAL_DLLPRIVATE void InsertShell_Impl( SfxShell& rShell, USHORT nPos );
    SAL_DLLPRIVATE void DoParentActivate_Impl();
    SAL_DLLPRIVATE void DoParentDeactivate_Impl();
    SAL_DLLPRIVATE void DoActivate_Impl( BOOL bMDI, SfxViewFrame* pOld );
    SAL_DLLPRIVATE void DoDeactivate_Impl( BOOL bMDI, SfxViewFrame* pNew );
    SAL_DLLPRIVATE void InvalidateBindings_Impl(BOOL);
    SAL_DLLPRIVATE USHORT GetNextToolBox_Impl( USHORT nPos, USHORT nType, String *pStr );
};

//--------------------------------------------------------------------

inline BOOL SfxDispatcher::IsFlushed() const

/*  [Beschreibung]

    Mit dieser Methode l"a"st sich erfragen, ob der Stack des
    SfxDispatchers geflusht ist, oder noch Push- oder Pop-Befehle
    ausstehen.
*/

{
     return bFlushed;
}

//--------------------------------------------------------------------

inline void SfxDispatcher::Flush()

/*  [Beschreibung]

    Diese Methode f"uhrt ausstehenden Push- und Pop-Befehle aus.
    F"ur <SfxShell>s, die dabei neu auf den Stack kommen, wird
    <SfxShell::Activate(BOOL)> mit bMDI == TRUE aufgerufen, f"ur
    SfxShells, die vom Stack entfernt werden, wird <SfxShell::Deactivate(BOOL)>
    mit bMDI == TRUE aufgerufen.
*/

{
    if ( !bFlushed ) FlushImpl();
}

//--------------------------------------------------------------------

inline void SfxDispatcher::Push( SfxShell& rShell )

/*  [Beschreibung]

    Mit dieser Methode wird eine <SfxShell> auf den SfxDispatcher
    gepusht. Die SfxShell wird zun"achst zum pushen vermerkt und
    es wird ein Timer aufgesetzt. Erst bei Ablauf des Timers wird
    tats"achlich gepusht (<SfxDispatcher::Flush()>) und die <SfxBindings>
    werden invalidiert. W"ahrend der Timer l"auft gleichen sich
    entgegengesetzte Push und Pop Befehle mit derselben SfxShell aus.
*/

{
    Pop( rShell, SFX_SHELL_PUSH );
}

//--------------------------------------------------------------------

inline BOOL SfxDispatcher::IsActive( const SfxShell& rShell )

/*  [Beschreibung]

    Mit dieser Methode kann abgefragt werden, ob sich eine bestimmte
    <SfxShell>-Instanz auf dem SfxDispatcher befindet.

    [R"uckgabewert]

    BOOL                TRUE
                        Die SfxShell-Instanz befindet sich auf dem
                        SfxDispatcher.

                        FALSE
                        Die SfxShell-Instanz befindet sich nicht auf dem
                        SfxDispatcher.

*/

{
    return CheckVirtualStack( rShell, TRUE );
}
//--------------------------------------------------------------------

inline BOOL SfxDispatcher::IsOnTop( const SfxShell& rShell )

/*  [Beschreibung]

    Mit dieser Methode kann abgefragt werden, ob sich eine bestimmte
    <SfxShell>-Instanz zuoberst auf dem SfxDispatcher befindet.

    [R"uckgabewert]

    BOOL                TRUE
                        Die SfxShell-Instanz befindet sich als oberste
                        SfxShell auf dem SfxDispatcher.

                        FALSE
                        Die SfxShell-Instanz befindet sich nicht als
                        oberste SfxShell auf dem SfxDispatcher.

*/

{
    return CheckVirtualStack( rShell, FALSE );
}

//--------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
