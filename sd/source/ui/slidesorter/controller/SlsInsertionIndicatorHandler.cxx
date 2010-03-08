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
#include "view/SlsLayouter.hxx"
#include "view/SlsInsertionIndicatorOverlay.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "SlideSorter.hxx"

using namespace ::com::sun::star::datatransfer::dnd::DNDConstants;

namespace sd { namespace slidesorter { namespace controller {


InsertionIndicatorHandler::InsertionIndicatorHandler (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mpInsertAnimator(),
      mpInsertionIndicatorOverlay(new view::InsertionIndicatorOverlay(rSlideSorter)),
      maInsertPosition(),
      meMode(MoveMode),
      mbIsActive(false),
      mbIsReadOnly(mrSlideSorter.GetModel().IsReadOnly()),
      mbIsOverSourceView(true),
      maIconSize(0,0)
{
}




InsertionIndicatorHandler::~InsertionIndicatorHandler (void)
{
}




void InsertionIndicatorHandler::Start (const bool bIsOverSourceView)
{
    if (mbIsActive)
    {
        OSL_ASSERT(!mbIsActive);
    }

    mbIsReadOnly = mrSlideSorter.GetModel().IsReadOnly();
    if (mbIsReadOnly)
        return;

    mbIsActive = true;
    mbIsOverSourceView = bIsOverSourceView;
}




void InsertionIndicatorHandler::End (void)
{
    if ( ! mbIsActive)
        return;

    if (mbIsReadOnly)
        return;

    GetInsertAnimator()->Reset();

    mbIsActive = false;
    mpInsertionIndicatorOverlay->Hide();
}




void InsertionIndicatorHandler::UpdateIndicatorIcon (const Transferable* pTransferable)
{
    mpInsertionIndicatorOverlay->Create(pTransferable);
    maIconSize = mpInsertionIndicatorOverlay->GetSize();
}




InsertionIndicatorHandler::Mode InsertionIndicatorHandler::GetModeFromDndAction (
    const sal_Int8 nDndAction)
{
    switch (nDndAction & (ACTION_COPY | ACTION_MOVE | ACTION_LINK))
    {
        case ACTION_COPY: return CopyMode;
        case ACTION_MOVE: return MoveMode;
        default: return UnknownMode;
    }
}




void InsertionIndicatorHandler::UpdatePosition (
    const Point& rMouseModelPosition,
    const Mode eMode)
{
    if ( ! mbIsActive)
        return;

    if (mbIsReadOnly)
        return;

    SetPosition(rMouseModelPosition, eMode);
}




void InsertionIndicatorHandler::UpdatePosition (
    const Point& rMouseModelPosition,
    const sal_Int8 nDndAction)
{
    UpdatePosition(rMouseModelPosition, GetModeFromDndAction(nDndAction));
}




bool InsertionIndicatorHandler::IsActive (void) const
{
    return mbIsActive;
}




sal_Int32 InsertionIndicatorHandler::GetInsertionPageIndex (void) const
{
    if (mbIsReadOnly)
        return -1;
    else
        return maInsertPosition.GetIndex();
}




void InsertionIndicatorHandler::SetPosition (
    const Point& rPoint,
    const Mode eMode)
{
    static const bool bAllowHorizontalInsertMarker = true;
    view::Layouter& rLayouter (mrSlideSorter.GetView().GetLayouter());
    USHORT nPageCount ((USHORT)mrSlideSorter.GetModel().GetPageCount());

    const view::InsertPosition aInsertPosition (rLayouter.GetInsertPosition(
        rPoint,
        maIconSize));

    if (maInsertPosition != aInsertPosition
        || meMode != eMode
        || ! mpInsertionIndicatorOverlay->IsVisible())
    {
        maInsertPosition = aInsertPosition;
        meMode = eMode;
        mbIsInsertionTrivial = IsInsertionTrivial(maInsertPosition.GetIndex(), eMode);

        if (maInsertPosition.GetIndex()>=0 && ! mbIsInsertionTrivial)
        {
            mpInsertionIndicatorOverlay->SetLocation(maInsertPosition.GetLocation());

            GetInsertAnimator()->SetInsertPosition(
                maInsertPosition,
                maIconSize);
            mpInsertionIndicatorOverlay->Show();
        }
        else
        {
            GetInsertAnimator()->Reset();
            mpInsertionIndicatorOverlay->Hide();
        }
    }
}




::boost::shared_ptr<view::InsertAnimator> InsertionIndicatorHandler::GetInsertAnimator (void)
{
    if ( ! mpInsertAnimator)
        mpInsertAnimator.reset(new view::InsertAnimator(mrSlideSorter));
    return mpInsertAnimator;
}




bool InsertionIndicatorHandler::IsInsertionTrivial (
    const sal_Int32 nInsertionIndex,
    const Mode eMode) const
{
    if (eMode == CopyMode)
        return false;
    else if (eMode == UnknownMode)
        return true;

    if ( ! mbIsOverSourceView)
        return false;

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
    if (nInsertionIndex<nFirstIndex || nInsertionIndex>(nLastIndex+1))
        return false;

    return true;
}



} } } // end of namespace ::sd::slidesorter::controller
