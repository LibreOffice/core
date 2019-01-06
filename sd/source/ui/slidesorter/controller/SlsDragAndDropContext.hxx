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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CONTROLLER_SLSDRAGANDDROPCONTEXT_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CONTROLLER_SLSDRAGANDDROPCONTEXT_HXX

#include <controller/SlsInsertionIndicatorHandler.hxx>

class Point;

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
    explicit DragAndDropContext (SlideSorter& rSlideSorter);
    ~DragAndDropContext() COVERITY_NOEXCEPT_FALSE;

    /** Call this method (for example as reaction to ESC key press) to avoid
        processing (ie moving or inserting) the substition when the called
        DragAndDropContext object is destroyed.
    */
    void Dispose();

    /** Move the substitution display by the distance the mouse has
        travelled since the last call to this method or to
        CreateSubstitution().  The given point becomes the new anchor.
    */
    void UpdatePosition (
        const Point& rMousePosition,
        const InsertionIndicatorHandler::Mode eMode,
        const bool bAllowAutoScroll);

    void SetTargetSlideSorter();

private:
    SlideSorter* mpTargetSlideSorter;
    sal_Int32 mnInsertionIndex;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
