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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSCLIPBOARD_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSCLIPBOARD_HXX

#include <memory>
#include <ViewClipboard.hxx>
#include <controller/SlsSelectionObserver.hxx>
#include <sdxfer.hxx>

#include <sal/types.h>
#include <o3tl/deleter.hxx>
#include <svx/svdtypes.hxx>

#include <sddllapi.h>

class SfxRequest;
struct AcceptDropEvent;
class DropTargetHelper;
struct ExecuteDropEvent;
struct ImplSVEvent;
class Point;
class SdPage;
namespace vcl { class Window; }

namespace sd {
class Window;
}

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

class Clipboard
    : public ViewClipboard
{
public:
    Clipboard (SlideSorter& rSlideSorter);
    virtual ~Clipboard() override;

    /** Create a slide sorter transferable from the given sd
        transferable.  The returned transferable is set up with all
        information necessary so that it can be dropped on a slide sorter.
    */
    static std::shared_ptr<SdTransferable::UserData> CreateTransferableUserData (SdTransferable* pTransferable);

    void HandleSlotCall (SfxRequest& rRequest);

    void DoCut ();
    // Exported for unit test
    SD_DLLPUBLIC void DoCopy();
    // Exported for unit test
    SD_DLLPUBLIC void DoPaste();
    void DoDelete ();

    void StartDrag (
        const Point& rDragPt,
        vcl::Window* pWindow );

    void DragFinished (
        sal_Int8 nDropAction);

    sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        SdrLayerID nLayer );

    sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow,
        sal_uInt16 nPage,
        SdrLayerID nLayer );

    void Abort();

protected:
    virtual sal_uInt16 DetermineInsertPosition (
        const SdTransferable& rTransferable) override;

private:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    typedef ::std::vector<SdPage*> PageList;
    /** Remember the pages that are dragged to another document or to
        another place in the same document so that they can be removed after
        a move operation.
    */
    PageList maPagesToRemove;

    /** Used when a drop is executed to combine all undo actions into one.
        Typically created in ExecuteDrop() and released in DragFinish().
    */
    class UndoContext;
    std::unique_ptr<UndoContext> mxUndoContext;

    std::unique_ptr<SelectionObserver::Context, o3tl::default_delete<SelectionObserver::Context>> mxSelectionObserverContext;
    ImplSVEvent * mnDragFinishedUserEventId;

    void CreateSlideTransferable (
        vcl::Window* pWindow,
        bool bDrag);

    /** Determine the position of where to insert the pages in the current
        transferable of the sd module.
        @return
            The index in the range [0,n] (both inclusive) with n the number
            of pages is returned.
    */
    sal_Int32 GetInsertionPosition ();

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

    /** Select a range of pages of the model.  Typically usage is the
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
    enum DropType { DT_PAGE, DT_PAGE_FROM_NAVIGATOR, DT_SHAPE, DT_NONE };
    DropType IsDropAccepted() const;

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
            we have to use a void* to pass these structs.
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
        SdrLayerID nLayer);

    /** Return whether the insertion defined by the transferable is
        trivial, ie would not change either source nor target document.
    */
    bool IsInsertionTrivial (
        SdTransferable const * pTransferable,
        const sal_Int8 nDndAction) const;

    /** Asynchronous part of DragFinished.  The argument is the sal_Int8
        nDropAction, disguised as void*.
    */
    DECL_LINK(ProcessDragFinished, void*, void);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
