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

#pragma once

#include <model/SlsSharedPageDescriptor.hxx>

#include <vector>
#include <memory>

#include <sddllapi.h>

class SdPage;

namespace sd::slidesorter
{
class SlideSorter;
}
namespace sd::slidesorter::model
{
class SlideSorterModel;
}

namespace sd::slidesorter::controller
{
class SlideSorterController;

/** A sub-controller that handles page selection of the slide browser.
    Selecting a page does not make it the current page (of the main view)
    automatically as this would not be desired in a multi selection.  This
    has to be done explicitly by calling the
    CurrentSlideManager::SetCurrentSlide() method.

    Indices of pages relate always to the number of all pages in the model
    (as returned by GetPageCount()) not just the selected pages.
*/
class PageSelector
{
public:
    explicit PageSelector(SlideSorter& rSlideSorter);
    PageSelector(const PageSelector&) = delete;
    PageSelector& operator=(const PageSelector&) = delete;

    // Exported for unit test
    SD_DLLPUBLIC void SelectAllPages();
    SD_DLLPUBLIC void DeselectAllPages();

    /** Update the selection state of all page descriptors to be the same as
        that of the corresponding pages of the SdPage objects and issue
        redraw requests where necessary.
    */
    void GetCoreSelection();

    /** Update the selection state of the SdPage objects to be the same as
        that of the corresponding page descriptors.
    */
    void SetCoreSelection();

    /** Select the specified descriptor.  The selection state of the other
        descriptors is not affected.
    */
    void SelectPage(int nPageIndex);
    /** Select the descriptor that is associated with the given page.  The
        selection state of the other descriptors is not affected.
    */
    void SelectPage(const SdPage* pPage);
    /** Select the specified descriptor.  The selection state of the other
        descriptors is not affected.
    */
    void SelectPage(const model::SharedPageDescriptor& rpDescriptor);

    /** Return whether the specified page is selected.  This convenience
        method is a substitute for
        SlideSorterModel::GetPageDescriptor(i)->HasState(ST_Selected) is
        included here to make this class more self contained.
    */
    SD_DLLPUBLIC bool IsPageSelected(int nPageIndex);

    /** Return whether the specified page is visible.  This convenience
        method is a substitute for
        SlideSorterModel::GetPageDescriptor(i)->HasState(ST_Visible) is
        included here to make this class more self contained.
    */
    bool IsPageVisible(int nPageIndex);

    /** Deselect the descriptor that is associated with the given page.
        The current page is updated to the first slide
        of the remaining selection.
    */
    void DeselectPage(int nPageIndex);
    void DeselectPage(const model::SharedPageDescriptor& rpDescriptor,
                      const bool bUpdateCurrentPage = true);

    /** This convenience method returns the same number of pages that
        SlideSorterModel.GetPageCount() returns.  It is included here so
        that it is self contained for iterating over all pages to select or
        deselect them.
    */
    int GetPageCount() const;
    int GetSelectedPageCount() const { return mnSelectedPageCount; }

    /** Return the anchor for a range selection.  This usually is the first
        selected page after all pages have been deselected.
        @return
            The returned anchor may be NULL.
    */
    const model::SharedPageDescriptor& GetSelectionAnchor() const { return mpSelectionAnchor; }

    typedef ::std::vector<SdPage*> PageSelection;

    /** Return an object that describes the current selection.  The caller
        can use that object to later restore the selection.
        @return
            The object returned describes the selection via indices.  So
            even if pages are exchanged a later call to SetPageSelection()
            is valid.
    */
    std::shared_ptr<PageSelection> GetPageSelection() const;

    /** Restore a page selection according to the given selection object.
        @param rSelection
            Typically obtained by calling GetPageSelection() this object
            is used to restore the selection.  If pages were exchanged since
            the last call to GetPageSelection() it is still valid to call
            this method with the selection.  When pages have been inserted
            or removed the result may be unexpected.
        @param bUpdateCurrentPage
            When <TRUE/> (the default value) then after setting the
            selection update the current page to the first page of the
            selection.
            When called from within UpdateCurrentPage() then this flag is
            used to prevent a recursion loop.
    */
    void SetPageSelection(const std::shared_ptr<PageSelection>& rSelection,
                          const bool bUpdateCurrentPage);

    /** Call this method after the model has changed to set the number
        of selected pages.
    */
    void CountSelectedPages();

    /** Use the UpdateLock whenever you do a complex selection, i.e. call
        more than one method in a row.  An active lock prevents intermediate
        changes of the current slide.
    */
    class UpdateLock
    {
    public:
        UpdateLock(SlideSorter const& rSlideSorter);
        UpdateLock(PageSelector& rPageSelector);
        ~UpdateLock();
        void Release();

    private:
        PageSelector* mpSelector;
    };

    class BroadcastLock
    {
    public:
        BroadcastLock(SlideSorter const& rSlideSorter);
        BroadcastLock(PageSelector& rPageSelector);
        ~BroadcastLock();

    private:
        PageSelector& mrSelector;
    };

private:
    model::SlideSorterModel& mrModel;
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;
    int mnSelectedPageCount;
    int mnBroadcastDisableLevel;
    bool mbSelectionChangeBroadcastPending;
    model::SharedPageDescriptor mpMostRecentlySelectedPage;
    /// Anchor for a range selection.
    model::SharedPageDescriptor mpSelectionAnchor;
    sal_Int32 mnUpdateLockCount;
    bool mbIsUpdateCurrentPagePending;

    /** Enable the broadcasting of selection change events.  This calls the
        SlideSorterController::SelectionHasChanged() method to do the actual
        work.  When EnableBroadcasting has been called as many times as
        DisableBroadcasting() was called before and the selection has been
        changed in the meantime, this change will be broadcasted.
    */
    void EnableBroadcasting();

    /** Disable the broadcasting of selection change events.  Subsequent
        changes of the selection will set a flag that triggers the sending
        of events when EnableBroadcasting() is called.
    */
    void DisableBroadcasting();

    void UpdateCurrentPage(const bool bUpdateOnlyWhenPending = false);

    void CheckConsistency() const;
};

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
