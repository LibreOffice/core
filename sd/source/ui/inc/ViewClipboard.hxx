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

#ifndef INCLUDED_SD_SOURCE_UI_INC_VIEWCLIPBOARD_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VIEWCLIPBOARD_HXX

#include <sal/types.h>

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
    virtual ~ViewClipboard();

    /** Handle the drop of a drag-and-drop action where the transferable
        contains a set of pages.
    */
    void HandlePageDrop (const SdTransferable& rTransferable);

protected:
    ::sd::View& mrView;

    /** Return the first master page of the given transferable.  When the
        bookmark list of the transferable contains at least one non-master
        page then NULL is returned.
    */
    static SdPage* GetFirstMasterPage (const SdTransferable& rTransferable);

    /** Assign the (first) master page of the given transferable to the
        (...) slide.
    */
    void AssignMasterPage (
        const SdTransferable& rTransferable,
        SdPage const * pMasterPage);

    /** Return an index of a page after which the pages of the transferable
        are to be inserted into the target document.
    */
    virtual sal_uInt16 DetermineInsertPosition (
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
    sal_uInt16 InsertSlides (
        const SdTransferable& rTransferable,
        sal_uInt16 nInsertPosition);
};

} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
