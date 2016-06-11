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

#include "ViewShellManager.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "DrawDocShell.hxx"
#include "FormShellManager.hxx"

#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <svx/fmshell.hxx>

#include <iterator>
#include <unordered_map>

namespace sd {

namespace {

/** The ShellDescriptor class is used to shells together with their ids and
    the factory that was used to create the shell.

    The shell pointer may be NULL.  In that case the shell is created on
    demand by a factory.

    The factory pointer may be NULL.  In that case the shell pointer is
    given to the ViewShellManager.

    Shell pointer and factory pointer can but should not be NULL at the same
    time.
*/
class ShellDescriptor {
public:
    SfxShell* mpShell;
    ShellId mnId;
    ViewShellManager::SharedShellFactory mpFactory;
    bool mbIsListenerAddedToWindow;

    ShellDescriptor ();
    ShellDescriptor (SfxShell* pShell, ShellId nId);
    ShellDescriptor (const ShellDescriptor& rDescriptor);
    ShellDescriptor& operator= (const ShellDescriptor& rDescriptor);
    bool IsMainViewShell() const;
    vcl::Window* GetWindow() const;
};

/** This functor can be used to search for a shell in an STL container when the
    shell pointer is given.
*/
class IsShell : public ::std::unary_function<ShellDescriptor,bool>
{
public:
    explicit IsShell (const SfxShell* pShell) : mpShell(pShell) {}
    bool operator() (const ShellDescriptor& rDescriptor)
    { return rDescriptor.mpShell == mpShell; }
private:
    const SfxShell* mpShell;
};

/** This functor can be used to search for a shell in an STL container when the
    id of the shell is given.
*/
class IsId : public ::std::unary_function<ShellDescriptor,bool>
{
public:
    explicit IsId (ShellId nId) : mnId(nId) {}
    bool operator() (const ShellDescriptor& rDescriptor)
    { return rDescriptor.mnId == mnId; }
private:
    ShellId mnId;
};

} // end of anonymous namespace

class ViewShellManager::Implementation
{
public:
    Implementation (
        ViewShellManager& rManager,
        ViewShellBase& rBase);
    ~Implementation();

    void AddShellFactory (
        const SfxShell* pViewShell,
        const SharedShellFactory& rpFactory);
    void RemoveShellFactory (
        const SfxShell* pViewShell,
        const SharedShellFactory& rpFactory);
    void ActivateViewShell (
        ViewShell* pViewShell);
    void DeactivateViewShell (const ViewShell& rShell);
    void ActivateShell (SfxShell& rShell);
    void DeactivateShell (const SfxShell& rShell);
    void ActivateShell (const ShellDescriptor& rDescriptor);
    void SetFormShell (const ViewShell* pViewShell, FmFormShell* pFormShell, bool bAbove);
    void ActivateSubShell (const SfxShell& rParentShell, ShellId nId);
    void DeactivateSubShell (const SfxShell& rParentShell, ShellId nId);
    void MoveToTop (const SfxShell& rParentShell);
    SfxShell* GetShell (ShellId nId) const;
    SfxShell* GetTopShell() const;
    SfxShell* GetTopViewShell() const;
    void Shutdown();
    void InvalidateAllSubShells (const SfxShell* pParentShell);

    /** Remove all shells from the SFX stack above and including the given
        shell.
    */
    void TakeShellsFromStack (const SfxShell* pShell);

    class UpdateLock
    {
    public:
        explicit UpdateLock (Implementation& rImpl) : mrImpl(rImpl) {mrImpl.LockUpdate();}
        ~UpdateLock() {mrImpl.UnlockUpdate();};
    private:
        Implementation& mrImpl;
    };

    /** Prevent updates of the shell stack.  While the sub shell manager is
        locked it will update its internal data structures but not alter the
        shell stack.  Use this method when there are several modifications
        to the shell stack to prevent multiple rebuilds of the shell stack
        and resulting broadcasts.
    */
    void LockUpdate();

    /** Allow updates of the shell stack.  This method has to be called the
        same number of times as LockUpdate() to really allow a rebuild of
        the shell stack.
    */
    void UnlockUpdate();

private:
    ViewShellBase& mrBase;
    mutable ::osl::Mutex maMutex;

    class ShellHash { public: size_t operator()(const SfxShell* p) const { return reinterpret_cast<size_t>(p);} };
    typedef std::unordered_multimap<const SfxShell*,SharedShellFactory,ShellHash>
        FactoryList;
    FactoryList maShellFactories;

    /** List of the active view shells.  In order to create gather all shells
        to put on the shell stack each view shell in this list is asked for
        its sub-shells (typically toolbars).
    */
    typedef std::list<ShellDescriptor> ActiveShellList;
    ActiveShellList maActiveViewShells;

    typedef std::list<ShellDescriptor> SubShellSubList;
    typedef std::unordered_map<const SfxShell*,SubShellSubList,ShellHash> SubShellList;
    SubShellList maActiveSubShells;

    /** In this member we remember what shells we have pushed on the shell
        stack.
    */
    typedef ::std::vector<SfxShell*> ShellStack;

    int mnUpdateLockCount;

    /** When this flag is set then the main view shell is always kept at the
        top of the shell stack.
    */
    bool mbKeepMainViewShellOnTop;

    /** The UpdateShellStack() method can be called recursively.  This flag
        is used to communicate between different levels of invocation: if
        the stack has been updated in an inner call the outer call can (has
        to) stop and return immediately.
    */
    bool mbShellStackIsUpToDate;

    SfxShell* mpFormShell;
    const ViewShell* mpFormShellParent;
    bool mbFormShellAboveParent;

    SfxShell* mpTopShell;
    SfxShell* mpTopViewShell;


    void UpdateShellStack();

    void CreateShells();

    /** This method rebuilds the stack of shells that are stacked upon the
        view shell base.
    */
    void CreateTargetStack (ShellStack& rStack) const;

    DECL_LINK_TYPED(WindowEventHandler, VclWindowEvent&, void);

#if OSL_DEBUG_LEVEL >= 2
    void DumpShellStack (const ShellStack& rStack);
    void DumpSfxShellStack();
#endif

    /** To be called before a shell is taken fom the SFX shell stack.  This
        method deactivates an active text editing to avoid problems with
        undo managers.
        Afterwards the Deactivate() of the shell is called.
    */
    static void Deactivate (SfxShell* pShell);

    ShellDescriptor CreateSubShell (
        SfxShell* pShell,
        ShellId nShellId,
        vcl::Window* pParentWindow,
        FrameView* pFrameView);
    void DestroyViewShell (ShellDescriptor& rDescriptor);
    static void DestroySubShell (
        const SfxShell& rViewShell,
        const ShellDescriptor& rDescriptor);
};

//===== ViewShellManager ======================================================

ViewShellManager::ViewShellManager (ViewShellBase& rBase)
    : mpImpl(new Implementation(*this,rBase)),
      mbValid(true)
{
}

ViewShellManager::~ViewShellManager()
{
}

void ViewShellManager::AddSubShellFactory (
    ViewShell* pViewShell,
    const SharedShellFactory& rpFactory)
{
    if (mbValid)
        mpImpl->AddShellFactory(pViewShell, rpFactory);
}

void ViewShellManager::RemoveSubShellFactory (
    ViewShell* pViewShell,
    const SharedShellFactory& rpFactory)
{
    if (mbValid)
        mpImpl->RemoveShellFactory(pViewShell, rpFactory);
}

void ViewShellManager::ActivateViewShell (ViewShell* pViewShell)
{
    if (mbValid)
        return mpImpl->ActivateViewShell(pViewShell);
}

void ViewShellManager::DeactivateViewShell (const ViewShell* pShell)
{
    if (mbValid && pShell!=nullptr)
        mpImpl->DeactivateViewShell(*pShell);
}

void ViewShellManager::SetFormShell (
    const ViewShell* pParentShell,
    FmFormShell* pFormShell,
    bool bAbove)
{
    if (mbValid)
        mpImpl->SetFormShell(pParentShell,pFormShell,bAbove);
}

void ViewShellManager::ActivateSubShell (const ViewShell& rViewShell, ShellId nId)
{
    if (mbValid)
        mpImpl->ActivateSubShell(rViewShell,nId);
}

void ViewShellManager::DeactivateSubShell (const ViewShell& rViewShell, ShellId nId)
{
    if (mbValid)
        mpImpl->DeactivateSubShell(rViewShell,nId);
}

void ViewShellManager::InvalidateAllSubShells (ViewShell* pViewShell)
{
    if (mbValid)
        mpImpl->InvalidateAllSubShells(pViewShell);
}

void ViewShellManager::ActivateShell (SfxShell* pShell)
{
    if (mbValid && pShell!=nullptr)
        mpImpl->ActivateShell(*pShell);
}

void ViewShellManager::DeactivateShell (const SfxShell* pShell)
{
    if (mbValid && pShell!=nullptr)
        mpImpl->DeactivateShell(*pShell);
}

void ViewShellManager::MoveToTop (const ViewShell& rParentShell)
{
    if (mbValid)
        mpImpl->MoveToTop(rParentShell);
}

SfxShell* ViewShellManager::GetShell (ShellId nId) const
{
    if (mbValid)
        return mpImpl->GetShell(nId);
    else
        return nullptr;
}

SfxShell* ViewShellManager::GetTopShell() const
{
    if (mbValid)
        return mpImpl->GetTopShell();
    else
        return nullptr;
}

SfxShell* ViewShellManager::GetTopViewShell() const
{
    if (mbValid)
        return mpImpl->GetTopViewShell();
    else
        return nullptr;
}

void ViewShellManager::Shutdown()
{
    if (mbValid)
    {
        mpImpl->Shutdown();
        mbValid = false;
    }
}

void ViewShellManager::LockUpdate()
{
    mpImpl->LockUpdate();
}

void ViewShellManager::UnlockUpdate()
{
    mpImpl->UnlockUpdate();
}

//===== ViewShellManager::Implementation ======================================

ViewShellManager::Implementation::Implementation (
    ViewShellManager& rManager,
    ViewShellBase& rBase)
    : mrBase(rBase),
      maMutex(),
      maShellFactories(),
      maActiveViewShells(),
      mnUpdateLockCount(0),
      mbKeepMainViewShellOnTop(false),
      mbShellStackIsUpToDate(true),
      mpFormShell(nullptr),
      mpFormShellParent(nullptr),
      mbFormShellAboveParent(true),
      mpTopShell(nullptr),
      mpTopViewShell(nullptr)
{
    (void)rManager;
}

ViewShellManager::Implementation::~Implementation()
{
    Shutdown();
}

void ViewShellManager::Implementation::AddShellFactory (
    const SfxShell* pViewShell,
    const SharedShellFactory& rpFactory)
{
    bool bAlreadyAdded (false);

    // Check that the given factory has not already been added.
    ::std::pair<FactoryList::iterator,FactoryList::iterator> aRange(
        maShellFactories.equal_range(pViewShell));
    for (FactoryList::const_iterator iFactory=aRange.first; iFactory!=aRange.second; ++iFactory)
        if (iFactory->second == rpFactory)
        {
            bAlreadyAdded = true;
            break;
        }

    // Add the factory if it is not already present.
    if ( ! bAlreadyAdded)
        maShellFactories.insert(FactoryList::value_type(pViewShell, rpFactory));
}

void ViewShellManager::Implementation::RemoveShellFactory (
    const SfxShell* pViewShell,
    const SharedShellFactory& rpFactory)
{
    ::std::pair<FactoryList::iterator,FactoryList::iterator> aRange(
        maShellFactories.equal_range(pViewShell));
    for (FactoryList::iterator iFactory=aRange.first; iFactory!=aRange.second; ++iFactory)
        if (iFactory->second == rpFactory)
        {
            maShellFactories.erase(iFactory);
            break;
        }
}

void ViewShellManager::Implementation::ActivateViewShell (ViewShell* pViewShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    ShellDescriptor aResult;
    aResult.mpShell = pViewShell;

    // Register as window listener so that the shells of the current
    // window can be moved to the top of the shell stack.
    if (aResult.mpShell != nullptr)
    {
        vcl::Window* pWindow = aResult.GetWindow();
        if (pWindow != nullptr)
        {
            pWindow->AddEventListener(
                LINK(this, ViewShellManager::Implementation, WindowEventHandler));
            aResult.mbIsListenerAddedToWindow = true;
        }
        else
        {
            SAL_WARN("sd",
                "ViewShellManager::ActivateViewShell: "
                "new view shell has no active window");
        }
    }

    ActivateShell(aResult);
}

void ViewShellManager::Implementation::DeactivateViewShell (const ViewShell& rShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    ActiveShellList::iterator iShell (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(&rShell)));
    if (iShell != maActiveViewShells.end())
    {
        UpdateLock aLocker (*this);

        ShellDescriptor aDescriptor(*iShell);
        mrBase.GetDocShell()->Disconnect(dynamic_cast<ViewShell*>(aDescriptor.mpShell));
        maActiveViewShells.erase(iShell);
        TakeShellsFromStack(aDescriptor.mpShell);

        // Deactivate sub shells.
        SubShellList::iterator iList (maActiveSubShells.find(&rShell));
        if (iList != maActiveSubShells.end())
        {
            SubShellSubList& rList (iList->second);
            while ( ! rList.empty())
                DeactivateSubShell(rShell, rList.front().mnId);
        }

        DestroyViewShell(aDescriptor);
    }
}

void ViewShellManager::Implementation::ActivateShell (SfxShell& rShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Create a new shell or recycle on in the cache.
    ShellDescriptor aDescriptor;
    aDescriptor.mpShell = &rShell;

    ActivateShell(aDescriptor);
}

void ViewShellManager::Implementation::ActivateShell (const ShellDescriptor& rDescriptor)
{
    // Put shell on top of the active view shells.
    if (rDescriptor.mpShell != nullptr)
    {
        // Determine where to put the view shell on the stack.  By default
        // it is put on top of the stack.  When the view shell of the center
        // pane is to be kept top most and the new view shell is not
        // displayed in the center pane then it is inserted at the position
        // one below the top.
        ActiveShellList::iterator iInsertPosition (maActiveViewShells.begin());
        if (iInsertPosition != maActiveViewShells.end()
            && mbKeepMainViewShellOnTop
            && ! rDescriptor.IsMainViewShell()
            && iInsertPosition->IsMainViewShell())
        {
            ++iInsertPosition;
        }
        maActiveViewShells.insert(
            iInsertPosition,
            rDescriptor);
    }
}

void ViewShellManager::Implementation::DeactivateShell (const SfxShell& rShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    ActiveShellList::iterator iShell (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(&rShell)));
    if (iShell != maActiveViewShells.end())
    {
        UpdateLock aLocker (*this);

        ShellDescriptor aDescriptor(*iShell);
        mrBase.GetDocShell()->Disconnect(dynamic_cast<ViewShell*>(aDescriptor.mpShell));
        maActiveViewShells.erase(iShell);
        TakeShellsFromStack(aDescriptor.mpShell);

        // Deactivate sub shells.
        SubShellList::iterator iList (maActiveSubShells.find(&rShell));
        if (iList != maActiveSubShells.end())
        {
            SubShellSubList& rList (iList->second);
            while ( ! rList.empty())
                DeactivateSubShell(rShell, rList.front().mnId);
        }

        DestroyViewShell(aDescriptor);
    }
}

void ViewShellManager::Implementation::ActivateSubShell (
    const SfxShell& rParentShell,
    ShellId nId)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Check that the given view shell is active.
    ActiveShellList::iterator iShell (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(&rParentShell)));
    if (iShell == maActiveViewShells.end())
        return;

    // Create the sub shell list if it does not yet exist.
    SubShellList::iterator iList (maActiveSubShells.find(&rParentShell));
    if (iList == maActiveSubShells.end())
        iList = maActiveSubShells.insert(
            SubShellList::value_type(&rParentShell,SubShellSubList())).first;

    // Do not activate an object bar that is already active.  Requesting
    // this is not exactly an error but may be an indication of one.
    SubShellSubList& rList (iList->second);
    if (::std::find_if(rList.begin(),rList.end(), IsId(nId)) != rList.end())
        return;

    // Add just the id of the sub shell. The actual shell is created
    // later in CreateShells().
    UpdateLock aLock (*this);
    rList.push_back(ShellDescriptor(nullptr, nId));
}

void ViewShellManager::Implementation::DeactivateSubShell (
    const SfxShell& rParentShell,
    ShellId nId)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Check that the given view shell is active.
    SubShellList::iterator iList (maActiveSubShells.find(&rParentShell));
    if (iList == maActiveSubShells.end())
        return;

    // Look up the sub shell.
    SubShellSubList& rList (iList->second);
    SubShellSubList::iterator iShell (
        ::std::find_if(rList.begin(),rList.end(), IsId(nId)));
    if (iShell == rList.end())
        return;
    SfxShell* pShell = iShell->mpShell;
    if (pShell == nullptr)
        return;

    UpdateLock aLock (*this);

    ShellDescriptor aDescriptor(*iShell);
    // Remove the sub shell from both the internal structure as well as the
    // SFX shell stack above and including the sub shell.
    rList.erase(iShell);
    TakeShellsFromStack(pShell);

    DestroySubShell(rParentShell, aDescriptor);
}

void ViewShellManager::Implementation::MoveToTop (const SfxShell& rShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Check that we have access to a dispatcher.  If not, then we are
    // (probably) called while the view shell is still being created or
    // initialized.  Without dispatcher we can not rebuild the shell stack
    // to move the requested shell to the top.  So return right away instead
    // of making a mess without being able to clean up afterwards.
    if (mrBase.GetDispatcher() == nullptr)
        return;

    ActiveShellList::iterator iShell (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(&rShell)));
    bool bMove = true;
    if (iShell != maActiveViewShells.end())
    {
        // Is the shell already at the top of the stack?  We have to keep
        // the case in mind that mbKeepMainViewShellOnTop is true.  Shells
        // that are not the main view shell are placed on the second-to-top
        // position in this case.
            if (iShell == maActiveViewShells.begin()
            && (iShell->IsMainViewShell() || ! mbKeepMainViewShellOnTop))
        {
            // The shell is at the top position and is either a) the main
            // view shell or b) another shell but the main view shell is not
            // kept at the top position.  We do not have to move the shell.
            bMove = false;
        }
        else if (iShell == ++maActiveViewShells.begin()
            && ! iShell->IsMainViewShell()
            && mbKeepMainViewShellOnTop)
        {
            // The shell is a the second-to-top position, not the main view
            // shell and the main view shell is kept at the top position.
            // Therefore we do not have to move the shell.
            bMove = false;
        }
    }
    else
    {
        // The shell is not on the stack.  Therefore it can not be moved.
        // We could insert it but we don't.  Use ActivateViewShell() for
        // that.
        bMove = false;
    }

    // When the shell is not at the right position it is removed from the
    // internal list of shells and inserted at the correct position.
    if (bMove)
    {
        UpdateLock aLock (*this);

        ShellDescriptor aDescriptor(*iShell);

        TakeShellsFromStack(&rShell);
        maActiveViewShells.erase(iShell);

        // Find out whether to insert at the top or one below.
        ActiveShellList::iterator aInsertPosition (maActiveViewShells.begin());
        if (mbKeepMainViewShellOnTop && ! aDescriptor.IsMainViewShell())
        {
            if (maActiveViewShells.back().IsMainViewShell())
                ++aInsertPosition;
        }

        maActiveViewShells.insert(aInsertPosition, aDescriptor);
    }
}

SfxShell* ViewShellManager::Implementation::GetShell (ShellId nId) const
{
    ::osl::MutexGuard aGuard (maMutex);

    SfxShell* pShell = nullptr;

    // First search the active view shells.
    ActiveShellList::const_iterator iShell (
        ::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsId(nId)));
    if (iShell != maActiveViewShells.end())
        pShell = iShell->mpShell;
    else
    {
        // Now search the active sub shells of every active view shell.
        SubShellList::const_iterator iList;
        for (iList=maActiveSubShells.begin(); iList!=maActiveSubShells.end(); ++iList)
        {
            const SubShellSubList& rList (iList->second);
            SubShellSubList::const_iterator iSubShell(
                ::std::find_if(rList.begin(),rList.end(), IsId(nId)));
            if (iSubShell != rList.end())
            {
                pShell = iSubShell->mpShell;
                break;
            }
        }
    }

    return pShell;
}

SfxShell* ViewShellManager::Implementation::GetTopShell() const
{
    OSL_ASSERT(mpTopShell == mrBase.GetSubShell(0));
    return mpTopShell;
}

SfxShell* ViewShellManager::Implementation::GetTopViewShell() const
{
    return mpTopViewShell;
}

void ViewShellManager::Implementation::LockUpdate()
{
    mnUpdateLockCount++;
}

void ViewShellManager::Implementation::UnlockUpdate()
{
    ::osl::MutexGuard aGuard (maMutex);

    mnUpdateLockCount--;
    if (mnUpdateLockCount < 0)
    {
        // This should not happen.
        OSL_ASSERT (mnUpdateLockCount>=0);
        mnUpdateLockCount = 0;
    }
    if (mnUpdateLockCount == 0)
        UpdateShellStack();
}

/** Update the SFX shell stack (the portion that is visible to us) so that
    it matches the internal shell stack.  This is done in six steps:
    1. Create the missing view shells and sub shells.
    2. Set up the internal shell stack.
    3. Get the SFX shell stack.
    4. Find the lowest shell in which the two stacks differ.
    5. Remove all shells above and including that shell from the SFX stack.
    6. Push all shells of the internal stack on the SFX shell stack that are
    not already present on the later.
*/
void ViewShellManager::Implementation::UpdateShellStack()
{
    ::osl::MutexGuard aGuard (maMutex);

    // Remember the undo manager from the top-most shell on the stack.
    SfxShell* pTopMostShell = mrBase.GetSubShell(0);
    ::svl::IUndoManager* pUndoManager = (pTopMostShell!=nullptr)
        ? pTopMostShell->GetUndoManager()
        : nullptr;

    // 1. Create the missing shells.
    CreateShells();

    // Update the pointer to the top-most active view shell.
    mpTopViewShell = (maActiveViewShells.empty())
        ? nullptr : maActiveViewShells.begin()->mpShell;


    // 2. Create the internal target stack.
    ShellStack aTargetStack;
    CreateTargetStack(aTargetStack);

    // 3. Get SFX shell stack.
    ShellStack aSfxShellStack;
    sal_uInt16 nIndex (0);
    while (mrBase.GetSubShell(nIndex)!=nullptr)
        ++nIndex;
    aSfxShellStack.reserve(nIndex);
    while (nIndex-- > 0)
        aSfxShellStack.push_back(mrBase.GetSubShell(nIndex));

#if OSL_DEBUG_LEVEL >= 2
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": Current SFX Stack");
    DumpShellStack(aSfxShellStack);
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": Target Stack");
    DumpShellStack(aTargetStack);
#endif

    // 4. Find the lowest shell in which the two stacks differ.
    ShellStack::iterator iSfxShell (aSfxShellStack.begin());
    ShellStack::iterator iTargetShell (aTargetStack.begin());
    while (iSfxShell != aSfxShellStack.end()
        && iTargetShell!=aTargetStack.end()
        && (*iSfxShell)==(*iTargetShell))
    {
        ++iSfxShell;
        ++iTargetShell;
    }

    // 5. Remove all shells above and including the differing shell from the
    // SFX stack starting with the shell on top of the stack.
    for (std::reverse_iterator<ShellStack::const_iterator> i(aSfxShellStack.end()), iLast(iSfxShell);
            i != iLast; ++i)
    {
        SfxShell* const pShell = *i;
        SAL_INFO("sd.view", OSL_THIS_FUNC << ": removing shell " << pShell << " from stack");
        mrBase.RemoveSubShell(pShell);
    }
    aSfxShellStack.erase(iSfxShell, aSfxShellStack.end());

    // 6. Push shells from the given stack onto the SFX stack.
    mbShellStackIsUpToDate = false;
    while (iTargetShell != aTargetStack.end())
    {
        SAL_INFO("sd.view", OSL_THIS_FUNC << ": pushing shell " << *iTargetShell << " on stack");
        mrBase.AddSubShell(**iTargetShell);
        ++iTargetShell;

        // The pushing of the shell on to the shell stack may have lead to
        // another invocation of this method.  In this case we have to abort
        // pushing shells on the stack and return immediately.
        if (mbShellStackIsUpToDate)
            break;
    }
    if (mrBase.GetDispatcher() != nullptr)
        mrBase.GetDispatcher()->Flush();

    // Update the pointer to the top-most shell and set its undo manager
    // to the one of the previous top-most shell.
    mpTopShell = mrBase.GetSubShell(0);
    if (mpTopShell!=nullptr && pUndoManager!=nullptr && mpTopShell->GetUndoManager()==nullptr)
        mpTopShell->SetUndoManager(pUndoManager);

    // Finally tell an invocation of this method on a higher level that it can (has
    // to) abort and return immediately.
    mbShellStackIsUpToDate = true;

#if OSL_DEBUG_LEVEL >= 2
    SAL_INFO("sd.view", OSL_THIS_FUNC << ": New current stack");
    DumpSfxShellStack();
#endif
}

void ViewShellManager::Implementation::TakeShellsFromStack (const SfxShell* pShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Remember the undo manager from the top-most shell on the stack.
    SfxShell* pTopMostShell = mrBase.GetSubShell(0);
    ::svl::IUndoManager* pUndoManager = (pTopMostShell!=nullptr)
        ? pTopMostShell->GetUndoManager()
        : nullptr;

#if OSL_DEBUG_LEVEL >= 2
    SAL_INFO("sd.view", OSL_THIS_FUNC << "TakeShellsFromStack( " << pShell << ")");
    DumpSfxShellStack();
#endif

    // 0.Make sure that the given shell is on the stack.  This is a
    // preparation for the following assertion.
    for (sal_uInt16 nIndex=0; true; nIndex++)
    {
        SfxShell* pShellOnStack = mrBase.GetSubShell(nIndex);
        if (pShellOnStack == nullptr)
        {
            // Set pShell to NULL to indicate the following code that the
            // shell is not on the stack.
            pShell = nullptr;
            break;
        }
        else if (pShellOnStack == pShell)
            break;
    }

    if (pShell != nullptr)
    {
        // 1. Deactivate our shells on the stack before they are removed so
        // that during the Deactivation() calls the stack is still intact.
        for (sal_uInt16 nIndex=0; true; nIndex++)
        {
            SfxShell* pShellOnStack = mrBase.GetSubShell(nIndex);
            Deactivate(pShellOnStack);
            if (pShellOnStack == pShell)
                break;
        }

        // 2. Remove the shells from the stack.
        while (true)
        {
            SfxShell* pShellOnStack = mrBase.GetSubShell(0);
            SAL_INFO("sd.view", OSL_THIS_FUNC << "removing shell " << pShellOnStack << " from stack");
            mrBase.RemoveSubShell(pShellOnStack);
            if (pShellOnStack == pShell)
                break;
        }

        // 3. Update the stack.
        if (mrBase.GetDispatcher() != nullptr)
            mrBase.GetDispatcher()->Flush();

        // Update the pointer to the top-most shell and set its undo manager
        // to the one of the previous top-most shell.
        mpTopShell = mrBase.GetSubShell(0);
        if (mpTopShell!=nullptr && pUndoManager!=nullptr && mpTopShell->GetUndoManager()==nullptr)
            mpTopShell->SetUndoManager(pUndoManager);
    }

#if OSL_DEBUG_LEVEL >= 2
    SAL_INFO("sd.view", OSL_THIS_FUNC << "Sfx shell stack is:");
    DumpSfxShellStack();
#endif
}

void ViewShellManager::Implementation::CreateShells()
{
    ::osl::MutexGuard aGuard (maMutex);

    // Iterate over all view shells.
    ActiveShellList::reverse_iterator iShell;
    for (iShell=maActiveViewShells.rbegin(); iShell!=maActiveViewShells.rend(); ++iShell)
    {
        // Get the list of associated sub shells.
        SubShellList::iterator iList (maActiveSubShells.find(iShell->mpShell));
        if (iList != maActiveSubShells.end())
        {
            SubShellSubList& rList (iList->second);

            // Iterate over all sub shells of the current view shell.
            SubShellSubList::iterator iSubShell;
            for (iSubShell=rList.begin(); iSubShell!=rList.end(); ++iSubShell)
            {
                if (iSubShell->mpShell == nullptr)
                {
                    *iSubShell = CreateSubShell(iShell->mpShell,iSubShell->mnId,nullptr,nullptr);
                }
            }
        }
   }
}

void ViewShellManager::Implementation::CreateTargetStack (ShellStack& rStack) const
{
    // Create a local stack of the shells that are to push on the shell
    // stack.  We can thus safely create the required shells wile still
    // having a valid shell stack.
    for (ActiveShellList::const_reverse_iterator iViewShell (maActiveViewShells.rbegin());
         iViewShell != maActiveViewShells.rend();
         ++iViewShell)
    {
        // Possibly place the form shell below the current view shell.
        if ( ! mbFormShellAboveParent
            && mpFormShell!=nullptr
            && iViewShell->mpShell==mpFormShellParent)
        {
            rStack.push_back(mpFormShell);
        }

        // Put the view shell itself on the local stack.
        rStack.push_back (iViewShell->mpShell);

        // Possibly place the form shell above the current view shell.
        if (mbFormShellAboveParent
            && mpFormShell!=nullptr
            && iViewShell->mpShell==mpFormShellParent)
        {
            rStack.push_back(mpFormShell);
        }

        // Add all other sub shells.
        SubShellList::const_iterator iList (maActiveSubShells.find(iViewShell->mpShell));
        if (iList != maActiveSubShells.end())
        {
            const SubShellSubList& rList (iList->second);
            SubShellSubList::const_reverse_iterator iSubShell;
            for (iSubShell=rList.rbegin(); iSubShell!=rList.rend(); ++iSubShell)
                if (iSubShell->mpShell != mpFormShell)
                    rStack.push_back(iSubShell->mpShell);
        }
    }
}

IMPL_LINK_TYPED(ViewShellManager::Implementation, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
        vcl::Window* pEventWindow = rEvent.GetWindow();

        switch (rEvent.GetId())
        {
            case VCLEVENT_WINDOW_GETFOCUS:
            {
                for (ActiveShellList::iterator aI(maActiveViewShells.begin());
                     aI!=maActiveViewShells.end();
                     ++aI)
                {
                    if (pEventWindow == static_cast< vcl::Window*>(aI->GetWindow()))
                    {
                        MoveToTop(*aI->mpShell);
                        break;
                    }
                }
            }
            break;

            case VCLEVENT_WINDOW_LOSEFOCUS:
                break;

            case VCLEVENT_OBJECT_DYING:
                // Remember that we do not have to remove the window
                // listener for this window.
                for (ActiveShellList::iterator
                         iShell(maActiveViewShells.begin()),
                         iEnd(maActiveViewShells.end());
                     iShell!=iEnd;
                     ++iShell)
                {
                    if (iShell->GetWindow() == pEventWindow)
                    {
                        iShell->mbIsListenerAddedToWindow = false;
                        break;
                    }
                }
                break;
        }
}

ShellDescriptor ViewShellManager::Implementation::CreateSubShell (
    SfxShell* pParentShell,
    ShellId nShellId,
    vcl::Window* pParentWindow,
    FrameView* pFrameView)
{
    ::osl::MutexGuard aGuard (maMutex);
    ShellDescriptor aResult;

    // Look up the factories for the parent shell.
    ::std::pair<FactoryList::iterator,FactoryList::iterator> aRange(
        maShellFactories.equal_range(pParentShell));

    // Try all factories to create the shell.
    for (FactoryList::const_iterator iFactory=aRange.first; iFactory!=aRange.second; ++iFactory)
    {
        SharedShellFactory pFactory = iFactory->second;
        if (pFactory != nullptr)
            aResult.mpShell = pFactory->CreateShell(nShellId, pParentWindow, pFrameView);

        // Exit the loop when the shell has been successfully created.
        if (aResult.mpShell != nullptr)
        {
            aResult.mpFactory = pFactory;
            aResult.mnId = nShellId;
            break;
        }
    }

    return aResult;
}

void ViewShellManager::Implementation::DestroyViewShell (
    ShellDescriptor& rDescriptor)
{
    OSL_ASSERT(rDescriptor.mpShell != nullptr);

    if (rDescriptor.mbIsListenerAddedToWindow)
    {
        rDescriptor.mbIsListenerAddedToWindow = false;
        vcl::Window* pWindow = rDescriptor.GetWindow();
        if (pWindow != nullptr)
        {
            pWindow->RemoveEventListener(
                LINK(this, ViewShellManager::Implementation, WindowEventHandler));
        }
    }

    // Destroy the sub shell factories.
    ::std::pair<FactoryList::iterator,FactoryList::iterator> aRange(
        maShellFactories.equal_range(rDescriptor.mpShell));
    if (aRange.first != maShellFactories.end())
        maShellFactories.erase(aRange.first, aRange.second);

    // Release the shell.
    if (rDescriptor.mpFactory.get() != nullptr)
        rDescriptor.mpFactory->ReleaseShell(rDescriptor.mpShell);
}

void ViewShellManager::Implementation::DestroySubShell (
    const SfxShell& rParentShell,
    const ShellDescriptor& rDescriptor)
{
    (void)rParentShell;
    OSL_ASSERT(rDescriptor.mpFactory.get() != nullptr);
    rDescriptor.mpFactory->ReleaseShell(rDescriptor.mpShell);
}

void ViewShellManager::Implementation::InvalidateAllSubShells (const SfxShell* pParentShell)
{
    ::osl::MutexGuard aGuard (maMutex);

    SubShellList::iterator iList (maActiveSubShells.find(pParentShell));
    if (iList != maActiveSubShells.end())
    {
        SubShellSubList& rList (iList->second);
        SubShellSubList::iterator iShell;
        for (iShell=rList.begin(); iShell!=rList.end(); ++iShell)
            if (iShell->mpShell != nullptr)
                iShell->mpShell->Invalidate();
    }
}

void ViewShellManager::Implementation::Shutdown()
{
    ::osl::MutexGuard aGuard (maMutex);

    // Take stacked shells from stack.
    if ( ! maActiveViewShells.empty())
    {
        UpdateLock aLock (*this);

        while ( ! maActiveViewShells.empty())
        {
            SfxShell* pShell = maActiveViewShells.front().mpShell;
            if (pShell != nullptr)
            {
                ViewShell* pViewShell = dynamic_cast<ViewShell*>(pShell);
                if (pViewShell != nullptr)
                    DeactivateViewShell(*pViewShell);
                else
                    DeactivateShell(*pShell);
            }
            else
            {
                SAL_WARN("sd",
                    "ViewShellManager::Implementation::Shutdown(): empty active shell descriptor");
                maActiveViewShells.pop_front();
            }
        }
    }
    mrBase.RemoveSubShell ();

    maShellFactories.clear();
}

#if OSL_DEBUG_LEVEL >= 2
void ViewShellManager::Implementation::DumpShellStack (const ShellStack& rStack)
{
    ShellStack::const_reverse_iterator iEntry;
    for (iEntry=rStack.rbegin(); iEntry!=rStack.rend(); ++iEntry)
        if (*iEntry != NULL)
            SAL_INFO("sd.view", OSL_THIS_FUNC << ":    " <<
                *iEntry << " : " <<
                OUStringToOString((*iEntry)->GetName(),RTL_TEXTENCODING_UTF8).getStr());
        else
            SAL_INFO("sd.view", OSL_THIS_FUNC << "     null");
}

void ViewShellManager::Implementation::DumpSfxShellStack()
{
    ShellStack aSfxShellStack;
    sal_uInt16 nIndex (0);
    while (mrBase.GetSubShell(nIndex)!=NULL)
        ++nIndex;
    aSfxShellStack.reserve(nIndex);
    while (nIndex-- > 0)
        aSfxShellStack.push_back(mrBase.GetSubShell(nIndex));
    DumpShellStack(aSfxShellStack);
}
#endif

void ViewShellManager::Implementation::Deactivate (SfxShell* pShell)
{
    OSL_ASSERT(pShell!=nullptr);

    // We have to end a text edit for view shells that are to be taken from
    // the shell stack.
    ViewShell* pViewShell = dynamic_cast<ViewShell*>(pShell);
    if (pViewShell != nullptr)
    {
        sd::View* pView = pViewShell->GetView();
        if (pView!=nullptr && pView->IsTextEdit())
        {
            pView->SdrEndTextEdit();
            pView->UnmarkAll();
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_OBJECT_SELECT,
                SfxCallMode::ASYNCHRON);
        }
    }

    // Now we can deactivate the shell.
    pShell->Deactivate(true);
}

void ViewShellManager::Implementation::SetFormShell (
    const ViewShell* pFormShellParent,
    FmFormShell* pFormShell,
    bool bFormShellAboveParent)
{
    ::osl::MutexGuard aGuard (maMutex);

    mpFormShellParent = pFormShellParent;
    mpFormShell = pFormShell;
    mbFormShellAboveParent = bFormShellAboveParent;
}

namespace {

ShellDescriptor::ShellDescriptor()
    : mpShell(nullptr),
      mnId(0),
      mpFactory(),
      mbIsListenerAddedToWindow(false)
{
}

ShellDescriptor::ShellDescriptor (
    SfxShell* pShell,
    ShellId nId)
    : mpShell(pShell),
      mnId(nId),
      mpFactory(),
      mbIsListenerAddedToWindow(false)
{
}

ShellDescriptor::ShellDescriptor (const ShellDescriptor& rDescriptor)
    : mpShell(rDescriptor.mpShell),
      mnId(rDescriptor.mnId),
      mpFactory(rDescriptor.mpFactory),
      mbIsListenerAddedToWindow(rDescriptor.mbIsListenerAddedToWindow)
{
}

ShellDescriptor& ShellDescriptor::operator= (const ShellDescriptor& rDescriptor)
{
    if (this != &rDescriptor)
    {
        mpShell = rDescriptor.mpShell;
        mnId = rDescriptor.mnId;
        mpFactory = rDescriptor.mpFactory;
        mbIsListenerAddedToWindow = rDescriptor.mbIsListenerAddedToWindow;
    }
    return *this;
}

bool ShellDescriptor::IsMainViewShell() const
{
    ViewShell* pViewShell = dynamic_cast<ViewShell*>(mpShell);
    if (pViewShell != nullptr)
        return pViewShell->IsMainViewShell();
    else
        return false;
}

vcl::Window* ShellDescriptor::GetWindow() const
{
    ViewShell* pViewShell = dynamic_cast<ViewShell*>(mpShell);
    if (pViewShell != nullptr)
        return pViewShell->GetActiveWindow();
    else
        return nullptr;
}

} // end of anonymous namespace

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
