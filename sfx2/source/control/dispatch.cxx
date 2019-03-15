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

#include <config_features.h>

#include <algorithm>
#include <deque>
#include <vector>

#include <stdarg.h>
#include <stdlib.h>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/hintpost.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/module.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>
#include <svl/undo.hxx>
#include <svl/whiter.hxx>
#include <svtools/helpopt.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/idle.hxx>

#include <appdata.hxx>
#include <sfxtypes.hxx>
#include <slotserv.hxx>
#include <workwin.hxx>

typedef std::vector<SfxShell*> SfxShellStack_Impl;

struct SfxToDo_Impl
{
    SfxShell*  pCluster;
    bool       bPush;
    bool       bDelete;
    bool       bDeleted;
    bool       bUntil;

    SfxToDo_Impl( bool bOpPush, bool bOpDelete, bool bOpUntil, SfxShell& rCluster )
        : pCluster(&rCluster)
        , bPush(bOpPush)
        , bDelete(bOpDelete)
        , bDeleted(false)
        , bUntil(bOpUntil)
                {}
};

struct SfxObjectBars_Impl
{
    ToolbarId          eId;      // ConfigId of the Toolbox
    sal_uInt16         nPos;
    SfxVisibilityFlags nFlags;   // special visibility flags

    SfxObjectBars_Impl() : eId(ToolbarId::None), nPos(0), nFlags(SfxVisibilityFlags::Invisible) {}
};

struct SfxDispatcher_Impl
{
    //When the dispatched is locked, SfxRequests accumulate in aReqArr for
    //later dispatch when unlocked via Post

    //The pointers are typically deleted in Post, only if we never get around
    //to posting them do we delete the unposted requests.
    std::vector<std::unique_ptr<SfxRequest>>
                         aReqArr;
    SfxShellStack_Impl   aStack;        // active functionality
    Idle                 aIdle;        // for Flush
    std::deque<SfxToDo_Impl> aToDoStack;    // not processed Push/Pop
    SfxViewFrame*        pFrame;        // NULL or associated Frame
    tools::SvRef<SfxHintPoster>
                         xPoster;       // Execute asynchronous
    bool                 bFlushing;     // sal_True during Flush //?
    bool                 bUpdated;      // Update_Impl has run
    bool                 bLocked;       // No Execute
    bool                 bInvalidateOnUnlock;   // because someone asked
    bool                 bActive;       // not to be confused with set!
    bool*                pInCallAliveFlag;   // view the Destructor Stack
    SfxObjectBars_Impl   aObjBars[SFX_OBJECTBAR_MAX];
    SfxObjectBars_Impl   aFixedObjBars[SFX_OBJECTBAR_MAX];
    std::vector<sal_uInt32> aChildWins;
    bool                 bNoUI;         // UI only from Parent Dispatcher
    bool                 bReadOnly;     // Document is ReadOnly
    bool                 bQuiet;        // Only use parent dispatcher

    SfxSlotFilterState   nFilterEnabling; // 1==filter enabled slots,
                                          // 2==ReadOnlyDoc overturned
    o3tl::span<sal_uInt16 const>
                         pFilterSIDs;   // sorted Array of SIDs
    SfxDisableFlags      nDisableFlags;
    bool                 bFlushed;
    std::deque< std::deque<SfxToDo_Impl> > aToDoCopyStack;
};

namespace {

    boost::property_tree::ptree fillPopupMenu(Menu* pMenu)
    {
        // Activate this menu first
        pMenu->HandleMenuActivateEvent(pMenu);
        pMenu->HandleMenuDeActivateEvent(pMenu);

        boost::property_tree::ptree aTree;
        // If last item inserted is some valid text
        bool bIsLastItemText = false;
        sal_uInt16 nCount = pMenu->GetItemCount();
        for (sal_uInt16 nPos = 0; nPos < nCount; nPos++)
        {
            boost::property_tree::ptree aItemTree;
            const MenuItemType aItemType = pMenu->GetItemType(nPos);

            if (aItemType == MenuItemType::DONTKNOW)
                continue;

            if (aItemType == MenuItemType::SEPARATOR)
            {
                if (bIsLastItemText)
                    aItemTree.put("type", "separator");
                bIsLastItemText = false;
            }
            else
            {
                const sal_uInt16 nItemId = pMenu->GetItemId(nPos);
                OUString aCommandURL = pMenu->GetItemCommand(nItemId);

                if (aCommandURL.isEmpty())
                {
                    const SfxSlot *pSlot = SFX_SLOTPOOL().GetSlot(nItemId);
                    if (pSlot)
                        aCommandURL = pSlot->GetCommandString();
                }

                const OUString aItemText = pMenu->GetItemText(nItemId);
                Menu* pPopupSubmenu = pMenu->GetPopupMenu(nItemId);

                if (!aItemText.isEmpty())
                    aItemTree.put("text", aItemText.toUtf8().getStr());

                if (pPopupSubmenu)
                {
                    boost::property_tree::ptree aSubmenu = fillPopupMenu(pPopupSubmenu);
                    if (aSubmenu.empty())
                        continue;

                    aItemTree.put("type", "menu");
                    if (!aCommandURL.isEmpty())
                        aItemTree.put("command", aCommandURL.toUtf8().getStr());
                    aItemTree.push_back(std::make_pair("menu", aSubmenu));
                }
                else
                {
                    // no point in exposing choices that don't have the .uno:
                    // command
                    if (aCommandURL.isEmpty())
                        continue;

                    aItemTree.put("type", "command");
                    aItemTree.put("command", aCommandURL.toUtf8().getStr());
                }

                aItemTree.put("enabled", pMenu->IsItemEnabled(nItemId));

                MenuItemBits aItemBits = pMenu->GetItemBits(nItemId);
                bool bHasChecks = true;
                if (aItemBits & MenuItemBits::CHECKABLE)
                    aItemTree.put("checktype", "checkmark");
                else if (aItemBits & MenuItemBits::RADIOCHECK)
                    aItemTree.put("checktype", "radio");
                else if (aItemBits & MenuItemBits::AUTOCHECK)
                    aItemTree.put("checktype", "auto");
                else
                    bHasChecks = false;

                if (bHasChecks)
                    aItemTree.put("checked", pMenu->IsItemChecked(nItemId));
            }

            if (!aItemTree.empty())
            {
                aTree.push_back(std::make_pair("", aItemTree));
                if (aItemType != MenuItemType::SEPARATOR)
                    bIsLastItemText = true;
            }
        }

        return aTree;
    }

} // end anonymous namespace


/** This method checks if the stack of the SfxDispatchers is flushed, or if
    push- or pop- commands are pending.
*/
bool SfxDispatcher::IsFlushed() const
{
     return xImp->bFlushed;
}

/** This method performs outstanding push- and pop- commands. For <SfxShell>s,
    which are new on the stack, the <SfxShell::Activate(bool)> is invoked
    with bMDI == sal_True, for SfxShells that are removed from the stack, the
    <SfxShell::Deactivate(bool)> is invoked with bMDI == sal_True
*/
void SfxDispatcher::Flush()
{
    if (!xImp->bFlushed) FlushImpl();
}

/** With this method, a <SfxShell> pushed on to the SfxDispatcher.
    The SfxShell is first marked for push and a timer is set up.
    First when the timer has counted down to zero the push
    ( <SfxDispatcher::Flush()> ) is actually performed and the
    <SfxBindings> is invalidated. While the timer is counting down
    the opposing push and pop commands on the same SfxShell are
    leveled out.
*/
void SfxDispatcher::Push(SfxShell& rShell)

{
    Pop( rShell, SfxDispatcherPopFlags::PUSH );
}

/** This method checks whether a particular <SfxShell> instance is
    on the SfxDispatcher.

    @returns true   The SfxShell instance is on the SfxDispatcher.
             false  The SfxShell instance is not on the SfxDispatcher.
*/
bool SfxDispatcher::IsActive(const SfxShell& rShell)

{
    return CheckVirtualStack(rShell);
}

/** With this method it can be determined whether the SfxDispatcher is
    locked or unlocked. A locked SfxDispatcher does not perform <SfxRequest>s
    and no longer provides any status information. It behaves as if all the
    slots are disabled.

    The dispatcher is also marked as blocked, if all Dispatcher are locked
    (<SfxApplication::LockDispatcher()>) or the associated top frame is in the
    modal-mode and if the specified slot are handled as frame-specific
    (ie, not served by the application).
*/
bool SfxDispatcher::IsLocked() const
{
    return xImp->bLocked;
}

/** With this method it can be determined if the SfxDispacher is the
    applications dispatcher.

    @return bool it is the application dispatcher.
*/
bool SfxDispatcher::IsAppDispatcher() const
{
    return !xImp->pFrame;
}

/** Helper function to check whether a slot can be executed and
    check the execution itself
*/
void SfxDispatcher::Call_Impl(SfxShell& rShell, const SfxSlot &rSlot, SfxRequest &rReq, bool bRecord)
{
    SFX_STACK(SfxDispatcher::Call_Impl);

    // The slot may be called (meaning enabled)
    if ( !rSlot.IsMode(SfxSlotMode::FASTCALL) && !rShell.CanExecuteSlot_Impl(rSlot) && !rShell.IsConditionalFastCall(rReq) )
        return;

    if ( GetFrame() )
    {
        // Recording may start
        css::uno::Reference< css::frame::XFrame > xFrame(
                GetFrame()->GetFrame().GetFrameInterface(),
                css::uno::UNO_QUERY);

        css::uno::Reference< css::beans::XPropertySet > xSet(
                xFrame,
                css::uno::UNO_QUERY);

        if ( xSet.is() )
        {
            css::uno::Any aProp = xSet->getPropertyValue("DispatchRecorderSupplier");
            css::uno::Reference< css::frame::XDispatchRecorderSupplier > xSupplier;
            css::uno::Reference< css::frame::XDispatchRecorder > xRecorder;
            aProp >>= xSupplier;
            if(xSupplier.is())
                xRecorder = xSupplier->getDispatchRecorder();

            if ( bRecord && xRecorder.is() && !rSlot.IsMode(SfxSlotMode::NORECORD) )
                rReq.Record_Impl( rShell, rSlot, xRecorder, GetFrame() );
        }
    }
    // Get all that is needed, because the slot may not have survived the
    // Execute if it is a 'pseudo slot' for macros or verbs.
    bool bAutoUpdate = rSlot.IsMode(SfxSlotMode::AUTOUPDATE);

    // API-call parentheses and document-lock during the calls
    {
        // 'this' must respond in the Destructor
        bool bThisDispatcherAlive = true;
        bool *pOldInCallAliveFlag = xImp->pInCallAliveFlag;
        xImp->pInCallAliveFlag = &bThisDispatcherAlive;

        SfxExecFunc pFunc = rSlot.GetExecFnc();
        rShell.CallExec( pFunc, rReq );

        // If 'this' is still alive
        if ( bThisDispatcherAlive )
            xImp->pInCallAliveFlag = pOldInCallAliveFlag;
        else
        {
            if ( pOldInCallAliveFlag )
            {
                // also protect nested stack frames
                *pOldInCallAliveFlag = false;
            }

            // do nothing after this object is dead
            return;
        }
    }

    if ( rReq.IsDone() )
    {
        SfxBindings *pBindings = GetBindings();

        // When AutoUpdate update immediately
        if ( bAutoUpdate && pBindings )
        {
            pBindings->Invalidate(rSlot.GetSlotId());
            pBindings->Update(rSlot.GetSlotId());
        }
    }
}

void SfxDispatcher::Construct_Impl()
{
    xImp.reset(new SfxDispatcher_Impl);
    xImp->bFlushed = true;

    xImp->bFlushing = false;
    xImp->bUpdated = false;
    xImp->bLocked = false;
    xImp->bActive = false;
    xImp->bNoUI = false;
    xImp->bReadOnly = false;
    xImp->bQuiet = false;
    xImp->pInCallAliveFlag = nullptr;
    xImp->nFilterEnabling = SfxSlotFilterState::DISABLED;
    xImp->nDisableFlags = SfxDisableFlags::NONE;

    xImp->bInvalidateOnUnlock = false;

    for (SfxObjectBars_Impl & rObjBar : xImp->aObjBars)
        rObjBar.eId = ToolbarId::None;

    xImp->xPoster = new SfxHintPoster(std::bind(&SfxDispatcher::PostMsgHandler, this, std::placeholders::_1));

    xImp->aIdle.SetPriority(TaskPriority::HIGH_IDLE );
    xImp->aIdle.SetInvokeHandler( LINK(this, SfxDispatcher, EventHdl_Impl ) );
    xImp->aIdle.SetDebugName( "sfx::SfxDispatcher_Impl aIdle" );
}

SfxDispatcher::SfxDispatcher()
{
    Construct_Impl();
    xImp->pFrame = nullptr;
}

/** The constructor of the SfxDispatcher class places a stack of empty
    <SfxShell> pointers. It is not initially locked and is considered flushed.
*/
SfxDispatcher::SfxDispatcher(SfxViewFrame *pViewFrame)
{
    Construct_Impl();
    xImp->pFrame = pViewFrame;
}

/** The destructor of the SfxDispatcher class should not be called when the
    SfxDispatcher instance is active. It may, however, still be a <SfxShell>
    pointer on the stack.
*/
SfxDispatcher::~SfxDispatcher()
{
    SAL_INFO("sfx.control", "Delete Dispatcher " << reinterpret_cast<sal_Int64>(this));
    DBG_ASSERT( !xImp->bActive, "deleting active Dispatcher" );

    // So that no timer by Reschedule in PlugComm strikes the LeaveRegistrations
    xImp->aIdle.Stop();
    xImp->xPoster->SetEventHdl( std::function<void (std::unique_ptr<SfxRequest>)>() );

    // Notify the stack variables in Call_Impl
    if ( xImp->pInCallAliveFlag )
        *xImp->pInCallAliveFlag = false;

    // Get bindings and application
    SfxApplication *pSfxApp = SfxGetpApp();
    SfxBindings* pBindings = GetBindings();

    // When not flushed, revive the bindings
    if (pBindings && !pSfxApp->IsDowning() && !xImp->bFlushed)
        pBindings->DLEAVEREGISTRATIONS();

    // may unregister the bindings
    while ( pBindings )
    {
        if ( pBindings->GetDispatcher_Impl() == this)
            pBindings->SetDispatcher(nullptr);
        pBindings = pBindings->GetSubBindings_Impl();
    }
}

/** With this method, one or more <SfxShell> are poped from the SfxDispatcher.
    The SfxShell is marked for popping and a timer is set up. Only when the
    timer has reached the end, the pop is actually performed
    ( <SfxDispatcher::Flush()> ) and the <SfxBindings> is invalidated.
    While the timer is running the opposing push and pop commands on one
    SfxShell cancel each other out.

    @param rShell the stack to take the SfxShell instance.
    @param nMode SfxDispatcherPopFlags::POP_UNTIL
                            Also all 'rShell' of SfxShells are taken from the
                            stack.

                 SfxDispatcherPopFlags::POP_DELETE
                            All SfxShells actually taken from the stack
                            will be deleted.

                 SfxDispatcherPopFlags::PUSH (InPlace use only)
                            The Shell is pushed.
*/
void SfxDispatcher::Pop(SfxShell& rShell, SfxDispatcherPopFlags nMode)
{
    DBG_ASSERT( rShell.GetInterface(),
                "pushing SfxShell without previous RegisterInterface()" );

    bool bDelete = bool(nMode & SfxDispatcherPopFlags::POP_DELETE);
    bool bUntil = bool(nMode & SfxDispatcherPopFlags::POP_UNTIL);
    bool bPush = bool(nMode & SfxDispatcherPopFlags::PUSH);

    SfxApplication *pSfxApp = SfxGetpApp();

    SAL_INFO(
        "sfx.control",
        "-SfxDispatcher(" << this << (bPush ? ")::Push(" : ")::Pop(")
            << (rShell.GetInterface()
                ? rShell.GetInterface()->GetClassName() : SAL_STREAM(&rShell))
            << (bDelete ? ") with delete" : ")")
            << (bUntil ? " (up to)" : ""));

    // same shell as on top of the to-do stack?
    if(!xImp->aToDoStack.empty() && xImp->aToDoStack.front().pCluster == &rShell)
    {
        // cancel inverse actions
        if ( xImp->aToDoStack.front().bPush != bPush )
            xImp->aToDoStack.pop_front();
        else
        {
            DBG_ASSERT( bPush, "SfxInterface pushed more than once" );
            DBG_ASSERT( !bPush, "SfxInterface popped more than once" );
        }
    }
    else
    {
        // Remember Action
        xImp->aToDoStack.push_front( SfxToDo_Impl(bPush, bDelete, bUntil, rShell) );
        if (xImp->bFlushed)
        {
            SAL_INFO("sfx.control", "Unflushed dispatcher!");
            xImp->bFlushed = false;
            xImp->bUpdated = false;

            // Put bindings to sleep
            SfxBindings* pBindings = GetBindings();
            if ( pBindings )
                pBindings->DENTERREGISTRATIONS();
        }
    }

    if(!pSfxApp->IsDowning() && !xImp->aToDoStack.empty())
    {
        // No immediate update is requested
        xImp->aIdle.Start();
    }
    else
    {
        // but to do nothing
        xImp->aIdle.Stop();

        // Bindings may wake up again
        if(xImp->aToDoStack.empty())
        {
            SfxBindings* pBindings = GetBindings();
            if ( pBindings )
                pBindings->DLEAVEREGISTRATIONS();
        }
    }
}


/** This handler is called after <SfxDispatcher::Invalidate()> or after
    changes on the stack (<SfxDispatcher::Push()> and <SfxDispatcher::Pop())

    It flushes the Stack, if it is dirty, thus it actually executes the
    pending Push and Pop commands.
*/
IMPL_LINK_NOARG( SfxDispatcher, EventHdl_Impl, Timer *, void )
{
    Flush();
    Update_Impl();
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->StartUpdate_Impl();
}

/** With this method it can be tested whether the <SfxShell> rShell is on the
    stack, when it was flushed. This way the SfxDispatcher is not actually
    flushed.

    This method is intended among other things to make assertions possible
    without the side effect of having to flush the SfxDispathcer.
*/
bool SfxDispatcher::CheckVirtualStack(const SfxShell& rShell)
{
    SFX_STACK(SfxDispatcher::CheckVirtualStack);

    SfxShellStack_Impl aStack( xImp->aStack );
    for(std::deque<SfxToDo_Impl>::reverse_iterator i = xImp->aToDoStack.rbegin(); i != xImp->aToDoStack.rend(); ++i)
    {
        if(i->bPush)
            aStack.push_back(i->pCluster);
        else
        {
            SfxShell* pPopped(nullptr);
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

    bool bReturn = std::find(aStack.begin(), aStack.end(), &rShell) != aStack.end();
    return bReturn;
}

/** Determines the position of a given SfxShell in the stack of the dispatcher.
    If possible this is flushed before.

    [Return value]

    sal_uInt16                  == USRT_MAX
                                The SfxShell is not on this SfxDispatcher.

                                < USHRT_MAX
                                Position of the SfxShell on the Dispatcher
                                from the top count stating with 0.
*/
sal_uInt16 SfxDispatcher::GetShellLevel(const SfxShell& rShell)
{
    SFX_STACK(SfxDispatcher::GetShellLevel);
    Flush();

    for ( size_t n = 0; n < xImp->aStack.size(); ++n )
        if ( *( xImp->aStack.rbegin() + n ) == &rShell )
            return n;

    return USHRT_MAX;
}

/** Returns a pointer to the <SfxShell> which is at the position nIdx
    (from the top, last pushed is 0) on the stack.

    Thus the SfxDispatcher is not flushed.

    Is the stack not deep enough a NULL-Pointer is returned.
*/
SfxShell *SfxDispatcher::GetShell(sal_uInt16 nIdx) const
{
    sal_uInt16 nShellCount = xImp->aStack.size();
    if ( nIdx < nShellCount )
        return *(xImp->aStack.rbegin() + nIdx);
    return nullptr;
}

/** This method returns a pointer to the <SfxBinding> Instance on which the
    SfxDispatcher is currently bound. A SfxDispatcher is only bound to
    the SfxBindings when it is <UI-aktiv>. If it is not UI-active,
    a NULL-pointer is returned.

    The returned pointer is only valid in the immediate context of the method
    call.
*/
SfxBindings* SfxDispatcher::GetBindings() const
{
    if ( xImp->pFrame )
        return &xImp->pFrame->GetBindings();
    else
        return nullptr;
}

/** Returns a pointer to the <SfxViewFrame> instance, which belongs to
    this SfxDispatcher. If it is about the application dispatcher,
    a NULL-pointer is returned.
*/
SfxViewFrame* SfxDispatcher::GetFrame() const
{
    return xImp->pFrame;
}

/** This method controls the activation of a dispatcher.

    Since the application dispatcher is always active, either as a sub
    dispatcher of the <SfxViewFrame> dispatcher or as itself, it is never
    activated as a whole, instead only its individual <SfxShell>s at
    <SfxDispatcher::Push(SfxShell&)>.

    When activating a SfxDispatcher all of the SfxShells located on its stack
    are called with the handler <SfxShell::Activate(bool)>, starting with
    the lowest.
*/
void SfxDispatcher::DoActivate_Impl(bool bMDI)
{
    SFX_STACK(SfxDispatcher::DoActivate);
    if ( bMDI )
    {
        SAL_INFO("sfx.control", "Activate Dispatcher " << reinterpret_cast<sal_Int64>(this));
        DBG_ASSERT( !xImp->bActive, "Activation error" );

        xImp->bActive = true;
        xImp->bUpdated = false;
        SfxBindings* pBindings = GetBindings();
        if ( pBindings )
        {
            pBindings->SetDispatcher(this);
            pBindings->SetActiveFrame( xImp->pFrame->GetFrame().GetFrameInterface() );
        }
    }
    else
    {
        SAL_INFO("sfx.control", "Non-MDI-Activate Dispatcher " << reinterpret_cast<sal_Int64>(this));
    }

    if ( IsAppDispatcher() )
        return;

    for ( int i = int(xImp->aStack.size()) - 1; i >= 0; --i )
        (*(xImp->aStack.rbegin() + i ))->DoActivate_Impl(xImp->pFrame, bMDI);

    if ( bMDI && xImp->pFrame )
    {
        xImp->pFrame->GetFrame().GetWorkWindow_Impl()->HidePopups_Impl( false, 1 );
    }

    if(!xImp->aToDoStack.empty())
    {
        // No immediate update is requested
        xImp->aIdle.Start();
    }
}

/** This method controls the deactivation of a dispatcher.

    Since the application dispatcher is always active, either as a sub
    dispatcher of the <SfxViewFrame> dispatcher or as itself, it is never
    deactivated as a whole, instead only its individual <SfxShell>s at
    <SfxDispatcher::Pop(SfxShell&)>.

    When deactivating a SfxDispatcher all of the SfxShells located on its stack
    are called with the handler <SfxShell::Deactivate(bool)>, starting with
    the lowest.
*/
void SfxDispatcher::DoDeactivate_Impl(bool bMDI, SfxViewFrame const * pNew)
{
    SFX_STACK(SfxDispatcher::DoDeactivate);

    SfxApplication *pSfxApp = SfxGetpApp();

    if ( bMDI )
    {
        SAL_INFO("sfx.control", "Deactivate Dispatcher " << this);
        DBG_ASSERT( xImp->bActive, "Deactivate error" );
        xImp->bActive = false;

        if ( xImp->pFrame && !(xImp->pFrame->GetObjectShell()->IsInPlaceActive() ) )
        {
            SfxWorkWindow *pWorkWin = xImp->pFrame->GetFrame().GetWorkWindow_Impl();
            if ( pWorkWin )
            {
                for (size_t n=0; n<xImp->aChildWins.size();)
                {
                    SfxChildWindow *pWin = pWorkWin->GetChildWindow_Impl( static_cast<sal_uInt16>( xImp->aChildWins[n] & 0xFFFF ) );
                    if (!pWin || pWin->GetAlignment() == SfxChildAlignment::NOALIGNMENT)
                        xImp->aChildWins.erase(xImp->aChildWins.begin()+n);
                    else
                        n++;
                }
            }
        }
    }
    else {
        SAL_INFO("sfx.control", "Non-MDI-DeActivate Dispatcher " << this);
    }

    if ( IsAppDispatcher() && !pSfxApp->IsDowning() )
        return;

    for ( size_t i = 0; i < xImp->aStack.size(); ++i )
        (*(xImp->aStack.rbegin() + i))->DoDeactivate_Impl(xImp->pFrame, bMDI);

    bool bHidePopups = bMDI && xImp->pFrame;
    if ( pNew && xImp->pFrame )
    {
        css::uno::Reference< css::frame::XFrame > xOldFrame(
            pNew->GetFrame().GetFrameInterface()->getCreator(), css::uno::UNO_QUERY );

        css::uno::Reference< css::frame::XFrame > xMyFrame(
            GetFrame()->GetFrame().GetFrameInterface(), css::uno::UNO_QUERY );

        if ( xOldFrame == xMyFrame )
            bHidePopups = false;
    }

    if ( bHidePopups )
    {
        xImp->pFrame->GetFrame().GetWorkWindow_Impl()->HidePopups_Impl( true, 1 );
    }

    Flush();
}

/** This method searches in SfxDispatcher after <SfxShell> , from the Slot Id
    nSlot currently being handled. For this, the dispatcher is first flushed.

    @param nSlot the searchable Slot-Id
    @param ppShell the SfxShell, which are currently handled the nSlot
    @param ppSlot the SfxSlot, which are currently handled the nSlot

    @return int      sal_True
                     The SfxShell was found, ppShell and ppSlot are valid.

                     sal_False
                     The SfxShell was not found, ppShell and ppSlot are invalid.
*/
bool SfxDispatcher::GetShellAndSlot_Impl(sal_uInt16 nSlot, SfxShell** ppShell,
        const SfxSlot** ppSlot, bool bOwnShellsOnly, bool bRealSlot)
{
    SFX_STACK(SfxDispatcher::GetShellAndSlot_Impl);

    Flush();
    SfxSlotServer aSvr;
    if ( FindServer_(nSlot, aSvr) )
    {
        if ( bOwnShellsOnly && aSvr.GetShellLevel() >= xImp->aStack.size() )
            return false;

        *ppShell = GetShell(aSvr.GetShellLevel());
        *ppSlot = aSvr.GetSlot();
        if ( nullptr == (*ppSlot)->GetExecFnc() && bRealSlot )
            *ppSlot = (*ppShell)->GetInterface()->GetRealSlot(*ppSlot);
        // Check only real slots as enum slots don't have an execute function!
        return !bRealSlot || !((nullptr == *ppSlot) || (nullptr == (*ppSlot)->GetExecFnc()) );
    }

    return false;
}

/** This method performs a request for a cached <Slot-Server>.

    @param rShell to the calling <SfxShell>
    @param rSlot to the calling <SfxSlot>
    @param rReq function to be performed (Id and optional parameters)
    @param eCallMode Synchronously, asynchronously or as shown in the slot
*/
void SfxDispatcher::Execute_(SfxShell& rShell, const SfxSlot& rSlot,
        SfxRequest& rReq, SfxCallMode eCallMode)
{
    SFX_STACK(SfxDispatcher::Execute_);
    DBG_ASSERT( !xImp->bFlushing, "recursive call to dispatcher" );
    DBG_ASSERT( xImp->aToDoStack.empty(), "unprepared InPlace _Execute" );

    if ( IsLocked() )
        return;

    if ( bool(eCallMode & SfxCallMode::ASYNCHRON) ||
         ( (eCallMode & SfxCallMode::SYNCHRON) == SfxCallMode::SLOT &&
           rSlot.IsMode(SfxSlotMode::ASYNCHRON) ) )
    {
        sal_uInt16 nShellCount = xImp->aStack.size();
        for ( sal_uInt16 n=0; n<nShellCount; n++ )
        {
            if ( &rShell == *(xImp->aStack.rbegin() + n) )
            {
                if ( bool(eCallMode & SfxCallMode::RECORD) )
                    rReq.AllowRecording( true );
                xImp->xPoster->Post(std::make_unique<SfxRequest>(rReq));
                return;
            }
        }
    }
    else
        Call_Impl( rShell, rSlot, rReq, SfxCallMode::RECORD==(eCallMode&SfxCallMode::RECORD) );
}

/** Helper function to put from rItem below the Which-ID in the pool of the
    Item Sets rSet.
*/
static void MappedPut_Impl(SfxAllItemSet &rSet, const SfxPoolItem &rItem)
{
    // Put with mapped Which-Id if possible
    const SfxItemPool *pPool = rSet.GetPool();
    sal_uInt16 nWhich = rItem.Which();
    if ( SfxItemPool::IsSlot(nWhich) )
        nWhich = pPool->GetWhich(nWhich);
    rSet.Put( rItem, nWhich );
}

const SfxSlot* SfxDispatcher::GetSlot( const OUString& rCommand )
{
    // Count the number of Shells on the linked Dispatcher
    Flush();
    sal_uInt16 nTotCount = xImp->aStack.size();

    for ( sal_uInt16 i = 0; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot( rCommand );
        if ( pSlot )
            return pSlot;
    }

    return nullptr;
}

const SfxPoolItem* SfxDispatcher::Execute(sal_uInt16 nSlot, SfxCallMode nCall,
        SfxItemSet const * pArgs, SfxItemSet const * pInternalArgs, sal_uInt16 nModi)
{
    if ( IsLocked() )
        return nullptr;

    SfxShell *pShell = nullptr;
    const SfxSlot *pSlot = nullptr;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, false, true ) )
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
        SfxRequest aReq(nSlot, nCall, aSet);
        if (pInternalArgs)
            aReq.SetInternalArgs_Impl( *pInternalArgs );
        aReq.SetModifier( nModi );

        Execute_( *pShell, *pSlot, aReq, nCall );
        return aReq.GetReturnValue();
    }
    return nullptr;
}

/** Method to execute a <SfxSlot>s over the Slot-Id.

    @param nSlot the Id of the executing function
    @param eCall SfxCallMode::SYNCRHON, ..._ASYNCHRON or ..._SLOT
    @param pArgs Zero terminated C-Array of Parameters
    @param pInternalArgs Zero terminated C-Array of Parameters

    @return const SfxPoolItem* Pointer to the SfxPoolItem valid to the next run
                            though the Message-Loop, which contains the return
                            value.

                            Or a NULL-Pointer, when the function was not
                            executed (for example canceled by the user).
*/
const SfxPoolItem* SfxDispatcher::Execute(sal_uInt16 nSlot, SfxCallMode eCall,
        const SfxPoolItem **pArgs, sal_uInt16 nModi, const SfxPoolItem **pInternalArgs)
{
    if ( IsLocked() )
        return nullptr;

    SfxShell *pShell = nullptr;
    const SfxSlot *pSlot = nullptr;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, false, true ) )
    {
        std::unique_ptr<SfxRequest> pReq;
        if ( pArgs && *pArgs )
        {
            SfxAllItemSet aSet( pShell->GetPool() );
            for ( const SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                MappedPut_Impl( aSet, **pArg );
            pReq.reset(new SfxRequest( nSlot, eCall, aSet ));
        }
        else
            pReq.reset(new SfxRequest( nSlot, eCall, pShell->GetPool() ));
        pReq->SetModifier( nModi );
        if( pInternalArgs && *pInternalArgs)
        {
            SfxAllItemSet aSet( SfxGetpApp()->GetPool() );
            for ( const SfxPoolItem **pArg = pInternalArgs; *pArg; ++pArg )
                aSet.Put( **pArg );
            pReq->SetInternalArgs_Impl( aSet );
        }
        Execute_( *pShell, *pSlot, *pReq, eCall );
        const SfxPoolItem* pRet = pReq->GetReturnValue();
        return pRet;
    }
    return nullptr;
}

/** Method to execute a <SfxSlot>s over the Slot-Id.

    @param nSlot the Id of the executing function
    @param eCall SfxCallMode::SYNCRHON, ..._ASYNCHRON or ..._SLOT
    @param rArgs <SfxItemSet> with the parameters

    @return const SfxPoolItem* Pointer to the SfxPoolItem valid to the next run
                            though the Message-Loop, which contains the return
                            value.

                            Or a NULL-Pointer, when the function was not
                            executed (for example canceled by the user).
*/
const SfxPoolItem* SfxDispatcher::Execute(sal_uInt16 nSlot, SfxCallMode eCall,
        const SfxItemSet &rArgs)
{
    if ( IsLocked() )
        return nullptr;

    SfxShell *pShell = nullptr;
    const SfxSlot *pSlot = nullptr;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, false, true ) )
    {
        SfxAllItemSet aSet( pShell->GetPool() );
        SfxItemIter aIter(rArgs);
        for ( const SfxPoolItem *pArg = aIter.FirstItem();
              pArg;
              pArg = aIter.NextItem() )
            MappedPut_Impl( aSet, *pArg );
        SfxRequest aReq( nSlot, eCall, aSet );
        aReq.SetModifier( 0 );
        Execute_( *pShell, *pSlot, aReq, eCall );
        return aReq.GetReturnValue();
    }
    return nullptr;
}

/** Method to execute a <SfxSlot>s over the Slot-Id.

    [Note]

    The parameters are copied, can therefore be passed on as the address
    of stack objects.

    @param nSlot the Id of the executing function
    @param eCall SfxCallMode::SYNCRHON, ..._ASYNCHRON or ..._SLOT
    @param args  list of SfxPoolItem arguments

    @return                 Pointer to the SfxPoolItem valid to the next run
                            though the Message-Loop, which contains the return
                            value.

                            Or a NULL-Pointer, when the function was not
                            executed (for example canceled by the user).

    [Example]

    pDispatcher->Execute( SID_OPENDOCUMENT, SfxCallMode::SYNCHRON,
        {   &SfxStringItem( SID_FILE_NAME, "\\tmp\\temp.sdd" ),
            &SfxStringItem( SID_FILTER_NAME, "StarDraw Presentation" ),
            &SfxBoolItem( SID_DOC_READONLY, sal_False ),
        });
*/
const SfxPoolItem* SfxDispatcher::ExecuteList(sal_uInt16 nSlot, SfxCallMode eCall,
        std::initializer_list<SfxPoolItem const*> args,
        std::initializer_list<SfxPoolItem const*> internalargs)
{
    if ( IsLocked() )
        return nullptr;

    SfxShell *pShell = nullptr;
    const SfxSlot *pSlot = nullptr;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, false, true ) )
    {
       SfxAllItemSet aSet( pShell->GetPool() );

       for (const SfxPoolItem *pArg : args)
       {
           assert(pArg);
           MappedPut_Impl( aSet, *pArg );
       }

       SfxRequest aReq(nSlot, eCall, aSet);

       if (internalargs.begin() != internalargs.end())
       {
           SfxAllItemSet aInternalSet(SfxGetpApp()->GetPool());
           for (const SfxPoolItem *pArg : internalargs)
           {
               assert(pArg);
               aInternalSet.Put(*pArg);
           }
           aReq.SetInternalArgs_Impl(aInternalSet);
       }

       Execute_( *pShell, *pSlot, aReq, eCall );
       return aReq.GetReturnValue();
    }
    return nullptr;
}

/** Helper method to receive the asynchronously executed <SfxRequest>s.
*/
void SfxDispatcher::PostMsgHandler(std::unique_ptr<SfxRequest> pReq)
{
    DBG_ASSERT( !xImp->bFlushing, "recursive call to dispatcher" );
    SFX_STACK(SfxDispatcher::PostMsgHandler);

    // Has also the Pool not yet died?
    if ( pReq->IsCancelled() )
        return;

    if ( !IsLocked() )
    {
        Flush();
        SfxSlotServer aSvr;
        if ( FindServer_(pReq->GetSlot(), aSvr ) ) // HACK(x), whatever that was supposed to mean
        {
            const SfxSlot *pSlot = aSvr.GetSlot();
            SfxShell *pSh = GetShell(aSvr.GetShellLevel());

            // When the pSlot is a "Pseudoslot" for macros or Verbs, it can
            // be destroyed in the Call_Impl, thus do not use it anymore!
            pReq->SetSynchronCall( false );
            Call_Impl( *pSh, *pSlot, *pReq, pReq->AllowsRecording() ); //! why bRecord?
        }
    }
    else
    {
        if ( xImp->bLocked )
            xImp->aReqArr.emplace_back(std::move(pReq));
        else
            xImp->xPoster->Post(std::move(pReq));
    }
}

void SfxDispatcher::SetMenu_Impl()
{
#if HAVE_FEATURE_DESKTOP
    if ( !xImp->pFrame )
        return;

    SfxViewFrame* pTop = xImp->pFrame->GetTopViewFrame();
    if ( !pTop || pTop->GetBindings().GetDispatcher() != this )
        return;

    SfxFrame& rFrame = pTop->GetFrame();
    if ( !rFrame.IsMenuBarOn_Impl() )
        return;

    css::uno::Reference < css::beans::XPropertySet > xPropSet( rFrame.GetFrameInterface(), css::uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
        aValue >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            OUString aMenuBarURL( "private:resource/menubar/menubar" );
            if ( !xLayoutManager->isElementVisible( aMenuBarURL ) )
                xLayoutManager->createElement( aMenuBarURL );
        }
    }
#endif
}

void SfxDispatcher::Update_Impl( bool bForce )
{
    SFX_STACK(SfxDispatcher::Update_Impl);

    Flush();

    if ( !xImp->pFrame )
        return;

    bool bUpdate = bForce;
    if ( xImp->pFrame )
    {
        SfxWorkWindow *pWork = xImp->pFrame->GetFrame().GetWorkWindow_Impl();
        SfxDispatcher *pAct = pWork->GetBindings().GetDispatcher_Impl();
        if (pAct == this)
        {
            if ( !bUpdate )
                bUpdate = !xImp->bUpdated;
            xImp->bUpdated = true;
        }
    }

    if ( !bUpdate || xImp->pFrame->GetFrame().IsClosing_Impl() )
        return;

    SfxViewFrame* pTop = xImp->pFrame ? xImp->pFrame->GetTopViewFrame() : nullptr;
    bool bUIActive = pTop && pTop->GetBindings().GetDispatcher() == this && !comphelper::LibreOfficeKit::isActive();

    if ( !bUIActive && pTop && GetBindings() == &pTop->GetBindings() )
        // keep own tools internally for collecting
        GetBindings()->GetDispatcher()->xImp->bUpdated = false;

    css::uno::Reference< css::frame::XFrame > xFrame;
    SfxBindings* pBindings = GetBindings();
    if (pBindings)
    {
        pBindings->DENTERREGISTRATIONS();
        xFrame = pBindings->GetActiveFrame();
    }
    css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
        }
        catch (const css::uno::Exception&)
        {
        }
    }

    if ( xLayoutManager.is() )
        xLayoutManager->lock();

    bool bIsIPActive = xImp->pFrame && xImp->pFrame->GetObjectShell()->IsInPlaceActive();
    SfxInPlaceClient *pClient = xImp->pFrame ? xImp->pFrame->GetViewShell()->GetUIActiveClient() : nullptr;
    if ( bUIActive && /* !bIsIPActive && */ ( !pClient || !pClient->IsObjectUIActive() ) )
        SetMenu_Impl();

    SfxWorkWindow *pWorkWin = xImp->pFrame->GetFrame().GetWorkWindow_Impl();
    pWorkWin->ResetStatusBar_Impl();

    {
        SfxWorkWindow *pWork = xImp->pFrame->GetFrame().GetWorkWindow_Impl();
        SfxDispatcher *pAct = pWork->GetBindings().GetDispatcher_Impl();
        if (pAct == this)
        {
            pWork->ResetObjectBars_Impl();
            pWork->ResetChildWindows_Impl();
        }
    }

    bool bIsActive = false;
    SfxDispatcher *pActDispat = pWorkWin->GetBindings().GetDispatcher_Impl();
    if ( !bIsActive && this == pActDispat )
        bIsActive = true;

    Update_Impl_( bUIActive, !bIsIPActive, bIsIPActive, pWorkWin );
    if ( (bUIActive || bIsActive) && !comphelper::LibreOfficeKit::isActive() )
        pWorkWin->UpdateObjectBars_Impl();

    if ( pBindings )
        pBindings->DLEAVEREGISTRATIONS();

    if ( xLayoutManager.is() )
        xLayoutManager->unlock();

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetDispatcher() )
    {
        const SfxPoolItem *pItem;
        SfxViewShell::Current()->GetDispatcher()->QueryState(SID_NOTEBOOKBAR, pItem);
    }
}

void SfxDispatcher::Update_Impl_( bool bUIActive, bool bIsMDIApp, bool bIsIPOwner, SfxWorkWindow *pTaskWin )
{
    SfxWorkWindow *pWorkWin = xImp->pFrame->GetFrame().GetWorkWindow_Impl();
    bool bIsActive = false;
    SfxDispatcher *pActDispat = pWorkWin->GetBindings().GetDispatcher_Impl();
    if ( pActDispat && !bIsActive )
    {
        if ( this == pActDispat )
            bIsActive = true;
    }

    for (SfxObjectBars_Impl & rObjBar : xImp->aObjBars)
        rObjBar.eId = ToolbarId::None;
    xImp->aChildWins.clear();

    // bQuiet: own shells aren't considered for UI and SlotServer
    // bNoUI: own Shells aren't considered forms UI
    if ( xImp->bQuiet || xImp->bNoUI || (xImp->pFrame && xImp->pFrame->GetObjectShell()->IsPreview()) )
        return;

    StatusBarId eStatBarId = StatusBarId::None;

    SfxSlotPool* pSlotPool = &SfxSlotPool::GetSlotPool( GetFrame() );
    sal_uInt16 nTotCount = xImp->aStack.size();
    for ( sal_uInt16 nShell = nTotCount; nShell > 0; --nShell )
    {
        SfxShell *pShell = GetShell( nShell-1 );
        SfxInterface *pIFace = pShell->GetInterface();

        // don't consider shells if "Hidden" or "Quiet"
        bool bReadOnlyShell = IsReadOnlyShell_Impl( nShell-1 );
        sal_uInt16 nNo;
        for ( nNo = 0; pIFace && nNo<pIFace->GetObjectBarCount(); ++nNo )
        {
            sal_uInt16 nPos = pIFace->GetObjectBarPos(nNo);
            SfxVisibilityFlags nFlags = pIFace->GetObjectBarFlags(nNo);
            if ( bReadOnlyShell && !( nFlags & SfxVisibilityFlags::ReadonlyDoc ) )
                continue;

            // check whether toolbar needs activation of a special feature
            SfxShellFeature nFeature = pIFace->GetObjectBarFeature(nNo);
            if ((nFeature != SfxShellFeature::NONE) && !pShell->HasUIFeature(nFeature))
                continue;

            // check for toolboxes that are exclusively for a viewer
            if ( xImp->pFrame)
            {
                bool bViewerTbx( nFlags & SfxVisibilityFlags::Viewer );
                SfxObjectShell* pSh = xImp->pFrame->GetObjectShell();
                const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(pSh->GetMedium()->GetItemSet(), SID_VIEWONLY, false);
                bool bIsViewer = pItem && pItem->GetValue();
                if ( bIsViewer != bViewerTbx )
                    continue;
            }

            // always register toolbars, allows to switch them on
            bool bVisible = pIFace->IsObjectBarVisible(nNo);
            if ( !bVisible )
                nFlags = SfxVisibilityFlags::Invisible;

            SfxObjectBars_Impl& rBar = xImp->aObjBars[nPos];
            rBar.nPos = nPos;
            rBar.nFlags = nFlags;
            rBar.eId = pIFace->GetObjectBarId(nNo);

            if ( bUIActive || bIsActive )
            {
                pWorkWin->SetObjectBar_Impl(nPos, nFlags, rBar.eId);
            }

            if ( !bVisible )
                rBar.eId = ToolbarId::None;
        }

        for ( nNo=0; pIFace && nNo<pIFace->GetChildWindowCount(); nNo++ )
        {
            sal_uInt32 nId = pIFace->GetChildWindowId(nNo);
            const SfxSlot *pSlot = pSlotPool->GetSlot( static_cast<sal_uInt16>(nId) );
            SAL_WARN_IF( !pSlot, "sfx.control", "Childwindow slot missing: " << nId );
            if ( bReadOnlyShell )
            {
                // only show ChildWindows if their slot is allowed for readonly documents
                if ( pSlot && !pSlot->IsMode( SfxSlotMode::READONLYDOC ) )
                    continue;
            }

            SfxShellFeature nFeature = pIFace->GetChildWindowFeature(nNo);
            if ((nFeature != SfxShellFeature::NONE) && !pShell->HasUIFeature(nFeature))
                continue;

            // slot decides whether a ChildWindow is shown when document is OLE server or OLE client
            SfxVisibilityFlags nMode = SfxVisibilityFlags::Standard;
            if( pSlot )
            {
                if ( pSlot->IsMode(SfxSlotMode::CONTAINER) )
                {
                    if ( pWorkWin->IsVisible_Impl( SfxVisibilityFlags::Client ) )
                        nMode |= SfxVisibilityFlags::Client;
                }
                else
                {
                    if ( pWorkWin->IsVisible_Impl( SfxVisibilityFlags::Server ) )
                        nMode |= SfxVisibilityFlags::Server;
                }
            }

            if ( bUIActive || bIsActive )
                pWorkWin->SetChildWindowVisible_Impl( nId, true, nMode );
            if ( bUIActive || bIsActive || !pWorkWin->IsFloating( static_cast<sal_uInt16>( nId & 0xFFFF ) ) )
                xImp->aChildWins.push_back( nId );
        }

        if ( bIsMDIApp || bIsIPOwner )
        {
            StatusBarId eId = pIFace ? pIFace->GetStatusBarId() : StatusBarId::None;
            if (eId != StatusBarId::None)
                eStatBarId = eId;
        }
    }

    for ( sal_uInt16 nPos=0; nPos<SFX_OBJECTBAR_MAX; nPos++ )
    {
        SfxObjectBars_Impl& rFixed = xImp->aFixedObjBars[nPos];
        if (rFixed.eId != ToolbarId::None)
        {
            SfxObjectBars_Impl& rBar = xImp->aObjBars[nPos];
            rBar = rFixed;
            pWorkWin->SetObjectBar_Impl(rFixed.nPos, rFixed.nFlags,
                rFixed.eId);
        }
    }

    if ( !pTaskWin || ( !bIsMDIApp && !bIsIPOwner ) )
        return;

    bool bIsTaskActive = false;

    SfxDispatcher *pActDispatcher = pTaskWin->GetBindings().GetDispatcher_Impl();
    if ( pActDispatcher && !bIsTaskActive )
    {
        if ( this == pActDispatcher )
            bIsTaskActive = true;
    }

    if (bIsTaskActive && eStatBarId != StatusBarId::None && xImp->pFrame)
    {
        // internal frames also may control statusbar
        xImp->pFrame->GetFrame().GetWorkWindow_Impl()->SetStatusBar_Impl(eStatBarId);
    }
}

/** Helper method to execute the outstanding push and pop commands.
*/
void SfxDispatcher::FlushImpl()
{
    SFX_STACK(SfxDispatcher::FlushImpl);

    SAL_INFO("sfx.control", "Flushing dispatcher!");

    xImp->aIdle.Stop();

    xImp->bFlushing = !xImp->bFlushing;
    if ( !xImp->bFlushing )
    {
        xImp->bFlushing = true;
        return;
    }

    SfxApplication *pSfxApp = SfxGetpApp();

    // Re-build the true stack in the first round
    std::deque<SfxToDo_Impl> aToDoCopy;
    bool bModify = false;
    for(std::deque<SfxToDo_Impl>::reverse_iterator i = xImp->aToDoStack.rbegin(); i != xImp->aToDoStack.rend(); ++i)
    {
        bModify = true;

        if(i->bPush)
        {
            // Actually push
            DBG_ASSERT( std::find(xImp->aStack.begin(), xImp->aStack.end(), i->pCluster) == xImp->aStack.end(),
                       "pushed SfxShell already on stack" );
            xImp->aStack.push_back(i->pCluster);
            i->pCluster->SetDisableFlags(xImp->nDisableFlags);

            // Mark the moved shell
            aToDoCopy.push_front(*i);
        }
        else
        {
            // Actually pop
            SfxShell* pPopped = nullptr;
            bool bFound = false;
            do
            {
                DBG_ASSERT( !xImp->aStack.empty(), "popping from empty stack" );
                pPopped = xImp->aStack.back();
                xImp->aStack.pop_back();
                pPopped->SetDisableFlags( SfxDisableFlags::NONE );
                bFound = (pPopped == i->pCluster);

                // Mark the moved Shell
                aToDoCopy.push_front(SfxToDo_Impl(false, i->bDelete, false, *pPopped));
            }
            while(i->bUntil && !bFound);
            DBG_ASSERT( bFound, "wrong SfxShell popped" );
        }
    }
    xImp->aToDoStack.clear();

    // Invalidate bindings, if possible
    if ( !pSfxApp->IsDowning() )
    {
        InvalidateBindings_Impl( bModify );
    }

    xImp->bFlushing = false;
    xImp->bUpdated = false; // not only when bModify, if Doc/Template-Config
    xImp->bFlushed = true;
    SAL_INFO("sfx.control", "Successfully flushed dispatcher!");

    //fdo#70703 FlushImpl may call back into itself so use aToDoCopyStack to talk
    //to outer levels of ourself. If DoActivate_Impl/DoDeactivate_Impl deletes
    //an entry, then they will walk back up aToDoCopyStack and set outer
    //levels's entries to bDeleted
    xImp->aToDoCopyStack.push_back(aToDoCopy);
    std::deque<SfxToDo_Impl>& rToDoCopy = xImp->aToDoCopyStack.back();
    // Activate the Shells and possible delete them in the 2nd round
    for(std::deque<SfxToDo_Impl>::reverse_iterator i = rToDoCopy.rbegin(); i != rToDoCopy.rend(); ++i)
    {
        if (i->bDeleted)
            continue;
        if (!xImp->bActive)
            continue;
        if (i->bPush)
            i->pCluster->DoActivate_Impl(xImp->pFrame, true);
        else
            i->pCluster->DoDeactivate_Impl(xImp->pFrame, true);
    }

    aToDoCopy = xImp->aToDoCopyStack.back();
    xImp->aToDoCopyStack.pop_back();

    for(std::deque<SfxToDo_Impl>::reverse_iterator i = aToDoCopy.rbegin(); i != aToDoCopy.rend(); ++i)
    {
        if (i->bDelete && !i->bDeleted)
        {
            if (!xImp->aToDoCopyStack.empty())
            {
                //fdo#70703 if there is an outer FlushImpl then inform it that
                //we have deleted this cluster
                for (auto & elem : xImp->aToDoCopyStack)
                {
                    for (auto & subelem : elem)
                    {
                        if (subelem.pCluster == i->pCluster)
                            subelem.bDeleted = true;
                    }
                }
            }
            delete i->pCluster;
        }
    }
    bool bAwakeBindings = !aToDoCopy.empty();
    if( bAwakeBindings )
        aToDoCopy.clear();

    // If more changes have occurred on the stack when
    // Activate/Deactivate/Delete:
    if (!xImp->bFlushed)
        // If Push/Pop has been called by someone, then also EnterReg was called!
        FlushImpl();

    if( bAwakeBindings && GetBindings() )
        GetBindings()->DLEAVEREGISTRATIONS();

    for (SfxObjectBars_Impl & rFixedObjBar : xImp->aFixedObjBars)
        rFixedObjBar.eId = ToolbarId::None;

    SAL_INFO("sfx.control", "SfxDispatcher(" << this << ")::Flush() done");
}

/** With this method a filter set, the target slots can be enabled or disabled.
    The passed array must be retained until the destructor or the next
    <SetSlotFilter()>, it is not deleted from the dispatcher, so it can thus be
    static.

    In read-only documents the quasi ReadOnlyDoc Flag of slots can be
    overturned by the use of 'bEnable == 2', so this will be displayed again.
    On the other slots it has no effect.

    // HACK(here should be used an enum) ???
    @param nEnable  1==true: only enable specified slots, disable all other
                    0==false: disable specified slots, first enable all other
    @param nCount Number of SIDs in the following Array
    @param pSIDs sorted Array of 'nCount' SIDs

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
void SfxDispatcher::SetSlotFilter(SfxSlotFilterState nEnable,
        o3tl::span<sal_uInt16 const> pSIDs)
{
#ifdef DBG_UTIL
    // Check Array
    for ( o3tl::span<sal_uInt16 const>::index_type n = 1; n < pSIDs.size(); ++n )
        DBG_ASSERT( pSIDs[n] > pSIDs[n-1], "SetSlotFilter: SIDs not sorted" );
#endif

    xImp->nFilterEnabling = nEnable;
    xImp->pFilterSIDs = pSIDs;

    GetBindings()->InvalidateAll(true);
}

extern "C" {

static int SfxCompareSIDs_Impl(const void* pSmaller, const void* pBigger)
{
    return static_cast<long>(*static_cast<sal_uInt16 const *>(pSmaller)) - static_cast<long>(*static_cast<sal_uInt16 const *>(pBigger));
}

}

/** Searches for 'nSID' in the Filter set by <SetSlotFilter()> and
    returns sal_True, if the SIDis allowed, or sal_False, if it is
    disabled by the Filter.

    @return             0       =>      disabled
                        1       =>      enabled
                        2       =>      enabled even if ReadOnlyDoc
*/
SfxSlotFilterState SfxDispatcher::IsSlotEnabledByFilter_Impl( sal_uInt16 nSID ) const
{
    // no filter?
    if ( xImp->pFilterSIDs.empty() )
        // => all SIDs allowed
        return SfxSlotFilterState::ENABLED;

    // search
    bool bFound = nullptr != bsearch( &nSID, xImp->pFilterSIDs.data(), xImp->pFilterSIDs.size(),
                                sizeof(sal_uInt16), SfxCompareSIDs_Impl );

    // even if ReadOnlyDoc
    if ( SfxSlotFilterState::ENABLED_READONLY == xImp->nFilterEnabling )
        return bFound ? SfxSlotFilterState::ENABLED_READONLY : SfxSlotFilterState::ENABLED;
    // Otherwise after Negative/Positive Filter
    else if ( SfxSlotFilterState::ENABLED == xImp->nFilterEnabling )
        return bFound ? SfxSlotFilterState::ENABLED : SfxSlotFilterState::DISABLED;
    else
        return bFound ? SfxSlotFilterState::DISABLED : SfxSlotFilterState::ENABLED;
}

/** This helper method searches for the <Slot-Server> which currently serves
    the nSlot. As the result, rServe is filled accordingly.

    If known the SfxInterface which is currently served by nSlot can be
    passed along.

    The SfxDispatcher is flushed while searching for nSlot.

    @param nSlot Slot-Id to search for
    @param rServer <SfxSlotServer>-Instance to fill

    @return         true
                    The Slot was found, rServer is valid.

                    false
                    The Slot is currently not served, rServer is invalid.
*/
bool SfxDispatcher::FindServer_(sal_uInt16 nSlot, SfxSlotServer& rServer)
{
    SFX_STACK(SfxDispatcher::FindServer_);

    // Dispatcher locked? (nevertheless let SID_HELP_PI through)
    if ( IsLocked() )
    {
        xImp->bInvalidateOnUnlock = true;
        return false;
    }

    // Count the number of Shells in the linked dispatchers.
    Flush();
    sal_uInt16 nTotCount = xImp->aStack.size();

    // Verb-Slot?
    if (nSlot >= SID_VERB_START && nSlot <= SID_VERB_END)
    {
        for ( sal_uInt16 nShell = 0;; ++nShell )
        {
            SfxShell *pSh = GetShell(nShell);
            if ( pSh == nullptr )
                return false;
            if ( dynamic_cast< const SfxViewShell *>( pSh ) !=  nullptr )
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
    SfxSlotFilterState nSlotEnableMode = SfxSlotFilterState::DISABLED;
    if ( xImp->pFrame )
    {
        nSlotEnableMode = IsSlotEnabledByFilter_Impl( nSlot );
        if ( SfxSlotFilterState::DISABLED == nSlotEnableMode )
            return false;
    }

    // In Quiet-Mode only Parent-Dispatcher
    if ( xImp->bQuiet )
    {
        return false;
    }

    bool bReadOnly = ( SfxSlotFilterState::ENABLED_READONLY != nSlotEnableMode && xImp->bReadOnly );

    // search through all the shells of the chained dispatchers
    // from top to bottom
    sal_uInt16 nFirstShell = 0;
    for ( sal_uInt16 i = nFirstShell; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot(nSlot);

        if ( pSlot && pSlot->nDisableFlags != SfxDisableFlags::NONE &&
             ( static_cast<int>(pSlot->nDisableFlags) & static_cast<int>(pObjShell->GetDisableFlags()) ) != 0 )
            return false;

        if ( pSlot && !( pSlot->nFlags & SfxSlotMode::READONLYDOC ) && bReadOnly )
            return false;

        if ( pSlot )
        {
            // Slot belongs to Container?
            bool bIsContainerSlot = pSlot->IsMode(SfxSlotMode::CONTAINER);
            bool bIsInPlace = xImp->pFrame && xImp->pFrame->GetObjectShell()->IsInPlaceActive();

            // Shell belongs to Server?
            // AppDispatcher or IPFrame-Dispatcher
            bool bIsServerShell = !xImp->pFrame || bIsInPlace;

            // Of course ShellServer-Slots are also executable even when it is
            // executed on a container dispatcher without a IPClient.
            if ( !bIsServerShell )
            {
                SfxViewShell *pViewSh = xImp->pFrame->GetViewShell();
                bIsServerShell = !pViewSh || !pViewSh->GetUIActiveClient();
            }

            // Shell belongs to Container?
            // AppDispatcher or no IPFrameDispatcher
            bool bIsContainerShell = !xImp->pFrame || !bIsInPlace;
            // Shell and Slot match
            if ( !( ( bIsContainerSlot && bIsContainerShell ) ||
                    ( !bIsContainerSlot && bIsServerShell ) ) )
                pSlot = nullptr;
        }

        if ( pSlot )
        {
            rServer.SetSlot(pSlot);
            rServer.SetShellLevel(i);
            return true;
        }
    }

    return false;
}

/** Helper method to obtain the status of the <Slot-Server>s rSvr.
    The required slots IDs (partly converted to Which-IDs of the pool)
    must be present in rstate.

    The SfxDispatcher is flushed before the query.

    @param rSvr Slot-Server to query
    @param rState SfxItemSet to be filled
    @param pRealSlot The actual Slot if possible
*/
bool SfxDispatcher::FillState_(const SfxSlotServer& rSvr, SfxItemSet& rState,
        const SfxSlot* pRealSlot)
{
    SFX_STACK(SfxDispatcher::FillState_);

    const SfxSlot *pSlot = rSvr.GetSlot();
    if ( pSlot && IsLocked() )
    {
        xImp->bInvalidateOnUnlock = true;
        return false;
    }

    if ( pSlot )
    {
        DBG_ASSERT(xImp->bFlushed,
                "Dispatcher not flushed after retrieving slot servers!");
        if (!xImp->bFlushed)
            return false;

        // Determine the object and call the Message of this object
        SfxShell *pSh = GetShell(rSvr.GetShellLevel());
        DBG_ASSERT(pSh, "ObjectShell not found");

        SfxStateFunc pFunc;

        if (pRealSlot)
            pFunc = pRealSlot->GetStateFnc();
        else
            pFunc = pSlot->GetStateFnc();

        pSh->CallState( pFunc, rState );
#ifdef DBG_UTIL
        // To examine the conformity of IDL (SlotMap) and current Items
        if ( rState.Count() )
        {
            SfxInterface *pIF = pSh->GetInterface();
            SfxItemIter aIter( rState );
            for ( const SfxPoolItem *pItem = aIter.FirstItem();
                  pItem;
                  pItem = aIter.NextItem() )
            {
                if ( !IsInvalidItem(pItem) && !pItem->IsVoidItem() )
                {
                    sal_uInt16 nSlotId = rState.GetPool()->GetSlotId(pItem->Which());
                    SAL_INFO_IF(
                        typeid(pItem) != *pIF->GetSlot(nSlotId)->GetType()->Type(),
                        "sfx.control",
                        "item-type unequal to IDL (=> no BASIC) with SID: "
                            << nSlotId << " in " << pIF->GetClassName());
                }
            }
        }
#endif

        return true;
    }

    return false;
}

void SfxDispatcher::ExecutePopup( vcl::Window *pWin, const Point *pPos )
{
    SfxDispatcher &rDisp = *SfxGetpApp()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;
    SfxShell *pSh;

    if ( rDisp.xImp->bQuiet )
        nShLevel = rDisp.xImp->aStack.size();

    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        const OUString& rResName = pSh->GetInterface()->GetPopupMenuName();
        if ( !rResName.isEmpty() )
        {
            rDisp.ExecutePopup( rResName, pWin, pPos );
            return;
        }
    }
}

void SfxDispatcher::ExecutePopup( const OUString& rResName, vcl::Window* pWin, const Point* pPos )
{
    css::uno::Sequence< css::uno::Any > aArgs( 3 );
    aArgs[0] <<= comphelper::makePropertyValue( "Value", rResName );
    aArgs[1] <<= comphelper::makePropertyValue( "Frame", GetFrame()->GetFrame().GetFrameInterface() );
    aArgs[2] <<= comphelper::makePropertyValue( "IsContextMenu", true );

    css::uno::Reference< css::uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    css::uno::Reference< css::frame::XPopupMenuController > xPopupController(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        "com.sun.star.comp.framework.ResourceMenuController", aArgs, xContext ), css::uno::UNO_QUERY );

    css::uno::Reference< css::awt::XPopupMenu > xPopupMenu( xContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.awt.PopupMenu", xContext ), css::uno::UNO_QUERY );

    if ( !xPopupController.is() || !xPopupMenu.is() )
        return;

    vcl::Window* pWindow = pWin ? pWin : xImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
    Point aPos = pPos ? *pPos : pWindow->GetPointerPosPixel();

    css::ui::ContextMenuExecuteEvent aEvent;
    aEvent.SourceWindow = VCLUnoHelper::GetInterface( pWindow );
    aEvent.ExecutePosition.X = aPos.X();
    aEvent.ExecutePosition.Y = aPos.Y();

    xPopupController->setPopupMenu( xPopupMenu );
    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( xPopupMenu );
    PopupMenu* pVCLMenu = static_cast< PopupMenu* >( pAwtMenu->GetMenu() );
    if (comphelper::LibreOfficeKit::isActive())
    {
        boost::property_tree::ptree aMenu = fillPopupMenu(pVCLMenu);
        boost::property_tree::ptree aRoot;
        aRoot.add_child("menu", aMenu);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aRoot, true);
        if (SfxViewShell* pViewShell = xImp->pFrame->GetViewShell())
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CONTEXT_MENU, aStream.str().c_str());
    }
    else
    {
        OUString aMenuURL = "private:resource/popupmenu/" + rResName;
        if (pVCLMenu && GetFrame()->GetViewShell()->TryContextMenuInterception(*pVCLMenu, aMenuURL, aEvent))
        {
            pVCLMenu->Execute(pWindow, aPos);
        }
    }

    css::uno::Reference< css::lang::XComponent > xComponent( xPopupController, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
}

/** With this method the SfxDispatcher can be locked and released. A locked
    SfxDispatcher does not perform <SfxRequest>s and does no longer provide
    status information. It behaves as if all the slots were disabled.
*/
void SfxDispatcher::Lock( bool bLock )
{
    SfxBindings* pBindings = GetBindings();
    if ( !bLock && xImp->bLocked && xImp->bInvalidateOnUnlock )
    {
        if ( pBindings )
            pBindings->InvalidateAll(true);
        xImp->bInvalidateOnUnlock = false;
    }
    else if ( pBindings )
        pBindings->InvalidateAll(false);
    xImp->bLocked = bLock;
    if ( !bLock )
    {
        for(size_t i = 0; i < xImp->aReqArr.size(); ++i)
            xImp->xPoster->Post(std::move(xImp->aReqArr[i]));
        xImp->aReqArr.clear();
    }
}

ToolbarId SfxDispatcher::GetObjectBarId( sal_uInt16 nPos ) const
{
    return xImp->aObjBars[nPos].eId;
}

void SfxDispatcher::HideUI( bool bHide )
{
    bool bWasHidden = xImp->bNoUI;
    xImp->bNoUI = bHide;
    if ( xImp->pFrame )
    {
        SfxViewFrame* pTop = xImp->pFrame->GetTopViewFrame();
        if ( pTop && pTop->GetBindings().GetDispatcher() == this )
        {
            SfxFrame& rFrame = pTop->GetFrame();
            if ( rFrame.IsMenuBarOn_Impl() )
            {
                css::uno::Reference < css::beans::XPropertySet > xPropSet( rFrame.GetFrameInterface(), css::uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
                    css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                        xLayoutManager->setVisible( !bHide );
                }
            }
        }
    }

    if ( bHide != bWasHidden )
        Update_Impl( true );
}

void SfxDispatcher::SetReadOnly_Impl( bool bOn )
{
    xImp->bReadOnly = bOn;
}

bool SfxDispatcher::GetReadOnly_Impl() const
{
    return xImp->bReadOnly;
}

/** With 'bOn' the Dispatcher is quasi dead and transfers everything to the
    Parent-Dispatcher.
*/
void SfxDispatcher::SetQuietMode_Impl( bool bOn )
{
    xImp->bQuiet = bOn;
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->InvalidateAll(true);
}

SfxItemState SfxDispatcher::QueryState( sal_uInt16 nSlot, const SfxPoolItem* &rpState )
{
    SfxShell *pShell = nullptr;
    const SfxSlot *pSlot = nullptr;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, false, true ) )
    {
        rpState = pShell->GetSlotState(nSlot);
        if ( !rpState )
            return SfxItemState::DISABLED;
        else
            return SfxItemState::DEFAULT;
    }

    return SfxItemState::DISABLED;
}

SfxItemState SfxDispatcher::QueryState( sal_uInt16 nSID, css::uno::Any& rAny )
{
    SfxShell *pShell = nullptr;
    const SfxSlot *pSlot = nullptr;
    if ( GetShellAndSlot_Impl( nSID, &pShell, &pSlot, false, true ) )
    {
        const SfxPoolItem* pItem( nullptr );

        pItem = pShell->GetSlotState( nSID );
        if ( !pItem )
            return SfxItemState::DISABLED;
        else
        {
            css::uno::Any aState;
            if ( !pItem->IsVoidItem() )
            {
                sal_uInt16 nSubId( 0 );
                SfxItemPool& rPool = pShell->GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( nSID );
                if ( rPool.GetMetric( nWhich ) == MapUnit::MapTwip )
                    nSubId |= CONVERT_TWIPS;
                pItem->QueryValue( aState, static_cast<sal_uInt8>(nSubId) );
            }
            rAny = aState;

            return SfxItemState::DEFAULT;
        }
    }

    return SfxItemState::DISABLED;
}

bool SfxDispatcher::IsReadOnlyShell_Impl( sal_uInt16 nShell ) const
{
    sal_uInt16 nShellCount = xImp->aStack.size();
    if ( nShell < nShellCount )
    {
        SfxShell* pShell = *( xImp->aStack.rbegin() + nShell );
        if( dynamic_cast< const SfxModule *>( pShell ) != nullptr || dynamic_cast< const SfxApplication *>( pShell ) != nullptr || dynamic_cast< const SfxViewFrame *>( pShell ) !=  nullptr )
            return false;
        else
            return xImp->bReadOnly;
    }
    return true;
}

void SfxDispatcher::RemoveShell_Impl( SfxShell& rShell )
{
    Flush();

    sal_uInt16 nCount = xImp->aStack.size();
    for ( sal_uInt16 n=0; n<nCount; ++n )
    {
        if ( xImp->aStack[n] == &rShell )
        {
            xImp->aStack.erase( xImp->aStack.begin() + n );
            rShell.SetDisableFlags( SfxDisableFlags::NONE );
            rShell.DoDeactivate_Impl(xImp->pFrame, true);
            break;
        }
    }

    if ( !SfxGetpApp()->IsDowning() )
    {
        xImp->bUpdated = false;
        InvalidateBindings_Impl(true);
    }
}

void SfxDispatcher::InvalidateBindings_Impl( bool bModify )
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
        if ( pDisp == this )
        {
            GetBindings()->InvalidateAll( bModify );
        }
    }
}

bool SfxDispatcher::IsUpdated_Impl() const
{
    return xImp->bUpdated;
}

void SfxDispatcher::SetDisableFlags( SfxDisableFlags nFlags )
{
    xImp->nDisableFlags = nFlags;
    for ( SfxShellStack_Impl::reverse_iterator it = xImp->aStack.rbegin(); it != xImp->aStack.rend(); ++it )
        (*it)->SetDisableFlags( nFlags );
}

SfxDisableFlags SfxDispatcher::GetDisableFlags() const
{
    return xImp->nDisableFlags;
}

SfxModule* SfxDispatcher::GetModule() const
{
    for ( sal_uInt16 nShell = 0;; ++nShell )
    {
        SfxShell *pSh = GetShell(nShell);
        if ( pSh == nullptr )
            return nullptr;
        if ( dynamic_cast< const SfxModule *>( pSh ) !=  nullptr )
            return static_cast<SfxModule*>(pSh);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
