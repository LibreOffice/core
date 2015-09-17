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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSFOCUSMANAGER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSFOCUSMANAGER_HXX

#include <model/SlsSharedPageDescriptor.hxx>

#include <sal/types.h>
#include <tools/link.hxx>
#include <vector>

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {

/** This class manages the focus of the slide sorter.  There is the focus
    page which is or is not focused.  Initialized to point to the first page
    it can be set to other pages by using the MoveFocus() method.  The
    focused state of the focus page can be toggled with the ToggleFocus()
    method.
*/
class FocusManager
{
public:
    /** Create a new focus manager that operates on the pages of the model
        associated with the given controller.  The focus page is set to the
        first page.  Focused state is off.
    */
    FocusManager (SlideSorter& rSlideSorter);

    ~FocusManager();

    enum FocusMoveDirection
    {
        FMD_NONE,
        FMD_LEFT,
        FMD_RIGHT,
        FMD_UP,
        FMD_DOWN
    };

    /** Move the focus from the currently focused page to one that is
        displayed adjacent to it, either vertically or horizontally.
        @param eDirection
            Direction in which to move the focus.  Wrap around is done
            differently when moving vertically or horizontally.  Vertical
            wrap around takes place in the same column, i.e. when you are
            in the top row and move up you come out in the bottom row in the
            same column.  Horizontal wrap around moves to the next
            (FMD_RIGHT) or previous (FMD_LEFT) page.  Moving to the right
            from the last page goes to the first page and vice versa.
            When FMD_NONE is given, the current page index is checked for
            being valid.  If it is not, then it is set to the nearest valid
            page index.
    */
    void MoveFocus (FocusMoveDirection eDirection);

    /** Show the focus indicator of the current slide.
        @param bScrollToFocus
            When <TRUE/> (the default) then the view is scrolled so that the
            focus rectangle lies inside its visible area.
    */
    void ShowFocus (const bool bScrollToFocus = true);

    /** Hide the focus indicator.
    */
    void HideFocus();

    /** Toggle the focused state of the current slide.
        @return
            Returns the focused state of the focus page after the call.
    */
    bool ToggleFocus();

    /** Return whether the window managed by the called focus manager has
        the input focus of the application.
    */
    bool HasFocus() const;

    /** Return the descriptor of the page that currently has the focus.
        @return
            When there is no page that currently has the focus then NULL is
            returned.
    */
    model::SharedPageDescriptor GetFocusedPageDescriptor() const;

    /** Return the index of the page that currently has the focus as it is
        accepted by the slide sorter model.
        @return
            When there is no page that currently has the focus then -1 is
            returned.
    */
    sal_Int32 GetFocusedPageIndex() const { return mnPageIndex;}

    /** Set the focused page to the one described by the given page
        descriptor.  The visibility of the focus indicator is not modified.
        @param rDescriptor
            One of the page descriptors that are currently managed by the
            SlideSorterModel.
    */
    void SetFocusedPage (const model::SharedPageDescriptor& rDescriptor);

    /** Set the focused page to the one described by the given page
        index.  The visibility of the focus indicator is not modified.
        @param nPageIndex
            A valid page index that is understood by the SlideSorterModel.
    */
    void SetFocusedPage (sal_Int32 nPageIndex);

    void SetFocusedPageToCurrentPage();

    /** Return <TRUE/> when the focus inidcator is currently shown.  A
        prerequisite is that the window managed by this focus manager has
        the input focus as indicated by a <TRUE/> return value of
        HasFocus().  It is not necessary that the focus indicator is
        visible.  It may have been scrolled outside the visible area.
    */
    bool IsFocusShowing() const;

    /** Add a listener that is called when the focus is shown or hidden or
        set to another page object.
        @param rListener
            When this method is called multiple times for the same listener
            the second and all following calls are ignored.  Each listener
            is added only once.
    */
    void AddFocusChangeListener (const Link<LinkParamNone*,void>& rListener);

    /** Remove a focus change listener.
        @param rListener
            It is save to pass a listener that was not added are has been
            removed previously.  Such calls are ignored.
    */
    void RemoveFocusChangeListener (const Link<LinkParamNone*,void>& rListener);

    /** Create an instance of this class to temporarily hide the focus
        indicator.  It is restored to its former visibility state when the
        FocusHider is destroyed.
    */
    class FocusHider
    {
    public:
        FocusHider (FocusManager&);
        ~FocusHider();
    private:
        bool mbFocusVisible;
        FocusManager& mrManager;
    };

private:
    SlideSorter& mrSlideSorter;

    /** Index of the page that may be focused.  It is -1 when the model
        contains no page.
    */
    sal_Int32 mnPageIndex;

    /** This flag indicates whether the page pointed to by mpFocusDescriptor
        has the focus.
    */
    bool mbPageIsFocused;

    ::std::vector<Link<LinkParamNone*,void>> maFocusChangeListeners;

    /** When vertical wrap is active then pressing UP in the top row moves
        the focus to the bottom row, DOWN in the bottom row moves the focus
        to the top row.
    */
    bool mbIsVerticalWrapActive;

    /** Reset the focus state of the given descriptor and request a repaint
        so that the focus indicator is hidden.
        @param pDescriptor
            When NULL is given then the call is ignored.
    */
    void HideFocusIndicator (const model::SharedPageDescriptor& rpDescriptor);

    /** Set the focus state of the given descriptor, scroll it into the
        visible area and request a repaint so that the focus indicator is
        made visible.
        @param pDescriptor
            When NULL is given then the call is ignored.
        @param bScrollToFocus
            When <TRUE/> (the default) then the view is scrolled so that the
            focus rectangle lies inside its visible area.
    */
    void ShowFocusIndicator (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bScrollToFocus);

    /** Call all currently registered listeners that a focus change has
        happened.  The focus may be hidden or shown or moved from one page
        object to another.
    */
    void NotifyFocusChangeListeners() const;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
