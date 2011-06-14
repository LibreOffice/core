/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    virtual sal_uInt16 InsertSlides (
        const SdTransferable& rTransferable,
        sal_uInt16 nInsertPosition);
};

} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
