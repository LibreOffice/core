/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
