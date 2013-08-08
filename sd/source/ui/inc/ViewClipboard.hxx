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



#ifndef SD_VIEW_CLIPBOARD_HXX
#define SD_VIEW_CLIPBOARD_HXX

#include <tools/solar.h>

class SdPage;
class SdTransferable;

namespace sd {

class View;


/** Handle clipboard related tasks for the draw view.
*/
class ViewClipboard
{
public:
    ViewClipboard (::sd::View& rView);
    virtual ~ViewClipboard (void);

    /** Handle the drop of a drag-and-drop action where the transferable
        contains a set of pages.
    */
    virtual void HandlePageDrop (const SdTransferable& rTransferable);

protected:
    ::sd::View& mrView;

    /** Return the first master page of the given transferable.  When the
        bookmark list of the transferable contains at least one non-master
        page then NULL is returned.
    */
    SdPage* GetFirstMasterPage (const SdTransferable& rTransferable);

    /** Assign the (first) master page of the given transferable to the
        (...) slide.
    */
    virtual void AssignMasterPage (
        const SdTransferable& rTransferable,
        SdPage* pMasterPage);

    /** Return an index of a page after which the pages of the transferable
        are to be inserted into the target document.
    */
    virtual sal_uInt32 DetermineInsertPosition (
        const SdTransferable& rTransferable);

    /** Insert the slides in the given transferable behind the last selected
        slide or, when the selection is empty, behind the last slide.
        @param rTransferable
            This transferable defines which pages to insert.
        @param nInsertPosition
            The pages of the transferable will be inserted behind the page
            with this index.
        @return
            Returns the number of inserted slides.
    */
    virtual sal_uInt32 InsertSlides (
        const SdTransferable& rTransferable,
        sal_uInt32 nInsertPosition);
};

} // end of namespace ::sd

#endif
