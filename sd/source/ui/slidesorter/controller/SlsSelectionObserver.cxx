/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

        maDeletedPages.push_back(pPage->GetPageNumber());
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
