/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsSelectionManager.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_SLIDESORTER_CONTROLLER_SELECTION_MANAGER_HXX
#define SD_SLIDESORTER_CONTROLLER_SELECTION_MANAGER_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <sal/types.h>
#include <tools/gen.hxx>
#include <vector>

class Link;
class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

/** This class is a part of the controller and handles the selection of
    slides.
    <p>It has methods to modify the selected slides (delete them or
    move them to other places in the document), change the visible area so
    to make the selected slides visble, tell listeners when the selection
    changes.</p>
*/
class SelectionManager
{
public:
    /** Create a new SelectionManger for the given slide sorter.
    */
    SelectionManager (SlideSorter& rSlideSorter);

    ~SelectionManager (void);

    /** Delete the currently selected slides.  When this method returns the
        selection is empty.
    */
    void DeleteSelectedPages (void);

    /** Move the maked pages to a position directly after the specified page.
    */
    bool MoveSelectedPages (const sal_Int32 nTargetPage);

    /** Call this method after the selection has changed (possible several
        calls to the PageSelector) to invalidate the relevant slots and send
        appropriate events.
    */
    void SelectionHasChanged (const bool bMakeSelectionVisible = true);

    /** Return <TRUE/> when the selection has changed but has not yet been
        moved to the visible area of the slide sorter view.
    */
    bool IsMakeSelectionVisiblePending (void) const;

    enum SelectionHint { SH_FIRST, SH_LAST, SH_RECENT };

    /** Try to make all currently selected page objects visible, i.e. set
        the origin so that the page objects lie inside the visible area.
        When the selection is empty then the visible area is not modified.

        <p>This method, and the ones is calls, look into the Properties
        object of the SlideSorter in order to determine whether the current
        selection is to be displayed centered.</p>
        @param eSelectionHint
            This is an advice on which selected page object to handle with
            the highest priority when the whole selection does not fit into
            the visible area.
        @return
            Returns the vertical translation of the visible area.  It is 0
            when no update of the visible area was done.
    */
    Size MakeSelectionVisible (
        const SelectionHint eSelectionHint = SH_RECENT);

    /** Modify the origin of the visible area so that the given rectangle
        comes into view.  This is done with the smallest change: no
        scrolling takes place when the given rectangle already lies in the
        visible area.  Otherwise either the top or the bottom of the given
        rectangle is aligned with the top or the bottom of the visible area.
        @return
            Returns the vertical translation of the visible area.  It is 0
            when no update of the visible area was done.
    */
    Size MakeRectangleVisible (const Rectangle& rBox);

    /** Add a listener that is called when the selection of the slide sorter
        changes.
        @param rListener
            When this method is called multiple times for the same listener
            the second and all following calls are ignored.  Each listener
            is added only once.
    */
    void AddSelectionChangeListener (const Link& rListener);

    /** Remove a listener that was called when the selection of the slide
        sorter changes.
        @param rListener
            It is save to pass a listener that was not added are has been
            removed previously.  Such calls are ignored.
    */
    void RemoveSelectionChangeListener (const Link& rListener);

private:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    ::std::vector<Link> maSelectionChangeListeners;

    /** This array stores the indices of the  selected page descriptors at
        the time when the edit mode is switched to EM_MASTERPAGE.  With this
        we can restore the selection when switching back to EM_PAGE mode.
    */
    ::std::vector<SdPage*> maSelectionBeforeSwitch;

    /** When this flag is set then on the next call to Paint() the selection
        is moved into the visible area.
    */
    bool mbIsMakeSelectionVisiblePending;

    /** Delete the given list of normal pages.  This method is a helper
        function for DeleteSelectedPages().
        @param rSelectedNormalPages
            A list of normal pages.  Supplying master pages is an error.
    */
    void DeleteSelectedNormalPages (const ::std::vector<SdPage*>& rSelectedNormalPages);

    /** Delete the given list of master pages.  This method is a helper
        function for DeleteSelectedPages().
        @param rSelectedMasterPages
            A list of master pages.  Supplying normal pages is an error.
    */
    void DeleteSelectedMasterPages (const ::std::vector<SdPage*>& rSelectedMasterPages);

    /** Return <TRUE/> when the given rectangle, that typically is the
        bounding box of all currently selected slides, does not fit entirely
        into the visible area of the slide sorter view.
    */
    bool DoesSelectionExceedVisibleArea (const Rectangle& rSelectionBox) const;

    /** When not all currently selected slides fit into the visible area of
        the slide sorter view, and thus DoesSelectionExceedVisibleArea()
        would return <TRUE/>, then it is the task of this method to
        determine which part of the selection to move into the visible area.
        @param rpFirst
            The first selected slide.  Must not be an empty pointer.
        @param rpLast
            The last selected slide.  Must not be an empty pointer.
        @param eSelectionHint
            This hint tells the method on which slide to concentrate,
            i.e. which slide has to be inside the returned visible area.
        @return
            Returns the new visible area.
    */
    Rectangle ResolveLargeSelection (
        const model::SharedPageDescriptor& rpFirst,
        const model::SharedPageDescriptor& rpLast,
        const SelectionHint eSelectionHint);

};

} } } // end of namespace ::sd::slidesorter::controller

#endif
