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
    sal_uInt16 nSlot;
    SfxCallMode eCall;
    sal_uInt16 nModifier;
public:
    sal_uInt16                   GetSlot() const { return nSlot; }
    sal_uInt16                   GetModifier() const { return nModifier; }
    void                     SetModifier( sal_uInt16 nModifierP ) { nModifier = nModifierP; }
    SfxCallMode              GetCallMode() const { return eCall; }
    void                     SetCallMode( SfxCallMode eMode ) { eCall = eMode; }
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
                             SfxExecuteItem(
                                 sal_uInt16 nWhich, sal_uInt16 nSlot, SfxCallMode eMode,
                                 const SfxPoolItem *pArg1, ... );
                             SfxExecuteItem(
                                 sal_uInt16 nWhich, sal_uInt16 nSlot, SfxCallMode eMode );
                             SfxExecuteItem( const SfxExecuteItem& );
};

//=========================================================================

class SFX2_DLLPUBLIC SfxDispatcher
{
    SfxDispatcher_Impl*             pImp;
    sal_Bool                            bFlushed;

private:
    // Search for temporary evaluated Todos
    SAL_DLLPRIVATE sal_Bool CheckVirtualStack( const SfxShell& rShell, sal_Bool bDeep );

#ifndef _SFX_HXX

friend class SfxApplication;
friend class SfxViewFrame;

    DECL_DLLPRIVATE_LINK( EventHdl_Impl, void * );
    DECL_DLLPRIVATE_LINK( PostMsgHandler, SfxRequest * );

    SAL_DLLPRIVATE int Call_Impl( SfxShell& rShell, const SfxSlot &rSlot, SfxRequest &rReq, sal_Bool bRecord );
    SAL_DLLPRIVATE void _Update_Impl( sal_Bool,sal_Bool,sal_Bool,SfxWorkWindow*);
    SAL_DLLPRIVATE void CollectTools_Impl(SfxWorkWindow*);

protected:
friend class SfxBindings;
friend class SfxStateCache;
friend class SfxPopupMenuManager;
friend class SfxHelp;
                        // For bindings: Finding the Message;
                        // level for re-access
    SAL_DLLPRIVATE sal_Bool _TryIntercept_Impl( sal_uInt16 nId, SfxSlotServer &rServer, sal_Bool bModal );
    sal_Bool                _FindServer( sal_uInt16 nId, SfxSlotServer &rServer, sal_Bool bModal );
    sal_Bool                _FillState( const SfxSlotServer &rServer,
                                    SfxItemSet &rState, const SfxSlot *pRealSlot );
    const SfxPoolItem*  _Execute( const SfxSlotServer &rServer );
    void                _Execute( SfxShell &rShell, const SfxSlot &rSlot,
                                  SfxRequest &rReq,
                                  SfxCallMode eCall = SFX_CALLMODE_STANDARD);
    const SfxPoolItem*  _Execute( sal_uInt16 nSlot, SfxCallMode eCall,
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
    virtual sal_uInt16      ExecuteFunction( sal_uInt16 nSID, SfxPoolItem** ppArgs=0, sal_uInt16 nMode=0 );
    sal_uInt16              ExecuteFunction( sal_uInt16 nSID, const SfxItemSet& rArgs , sal_uInt16 nMode=0 );

    virtual void        SetExecuteMode( sal_uInt16 );

    const SfxPoolItem*  Execute( sal_uInt16 nSlot,
                                 SfxCallMode nCall = SFX_CALLMODE_SLOT,
                                 const SfxPoolItem **pArgs = 0,
                                 sal_uInt16 nModi = 0,
                                 const SfxPoolItem **pInternalArgs = 0);

    const SfxPoolItem*  Execute( sal_uInt16 nSlot,
                                 SfxCallMode nCall,
                                 SfxItemSet* pArgs,
                                 SfxItemSet* pInternalArgs,
                                 sal_uInt16 nModi = 0);

    const SfxPoolItem*  Execute( sal_uInt16 nSlot,
                                 SfxCallMode nCall,
                                 const SfxPoolItem *pArg1, ... );

    const SfxPoolItem*  Execute( sal_uInt16 nSlot,
                                 SfxCallMode nCall,
                                 const SfxItemSet &rArgs );

    const SfxPoolItem*  Execute( sal_uInt16 nSlot,
                                 SfxCallMode nCall,
                                 sal_uInt16 nModi,
                                 const SfxItemSet &rArgs );

    sal_uInt16              GetSlotId( const String& rCommand );
    const SfxSlot*      GetSlot( const String& rCommand );

    sal_Bool                IsActive( const SfxShell& rShell );
    sal_Bool                IsOnTop( const SfxShell& rShell );
    sal_uInt16              GetShellLevel( const SfxShell &rShell );
    SfxBindings*        GetBindings() const;

    void                Push( SfxShell& rShell );
    void                Pop( SfxShell& rShell, sal_uInt16 nMode = 0 );

    SfxShell*           GetShell(sal_uInt16 nIdx) const;
    SfxViewFrame*       GetFrame() const;
    SfxModule*      GetModule() const;
    // caller has to clean up the Manager on his own
    static SfxPopupMenuManager* Popup( sal_uInt16 nConfigId,Window *pWin, const Point *pPos );

    void                ExecutePopup( const ResId &rId,
                              Window *pWin = 0, const Point *pPosPixel = 0 );
    static void         ExecutePopup( sal_uInt16 nConfigId = 0,
                              Window *pWin = 0, const Point *pPosPixel = 0 );
    static void         ExecutePopup( sal_uInt16 nConfigId,
                                      Window *pWin, const Point *pPosPixel,
                                      const SfxPoolItem *pArg1, ...  );

    sal_Bool            IsAppDispatcher() const;
    sal_Bool            IsFlushed() const;
    void                Flush();
    void                Lock( sal_Bool bLock );
    sal_Bool                IsLocked( sal_uInt16 nSID = 0 ) const;
    void                SetSlotFilter( sal_Bool bEnable = sal_False,
                                       sal_uInt16 nCount = 0, const sal_uInt16 *pSIDs = 0 );

    void                HideUI( sal_Bool bHide = sal_True );
    void                ShowObjectBar(sal_uInt16 nId, SfxShell *pShell=0) const;
    sal_uInt32          GetObjectBarId( sal_uInt16 nPos ) const;

    SfxItemState        QueryState( sal_uInt16 nSID, const SfxPoolItem* &rpState );
    SfxItemState        QueryState( sal_uInt16 nSID, ::com::sun::star::uno::Any& rAny );

    sal_Bool                IsAllowed( sal_uInt16 nSlot ) const;
    ::com::sun::star::frame::XDispatch*          GetDispatchInterface( const String& );
    void                SetDisableFlags( sal_uInt32 nFlags );
    sal_uInt32              GetDisableFlags() const;

    SAL_DLLPRIVATE sal_Bool HasSlot_Impl( sal_uInt16 );
    SAL_DLLPRIVATE void SetMenu_Impl();
    SAL_DLLPRIVATE void Update_Impl( sal_Bool bForce = sal_False ); // ObjectBars etc.
    SAL_DLLPRIVATE sal_Bool IsUpdated_Impl() const;
    SAL_DLLPRIVATE void DebugOutput_Impl() const;
    SAL_DLLPRIVATE void ResetObjectBars_Impl();
    SAL_DLLPRIVATE int GetShellAndSlot_Impl( sal_uInt16 nSlot, SfxShell **ppShell, const SfxSlot **ppSlot,
                                              sal_Bool bOwnShellsOnly, sal_Bool bModal, sal_Bool bRealSlot=sal_True );
    SAL_DLLPRIVATE void LockUI_Impl( sal_Bool bLock = sal_True );
    SAL_DLLPRIVATE void SetReadOnly_Impl( sal_Bool  bOn );
    SAL_DLLPRIVATE sal_Bool GetReadOnly_Impl() const;
    SAL_DLLPRIVATE sal_Bool IsSlotEnabledByFilter_Impl( sal_uInt16 nSID ) const;
    SAL_DLLPRIVATE void SetQuietMode_Impl( sal_Bool bOn );
    SAL_DLLPRIVATE void SetModalMode_Impl( sal_Bool bOn );
    SAL_DLLPRIVATE sal_Bool IsReadOnlyShell_Impl( sal_uInt16 nShell ) const;
    SAL_DLLPRIVATE void RemoveShell_Impl( SfxShell& rShell );
    SAL_DLLPRIVATE void InsertShell_Impl( SfxShell& rShell, sal_uInt16 nPos );
    SAL_DLLPRIVATE void DoParentActivate_Impl();
    SAL_DLLPRIVATE void DoParentDeactivate_Impl();
    SAL_DLLPRIVATE void DoActivate_Impl( sal_Bool bMDI, SfxViewFrame* pOld );
    SAL_DLLPRIVATE void DoDeactivate_Impl( sal_Bool bMDI, SfxViewFrame* pNew );
    SAL_DLLPRIVATE void InvalidateBindings_Impl(sal_Bool);
    SAL_DLLPRIVATE sal_uInt16 GetNextToolBox_Impl( sal_uInt16 nPos, sal_uInt16 nType, String *pStr );
};

//--------------------------------------------------------------------

inline sal_Bool SfxDispatcher::IsFlushed() const

/*  [Description]

    This method checks if the stack of the SfxDispatchers is flushed, or if
    push- or pop- commands are pending.
*/

{
     return bFlushed;
}

//--------------------------------------------------------------------

inline void SfxDispatcher::Flush()

/*  [Description]

    This method performs outstanding push- and pop- commands. For <SfxShell>s,
    which are new on the stack, the <SfxShell::Activate(sal_Bool)> is invoked with
    bMDI == sal_True, for SfxShells that are removed from the stack, the
    <SfxShell::Deactivate(sal_Bool)> is invoked with bMDI == sal_True
*/

{
    if ( !bFlushed ) FlushImpl();
}

//--------------------------------------------------------------------

inline void SfxDispatcher::Push( SfxShell& rShell )

/*  [Description]

    With this method, a <SfxShell> pushed on to the SfxDispatcher.
    The SfxShell is first marked for push and a timer is set up.
    First when the timer has couted down to zero the push
    ( <SfxDispatcher::Flush()> ) is actually performed and the
    <SfxBindings> is invalidated. While the timer is counting down
    the opposing push and pop commands on the same SfxShell are
    leveled out.
*/

{
    Pop( rShell, SFX_SHELL_PUSH );
}

//--------------------------------------------------------------------

inline sal_Bool SfxDispatcher::IsActive( const SfxShell& rShell )

/*  [Description]

    This method checks whether a particular <SfxShell> instance is
    on the SfxDispatcher.

    [Return value]

    sal_Bool                sal_True
                        The SfxShell instance is on the SfxDispatcher.

                        sal_False
                        The SfxShell instance is not on the SfxDispatcher.
*/

{
    return CheckVirtualStack( rShell, sal_True );
}
//--------------------------------------------------------------------

inline sal_Bool SfxDispatcher::IsOnTop( const SfxShell& rShell )

/*  [Description]

    This method checks whether a particular <SfxShell> instance is on
    top of the SfxDispatcher.

    [Return value]

    sal_Bool                sal_True
                        The SfxShell instance is on the top of
                        the SfxDispatcher.

                        sal_False
                        The SfxShell instance is not on the top of
                        the SfxDispatcher.
*/

{
    return CheckVirtualStack( rShell, sal_False );
}

//--------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
