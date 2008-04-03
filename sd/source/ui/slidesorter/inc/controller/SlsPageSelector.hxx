/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageSelector.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:34:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_PAGE_SELECTOR_HXX
#define SD_SLIDESORTER_PAGE_SELECTOR_HXX

#include "model/SlsSharedPageDescriptor.hxx"

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vector>
#include <memory>

class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;


/** A sub-controller that handles page selection of the slide browser.
    Selecting a page does not make it the current page (of the main view)
    automatically as this would not be desired in a multi selection.  This
    has to be done explicitly by calling the
    CurrentSlideManager::SetCurrentSlide() method.

    Indices of pages relate allways to the number of all pages in the model
    (as returned by GetPageCount()) not just the selected pages.
*/
class PageSelector
{
public:
    PageSelector (SlideSorter& rSlideSorter);

    void SelectAllPages (void);
    void DeselectAllPages (void);
    /** Update the selection state of all page descriptors to be the same as
        that of the pages of the SdDrawDocument they describe and issue
        redraw requests where necessary.
    */
    void UpdateAllPages (void);

    void SelectPage (int nPageIndex);
    /** Select the descriptor that is associated with the given page.
    */
    void SelectPage (const SdPage* pPage);
    void SelectPage (const model::SharedPageDescriptor& rpDescriptor);

    /** Return whether the specified page is selected.  This convenience
        method is a subsitute for
        SlideSorterModel::GetPageDescriptor(i)->IsSelected() is included
        here to make this class more self contained.
    */
    bool IsPageSelected (int nPageIndex);

    /** Deselect the descriptor that is associated with the given page.
    */
    void DeselectPage (int nPageIndex);
    void DeselectPage (const SdPage* pPage);
    void DeselectPage (const model::SharedPageDescriptor& rpDescriptor);

    /** This convenience method returns the same number of pages that
        SlideSorterModel.GetPageCount() returns.  It is included here so
        that it is self contained for iterating over all pages to select or
        deselect them.
    */
    int GetPageCount (void) const;
    int GetSelectedPageCount (void) const;

    void PrepareModelChange (void);
    void HandleModelChange (void);

    /** Enable the broadcasting of selection change events.  This calls the
        SlideSorterController::SelectionHasChanged() method to do the actual
        work.  When EnableBroadcasting has been called as many times as
        DisableBroadcasting() was called before and the selection has been
        changed in the mean time, this change will be broadcasted.
    */
    void EnableBroadcasting (bool bMakeSelectionVisible = true);

    /** Disable the broadcasting o selectio change events.  Subsequent
        changes of the selection will set a flag that triggers the sending
        of events when EnableBroadcasting() is called.
    */
    void DisableBroadcasting (void);

    /** Return the descriptor of the most recently selected page.  This
        works only when the page has not been de-selected in the mean time.
        This method helps the view when it scrolls the selection into the
        visible area.
        @return
            When the selection is empty or when the most recently selected
            page has been deselected already (but other pages are still
            selected) then NULL is returned, even when a selection did exist
            but has been cleared.
    */
    model::SharedPageDescriptor GetMostRecentlySelectedPage (void) const;

    /** Return the anchor for a range selection.  This usually is the first
        selected page after all pages have been deselected.
        @return
            The returned anchor may be NULL.
    */
    model::SharedPageDescriptor GetSelectionAnchor (void) const;


    typedef ::std::vector<SdPage*> PageSelection;

    /** Return an object that describes the current selection.  The caller
        can use that object to later restore the selection.
        @return
            The object returned describes the selection via indices.  So
            even if pages are exchanged a later call to SetPageSelection()
            is valid.
    */
    ::boost::shared_ptr<PageSelection> GetPageSelection (void) const;

    /** Restore a page selection according to the given selection object.
        @param rSelection
            Typically obtained by calling GetPageSelection() this object
            is used to restore the selection.  If pages were exchanged since
            the last call to GetPageSelection() it is still valid to call
            this method with the selection.  When pages have been inserted
            or removed the result may be unexpected.
    */
    void SetPageSelection (const ::boost::shared_ptr<PageSelection>& rSelection);

    void UpdateCurrentPage (const model::SharedPageDescriptor& rCurrentPageDescriptor);

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
    model::SharedPageDescriptor mpCurrentPage;

    void CountSelectedPages (void);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
