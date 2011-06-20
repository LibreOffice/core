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

#ifndef SD_SLIDESORTER_CLIPBOARD
#define SD_SLIDESORTER_CLIPBOARD

#include "ViewClipboard.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include <sal/types.h>
#include <tools/solar.h>
#include <svx/svdpage.hxx>

#include <set>

class SfxRequest;
class Window;

struct AcceptDropEvent;
class DropTargetHelper;
struct ExecuteDropEvent;
class Point;
class SdPage;
class Window;

namespace sd {
class Window;
}

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

class Clipboard
    : public ViewClipboard
{
public:
    Clipboard (SlideSorter& rSlideSorter);
    ~Clipboard (void);

    void HandleSlotCall (SfxRequest& rRequest);

    void DoCut (::Window* pWindow = 0);
    void DoCopy (::Window* pWindow = 0);
    void DoPaste (::Window* pWindow = 0);
    void DoDelete (::Window* pWindow = 0);

    void StartDrag (
        const Point& rDragPt,
        ::Window* pWindow );

    void DragFinished (
        sal_Int8 nDropAction);

    sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND );

    sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        sal_uInt16 nPage = SDRPAGE_NOTFOUND,
        sal_uInt16 nLayer = SDRPAGE_NOTFOUND);

    void Abort (void);

protected:
    virtual sal_uInt16 DetermineInsertPosition (
        const SdTransferable& rTransferable);

    virtual sal_uInt16 InsertSlides (
        const SdTransferable& rTransferable,
        sal_uInt16 nInsertPosition);

private:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    typedef ::std::vector<SdPage*> PageList;
    /** Remember the pages that are dragged to another document or to
        another place in the same document so that they can be removed after
        a move operation.
    */
    PageList maPagesToRemove;

    /** Remember the pages inserted from another document or another place
        in the same document so that they can be selected after the
        drag-and-drop operation is completed.
    */
    PageList maPagesToSelect;

    /** When pages are moved or copied then the selection of the slide
        sorter has to be updated.  This flag is used to remember whether the
        selection has to be updated or can stay as it is (sal_False).
    */
    bool mbUpdateSelectionPending;

    /** Used when a drop is executed to combine all undo actions into one.
        Typically created in ExecuteDrop() and released in DragFinish().
    */
    class UndoContext;
    ::boost::scoped_ptr<UndoContext> mpUndoContext;

    ::boost::scoped_ptr<SelectionObserver::Context> mpSelectionObserverContext;
    sal_uLong mnDragFinishedUserEventId;

    void CreateSlideTransferable (
        ::Window* pWindow,
        bool bDrag);

    /** Select the pages stored in the maPagesToSelect member.  The list in
        the member is cleared afterwards.
    */
    void SelectPages (void);

    /** Determine the position of where to insert the pages in the current
        transferable of the sd module.
        @param pWindow
            This window is used as parent for dialogs that have to be shown
            to the user.
        @return
            The index in the range [0,n] (both inclusive) with n the number
            of pages is returned.
    */
    sal_Int32 GetInsertionPosition (::Window* pWindow);

    /** Paste the pages of the transferable of the sd module at the given
        position.
        @param nInsertPosition
            The position at which to insert the pages.  The valid range is
            [0,n] (both inclusive) with n the number of pages in the
            document.
        @return
            The number of inserted pages is returned.
    */
    sal_Int32 PasteTransferable (sal_Int32 nInsertPosition);

    /** Select a range of pages of the model.  Typicall usage is the
        selection of newly inserted pages.
        @param nFirstIndex
            The index of the first page to select.
        @param nPageCount
            The number of pages to select.
    */
    void SelectPageRange (sal_Int32 nFirstIndex, sal_Int32 nPageCount);

    /** Return <TRUE/> when the current transferable in the current state of
        the slidesorter is acceptable to be pasted.  For this the
        transferable has to
        a) exist,
        b) contain one or more regular draw pages, no master pages.
        When master pages are involved, either in the transferable or in the
        slide sorter (by it displaying master pages) the drop of the
        transferable is not accepted.  The reason is the missing
        implementation of proper handling master pages copy-and-paste.
    */
    enum DropType { DT_PAGE, DT_SHAPE, DT_NONE };
    DropType IsDropAccepted (void) const;

    /** This method contains the code for AcceptDrop() and ExecuteDrop() shapes.
        There are only minor differences for the two cases at this level.
        @param eCommand
            This parameter specifies whether to do a AcceptDrop() or
            ExecuteDrop().
        @param rPosition
            Since the event is given as void pointer we can not take the
            mouse position from it.  The caller has to supply it in this
            parameter.
        @param pDropEvent
            Event though the AcceptDropEvent and ExecuteDropEvent are very
            similar they do not have a common base class.  Because of that
            we have to use a void* to pase these structs.
        @param nPage
            When the page number is given as 0xffff then it is replaced by
            the number of the page at the mouse position.  If the mouse is
            not over a page then neither AcceptDrop() nor ExecuteDrop() are
            executed.
    */
    enum DropCommand { DC_ACCEPT, DC_EXECUTE };
    sal_Int8 ExecuteOrAcceptShapeDrop (
        DropCommand eCommand,
        const Point& rPosition,
        const void* pDropEvent ,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        sal_uInt16 nLayer);

    /** Asynchronous part of DragFinished.  The argument is the sal_Int8
        nDropAction, disguised as void*.
    */
    DECL_LINK(ProcessDragFinished, void*);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
