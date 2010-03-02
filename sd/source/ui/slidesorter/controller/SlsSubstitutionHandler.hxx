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

#ifndef SD_SLIDESORTER_SUBSTITUTION_HANDLER_HXX
#define SD_SLIDESORTER_SUBSTITUTION_HANDLER_HXX

#include <tools/gen.hxx>

#include "model/SlsSharedPageDescriptor.hxx"
#include "view/SlsViewOverlay.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"

namespace sd { namespace slidesorter {
class SlideSorter;
} }



namespace sd { namespace slidesorter { namespace controller {


/** A SubstitutionHandler object handles the display of a number of selected
    slides at the mouse position and the insertion or (with or without
    removing the pages at their original position) when the object is
    destoyed.
*/
class SubstitutionHandler
{
public:
    /** Create a substitution display of the currently selected pages and
        use the given position as the anchor point.
    */
    SubstitutionHandler (
        SlideSorter& rSlideSorter,
        const model::SharedPageDescriptor& rpHitDescriptor,
        const Point& rMouseModelPosition);
    ~SubstitutionHandler (void);

    /** Call this method (for example as reaction to ESC key press) to avoid
        processing (ie moving or inserting) the substition when the called
        SubstitutionHandler object is destroyed.
    */
    void Dispose (void);

    /** Move the substitution display by the distance the mouse has
        travelled since the last call to this method or to
        CreateSubstitution().  The given point becomes the new anchor.
    */
    void UpdatePosition (
        const Point& rMousePosition,
        const InsertionIndicatorHandler::Mode eMode,
        const bool bAllowAutoScroll = true);

    void Show (void);
    void Hide (void);
    void SetTargetSlideSorter (
        SlideSorter* pSlideSorter = NULL,
        const Point aMousePosition = Point(0,0),
        const InsertionIndicatorHandler::Mode eMode = InsertionIndicatorHandler::UnknownMode,
        const bool bIsOverSourceView = false);

private:
    SlideSorter* mpTargetSlideSorter;
    model::SharedPageDescriptor mpHitDescriptor;
    sal_Int32 mnInsertionIndex;

    /** Move the substitution display of the currently selected pages.
    */
    void Process (void);
};



} } } // end of namespace ::sd::slidesorter::controller

#endif
