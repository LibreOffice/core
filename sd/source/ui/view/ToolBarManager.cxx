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

#include <ToolBarManager.hxx>

#include <DrawViewShell.hxx>
#include <EventMultiplexer.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <sal/log.hxx>
#include <osl/mutex.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/enumrange.hxx>
#include <rtl/ref.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/toolbarids.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <tools/link.hxx>

#include <map>
#include <utility>
#include <memory>
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
    Reference<frame::XLayoutManager> mxLayouter;
public:
    explicit LayouterLock (const Reference<frame::XLayoutManager>& rxLayouter);
    ~LayouterLock();
    bool is() const { return mxLayouter.is(); }
};

/** Store a list of tool bars for each of the tool bar groups.  From
    this the list of requested tool bars is built.
*/
class ToolBarList
{
public:
    ToolBarList();

    void ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup);
    void AddToolBar (sd::ToolBarManager::ToolBarGroup eGroup, const OUString& rsName);
    bool RemoveToolBar (sd::ToolBarManager::ToolBarGroup eGroup, const OUString& rsName);

    void GetToolBarsToActivate (std::vector<OUString>& rToolBars) const;
    void GetToolBarsToDeactivate (std::vector<OUString>& rToolBars) const;

    void MarkToolBarAsActive (const OUString& rsName);
    void MarkToolBarAsNotActive (const OUString& rsName);
    void MarkAllToolBarsAsNotActive();

private:
    typedef ::std::map<sd::ToolBarManager::ToolBarGroup, std::vector<OUString> > Groups;
    Groups maGroups;
    std::vector<OUString> maActiveToolBars;

    void MakeRequestedToolBarList (std::vector<OUString>& rToolBars) const;
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
    ToolBarShellList();

    /** Remove all shells from a group.  Calling this method should normally
        not be necessary because after the construction or after a call to
        UpdateShells() the requested list is empty.
        @param eGroup
            The group to clear. Shells in other groups are not modified.
    */
    void ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup);

    /** Add a shell. When the specified shell has already been requested
        for another group then it is moved to this group.
        @param eGroup
            The group to which to add the shell.
        @param nId
            The id of the shell to add.
    */
    void AddShellId (sd::ToolBarManager::ToolBarGroup eGroup, sd::ShellId nId);

    /** Releasing all shells means that the given ToolBarRules object is
        informed that every shell managed by the called ToolBarShellList is
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
        const std::shared_ptr<ViewShell>& rpMainViewShell,
        const std::shared_ptr<ViewShellManager>& rpManager);

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
        const std::shared_ptr<ToolBarManager>& rpToolBarManager,
        const std::shared_ptr<ViewShellManager>& rpViewShellManager);

    /** This method calls MainViewShellChanged() and SelectionHasChanged()
        for the current main view shell and its view.
    */
    void Update (ViewShellBase const & rBase);

    /** Reset all tool bars in all groups and add tool bars and tool bar
        shells to the ToolBarGroup::Permanent group for the specified ViewShell type.
    */
    void MainViewShellChanged (ViewShell::ShellType nShellType);

    /** Reset all tool bars in all groups and add tool bars and tool bar
        shells to the ToolBarGroup::Permanent group for the specified ViewShell.
    */
    void MainViewShellChanged (const ViewShell& rMainViewShell);

    /** Reset all tool bars in the ToolBarGroup::Function group and add tool bars and tool bar
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
    std::shared_ptr<ToolBarManager> mpToolBarManager;
    std::shared_ptr<ViewShellManager> mpViewShellManager;
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
        const std::shared_ptr<sd::tools::EventMultiplexer>& rpMultiplexer,
        const std::shared_ptr<ViewShellManager>& rpViewShellManager,
        const std::shared_ptr<ToolBarManager>& rpToolBarManager);
    ~Implementation();

    void SetValid (bool bValid);

    void ResetToolBars (ToolBarGroup eGroup);
    void ResetAllToolBars();
    void AddToolBar (ToolBarGroup eGroup, const OUString& rsToolBarName);
    void AddToolBarShell (ToolBarGroup eGroup, ShellId nToolBarId);
    void RemoveToolBar (ToolBarGroup eGroup, const OUString& rsToolBarName);

    /** Release all tool bar shells and the associated framework tool bars.
        Typically called when the main view shell is being replaced by
        another, all tool bar shells are released.  In that process the
        shells are destroyed anyway and without calling this method they
        would still be referenced.
    */
    void ReleaseAllToolBarShells();

    void ToolBarsDestroyed();

    void RequestUpdate();

    void PreUpdate();
    void PostUpdate();
    /** Tell the XLayoutManager about the tool bars that we would like to be
        shown.
        @param rpLayouterLock
            This typically is the mpSynchronousLayouterLock that is used in
            this method and that is either released at its end or assigned
            to mpAsynchronousLock in order to be unlocked later.
    */
    void Update (::std::unique_ptr<LayouterLock> pLayouterLock);

    class UpdateLockImplementation
    {
    public:
        explicit UpdateLockImplementation (Implementation& rImplementation)
            : mrImplementation(rImplementation) { mrImplementation.LockUpdate();  }
        ~UpdateLockImplementation() { mrImplementation.UnlockUpdate(); }
    private:
        Implementation& mrImplementation;
    };

    void LockViewShellManager();
    void LockUpdate();
    void UnlockUpdate();

    ToolBarRules& GetToolBarRules() { return maToolBarRules;}

private:
    mutable ::osl::Mutex maMutex;
    ViewShellBase& mrBase;
    std::shared_ptr<sd::tools::EventMultiplexer> mpEventMultiplexer;
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
    ::std::unique_ptr<LayouterLock> mpSynchronousLayouterLock;
    ::std::unique_ptr<LayouterLock> mpAsynchronousLayouterLock;
    ::std::unique_ptr<ViewShellManager::UpdateLock, o3tl::default_delete<ViewShellManager::UpdateLock>> mpViewShellManagerLock;
    ImplSVEvent * mnPendingUpdateCall;
    ImplSVEvent * mnPendingSetValidCall;
    ToolBarRules maToolBarRules;

    static OUString GetToolBarResourceName (const OUString& rsBaseName);
    bool CheckPlugInMode (const OUString& rsName) const;

    DECL_LINK(UpdateCallback, void *, void);
    DECL_LINK(EventMultiplexerCallback, sd::tools::EventMultiplexerEvent&, void);
    DECL_LINK(SetValidCallback, void*, void);
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

std::shared_ptr<ToolBarManager> ToolBarManager::Create (
    ViewShellBase& rBase,
    const std::shared_ptr<sd::tools::EventMultiplexer>& rpMultiplexer,
    const std::shared_ptr<ViewShellManager>& rpViewShellManager)
{
    std::shared_ptr<ToolBarManager> pManager (new ToolBarManager());
    pManager->mpImpl.reset(
        new Implementation(rBase,rpMultiplexer,rpViewShellManager,pManager));
    return pManager;
}

ToolBarManager::ToolBarManager()
        : mpImpl()
{
}

ToolBarManager::~ToolBarManager()
{
}

void ToolBarManager::Shutdown()
{
    if (mpImpl != nullptr)
        mpImpl.reset();
}

void ToolBarManager::ResetToolBars (ToolBarGroup eGroup)
{
    if (mpImpl != nullptr)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetToolBars(eGroup);
    }
}

void ToolBarManager::ResetAllToolBars()
{
    if (mpImpl != nullptr)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetAllToolBars();
    }
}

void ToolBarManager::AddToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    if (mpImpl != nullptr)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->AddToolBar(eGroup,rsToolBarName);
    }
}

void ToolBarManager::AddToolBarShell (
    ToolBarGroup eGroup,
    ShellId nToolBarId)
{
    if (mpImpl != nullptr)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->AddToolBarShell(eGroup,nToolBarId);
    }
}

void ToolBarManager::RemoveToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    if (mpImpl != nullptr)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->RemoveToolBar(eGroup,rsToolBarName);
    }
}

void ToolBarManager::SetToolBar (
    ToolBarGroup eGroup,
    const OUString& rsToolBarName)
{
    if (mpImpl != nullptr)
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
    if (mpImpl != nullptr)
    {
        UpdateLock aLock (shared_from_this());
        mpImpl->ResetToolBars(eGroup);
        mpImpl->AddToolBarShell(eGroup,nToolBarId);
    }
}

void ToolBarManager::PreUpdate()
{
    if (mpImpl != nullptr)
        mpImpl->PreUpdate();
}

void ToolBarManager::RequestUpdate()
{
    if (mpImpl != nullptr)
        mpImpl->RequestUpdate();
}

void ToolBarManager::LockViewShellManager()
{
    if (mpImpl != nullptr)
        mpImpl->LockViewShellManager();
}

void ToolBarManager::LockUpdate()
{
    if (mpImpl != nullptr)
        mpImpl->LockUpdate();
}

void ToolBarManager::UnlockUpdate()
{
    if (mpImpl != nullptr)
        mpImpl->UnlockUpdate();
}

void ToolBarManager::MainViewShellChanged ()
{
    if (mpImpl != nullptr)
    {
        mpImpl->ReleaseAllToolBarShells();
        mpImpl->GetToolBarRules().MainViewShellChanged(ViewShell::ST_NONE);
    }
}

void ToolBarManager::MainViewShellChanged (const ViewShell& rMainViewShell)
{
    if (mpImpl != nullptr)
    {
        mpImpl->ReleaseAllToolBarShells();
        mpImpl->GetToolBarRules().MainViewShellChanged(rMainViewShell);
    }
}

void ToolBarManager::SelectionHasChanged (
    const ViewShell& rViewShell,
    const SdrView& rView)
{
    if (mpImpl != nullptr)
        mpImpl->GetToolBarRules().SelectionHasChanged(rViewShell,rView);
}

void ToolBarManager::ToolBarsDestroyed()
{
    if (mpImpl != nullptr)
        mpImpl->ToolBarsDestroyed();
}

//===== ToolBarManager::Implementation =======================================

ToolBarManager::Implementation::Implementation (
    ViewShellBase& rBase,
    const std::shared_ptr<sd::tools::EventMultiplexer>& rpMultiplexer,
    const std::shared_ptr<ViewShellManager>& rpViewShellManager,
    const std::shared_ptr<ToolBarManager>& rpToolBarManager)
    : maMutex(),
      mrBase(rBase),
      mpEventMultiplexer(rpMultiplexer),
      mbIsValid(false),
      maToolBarList(),
      maToolBarShellList(),
      mnLockCount(0),
      mbPreUpdatePending(false),
      mbPostUpdatePending(false),
      mpSynchronousLayouterLock(),
      mpAsynchronousLayouterLock(),
      mpViewShellManagerLock(),
      mnPendingUpdateCall(nullptr),
      mnPendingSetValidCall(nullptr),
      maToolBarRules(rpToolBarManager,rpViewShellManager)
{
    Link<tools::EventMultiplexerEvent&,void> aLink (LINK(this,ToolBarManager::Implementation,EventMultiplexerCallback));
    mpEventMultiplexer->AddEventListener( aLink );
}

/** The order of statements is important.
    First unregister listeners, which may post user events.
    Then remove pending user events.
*/
ToolBarManager::Implementation::~Implementation()
{
    // Unregister at broadcasters.
    Link<tools::EventMultiplexerEvent&,void> aLink (LINK(this,ToolBarManager::Implementation,EventMultiplexerCallback));
    mpEventMultiplexer->RemoveEventListener(aLink);

    // Abort pending user calls.
    if (mnPendingUpdateCall != nullptr)
        Application::RemoveUserEvent(mnPendingUpdateCall);
    if (mnPendingSetValidCall != nullptr)
        Application::RemoveUserEvent(mnPendingSetValidCall);
}

void ToolBarManager::Implementation::ToolBarsDestroyed()
{
    maToolBarList.MarkAllToolBarsAsNotActive();
}

void ToolBarManager::Implementation::SetValid (bool bValid)
{
    ::osl::MutexGuard aGuard(maMutex);

    if (mbIsValid == bValid)
        return;

    UpdateLockImplementation aUpdateLock (*this);

    mbIsValid = bValid;
    if (mbIsValid)
    {
        Reference<frame::XFrame> xFrame;
        if (mrBase.GetViewFrame() != nullptr)
            xFrame = mrBase.GetViewFrame()->GetFrame().GetFrameInterface();
        try
        {
            Reference<beans::XPropertySet> xFrameProperties (xFrame, UNO_QUERY_THROW);
            Any aValue (xFrameProperties->getPropertyValue("LayoutManager"));
            aValue >>= mxLayouter;
            // tdf#119997 if mpSynchronousLayouterLock was created before mxLayouter was
            // set then update it now that its available
            if (mpSynchronousLayouterLock && !mpSynchronousLayouterLock->is())
                mpSynchronousLayouterLock.reset(new LayouterLock(mxLayouter));
        }
        catch (const RuntimeException&)
        {
        }

        GetToolBarRules().Update(mrBase);
    }
    else
    {
        ResetAllToolBars();
        mxLayouter = nullptr;
    }
}

void ToolBarManager::Implementation::ResetToolBars (ToolBarGroup eGroup)
{
    ::osl::MutexGuard aGuard(maMutex);

    maToolBarList.ClearGroup(eGroup);
    maToolBarShellList.ClearGroup(eGroup);

    mbPreUpdatePending = true;
}

void ToolBarManager::Implementation::ResetAllToolBars()
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": resetting all tool bars");
    for (auto i : o3tl::enumrange<ToolBarGroup>())
        ResetToolBars(i);
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
    if (pMainViewShell != nullptr)
    {
        maToolBarShellList.AddShellId(eGroup,nToolBarId);
        GetToolBarRules().SubShellAdded(eGroup, nToolBarId);
    }
}

void ToolBarManager::Implementation::ReleaseAllToolBarShells()
{
    maToolBarShellList.ReleaseAllShells(GetToolBarRules());
    maToolBarShellList.UpdateShells(mrBase.GetMainViewShell(), mrBase.GetViewShellManager());
}

void ToolBarManager::Implementation::RequestUpdate()
{
    if (mnPendingUpdateCall == nullptr)
    {
        mnPendingUpdateCall = Application::PostUserEvent(
            LINK(this,ToolBarManager::Implementation,UpdateCallback));
    }
}

void ToolBarManager::Implementation::PreUpdate()
{
    ::osl::MutexGuard aGuard(maMutex);

    if (!(mbIsValid
        && mbPreUpdatePending
        && mxLayouter.is()))
        return;

    mbPreUpdatePending = false;

    SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PreUpdate [");

    // Get the list of tool bars that are not used anymore and are to be
    // deactivated.
    std::vector<OUString> aToolBars;
    maToolBarList.GetToolBarsToDeactivate(aToolBars);

    // Turn off the tool bars.
    for (auto& aToolBar : aToolBars)
    {
        OUString sFullName (GetToolBarResourceName(aToolBar));
        SAL_INFO("sd.view", OSL_THIS_FUNC << ":    turning off tool bar " << sFullName);
        mxLayouter->destroyElement(sFullName);
        maToolBarList.MarkToolBarAsNotActive(aToolBar);
    }

    SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PreUpdate ]");
}

void ToolBarManager::Implementation::PostUpdate()
{
    ::osl::MutexGuard aGuard(maMutex);

    if (!(mbIsValid
        && mbPostUpdatePending
        && mxLayouter.is()))
        return;

    mbPostUpdatePending = false;

    // Create the list of requested tool bars.
    std::vector<OUString> aToolBars;
    maToolBarList.GetToolBarsToActivate(aToolBars);

    SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PostUpdate [");

    // Turn on the tool bars that are visible in the new context.
    for (auto& aToolBar : aToolBars)
    {
        OUString sFullName (GetToolBarResourceName(aToolBar));
        SAL_INFO("sd.view", OSL_THIS_FUNC << ":    turning on tool bar " << sFullName);
        mxLayouter->requestElement(sFullName);
        maToolBarList.MarkToolBarAsActive(aToolBar);
    }

    SAL_INFO("sd.view", OSL_THIS_FUNC << ": ToolBarManager::PostUpdate ]");
}

void ToolBarManager::Implementation::LockViewShellManager()
{
    if (mpViewShellManagerLock == nullptr)
        mpViewShellManagerLock.reset(
            new ViewShellManager::UpdateLock(mrBase.GetViewShellManager()));
}

void ToolBarManager::Implementation::LockUpdate()
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": LockUpdate " << mnLockCount);
    ::osl::MutexGuard aGuard(maMutex);

    DBG_ASSERT(mnLockCount<100, "ToolBarManager lock count unusually high");
    if (mnLockCount == 0)
    {
        OSL_ASSERT(mpSynchronousLayouterLock == nullptr);

        mpSynchronousLayouterLock.reset(new LayouterLock(mxLayouter));
    }
    ++mnLockCount;
}

void ToolBarManager::Implementation::UnlockUpdate()
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": UnlockUpdate " << mnLockCount);
    ::osl::MutexGuard aGuard(maMutex);

    OSL_ASSERT(mnLockCount>0);
    --mnLockCount;
    if (mnLockCount == 0)
    {
        Update(std::move(mpSynchronousLayouterLock));
    }
}

void ToolBarManager::Implementation::Update (
    ::std::unique_ptr<LayouterLock> pLocalLayouterLock)
{
    // When the lock is released and there are pending changes to the set of
    // tool bars then update this set now.
    if (mnLockCount != 0)
        return;

    // During creation of ViewShellBase we may have the situation that
    // the controller has already been created and attached to the frame
    // but that the ToolBarManager has not yet completed its
    // initialization (by initializing the mxLayouter member.)  We do
    // this here so that we do not have to wait for the next Update()
    // call to show the tool bars.
    if (mnPendingSetValidCall != nullptr)
    {
        Application::RemoveUserEvent(mnPendingSetValidCall);
        mnPendingSetValidCall = nullptr;
        SetValid(true);
    }

    if (mbIsValid && mxLayouter.is() && (mbPreUpdatePending || mbPostUpdatePending))
    {
        // 1) Release UNO tool bars that are no longer used.  Do this
        // now so that they are not updated when the SFX shell stack is
        // modified.
        if (mbPreUpdatePending)
            PreUpdate();

        // 2) Update the requested shells that represent tool bar
        // functionality. Those that are not used anymore are
        // deactivated now.  Those that are missing are activated in the
        // next step together with the view shells.
        if (mpViewShellManagerLock == nullptr)
            mpViewShellManagerLock.reset(
                new ViewShellManager::UpdateLock(mrBase.GetViewShellManager()));
        maToolBarShellList.UpdateShells(
            mrBase.GetMainViewShell(),
            mrBase.GetViewShellManager());

        // 3) Unlock the ViewShellManager::UpdateLock.  This updates the
        // shell stack.
        mpViewShellManagerLock.reset();

        // 4) Make the UNO tool bars visible.  The outstanding call to
        // PostUpdate() is done via PostUserEvent() so that it is
        // guaranteed to be executed when the SFX shell stack has been
        // updated (under the assumption that our lock to the
        // ViewShellManager was the only one open.  If that is not the
        // case then all should still be well but not as fast.)

        // Note that the lock count may have been increased since
        // entering this method.  In that case one of the next
        // UnlockUpdate() calls will post the UpdateCallback.
        if (mnLockCount==0)
        {
            mpAsynchronousLayouterLock = std::move(pLocalLayouterLock);
            if (mnPendingUpdateCall==nullptr)
            {
                mnPendingUpdateCall = Application::PostUserEvent(
                    LINK(this,ToolBarManager::Implementation,UpdateCallback));
            }
        }
    }
    else
    {
        mpViewShellManagerLock.reset();
        pLocalLayouterLock.reset();
    }
}

IMPL_LINK_NOARG(ToolBarManager::Implementation, UpdateCallback, void*, void)
{
    mnPendingUpdateCall = nullptr;
    if (mnLockCount == 0)
    {
        if (mbPreUpdatePending)
            PreUpdate();
        if (mbPostUpdatePending)
            PostUpdate();
        if (mbIsValid && mxLayouter.is())
            mpAsynchronousLayouterLock.reset();
    }
}

IMPL_LINK(ToolBarManager::Implementation,EventMultiplexerCallback,
    sd::tools::EventMultiplexerEvent&, rEvent, void)
{
    SolarMutexGuard g;
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::ControllerAttached:
            if (mnPendingSetValidCall == nullptr)
                mnPendingSetValidCall
                    = Application::PostUserEvent(LINK(this,Implementation,SetValidCallback));
            break;

        case EventMultiplexerEventId::ControllerDetached:
            SetValid(false);
            break;

        default: break;
    }
}

IMPL_LINK_NOARG(ToolBarManager::Implementation, SetValidCallback, void*, void)
{
    mnPendingSetValidCall = nullptr;
    SetValid(true);
}

OUString ToolBarManager::Implementation::GetToolBarResourceName (
    const OUString& rsBaseName)
{
    return "private:resource/toolbar/" + rsBaseName;
}

bool ToolBarManager::Implementation::CheckPlugInMode (const OUString& rsName) const
{
    bool bValid (false);

    // Determine the plug in mode.
    bool bIsPlugInMode (false);
    do
    {
        SfxObjectShell* pObjectShell = mrBase.GetObjectShell();
        if (pObjectShell == nullptr)
            break;

        SfxMedium* pMedium = pObjectShell->GetMedium();
        if (pMedium == nullptr)
            break;

        const SfxBoolItem* pViewOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_VIEWONLY, false);
        if (pViewOnlyItem == nullptr)
            break;

        bIsPlugInMode = pViewOnlyItem->GetValue();
    }
    while (false);

    if (rsName == msViewerToolBar)
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

LayouterLock::~LayouterLock()
{
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": ~LayouterLock " << (mxLayouter.is() ? 1 :0));
    if (mxLayouter.is())
        mxLayouter->unlock();
}

//===== ToolBarRules ==========================================================

ToolBarRules::ToolBarRules (
    const std::shared_ptr<sd::ToolBarManager>& rpToolBarManager,
    const std::shared_ptr<sd::ViewShellManager>& rpViewShellManager)
    : mpToolBarManager(rpToolBarManager),
      mpViewShellManager(rpViewShellManager)
{
}

void ToolBarRules::Update (ViewShellBase const & rBase)
{
    ViewShell* pMainViewShell = rBase.GetMainViewShell().get();
    if (pMainViewShell != nullptr)
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
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msOptionsToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msViewerToolBar);
            break;

        case ::sd::ViewShell::ST_DRAW:
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msOptionsToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msViewerToolBar);
            break;

        case ViewShell::ST_OUTLINE:
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msOutlineToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msViewerToolBar);
            mpToolBarManager->AddToolBarShell(
                ToolBarManager::ToolBarGroup::Permanent, ToolbarId::Draw_Text_Toolbox_Sd);
            break;

        case ViewShell::ST_SLIDE_SORTER:
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msViewerToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
                ToolBarManager::msSlideSorterToolBar);
            mpToolBarManager->AddToolBar(
                ToolBarManager::ToolBarGroup::Permanent,
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
            if (pDrawViewShell != nullptr)
            {
                if (pDrawViewShell->GetEditMode() == EditMode::MasterPage)
                    mpToolBarManager->AddToolBar(
                        ToolBarManager::ToolBarGroup::MasterMode,
                        ToolBarManager::msMasterViewToolBar);
                else if ( rMainViewShell.GetShellType() != ::sd::ViewShell::ST_DRAW )
                    mpToolBarManager->AddToolBar(
                        ToolBarManager::ToolBarGroup::CommonTask,
                        ToolBarManager::msCommonTaskToolBar);
            }
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

    mpToolBarManager->ResetToolBars(ToolBarManager::ToolBarGroup::Function);

    switch (rView.GetContext())
    {
        case SdrViewContext::Graphic:
            if( !bTextEdit )
                mpToolBarManager->SetToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Draw_Graf_Toolbox);
            break;

        case SdrViewContext::Media:
            if( !bTextEdit )
                mpToolBarManager->SetToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Draw_Media_Toolbox);
            break;

        case SdrViewContext::Table:
            mpToolBarManager->SetToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Draw_Table_Toolbox);
            bTextEdit = true;
            break;

        case SdrViewContext::Standard:
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
                            ToolBarManager::ToolBarGroup::Function,
                            ToolBarManager::msDrawingObjectToolBar);
                        break;
                    default:
                        break;
                }
                break;
            }
    }

    if( bTextEdit )
        mpToolBarManager->AddToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Draw_Text_Toolbox_Sd);

    SdrView* pView = &const_cast<SdrView&>(rView);
    // Check if the extrusion tool bar and the fontwork tool bar have to
    // be activated.
    if (svx::checkForSelectedCustomShapes(pView, true /* bOnlyExtruded */ ))
        mpToolBarManager->AddToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Svx_Extrusion_Bar);
    sal_uInt32 nCheckStatus = 0;
    if (svx::checkForSelectedFontWork(pView, nCheckStatus))
        mpToolBarManager->AddToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Svx_Fontwork_Bar);

    // Switch on additional context-sensitive tool bars.
    if (rView.GetContext() == SdrViewContext::PointEdit)
        mpToolBarManager->AddToolBarShell(ToolBarManager::ToolBarGroup::Function, ToolbarId::Bezier_Toolbox_Sd);
}

void ToolBarRules::SubShellAdded (
    ::sd::ToolBarManager::ToolBarGroup eGroup,
    sd::ShellId nShellId)
{
    // For some tool bar shells (those defined in sd) we have to add the
    // actual tool bar here.
    switch (nShellId)
    {
        case ToolbarId::Draw_Graf_Toolbox:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msGraphicObjectBar);
            break;

        case ToolbarId::Draw_Media_Toolbox:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msMediaObjectBar);
            break;

        case ToolbarId::Draw_Text_Toolbox_Sd:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msTextObjectBar);
            break;

        case ToolbarId::Bezier_Toolbox_Sd:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msBezierObjectBar);
            break;

        case ToolbarId::Draw_Table_Toolbox:
            mpToolBarManager->AddToolBar(eGroup, ToolBarManager::msTableObjectBar);
            break;

        default:
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
        case ToolbarId::Draw_Graf_Toolbox:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msGraphicObjectBar);
            break;

        case ToolbarId::Draw_Media_Toolbox:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msMediaObjectBar);
            break;

        case ToolbarId::Draw_Text_Toolbox_Sd:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msTextObjectBar);
            break;

        case ToolbarId::Bezier_Toolbox_Sd:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msBezierObjectBar);
            break;

        case ToolbarId::Draw_Table_Toolbox:
            mpToolBarManager->RemoveToolBar(eGroup, ToolBarManager::msTableObjectBar);
            break;

        default:
            break;
    }
}

//===== ToolBarList ===========================================================

ToolBarList::ToolBarList()
    : maGroups(),
      maActiveToolBars()
{
}

void ToolBarList::ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup)
{
    Groups::iterator iGroup (maGroups.find(eGroup));
    if (iGroup != maGroups.end())
    {
        iGroup->second.clear();
    }
}

void ToolBarList::AddToolBar (
    sd::ToolBarManager::ToolBarGroup eGroup,
    const OUString& rsName)
{
    Groups::iterator iGroup (maGroups.find(eGroup));
    if (iGroup == maGroups.end())
        iGroup = maGroups.emplace(eGroup,std::vector<OUString>()).first;

    if (iGroup != maGroups.end())
    {
        auto iBar (std::find(iGroup->second.cbegin(),iGroup->second.cend(),rsName));
        if (iBar == iGroup->second.cend())
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
        auto iBar (std::find(iGroup->second.begin(),iGroup->second.end(),rsName));
        if (iBar != iGroup->second.end())
        {
            iGroup->second.erase(iBar);
            return true;
        }
    }
    return false;
}

void ToolBarList::MakeRequestedToolBarList (std::vector<OUString>& rRequestedToolBars) const
{
    for (auto eGroup : o3tl::enumrange<sd::ToolBarManager::ToolBarGroup>())
    {
        Groups::const_iterator iGroup (maGroups.find(eGroup));
        if (iGroup != maGroups.end())
            ::std::copy(
                iGroup->second.begin(),
                iGroup->second.end(),
                ::std::inserter(rRequestedToolBars,rRequestedToolBars.end()));
    }
}

void ToolBarList::GetToolBarsToActivate (std::vector<OUString>& rToolBars) const
{
    std::vector<OUString> aRequestedToolBars;
    MakeRequestedToolBarList(aRequestedToolBars);

    for (auto& aToolBar : aRequestedToolBars)
    {
        if (::std::find(maActiveToolBars.begin(),maActiveToolBars.end(),aToolBar)
            == maActiveToolBars.end())
        {
            rToolBars.push_back(aToolBar);
        }
    }
}

void ToolBarList::GetToolBarsToDeactivate (std::vector<OUString>& rToolBars) const
{
    std::vector<OUString> aRequestedToolBars;
    MakeRequestedToolBarList(aRequestedToolBars);

    for (auto& aToolBar : maActiveToolBars)
    {
        if (::std::find(aRequestedToolBars.begin(),aRequestedToolBars.end(),aToolBar)
            == aRequestedToolBars.end())
        {
            rToolBars.push_back(aToolBar);
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

void ToolBarList::MarkAllToolBarsAsNotActive()
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

ToolBarShellList::ToolBarShellList()
: maNewList()
, maCurrentList()
{
}

void ToolBarShellList::ClearGroup (sd::ToolBarManager::ToolBarGroup eGroup)
{
    for (GroupedShellList::iterator iDescriptor = maNewList.begin(); iDescriptor != maNewList.end(); )
        if (iDescriptor->meGroup == eGroup)
            iDescriptor = maNewList.erase(iDescriptor);
        else
            ++iDescriptor;
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
    for (const auto& rDescriptor : aList)
    {
        rRules.SubShellRemoved(rDescriptor.meGroup, rDescriptor.mnId);
    }

    // Clear the list of requested tool bars.
    maNewList.clear();
}

void ToolBarShellList::UpdateShells (
    const std::shared_ptr<ViewShell>& rpMainViewShell,
    const std::shared_ptr<ViewShellManager>& rpManager)
{
    if (rpMainViewShell == nullptr)
        return;

    GroupedShellList aList;

    // Deactivate shells that are in maCurrentList, but not in
    // maNewList.
    ::std::set_difference(maCurrentList.begin(), maCurrentList.end(),
        maNewList.begin(), maNewList.end(),
        std::insert_iterator<GroupedShellList>(aList,aList.begin()));
    for (const auto& rShell : aList)
    {
        SAL_INFO("sd.view", OSL_THIS_FUNC << ": deactivating tool bar shell " << static_cast<sal_uInt32>(rShell.mnId));
        rpManager->DeactivateSubShell(*rpMainViewShell, rShell.mnId);
    }

    // Activate shells that are in maNewList, but not in
    // maCurrentList.
    aList.clear();
    ::std::set_difference(maNewList.begin(), maNewList.end(),
        maCurrentList.begin(), maCurrentList.end(),
        std::insert_iterator<GroupedShellList>(aList,aList.begin()));
    for (const auto& rShell : aList)
    {
        SAL_INFO("sd.view", OSL_THIS_FUNC << ": activating tool bar shell " << static_cast<sal_uInt32>(rShell.mnId));
        rpManager->ActivateSubShell(*rpMainViewShell, rShell.mnId);
    }

    // The maNewList now reflects the current state and thus is made
    // maCurrentList.
    maCurrentList = maNewList;
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
