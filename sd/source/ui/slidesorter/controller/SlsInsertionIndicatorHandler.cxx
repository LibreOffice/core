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

#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsProperties.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsLayouter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"

#include "SlideSorter.hxx"


namespace sd { namespace slidesorter { namespace controller {


InsertionIndicatorHandler::InsertionIndicatorHandler (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mpInsertAnimator(),
      mpInsertionIndicatorOverlay(
          mrSlideSorter.GetView().GetOverlay().GetInsertionIndicatorOverlay()),
      mnInsertionIndex(-1),
      mbIsBeforePage(false),
      mbIsActive(false)
{
}




InsertionIndicatorHandler::~InsertionIndicatorHandler (void)
{
}




void InsertionIndicatorHandler::Start (const Point& rMouseModelPosition)
{
    if (mbIsActive)
    {
        OSL_ASSERT(!mbIsActive);
    }

    if (mrSlideSorter.GetProperties()->IsUIReadOnly())
        return;

    SetPosition(rMouseModelPosition);

    mbIsActive = true;
}




void InsertionIndicatorHandler::UpdatePosition (const Point& rMouseModelPosition)
{
    OSL_ASSERT(mbIsActive);

    if (mrSlideSorter.GetProperties()->IsUIReadOnly())
        return;

    SetPosition(rMouseModelPosition);
}




void InsertionIndicatorHandler::End (void)
{
    OSL_ASSERT(mbIsActive);
    GetInsertAnimator()->SetInsertPosition(-1, false);

    mbIsActive = false;
    mpInsertionIndicatorOverlay->SetIsVisible(false);
    GetInsertAnimator()->Reset();
}




bool InsertionIndicatorHandler::IsActive (void) const
{
    return mbIsActive;
}




sal_Int32 InsertionIndicatorHandler::GetInsertionPageIndex (void) const
{
    return mnInsertionIndex;
}




void InsertionIndicatorHandler::SetPosition (const Point& rPoint)
{
    static const bool bAllowHorizontalInsertMarker = true;
    view::Layouter& rLayouter (mrSlideSorter.GetView().GetLayouter());
    USHORT nPageCount ((USHORT)mrSlideSorter.GetModel().GetPageCount());

    sal_Int32 nInsertionIndex = rLayouter.GetInsertionIndex (rPoint,
        bAllowHorizontalInsertMarker);
    if (nInsertionIndex >= nPageCount)
        nInsertionIndex = nPageCount-1;
    sal_Int32 nDrawIndex = nInsertionIndex;

    bool bVertical = false;
    bool bIsBeforePage = false;
    if (nInsertionIndex >= 0)
    {
        // Now that we know where to insert, we still have to determine
        // where to draw the marker.  There are two decisions to make:
        // 1. Draw a vertical or a horizontal insert marker.
        //    The horizontal one may only be chosen when there is only one
        //    column.
        // 2. The vertical (standard) insert marker may be painted left to
        //    the insert page or right of the previous one.  When both pages
        //    are in the same row this makes no difference.  Otherwise the
        //    posiotions are at the left and right ends of two rows.

        Point aPageCenter (rLayouter.GetPageObjectBox (
            nInsertionIndex).Center());

        if (bAllowHorizontalInsertMarker
            && rLayouter.GetColumnCount() == 1)
        {
            bVertical = false;
            bIsBeforePage = (rPoint.Y() <= aPageCenter.Y());
        }
        else
        {
            bVertical = true;
            bIsBeforePage = (rPoint.X() <= aPageCenter.X());
        }

        // Add one when the mark was painted below or to the right of the
        // page object.
        if ( ! bIsBeforePage)
            nInsertionIndex += 1;
    }

    if (mnInsertionIndex!=nInsertionIndex || mbIsBeforePage!=bIsBeforePage)
    {
        mnInsertionIndex = nInsertionIndex;
        mbIsBeforePage = bIsBeforePage;
        mbIsInsertionTrivial = IsInsertionTrivial();

        mpInsertionIndicatorOverlay->SetLocation(
            rLayouter.GetInsertionMarkerLocation (
                nDrawIndex,
                bVertical,
                mbIsBeforePage));
    }

    if (mnInsertionIndex>=0 && ! mbIsInsertionTrivial)
    {
        GetInsertAnimator()->SetInsertPosition(
            mnInsertionIndex,
            mbIsBeforePage);
        mpInsertionIndicatorOverlay->SetIsVisible(true);
    }
    else
    {
        GetInsertAnimator()->Reset();
        mpInsertionIndicatorOverlay->SetIsVisible(false);
    }
}




::boost::shared_ptr<view::InsertAnimator> InsertionIndicatorHandler::GetInsertAnimator (void)
{
    if ( ! mpInsertAnimator)
        mpInsertAnimator.reset(new view::InsertAnimator(mrSlideSorter));
    return mpInsertAnimator;
}




bool InsertionIndicatorHandler::IsInsertionTrivial (void) const
{
    // Iterate over all selected pages and check whether there are
    // holes.  While we do this we remember the indices of the first and
    // last selected page as preparation for the next step.
    sal_Int32 nCurrentIndex = -1;
    sal_Int32 nFirstIndex = -1;
    sal_Int32 nLastIndex = -1;
    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());

        // Get the page number and compare it to the last one.
        const sal_Int32 nPageNumber (pDescriptor->GetPageIndex());
        if (nCurrentIndex>=0 && nPageNumber>(nCurrentIndex+1))
            return false;
        else
            nCurrentIndex = nPageNumber;

        // Remember indices of the first and last page of the selection.
        if (nFirstIndex == -1)
            nFirstIndex = nPageNumber;
        nLastIndex = nPageNumber;
    }

    // When we come here then the selection has no holes.  We still have
    // to check that the insertion position is not directly in front or
    // directly behind the selection and thus moving the selection there
    // would not change the model.
    if (mnInsertionIndex<nFirstIndex || mnInsertionIndex>(nLastIndex+1))
        return false;

    return true;
}



} } } // end of namespace ::sd::slidesorter::controller
