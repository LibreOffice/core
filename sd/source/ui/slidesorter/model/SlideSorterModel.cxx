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

#include <model/SlideSorterModel.hxx>

#include <SlideSorter.hxx>
#include <sal/log.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <controller/SlsSlotManager.hxx>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

#include <ViewShellBase.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <FrameView.hxx>

#include <o3tl/safeint.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd::slidesorter::model {

namespace {
    bool PrintModel (const SlideSorterModel& rModel)
    {
        for (sal_Int32 nIndex=0,nCount=rModel.GetPageCount(); nIndex<nCount; ++nIndex)
        {
            SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));
            if (pDescriptor)
            {
                SAL_INFO(
                    "sd.sls",
                    nIndex << " " << pDescriptor->GetPageIndex() << " "
                        << pDescriptor->GetVisualState().mnPageId << " "
                        << FromCoreIndex(pDescriptor->GetPage()->GetPageNum())
                        << " " << pDescriptor->GetPage());
            }
            else
            {
                SAL_INFO("sd.sls", nIndex);
            }
        }

        return true;
    }
    bool CheckModel (const SlideSorterModel& rModel)
    {
        for (sal_Int32 nIndex=0,nCount=rModel.GetPageCount(); nIndex<nCount; ++nIndex)
        {
            SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));
            if ( ! pDescriptor)
            {
                PrintModel(rModel);
                assert(pDescriptor);
                return false;
            }
            if (nIndex != pDescriptor->GetPageIndex())
            {
                PrintModel(rModel);
                assert(nIndex == pDescriptor->GetPageIndex());
                return false;
            }
            if (nIndex != pDescriptor->GetVisualState().mnPageId)
            {
                PrintModel(rModel);
                assert(nIndex == pDescriptor->GetVisualState().mnPageId);
                return false;
            }
        }

        return true;
    }
}

namespace {

void collectUIInformation(const OUString& num, const OUString& rAction)
{
    EventDescription aDescription;
    aDescription.aID = "impress_win_or_draw_win";
    aDescription.aParameters = {{"POS", num}};
    aDescription.aAction = rAction;
    aDescription.aKeyWord = "ImpressWindowUIObject";
    aDescription.aParent = "MainWindow";

    UITestLogger::getInstance().logEvent(aDescription);
}

}

SlideSorterModel::SlideSorterModel (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      meEditMode(EditMode::Page),
      maPageDescriptors(0)
{
}

SlideSorterModel::~SlideSorterModel()
{
    ClearDescriptorList ();
}

void SlideSorterModel::Dispose()
{
    ClearDescriptorList ();
}

SdDrawDocument* SlideSorterModel::GetDocument()
{
    if (mrSlideSorter.GetViewShellBase() != nullptr)
        return mrSlideSorter.GetViewShellBase()->GetDocument();
    else
         return nullptr;
}

bool SlideSorterModel::SetEditMode (EditMode eEditMode)
{
    bool bEditModeChanged = false;
    if (meEditMode != eEditMode)
    {
        meEditMode = eEditMode;
        UpdatePageList();
        bEditModeChanged = true;
    }
    return bEditModeChanged;
}

sal_Int32 SlideSorterModel::GetPageCount() const
{
    return maPageDescriptors.size();
}

SharedPageDescriptor SlideSorterModel::GetPageDescriptor (
    const sal_Int32 nPageIndex,
    const bool bCreate) const
{
    ::osl::MutexGuard aGuard (maMutex);

    SharedPageDescriptor pDescriptor;

    if (nPageIndex>=0 && nPageIndex<GetPageCount())
    {
        pDescriptor = maPageDescriptors[nPageIndex];
        if (pDescriptor == nullptr && bCreate && mxSlides.is())
        {
            SdPage* pPage = GetPage(nPageIndex);
            pDescriptor = std::make_shared<PageDescriptor>(
                Reference<drawing::XDrawPage>(mxSlides->getByIndex(nPageIndex),UNO_QUERY),
                pPage,
                nPageIndex);
            maPageDescriptors[nPageIndex] = pDescriptor;
        }
    }

    return pDescriptor;
}

sal_Int32 SlideSorterModel::GetIndex (const Reference<drawing::XDrawPage>& rxSlide) const
{
    ::osl::MutexGuard aGuard (maMutex);

    // First try to guess the right index.
    Reference<beans::XPropertySet> xSet (rxSlide, UNO_QUERY);
    if (xSet.is())
    {
        try
        {
            const Any aNumber (xSet->getPropertyValue(u"Number"_ustr));
            sal_Int16 nNumber (-1);
            aNumber >>= nNumber;
            nNumber -= 1;
            SharedPageDescriptor pDescriptor (GetPageDescriptor(nNumber, false));
            if (pDescriptor
                && pDescriptor->GetXDrawPage() == rxSlide)
            {
                return nNumber;
            }
        }
        catch (uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("sd");
        }
    }

    // Guess was wrong, iterate over all slides and search for the right
    // one.
    const sal_Int32 nCount (maPageDescriptors.size());
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        SharedPageDescriptor pDescriptor (maPageDescriptors[nIndex]);

        // Make sure that the descriptor exists.  Without it the given slide
        // can not be found.
        if (!pDescriptor)
        {
            // Call GetPageDescriptor() to create the missing descriptor.
            pDescriptor = GetPageDescriptor(nIndex);
        }

        if (pDescriptor->GetXDrawPage() == rxSlide)
            return nIndex;
    }

    return  -1;
}

sal_Int32 SlideSorterModel::GetIndex (const SdrPage* pPage) const
{
    if (pPage == nullptr)
        return -1;

    ::osl::MutexGuard aGuard (maMutex);

    // First try to guess the right index.
    sal_Int16 nNumber ((pPage->GetPageNum()-1)/2);
    SharedPageDescriptor pDescriptor (GetPageDescriptor(nNumber, false));
    if (pDescriptor
        && pDescriptor->GetPage() == pPage)
    {
        return nNumber;
    }

    // Guess was wrong, iterate over all slides and search for the right
    // one.
    const sal_Int32 nCount (maPageDescriptors.size());
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        pDescriptor = maPageDescriptors[nIndex];

        // Make sure that the descriptor exists.  Without it the given slide
        // can not be found.
        if (!pDescriptor)
        {
            // Call GetPageDescriptor() to create the missing descriptor.
            pDescriptor = GetPageDescriptor(nIndex);
        }

        if (pDescriptor->GetPage() == pPage)
            return nIndex;
    }

    return  -1;
}

sal_uInt16 SlideSorterModel::GetCoreIndex (const sal_Int32 nIndex) const
{
    SharedPageDescriptor pDescriptor (GetPageDescriptor(nIndex));
    if (pDescriptor)
        return pDescriptor->GetPage()->GetPageNum();
    else
        return mxSlides->getCount()*2+1;
}

/** For now this method uses a trivial algorithm: throw away all descriptors
    and create them anew (on demand).  The main problem that we are facing
    when designing a better algorithm is that we can not compare pointers to
    pages stored in the PageDescriptor objects and those obtained from the
    document: pages may have been deleted and others may have been created
    at the exact same memory locations.
*/
void SlideSorterModel::Resync()
{
    ::osl::MutexGuard aGuard (maMutex);

    // Check if document and this model really differ.
    bool bIsUpToDate (true);
    SdDrawDocument* pDocument = GetDocument();
    if (pDocument!=nullptr && maPageDescriptors.size()==pDocument->GetSdPageCount(PageKind::Standard))
    {
        for (sal_Int32 nIndex=0,nCount=maPageDescriptors.size(); nIndex<nCount; ++nIndex)
        {
            if (maPageDescriptors[nIndex]
                && maPageDescriptors[nIndex]->GetPage()
                != GetPage(nIndex))
            {
                SAL_INFO("sd.sls", "page " << nIndex << " differs");
                bIsUpToDate = false;
                break;
            }
        }
    }
    else
    {
        bIsUpToDate = false;
    }

    if ( ! bIsUpToDate)
    {
        SynchronizeDocumentSelection(); // Try to make the current selection persistent.
        ClearDescriptorList ();
        AdaptSize();
        SynchronizeModelSelection();
        mrSlideSorter.GetController().GetPageSelector().CountSelectedPages();
    }
    CheckModel(*this);
}

void SlideSorterModel::ClearDescriptorList()
{
    ::std::vector<SharedPageDescriptor> aDescriptors;

    {
        ::osl::MutexGuard aGuard (maMutex);
        aDescriptors.swap(maPageDescriptors);
    }

    for (auto& rxDescriptor : aDescriptors)
    {
        if (rxDescriptor != nullptr)
        {
            if (rxDescriptor.use_count() > 1)
            {
                SAL_INFO(
                    "sd.sls",
                    "trying to delete page descriptor that is still used with"
                        " count " << rxDescriptor.use_count());
                // No assertion here because that can hang the office when
                // opening a dialog from here.
            }
            rxDescriptor.reset();
        }
    }
}

void SlideSorterModel::SynchronizeDocumentSelection()
{
    ::osl::MutexGuard aGuard (maMutex);

    PageEnumeration aAllPages (PageEnumerationProvider::CreateAllPagesEnumeration(*this));
    while (aAllPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        const bool bIsSelected (pDescriptor->HasState(PageDescriptor::ST_Selected));
        pDescriptor->GetPage()->SetSelected(bIsSelected);
    }
}

void SlideSorterModel::SynchronizeModelSelection()
{
    ::osl::MutexGuard aGuard (maMutex);

    PageEnumeration aAllPages (PageEnumerationProvider::CreateAllPagesEnumeration(*this));
    while (aAllPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        const bool bIsSelected (pDescriptor->GetPage()->IsSelected());
        pDescriptor->SetState(PageDescriptor::ST_Selected, bIsSelected);
    }
}

void SlideSorterModel::SetDocumentSlides (
    const Reference<container::XIndexAccess>& rxSlides)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Make the current selection persistent and then release the
    // current set of pages.
    SynchronizeDocumentSelection();
    mxSlides = nullptr;
    ClearDescriptorList ();

    // Reset the current page to cause everybody to release references to it.
    mrSlideSorter.GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(-1);

    // Set the new set of pages.
    mxSlides = rxSlides;
    AdaptSize();
    SynchronizeModelSelection();
    mrSlideSorter.GetController().GetPageSelector().CountSelectedPages();

    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(*this));
    if (aSelectedPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        mrSlideSorter.GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
            pDescriptor->GetPage());
    }

    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell != nullptr)
    {
        SdPage* pPage = pViewShell->getCurrentPage();
        if (pPage != nullptr)
            mrSlideSorter.GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
                pPage);
        else
        {
            // No current page.  This can only be when the slide sorter is
            // the main view shell.  Get current slide form frame view.
            const FrameView* pFrameView = pViewShell->GetFrameView();
            if (pFrameView != nullptr)
                mrSlideSorter.GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
                    pFrameView->GetSelectedPage());
            else
            {
                // No frame view.  As a last resort use the first slide as
                // current slide.
                mrSlideSorter.GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
                    sal_Int32(0));
            }
        }
    }

    mrSlideSorter.GetController().GetSlotManager()->NotifyEditModeChange();
}

Reference<container::XIndexAccess> SlideSorterModel::GetDocumentSlides() const
{
    ::osl::MutexGuard aGuard (maMutex);
    return mxSlides;
}

void SlideSorterModel::UpdatePageList()
{
    ::osl::MutexGuard aGuard (maMutex);

    Reference<container::XIndexAccess> xPages;

    // Get the list of pages according to the edit mode.
    Reference<frame::XController> xController (mrSlideSorter.GetXController());
    if (xController.is())
    {
        switch (meEditMode)
        {
            case EditMode::MasterPage:
            {
                Reference<drawing::XMasterPagesSupplier> xSupplier (
                    xController->getModel(), UNO_QUERY);
                if (xSupplier.is())
                {
                    xPages = xSupplier->getMasterPages();
                }
            }
            break;

            case EditMode::Page:
            {
                Reference<drawing::XDrawPagesSupplier> xSupplier (
                    xController->getModel(), UNO_QUERY);
                if (xSupplier.is())
                {
                    xPages = xSupplier->getDrawPages();
                }
            }
            break;

            default:
                // We should never get here.
                assert(false);
                break;
        }
    }

    mrSlideSorter.GetController().SetDocumentSlides(xPages);
}

void SlideSorterModel::AdaptSize()
{
    if (mxSlides.is())
        maPageDescriptors.resize(mxSlides->getCount());
    else
        maPageDescriptors.resize(0);
}

bool SlideSorterModel::IsReadOnly() const
{
    if (mrSlideSorter.GetViewShellBase() != nullptr
        && mrSlideSorter.GetViewShellBase()->GetDocShell())
        return mrSlideSorter.GetViewShellBase()->GetDocShell()->IsReadOnly();
    else
        return true;
}

void SlideSorterModel::SaveCurrentSelection()
{
    PageEnumeration aPages (PageEnumerationProvider::CreateAllPagesEnumeration(*this));
    while (aPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aPages.GetNextElement());
        pDescriptor->SetState(
            PageDescriptor::ST_WasSelected,
            pDescriptor->HasState(PageDescriptor::ST_Selected));
    }
}

vcl::Region SlideSorterModel::RestoreSelection()
{
    vcl::Region aRepaintRegion;
    PageEnumeration aPages (PageEnumerationProvider::CreateAllPagesEnumeration(*this));
    while (aPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aPages.GetNextElement());
        if (pDescriptor->SetState(
            PageDescriptor::ST_Selected,
            pDescriptor->HasState(PageDescriptor::ST_WasSelected)))
        {
            aRepaintRegion.Union(pDescriptor->GetBoundingBox());
        }
    }
    return aRepaintRegion;
}

bool SlideSorterModel::NotifyPageEvent (const SdrPage* pSdrPage)
{
    ::osl::MutexGuard aGuard (maMutex);

    SdPage* pPage = const_cast<SdPage*>(dynamic_cast<const SdPage*>(pSdrPage));
    if (pPage == nullptr)
        return false;

    // We are only interested in pages that are currently served by this
    // model.
    if (pPage->GetPageKind() != PageKind::Standard)
        return false;
    if (pPage->IsMasterPage() != (meEditMode==EditMode::MasterPage))
        return false;

    //NotifyPageEvent is called for add, remove, *and* change position so for
    //the change position case we must ensure we don't end up with the slide
    //duplicated in our list
    bool bSelected = DeleteSlide(pPage);
    if (pPage->IsInserted())
    {
        InsertSlide(pPage, bSelected);
    }
    CheckModel(*this);

    return true;
}

void SlideSorterModel::InsertSlide(SdPage* pPage, bool bMarkSelected)
{
    // Find the index at which to insert the given page.
    sal_uInt16 nCoreIndex (pPage->GetPageNum());
    sal_Int32 nIndex (FromCoreIndex(nCoreIndex));
    if (pPage != GetPage(nIndex))
        return;

    // Check that the pages in the document before and after the given page
    // are present in this model.
    if (nIndex>0)
        if (GetPage(nIndex-1) != GetPageDescriptor(nIndex-1)->GetPage())
            return;
    if (nIndex < static_cast<sal_Int32>(maPageDescriptors.size()) -1)
        if (GetPage(nIndex+1) != GetPageDescriptor(nIndex)->GetPage())
            return;

    auto iter = maPageDescriptors.begin() + nIndex;

    // Insert the given page at index nIndex
    iter = maPageDescriptors.insert(
        iter,
        std::make_shared<PageDescriptor>(
                Reference<drawing::XDrawPage>(mxSlides->getByIndex(nIndex),UNO_QUERY),
                pPage,
                nIndex));

    if (bMarkSelected)
        (*iter)->SetState(PageDescriptor::ST_Selected, true);

    // Update page indices.
    UpdateIndices(nIndex+1);
}

bool SlideSorterModel::DeleteSlide (const SdPage* pPage)
{
    sal_Int32 nIndex(0);

    // Caution, GetIndex() may be negative since it uses GetPageNumber()-1
    // for calculation, so do this only when page is inserted, else the
    // GetPageNumber() will be zero and thus GetIndex() == -1
    if(pPage->IsInserted())
    {
        nIndex = GetIndex(pPage);
    }
    else
    {
        // if not inserted, search for page
        for(; nIndex < static_cast<sal_Int32>(maPageDescriptors.size()); nIndex++)
        {
            if(maPageDescriptors[nIndex]->GetPage() == pPage)
            {
                break;
            }
        }
    }

    bool bMarkedSelected(false);

    if(nIndex >= 0 && o3tl::make_unsigned(nIndex) < maPageDescriptors.size())
    {
        if (maPageDescriptors[nIndex])
            if (maPageDescriptors[nIndex]->GetPage() != pPage)
                return false;

        auto iter = maPageDescriptors.begin() + nIndex;
        bMarkedSelected = (*iter)->HasState(PageDescriptor::ST_Selected);
        maPageDescriptors.erase(iter);
        UpdateIndices(nIndex);

        collectUIInformation(OUString::number(nIndex + 1), u"Delete_Slide_or_Page"_ustr);
    }
    return bMarkedSelected;
}

void SlideSorterModel::UpdateIndices (const sal_Int32 nFirstIndex)
{
    for (sal_Int32 nDescriptorIndex=0,nCount=maPageDescriptors.size();
         nDescriptorIndex<nCount;
         ++nDescriptorIndex)
    {
        SharedPageDescriptor& rpDescriptor (maPageDescriptors[nDescriptorIndex]);
        if (rpDescriptor)
        {
            if (nDescriptorIndex < nFirstIndex)
            {
                if (rpDescriptor->GetPageIndex()!=nDescriptorIndex)
                {
                    assert(rpDescriptor->GetPageIndex()==nDescriptorIndex);
                }
            }
            else
            {
                rpDescriptor->SetPageIndex(nDescriptorIndex);
            }
        }
    }
}

SdPage* SlideSorterModel::GetPage (const sal_Int32 nSdIndex) const
{
    SdDrawDocument* pModel = const_cast<SlideSorterModel*>(this)->GetDocument();
    if (pModel != nullptr)
    {
        if (meEditMode == EditMode::Page)
            return pModel->GetSdPage (static_cast<sal_uInt16>(nSdIndex), PageKind::Standard);
        else
            return pModel->GetMasterSdPage (static_cast<sal_uInt16>(nSdIndex), PageKind::Standard);
    }
    else
        return nullptr;
}

} // end of namespace ::sd::slidesorter::model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
