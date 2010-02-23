/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsSelectionFunction.cxx,v $
 * $Revision: 1.37 $
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

#include "SlsSubstitutionHandler.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsViewOverlay.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "app.hrc"
#include <sfx2/bindings.hxx>
#include <boost/bind.hpp>

namespace sd { namespace slidesorter { namespace controller {

SubstitutionHandler::SubstitutionHandler (
    SlideSorter& rSlideSorter,
    const model::SharedPageDescriptor& rpHitDescriptor,
    const Point& rMouseModelPosition)
    : mpTargetSlideSorter(&rSlideSorter),
      mpHitDescriptor(rpHitDescriptor),
      mnInsertionIndex(-1)
{
    // No Drag-and-Drop for master pages.
    if (rSlideSorter.GetModel().GetEditMode() != EM_PAGE)
        return;

    view::ViewOverlay& rOverlay (rSlideSorter.GetView().GetOverlay());

    if ( ! rOverlay.GetSubstitutionOverlay()->IsVisible())
    {
        // Show a new substitution for the selected page objects.
        model::PageEnumeration aSelectedPages(
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                rSlideSorter.GetModel()));
        rOverlay.GetSubstitutionOverlay()->Create(
            aSelectedPages,
            rMouseModelPosition,
            mpHitDescriptor);
        rOverlay.GetSubstitutionOverlay()->SetIsVisible(true);
        rSlideSorter.GetController().GetInsertionIndicatorHandler()->Start(
            rMouseModelPosition,
            InsertionIndicatorHandler::MoveMode,
            true);
    }
}




SubstitutionHandler::~SubstitutionHandler (void)
{
    if (mpTargetSlideSorter != NULL)
        mpTargetSlideSorter->GetController().GetScrollBarManager().StopAutoScroll();

    Process();

    if (mpTargetSlideSorter != NULL)
    {
        view::ViewOverlay& rOverlay (mpTargetSlideSorter->GetView().GetOverlay());
        rOverlay.GetSubstitutionOverlay()->SetIsVisible(false);
        rOverlay.GetSubstitutionOverlay()->Clear();
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->End();
    }
    mpHitDescriptor.reset();
}




void SubstitutionHandler::Dispose (void)
{
    mnInsertionIndex = -1;
}




void SubstitutionHandler::UpdatePosition (
    const Point& rMousePosition,
    const InsertionIndicatorHandler::Mode eMode,
    const bool bAllowAutoScroll)
{
    if (mpTargetSlideSorter == NULL)
        return;

    if (mpTargetSlideSorter->GetProperties()->IsUIReadOnly())
        return;

    // Convert window coordinates into model coordinates (we need the
    // window coordinates for auto-scrolling because that remains
    // constant while scrolling.)
    SharedSdWindow pWindow (mpTargetSlideSorter->GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));

    if ( ! (bAllowAutoScroll
            && mpTargetSlideSorter->GetController().GetScrollBarManager().AutoScroll(
        rMousePosition,
        ::boost::bind(
            &SubstitutionHandler::UpdatePosition,
            this,
            rMousePosition,
            eMode,
            false))))
    {
        view::ViewOverlay& rOverlay (mpTargetSlideSorter->GetView().GetOverlay());

        // Move the existing substitution to the new position.
        rOverlay.GetSubstitutionOverlay()->SetPosition(aMouseModelPosition);

        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->UpdatePosition(
            aMouseModelPosition,
            eMode);

        // Remember the new insertion index.
        if (mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->IsInsertionTrivial(eMode))
            mnInsertionIndex = -1;
        else
            mnInsertionIndex = mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()
                ->GetInsertionPageIndex();
    }
}




void SubstitutionHandler::Process (void)
{
    if (mpTargetSlideSorter == NULL)
        return;

    if (mpTargetSlideSorter->GetProperties()->IsUIReadOnly())
        return;

    if (mnInsertionIndex >= 0)
    {
        // Tell the model to move the selected pages behind the one with the
        // index mnInsertionIndex which first has to transformed into an index
        // understandable by the document.
        USHORT nDocumentIndex = (USHORT)mnInsertionIndex-1;
        mpTargetSlideSorter->GetController().GetSelectionManager()->MoveSelectedPages(nDocumentIndex);

        ViewShell* pViewShell = mpTargetSlideSorter->GetViewShell();
        if (pViewShell != NULL)
            pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_STATUS_PAGE);
    }
}




void SubstitutionHandler::Show (void)
{
    if (mpTargetSlideSorter != NULL)
    {
        view::ViewOverlay& rOverlay (mpTargetSlideSorter->GetView().GetOverlay());
        rOverlay.GetSubstitutionOverlay()->SetIsVisible(true);
    }
}




void SubstitutionHandler::Hide (void)
{
    if (mpTargetSlideSorter != NULL)
    {
        view::ViewOverlay& rOverlay (mpTargetSlideSorter->GetView().GetOverlay());
        rOverlay.GetSubstitutionOverlay()->SetIsVisible(false);
    }
}




void SubstitutionHandler::SetTargetSlideSorter (
    SlideSorter* pSlideSorter,
    const Point aMousePosition,
    const InsertionIndicatorHandler::Mode eMode,
    const bool bIsOverSourceView)
{
    if (mpTargetSlideSorter != NULL)
    {
        mpOverlayState = mpTargetSlideSorter->GetView().GetOverlay().GetSubstitutionOverlay()
            ->GetInternalState();
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->End();
    }

    mpTargetSlideSorter = pSlideSorter;

    if (mpTargetSlideSorter != NULL)
    {
        mpTargetSlideSorter->GetView().GetOverlay().GetSubstitutionOverlay()->SetInternalState(
            mpOverlayState);
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->Start(
            aMousePosition,
            eMode,
            bIsOverSourceView);
    }
}



} } } // end of namespace ::sd::slidesorter::controller
