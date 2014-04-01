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
#ifndef INCLUDED_SFX2_DISPATCH_HXX
#define INCLUDED_SFX2_DISPATCH_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>

#include <stdarg.h>

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <boost/scoped_ptr.hpp>

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



#define SFX_SHELL_POP_UNTIL     4
#define SFX_SHELL_POP_DELETE    2
#define SFX_SHELL_PUSH          1



class SFX2_DLLPUBLIC SfxDispatcher
{
    boost::scoped_ptr<SfxDispatcher_Impl> pImp;

private:
    // Search for temporary evaluated Todos
    SAL_DLLPRIVATE bool CheckVirtualStack( const SfxShell& rShell, bool bDeep );

#ifndef _SFX_HXX

friend class SfxApplication;
friend class SfxViewFrame;

    DECL_DLLPRIVATE_LINK( EventHdl_Impl, void * );
    DECL_DLLPRIVATE_LINK( PostMsgHandler, SfxRequest * );

    SAL_DLLPRIVATE int Call_Impl( SfxShell& rShell, const SfxSlot &rSlot, SfxRequest &rReq, bool bRecord );
    SAL_DLLPRIVATE void _Update_Impl( bool,bool,bool,SfxWorkWindow*);
    SAL_DLLPRIVATE void CollectTools_Impl(SfxWorkWindow*);

protected:
friend class SfxBindings;
friend class SfxStateCache;
friend class SfxPopupMenuManager;
friend class SfxHelp;
                        // For bindings: Finding the Message;
                        // level for re-access
    SAL_DLLPRIVATE bool _TryIntercept_Impl( sal_uInt16 nId, SfxSlotServer &rServer, bool bModal );
    bool                _FindServer( sal_uInt16 nId, SfxSlotServer &rServer, bool bModal );
    bool                _FillState( const SfxSlotServer &rServer,
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

    bool                IsActive( const SfxShell& rShell );
    sal_uInt16          GetShellLevel( const SfxShell &rShell );
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

    bool                IsAppDispatcher() const;
    bool                IsFlushed() const;
    void                Flush();
    void                Lock( bool bLock );
    bool                IsLocked( sal_uInt16 nSID = 0 ) const;
    // bEnable can be sal_True,sal_False, or 2(some kind of read-only override hack)
    void                SetSlotFilter( sal_Bool bEnable = sal_False,
                                       sal_uInt16 nCount = 0, const sal_uInt16 *pSIDs = 0 );

    void                HideUI( bool bHide = true );
    void                ShowObjectBar(sal_uInt16 nId, SfxShell *pShell=0) const;
    sal_uInt32          GetObjectBarId( sal_uInt16 nPos ) const;

    SfxItemState        QueryState( sal_uInt16 nSID, const SfxPoolItem* &rpState );
    SfxItemState        QueryState( sal_uInt16 nSID, ::com::sun::star::uno::Any& rAny );

    ::com::sun::star::frame::XDispatch*          GetDispatchInterface( const OUString& );
    void                SetDisableFlags( sal_uInt32 nFlags );
    sal_uInt32              GetDisableFlags() const;

    SAL_DLLPRIVATE void SetMenu_Impl();
    SAL_DLLPRIVATE void Update_Impl( bool bForce = false ); // ObjectBars etc.
    SAL_DLLPRIVATE bool IsUpdated_Impl() const;
    SAL_DLLPRIVATE int GetShellAndSlot_Impl( sal_uInt16 nSlot, SfxShell **ppShell, const SfxSlot **ppSlot,
                                              bool bOwnShellsOnly, bool bModal, bool bRealSlot=true );
    SAL_DLLPRIVATE void SetReadOnly_Impl( bool  bOn );
    SAL_DLLPRIVATE bool GetReadOnly_Impl() const;
    // bEnable can be sal_True,sal_False, or 2(some kind of read-only override hack)
    SAL_DLLPRIVATE sal_Bool IsSlotEnabledByFilter_Impl( sal_uInt16 nSID ) const;
    SAL_DLLPRIVATE void SetQuietMode_Impl( bool bOn );
    SAL_DLLPRIVATE bool IsReadOnlyShell_Impl( sal_uInt16 nShell ) const;
    SAL_DLLPRIVATE void RemoveShell_Impl( SfxShell& rShell );
    SAL_DLLPRIVATE void DoParentActivate_Impl();
    SAL_DLLPRIVATE void DoParentDeactivate_Impl();
    SAL_DLLPRIVATE void DoActivate_Impl( bool bMDI, SfxViewFrame* pOld );
    SAL_DLLPRIVATE void DoDeactivate_Impl( bool bMDI, SfxViewFrame* pNew );
    SAL_DLLPRIVATE void InvalidateBindings_Impl(bool);
    SAL_DLLPRIVATE sal_uInt16 GetNextToolBox_Impl( sal_uInt16 nPos, sal_uInt16 nType, OUString *pStr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
