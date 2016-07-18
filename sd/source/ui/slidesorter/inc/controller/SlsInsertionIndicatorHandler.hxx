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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSINSERTIONINDICATORHANDLER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSINSERTIONINDICATORHANDLER_HXX

#include "view/SlsInsertAnimator.hxx"

#include "view/SlsLayouter.hxx"
#include "sdxfer.hxx"

namespace sd { namespace slidesorter { class SlideSorter; } }
namespace sd { namespace slidesorter { namespace view {
class InsertAnimator;
class InsertionIndicatorOverlay;
} } }

namespace sd { namespace slidesorter { namespace controller {

class Transferable;

/** Manage the visibility and location of the insertion indicator.  Its
    actual display is controlled by the InsertionIndicatorOverlay.
*/
class InsertionIndicatorHandler
{
public:
    InsertionIndicatorHandler (SlideSorter& rSlideSorter);
    ~InsertionIndicatorHandler();

    enum Mode { CopyMode, MoveMode, UnknownMode };
    static Mode GetModeFromDndAction (const sal_Int8 nDndAction);

    /** Activate the insertion marker at the given coordinates.
    */
    void Start (const bool bIsOverSourceView);

    /** Deactivate the insertion marker.
    */
    void End (const controller::Animator::AnimationMode eMode);

    /** This context make sure that the insertion indicator is shown
        (provided that the clipboard is not empty) while the context is
        alive.  Typically used while a context menu is displayed.
    */
    class ForceShowContext
    {
    public:
        ForceShowContext (const std::shared_ptr<InsertionIndicatorHandler>& rpHandler);
        ~ForceShowContext();
    private:
        const std::shared_ptr<InsertionIndicatorHandler> mpHandler;
    };

    /** Update the indicator icon from the current transferable (from the
        clipboard or an active drag and drop operation.)
    */
    void UpdateIndicatorIcon (const SdTransferable* pTransferable);

    /** Set the position of the insertion marker to the given coordinates.
    */
    void UpdatePosition (
        const Point& rMouseModelPosition,
        const Mode eMode);
    void UpdatePosition (
        const Point& rMouseModelPosition,
        const sal_Int8 nDndAction);

    /** Return whether the insertion marker is active.
    */
    bool IsActive() const { return mbIsActive;}

    /** Return the insertion index that corresponds with the current
        graphical location of the insertion indicator.
    */
    sal_Int32 GetInsertionPageIndex() const;

    /** Determine whether moving the current selection to the current
        position of the insertion marker would alter the document.  This
        would be the case when the selection is not consecutive or would be
        moved to a position outside and not adjacent to the selection.
    */
    bool IsInsertionTrivial (
        const sal_Int32 nInsertionIndex,
        const Mode eMode) const;
    /** This method is like the other variant.  It operates implicitly
        on the current insertion index as would be returned by
        GetInsertionPageIndex().
    */
    bool IsInsertionTrivial (const sal_Int8 nDndAction);

private:
    SlideSorter& mrSlideSorter;
    std::shared_ptr<view::InsertAnimator> mpInsertAnimator;
    std::shared_ptr<view::InsertionIndicatorOverlay> mpInsertionIndicatorOverlay;
    view::InsertPosition maInsertPosition;
    Mode meMode;
    bool mbIsInsertionTrivial;
    bool mbIsActive;
    bool mbIsReadOnly;
    bool mbIsOverSourceView;
    Size maIconSize;
    bool mbIsForcedShow;

    void SetPosition (
        const Point& rPoint,
        const Mode eMode);
    std::shared_ptr<view::InsertAnimator> const & GetInsertAnimator();

    /** Make the insertion indicator visible (that is the show part) and
        keep it visible, even when the mouse leaves the window (that is the
        force part).  We need this when a context menu is displayed (mouse
        over the popup menu triggers a mouse leave event) while the
        insertion indicator remains visible in the background.

        In effect all calls to End() are ignored until ForceEnd() is called.
    */
    void ForceShow();
    void ForceEnd();
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
