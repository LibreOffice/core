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

#include <sal/types.h>
#include <tools/link.hxx>
#include <vector>
#include <memory>

class SdPage;

namespace sd::slidesorter
{
class SlideSorter;
}

namespace sd::slidesorter::controller
{
class SlideSorterController;
class SelectionObserver;

/** This class is a part of the controller and handles the selection of
    slides.
    <p>It has methods to modify the selected slides (delete them or
    move them to other places in the document), change the visible area so
    to make the selected slides visible, tell listeners when the selection
    changes.</p>
*/
class SelectionManager
{
public:
    /** Create a new SelectionManager for the given slide sorter.
    */
    SelectionManager(SlideSorter& rSlideSorter);

    ~SelectionManager();

    /** Delete the currently selected slides.  When this method returns the
        selection is empty.
        @param bSelectFollowingPage
            When <TRUE/> then after deleting the selected pages make the
            slide after the last selected page the new current page.
            When <FALSE/> then make the first slide before the selected
            pages the new current slide.
    */
    void DeleteSelectedPages(const bool bSelectFollowingPage = true);

    /** Call this method after the selection has changed (possible several
        calls to the PageSelector) to invalidate the relevant slots and send
        appropriate events.
    */
    void SelectionHasChanged();

    /** Add a listener that is called when the selection of the slide sorter
        changes.
        @param rListener
            When this method is called multiple times for the same listener
            the second and all following calls are ignored.  Each listener
            is added only once.
    */
    void AddSelectionChangeListener(const Link<LinkParamNone*, void>& rListener);

    /** Remove a listener that was called when the selection of the slide
        sorter changes.
        @param rListener
            It is safe to pass a listener that was not added are has been
            removed previously.  Such calls are ignored.
    */
    void RemoveSelectionChangeListener(const Link<LinkParamNone*, void>& rListener);

    /** Return the position where to insert pasted slides based on the
        current selection.  When there is a selection then the insert
        position is behind the last slide.  When the selection is empty then
        most of the time the insert position is at the end of the document.
        There is an exception right after the display of a popup-menu.  The
        position of the associated insertion marker is stored here and reset
        the next time the selection changes.
    */
    sal_Int32 GetInsertionPosition() const;

    /** Store an insertion position temporarily.  It is reset when the
        selection changes the next time.
    */
    void SetInsertionPosition(const sal_Int32 nInsertionPosition);

    const std::shared_ptr<SelectionObserver>& GetSelectionObserver() const
    {
        return mpSelectionObserver;
    }

private:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    ::std::vector<Link<LinkParamNone*, void>> maSelectionChangeListeners;

    /** The insertion position is only temporarily valid.  Negative values
        indicate that the explicit insertion position is not valid.  In this
        case GetInsertionPosition() calculates it from the current selection.
    */
    sal_Int32 mnInsertionPosition;

    std::shared_ptr<SelectionObserver> mpSelectionObserver;

    /** Delete the given list of normal pages.  This method is a helper
        function for DeleteSelectedPages().
        @param rSelectedNormalPages
            A list of normal pages.  Supplying master pages is an error.
    */
    void DeleteSelectedNormalPages(const ::std::vector<SdPage*>& rSelectedNormalPages);

    /** Delete the given list of master pages.  This method is a helper
        function for DeleteSelectedPages().
        @param rSelectedMasterPages
            A list of master pages.  Supplying normal pages is an error.
    */
    void DeleteSelectedMasterPages(const ::std::vector<SdPage*>& rSelectedMasterPages);
};

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
