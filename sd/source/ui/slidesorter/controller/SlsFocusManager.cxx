/*************************************************************************
 *
 *  $RCSfile: SlsFocusManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:13:32 $
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

#include "controller/SlsFocusManager.hxx"

#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "Window.hxx"


namespace sd { namespace slidesorter { namespace controller {

FocusManager::FocusManager (SlideSorterController& rController)
    : mrController (rController),
      mnPageIndex (-1),
      mbPageIsFocused (false)
{
    if (mrController.GetModel().GetPageCount() > 0)
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

        int nColumnCount
            = mrController.GetView().GetLayouter().GetColumnCount();
        switch (eDirection)
        {
            case FMD_NONE:
                if (mnPageIndex >= mrController.GetModel().GetPageCount())
                    mnPageIndex = mrController.GetModel().GetPageCount() - 1;
                break;

            case FMD_LEFT:
                mnPageIndex -= 1;
                if (mnPageIndex < 0)
                    mnPageIndex = mrController.GetModel().GetPageCount() - 1;
                break;

            case FMD_RIGHT:
                mnPageIndex += 1;
                if (mnPageIndex >= mrController.GetModel().GetPageCount())
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
                    int nCandidate = mrController.GetModel().GetPageCount()-1;
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
                if (mnPageIndex >= mrController.GetModel().GetPageCount())
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
    return mrController.GetView().GetWindow()->HasFocus();
}




model::PageDescriptor* FocusManager::GetFocusedPageDescriptor (void) const
{
    return mrController.GetModel().GetPageDescriptor (mnPageIndex);
}




sal_Int32 FocusManager::GetFocusedPageIndex (void) const
{
    return mnPageIndex;
}




bool FocusManager::IsFocusShowing (void) const
{
    return HasFocus() && mbPageIsFocused;
}




void FocusManager::HideFocusIndicator (model::PageDescriptor* pDescriptor)
{
    pDescriptor->RemoveFocus();
    mrController.GetView().RequestRepaint (*pDescriptor);
}




void FocusManager::ShowFocusIndicator (model::PageDescriptor* pDescriptor)
{
    if (pDescriptor != NULL)
    {
        pDescriptor->SetFocus ();

        // Scroll the focused page object into the visible area and repaint
        // it, so that the focus indicator becomes visible.
        view::SlideSorterView& rView (mrController.GetView());
        mrController.MakeRectangleVisible (
            rView.GetPageBoundingBox (
                *GetFocusedPageDescriptor(),
                view::SlideSorterView::CS_MODEL,
                view::SlideSorterView::BBT_INFO));

        mrController.GetView().RequestRepaint (*pDescriptor);
    }
}




FocusManager::FocusHider::FocusHider (FocusManager& rManager)
    : mrManager(rManager),
      mbFocusVisible(rManager.IsFocusShowing())
{
    mrManager.HideFocus();
}




FocusManager::FocusHider::~FocusHider (void)
{
    if (mbFocusVisible)
        mrManager.ShowFocus();
}

} } } // end of namespace ::sd::slidesorter::controller

