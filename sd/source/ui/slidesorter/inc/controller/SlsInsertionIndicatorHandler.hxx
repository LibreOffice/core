/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsSelectionFunction.cxx,v $
 * $Revision: 1.37 $
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

namespace sd { namespace slidesorter { class SlideSorter; } }
namespace sd { namespace slidesorter { namespace view {
class InsertAnimator;
class InsertionIndicatorOverlay;
} } }


namespace sd { namespace slidesorter { namespace controller {


/** Manage the visibility and location of the insertion indicator.  Its
    actual display is controlled by the InsertionIndicatorOverlay.
*/
class InsertionIndicatorHandler
{
public:
    InsertionIndicatorHandler (SlideSorter& rSlideSorter);
    ~InsertionIndicatorHandler (void);

    /** Activate the insertion marker at the given coordinates.
    */
    void Start (const Point& rMouseModelPosition);

    /** Set the position of the insertion marker to the given coordinates.
    */
    void UpdatePosition (const Point& rMouseModelPosition);

    /** Deactivate the insertion marker.
    */
    void End (void);

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
    bool IsInsertionTrivial (void) const;

private:
    SlideSorter& mrSlideSorter;
    ::boost::shared_ptr<view::InsertAnimator> mpInsertAnimator;
    ::boost::shared_ptr<view::InsertionIndicatorOverlay> mpInsertionIndicatorOverlay;
    sal_Int32 mnInsertionIndex;
    bool mbIsBeforePage;
    bool mbIsInsertionTrivial;
    bool mbIsActive;

    void SetPosition (const Point& rPoint);
    ::boost::shared_ptr<view::InsertAnimator> GetInsertAnimator (void);
};


} } } // end of namespace ::sd::slidesorter::controller

#endif
