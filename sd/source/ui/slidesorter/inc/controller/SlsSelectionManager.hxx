/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_SLIDESORTER_CONTROLLER_SELECTION_MANAGER_HXX
#define SD_SLIDESORTER_CONTROLLER_SELECTION_MANAGER_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include "controller/SlsAnimator.hxx"
#include <sal/types.h>
#include <tools/gen.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <vector>

class Link;
class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;
class SelectionObserver;

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
        @param bSelectFollowingPage
            When <TRUE/> then after deleting the selected pages make the
            slide after the last selected page the new current page.
            When <FALSE/> then make the first slide before the selected
            pages the new current slide.
    */
    void DeleteSelectedPages (const bool bSelectFollowingPage = true);

    /** Call this method after the selection has changed (possible several
        calls to the PageSelector) to invalidate the relevant slots and send
        appropriate events.
    */
    void SelectionHasChanged (const bool bMakeSelectionVisible = true);

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

    /** Return the position where to insert pasted slides based on the
        current selection.  When there is a selection then the insert
        position is behind the last slide.  When the selection is empty then
        most of the time the insert position is at the end of the document.
        There is an exception right after the display of a popup-menu.  The
        position of the associated insertion marker is stored here and reset
        the next time the selection changes.
    */
    sal_Int32 GetInsertionPosition (void) const;

    /** Store an insertion position temporarily.  It is reset when the
        selection changes the next time.
    */
    void SetInsertionPosition (const sal_Int32 nInsertionPosition);

    ::boost::shared_ptr<SelectionObserver> GetSelectionObserver (void) const;

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

    /** The insertion position is only temporarily valid.  Negative values
        indicate that the explicit insertion position is not valid.  In this
        case GetInsertionPosition() calculates it from the current selection.
    */
    sal_Int32 mnInsertionPosition;

    /** Animation id for a scroll animation the will eventually set the top
        and left of the visible area to maRequestedTopLeft.
    */
    Animator::AnimationId mnAnimationId;
    Point maRequestedTopLeft;

    class PageInsertionListener;
    ::boost::scoped_ptr<PageInsertionListener> mpPageInsertionListener;

    ::boost::shared_ptr<SelectionObserver> mpSelectionObserver;

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
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
