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
#ifndef _SFXDISPATCH_HXX
#define _SFXDISPATCH_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"

#include <stdarg.h>

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <map>
#include <vector>

class SfxSlotServer;
class SfxShell;
class SfxRequest;
class SfxHintPoster;
class SfxViewFrame;
class SfxBindings;
class SfxItemSet;
class SfxPopupMenuManager;
class SfxModule;
struct SfxDispatcher_Impl;

typedef std::vector<SfxShell*> SfxShellStack_Impl;

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

// Maps the Which() field to a pointer to a SfxPoolItem
class SfxItemPtrMap : public std::map<sal_uInt16, SfxPoolItem*>
{
public:
    ~SfxItemPtrMap()
    {
        for(iterator it = begin(); it != end(); ++it)
            delete it->second;
    }
};

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
    void                _Execute( SfxShell &rShell, const SfxSlot &rSlot,
                                  SfxRequest &rReq,
                                  SfxCallMode eCall = SFX_CALLMODE_STANDARD);
#endif
protected:
    void FlushImpl();

public:
                        SfxDispatcher( SfxDispatcher* pParent );
                        SfxDispatcher( SfxViewFrame *pFrame = 0 );

    SAL_DLLPRIVATE void Construct_Impl( SfxDispatcher* pParent );

    virtual             ~SfxDispatcher();

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

    const SfxSlot*      GetSlot( const OUString& rCommand );

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

    ::com::sun::star::frame::XDispatch*          GetDispatchInterface( const OUString& );
    void                SetDisableFlags( sal_uInt32 nFlags );
    sal_uInt32              GetDisableFlags() const;

    SAL_DLLPRIVATE void SetMenu_Impl();
    SAL_DLLPRIVATE void Update_Impl( sal_Bool bForce = sal_False ); // ObjectBars etc.
    SAL_DLLPRIVATE sal_Bool IsUpdated_Impl() const;
    SAL_DLLPRIVATE int GetShellAndSlot_Impl( sal_uInt16 nSlot, SfxShell **ppShell, const SfxSlot **ppSlot,
                                              sal_Bool bOwnShellsOnly, sal_Bool bModal, sal_Bool bRealSlot=sal_True );
    SAL_DLLPRIVATE void SetReadOnly_Impl( sal_Bool  bOn );
    SAL_DLLPRIVATE sal_Bool GetReadOnly_Impl() const;
    SAL_DLLPRIVATE sal_Bool IsSlotEnabledByFilter_Impl( sal_uInt16 nSID ) const;
    SAL_DLLPRIVATE void SetQuietMode_Impl( sal_Bool bOn );
    SAL_DLLPRIVATE sal_Bool IsReadOnlyShell_Impl( sal_uInt16 nShell ) const;
    SAL_DLLPRIVATE void RemoveShell_Impl( SfxShell& rShell );
    SAL_DLLPRIVATE void DoParentActivate_Impl();
    SAL_DLLPRIVATE void DoParentDeactivate_Impl();
    SAL_DLLPRIVATE void DoActivate_Impl( sal_Bool bMDI, SfxViewFrame* pOld );
    SAL_DLLPRIVATE void DoDeactivate_Impl( sal_Bool bMDI, SfxViewFrame* pNew );
    SAL_DLLPRIVATE void InvalidateBindings_Impl(sal_Bool);
    SAL_DLLPRIVATE sal_uInt16 GetNextToolBox_Impl( sal_uInt16 nPos, sal_uInt16 nType, OUString *pStr );
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
