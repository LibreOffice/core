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

#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsSelectionManager.hxx>
#include <controller/SlsSelectionObserver.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsFocusManager.hxx>
#include <sdpage.hxx>
#include <osl/diagnose.h>

namespace sd::slidesorter::controller
{
SelectionObserver::Context::Context(SlideSorter const& rSlideSorter)
    : mpSelectionObserver(
          rSlideSorter.GetController().GetSelectionManager()->GetSelectionObserver())
{
    if (mpSelectionObserver)
        mpSelectionObserver->StartObservation();
}

SelectionObserver::Context::~Context() COVERITY_NOEXCEPT_FALSE
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

SelectionObserver::SelectionObserver(SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter)
    , mbIsObservationActive(false)
    , mbPageEventOccurred(false)
    , maInsertedPages()
{
}

SelectionObserver::~SelectionObserver() {}

void SelectionObserver::NotifyPageEvent(const SdrPage* pSdrPage)
{
    if (!mbIsObservationActive)
        return;

    mbPageEventOccurred = true;

    const SdPage* pPage = dynamic_cast<const SdPage*>(pSdrPage);
    if (pPage == nullptr)
        return;

    //NotifyPageEvent is called for add, remove, *and* change position so for
    //the change position case we must ensure we don't end up with the slide
    //duplicated in our list
    std::vector<const SdPage*>::iterator iPage(
        std::find(maInsertedPages.begin(), maInsertedPages.end(), pPage));
    if (iPage != maInsertedPages.end())
        maInsertedPages.erase(iPage);

    if (pPage->IsInserted())
        maInsertedPages.push_back(pPage);
}

void SelectionObserver::StartObservation()
{
    OSL_ASSERT(!mbIsObservationActive);
    maInsertedPages.clear();
    mbIsObservationActive = true;
}

void SelectionObserver::AbortObservation()
{
    OSL_ASSERT(mbIsObservationActive);
    mbIsObservationActive = false;
    maInsertedPages.clear();
}

void SelectionObserver::EndObservation()
{
    OSL_ASSERT(mbIsObservationActive);
    mbIsObservationActive = false;

    if (!mbPageEventOccurred)
        return;

    PageSelector& rSelector(mrSlideSorter.GetController().GetPageSelector());
    PageSelector::UpdateLock aUpdateLock(mrSlideSorter);
    rSelector.DeselectAllPages();
    if (!maInsertedPages.empty())
    {
        // Select the inserted pages.
        for (const auto& rpPage : maInsertedPages)
        {
            rSelector.SelectPage(rpPage);
        }
        maInsertedPages.clear();
    }

    aUpdateLock.Release();
    bool bSuccess = mrSlideSorter.GetController().GetFocusManager().SetFocusedPageToCurrentPage();
    // tdf#129346 nothing currently selected, select something, if possible
    // but (tdf#129346) only if setting focus to current page failed
    if (!bSuccess && rSelector.GetPageCount() && rSelector.GetSelectedPageCount() == 0)
        rSelector.SelectPage(0);
}

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
