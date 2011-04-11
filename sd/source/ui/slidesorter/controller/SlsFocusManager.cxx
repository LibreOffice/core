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

#include "precompiled_sd.hxx"
#include "controller/SlsFocusManager.hxx"

#include "SlideSorter.hxx"
#include "PaneDockingWindow.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsVisibleAreaManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include <vcl/toolbox.hxx>

#include "Window.hxx"
#include "sdpage.hxx"

#define UNIFY_FOCUS_AND_CURRENT_PAGE

namespace sd { namespace slidesorter { namespace controller {

FocusManager::FocusManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mnPageIndex(0),
      mbPageIsFocused(false),
      mbIsVerticalWrapActive(false)
{
    if (mrSlideSorter.GetModel().GetPageCount() > 0)
        mnPageIndex = 0;
}




FocusManager::~FocusManager (void)
{
}




void FocusManager::MoveFocus (FocusMoveDirection eDirection)
{
    if (mnPageIndex >= 0 && mbPageIsFocused)
    {
        HideFocusIndicator (GetFocusedPageDescriptor());

        const sal_Int32 nColumnCount (mrSlideSorter.GetView().GetLayouter().GetColumnCount());
        const sal_Int32 nPageCount (mrSlideSorter.GetModel().GetPageCount());
        switch (eDirection)
        {
            case FMD_NONE:
                // Nothing to be done.
                break;

            case FMD_LEFT:
                if (mnPageIndex > 0)
                    mnPageIndex -= 1;
                else if (mbIsVerticalWrapActive)
                    mnPageIndex = nPageCount-1;
                break;

            case FMD_RIGHT:
                if (mnPageIndex < nPageCount-1)
                    mnPageIndex += 1;
                else if (mbIsVerticalWrapActive)
                    mnPageIndex = 0;
                break;

            case FMD_UP:
            {
                const sal_Int32 nCandidate (mnPageIndex - nColumnCount);
                if (nCandidate < 0)
                {
                    if (mbIsVerticalWrapActive)
                    {
                        // Wrap arround to the bottom row or the one above
                        // and go to the correct column.
                        const sal_Int32 nLastIndex (nPageCount-1);
                        const sal_Int32 nLastColumn (nLastIndex % nColumnCount);
                        const sal_Int32 nCurrentColumn (mnPageIndex%nColumnCount);
                        if (nLastColumn >= nCurrentColumn)
                        {
                            // The last row contains the current column.
                            mnPageIndex = nLastIndex - (nLastColumn-nCurrentColumn);
                        }
                        else
                        {
                            // Only the second to last row contains the current column.
                            mnPageIndex = nLastIndex - nLastColumn
                                - nColumnCount
                                + nCurrentColumn;
                        }
                    }
                }
                else
                {
                    // Move the focus the previous row.
                    mnPageIndex = nCandidate;
                }
            }
            break;

            case FMD_DOWN:
            {
                const sal_Int32 nCandidate (mnPageIndex + nColumnCount);
                if (nCandidate >= nPageCount)
                {
                    if (mbIsVerticalWrapActive)
                    {
                        // Wrap arround to the correct column.
                        mnPageIndex = mnPageIndex % nColumnCount;
                    }
                    else
                    {
                        // Do not move the focus.
                    }
                }
                else
                {
                    // Move the focus to the next row.
                    mnPageIndex = nCandidate;
                }
            }
            break;
        }

        if (mnPageIndex < 0)
        {
            OSL_ASSERT(mnPageIndex>=0);
            mnPageIndex = 0;
        }
        else if (mnPageIndex >= nPageCount)
        {
            OSL_ASSERT(mnPageIndex<nPageCount);
            mnPageIndex = nPageCount - 1;
        }

        if (mbPageIsFocused)
        {
            ShowFocusIndicator(GetFocusedPageDescriptor(), true);
        }
    }
}




void FocusManager::ShowFocus (const bool bScrollToFocus)
{
    mbPageIsFocused = true;
    ShowFocusIndicator(GetFocusedPageDescriptor(), bScrollToFocus);
}




void FocusManager::HideFocus (void)
{
    mbPageIsFocused = false;
    HideFocusIndicator(GetFocusedPageDescriptor());
}




bool FocusManager::ToggleFocus (void)
{
    if (mnPageIndex >= 0)
    {
        if (mbPageIsFocused)
            HideFocus ();
        else
            ShowFocus ();
    }
    return mbPageIsFocused;
}




bool FocusManager::HasFocus (void) const
{
    return mrSlideSorter.GetContentWindow()->HasFocus();
}




model::SharedPageDescriptor FocusManager::GetFocusedPageDescriptor (void) const
{
    return mrSlideSorter.GetModel().GetPageDescriptor(mnPageIndex);
}




sal_Int32 FocusManager::GetFocusedPageIndex (void) const
{
    return mnPageIndex;
}



/*
void FocusManager::FocusPage (sal_Int32 nPageIndex)
{
    if (nPageIndex != mnPageIndex)
    {
        // Hide the focus while switching it to the specified page.
        FocusHider aHider (*this);
        mnPageIndex = nPageIndex;
    }

    if (HasFocus() && !IsFocusShowing())
        ShowFocus();
}
*/



void FocusManager::SetFocusedPage (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        FocusHider aFocusHider (*this);
        mnPageIndex = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
    }
}




void FocusManager::SetFocusedPage (sal_Int32 nPageIndex)
{
    FocusHider aFocusHider (*this);
    mnPageIndex = nPageIndex;
}




void FocusManager::SetFocusedPageToCurrentPage (void)
{
    SetFocusedPage(mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
}




bool FocusManager::IsFocusShowing (void) const
{
    return HasFocus() && mbPageIsFocused;
}




void FocusManager::HideFocusIndicator (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        mrSlideSorter.GetView().SetState(rpDescriptor, model::PageDescriptor::ST_Focused, false);
    }
}




void FocusManager::ShowFocusIndicator (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bScrollToFocus)
{
    if (rpDescriptor.get() != NULL)
    {
        mrSlideSorter.GetView().SetState(rpDescriptor, model::PageDescriptor::ST_Focused, true);

        if (bScrollToFocus)
        {
            // Scroll the focused page object into the visible area and repaint
            // it, so that the focus indicator becomes visible.
            mrSlideSorter.GetController().GetVisibleAreaManager().RequestVisible(rpDescriptor,true);
        }
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);

        NotifyFocusChangeListeners();
    }
}




void FocusManager::AddFocusChangeListener (const Link& rListener)
{
    if (::std::find (maFocusChangeListeners.begin(), maFocusChangeListeners.end(), rListener)
        == maFocusChangeListeners.end())
    {
        maFocusChangeListeners.push_back (rListener);
    }
}




void FocusManager::RemoveFocusChangeListener (const Link& rListener)
{
    maFocusChangeListeners.erase (
        ::std::find (maFocusChangeListeners.begin(), maFocusChangeListeners.end(), rListener));
}




void FocusManager::SetFocusToToolBox (void)
{
    HideFocus();

    if (mrSlideSorter.GetViewShell() != NULL)
    {
        ::Window* pParentWindow = mrSlideSorter.GetViewShell()->GetParentWindow();
        DockingWindow* pDockingWindow = NULL;
        while (pParentWindow!=NULL && pDockingWindow==NULL)
        {
            pDockingWindow = dynamic_cast<DockingWindow*>(pParentWindow);
            pParentWindow = pParentWindow->GetParent();
        }
        if (pDockingWindow)
        {
            PaneDockingWindow* pPaneDockingWindow = dynamic_cast<PaneDockingWindow*>(pDockingWindow);
            if (pPaneDockingWindow != NULL)
                pPaneDockingWindow->GetToolBox().GrabFocus();
        }
    }
}




void FocusManager::NotifyFocusChangeListeners (void) const
{
    // Create a copy of the listener list to be safe when that is modified.
    ::std::vector<Link> aListeners (maFocusChangeListeners);

    // Tell the slection change listeners that the selection has changed.
    ::std::vector<Link>::iterator iListener (aListeners.begin());
    ::std::vector<Link>::iterator iEnd (aListeners.end());
    for (; iListener!=iEnd; ++iListener)
    {
        iListener->Call(NULL);
    }
}




FocusManager::FocusHider::FocusHider (FocusManager& rManager)
: mbFocusVisible(rManager.IsFocusShowing())
, mrManager(rManager)
{
    mrManager.HideFocus();
}




FocusManager::FocusHider::~FocusHider (void)
{
    if (mbFocusVisible)
        mrManager.ShowFocus();
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
