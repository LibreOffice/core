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

SelectionObserver::Context::~Context()
{
    if (mpSelectionObserver)
        mpSelectionObserver->EndObservation();
}

void SelectionObserver::Context::Abort()
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
      mbIsOvservationActive(false),
      maInsertedPages(),
      maDeletedPages()
{
}

SelectionObserver::~SelectionObserver()
{
}

void SelectionObserver::NotifyPageEvent (const SdrPage* pSdrPage)
{
    if ( ! mbIsOvservationActive)
        return;

    const SdPage* pPage = dynamic_cast<const SdPage*>(pSdrPage);
    if (pPage == nullptr)
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

void SelectionObserver::StartObservation()
{
    OSL_ASSERT(!mbIsOvservationActive);
    maInsertedPages.clear();
    maDeletedPages.clear();
    mbIsOvservationActive = true;
}

void SelectionObserver::AbortObservation()
{
    OSL_ASSERT(mbIsOvservationActive);
    mbIsOvservationActive = false;
    maInsertedPages.clear();
    maDeletedPages.clear();
}

void SelectionObserver::EndObservation()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
