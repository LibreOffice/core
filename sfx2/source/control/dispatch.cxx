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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/undo.hxx>
#include <vcl/wrkwin.hxx>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>  // due to bsearch
#include <algorithm>

#include <svtools/helpopt.hxx>

// due to nAutoPageID
#include "appdata.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/hintpost.hxx>
#include "slotserv.hxx"
#include <sfx2/ipclient.hxx>
#include "sfxtypes.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/mnumgr.hxx>
#include "workwin.hxx"
#include <rtl/strbuf.hxx>

#include <deque>
#include <vector>

DBG_NAME(SfxDispatcherFlush)
DBG_NAME(SfxDispatcherFillState)

typedef std::vector<SfxRequest*> SfxRequestPtrArray;

struct SfxToDo_Impl
{
    SfxShell*  pCluster;
    bool       bPush;
    bool       bDelete;
    bool       bUntil;

    SfxToDo_Impl()
        : pCluster(0)
        , bPush(false)
        , bDelete(false)
        , bUntil(false)
                {}
    SfxToDo_Impl( bool bOpPush, bool bOpDelete, bool bOpUntil, SfxShell& rCluster )
        : pCluster(&rCluster)
        , bPush(bOpPush)
        , bDelete(bOpDelete)
        , bUntil(bOpUntil)
                {}
    ~SfxToDo_Impl(){}

    bool operator==( const SfxToDo_Impl& rWith ) const
    { return pCluster==rWith.pCluster && bPush==rWith.bPush; }
};

struct SfxObjectBars_Impl
{
    sal_uInt32     nResId;  // Resource - and ConfigId of the Toolbox
    sal_uInt16     nMode;   // special visibility flags
    OUString       aName;
    SfxInterface*  pIFace;

    SfxObjectBars_Impl() : nResId(0), nMode(0), pIFace(NULL) {}
};

//------------------------------------------------------------------

struct SfxDispatcher_Impl
{
    //When the dispatched is locked, SfxRequests accumulate in aReqArr for
    //later dispatch when unlocked via Post
    //
    //The pointers are typically deleted in Post, only if we never get around
    //to posting them do we delete the unposted requests.
    SfxRequestPtrArray aReqArr;
    ~SfxDispatcher_Impl()
    {
        for (SfxRequestPtrArray::iterator aI = aReqArr.begin(), aEnd = aReqArr.end(); aI != aEnd; ++aI)
            delete *aI;
    }
    const SfxSlotServer* pCachedServ1;  // last called message
    const SfxSlotServer* pCachedServ2;  // penultimate called Message
    SfxShellStack_Impl   aStack;        // active functionality
    Timer                aTimer;        // for Flush
    std::deque<SfxToDo_Impl> aToDoStack;    // not processed Push/Pop
    SfxViewFrame*        pFrame;        // NULL or associated Frame
    SfxDispatcher*       pParent;       // AppDispatcher, NULL if possible
    SfxHintPosterRef     xPoster;       // Execute asynchronous
    sal_Bool             bFlushing;     // sal_True during Flush //?
    sal_Bool             bUpdated;      // Update_Impl has run
    sal_Bool             bLocked;       // No Execute
    sal_Bool     bInvalidateOnUnlock;   // because someone asked
    sal_Bool             bActive;       // not to be confused with set!
    sal_Bool*       pInCallAliveFlag;   // view the Destructor Stack
    SfxObjectBars_Impl   aObjBars[SFX_OBJECTBAR_MAX];
    SfxObjectBars_Impl   aFixedObjBars[SFX_OBJECTBAR_MAX];
    std::vector<sal_uInt32> aChildWins;
    sal_uInt32           nEventId;      // EventId UserEvent
    sal_Bool             bNoUI;         // UI only from Parent Dispatcher
    sal_Bool             bReadOnly;     // Document is ReadOnly
    sal_Bool             bQuiet;        // Only use parent dispatcher
    sal_Bool             bModal;        // Only slots from parent dispatcher

    sal_Bool           bFilterEnabling; // sal_True=filter enabled slots,
                                        // 2==ReadOnlyDoc overturned
    sal_uInt16           nFilterCount;  // Number of SIDs in pFilterSIDs
    const sal_uInt16*    pFilterSIDs;   // sorted Array of SIDs
    sal_uInt32           nDisableFlags;
};

//------------------------------------------------------------------

#define SFX_FLUSH_TIMEOUT    50

//====================================================================
sal_Bool SfxDispatcher::IsLocked( sal_uInt16 ) const

/*  [Description]

    With this method it can be determined whether the SfxDispatcher is
    locked or unlocked. A locked SfxDispatcher does not perform <SfxRequest>s
    and no longer provides any status information. It behaves as if all the
    slots are disabled.

    The dispatcher is also marked as blocked, if all Dispatcher are locked
    (<SfxApplication::LockDispatcher()>) or the associated top frame is in the
    modal-mode and if the specified slot are handled as frame-specific
    (ie, not served by the application).
*/

{
    return pImp->bLocked;
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::IsAppDispatcher() const

/*  [Description]

    With this method it can be determined if the SfxDispacher is the
    applications dispatcher.

    [Return value]

    sal_Bool   sal_True       it is the application dispatcher.
               sal_Fals       it is a SfxViewFrame dispatcher.
*/

{
    return !pImp->pFrame;
}

//--------------------------------------------------------------------
int SfxDispatcher::Call_Impl( SfxShell& rShell, const SfxSlot &rSlot, SfxRequest &rReq, sal_Bool bRecord )

/*  [Description]

    Helper function to check whether a slot can be executed and
    check the execution itself
*/

{
    SFX_STACK(SfxDispatcher::Call_Impl);

    // The slot may be called (meaning enabled)
    if ( rSlot.IsMode(SFX_SLOT_FASTCALL) || rShell.CanExecuteSlot_Impl(rSlot) )
    {
        if ( GetFrame() )
        {
            // Recording may start
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                    GetFrame()->GetFrame().GetFrameInterface(),
                    com::sun::star::uno::UNO_QUERY);

            com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                    xFrame,
                    com::sun::star::uno::UNO_QUERY);

            if ( xSet.is() )
            {
                com::sun::star::uno::Any aProp = xSet->getPropertyValue("DispatchRecorderSupplier");
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;
                aProp >>= xSupplier;
                if(xSupplier.is())
                    xRecorder = xSupplier->getDispatchRecorder();

                if ( bRecord && xRecorder.is() && !rSlot.IsMode(SFX_SLOT_NORECORD) )
                    rReq.Record_Impl( rShell, rSlot, xRecorder, GetFrame() );
            }
        }
        // Get all that is needed, because the slot may not have survived the
        // Execute if it is a 'pseudo slot' for macros or verbs.
        sal_Bool bAutoUpdate = rSlot.IsMode(SFX_SLOT_AUTOUPDATE);

        // API-call parentheses and document-lock during the calls
        {
            // 'this' must respond in the Destructor
            sal_Bool bThisDispatcherAlive = sal_True;
            sal_Bool *pOldInCallAliveFlag = pImp->pInCallAliveFlag;
            pImp->pInCallAliveFlag = &bThisDispatcherAlive;

            SfxExecFunc pFunc = rSlot.GetExecFnc();
            rShell.CallExec( pFunc, rReq );

            // If 'this' is still alive
            if ( bThisDispatcherAlive )
                pImp->pInCallAliveFlag = pOldInCallAliveFlag;
            else
            {
                if ( pOldInCallAliveFlag )
                {
                    // also protect nested stack frames
                    *pOldInCallAliveFlag = sal_False;
                }

                // do nothing after this object is dead
                return rReq.IsDone();
            }
        }

        if ( rReq.IsDone() )
        {
            SfxBindings *pBindings = GetBindings();

            // When AutoUpdate update immediately; "Pseudoslots" must not be
            // Autoupdate!
            if ( bAutoUpdate && pBindings )
            {
                const SfxSlot* pSlave = rSlot.GetLinkedSlot();
                if (pSlave)
                {
                    // When enum slots take any bound slave slot
                    while (!pBindings->IsBound(pSlave->GetSlotId()) && pSlave != &rSlot )
                        pSlave = pSlave->GetLinkedSlot();
                    pBindings->Invalidate(pSlave->GetSlotId());
                    pBindings->Update(pSlave->GetSlotId());
                }
                else
                {
                    pBindings->Invalidate(rSlot.GetSlotId());
                    pBindings->Update(rSlot.GetSlotId());
                }
            }

            return sal_True;
        }
    }

    return sal_False;
}

//====================================================================
void SfxDispatcher::Construct_Impl( SfxDispatcher* pParent )
{
    pImp = new SfxDispatcher_Impl;
    bFlushed = sal_True;

    pImp->pCachedServ1 = 0;
    pImp->pCachedServ2 = 0;
    pImp->bFlushing = sal_False;
    pImp->bUpdated = sal_False;
    pImp->bLocked = sal_False;
    pImp->bActive = sal_False;
    pImp->pParent = NULL;
    pImp->bNoUI = sal_False;
    pImp->bReadOnly = sal_False;
    pImp->bQuiet = sal_False;
    pImp->bModal = sal_False;
    pImp->pInCallAliveFlag = 0;
    pImp->bFilterEnabling = sal_False;
    pImp->nFilterCount = 0;
    pImp->pFilterSIDs = 0;
    pImp->nDisableFlags = 0;

    pImp->pParent = pParent;

    pImp->bInvalidateOnUnlock = sal_False;

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].nResId = 0;

    GenLink aGenLink( LINK(this, SfxDispatcher, PostMsgHandler) );

    pImp->xPoster = new SfxHintPoster(aGenLink);

    pImp->aTimer.SetTimeout(SFX_FLUSH_TIMEOUT);
    pImp->aTimer.SetTimeoutHdl( LINK(this, SfxDispatcher, EventHdl_Impl ) );
}

SfxDispatcher::SfxDispatcher( SfxDispatcher* pParent )
{
    Construct_Impl( pParent );
    pImp->pFrame = 0;
}

SfxDispatcher::SfxDispatcher( SfxViewFrame *pViewFrame )

/*  [Description]

    The constructor of the SfxDispatcher class places a stack of empty
    <SfxShell> pointers. It is not initially locked and is considered flushed.
*/

{
    if ( pViewFrame )
    {
        SfxViewFrame *pFrame = pViewFrame->GetParentViewFrame();
        if ( pFrame )
            Construct_Impl(  pFrame->GetDispatcher() );
        else
            Construct_Impl( 0 );
    }
    else
        Construct_Impl( 0 );
    pImp->pFrame = pViewFrame;
}

//====================================================================
SfxDispatcher::~SfxDispatcher()

/*  [Description]

    The destructor of the SfxDispatcher class should not be called when the
    SfxDispatcher instance is active. It may, however, still be a <SfxShell>
    pointer on the stack.
*/

{
#ifdef DBG_UTIL
    OStringBuffer sTemp(RTL_CONSTASCII_STRINGPARAM("Delete Dispatcher "));
    sTemp.append(reinterpret_cast<sal_Int64>(this));
    OSL_TRACE("%s", sTemp.getStr());
    DBG_ASSERT( !pImp->bActive, "deleting active Dispatcher" );
#endif

    // So that no timer by Reschedule in PlugComm strikes the LeaveRegistrations
    pImp->aTimer.Stop();
    pImp->xPoster->SetEventHdl( Link() );

    // Notify the stack varialbles in Call_Impl
    if ( pImp->pInCallAliveFlag )
        *pImp->pInCallAliveFlag = sal_False;

    // Get bindings and application
    SfxApplication *pSfxApp = SFX_APP();
    SfxBindings* pBindings = GetBindings();

    // When not flushed, revive the bindings
    if ( pBindings && !pSfxApp->IsDowning() && !bFlushed )
        pBindings->DLEAVEREGISTRATIONS();

    // may unregister the bindings
    while ( pBindings )
    {
        if ( pBindings->GetDispatcher_Impl() == this)
            pBindings->SetDispatcher(0);
        pBindings = pBindings->GetSubBindings_Impl();
    }

    delete pImp;
}

//====================================================================
void SfxDispatcher::Pop
(
    SfxShell&   rShell,  /* the stack to take the SfxShell instance. */

    sal_uInt16   nMode   /* SFX_SHELL_POP_UNTIL
                            Also all 'rShell' of SfxShells are taken from the
                            stack.

                            SFX_SHELL_POP_DELETE
                            All SfxShells actually taken from the stack
                            will be deleted.

                            SFX_SHELL_PUSH (InPlace use only)
                            The Shell is pushed. */
)
/*  [Description]

    With this method, one or more <SfxShell> are poped from the SfxDispatcher.
    The SfxShell is marked for popping and a timer is set up. Only when the
    timer has reached the end, the pop is actually performed
    ( <SfxDispatcher::Flush()> ) and the <SfxBindings> is invalidated.
    While the timer is running the opposing push and pop commands on one
    SfxShell cancel each other out.
*/

{
    DBG_ASSERT( rShell.GetInterface(),
                "pushing SfxShell without previous RegisterInterface()" );

    bool bDelete = (nMode & SFX_SHELL_POP_DELETE) == SFX_SHELL_POP_DELETE;
    bool bUntil = (nMode & SFX_SHELL_POP_UNTIL) == SFX_SHELL_POP_UNTIL;
    bool bPush = (nMode & SFX_SHELL_PUSH) == SFX_SHELL_PUSH;

    SfxApplication *pSfxApp = SFX_APP();

    SAL_INFO(
        "sfx.control",
        "-SfxDispatcher(" << this << (bPush ? ")::Push(" : ")::Pop(")
            << (rShell.GetInterface()
                ? rShell.GetInterface()->GetClassName() : SAL_STREAM(&rShell))
            << (bDelete ? ") with delete" : ")")
            << (bUntil ? " (up to)" : ""));

    // same shell as on top of the to-do stack?
    if(pImp->aToDoStack.size() && pImp->aToDoStack.front().pCluster == &rShell)
    {
        // cancel inverse actions
        if ( pImp->aToDoStack.front().bPush != bPush )
            pImp->aToDoStack.pop_front();
        else
        {
            DBG_ASSERT( bPush, "SfxInterface pushed more than once" );
            DBG_ASSERT( !bPush, "SfxInterface popped more than once" );
        }
    }
    else
    {
        // Remember ::com::sun::star::chaos::Action
        pImp->aToDoStack.push_front( SfxToDo_Impl(bPush, bDelete, bUntil, rShell) );
        if ( bFlushed )
        {
            OSL_TRACE("Unflushed dispatcher!");
            bFlushed = sal_False;
            pImp->bUpdated = sal_False;

            // Put bindings to sleep
            SfxBindings* pBindings = GetBindings();
            if ( pBindings )
                pBindings->DENTERREGISTRATIONS();
        }
    }

    if(!pSfxApp->IsDowning() && !pImp->aToDoStack.empty())
    {
        // No immediate update is requested
        pImp->aTimer.SetTimeout(SFX_FLUSH_TIMEOUT);
        pImp->aTimer.SetTimeoutHdl( LINK(this, SfxDispatcher, EventHdl_Impl ) );
        pImp->aTimer.Start();
    }
    else
    {
        // but to do nothing
        pImp->aTimer.Stop();

        // Bindings may wake up again
        if(pImp->aToDoStack.empty())
        {
            SfxBindings* pBindings = GetBindings();
            if ( pBindings )
                pBindings->DLEAVEREGISTRATIONS();
        }
    }
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxDispatcher, EventHdl_Impl, void *, pvoid )

/*  [Description]

    This handler is called after <SfxDispatcher::Invalidate()> or after
    changes on the stack (<SfxDispatcher::Push()> and <SfxDispatcher::Pop())

    It flushes the Stack, if it is dirty, thus it actually excecutes the
    pending Push and Pop commands.
*/

{
    (void)pvoid; // unused

    Flush();
    Update_Impl();
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->StartUpdate_Impl(sal_False);
    return 0;
}
IMPL_LINK_INLINE_END( SfxDispatcher, EventHdl_Impl, void *, pvoid )

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::CheckVirtualStack( const SfxShell& rShell, sal_Bool bDeep )

/*  [Description]

    With this method it can be tested whether the <SfxShell> rShell is on the
    stack, when it was flushed. This way the SfxDispatcher is not actually
    flushed.

    This method is intended among other things to make assertions possible
    without the side effect of having to flush the SfxDispathcer.
*/

{
    SFX_STACK(SfxDispatcher::CheckVirtualStack);

    SfxShellStack_Impl aStack( pImp->aStack );
    for(std::deque<SfxToDo_Impl>::reverse_iterator i = pImp->aToDoStack.rbegin(); i != pImp->aToDoStack.rend(); ++i)
    {
        if(i->bPush)
            aStack.push_back(i->pCluster);
        else
        {
            SfxShell* pPopped(NULL);
            do
            {
                DBG_ASSERT( !aStack.empty(), "popping from empty stack" );
                pPopped = aStack.back();
                aStack.pop_back();
            }
            while(i->bUntil && pPopped != i->pCluster);
            DBG_ASSERT(pPopped == i->pCluster, "popping unpushed SfxInterface");
        }
    }

    sal_Bool bReturn;
    if ( bDeep )
        bReturn = std::find(aStack.begin(), aStack.end(), &rShell) != aStack.end();
    else
        bReturn = aStack.back() == &rShell;
    return bReturn;
}

//--------------------------------------------------------------------
sal_uInt16 SfxDispatcher::GetShellLevel( const SfxShell& rShell )

/*  [Description]

    Determines the position of a given SfxShell in the stack of the dispatcher.
    If possible this is flushed before.

    [Return value]

    sal_uInt16                  == USRT_MAX
                                The SfxShell is not on this SfxDispatcher.

                                < USHRT_MAX
                                Position of the SfxShell on the Dispatcher
                                from the top count stating with 0.
*/

{
    SFX_STACK(SfxDispatcher::GetShellLevel);
    Flush();

    for ( sal_uInt16 n = 0; n < pImp->aStack.size(); ++n )
        if ( *( pImp->aStack.rbegin() + n ) == &rShell )
            return n;
    if ( pImp->pParent )
    {
        sal_uInt16 nRet = pImp->pParent->GetShellLevel(rShell);
        if ( nRet == USHRT_MAX )
            return nRet;
        return  nRet + pImp->aStack.size();
    }

    return USHRT_MAX;
}

//--------------------------------------------------------------------
SfxShell *SfxDispatcher::GetShell(sal_uInt16 nIdx) const

/*  [Description]

    Returns a pointer to the <SfxShell> which is at the position nIdx
    (from the top, last pushed is 0) on the stack.

    Thus the SfxDispatcher is not flushed.

    Is the stack not deep enough a NULL-Pointer is returned.
*/

{
    sal_uInt16 nShellCount = pImp->aStack.size();
    if ( nIdx < nShellCount )
        return *(pImp->aStack.rbegin() + nIdx);
    else if ( pImp->pParent )
        return pImp->pParent->GetShell( nIdx - nShellCount );
    return 0;
}

//--------------------------------------------------------------------
SfxBindings* SfxDispatcher::GetBindings() const

/*  [Description]

    This method returns a pointer to the <SfxBinding> Instance on which the
    SfxDispatcher is curretly bound. A SfxDispatcher is only bound to
    the SfxBindings when it is <UI-aktiv>. If it is not UI-active,
    a NULL-pointer is returned.

    The returned pointer is only valid in the immediate context of the method
    call.
*/

{
    if ( pImp->pFrame )
        return &pImp->pFrame->GetBindings();
    else
        return NULL;
}

//--------------------------------------------------------------------
SfxViewFrame* SfxDispatcher::GetFrame() const

/*  [Description]

    Returns a pointer to the <SfxViewFrame> instance, which belongs to
    this SfxDispatcher. If it is about the application dispatcher,
    a NULL-pointer is returned.
*/

{
    return pImp->pFrame;
}

//--------------------------------------------------------------------
void SfxDispatcher::DoActivate_Impl( sal_Bool bMDI, SfxViewFrame* /* pOld */ )

/*  [Description]

    This method controls the activation of a dispatcher.

    Since the application dispatcher is always active, either as a sub
    dispatcher of the <SfxViewFrame> dispatcher or as itself, it is never
    activated as a whole, instead only its individual <SfxShell>s at
    <SfxDispatcher::Push(SfxShell&)>.

    When activating a SfxDispatcher all of the SfxShells located on its stack
    are called with the handler <SfxShell::Activate(sal_Bool)>, starting with
    the lowest.
*/

{
    SFX_STACK(SfxDispatcher::DoActivate);
    if ( bMDI )
    {
#ifdef DBG_UTIL
        OStringBuffer sTemp(
            RTL_CONSTASCII_STRINGPARAM("Activate Dispatcher "));
        sTemp.append(reinterpret_cast<sal_Int64>(this));
        OSL_TRACE("%s", sTemp.getStr());
        DBG_ASSERT( !pImp->bActive, "Activation error" );
#endif
        pImp->bActive = sal_True;
        pImp->bUpdated = sal_False;
        SfxBindings* pBindings = GetBindings();
        if ( pBindings )
        {
            pBindings->SetDispatcher(this);
            pBindings->SetActiveFrame( pImp->pFrame->GetFrame().GetFrameInterface() );
        }
    }
    else
    {
#ifdef DBG_UTIL
        OStringBuffer sTemp(
            RTL_CONSTASCII_STRINGPARAM("Non-MDI-Activate Dispatcher"));
        sTemp.append(reinterpret_cast<sal_Int64>(this));
        OSL_TRACE("%s", sTemp.getStr());
#endif
    }

    if ( IsAppDispatcher() )
        return;

    for ( int i = int(pImp->aStack.size()) - 1; i >= 0; --i )
        (*(pImp->aStack.rbegin() + i ))->DoActivate_Impl(pImp->pFrame, bMDI);

    if ( bMDI && pImp->pFrame )
    {
        SfxBindings *pBind = GetBindings();
        while ( pBind )
        {
            pBind->HidePopupCtrls_Impl( sal_False );
            pBind = pBind->GetSubBindings_Impl();
        }

        pImp->pFrame->GetFrame().GetWorkWindow_Impl()->HidePopups_Impl( sal_False, sal_False, 1 );
    }

    if(!pImp->aToDoStack.empty())
    {
        // No immediate update is requested
        pImp->aTimer.SetTimeout(SFX_FLUSH_TIMEOUT);
        pImp->aTimer.SetTimeoutHdl( LINK(this, SfxDispatcher, EventHdl_Impl ) );
        pImp->aTimer.Start();
    }
}

void SfxDispatcher::DoParentActivate_Impl()
{
    for ( int i = int(pImp->aStack.size()) - 1; i >= 0; --i )
        (*(pImp->aStack.rbegin() + i ))->ParentActivate();
}

//--------------------------------------------------------------------
void SfxDispatcher::DoDeactivate_Impl( sal_Bool bMDI, SfxViewFrame* pNew )

/*  [Description]

    This method controls the deactivation of a dispatcher.

    Since the application dispatcher is always active, either as a sub
    dispatcher of the <SfxViewFrame> dispatcher or as itself, it is never
    deactivated as a whole, instead only its individual <SfxShell>s at
    <SfxDispatcher::Pop(SfxShell&)>.

    When deactivating a SfxDispatcher all of the SfxShells located on its stack
    are called with the handler <SfxShell::Deactivate(sal_Bool)>, starting with
    the lowest.
*/

{
    SFX_STACK(SfxDispatcher::DoDeactivate);

    SfxApplication *pSfxApp = SFX_APP();

    if ( bMDI )
    {
        OSL_TRACE(OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Deactivate Dispatcher")).append(reinterpret_cast<sal_Int64>(this)).getStr());
        DBG_ASSERT( pImp->bActive, "Deactivate error" );
        pImp->bActive = sal_False;

        if ( pImp->pFrame && !(pImp->pFrame->GetObjectShell()->IsInPlaceActive() ) )
        {
            SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
            if ( pWorkWin )
            {
                for (size_t n=0; n<pImp->aChildWins.size();)
                {
                    SfxChildWindow *pWin = pWorkWin->GetChildWindow_Impl( (sal_uInt16) ( pImp->aChildWins[n] & 0xFFFF ) );
                    if (!pWin || (pWin && pWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT))
                        pImp->aChildWins.erase(pImp->aChildWins.begin()+n);
                    else
                        n++;
                }
            }
        }
    }
    else {
        OSL_TRACE(OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Non-MDI-DeActivate Dispatcher")).append(reinterpret_cast<sal_Int64>(this)).getStr());
    }

    if ( IsAppDispatcher() && !pSfxApp->IsDowning() )
        return;

    for ( sal_uInt16 i = 0; i < pImp->aStack.size(); ++i )
        (*(pImp->aStack.rbegin() + i))->DoDeactivate_Impl(pImp->pFrame, bMDI);

    sal_Bool bHidePopups = bMDI && pImp->pFrame;
    if ( pNew && pImp->pFrame )
    {
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xOldFrame(
            pNew->GetFrame().GetFrameInterface()->getCreator(), com::sun::star::uno::UNO_QUERY );

        com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xMyFrame(
            GetFrame()->GetFrame().GetFrameInterface(), com::sun::star::uno::UNO_QUERY );

        if ( xOldFrame == xMyFrame )
            bHidePopups = sal_False;
    }

    if ( bHidePopups )
    {
        SfxBindings *pBind = GetBindings();
        while ( pBind )
        {
            pBind->HidePopupCtrls_Impl( sal_True );
            pBind = pBind->GetSubBindings_Impl();
        }

        pImp->pFrame->GetFrame().GetWorkWindow_Impl()->HidePopups_Impl( sal_True, sal_False, 1 );
    }

    Flush();
}

void SfxDispatcher::DoParentDeactivate_Impl()
{
    for ( int i = int(pImp->aStack.size()) - 1; i >= 0; --i )
        (*(pImp->aStack.rbegin() + i))->ParentDeactivate();
}

//--------------------------------------------------------------------
int SfxDispatcher::GetShellAndSlot_Impl
(
    sal_uInt16      nSlot,    // the searchable Slot-Id
    SfxShell**      ppShell,  // the SfxShell, which are currently handled
                              // the nSlot
    const SfxSlot** ppSlot,   // the SfxSlot, which are currently handled
                              // the nSlot
    sal_Bool        bOwnShellsOnly,
    sal_Bool        bModal,   // ModalMode
    sal_Bool        bRealSlot
)

/*  [Description]

    This method searches in SfxDispatcher after <SfxShell> , from the Slot Id
    nSlot currently being handled. For this, the dispatcher is first flushed.

    [Return value]

    int              sal_True
                     The SfxShell was found, ppShell and ppSlot are valid.

                     sal_False
                     The SfxShell was not found, ppShell and ppSlot are invalid.
*/

{
    SFX_STACK(SfxDispatcher::GetShellAndSlot_Impl);

    Flush();
    SfxSlotServer aSvr;
    if ( _FindServer(nSlot, aSvr, bModal) )
    {
        if ( bOwnShellsOnly && aSvr.GetShellLevel() >= pImp->aStack.size() )
            return sal_False;

        *ppShell = GetShell(aSvr.GetShellLevel());
        *ppSlot = aSvr.GetSlot();
        if ( 0 == (*ppSlot)->GetExecFnc() && bRealSlot )
            *ppSlot = (*ppShell)->GetInterface()->GetRealSlot(*ppSlot);
        // Check only real slots as enum slots don't have an execute function!
        if ( bRealSlot && ((0 == *ppSlot) || (0 == (*ppSlot)->GetExecFnc()) ))
            return sal_False;

        return sal_True;
    }

    return sal_False;
}

//--------------------------------------------------------------------
void SfxDispatcher::_Execute
(
    SfxShell&       rShell,    // to the calling <SfxShell>
    const SfxSlot&  rSlot,     // to the calling <SfxSlot>
    SfxRequest&     rReq,      // function to be performed
                               // (Id and optional parameters)
    SfxCallMode     eCallMode  // Synchronously, asynchronously or as shown in
                               // the slot
)

/*  [Description]

    This method performs a request for a cached <Slot-Server>.
*/

{
    DBG_ASSERT( !pImp->bFlushing, "recursive call to dispatcher" );
    DBG_ASSERT( pImp->aToDoStack.empty(), "unprepared InPlace _Execute" );

    if ( IsLocked( rSlot.GetSlotId() ) )
        return;

    if ( (eCallMode & SFX_CALLMODE_ASYNCHRON) ||
         ( !(eCallMode & SFX_CALLMODE_SYNCHRON) &&
           rSlot.IsMode(SFX_SLOT_ASYNCHRON) ) )
    {
        SfxDispatcher *pDispat = this;
        while ( pDispat )
        {
            sal_uInt16 nShellCount = pDispat->pImp->aStack.size();
            for ( sal_uInt16 n=0; n<nShellCount; n++ )
            {
                if ( &rShell == *(pDispat->pImp->aStack.rbegin() + n) )
                {
                    if ( eCallMode & SFX_CALLMODE_RECORD )
                        rReq.AllowRecording( sal_True );
                    pDispat->pImp->xPoster->Post(new SfxRequest(rReq));
                    return;
                }
            }

            pDispat = pDispat->pImp->pParent;
        }
    }
    else
        Call_Impl( rShell, rSlot, rReq, SFX_CALLMODE_RECORD==(eCallMode&SFX_CALLMODE_RECORD) );
}

//--------------------------------------------------------------------
void MappedPut_Impl( SfxAllItemSet &rSet, const SfxPoolItem &rItem )

/*  [Description]

    Helper function to put from rItem below the Which-ID in the pool of the
    Item Sets rSet.
*/

{
    // Put with mapped Which-Id if possible
    const SfxItemPool *pPool = rSet.GetPool();
    sal_uInt16 nWhich = rItem.Which();
    if ( pPool->IsSlot(nWhich) )
        nWhich = pPool->GetWhich(nWhich);
    rSet.Put( rItem, nWhich );
}

//--------------------------------------------------------------------

const SfxSlot* SfxDispatcher::GetSlot( const OUString& rCommand )
{
    // Count the number of Shells on the linked Dispatcher
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.size();
    if ( pImp->pParent )
    {
        SfxDispatcher *pParent = pImp->pParent;
        while ( pParent )
        {
            nTotCount = nTotCount + pParent->pImp->aStack.size();
            pParent = pParent->pImp->pParent;
        }
    }

    const SfxSlot *pSlot=NULL;
    sal_uInt16 nFirstShell = 0;
    for ( sal_uInt16 i = nFirstShell; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        pSlot = pIFace->GetSlot( rCommand );
        if ( pSlot )
            return pSlot;
    }

    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem*  SfxDispatcher::Execute(
    sal_uInt16 nSlot,
    SfxCallMode nCall,
    SfxItemSet* pArgs,
    SfxItemSet* pInternalArgs,
    sal_uInt16 nModi)
{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(nCall&SFX_CALLMODE_MODAL) ) )
    {
        SfxAllItemSet aSet( pShell->GetPool() );
        if ( pArgs )
        {
            SfxItemIter aIter(*pArgs);
            for ( const SfxPoolItem *pArg = aIter.FirstItem();
                pArg;
                pArg = aIter.NextItem() )
                MappedPut_Impl( aSet, *pArg );
        }
        SfxRequest aReq( nSlot, nCall, aSet );
        if (pInternalArgs)
            aReq.SetInternalArgs_Impl( *pInternalArgs );
        aReq.SetModifier( nModi );

        _Execute( *pShell, *pSlot, aReq, nCall );
        return aReq.GetReturnValue();
    }
    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute
(
    sal_uInt16 nSlot,                 // the Id of the executing function
    SfxCallMode eCall,                // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON
                                      // or ..._SLOT
    const SfxPoolItem **pArgs,        // Zero teminated C-Array of Parameters
    sal_uInt16 nModi,
    const SfxPoolItem **pInternalArgs // Zero terminated C-Array of Parameters
)

/*  [Description]

    Method to excecute a <SfxSlot>s over the Slot-Id.

    [Return value]

    const SfxPoolItem*      Pointer to the SfxPoolItem valid to the next run
                            though the Message-Loop, which contains the return
                            value.

                            Or a NULL-Pointer, when the function was not
                            executed (for example canceled by the user).
*/

{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
        SfxRequest* pReq;
        if ( pArgs && *pArgs )
        {
            SfxAllItemSet aSet( pShell->GetPool() );
            for ( const SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                MappedPut_Impl( aSet, **pArg );
            pReq = new SfxRequest( nSlot, eCall, aSet );
        }
        else
            pReq =  new SfxRequest( nSlot, eCall, pShell->GetPool() );
        pReq->SetModifier( nModi );
        if( pInternalArgs && *pInternalArgs)
        {
            SfxAllItemSet aSet( SFX_APP()->GetPool() );
            for ( const SfxPoolItem **pArg = pInternalArgs; *pArg; ++pArg )
                aSet.Put( **pArg );
            pReq->SetInternalArgs_Impl( aSet );
        }
        _Execute( *pShell, *pSlot, *pReq, eCall );
        const SfxPoolItem* pRet = pReq->GetReturnValue();
        delete pReq; return pRet;
    }
    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute
(
    sal_uInt16 nSlot,        // the Id of the executing function
    SfxCallMode eCall,       // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON or ..._SLOT
    const SfxItemSet &rArgs  // <SfxItemSet> with the parameters
)

/*  [Description]

    Method to excecute a <SfxSlot>s over the Slot-Id.

    [Return value]

    const SfxPoolItem*      Pointer to the SfxPoolItem valid to the next run
                            though the Message-Loop, which contains the return
                            value.

                            Or a NULL-Pointer, when the function was not
                            executed (for example canceled by the user).
*/

{
    return Execute( nSlot, eCall, 0, rArgs );
}

//--------------------------------------------------------------------
const SfxPoolItem*  SfxDispatcher::Execute
(
    sal_uInt16 nSlot,
    SfxCallMode eCall,
    sal_uInt16 nModi,
    const SfxItemSet &rArgs
)
{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
        SfxAllItemSet aSet( pShell->GetPool() );
        SfxItemIter aIter(rArgs);
        for ( const SfxPoolItem *pArg = aIter.FirstItem();
              pArg;
              pArg = aIter.NextItem() )
            MappedPut_Impl( aSet, *pArg );
        SfxRequest aReq( nSlot, eCall, aSet );
        aReq.SetModifier( nModi );
        _Execute( *pShell, *pSlot, aReq, eCall );
        return aReq.GetReturnValue();
    }
    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute
(
    sal_uInt16          nSlot,  // the Id of the executing function
    SfxCallMode         eCall,  // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON or
                                // ..._SLOT
    const SfxPoolItem*  pArg1,  // First parameter
    ...                         // Zero terminated list of parameters
)

/*  [Description]

    Method to excecute a <SfxSlot>s over the Slot-Id.

    [Note]

    The parameters are copied, can therefore be passed on as the address
    of stack objects.

    [Return value]

    const SfxPoolItem*      Pointer to the SfxPoolItem valid to the next run
                            though the Message-Loop, which contains the return
                            value.

                            Or a NULL-Pointer, when the function was not
                            executed (for example canceled by the user).

    [Example]

    pDispatcher->Execute( SID_OPENDOCUMENT, SFX_CALLMODE_SYNCHRON,
        &SfxStringItem( SID_FILE_NAME, "\\tmp\\temp.sdd" ),
        &SfxStringItem( SID_FILTER_NAME, "StarDraw Presentation" ),
        &SfxBoolItem( SID_DOC_READONLY, sal_False ),
        0L );
*/

{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
       SfxAllItemSet aSet( pShell->GetPool() );

       va_list pVarArgs;
       va_start( pVarArgs, pArg1 );
       for ( const SfxPoolItem *pArg = pArg1;
             pArg;
             pArg = va_arg( pVarArgs, const SfxPoolItem* ) )
           MappedPut_Impl( aSet, *pArg );
       va_end(pVarArgs);

       SfxRequest aReq( nSlot, eCall, aSet );
       _Execute( *pShell, *pSlot, aReq, eCall );
       return aReq.GetReturnValue();
    }
    return 0;
}

//--------------------------------------------------------------------

IMPL_LINK( SfxDispatcher, PostMsgHandler, SfxRequest*, pReq )

/*  [Description]

    Helper method to receive the asynchronously executed <SfxRequest>s.
*/

{
    DBG_ASSERT( !pImp->bFlushing, "recursive call to dispatcher" );
    SFX_STACK(SfxDispatcher::PostMsgHandler);

    // Has also the Pool not yet died?
    if ( !pReq->IsCancelled() )
    {
        if ( !IsLocked(pReq->GetSlot()) )
        {
            Flush();
            SfxSlotServer aSvr;
            if ( _FindServer(pReq->GetSlot(), aSvr, sal_True ) ) // HACK(x), whatever that was supposed to mean
            {
                const SfxSlot *pSlot = aSvr.GetSlot();
                SfxShell *pSh = GetShell(aSvr.GetShellLevel());

                DBG( SfxApplication *pSfxApp = SFX_APP() );
                DBG( pSfxApp->EnterAsynchronCall_Impl() );

                // When the pSlot is a "Pseudoslot" for macros or Verbs, it can
                // be destroyed in the Call_Impl, thus do not use it anymore!
                pReq->SetSynchronCall( sal_False );
                Call_Impl( *pSh, *pSlot, *pReq, pReq->AllowsRecording() ); //! why bRecord?
                DBG( pSfxApp->LeaveAsynchronCall_Impl() );
            }
        }
        else
        {
            if ( pImp->bLocked )
                pImp->aReqArr.push_back(new SfxRequest(*pReq));
            else
                pImp->xPoster->Post(new SfxRequest(*pReq));
        }
    }

    delete pReq;
    return 0;
}
//--------------------------------------------------------------------
void SfxDispatcher::SetMenu_Impl()
{
    if ( pImp->pFrame )
    {
        SfxViewFrame* pTop = pImp->pFrame->GetTopViewFrame();
        if ( pTop && pTop->GetBindings().GetDispatcher() == this )
        {
            SfxFrame& rFrame = pTop->GetFrame();
            if ( rFrame.IsMenuBarOn_Impl() )
            {
                com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > xPropSet( rFrame.GetFrameInterface(), com::sun::star::uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                    com::sun::star::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                    {
                        OUString aMenuBarURL( "private:resource/menubar/menubar" );
                        if ( !xLayoutManager->isElementVisible( aMenuBarURL ) )
                            xLayoutManager->createElement( aMenuBarURL );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------
void SfxDispatcher::Update_Impl( sal_Bool bForce )
{
    SFX_STACK(SfxDispatcher::Update_Impl);

    Flush();

    if ( !pImp->pFrame )
        return;

    SFX_APP();  // -Wall is this required???
    SfxDispatcher *pDisp = this;
    sal_Bool bUpdate = bForce;
    while ( pDisp && pDisp->pImp->pFrame )
    {
        SfxWorkWindow *pWork = pDisp->pImp->pFrame->GetFrame().GetWorkWindow_Impl();
        SfxDispatcher *pAct = pWork->GetBindings().GetDispatcher_Impl();
        if ( pAct == pDisp || pAct == this )
        {
            if ( !bUpdate )
                bUpdate = !pDisp->pImp->bUpdated;
            pDisp->pImp->bUpdated = sal_True;
        }
        else
            break;

        pDisp = pDisp->pImp->pParent;
    }

    if ( !bUpdate || pImp->pFrame->GetFrame().IsClosing_Impl() )
        return;

    SfxViewFrame* pTop = pImp->pFrame ? pImp->pFrame->GetTopViewFrame() : NULL;
    sal_Bool bUIActive = pTop && pTop->GetBindings().GetDispatcher() == this;

    if ( !bUIActive && pTop && GetBindings() == &pTop->GetBindings() )
        // keep own tools internally for collecting
        GetBindings()->GetDispatcher()->pImp->bUpdated = sal_False;

    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->DENTERREGISTRATIONS();

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame = pBindings->GetActiveFrame();
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, com::sun::star::uno::UNO_QUERY );
    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            com::sun::star::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
        }
        catch (const com::sun::star::uno::Exception&)
        {
        }
    }

    if ( xLayoutManager.is() )
        xLayoutManager->lock();

    sal_Bool bIsIPActive = pImp->pFrame && pImp->pFrame->GetObjectShell()->IsInPlaceActive();
    SfxInPlaceClient *pClient = pImp->pFrame ? pImp->pFrame->GetViewShell()->GetUIActiveClient() : NULL;
    if ( bUIActive && /* !bIsIPActive && */ ( !pClient || !pClient->IsObjectUIActive() ) )
        SetMenu_Impl();

    SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
    SfxWorkWindow *pTaskWin = pImp->pFrame->GetTopFrame().GetWorkWindow_Impl();
    pTaskWin->ResetStatusBar_Impl();

    SfxDispatcher *pDispat = this;
    while ( pDispat )
    {
        SfxWorkWindow *pWork = pDispat->pImp->pFrame->GetFrame().GetWorkWindow_Impl();
        SfxDispatcher *pAct = pWork->GetBindings().GetDispatcher_Impl();
        if ( pAct == pDispat || pAct == this )
        {
            pWork->ResetObjectBars_Impl();
            pWork->ResetChildWindows_Impl();
        }

        pDispat = pDispat->pImp->pParent;
    }

    sal_Bool bIsActive = sal_False;
    SfxDispatcher *pActDispat = pWorkWin->GetBindings().GetDispatcher_Impl();
    pDispat = this;
    while ( pActDispat && !bIsActive )
    {
        if ( pDispat == pActDispat )
            bIsActive = sal_True;
        pActDispat = pActDispat->pImp->pParent;
    }

    _Update_Impl( bUIActive, !bIsIPActive, bIsIPActive, pTaskWin );
    if ( bUIActive || bIsActive )
        pWorkWin->UpdateObjectBars_Impl();

    if ( pBindings )
        pBindings->DLEAVEREGISTRATIONS();

    if ( xLayoutManager.is() )
        xLayoutManager->unlock();

    return;
}

void SfxDispatcher::_Update_Impl( sal_Bool bUIActive, sal_Bool bIsMDIApp, sal_Bool bIsIPOwner, SfxWorkWindow *pTaskWin )
{
    SFX_APP();
    SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
    sal_Bool bIsActive = sal_False;
    sal_Bool bIsTaskActive = sal_False;
    SfxDispatcher *pActDispat = pWorkWin->GetBindings().GetDispatcher_Impl();
    SfxDispatcher *pDispat = this;
    while ( pActDispat && !bIsActive )
    {
        if ( pDispat == pActDispat )
            bIsActive = sal_True;
        pActDispat = pActDispat->pImp->pParent;
    }

    if ( pImp->pParent && !pImp->bQuiet /* && bUIActive */ )
        pImp->pParent->_Update_Impl( bUIActive, bIsMDIApp, bIsIPOwner, pTaskWin );

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].nResId = 0;
    pImp->aChildWins.clear();

    // bQuiet : own shells aren't considered for UI and SlotServer
    // bNoUI: own Shells aren't considered fors UI
    if ( pImp->bQuiet || pImp->bNoUI || (pImp->pFrame && pImp->pFrame->GetObjectShell()->IsPreview()) )
        return;

    sal_uInt32 nStatBarId=0;
    SfxShell *pStatusBarShell = NULL;

    SfxSlotPool* pSlotPool = &SfxSlotPool::GetSlotPool( GetFrame() );
    sal_uInt16 nTotCount = pImp->aStack.size();
    for ( sal_uInt16 nShell = nTotCount; nShell > 0; --nShell )
    {
        SfxShell *pShell = GetShell( nShell-1 );
        SfxInterface *pIFace = pShell->GetInterface();

        // don't consider shells if "Hidden" oder "Quiet"
        sal_Bool bReadOnlyShell = IsReadOnlyShell_Impl( nShell-1 );
        sal_uInt16 nNo;
        for ( nNo = 0; pIFace && nNo<pIFace->GetObjectBarCount(); ++nNo )
        {
            sal_uInt16 nPos = pIFace->GetObjectBarPos(nNo);
            if ( bReadOnlyShell && !( nPos & SFX_VISIBILITY_READONLYDOC ) )
                continue;

            // check whether toolbar needs activation of a special feature
            sal_uInt32 nFeature = pIFace->GetObjectBarFeature(nNo);
            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                continue;

            // check for toolboxes that are exclusively for a viewer
            if ( pImp->pFrame)
            {
                sal_Bool bViewerTbx = SFX_VISIBILITY_VIEWER == ( nPos & SFX_VISIBILITY_VIEWER );
                SfxObjectShell* pSh = pImp->pFrame->GetObjectShell();
                SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_VIEWONLY, sal_False );
                sal_Bool bIsViewer = pItem && pItem->GetValue();
                if ( bIsViewer != bViewerTbx )
                    continue;
            }

            // always register toolbars, allows to switch them on
            sal_Bool bVisible = pIFace->IsObjectBarVisible(nNo);
            if ( !bVisible )
                nPos &= SFX_POSITION_MASK;

            SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos & SFX_POSITION_MASK];
            rBar.nMode = nPos;
            rBar.nResId = pIFace->GetObjectBarResId(nNo).GetId();
            const OUString *pName = pIFace->GetObjectBarName(nNo);
            if ( pName )
                rBar.aName = *pName;
            else
                rBar.aName = "";
            rBar.pIFace = pIFace;

            if ( bUIActive || bIsActive )
            {
                pWorkWin->SetObjectBar_Impl(
                    nPos, rBar.nResId, rBar.pIFace, &rBar.aName );
            }

            if ( !bVisible )
                rBar.nResId = 0;
        }

        for ( nNo=0; pIFace && nNo<pIFace->GetChildWindowCount(); nNo++ )
        {
            sal_uInt32 nId = pIFace->GetChildWindowId(nNo);
            const SfxSlot *pSlot = pSlotPool->GetSlot( (sal_uInt16) nId );
            SAL_WARN_IF( !pSlot, "sfx.control", "Childwindow slot missing: " << nId );
            if ( bReadOnlyShell )
            {
                // only show ChildWindows if their slot is allowed for readonly documents
                if ( pSlot && !pSlot->IsMode( SFX_SLOT_READONLYDOC ) )
                    continue;
            }

            sal_uInt32 nFeature = pIFace->GetChildWindowFeature(nNo);
            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                continue;

            // slot decides whether a ChildWindow is shown when document is OLE server or OLE client
            sal_uInt16 nMode = SFX_VISIBILITY_STANDARD;
            if( pSlot )
            {
                if ( pSlot->IsMode(SFX_SLOT_CONTAINER) )
                {
                    if ( pWorkWin->IsVisible_Impl( SFX_VISIBILITY_CLIENT ) )
                        nMode |= SFX_VISIBILITY_CLIENT;
                }
                else
                {
                    if ( pWorkWin->IsVisible_Impl( SFX_VISIBILITY_SERVER ) )
                        nMode |= SFX_VISIBILITY_SERVER;
                }
            }

            if ( bUIActive || bIsActive )
                pWorkWin->SetChildWindowVisible_Impl( nId, sal_True, nMode );
            if ( bUIActive || bIsActive || !pWorkWin->IsFloating( (sal_uInt16) ( nId & 0xFFFF ) ) )
                pImp->aChildWins.push_back( nId );
        }

        if ( bIsMDIApp || bIsIPOwner )
        {
            sal_uInt32 nId = pIFace->GetStatusBarResId().GetId();
            if ( nId )
            {
                nStatBarId = nId;
                pStatusBarShell =  pShell;
            }
        }
    }

    for ( sal_uInt16 nPos=0; nPos<SFX_OBJECTBAR_MAX; nPos++ )
    {
        SfxObjectBars_Impl& rFixed = pImp->aFixedObjBars[nPos];
        if ( rFixed.nResId )
        {
            SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos];
            rBar = rFixed;
            pWorkWin->SetObjectBar_Impl( rFixed.nMode,
                rFixed.nResId, rFixed.pIFace, &rFixed.aName );
        }
    }

    if ( pTaskWin && ( bIsMDIApp || bIsIPOwner ) )
    {
        SfxDispatcher *pActDispatcher = pTaskWin->GetBindings().GetDispatcher_Impl();
        SfxDispatcher *pDispatcher = this;
        while ( pActDispatcher && !bIsTaskActive )
        {
            if ( pDispatcher == pActDispatcher )
                bIsTaskActive = sal_True;
            pActDispatcher = pActDispatcher->pImp->pParent;
        }

        if ( bIsTaskActive && nStatBarId && pImp->pFrame )
        {
            // internal frames also may control statusbar
            SfxBindings& rBindings = pImp->pFrame->GetBindings();
            pImp->pFrame->GetFrame().GetWorkWindow_Impl()->SetStatusBar_Impl( nStatBarId, pStatusBarShell, rBindings );
        }
    }
}

//--------------------------------------------------------------------
void SfxDispatcher::FlushImpl()

/*  [Description]

    Helper method to execute the outstanding push and pop commands.
*/

{
    DBG_PROFSTART(SfxDispatcherFlush);
    SFX_STACK(SfxDispatcher::FlushImpl);

    OSL_TRACE("Flushing dispatcher!");

    pImp->aTimer.Stop();

    if ( pImp->pParent )
        pImp->pParent->Flush();

    pImp->bFlushing = !pImp->bFlushing;
    if ( !pImp->bFlushing )
    {
        pImp->bFlushing = sal_True;
        DBG_PROFSTOP(SfxDispatcherFlush);
        return;
    }

    SfxApplication *pSfxApp = SFX_APP();

    // Re-build the true stack in the first round
    std::deque<SfxToDo_Impl> aToDoCopy;
    sal_Bool bModify = sal_False;
    for(std::deque<SfxToDo_Impl>::reverse_iterator i = pImp->aToDoStack.rbegin(); i != pImp->aToDoStack.rend(); ++i)
    {
        bModify = sal_True;

        if(i->bPush)
        {
            // Actually push
            DBG_ASSERT( std::find(pImp->aStack.begin(), pImp->aStack.end(), i->pCluster) == pImp->aStack.end(),
                       "pushed SfxShell already on stack" );
            pImp->aStack.push_back(i->pCluster);
            i->pCluster->SetDisableFlags(pImp->nDisableFlags);

            // Mark the moved shell
            aToDoCopy.push_front(*i);
        }
        else
        {
            // Actually pop
            SfxShell* pPopped = 0;
            bool bFound = false;
            do
            {
                DBG_ASSERT( !pImp->aStack.empty(), "popping from empty stack" );
                pPopped = pImp->aStack.back();
                pImp->aStack.pop_back();
                pPopped->SetDisableFlags( 0 );
                bFound = (pPopped == i->pCluster);

                // Mark the moved Shell
                aToDoCopy.push_front(SfxToDo_Impl(sal_False, i->bDelete, sal_False, *pPopped));
            }
            while(i->bUntil && !bFound);
            DBG_ASSERT( bFound, "wrong SfxShell popped" );
        }
    }
    pImp->aToDoStack.clear();

    // Invalidate bindings, if possible
    if ( !pSfxApp->IsDowning() )
    {
        if ( bModify )
        {
            pImp->pCachedServ1 = 0;
            pImp->pCachedServ2 = 0;
        }

        InvalidateBindings_Impl( bModify );
    }

    pImp->bFlushing = sal_False;
    pImp->bUpdated = sal_False; // not only when bModify, if Doc/Template-Config
    bFlushed = sal_True;
    OSL_TRACE("Successfully flushed dispatcher!");

    // Activate the Shells and possible delete them in the 2nd round
    for(std::deque<SfxToDo_Impl>::reverse_iterator i = aToDoCopy.rbegin(); i != aToDoCopy.rend(); ++i)
    {
        if(i->bPush)
        {
            if ( pImp->bActive )
                i->pCluster->DoActivate_Impl(pImp->pFrame, sal_True);
        }
        else if ( pImp->bActive )
                i->pCluster->DoDeactivate_Impl(pImp->pFrame, sal_True);
    }

    for(std::deque<SfxToDo_Impl>::reverse_iterator i = aToDoCopy.rbegin(); i != aToDoCopy.rend(); ++i)
    {
        if(i->bDelete)
            delete i->pCluster;
    }
    sal_Bool bAwakeBindings = !aToDoCopy.empty();
    if( bAwakeBindings )
        aToDoCopy.clear();

    // If more changes have occurred on the stach when
    // Activate/Deactivate/Delete:
    if (!bFlushed)
        // If Push/Pop hs been called by someone, theb also EnterReg was called!
        FlushImpl();

    if( bAwakeBindings && GetBindings() )
        GetBindings()->DLEAVEREGISTRATIONS();
    DBG_PROFSTOP(SfxDispatcherFlush);

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aFixedObjBars[n].nResId = 0;

    SAL_INFO("sfx.control", "SfxDispatcher(" << this << ")::Flush() done");
}

//--------------------------------------------------------------------
void SfxDispatcher::SetSlotFilter
(
    // HACK(hier muss mal ein enum rein) ???
    sal_Bool           bEnable,  /* sal_True:
                                    only enable specified slots,
                                    disable all other

                                    sal_False:
                                    disable specified slots,
                                    first enable all other
                                 */
    sal_uInt16         nCount,   // Number of SIDs in the following Array
    const sal_uInt16*  pSIDs     // sorted Array of 'nCount' SIDs
)

/*  [Description]

    With this method a filter set, the target slots can be enabled or disabled.
    The passed array must be retained until the destructor or the next
    <SetSlotFilter()>, it is not deleted from the dispatcher, so it can thus be
    static.

    In read-only documents the quasi ReadOnlyDoc Flag of slots can be
    overturned by the use of 'bEnable == 2', so this will be displayed again.
    On the other slots it has no effect.

    [Example]

    Targeted disabling of Slots 1, 2 and 3:

        static sal_uInt16 const pSIDs[] = { 1, 2, 3 };
        pDisp->SetSlotFilter( sal_False, sizeof(pSIDs)/sizeof(sal_uInt16), pSIDs );

    only permit Slots 5, 6 and 7:

        static sal_uInt16 const pSIDs[] = { 5, 6, 7 };
        pDisp->SetSlotFilter( sal_True, sizeof(pSIDs)/sizeof(sal_uInt16), pSIDs );

    Turn-off Filter:

        pDisp->SetSlotFilter();
*/

{
#ifdef DBG_UTIL
    // Check Array
    for ( sal_uInt16 n = 1; n < nCount; ++n )
        DBG_ASSERT( pSIDs[n] > pSIDs[n-1], "SetSlotFilter: SIDs not sorted" );
#endif

    if ( pImp->pFilterSIDs )
        pImp->pFilterSIDs = 0;

    pImp->bFilterEnabling = bEnable;
    pImp->nFilterCount = nCount;
    pImp->pFilterSIDs = pSIDs;

    GetBindings()->InvalidateAll(sal_True);
}

//--------------------------------------------------------------------
extern "C"
#ifdef WNT
int _cdecl
#else
int
#endif

SfxCompareSIDs_Impl( const void* pSmaller, const void* pBigger )
{
    return ( (long) *((sal_uInt16*)pSmaller) ) - ( (long) *((sal_uInt16*)pBigger) );
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::IsSlotEnabledByFilter_Impl( sal_uInt16 nSID ) const

/*  [Description]

    Searches for 'nSID' in the Filter set by <SetSlotFilter()> and
    returns sal_True, if the SIDis allowed, or sal_False, if it is
    disabled by the Filter.

    [Return value]
    sal_Bool            0       =>      disabled
                        1       =>      enabled
                        2       =>      enabled even if ReadOnlyDoc
*/

{
    // no filter?
    if ( 0 == pImp->nFilterCount )
        // => all SIDs allowed
        return sal_True;

    // search
    sal_Bool bFound = 0 != bsearch( &nSID, pImp->pFilterSIDs, pImp->nFilterCount,
                                sizeof(sal_uInt16), SfxCompareSIDs_Impl );

    // even if ReadOnlyDoc
    if ( 2 == pImp->bFilterEnabling )
        return bFound ? 2 : 1;
    // Otherwise after Negative/Positive Filter
    return pImp->bFilterEnabling ? bFound : !bFound;
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::_TryIntercept_Impl
(
    sal_uInt16      nSlot,    // Slot-Id to search for
    SfxSlotServer&  rServer,  // <SfxSlotServer>-Instance to fill
    sal_Bool        bSelf
)
{
    // Maybe the parent is also belongs to a component
    SfxDispatcher *pParent = pImp->pParent;
    sal_uInt16 nLevels = pImp->aStack.size();
    while ( pParent && pParent->pImp->pFrame )
    {
        if ( pParent->pImp->pFrame->GetFrame().HasComponent() )
        {
            // Components may be intercepted
            if ( pParent->_TryIntercept_Impl( nSlot, rServer, sal_True ) )
            {
                // The own shells are added to the Shell Level
                rServer.SetShellLevel( rServer.GetShellLevel() + nLevels );
                return sal_True;
            }
            else
                // No further Interception
                break;
        }
        else
            nLevels = nLevels + pParent->pImp->aStack.size();

        pParent = pParent->pImp->pParent;
    }

    if ( bSelf )
    {
        // Query the ComponentViewShell
        Flush();
        SfxShell *pObjShell = GetShell(0);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot(nSlot);

        if ( pSlot )
        {
            rServer.SetSlot(pSlot);
            rServer.SetShellLevel(0);
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool SfxDispatcher::_FindServer
(
    sal_uInt16      nSlot,     // Slot-Id to search for
    SfxSlotServer&  rServer,   // <SfxSlotServer>-Instance to fill
    sal_Bool        bModal     // Dispite ModalMode
)

/*  [Description]

    This helper method searches for the <Slot-Server> which currently serves
    the nSlot. As the result, rServe is filled accordingly.

    If known the SfxInterface which is currently served by nSlot can be
    passed along.

    The SfxDispatcher is flushed while searching for nSlot.

    [Return value]


    sal_Bool            sal_True
                        The Slot was found, rServer is valid.

                        sal_False
                        The Slot is currently not served, rServer is invalid.
*/

{
    SFX_STACK(SfxDispatcher::_FindServer);

    // Dispatcher locked? (nevertheless let SID_HELP_PI through)
    if ( IsLocked(nSlot) )
    {
        pImp->bInvalidateOnUnlock = sal_True;
        return sal_False;
    }

    // Count the number of Shells in the linked dispatchers.
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.size();
    if ( pImp->pParent )
    {
        SfxDispatcher *pParent = pImp->pParent;
        while ( pParent )
        {
            nTotCount = nTotCount + pParent->pImp->aStack.size();
            pParent = pParent->pImp->pParent;
        }
    }

    // Verb-Slot?
    if (nSlot >= SID_VERB_START && nSlot <= SID_VERB_END)
    {
        for ( sal_uInt16 nShell = 0;; ++nShell )
        {
            SfxShell *pSh = GetShell(nShell);
            if ( pSh == NULL )
                return false;
            if ( pSh->ISA(SfxViewShell) )
            {
                const SfxSlot* pSlot = pSh->GetVerbSlot_Impl(nSlot);
                if ( pSlot )
                {
                    rServer.SetShellLevel(nShell);
                    rServer.SetSlot( pSlot );
                    return true;
                }
            }
        }
    }

    // SID check against set filter
    sal_uInt16 nSlotEnableMode=0;
    if ( pImp->pFrame )
    {
        nSlotEnableMode = IsSlotEnabledByFilter_Impl( nSlot );
        if ( 0 == nSlotEnableMode )
            return sal_False;
    }

    // In Quiet-Mode only Parent-Dispatcher
    if ( pImp->bQuiet )
    {
        if ( pImp->pParent )
        {
            sal_Bool bRet = pImp->pParent->_FindServer( nSlot, rServer, bModal );
            rServer.SetShellLevel
                ( rServer.GetShellLevel() + pImp->aStack.size() );
            return bRet;
        }
        else
            return sal_False;
    }

    sal_Bool bReadOnly = ( 2 != nSlotEnableMode && pImp->bReadOnly );

    // search through all the shells of the chained dispatchers
    // from top to bottom
    sal_uInt16 nFirstShell = pImp->bModal && !bModal ? pImp->aStack.size() : 0;
    for ( sal_uInt16 i = nFirstShell; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot(nSlot);

        if ( pSlot && pSlot->nDisableFlags && ( pSlot->nDisableFlags & pObjShell->GetDisableFlags() ) != 0 )
            return sal_False;

        if ( pSlot && !( pSlot->nFlags & SFX_SLOT_READONLYDOC ) && bReadOnly )
            return sal_False;

        if ( pSlot )
        {
            // Slot belongs to Container?
            bool bIsContainerSlot = pSlot->IsMode(SFX_SLOT_CONTAINER);
            bool bIsInPlace = pImp->pFrame && pImp->pFrame->GetObjectShell()->IsInPlaceActive();

            // Shell belongs to Server?
            // AppDispatcher or IPFrame-Dispatcher
            bool bIsServerShell = !pImp->pFrame || bIsInPlace;

            // Of course ShellServer-Slots are also executable even when it is
            // excecuted on a container dispatcher without a IPClient.
            if ( !bIsServerShell )
            {
                SfxViewShell *pViewSh = pImp->pFrame->GetViewShell();
                bIsServerShell = !pViewSh || !pViewSh->GetUIActiveClient();
            }

            // Shell belongs to Container?
            // AppDispatcher or no IPFrameDispatcher
            bool bIsContainerShell = !pImp->pFrame || !bIsInPlace;
            // Shell and Slot match
            if ( !( ( bIsContainerSlot && bIsContainerShell ) ||
                    ( !bIsContainerSlot && bIsServerShell ) ) )
                pSlot = 0;
        }

        if ( pSlot )
        {
            rServer.SetSlot(pSlot);
            rServer.SetShellLevel(i);
            return sal_True;
        }
    }

    return sal_False;
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::_FillState
(
    const SfxSlotServer&  rSvr,      // <Slot-Server> to query
    SfxItemSet&           rState,    // <SfxItemSet> to be filled
    const SfxSlot*        pRealSlot  // The actual Slot if possible
)

/*  [Description]

    Helper method to obtain the status of the <Slot-Server>s rSvr.
    The required slots IDs (partly converted to Which-IDs of the pool)
    must be present in rstate.

    The SfxDispatcher is flushed before the query.
*/

{
    SFX_STACK(SfxDispatcher::_FillState);

    DBG_PROFSTART(SfxDispatcherFillState);

    const SfxSlot *pSlot = rSvr.GetSlot();
    if ( pSlot && IsLocked( pSlot->GetSlotId() ) )
    {
        pImp->bInvalidateOnUnlock = sal_True;
        DBG_PROFSTOP(SfxDispatcherFillState);
        return sal_False;
    }

    if ( pSlot )
    {
        DBG_ASSERT(bFlushed, "Dispatcher not flushed after retrieving slot servers!");
        if ( !bFlushed )
            return sal_False;

        // Determine the object and call the Message of this object
        SfxShell *pSh = GetShell(rSvr.GetShellLevel());
        DBG_ASSERT(pSh, "ObjektShell not found");

        SfxStateFunc pFunc;

        if (pRealSlot)
            pFunc = pRealSlot->GetStateFnc();
        else
            pFunc = pSlot->GetStateFnc();

        pSh->CallState( pFunc, rState );
#ifdef DBG_UTIL
        // To examine the conformity of IDL (SlotMap) and current Items
        if ( DbgIsAssertWarning() && rState.Count() )
        {
            SfxInterface *pIF = pSh->GetInterface();
            SfxItemIter aIter( rState );
            for ( const SfxPoolItem *pItem = aIter.FirstItem();
                  pItem;
                  pItem = aIter.NextItem() )
            {
                if ( !IsInvalidItem(pItem) && !pItem->ISA(SfxVoidItem) )
                {
                    sal_uInt16 nSlotId = rState.GetPool()->GetSlotId(pItem->Which());
                    if ( !pItem->IsA(pIF->GetSlot(nSlotId)->GetType()->Type()) )
                    {
                        OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM(
                            "item-type unequal to IDL (=> no BASIC)"));
                        aMsg.append(RTL_CONSTASCII_STRINGPARAM("\nwith SID: "));
                        aMsg.append(static_cast<sal_Int32>(nSlotId));
                        aMsg.append(RTL_CONSTASCII_STRINGPARAM("\nin "));
                        aMsg.append(pIF->GetClassName());
                        DbgOut(aMsg.getStr(), DBG_OUT_ERROR, __FILE__, __LINE__);
                    }
                }
            }
        }
#endif

        DBG_PROFSTOP(SfxDispatcherFillState);
        return sal_True;
    }

    DBG_PROFSTOP(SfxDispatcherFillState);
    return sal_False;
}

SfxPopupMenuManager* SfxDispatcher::Popup( sal_uInt16 nConfigId,Window *pWin, const Point *pPos )
{
    SfxDispatcher &rDisp = *SFX_APP()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;
    SfxShell *pSh;

    if ( rDisp.pImp->bQuiet )
    {
        nConfigId = 0;
        nShLevel = rDisp.pImp->aStack.size();
    }

    Window *pWindow = pWin ? pWin : rDisp.pImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        const ResId& rResId = pSh->GetInterface()->GetPopupMenuResId();
        if ( ( nConfigId == 0 && rResId.GetId() ) || ( nConfigId != 0 && rResId.GetId() == nConfigId ) )
        {
                return SfxPopupMenuManager::Popup( rResId, rDisp.GetFrame(), pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
        }
    }
    return 0;
}


//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( sal_uInt16 nConfigId, Window *pWin, const Point *pPos )
{
    SfxDispatcher &rDisp = *SFX_APP()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;
    SfxShell *pSh;

    if ( rDisp.pImp->bQuiet )
    {
        nConfigId = 0;
        nShLevel = rDisp.pImp->aStack.size();
    }

    Window *pWindow = pWin ? pWin : rDisp.pImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        const ResId& rResId = pSh->GetInterface()->GetPopupMenuResId();
        if ( ( nConfigId == 0 && rResId.GetId() ) || ( nConfigId != 0 && rResId.GetId() == nConfigId ) )
        {
            SfxPopupMenuManager::ExecutePopup( rResId, rDisp.GetFrame(), pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
            return;
        }
    }
}

//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( const ResId &rId, Window *pWin, const Point *pPos )
{
    Window *pWindow = pWin ? pWin : pImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
    SfxPopupMenuManager::ExecutePopup( rId, GetFrame(), pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
}

//--------------------------------------------------------------------
void SfxDispatcher::Lock( sal_Bool bLock )

/*  [Description]

    With this method the SfxDispatcher can be locked and released. A locked
    SfxDispatcher does not perform <SfxRequest>s and does no longer provide
    status information. It behaves as if all the slots were disabled.
*/

{
    SfxBindings* pBindings = GetBindings();
    if ( !bLock && pImp->bLocked && pImp->bInvalidateOnUnlock )
    {
        if ( pBindings )
            pBindings->InvalidateAll(sal_True);
        pImp->bInvalidateOnUnlock = sal_False;
    }
    else if ( pBindings )
        pBindings->InvalidateAll(sal_False);
    pImp->bLocked = bLock;
    if ( !bLock )
    {
        for(size_t i = 0; i < pImp->aReqArr.size(); ++i)
            pImp->xPoster->Post(pImp->aReqArr[i]);
        pImp->aReqArr.clear();
    }
}

sal_uInt32 SfxDispatcher::GetObjectBarId( sal_uInt16 nPos ) const
{
    return pImp->aObjBars[nPos].nResId;
}

//-------------------------------------------------------------------------
void SfxDispatcher::HideUI( sal_Bool bHide )
{
    sal_Bool bWasHidden = pImp->bNoUI;
    pImp->bNoUI = bHide;
    if ( pImp->pFrame )
    {
        SfxViewFrame* pTop = pImp->pFrame->GetTopViewFrame();
        if ( pTop && pTop->GetBindings().GetDispatcher() == this )
        {
            SfxFrame& rFrame = pTop->GetFrame();
            if ( rFrame.IsMenuBarOn_Impl() )
            {
                com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > xPropSet( rFrame.GetFrameInterface(), com::sun::star::uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                    com::sun::star::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                        xLayoutManager->setVisible( !bHide );
                }
            }
        }
    }

    if ( bHide != bWasHidden )
        Update_Impl( sal_True );
}

void SfxDispatcher::SetReadOnly_Impl( sal_Bool bOn )
{
    pImp->bReadOnly = bOn;
}

sal_Bool SfxDispatcher::GetReadOnly_Impl() const
{
    return pImp->bReadOnly;
}

//-------------------------------------------------------------------------
void SfxDispatcher::SetQuietMode_Impl( sal_Bool bOn )

/*  [Description]

    With 'bOn' the Dispatcher is quasi dead and transfers everything to the
    Parent-Dispatcher.
*/

{
    pImp->bQuiet = bOn;
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->InvalidateAll(sal_True);
}

SfxItemState SfxDispatcher::QueryState( sal_uInt16 nSlot, const SfxPoolItem* &rpState )
{
    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False, sal_False ) )
    {
        rpState = pShell->GetSlotState(nSlot);
        if ( !rpState )
            return SFX_ITEM_DISABLED;
        else
            return SFX_ITEM_AVAILABLE;
    }

    return SFX_ITEM_DISABLED;
}

SfxItemState SfxDispatcher::QueryState( sal_uInt16 nSID, ::com::sun::star::uno::Any& rAny )
{
    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSID, &pShell, &pSlot, sal_False, sal_False ) )
    {
        const SfxPoolItem* pItem( 0 );

        pItem = pShell->GetSlotState( nSID );
        if ( !pItem )
            return SFX_ITEM_DISABLED;
        else
        {
            ::com::sun::star::uno::Any aState;
            if ( !pItem->ISA(SfxVoidItem) )
            {
                sal_uInt16 nSubId( 0 );
                SfxItemPool& rPool = pShell->GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( nSID );
                if ( rPool.GetMetric( nWhich ) == SFX_MAPUNIT_TWIP )
                    nSubId |= CONVERT_TWIPS;
                pItem->QueryValue( aState, (sal_uInt8)nSubId );
            }
            rAny = aState;

            return SFX_ITEM_AVAILABLE;
        }
    }

    return SFX_ITEM_DISABLED;
}

sal_Bool SfxDispatcher::IsReadOnlyShell_Impl( sal_uInt16 nShell ) const
{
    sal_uInt16 nShellCount = pImp->aStack.size();
    if ( nShell < nShellCount )
    {
        SfxShell* pShell = *( pImp->aStack.rbegin() + nShell );
        if( pShell->ISA( SfxModule ) || pShell->ISA( SfxApplication ) || pShell->ISA( SfxViewFrame ) )
            return sal_False;
        else
            return pImp->bReadOnly;
    }
    else if ( pImp->pParent )
        return pImp->pParent->IsReadOnlyShell_Impl( nShell - nShellCount );
    return sal_True;
}

void SfxDispatcher::RemoveShell_Impl( SfxShell& rShell )
{
    Flush();

    sal_uInt16 nCount = pImp->aStack.size();
    for ( sal_uInt16 n=0; n<nCount; ++n )
    {
        if ( pImp->aStack[n] == &rShell )
        {
            pImp->aStack.erase( pImp->aStack.begin() + n );
            rShell.SetDisableFlags( 0 );
            rShell.DoDeactivate_Impl(pImp->pFrame, sal_True);
            break;
        }
    }

    if ( !SFX_APP()->IsDowning() )
    {
        pImp->bUpdated = sal_False;
        pImp->pCachedServ1 = 0;
        pImp->pCachedServ2 = 0;
        InvalidateBindings_Impl(sal_True);
    }
}

void SfxDispatcher::InvalidateBindings_Impl( sal_Bool bModify )
{
    // App-Dispatcher?
    if ( IsAppDispatcher() )
    {
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
                pFrame;
                pFrame = SfxViewFrame::GetNext( *pFrame ) )
            pFrame->GetBindings().InvalidateAll(bModify);
    }
    else
    {
        SfxDispatcher *pDisp = GetBindings()->GetDispatcher_Impl();
        while ( pDisp )
        {
            if ( pDisp == this )
            {
                GetBindings()->InvalidateAll( bModify );
                break;
            }

            pDisp = pDisp->pImp->pParent;
        }
    }
}

sal_Bool SfxDispatcher::IsUpdated_Impl() const
{
    return pImp->bUpdated;
}

void SfxDispatcher::SetDisableFlags( sal_uInt32 nFlags )
{
    pImp->nDisableFlags = nFlags;
    for ( SfxShellStack_Impl::const_reverse_iterator it = pImp->aStack.rbegin(); it != pImp->aStack.rend(); ++it )
        (*it)->SetDisableFlags( nFlags );
}

sal_uInt32 SfxDispatcher::GetDisableFlags() const
{
    return pImp->nDisableFlags;
}

SfxModule* SfxDispatcher::GetModule() const
{
    for ( sal_uInt16 nShell = 0;; ++nShell )
    {
        SfxShell *pSh = GetShell(nShell);
        if ( pSh == NULL )
            return 0;
        if ( pSh->ISA(SfxModule) )
            return (SfxModule*) pSh;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
