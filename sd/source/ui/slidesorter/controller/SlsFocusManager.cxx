/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsFocusManager.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:24:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"
#include "controller/SlsFocusManager.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"

#include "Window.hxx"
#include "sdpage.hxx"

namespace sd { namespace slidesorter { namespace controller {

FocusManager::FocusManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mnPageIndex (-1),
      mbPageIsFocused (false)
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

        int nColumnCount (mrSlideSorter.GetView().GetLayouter().GetColumnCount());
        switch (eDirection)
        {
            case FMD_NONE:
                if (mnPageIndex >= mrSlideSorter.GetModel().GetPageCount())
                    mnPageIndex = mrSlideSorter.GetModel().GetPageCount() - 1;
                break;

            case FMD_LEFT:
                mnPageIndex -= 1;
                if (mnPageIndex < 0)
                    mnPageIndex = mrSlideSorter.GetModel().GetPageCount() - 1;
                break;

            case FMD_RIGHT:
                mnPageIndex += 1;
                if (mnPageIndex >= mrSlideSorter.GetModel().GetPageCount())
                    mnPageIndex = 0;
                break;

            case FMD_UP:
            {
                int nColumn = mnPageIndex % nColumnCount;
                mnPageIndex -= nColumnCount;
                if (mnPageIndex < 0)
                {
                    // Wrap arround to the bottom row or the one above and
                    // go to the correct column.
                    int nCandidate = mrSlideSorter.GetModel().GetPageCount()-1;
                    int nCandidateColumn = nCandidate % nColumnCount;
                    if (nCandidateColumn > nColumn)
                        mnPageIndex = nCandidate - (nCandidateColumn-nColumn);
                    else if (nCandidateColumn < nColumn)
                        mnPageIndex = nCandidate
                            - nColumnCount
                            + (nColumn - nCandidateColumn);
                    else
                        mnPageIndex = nCandidate;
                }
            }
            break;

            case FMD_DOWN:
            {
                int nColumn = mnPageIndex % nColumnCount;
                mnPageIndex += nColumnCount;
                if (mnPageIndex >= mrSlideSorter.GetModel().GetPageCount())
                {
                    // Wrap arround to the correct column.
                    mnPageIndex = nColumn;
                }
            }
            break;
        }

        ShowFocusIndicator (GetFocusedPageDescriptor());
    }
}




void FocusManager::ShowFocus (void)
{
    mbPageIsFocused = true;
    ShowFocusIndicator (GetFocusedPageDescriptor());
}




void FocusManager::HideFocus (void)
{
    mbPageIsFocused = false;
    HideFocusIndicator (GetFocusedPageDescriptor());
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
    return mrSlideSorter.GetView().GetWindow()->HasFocus();
}




model::SharedPageDescriptor FocusManager::GetFocusedPageDescriptor (void) const
{
    return mrSlideSorter.GetModel().GetPageDescriptor(mnPageIndex);
}




sal_Int32 FocusManager::GetFocusedPageIndex (void) const
{
    return mnPageIndex;
}




void FocusManager::FocusPage (sal_Int32 nPageIndex)
{
    if (nPageIndex != mnPageIndex)
    {
        // Hide the focus while switching it to the specified page.
        FocusHider aHider (*this);
        mnPageIndex = nPageIndex;
    }
}




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




bool FocusManager::IsFocusShowing (void) const
{
    return HasFocus() && mbPageIsFocused;
}




void FocusManager::HideFocusIndicator (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        rpDescriptor->RemoveFocus();
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);
    }
}




void FocusManager::ShowFocusIndicator (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        rpDescriptor->SetFocus ();

        // Scroll the focused page object into the visible area and repaint
        // it, so that the focus indicator becomes visible.
        view::SlideSorterView& rView (mrSlideSorter.GetView());
        mrSlideSorter.GetController().GetSelectionManager()->MakeRectangleVisible (
            rView.GetPageBoundingBox (
                GetFocusedPageDescriptor(),
                view::SlideSorterView::CS_MODEL,
                view::SlideSorterView::BBT_INFO));

        mrSlideSorter.GetView().RequestRepaint (rpDescriptor);
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

