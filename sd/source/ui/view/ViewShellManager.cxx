/*************************************************************************
 *
 *  $RCSfile: ViewShellManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:53:38 $
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

#include <sfx2/dispatch.hxx>

#include <hash_map>


namespace sd {

class ViewShellManager::SpecializedFactoryList
    : public ::std::hash_map<ShellId, ViewShellFactory*>{};
class ActiveShellDescriptor {
public:
    ActiveShellDescriptor (ViewShell* pViewShell, ShellId nId)
        : mpViewShell(pViewShell), mnId(nId), mbIsOnStack(false) {}
    ViewShell* mpViewShell;
    ShellId mnId;
    bool mbIsOnStack;
};
class ViewShellManager::ActiveShellList
    : public ::std::list<ActiveShellDescriptor>{};

namespace {
class IsShell
{
public:
    IsShell (const ::sd::ViewShell* pShell) : mpShell(pShell) {}
    bool operator() (const ActiveShellDescriptor& rDescriptor)
    {
        return rDescriptor.mpViewShell == mpShell;
    }
    const ViewShell* mpShell;
};
class IsId
{
public:
    IsId (ShellId nId) : mnId(nId) {}
    bool operator() (const ActiveShellDescriptor& rDescriptor)
    {
        return rDescriptor.mnId == mnId;
    }
    ShellId mnId;
};

#define TRACE_ACTIVE_SHELLS \
    {ActiveShellList::const_iterator aI (mpActiveViewShells->begin());\
        for (; aI!=mpActiveViewShells->end(); ++aI)    { \
            OSL_TRACE ("    %d %p", aI->mnId, aI->mpViewShell);    } }

}




ViewShellManager::ViewShellManager (ViewShellBase& rBase)
    : mrBase(rBase),
      mpDefaultFactory (NULL),
      mpSpecializedFactories (
          ::std::auto_ptr<SpecializedFactoryList>(
              new SpecializedFactoryList())),
      mpActiveViewShells(
          ::std::auto_ptr<ActiveShellList>(
              new ActiveShellList())),
      mpCache (new ViewShellCache(*this)),
      mnUpdateLockCount(0),
      mbKeepMainViewShellOnTop(false),
      mbIsValid (true)
{
}




ViewShellManager::~ViewShellManager (void)
{
    LockUpdate();
    while ( ! mpActiveViewShells->empty())
    {
        DeactivateViewShell(mpActiveViewShells->front().mpViewShell);
    }
    UnlockUpdate();

    // We have the ownership of the factories and because they can not be
    // auto_ptrs in an stl list we have to delete them now.
    for (SpecializedFactoryList::iterator aI (mpSpecializedFactories->begin());
         aI!=mpSpecializedFactories->end();
         aI++)
    {
        delete aI->second;
    }
}




void ViewShellManager::RegisterDefaultFactory (
    ::std::auto_ptr<ViewShellFactory> pFactory)
{
    mpDefaultFactory = pFactory;
}




void ViewShellManager::RegisterFactory (
    ShellId nId,
    ::std::auto_ptr<ViewShellFactory> pFactory)
{
    (*mpSpecializedFactories)[nId] = pFactory.get();
    pFactory.release();
}




ViewShell* ViewShellManager::ActivateViewShell (
    ShellId nShellId,
    ::Window* pParentWindow,
    FrameView* pFrameView)
{
    ViewShell* pViewShell = mpCache->GetViewShell (
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
                    ViewShellManager,
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
            mpActiveViewShells->begin());
        if (iInsertPosition != mpActiveViewShells->end()
            && mbKeepMainViewShellOnTop
            && ! pViewShell->IsMainViewShell()
            && iInsertPosition->mpViewShell->IsMainViewShell())
        {
            ++iInsertPosition;
        }
        mpActiveViewShells->insert(
            iInsertPosition,
            ActiveShellDescriptor(pViewShell, nShellId));
    }

    return pViewShell;
}




void ViewShellManager::DeactivateViewShell (const ViewShell* pShell)
{
    ActiveShellList::iterator aI (::std::find_if (
        mpActiveViewShells->begin(),
        mpActiveViewShells->end(),
        IsShell(pShell)));
    if (aI != mpActiveViewShells->end())
    {
        UpdateLocker aLocker (*this);

        ViewShell* pViewShell = aI->mpViewShell;
        pViewShell->GetObjectBarManager().Clear();
        mpActiveViewShells->erase (aI);
        pViewShell->GetActiveWindow()->RemoveEventListener(
            LINK(
                this,
                ViewShellManager,
                WindowEventHandler));
        mrBase.RemoveSubShell(pViewShell);
        mpCache->ReleaseViewShell (pViewShell);
    }
}




void ViewShellManager::MoveToTop (const ViewShell* pShell)
{
    ActiveShellList::iterator aI (::std::find_if (
        mpActiveViewShells->begin(),
        mpActiveViewShells->end(),
        IsShell(pShell)));
    bool bMove = false;
    if (aI != mpActiveViewShells->end())
    {
        // Is the shell already at the top of the stack?
        if (aI->mpViewShell->IsMainViewShell()
            && aI == mpActiveViewShells->begin())
            bMove = false;
        else if (mbKeepMainViewShellOnTop
            && ! aI->mpViewShell->IsMainViewShell()
            && aI == ++mpActiveViewShells->begin())
            bMove = false;
        else
            bMove = true;
    }

    if (bMove)
    {
        LockUpdate();
        ViewShell* pNonConstViewShell = aI->mpViewShell;
        ShellId nId = aI->mnId;
        mpActiveViewShells->erase (aI);

        // Find out whether to insert at the top or one below.
        ActiveShellList::iterator aInsertPosition (
            mpActiveViewShells->begin());
        if (mbKeepMainViewShellOnTop && ! aI->mpViewShell->IsMainViewShell())
        {
            if (mpActiveViewShells->back().mpViewShell->IsMainViewShell())
                aInsertPosition++;
        }

        mpActiveViewShells->insert (
            aInsertPosition,
            ActiveShellDescriptor(pNonConstViewShell,nId));
        UnlockUpdate();
    }
}




ViewShell* ViewShellManager::GetShell (ShellId nId)
{
    ActiveShellList::iterator aI (::std::find_if (
        mpActiveViewShells->begin(),
        mpActiveViewShells->end(),
        IsId(nId)));
    if (aI != mpActiveViewShells->end())
        return aI->mpViewShell;
    else
        return NULL;
}




ShellId ViewShellManager::GetShellId (const ViewShell* pShell)
{
    ActiveShellList::iterator aI (::std::find_if (
        mpActiveViewShells->begin(),
        mpActiveViewShells->end(),
        IsShell(pShell)));
    if (aI != mpActiveViewShells->end())
        return aI->mnId;
    else
        return snInvalidShellId;
}




void ViewShellManager::LockUpdate (void)
{
    if (mnUpdateLockCount == 0)
        TakeShellsFromStack();
    mnUpdateLockCount++;
}




void ViewShellManager::UnlockUpdate (void)
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




void ViewShellManager::GatherActiveShells (
    ::std::vector<SfxShell*>& rShellList)
{
    for (ActiveShellList::iterator aI (mpActiveViewShells->begin());
         aI!=mpActiveViewShells->end();
         aI++)
    {
        // Get all its sub shells that are placed below the shell.
        aI->mpViewShell->GetLowerShellList (rShellList);

        // Put the shell itself on the local stack.
        rShellList.push_back (aI->mpViewShell);

        // Get all its sub shells that are placed above the shell.
        aI->mpViewShell->GetUpperShellList (rShellList);
    }

    OSL_TRACE ("gathered shell list:");
    ::std::vector<SfxShell*>::iterator iShell;
    for (iShell=rShellList.begin(); iShell!=rShellList.end(); iShell++)
        OSL_TRACE ("    %p: %s",
            *iShell,
            ::rtl::OUStringToOString((*iShell)->GetName(),
                RTL_TEXTENCODING_UTF8).getStr());
}




void ViewShellManager::TakeShellsFromStack (void)
{
    // Now do the actual removing.
    if (mbIsValid)
    {
        ::std::vector<SfxShell*> aShellsToRemove;
        GatherActiveShells (aShellsToRemove);

        ::std::vector<SfxShell*>::iterator iShell;
        // Deactivate our shells on the stack before they are removed so
        // that during the Deactivation() calls the stack is still intact.
        for (iShell=aShellsToRemove.begin();
             iShell!=aShellsToRemove.end();
             iShell++)
        {
            (*iShell)->Deactivate(TRUE);
        }
        for (iShell=aShellsToRemove.begin();
             iShell!=aShellsToRemove.end();
             iShell++)
        {
            mrBase.RemoveSubShell (*iShell);
        }
        if (mrBase.GetDispatcher() != NULL)
            mrBase.GetDispatcher()->Flush();
    }
}




void ViewShellManager::PushShellsOnStack (void)
{
    // Remove all stacked shells.
    mrBase.RemoveSubShell (NULL);

    // Create a local stack of the shells that are to push on the shell
    // stack.  We can thus safly create the required shells wile still
    // having a valid shell stack.
    if (mbIsValid)
    {
        ::std::vector<SfxShell*> aShellsToPush;
        GatherActiveShells (aShellsToPush);

        // Now do the actual pushing.
        ::std::vector<SfxShell*>::reverse_iterator iShell;
        for (iShell=aShellsToPush.rbegin();
             iShell!=aShellsToPush.rend();
             iShell++)
        {
            mrBase.AddSubShell (**iShell);
        }
        if (mrBase.GetDispatcher() != NULL)
            mrBase.GetDispatcher()->Flush();
    }
}




ViewShellBase& ViewShellManager::GetViewShellBase (void) const
{
    return mrBase;
}




void ViewShellManager::ShutDown (void)
{
    mbIsValid = false;
}




IMPL_LINK(ViewShellManager, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        ::Window* pEventWindow
            = static_cast<VclWindowEvent*>(pEvent)->GetWindow();

        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_GETFOCUS:
            {
                for (ActiveShellList::iterator aI(mpActiveViewShells->begin());
                     aI!=mpActiveViewShells->end();
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




ViewShell* ViewShellManager::CreateViewShell (
    ShellId nShellId,
    ::Window* pParentWindow,
    FrameView* pFrameView)
{
    ViewShell* pViewShell = NULL;

    // Initialize with the default factory.
    ViewShellFactory* pFactory = mpDefaultFactory.get();

    // Look up a specialized factory.
    SpecializedFactoryList::iterator aI (
        mpSpecializedFactories->find(nShellId));
    if (aI != mpSpecializedFactories->end())
        pFactory = aI->second;

    UpdateLocker aLocker (*this);

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




//===== ViewShellManager::UpdateLocker ========================================

ViewShellManager::UpdateLocker::UpdateLocker (ViewShellManager& rManager)
    : mrManager(rManager)
{
    mrManager.LockUpdate();
}




ViewShellManager::UpdateLocker::~UpdateLocker (void)
{
    mrManager.UnlockUpdate();
}




} // end of namespace sd

