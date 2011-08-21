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

#ifndef SD_SLIDESORTER_INSERTION_INDICATOR_HANDLER_HXX
#define SD_SLIDESORTER_INSERTION_INDICATOR_HANDLER_HXX

#include "view/SlsInsertAnimator.hxx"

#include "view/SlsLayouter.hxx"

namespace sd { namespace slidesorter { class SlideSorter; } }
namespace sd { namespace slidesorter { namespace model {
class PageEnumeration;
} } }
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
    ~InsertionIndicatorHandler (void);

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
        ForceShowContext (const ::boost::shared_ptr<InsertionIndicatorHandler>& rpHandler);
        ~ForceShowContext (void);
    private:
        const ::boost::shared_ptr<InsertionIndicatorHandler> mpHandler;
    };

    /** Update the indicator icon from the current transferable (from the
        clipboard or an active drag and drop operation.)
    */
    void UpdateIndicatorIcon (const Transferable* pTransferable);

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
    bool IsActive (void) const;

    /** Return the insertion index that corresponds with the current
        graphical location of the insertion indicator.
    */
    sal_Int32 GetInsertionPageIndex (void) const;

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
    ::boost::shared_ptr<view::InsertAnimator> mpInsertAnimator;
    ::boost::shared_ptr<view::InsertionIndicatorOverlay> mpInsertionIndicatorOverlay;
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
    ::boost::shared_ptr<view::InsertAnimator> GetInsertAnimator (void);

    /** Make the insertion indicator visible (that is the show part) and
        keep it visible, even when the mouse leaves the window (that is the
        force part).  We need this when a context menu is displayed (mouse
        over the popup menu triggers a mouse leave event) while the
        insertion indicator remains visible in the background.

        In effect all calls to End() are ignored until ForceEnd() is called.
    */
    void ForceShow (void);
    void ForceEnd (void);
};


} } } // end of namespace ::sd::slidesorter::controller

#endif
