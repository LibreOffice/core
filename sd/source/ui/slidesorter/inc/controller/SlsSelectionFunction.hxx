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

#ifndef SD_SLIDESORTER_SELECTION_FUNCTION_HXX
#define SD_SLIDESORTER_SELECTION_FUNCTION_HXX

#include "controller/SlsSlideFunction.hxx"
#include "model/SlsSharedPageDescriptor.hxx"
#include <memory>

class SdSlideViewShell;
class SdWindow;
class SdSlideView;
class SdDrawDocument;
class Sound;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

class SelectionFunction
    : public SlideFunction
{
public:
    TYPEINFO();

    static FunctionReference Create( SlideSorter& rSlideSorter, SfxRequest& rRequest );

    // Mouse- & Key-Events
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);
    virtual void Paint(const Rectangle&, ::sd::Window* );

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void ScrollStart();
    virtual void ScrollEnd();

    /// Forward to the clipboard manager.
    virtual void DoCut (void);

    /// Forward to the clipboard manager.
    virtual void DoCopy (void);

    /// Forward to the clipboard manager.
    virtual void DoPaste (void);

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns
            true if a active function was aborted
    */
    virtual bool cancel();

protected:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    SelectionFunction (
        SlideSorter& rSlideSorter,
        SfxRequest& rRequest);

    virtual ~SelectionFunction();

private:
    class SubstitutionHandler;
    class EventDescriptor;

    /// Set in MouseButtonDown this flag indicates that a page has been hit.
    bool mbPageHit;

    /// The rectangle of the mouse drag selection.
    Rectangle maDragSelectionRectangle;
    bool mbDragSelection;

    /// Box of the insert marker in model coordinates.
    Rectangle maInsertionMarkerBox;

    /** We use this flag to filter out the cases where MouseMotion() is called
        with a pressed mouse button but without a prior MouseButtonDown()
        call.  This is an indication that the mouse button was pressed over
        another control, e.g. the view tab bar, and that a re-layout of the
        controls moved the slide sorter under the mouse.
    */
    bool mbProcessingMouseButtonDown;

    ::std::auto_ptr<SubstitutionHandler> mpSubstitutionHandler;

    DECL_LINK( DragSlideHdl, Timer* );
    void StartDrag (void);

    /** Set the selection to exactly the specified page and also set it as
        the current page.
    */
    void SetCurrentPage (const model::SharedPageDescriptor& rpDescriptor);

    /** When the view on which this selection function is working is the
        main view then the view is switched to the regular editing view.
    */
    void SwitchView (const model::SharedPageDescriptor& rpDescriptor);

    /** Make the slide nOffset slides away of the current one the new
        current slide.  When the new index is outside the range of valid
        page numbers it is clipped to that range.
        @param nOffset
            When nOffset is negative then go back.  When nOffset if positive go
            forward.  When it is zero then ignore the call.
    */
    void GotoNextPage (int nOffset);

    void ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent);
    void ProcessKeyEvent (const KeyEvent& rEvent);

    // What follows are a couple of helper methods that are used by
    // ProcessMouseEvent().

    /// Select the specified page and set the selection anchor.
    void SelectHitPage (const model::SharedPageDescriptor& rpDescriptor);
    /// Deselect the specified page.
    void DeselectHitPage (const model::SharedPageDescriptor& rpDescriptor);
    /// Deselect all pages.
    void DeselectAllPages (void);

    /** for a possibly following mouse motion by starting the drag timer
        that after a short time of pressed but un-moved mouse starts a drag
        operation.
    */
    void PrepareMouseMotion (const Point& aMouseModelPosition);

    /** Select all pages between and including the selection anchor and the
        specified page.
    */
    void RangeSelect (const model::SharedPageDescriptor& rpDescriptor);

    /** Start a rectangle selection at the given position.
    */
    void StartRectangleSelection (const Point& aMouseModelPosition);

    /** Update the rectangle selection so that the given position becomes
        the new second point of the selection rectangle.
    */
    void UpdateRectangleSelection (const Point& aMouseModelPosition);

    /** Select all pages that lie completly in the selection rectangle.
    */
    void ProcessRectangleSelection (bool bToggleSelection);

    /** Compute a numerical code that describes a mouse event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeMouseEvent (
        const EventDescriptor& rDescriptor,
        const MouseEvent& rEvent) const;

    /** Compute a numerical code that describes a key event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeKeyEvent (
        const EventDescriptor& rDescriptor,
        const KeyEvent& rEvent) const;

    void EventPreprocessing (const EventDescriptor& rEvent);
    bool EventProcessing (const EventDescriptor& rEvent);
    void EventPostprocessing (const EventDescriptor& rEvent);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
