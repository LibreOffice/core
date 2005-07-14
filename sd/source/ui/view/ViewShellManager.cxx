/*************************************************************************
 *
 *  $RCSfile: ViewShellManager.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:45:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "ViewShellManager.hxx"

#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "ViewShellCache.hxx"
#include "Window.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif

#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>

#include <hash_map>

#undef VERBOSE

namespace sd {

namespace {

class ActiveShellDescriptor {
public:
    ActiveShellDescriptor (ViewShell* pViewShell, ShellId nId)
        : mpViewShell(pViewShell), mnId(nId) {}
    ViewShell* mpViewShell;
    ShellId mnId;
};

class IsShell : public ::std::unary_function<ActiveShellDescriptor,bool>
{
public:
    IsShell (const ::sd::ViewShell* pShell) : mpShell(pShell) {}
    bool operator() (const ActiveShellDescriptor& rDescriptor)
    { return rDescriptor.mpViewShell == mpShell; }
private:
    const ViewShell* mpShell;
};

class IsId : public ::std::unary_function<ActiveShellDescriptor,bool>
{
public:
    IsId (ShellId nId) : mnId(nId) {}
    bool operator() (const ActiveShellDescriptor& rDescriptor)
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
    ~Implementation (void);

    void RegisterDefaultFactory (::std::auto_ptr<ViewShellFactory> pFactory);
    void RegisterFactory (
        ShellId nId,
        ::std::auto_ptr<ViewShellFactory> pFactory);
    ViewShell* ActivateViewShell (
        ShellId nId,
        ::Window* pParentWindow,
        FrameView* pFrameView);
    void DeactivateViewShell (const ViewShell* pShell);
    void MoveToTop (const ViewShell* pShell);
    ViewShell* GetShell (ShellId nId);
    ShellId GetShellId (const ViewShell* pShell);
    ViewShellBase& GetViewShellBase (void) const;
    void Shutdown (void);
    ViewShell* CreateViewShell (
        ShellId nShellId,
        ::Window* pParentWindow,
        FrameView* pFrameView);

    /** Remove all shells from the SFX stack above and including the given
        shell.
    */
    void TakeShellsFromStack (const SfxShell* pShell);

    /** Prevent updates of the shell stack.  While the sub shell manager is
        locked it will update its internal data structures but not alter the
        shell stack.  Use this method when there are several modifications
        to the shell stack to prevent multiple rebuilds of the shell stack
        and resulting broadcasts.
    */
    void LockUpdate (void);

    /** Allow updates of the shell stack.  This method has to be called the
        same number of times as LockUpdate() to really allow a rebuild of
        the shell stack.
        */
    void UnlockUpdate (void);

private:
    ViewShellBase& mrBase;

    ::std::auto_ptr<ViewShellFactory> mpDefaultFactory;

    typedef ::std::hash_map<ShellId,ViewShellFactory*> SpecializedFactoryList;
    SpecializedFactoryList maSpecializedFactories;

    /** List of the active view shells.  In order to create gather all shells
        to put on the shell stack each view shell in this list is asked for
        its sub-shells (typically toolbars).
    */
    typedef ::std::list<ActiveShellDescriptor> ActiveShellList;
    ActiveShellList maActiveViewShells;

    /** In this member we remember what shells we have pushed on the shell
        stack.
    */
    typedef ::std::vector<SfxShell*> ShellStack;

    ViewShellCache maCache;

    int mnUpdateLockCount;

    /** When this flag is set then the main view shell is always kept at the
        top of the shell stack.
    */
    bool mbKeepMainViewShellOnTop;

    /** The PushShellsOnStack() method can be called recursively.  This flag
        is used to communicate between different levels of invocation: if
        the stack has been updated in an inner call the outer call can (has
        to) stop and return immediately.
    */
    bool mbShellStackIsUpToDate;

    void GatherActiveShells (ShellStack& rShellList);

    /** This method rebuilds the stack of shells that are stacked upon the
        view shell base.
    */
    void PushShellsOnStack (void);

    DECL_LINK(WindowEventHandler, VclWindowEvent*);

    void DumpActiveShell (const ActiveShellList& rList);
    void DumpShellStack (const ShellStack& rStack);
    void DumpSfxShellStack (void);

    void UpdateShellStack (ShellStack& rRequestedStack);

    /** To be called before a shell is taken fom the SFX shell stack.  This
        method deactivates an active text editing to avoid problems with
        undo managers.
        Afterwards the Deactivate() of the shell is called.
    */
    void Deactivate (SfxShell* pShell);
};




//===== ViewShellManager ======================================================

ViewShellManager::ViewShellManager (ViewShellBase& rBase)
    : mpImpl(new Implementation(*this,rBase)),
      mbValid(true)
{
}




ViewShellManager::~ViewShellManager (void)
{
}




void ViewShellManager::RegisterDefaultFactory (
    ::std::auto_ptr<ViewShellFactory> pFactory)
{
    if (mbValid)
        mpImpl->RegisterDefaultFactory(pFactory);
}




void ViewShellManager::RegisterFactory (
    ShellId nId,
    ::std::auto_ptr<ViewShellFactory> pFactory)
{
    if (mbValid)
        mpImpl->RegisterFactory(nId,pFactory);
}




ViewShell* ViewShellManager::ActivateViewShell (
    ShellId nShellId,
    ::Window* pParentWindow,
    FrameView* pFrameView)
{
    if (mbValid)
        return mpImpl->ActivateViewShell(nShellId,pParentWindow,pFrameView);
    else
        return NULL;
}




void ViewShellManager::DeactivateViewShell (const ViewShell* pShell)
{
    if (mbValid)
        mpImpl->DeactivateViewShell(pShell);
}




void ViewShellManager::InvalidateShellStack (const SfxShell* pShell)
{
    if (mbValid)
        mpImpl->TakeShellsFromStack(pShell);
}




void ViewShellManager::MoveToTop (const ViewShell* pShell)
{
    if (mbValid)
        mpImpl->MoveToTop(pShell);
}




ViewShell* ViewShellManager::GetShell (ShellId nId)
{
    if (mbValid)
        return mpImpl->GetShell(nId);
    else
        return NULL;
}




ShellId ViewShellManager::GetShellId (const ViewShell* pShell)
{
    if (mbValid)
        return mpImpl->GetShellId(pShell);
    else
        return snInvalidShellId;
}




ViewShellBase& ViewShellManager::GetViewShellBase (void) const
{
    return mpImpl->GetViewShellBase();
}




void ViewShellManager::Shutdown (void)
{
    if (mbValid)
    {
        mpImpl->Shutdown();
        mbValid = false;
    }
}




ViewShell* ViewShellManager::CreateViewShell (
    ShellId nShellId,
    ::Window* pParentWindow,
    FrameView* pFrameView)
{
    if (mbValid)
        return mpImpl->CreateViewShell(nShellId,pParentWindow,pFrameView);
    else
        return NULL;
}




//===== ViewShellManager::Implementation ======================================

ViewShellManager::Implementation::Implementation (
    ViewShellManager& rManager,
    ViewShellBase& rBase)
    : mrBase(rBase),
      mpDefaultFactory (NULL),
      maSpecializedFactories (),
      maActiveViewShells(),
      maCache (rManager),
      mnUpdateLockCount(0),
      mbKeepMainViewShellOnTop(false),
      mbShellStackIsUpToDate(true)
{
}




ViewShellManager::Implementation::~Implementation (void)
{
    Shutdown();
}


void ViewShellManager::Implementation::RegisterDefaultFactory (
    ::std::auto_ptr<ViewShellFactory> pFactory)
{
    mpDefaultFactory = pFactory;
}




void ViewShellManager::Implementation::RegisterFactory (
    ShellId nId,
    ::std::auto_ptr<ViewShellFactory> pFactory)
{
    maSpecializedFactories[nId] = pFactory.get();
    pFactory.release();
}




ViewShell* ViewShellManager::Implementation::ActivateViewShell (
    ShellId nShellId,
    ::Window* pParentWindow,
    FrameView* pFrameView)
{
    // Create a new shell or recycle on in the cache.
    ViewShell* pViewShell = maCache.GetViewShell (
        nShellId,
        pParentWindow,
        pFrameView);

    // Put shell on top of the active view shells.
    if (pViewShell != NULL)
    {
        ::Window* pWindow = pViewShell->GetActiveWindow();
        if (pWindow != NULL)
            pWindow->AddEventListener(
                LINK(
                    this,
                    ViewShellManager::Implementation,
                    WindowEventHandler));
        else
        {
            DBG_ASSERT (pViewShell->GetActiveWindow()!=NULL,
                "ViewShellManager::ActivateViewShell: "
                "new view shell has no active window");
        }
        // Determine where to put the view shell on the stack.  By default
        // it is put on top of the stack.  When the view shell of the center
        // pane is to be kept top most and the new view shell is not
        // displayed in the center pane then it is inserted at the position
        // one below the top.
        ActiveShellList::iterator iInsertPosition (
            maActiveViewShells.begin());
        if (iInsertPosition != maActiveViewShells.end()
            && mbKeepMainViewShellOnTop
            && ! pViewShell->IsMainViewShell()
            && iInsertPosition->mpViewShell->IsMainViewShell())
        {
            ++iInsertPosition;
        }
        maActiveViewShells.insert(
            iInsertPosition,
            ActiveShellDescriptor(pViewShell, nShellId));
    }

    return pViewShell;
}




void ViewShellManager::Implementation::DeactivateViewShell (const ViewShell* pShell)
{
    ActiveShellList::iterator aI (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(pShell)));
    if (aI != maActiveViewShells.end())
    {
        UpdateLock aLocker (*this);

        ViewShell* pViewShell = aI->mpViewShell;
        mrBase.GetDocShell()->Disconnect(pViewShell);
        pViewShell->GetObjectBarManager().Clear();
        maActiveViewShells.erase (aI);
        pViewShell->GetActiveWindow()->RemoveEventListener(
            LINK(
                this,
                ViewShellManager::Implementation,
                WindowEventHandler));
        TakeShellsFromStack(pViewShell);
        maCache.ReleaseViewShell (pViewShell);
    }
}




void ViewShellManager::Implementation::MoveToTop (const ViewShell* pShell)
{
    ActiveShellList::iterator aI (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(pShell)));
    bool bMove = true;
    if (aI != maActiveViewShells.end())
    {
        // Is the shell already at the top of the stack?  We have to keep
        // the case in mind that mbKeepMainViewShellOnTop is true.  Shells
        // that are not the main view shell are placed on the second-to-top
        // position in this case.
            if (aI == maActiveViewShells.begin()
            && (aI->mpViewShell->IsMainViewShell() || ! mbKeepMainViewShellOnTop))
        {
            // The shell is at the top position and is either a) the main
            // view shell or b) another shell but the main view shell is not
            // kept at the top position.  We do not have to move the shell.
            bMove = false;
        }
        else if (aI == ++maActiveViewShells.begin()
            && ! aI->mpViewShell->IsMainViewShell()
            && mbKeepMainViewShellOnTop)
        {
            // The shell is a the second-to-top position, not the main view
            // shell and the main view shell is kept at the top position.
            // Therefore we do not have to move the shell.
            bMove = false;
        }
    }
    else
        // The shell is not on the stack.  Therefore it can not be moved.
        // We could insert it but we don't.  Use ActivateViewShell() for
        // that.
        bMove = false;

    // When the shell is not at the right position it is removed from the
    // internal list of shells and inserted at the correct position.
    if (bMove)
    {
        UpdateLock aLock (*this);
        TakeShellsFromStack(pShell);

        ViewShell* pNonConstViewShell = aI->mpViewShell;
        ShellId nId = aI->mnId;
        maActiveViewShells.erase(aI);

        // Find out whether to insert at the top or one below.
        ActiveShellList::iterator aInsertPosition (
            maActiveViewShells.begin());
        if (mbKeepMainViewShellOnTop && ! aI->mpViewShell->IsMainViewShell())
        {
            if (maActiveViewShells.back().mpViewShell->IsMainViewShell())
                aInsertPosition++;
        }

        maActiveViewShells.insert (
            aInsertPosition,
            ActiveShellDescriptor(pNonConstViewShell,nId));
    }
}




ViewShell* ViewShellManager::Implementation::GetShell (ShellId nId)
{
    ActiveShellList::iterator aI (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsId(nId)));
    if (aI != maActiveViewShells.end())
        return aI->mpViewShell;
    else
        return NULL;
}




ShellId ViewShellManager::Implementation::GetShellId (const ViewShell* pShell)
{
    ActiveShellList::iterator aI (::std::find_if (
        maActiveViewShells.begin(),
        maActiveViewShells.end(),
        IsShell(pShell)));
    if (aI != maActiveViewShells.end())
        return aI->mnId;
    else
        return snInvalidShellId;
}




void ViewShellManager::Implementation::LockUpdate (void)
{
    mnUpdateLockCount++;
}




void ViewShellManager::Implementation::UnlockUpdate (void)
{
    mnUpdateLockCount--;
    if (mnUpdateLockCount < 0)
    {
        // This should not happen.
        OSL_ASSERT (mnUpdateLockCount>=0);
        mnUpdateLockCount = 0;
    }
    if (mnUpdateLockCount == 0)
        PushShellsOnStack();
}




void ViewShellManager::Implementation::PushShellsOnStack (void)
{
    // Create a local stack of the shells that are to push on the shell
    // stack.  We can thus safly create the required shells wile still
    // having a valid shell stack.
    ShellStack aShellStack;
    for (ActiveShellList::reverse_iterator aI (maActiveViewShells.rbegin());
         aI!=maActiveViewShells.rend();
         ++aI)
    {
        // Get all its sub shells that are placed below the shell.
        aI->mpViewShell->GetLowerShellList (aShellStack);

        // Put the shell itself on the local stack.
        aShellStack.push_back (aI->mpViewShell);

        // Get all its sub shells that are placed above the shell.
        aI->mpViewShell->GetUpperShellList (aShellStack);
    }

    // Now update the SFX stack to look like the local stack.
    UpdateShellStack (aShellStack);
}




/** Update the SFX shell stack (the portion that is visible to us) so that
    it matches the internal shell stack.  Since we can not look at the
    shells on the SFX stack directly we use the copy that we created the
    last time when we rebuilt the SFX stack. This is done in three steps:
    1. Find the lowest shell in which the two stack differ.
    2. Remove all shells above and including that shell from the SFX stack.
    This should not be necessary because deactivated shells should have been
    removed already.
    3. Push all shells of the internal stack on the SFX shell stack that are
    not already present on the later.
*/
void ViewShellManager::Implementation::UpdateShellStack (ShellStack& rRequestedStack)
{
    ShellStack aSfxShellStack;
    ShellStack::const_iterator iSfxShell;
    ShellStack::iterator iRequestedShell;

    mbShellStackIsUpToDate = false;

#ifdef VERBOSE
    OSL_TRACE("UpdateShellStack");
    OSL_TRACE("Current SFX Stack");
    DumpSfxShellStack();
    OSL_TRACE("Requested Stack");
    DumpShellStack(rRequestedStack);
#endif

    // 1. Put the SFX shell stack in a local data structure so that it can
    // be searched properly.
    for (USHORT nIndex=0; true; nIndex++)
    {
        if (mrBase.GetSubShell(nIndex)==NULL)
        {
            aSfxShellStack.reserve(nIndex);
            while (nIndex-- > 0)
                aSfxShellStack.push_back(mrBase.GetSubShell(nIndex));
            break;
        }
    }

    // 2. Find the lowest shell in which the two stacks differ.
    iSfxShell = aSfxShellStack.begin();
    iRequestedShell = rRequestedStack.begin();
    while (iSfxShell != aSfxShellStack.end()
        && iRequestedShell!=rRequestedStack.end()
        && (*iSfxShell)==(*iRequestedShell))
    {
        ++iSfxShell;
        ++iRequestedShell;
    }

    // 3. Remove all shells above and including the differing shell from the
    // SFX stack starting with the shell on top of the stack.
    while (iSfxShell != aSfxShellStack.end())
    {
        SfxShell* pShell = aSfxShellStack.back();
        aSfxShellStack.pop_back();
#ifdef VERBOSE
        OSL_TRACE("removing shell %p from stack", pShell);
#endif
        mrBase.RemoveSubShell(pShell);
    }

    // 4. Push shells from the given stack onto the SFX stack.
    while (iRequestedShell != rRequestedStack.end())
    {
#ifdef VERBOSE
        OSL_TRACE("pushing shell %p on stack", *iRequestedShell);
#endif
        mrBase.AddSubShell(**iRequestedShell);
        ++iRequestedShell;

        // The pushing of the shell on to the shell stack may have lead to
        // another invocation of this method.  In this case we have to abort
        // pushing shells on the stack and return immediately.
        if (mbShellStackIsUpToDate)
            break;
    }

    // 5. Tell the dispatcher to update the SFX shell.
    if (mrBase.GetDispatcher() != NULL)
        mrBase.GetDispatcher()->Flush();

    // 6. Finally Tell an invocation of this method on a higher level that it can (has
    // to) abort and return immediately.
    mbShellStackIsUpToDate = true;

#ifdef VERBOSE
    OSL_TRACE("New current stack");
    DumpSfxShellStack();
#endif
}




void ViewShellManager::Implementation::TakeShellsFromStack (const SfxShell* pShell)
{
#ifdef VERBOSE
    OSL_TRACE("TakeShellsFromStack(%p)", pShell);
    DumpSfxShellStack();
#endif

    // 0.Make sure that the given shell is on the stack.  This is a
    // preparation for the following assertion.
    for (USHORT nIndex=0; true; nIndex++)
    {
        SfxShell* pShellOnStack = mrBase.GetSubShell(nIndex);
        if (pShellOnStack == NULL)
        {
            // Set pShell to NULL to indicate the following code that the
            // shell is not on the stack.
            pShell = NULL;
            break;
        }
        else if (pShellOnStack == pShell)
            break;
    }

    if (pShell != NULL)
    {
        // 1. Deactivate our shells on the stack before they are removed so
        // that during the Deactivation() calls the stack is still intact.
        for (USHORT nIndex=0; true; nIndex++)
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
#ifdef VERBOSE
            OSL_TRACE("removing shell %p from stack", pShellOnStack);
#endif
            mrBase.RemoveSubShell(pShellOnStack);
            if (pShellOnStack == pShell)
                break;
        }

        // 3. Update the stack.
        if (mrBase.GetDispatcher() != NULL)
            mrBase.GetDispatcher()->Flush();
    }

#ifdef VERBOSE
    OSL_TRACE("Sfx shell stack is:");
    DumpSfxShellStack();
#endif
}




ViewShellBase& ViewShellManager::Implementation::GetViewShellBase (void) const
{
    return mrBase;
}




IMPL_LINK(ViewShellManager::Implementation, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        ::Window* pEventWindow
            = static_cast<VclWindowEvent*>(pEvent)->GetWindow();

        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_GETFOCUS:
            {
                for (ActiveShellList::iterator aI(maActiveViewShells.begin());
                     aI!=maActiveViewShells.end();
                     aI++)
                {
                    if (pEventWindow == static_cast< ::Window*>(
                        aI->mpViewShell->GetActiveWindow()))
                    {
                        MoveToTop (aI->mpViewShell);
                        break;
                    }
                }
            }
            break;

            case VCLEVENT_WINDOW_LOSEFOCUS:
                break;
        }
    }
    return TRUE;
}




ViewShell* ViewShellManager::Implementation::CreateViewShell (
    ShellId nShellId,
    ::Window* pParentWindow,
    FrameView* pFrameView)
{
    ViewShell* pViewShell = NULL;

    // Initialize with the default factory.
    ViewShellFactory* pFactory = mpDefaultFactory.get();

    // Look up a specialized factory.
    SpecializedFactoryList::iterator aI (maSpecializedFactories.find(nShellId));
    if (aI != maSpecializedFactories.end())
        pFactory = aI->second;

    UpdateLock aLocker (*this);

    if (pFactory!=NULL && pParentWindow!=NULL)
    {
        // Create view shell with the factory.
        pViewShell = pFactory->CreateShell (
            nShellId,
            pParentWindow,
            pFrameView);
    }

    return pViewShell;
}




void ViewShellManager::Implementation::Shutdown (void)
{
    // Take stacked shells from stack.
    if ( ! maActiveViewShells.empty())
    {
        UpdateLock aLock (*this);

        while ( ! maActiveViewShells.empty())
        {
            DeactivateViewShell(maActiveViewShells.front().mpViewShell);
        }
    }
    mrBase.RemoveSubShell (NULL);

    // We have the ownership of the factories and because they can not be
    // auto_ptrs in an stl list we have to delete them now by hand.
    for (SpecializedFactoryList::iterator aI (maSpecializedFactories.begin());
         aI!=maSpecializedFactories.end();
         aI++)
    {
        delete aI->second;
    }

    // Destroy members.
    maCache.Shutdown();
    mpDefaultFactory.reset();
}




void ViewShellManager::Implementation::DumpActiveShell (const ActiveShellList& rList)
{
    ActiveShellList::const_iterator aI;
    for (aI=rList.begin(); aI!=rList.end(); ++aI)
    {
        OSL_TRACE ("    %d %p",
            aI->mnId,
            aI->mpViewShell);
    }
}




void ViewShellManager::Implementation::DumpShellStack (const ShellStack& rStack)
{
    ShellStack::const_reverse_iterator iEntry;
    for (iEntry=rStack.rbegin(); iEntry!=rStack.rend(); ++iEntry)
        OSL_TRACE ("    %p: %s",
            *iEntry,
            ::rtl::OUStringToOString((*iEntry)->GetName(),RTL_TEXTENCODING_UTF8).getStr());
}




void ViewShellManager::Implementation::DumpSfxShellStack (void)
{
    USHORT nIndex;
    for (nIndex=0; true; nIndex++)
    {
        SfxShell* pShell = mrBase.GetSubShell(nIndex);
        if (pShell == NULL)
            break;
        OSL_TRACE ("    %d : %p: %s",
            nIndex,
            pShell,
            ::rtl::OUStringToOString(pShell->GetName(),RTL_TEXTENCODING_UTF8).getStr());
    }
}




void ViewShellManager::Implementation::Deactivate (SfxShell* pShell)
{
    OSL_ASSERT(pShell!=NULL);

    // We have to end a text edit for view shells that are to be taken from
    // the shell stack.
    ViewShell* pViewShell = dynamic_cast<ViewShell*>(pShell);
    if (pViewShell != NULL)
    {
        sd::View* pView = pViewShell->GetView();
        if (pView!=NULL && pView->IsTextEdit())
        {
            pView->EndTextEdit();
            pView->UnmarkAll();
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_OBJECT_SELECT,
                SFX_CALLMODE_ASYNCHRON);
        }
    }

    // Now we can deactivate the shell.
    pShell->Deactivate(TRUE);
}




//===== ViewShellManager::UpdateLock ==========================================

ViewShellManager::UpdateLock::UpdateLock (ViewShellManager& rManager)
    : mrManagerImplementation(*rManager.mpImpl.get())
{
    mrManagerImplementation.LockUpdate();
}




ViewShellManager::UpdateLock::UpdateLock (ViewShellManager::Implementation& rManagerImplementation)
    : mrManagerImplementation(rManagerImplementation)
{
    mrManagerImplementation.LockUpdate();
}




ViewShellManager::UpdateLock::~UpdateLock (void)
{
    mrManagerImplementation.UnlockUpdate();
}




} // end of namespace sd

