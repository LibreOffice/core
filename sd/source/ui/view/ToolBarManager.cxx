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


#include "ToolBarManager.hxx"

#include "DrawViewShell.hxx"
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/link.hxx>

#include <map>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

using namespace sd;

class ToolBarRules;

/** Lock of the frame::XLayoutManager.
*/
class LayouterLock
{
public:
    LayouterLock (const Reference<frame::XLayoutManager>& rxLayouter);
    ~LayouterLock (void);
private:
    Reference<frame::XLayoutManager> mxLayouter;
};


typedef ::std::vector<OUString> NameList;

/** Store a list of tool bars for each of the tool bar groups.  From
    this the list of requested tool bars is built.
*/
class ToolBarList
{
public:
    ToolBarList (void);

    void ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup);
    void AddToolBar (sd::ToolBarManager::ToolBarGroup eGroup, const OUString& rsName);
    bool RemoveToolBar (sd::ToolBarManager::ToolBarGroup eGroup, const OUString& rsName);

    void GetToolBarsToActivate (NameList& rToolBars) const;
    void GetToolBarsToDeactivate (NameList& rToolBars) const;

    void MarkToolBarAsActive (const OUString& rsName);
    void MarkToolBarAsNotActive (const OUString& rsName);
    void MarkAllToolBarsAsNotActive (void);

private:
    typedef ::std::map<sd::ToolBarManager::ToolBarGroup,NameList> Groups;
    Groups maGroups;
    NameList maActiveToolBars;

    void MakeRequestedToolBarList (NameList& rToolBars) const;
};




/** Manage tool bars that are implemented as sub shells of a view shell.
    The typical procedure of updating the sub shells of a view shell is to
    rebuild a list of sub shells that the caller would like to have active.
    The methods ClearGroup() and AddShellId() allow the caller to do that.  A
    final call to UpdateShells() activates the requested shells that are not
    active and deactivates the active shells that are not requested .

    This is done by maintaining two lists.  One (the current list)
    reflects the current state.  The other (the requested list) contains the
    currently requested shells.  UpdateShells() makes the requested
    list the current list and clears the current list.

    Each shell belongs to one group.  Different groups can be modified
    separately.
*/
class ToolBarShellList
{
public:
    /** Create a new object with an empty current list and an empty
        requested list.
    */
    ToolBarShellList (void);

    /** Remove all shells from a group.  Calling this method should normally
        not be necessary because after the construction or after a call to
        UpdateShells() the requested list is empty.
        @param eGroup
            The group to clear. Shells in other groups are not modified.
    */
    void ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup);

    /** Add a shell.  When the specified shell has alreadt been requested
        for another group then it is moved to this group.
        @param eGroup
            The group to which to add the shell.
        @param nId
            The id of the shell to add.
    */
    void AddShellId (sd::ToolBarManager::ToolBarGroup eGroup, sd::ShellId nId);

    /** Releasing all shells means that the given ToolBarRules object is
        informed that every shell mananged by the called ToolBarShellList is
        about to be removed and that the associated framework tool bars can
        be removed as well.  The caller still has to call UpdateShells().
    */
    void ReleaseAllShells (ToolBarRules& rRules);

    /** The requested list is made the current list by activating  all
        shells in the requested list and by deactivating the shells in the
        current list that are not in the requested list.
        @param pMainViewShell
            The shells that are activated or deactivated are sub shells of
            this view shell.
        @param rManager
            This ViewShellManager is used to activate or deactivate shells.
    */
    void UpdateShells (
        const ::boost::shared_ptr<ViewShell>& rpMainViewShell,
        const ::boost::shared_ptr<ViewShellManager>& rpManager);

private:
    class ShellDescriptor
    {public:
        ShellDescriptor (ShellId nId,sd::ToolBarManager::ToolBarGroup eGroup);
        ShellId mnId;
        sd::ToolBarManager::ToolBarGroup meGroup;
        friend bool operator<(const ShellDescriptor& r1, const ShellDescriptor& r2)
        { return r1.mnId < r2.mnId; }
    };

    /** The requested list of tool bar shells that will be active after the
        next call to UpdateShells().
    */
    typedef ::std::set<ShellDescriptor> GroupedShellList;
    GroupedShellList maNewList;

    /** The list of tool bar shells that are currently on the shell stack.
        Using a GroupedShellList is not strictly necessary but it makes
        things easier and does not waste too much memory.
    */
    GroupedShellList maCurrentList;
};




/** This class concentrates the knowledge about when to show what tool bars
    in one place.
*/
class ToolBarRules
{
public:
    ToolBarRules (
        const ::boost::shared_ptr<ToolBarManager>& rpToolBarManager,
        const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager);

    /** This method calls MainViewShellChanged() and SelectionHasChanged()
        for the current main view shell and its view.
    */
    void Update (ViewShellBase& rBase);

    /** Reset all tool bars in all groups and add tool bars and tool bar
        shells to the TBG_PERMANENT group for the specified ViewShell type.
    */
    void MainViewShellChanged (ViewShell::ShellType nShellType);

    /** Reset all tool bars in all groups and add tool bars and tool bar
        shells to the TBG_PERMANENT group for the specified ViewShell.
    */
    void MainViewShellChanged (const ViewShell& rMainViewShell);

    /** Reset all tool bars in the TBG_FUNCTION group and add tool bars and tool bar
        shells to this group for the current selection.
    */
    void SelectionHasChanged (
        const ::sd::ViewShell& rViewShell,
        const SdrView& rView);

    /** Add a tool bar for the specified tool bar shell.
    */
    void SubShellAdded (
        ::sd::ToolBarManager::ToolBarGroup eGroup,
        sd::ShellId nShellId);

    /** Remove a tool bar for the specified tool bar shell.
    */
    void SubShellRemoved (
        ::sd::ToolBarManager::ToolBarGroup eGroup,
        sd::ShellId nShellId);

private:
    ::boost::shared_ptr<ToolBarManager> mpToolBarManager;
    ::boost::shared_ptr<ViewShellManager> mpViewShellManager;
};

} // end of anonymous namespace




namespace sd {

//===== ToolBarManager::Implementation ========================================

class ToolBarManager::Implementation
{
public:
    /** This constructor takes three arguments even though the
        ToolBarManager could be taken from the ViewShellBase.  This is so to
        state explicitly which information has to be present when this
        constructor is called.  The ViewShellBase may not have been fully
        initialized at this point and must not be asked for this values.
    */
    Implementation (
        ViewShellBase& rBase,
        const ::boost::shared_ptr<sd::tools::EventMultiplexer>& rpMultiplexer,
        const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager,
        const ::boost::shared_ptr<ToolBarManager>& rpToolBarManager);
    ~Implementation (void);

    void SetValid (bool bValid);

    void ResetToolBars (ToolBarGroup eGroup);
    void ResetAllToolBars (void);
    void AddToolBar (ToolBarGroup eGroup, const OUString& rsToolBarName);
    void AddToolBarShell (ToolBarGroup eGroup, ShellId nToolBarId);
    void RemoveToolBar (ToolBarGroup eGroup, const OUString& rsToolBarName);

    /** Release all tool bar shells and the associated framework tool bars.
        Typically called when the main view shell is being replaced by
        another, all tool bar shells are released.  In that process the
        shells are destroyed anyway and without calling this method they
        would still be referenced.
    */
    void ReleaseAllToolBarShells (void);

    void ToolBarsDestroyed(void);

    void RequestUpdate (void);

    void PreUpdate (void);
    void PostUpdate (void);
    /** Tell the XLayoutManager about the tool bars that we would like to be
        shown.
        @param rpLayouterLock
            This typically is the mpSynchronousLayouterLock that is used in
            this method and that is either released at its end or assigned
            to mpAsynchronousLock in order to be unlocked later.
    */
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void Update (::std::auto_ptr<LayouterLock> pLayouterLock);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    class UpdateLockImplementation
    {
    public:
        UpdateLockImplementation (Implementation& rImplementation)
            : mrImplementation(rImplementation) { mrImplementation.LockUpdate();  }
        ~UpdateLockImplementation (void) { mrImplementation.UnlockUpdate(); }
    private:
        Implementation& mrImplementation;
    };

    void LockViewShellManager (void);
    void LockUpdate (void);
    void UnlockUpdate (void);

    ToolBarRules& GetToolBarRules (void);

private:
    const static OUString msToolBarResourcePrefix;

    mutable ::osl::Mutex maMutex;
    ViewShellBase& mrBase;
    ::boost::shared_ptr<sd::tools::EventMultiplexer> mpEventMultiplexer;
    bool mbIsValid;
    ToolBarList maToolBarList;
    ToolBarShellList maToolBarShellList;
    Reference<frame::XLayoutManager> mxLayouter;
    sal_Int32 mnLockCount;
    bool mbPreUpdatePending;
    bool mbPostUpdatePending;
    /** The layouter locks manage the locking of the XLayoutManager.  The
        lock() and unlock() functions are not called directly because the
        (final) unlocking  is usually done asynchronously *after* the
        list of requested toolbars is updated.
    */
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<LayouterLock> mpSynchronousLayouterLock;
    ::std::auto_ptr<LayouterLock> mpAsynchronousLayouterLock;
    ::std::auto_ptr<ViewShellManager::UpdateLock> mpViewShellManagerLock;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    sal_uLong mnPendingUpdateCall;
    sal_uLong mnPendingSetValidCall;
    ToolBarRules maToolBarRules;

    OUString GetToolBarResourceName (const OUString& rsBaseName) const;
    bool CheckPlugInMode (const OUString& rsName) const;

    DECL_LINK(UpdateCallback, void *);
    DECL_LINK(EventMultiplexerCallback, sd::tools::EventMultiplexerEvent*);
    DECL_LINK(SetValidCallback,void*);
};



//===== ToolBarManager ========================================================

const OUString ToolBarManager::msToolBar("toolbar");
const OUString ToolBarManager::msOptionsToolBar("optionsbar");
const OUString ToolBarManager::msCommonTaskToolBar("commontaskbar");
const OUString ToolBarManager::msViewerToolBar("viewerbar");
const OUString ToolBarManager::msSlideSorterToolBar("slideviewtoolbar");
const OUString ToolBarManager::msSlideSorterObjectBar("slideviewobjectbar");
const OUString ToolBarManager::msOutlineToolBar("outlinetoolbar");
const OUString ToolBarManager::msMasterViewToolBar("masterviewtoolbar");
const OUString ToolBarManager::msDrawingObjectToolBar("drawingobjectbar");
const OUString ToolBarManager::msGluePointsToolBar("gluepointsobjectbar");
const OUString ToolBarManager::msTextObjectBar("textobjectbar");
const OUString ToolBarManager::msBezierObjectBar("bezierobjectbar");
const OUString ToolBarManager::msGraphicObjectBar("graphicobjectbar");
const OUString ToolBarManager::msMediaObjectBar("mediaobjectbar");
const OUString ToolBarManager::msTableObjectBar("tableobjectbar");


::boost::shared_ptr<ToolBarManager> ToolBarManager::Create (
    ViewShellBase& rBase,
    const ::boost::shared_ptr<sd::tools::EventMultiplexer>& rpMultiplexer,
    const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager)
{
    ::boost::shared_ptr<ToolBarManager> pManager (new ToolBarManager());
    pManager->mpImpl.reset(
        new Implementation(rBase,rpMultiplexer,rpViewShellManager,pManager));
    return pManager;
}




ToolBarManager::ToolBarManager (void)
        : mpImpl()
{
}




ToolBarManager::~ToolBarManager (void)
{
}




void ToolBarManager::Shutdown (void)
{
    if (mpImpl.get() != NULL)
        mpImpl.reset();
}




void ToolBarManager::ResetToolBars (ToolBarGroup eGroup)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetToolBars(eGroup);
    }
}




void ToolBarManager::ResetAllToolBars (void)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetAllToolBars();
    }
}




void ToolBarManager::AddToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->AddToolBar(eGroup,rsToolBarName);
    }
}




void ToolBarManager::AddToolBarShell (
    ToolBarGroup eGroup,
    ShellId nToolBarId)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->AddToolBarShell(eGroup,nToolBarId);
    }
}




void ToolBarManager::RemoveToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->RemoveToolBar(eGroup,rsToolBarName);
    }
}




void ToolBarManager::SetToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetToolBars(eGroup);
        mpImpl->AddToolBar(eGroup,rsToolBarName);
    }
}




void ToolBarManager::SetToolBarShell (
    ToolBarGroup eGroup,
    ShellId nToolBarId)
{
    if (mpImpl.get() != NULL)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetToolBars(eGroup);
        mpImpl->AddToolBarShell(eGroup,nToolBarId);
    }
}




void ToolBarManager::PreUpdate (void)
{
    if (mpImpl.get()!=NULL)
        mpImpl->PreUpdate();
}




void ToolBarManager::RequestUpdate (void)
{
    if (mpImpl.get()!=NULL)
        mpImpl->RequestUpdate();
}




void ToolBarManager::LockViewShellManager (void)
{
    if (mpImpl.get() != NULL)
        mpImpl->LockViewShellManager();
}




void ToolBarManager::LockUpdate (void)
{
    if (mpImpl.get()!=NULL)
        mpImpl->LockUpdate();
}




void ToolBarManager::UnlockUpdate (void)
{
    if (mpImpl.get()!=NULL)
        mpImpl->UnlockUpdate();
}




void ToolBarManager::MainViewShellChanged (ViewShell::ShellType nShellType)
{
    if (mpImpl.get() != NULL)
    {
        mpImpl->ReleaseAllToolBarShells();
        mpImpl->GetToolBarRules().MainViewShellChanged(nShellType);
    }
}




void ToolBarManager::MainViewShellChanged (const ViewShell& rMainViewShell)
{
    if (mpImpl.get() != NULL)
    {
        mpImpl->ReleaseAllToolBarShells();
        mpImpl->GetToolBarRules().MainViewShellChanged(rMainViewShell);
    }
}




void ToolBarManager::SelectionHasChanged (
    const ViewShell& rViewShell,
    const SdrView& rView)
{
    if (mpImpl.get() != NULL)
        mpImpl->GetToolBarRules().SelectionHasChanged(rViewShell,rView);
}


void ToolBarManager::ToolBarsDestroyed(void)
{
    if (mpImpl.get() != NULL)
        mpImpl->ToolBarsDestroyed();
}


//===== ToolBarManager::Implementation =======================================

const OUString ToolBarManager::Implementation::msToolBarResourcePrefix("private:resource/toolbar/");

ToolBarManager::Implementation::Implementation (
    ViewShellBase& rBase,
    const ::boost::shared_ptr<sd::tools::EventMultiplexer>& rpMultiplexer,
    const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager,
    const ::boost::shared_ptr<ToolBarManager>& rpToolBarManager)
    : maMutex(),
      mrBase(rBase),
      mpEventMultiplexer(rpMultiplexer),
      mbIsValid(false),
      maToolBarList(),
      maToolBarShellList(),
      mxLayouter(NULL),
      mnLockCount(0),
      mbPreUpdatePending(false),
      mbPostUpdatePending(false),
      mpSynchronousLayouterLock(),
      mpAsynchronousLayouterLock(),
      mpViewShellManagerLock(),
      mnPendingUpdateCall(0),
      mnPendingSetValidCall(0),
      maToolBarRules(rpToolBarManager,rpViewShellManager)
{
    Link aLink (LINK(this,ToolBarManager::Implementation,EventMultiplexerCallback));
    mpEventMultiplexer->AddEventListener(
        aLink,
        tools::EventMultiplexerEvent::EID_CONTROLLER_ATTACHED
        | tools::EventMultiplexerEvent::EID_CONTROLLER_DETACHED
        | tools::EventMultiplexerEvent::EID_PANE_MANAGER_DYING);
}



/** The order of statements is important.
    First unregister listeners, which may post user events.
    Then remove pending user events.
*/
ToolBarManager::Implementation::~Implementation (void)
{
    // Unregister at broadcasters.
    Link aLink (LINK(this,ToolBarManager::Implementation,EventMultiplexerCallback));
    mpEventMultiplexer->RemoveEventListener(aLink);

    // Abort pending user calls.
    if (mnPendingUpdateCall != 0)
        Application::RemoveUserEvent(mnPendingUpdateCall);
    if (mnPendingSetValidCall != 0)
        Application::RemoveUserEvent(mnPendingSetValidCall);
}


void ToolBarManager::Implementation::ToolBarsDestroyed(void)
{
    maToolBarList.MarkAllToolBarsAsNotActive();
}


void ToolBarManager::Implementation::SetValid (bool bValid)
{
    ::osl::MutexGuard aGuard(maMutex);

    if (mbIsValid != bValid)
    {
        UpdateLockImplementation aUpdateLock (*this);

        mbIsValid = bValid;
        if (mbIsValid)
        {
            Reference<frame::XFrame> xFrame;
            if (mrBase.GetViewFrame() != NULL)
                xFrame = mrBase.GetViewFrame()->GetFrame().GetFrameInterface();
            try
            {
                Reference<beans::XPropertySet> xFrameProperties (xFrame, UNO_QUERY_THROW);
                Any aValue (xFrameProperties->getPropertyValue("LayoutManager"));
                aValue >>= mxLayouter;
            }
            catch (const RuntimeException&)
            {
            }

            GetToolBarRules().Update(mrBase);
        }
        else
        {
            ResetAllToolBars();
            mxLayouter = NULL;
        }
    }
}




void ToolBarManager::Implementation::ResetToolBars (ToolBarGroup eGroup)
{
    ::osl::MutexGuard aGuard(maMutex);

    maToolBarList.ClearGroup(eGroup);
    maToolBarShellList.ClearGroup(eGroup);

    mbPreUpdatePending = true;
}




void ToolBarManager::Implementation::ResetAllToolBars (void)
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": resetting all tool bars");
    for (int i=TBG__FIRST; i<=TBG__LAST; ++i)
        ResetToolBars((ToolBarGroup)i);
}




void ToolBarManager::Implementation::AddToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    ::osl::MutexGuard aGuard(maMutex);

    if (CheckPlugInMode(rsToolBarName))
    {
        maToolBarList.AddToolBar(eGroup,rsToolBarName);

        mbPostUpdatePending = true;
        if (mnLockCount == 0)
            PostUpdate();
    }
}




void ToolBarManager::Implementation::RemoveToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    ::osl::MutexGuard aGuard(maMutex);

    if (maToolBarList.RemoveToolBar(eGroup,rsToolBarName))
    {
        mbPreUpdatePending = true;
        if (mnLockCount == 0)
            PreUpdate();
    }
}




void ToolBarManager::Implementation::AddToolBarShell (
    ToolBarGroup eGroup,
    ShellId nToolBarId)
{
    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();
    if (pMainViewShell != NULL)
    {
        maToolBarShellList.AddShellId(eGroup,nToolBarId);
        GetToolBarRules().SubShellAdded(eGroup, nToolBarId);
    }
}




void ToolBarManager::Implementation::ReleaseAllToolBarShells (void)
{
    maToolBarShellList.ReleaseAllShells(GetToolBarRules());
    maToolBarShellList.UpdateShells(mrBase.GetMainViewShell(), mrBase.GetViewShellManager());
}




void ToolBarManager::Implementation::RequestUpdate (void)
{
    if (mnPendingUpdateCall == 0)
    {
        mnPendingUpdateCall = Application::PostUserEvent(
            LINK(this,ToolBarManager::Implementation,UpdateCallback));
    }
}




void ToolBarManager::Implementation::PreUpdate (void)
{
    ::osl::MutexGuard aGuard(maMutex);

    if (mbIsValid
        && mbPreUpdatePending
        && mxLayouter.is())
    {
        mbPreUpdatePending = false;

        SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PreUpdate [");

        // Get the list of tool bars that are not used anymore and are to be
        // deactivated.
        NameList aToolBars;
        maToolBarList.GetToolBarsToDeactivate(aToolBars);

        // Turn off the tool bars.
        NameList::const_iterator iToolBar;
        for (iToolBar=aToolBars.begin(); iToolBar!=aToolBars.end(); ++iToolBar)
        {
            OUString sFullName (GetToolBarResourceName(*iToolBar));
            SAL_INFO("sd.view", OSL_THIS_FUNC << ":    turning off tool bar " <<
                OUStringToOString(sFullName, RTL_TEXTENCODING_UTF8).getStr());
            mxLayouter->destroyElement(sFullName);
            maToolBarList.MarkToolBarAsNotActive(*iToolBar);
        }

        SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PreUpdate ]");
    }
}




void ToolBarManager::Implementation::PostUpdate (void)
{
    ::osl::MutexGuard aGuard(maMutex);

    if (mbIsValid
        && mbPostUpdatePending
        && mxLayouter.is())
    {
        mbPostUpdatePending = false;

        // Create the list of requested tool bars.
        NameList aToolBars;
        maToolBarList.GetToolBarsToActivate(aToolBars);

        SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PostUpdate [");

        // Turn on the tool bars that are visible in the new context.
        NameList::const_iterator iToolBar;
        for (iToolBar=aToolBars.begin(); iToolBar!=aToolBars.end(); ++iToolBar)
        {
            OUString sFullName (GetToolBarResourceName(*iToolBar));
            SAL_INFO("sd.view", OSL_THIS_FUNC << ":    turning on tool bar " <<
                OUStringToOString(sFullName, RTL_TEXTENCODING_UTF8).getStr());
            mxLayouter->requestElement(sFullName);
            maToolBarList.MarkToolBarAsActive(*iToolBar);
        }

        SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PostUpdate ]");
    }
}




void ToolBarManager::Implementation::LockViewShellManager (void)
{
    if (mpViewShellManagerLock.get() == NULL)
        mpViewShellManagerLock.reset(
            new ViewShellManager::UpdateLock(mrBase.GetViewShellManager()));
}




void ToolBarManager::Implementation::LockUpdate (void)
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": LockUpdate " << mnLockCount);
    ::osl::MutexGuard aGuard(maMutex);

    DBG_ASSERT(mnLockCount<100, "ToolBarManager lock count unusually high");
    if (mnLockCount == 0)
    {
        OSL_ASSERT(mpSynchronousLayouterLock.get()==NULL);

        mpSynchronousLayouterLock.reset(new LayouterLock(mxLayouter));
    }
    ++mnLockCount;
}




void ToolBarManager::Implementation::UnlockUpdate (void)
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": UnlockUpdate " << mnLockCount);
    ::osl::MutexGuard aGuard(maMutex);

    OSL_ASSERT(mnLockCount>0);
    --mnLockCount;
    if (mnLockCount == 0)
    {
        Update(mpSynchronousLayouterLock);
    }
}



SAL_WNODEPRECATED_DECLARATIONS_PUSH
void ToolBarManager::Implementation::Update (
    ::std::auto_ptr<LayouterLock> pLocalLayouterLock)
{
    // When the lock is released and there are pending changes to the set of
    // tool bars then update this set now.
    if (mnLockCount == 0)
    {
        // During ceation of ViewShellBase we may have the situation that
        // the controller has already been created and attached to the frame
        // but that the ToolBarManager has not yet completed its
        // initialization (by initializing the mxLayouter member.)  We do
        // this here so that we do not have to wait for the next Update()
        // call to show the tool bars.
        if (mnPendingSetValidCall != 0)
        {
            Application::RemoveUserEvent(mnPendingSetValidCall);
            mnPendingSetValidCall = 0;
            SetValid(true);
        }

        if (mbIsValid && mxLayouter.is() && (mbPreUpdatePending || mbPostUpdatePending))
        {
            // 1) Release UNO tool bars that are not longer used.  Do this
            // now so that they are not updated when the SFX shell stack is
            // modified.
            if (mbPreUpdatePending)
                PreUpdate();

            // 2) Update the requested shells that represent tool bar
            // functionality. Those that are not used anymore are
            // deactivated now.  Those that are missing are activated in the
            // next step together with the view shells.
            if (mpViewShellManagerLock.get() == NULL)
                mpViewShellManagerLock.reset(
                    new ViewShellManager::UpdateLock(mrBase.GetViewShellManager()));
            maToolBarShellList.UpdateShells(
                mrBase.GetMainViewShell(),
                mrBase.GetViewShellManager());

            // 3) Unlock the ViewShellManager::UpdateLock.  This updates the
            // shell stack.  We have to be carfull here.  The deletion of
            // the lock may end in a synchronous call to LockUpdate(). When
            // at this time the lock has been deleted but the auto_ptr has
            // not yet been reset then the lock is deleted a second time.
            ViewShellManager::UpdateLock* pLock = mpViewShellManagerLock.release();
            delete pLock;

            // 4) Make the UNO tool bars visible.  The outstanding call to
            // PostUpdate() is done via PostUserEvent() so that it is
            // guaranteed to be executed when the SFX shell stack has been
            // updated (under the assumption that our lock to the
            // ViewShellManager was the only one open.  If that is not the
            // case then all should still be well but not as fast.)
            //
            // Note that the lock count may have been increased since
            // entering this method.  In that case one of the next
            // UnlockUpdate() calls will post the UpdateCallback.
            if (mnPendingUpdateCall==0 && mnLockCount==0)
            {
                mpAsynchronousLayouterLock = pLocalLayouterLock;
                mnPendingUpdateCall = Application::PostUserEvent(
                    LINK(this,ToolBarManager::Implementation,UpdateCallback));
            }
        }
        else
        {
            //do this in two steps, first clear mpViewShellManagerLock to be NULL
            ViewShellManager::UpdateLock* pLock = mpViewShellManagerLock.release();
            //now delete the lock so reentry to this method triggered by this
            //delete will encounter an empty mpViewShellManagerLock
            delete pLock;
            pLocalLayouterLock.reset();
        }
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP




ToolBarRules& ToolBarManager::Implementation::GetToolBarRules (void)
{
    return maToolBarRules;
}




IMPL_LINK_NOARG(ToolBarManager::Implementation, UpdateCallback)
{
    mnPendingUpdateCall = 0;
    if (mnLockCount == 0)
    {
        if (mbPreUpdatePending)
            PreUpdate();
        if (mbPostUpdatePending)
            PostUpdate();
        if (mbIsValid && mxLayouter.is())
            mpAsynchronousLayouterLock.reset();
    }
    return 0;
}




IMPL_LINK(ToolBarManager::Implementation,EventMultiplexerCallback,
    sd::tools::EventMultiplexerEvent*,pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->meEventId)
        {
            case tools::EventMultiplexerEvent::EID_CONTROLLER_ATTACHED:
                if (mnPendingSetValidCall == 0)
                    mnPendingSetValidCall
                        = Application::PostUserEvent(LINK(this,Implementation,SetValidCallback));
                break;

            case tools::EventMultiplexerEvent::EID_CONTROLLER_DETACHED:
                SetValid(false);
                break;

            case tools::EventMultiplexerEvent::EID_PANE_MANAGER_DYING:
                SetValid(false);
                break;
        }
    }
    return 0;
}




IMPL_LINK_NOARG(ToolBarManager::Implementation, SetValidCallback)
{
    mnPendingSetValidCall = 0;
    SetValid(true);
    return 0;
}





OUString ToolBarManager::Implementation::GetToolBarResourceName (
    const OUString& rsBaseName) const
{
    OUString sToolBarName (msToolBarResourcePrefix);
    sToolBarName += rsBaseName;
    return sToolBarName;
}




bool ToolBarManager::Implementation::CheckPlugInMode (const OUString& rsName) const
{
    bool bValid (false);

    // Determine the plug in mode.
    bool bIsPlugInMode (false);
    do
    {
        SfxObjectShell* pObjectShell = mrBase.GetObjectShell();
        if (pObjectShell == NULL)
            break;

        SfxMedium* pMedium = pObjectShell->GetMedium();
        if (pMedium == NULL)
            break;

        SFX_ITEMSET_ARG(pMedium->GetItemSet(),pViewOnlyItem,SfxBoolItem,SID_VIEWONLY,sal_False);
        if (pViewOnlyItem == NULL)
            break;

        bIsPlugInMode = pViewOnlyItem->GetValue();
    }
    while (false);

    if (rsName.equals(msViewerToolBar))
        bValid = bIsPlugInMode;
    else
        bValid = ! bIsPlugInMode;

    return bValid;
}




} // end of namespace sd




namespace {

using namespace ::sd;

//===== LayouterLock ==========================================================

LayouterLock::LayouterLock (const Reference<frame::XLayoutManager>& rxLayouter)
    : mxLayouter(rxLayouter)
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": LayouterLock " << (mxLayouter.is() ? 1 :0));
    if (mxLayouter.is())
        mxLayouter->lock();
}




LayouterLock::~LayouterLock (void)
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": ~LayouterLock " << (mxLayouter.is() ? 1 :0));
    if (mxLayouter.is())
        mxLayouter->unlock();
}




//===== ToolBarRules ==========================================================

ToolBarRules::ToolBarRules (
    const ::boost::shared_ptr<sd::ToolBarManager>& rpToolBarManager,
    const ::boost::shared_ptr<sd::ViewShellManager>& rpViewShellManager)
    : mpToolBarManager(rpToolBarManager),
      mpViewShellManager(rpViewShellManager)
{
}




void ToolBarRules::Update (ViewShellBase& rBase)
{
    ViewShell* pMainViewShell = rBase.GetMainViewShell().get();
    if (pMainViewShell != NULL)
    {
        MainViewShellChanged(pMainViewShell->GetShellType());
        if (pMainViewShell->GetView())
            SelectionHasChanged (*pMainViewShell, *pMainViewShell->GetView());
    }
    else
        MainViewShellChanged(ViewShell::ST_NONE);
}




void ToolBarRules::MainViewShellChanged (ViewShell::ShellType nShellType)
{
    ::sd::ToolBarManager::UpdateLock aToolBarManagerLock (mpToolBarManager);
    ::sd::ViewShellManager::UpdateLock aViewShellManagerLock (mpViewShellManager);

    mpToolBarManager->ResetAllToolBars();

    switch(nShellType)
    {
        case ::sd::ViewShell::ST_IMPRESS:
        case ::sd::ViewShell::ST_NOTES:
        case ::sd::ViewShell::ST_HANDOUT:
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msOptionsToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msCommonTaskToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msViewerToolBar);
            break;

        case ::sd::ViewShell::ST_DRAW:
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msOptionsToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msViewerToolBar);
            break;

        case ViewShell::ST_OUTLINE:
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msOutlineToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msViewerToolBar);
            mpToolBarManager->AddToolBarShell(
                ToolBarManager::TBG_PERMANENT, RID_DRAW_TEXT_TOOLBOX);
            break;

        case ViewShell::ST_SLIDE_SORTER:
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msViewerToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msSlideSorterToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::TBG_PERMANENT,
                ToolBarManager::msSlideSorterObjectBar);
            break;

        case ViewShell::ST_NONE:
        case ViewShell::ST_PRESENTATION:
        case ViewShell::ST_SIDEBAR:
        default:
            break;
    }
}




void ToolBarRules::MainViewShellChanged (const ViewShell& rMainViewShell)
{
    ::sd::ToolBarManager::UpdateLock aToolBarManagerLock (mpToolBarManager);
    ::sd::ViewShellManager::UpdateLock aViewShellManagerLock (mpViewShellManager);

    MainViewShellChanged(rMainViewShell.GetShellType());
    switch(rMainViewShell.GetShellType())
    {
        case ::sd::ViewShell::ST_IMPRESS:
        case ::sd::ViewShell::ST_DRAW:
        case ::sd::ViewShell::ST_NOTES:
        {
            const DrawViewShell* pDrawViewShell
                = dynamic_cast<const DrawViewShell*>(&rMainViewShell);
            if (pDrawViewShell != NULL)
                if (pDrawViewShell->GetEditMode() == EM_MASTERPAGE)
                    mpToolBarManager->AddToolBar(
                        ToolBarManager::TBG_MASTER_MODE,
                        ToolBarManager::msMasterViewToolBar);
            break;
        }

        default:
            break;
    }
}




void ToolBarRules::SelectionHasChanged (
    const ::sd::ViewShell& rViewShell,
    const SdrView& rView)
{
    ::sd::ToolBarManager::UpdateLock aLock (mpToolBarManager);
    mpToolBarManager->LockViewShellManager();
    bool bTextEdit = rView.IsTextEdit();

    mpToolBarManager->ResetToolBars(ToolBarManager::TBG_FUNCTION);

    switch (rView.GetContext())
    {
        case SDRCONTEXT_GRAPHIC:
            if( !bTextEdit )
                mpToolBarManager->SetToolBarShell(ToolBarManager::TBG_FUNCTION, RID_DRAW_GRAF_TOOLBOX);
            break;

        case SDRCONTEXT_MEDIA:
            if( !bTextEdit )
                mpToolBarManager->SetToolBarShell(ToolBarManager::TBG_FUNCTION, RID_DRAW_MEDIA_TOOLBOX);
            break;

        case SDRCONTEXT_TABLE:
            mpToolBarManager->SetToolBarShell(ToolBarManager::TBG_FUNCTION, RID_DRAW_TABLE_TOOLBOX);
            bTextEdit = true;
            break;

        case SDRCONTEXT_STANDARD:
        default:
            if( !bTextEdit )
            {
                switch(rViewShell.GetShellType())
                {
                    case ::sd::ViewShell::ST_IMPRESS:
                    case ::sd::ViewShell::ST_DRAW:
                    case ::sd::ViewShell::ST_NOTES:
                    case ::sd::ViewShell::ST_HANDOUT:
                        mpToolBarManager->SetToolBar(
                            ToolBarManager::TBG_FUNCTION,
                            ToolBarManager::msDrawingObjectToolBar);
                        break;
                    default:
                        break;
                }
                break;
            }
    }

    if( bTextEdit )
        mpToolBarManager->AddToolBarShell(ToolBarManager::TBG_FUNCTION, RID_DRAW_TEXT_TOOLBOX);

    SdrView* pView = &const_cast<SdrView&>(rView);
    // Check if the extrusion tool bar and the fontwork tool bar have to
    // be activated.
    if (svx::checkForSelectedCustomShapes(pView, true /* bOnlyExtruded */ ))
        mpToolBarManager->AddToolBarShell(ToolBarManager::TBG_FUNCTION, RID_SVX_EXTRUSION_BAR);
    sal_uInt32 nCheckStatus = 0;
    if (svx::checkForSelectedFontWork(pView, nCheckStatus))
        mpToolBarManager->AddToolBarShell(ToolBarManager::TBG_FUNCTION, RID_SVX_FONTWORK_BAR);

    // Switch on additional context-sensitive tool bars.
    if (rView.GetContext() == SDRCONTEXT_POINTEDIT)
        mpToolBarManager->AddToolBarShell(ToolBarManager::TBG_FUNCTION, RID_BEZIER_TOOLBOX);
}




void ToolBarRules::SubShellAdded (
    ::sd::ToolBarManager::ToolBarGroup eGroup,
    sd::ShellId nShellId)
{
    // For some tool bar shells (those defined in sd) we have to add the
    // actual tool bar here.
    switch (nShellId)
    {
        case RID_DRAW_GRAF_TOOLBOX:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msGraphicObjectBar);
            break;

        case RID_DRAW_MEDIA_TOOLBOX:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msMediaObjectBar);
            break;

        case RID_DRAW_TEXT_TOOLBOX:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msTextObjectBar);
            break;

        case RID_BEZIER_TOOLBOX:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msBezierObjectBar);
            break;

        case RID_DRAW_TABLE_TOOLBOX:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msTableObjectBar);
            break;
    }
}




void ToolBarRules::SubShellRemoved (
    ::sd::ToolBarManager::ToolBarGroup eGroup,
    sd::ShellId nShellId)
{
    // For some tool bar shells (those defined in sd) we have to add the
    // actual tool bar here.
    switch (nShellId)
    {
        case RID_DRAW_GRAF_TOOLBOX:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msGraphicObjectBar);
            break;

        case RID_DRAW_MEDIA_TOOLBOX:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msMediaObjectBar);
            break;

        case RID_DRAW_TEXT_TOOLBOX:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msTextObjectBar);
            break;

        case RID_BEZIER_TOOLBOX:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msBezierObjectBar);
            break;

        case RID_DRAW_TABLE_TOOLBOX:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msTableObjectBar);
            break;
    }
}




//===== ToolBarList ===========================================================

ToolBarList::ToolBarList (void)
    : maGroups(),
      maActiveToolBars()
{
}




void ToolBarList::ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup)
{
    Groups::iterator iGroup (maGroups.find(eGroup));
    if (iGroup != maGroups.end())
    {
        if ( ! iGroup->second.empty())
        {
            iGroup->second.clear();
        }
    }
}




void ToolBarList::AddToolBar (
    sd::ToolBarManager::ToolBarGroup eGroup,
    const OUString& rsName)
{
    Groups::iterator iGroup (maGroups.find(eGroup));
    if (iGroup == maGroups.end())
        iGroup = maGroups.insert(Groups::value_type(eGroup,NameList())).first;

    if (iGroup != maGroups.end())
    {
        NameList::const_iterator iBar (
            ::std::find(iGroup->second.begin(),iGroup->second.end(),rsName));
        if (iBar == iGroup->second.end())
        {
            iGroup->second.push_back(rsName);
        }
    }
}




bool ToolBarList::RemoveToolBar (
    sd::ToolBarManager::ToolBarGroup eGroup,
    const OUString& rsName)
{
    Groups::iterator iGroup (maGroups.find(eGroup));
    if (iGroup != maGroups.end())
    {
        NameList::iterator iBar (
            ::std::find(iGroup->second.begin(),iGroup->second.end(),rsName));
        if (iBar != iGroup->second.end())
        {
            iGroup->second.erase(iBar);
            return true;
        }
    }
    return false;
}




void ToolBarList::MakeRequestedToolBarList (NameList& rRequestedToolBars) const
{
    for (int i=sd::ToolBarManager::TBG__FIRST; i<=sd::ToolBarManager::TBG__LAST; ++i)
    {
        ::sd::ToolBarManager::ToolBarGroup eGroup = (::sd::ToolBarManager::ToolBarGroup)i;
        Groups::const_iterator iGroup (maGroups.find(eGroup));
        if (iGroup != maGroups.end())
            ::std::copy(
                iGroup->second.begin(),
                iGroup->second.end(),
                ::std::inserter(rRequestedToolBars,rRequestedToolBars.end()));
    }
}




void ToolBarList::GetToolBarsToActivate (NameList& rToolBars) const
{
    NameList aRequestedToolBars;
    MakeRequestedToolBarList(aRequestedToolBars);

    NameList::const_iterator iToolBar;
    for (iToolBar=aRequestedToolBars.begin(); iToolBar!=aRequestedToolBars.end(); ++iToolBar)
    {
        if (::std::find(maActiveToolBars.begin(),maActiveToolBars.end(),*iToolBar)
            == maActiveToolBars.end())
        {
            rToolBars.push_back(*iToolBar);
        }
    }
}




void ToolBarList::GetToolBarsToDeactivate (NameList& rToolBars) const
{
    NameList aRequestedToolBars;
    MakeRequestedToolBarList(aRequestedToolBars);

    NameList::const_iterator iToolBar;
    for (iToolBar=maActiveToolBars.begin(); iToolBar!=maActiveToolBars.end(); ++iToolBar)
    {
        if (::std::find(aRequestedToolBars.begin(),aRequestedToolBars.end(),*iToolBar)
            == aRequestedToolBars.end())
        {
            rToolBars.push_back(*iToolBar);
        }
    }
}




void ToolBarList::MarkToolBarAsActive (const OUString& rsName)
{
    maActiveToolBars.push_back(rsName);
}




void ToolBarList::MarkToolBarAsNotActive (const OUString& rsName)
{
    maActiveToolBars.erase(
        ::std::find(maActiveToolBars.begin(),maActiveToolBars.end(), rsName));
}




void ToolBarList::MarkAllToolBarsAsNotActive (void)
{
    maActiveToolBars.clear();
}




//===== ToolBarShellList ======================================================

ToolBarShellList::ShellDescriptor::ShellDescriptor (
    ShellId nId,
    sd::ToolBarManager::ToolBarGroup eGroup)
    : mnId(nId),
      meGroup(eGroup)
{
}




ToolBarShellList::ToolBarShellList (void)
: maNewList()
, maCurrentList()
{
}




void ToolBarShellList::ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup)
{
    // In every loop we erase the first member of the specified group.
    // Because that invalidates the iterator another loop is started after
    // that.  The loop is left only when no member of the group is found and
    // no element is erased
    bool bLoop;
    do
    {
        bLoop = false;

        GroupedShellList::iterator iDescriptor;
        for (iDescriptor=maNewList.begin(); iDescriptor!=maNewList.end(); ++iDescriptor)
            if (iDescriptor->meGroup == eGroup)
            {
                maNewList.erase(iDescriptor);
                // Erasing the descriptor invalidated the iterator so we
                // have to exit the for loop and start anew to search for
                // further elements of the group.
                bLoop = true;
                break;
            }
    }
    while (bLoop);
}




void ToolBarShellList::AddShellId (sd::ToolBarManager::ToolBarGroup eGroup, sd::ShellId nId)
{
    // Make sure that the shell is not added twice (and possibly in
    // different groups.)
    ShellDescriptor aDescriptor (nId,eGroup);
    GroupedShellList::iterator iDescriptor (maNewList.find(aDescriptor));
    if (iDescriptor != maNewList.end())
    {
        // The shell is already requested.
        if (iDescriptor->meGroup != eGroup)
        {
            // It is now being requested for another group.
            // (Is this an error?)
            // Move it to that group.
            maNewList.erase(iDescriptor);
            maNewList.insert(aDescriptor);
        }
        // else nothing to do.
    }
    else
        maNewList.insert(aDescriptor);
}




void ToolBarShellList::ReleaseAllShells (ToolBarRules& rRules)
{
    // Release the currently active tool bars.
    GroupedShellList aList (maCurrentList);
    GroupedShellList::iterator iDescriptor;
    for (iDescriptor=aList.begin(); iDescriptor!=aList.end(); ++iDescriptor)
    {
        rRules.SubShellRemoved(iDescriptor->meGroup, iDescriptor->mnId);
    }

    // Clear the list of requested tool bars.
    maNewList.clear();
}




void ToolBarShellList::UpdateShells (
    const ::boost::shared_ptr<ViewShell>& rpMainViewShell,
    const ::boost::shared_ptr<ViewShellManager>& rpManager)
{
    if (rpMainViewShell.get() != NULL)
    {
        GroupedShellList aList;

        // Deactivate shells that are in maCurrentList, but not in
        // maNewList.
        ::std::set_difference(maCurrentList.begin(), maCurrentList.end(),
            maNewList.begin(), maNewList.end(),
            std::insert_iterator<GroupedShellList>(aList,aList.begin()));
        for (GroupedShellList::iterator iShell=aList.begin(); iShell!=aList.end(); ++iShell)
        {
            SAL_INFO("sd.view", OSL_THIS_FUNC << ": deactivating tool bar shell " << iShell->mnId);
            rpManager->DeactivateSubShell(*rpMainViewShell, iShell->mnId);
        }

        // Activate shells that are in maNewList, but not in
        // maCurrentList.
        aList.clear();
        ::std::set_difference(maNewList.begin(), maNewList.end(),
            maCurrentList.begin(), maCurrentList.end(),
            std::insert_iterator<GroupedShellList>(aList,aList.begin()));
        for (GroupedShellList::iterator iShell=aList.begin(); iShell!=aList.end(); ++iShell)
        {
            SAL_INFO("sd.view", OSL_THIS_FUNC << ": activating tool bar shell " << iShell->mnId);
            rpManager->ActivateSubShell(*rpMainViewShell, iShell->mnId);
        }

        // The maNewList now refelects the current state and thus is made
        // maCurrentList.
        maCurrentList = maNewList;
    }
}




} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
