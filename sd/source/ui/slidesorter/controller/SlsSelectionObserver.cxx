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

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsFocusManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include <svx/svdmodel.hxx>
#include "drawdoc.hxx"


namespace sd { namespace slidesorter { namespace controller {

SelectionObserver::Context::Context (SlideSorter& rSlideSorter)
    : mpSelectionObserver(
        rSlideSorter.GetController().GetSelectionManager()->GetSelectionObserver())
{
    if (mpSelectionObserver)
        mpSelectionObserver->StartObservation();
}




SelectionObserver::Context::~Context(void)
{
    if (mpSelectionObserver)
        mpSelectionObserver->EndObservation();
}




void SelectionObserver::Context::Abort(void)
{
    if (mpSelectionObserver)
    {
        mpSelectionObserver->AbortObservation();
        mpSelectionObserver.reset();
    }
}




//===== SelectionObserver =====================================================

SelectionObserver::SelectionObserver (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mpDocument(mrSlideSorter.GetModel().GetDocument()),
      mbIsOvservationActive(false),
      maInsertedPages(),
      maDeletedPages()
{
}




SelectionObserver::~SelectionObserver (void)
{
}




void SelectionObserver::NotifyPageEvent (const SdrPage* pSdrPage)
{
    if ( ! mbIsOvservationActive)
        return;

    const SdPage* pPage = dynamic_cast<const SdPage*>(pSdrPage);
    if (pPage == NULL)
        return;

    if (pPage->IsInserted())
        maInsertedPages.push_back(pPage);
    else
    {
        ::std::vector<const SdPage*>::iterator iPage(
            ::std::find(maInsertedPages.begin(), maInsertedPages.end(), pPage));
        if (iPage != maInsertedPages.end())
            maInsertedPages.erase(iPage);

        maDeletedPages.push_back(pPage->GetPageNum());
    }
}



void SelectionObserver::StartObservation (void)
{
    OSL_ASSERT(!mbIsOvservationActive);
    maInsertedPages.clear();
    maDeletedPages.clear();
    mbIsOvservationActive = true;
}




void SelectionObserver::AbortObservation (void)
{
    OSL_ASSERT(mbIsOvservationActive);
    mbIsOvservationActive = false;
    maInsertedPages.clear();
    maDeletedPages.clear();
}




void SelectionObserver::EndObservation (void)
{
    OSL_ASSERT(mbIsOvservationActive);
    mbIsOvservationActive = false;

    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());
    PageSelector::UpdateLock aUpdateLock (mrSlideSorter);
    rSelector.DeselectAllPages();
    if ( ! maInsertedPages.empty())
    {
        // Select the inserted pages.
        for (::std::vector<const SdPage*>::const_iterator
                 iPage(maInsertedPages.begin()),
                 iEnd(maInsertedPages.end());
             iPage!=iEnd;
             ++iPage)
        {
            rSelector.SelectPage(*iPage);
        }
        maInsertedPages.clear();
    }
    maDeletedPages.clear();

    aUpdateLock.Release();
    mrSlideSorter.GetController().GetFocusManager().SetFocusedPageToCurrentPage();

}



} } } // end of namespace ::sd::slidesorter::controller
