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

#ifndef SD_SLIDESORTER_SUBSTITUTION_HANDLER_HXX
#define SD_SLIDESORTER_SUBSTITUTION_HANDLER_HXX

#include <tools/gen.hxx>

#include "model/SlsSharedPageDescriptor.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include <vector>


namespace sd { namespace slidesorter {
class SlideSorter;
} }



namespace sd { namespace slidesorter { namespace controller {

/** A DragAndDropContext object handles an active drag and drop operation.
    When the mouse is moved from one slide sorter window to another the
    target SlideSorter object is exchanged accordingly.
*/
class DragAndDropContext
{
public:
    /** Create a substitution display of the currently selected pages or,
        when provided, the pages in the transferable.
    */
    DragAndDropContext (SlideSorter& rSlideSorter);
    ~DragAndDropContext (void);

    /** Call this method (for example as reaction to ESC key press) to avoid
        processing (ie moving or inserting) the substition when the called
        DragAndDropContext object is destroyed.
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

    void SetTargetSlideSorter (
        SlideSorter* pSlideSorter = NULL,
        const Point aMousePosition = Point(0,0),
        const InsertionIndicatorHandler::Mode eMode = InsertionIndicatorHandler::UnknownMode,
        const bool bIsOverSourceView = false);

private:
    SlideSorter* mpTargetSlideSorter;
    model::SharedPageDescriptor mpHitDescriptor;
    sal_Int32 mnInsertionIndex;

    void GetPagesFromBookmarks (
        ::std::vector<const SdPage*>& rPages,
        sal_Int32& rnSelectionCount,
        DrawDocShell* pDocShell,
        const List& rBookmarks) const;
    void GetPagesFromSelection (
        ::std::vector<const SdPage*>& rPages,
        sal_Int32& rnSelectionCount,
        model::PageEnumeration& rSelection) const;
};



} } } // end of namespace ::sd::slidesorter::controller

#endif
