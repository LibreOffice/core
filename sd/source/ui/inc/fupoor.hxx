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

#pragma once

#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>
#include <svx/svdobj.hxx>

#include <helper/simplereferencecomponent.hxx>

class SdDrawDocument;
class SfxRequest;
class CommandEvent;
class HelpEvent;
class KeyEvent;
class MouseEvent;

namespace sd {

class DrawDocShell;
class View;
class ViewShell;
class Window;

/**
 * Base class for all functions
 */
class FuPoor : public SimpleReferenceComponent
{
public:
    static const int HITPIX = 2;                   // hit tolerance in pixel
    static const int HITLOG = 53;                  // hit tolerance in mm100
    static const int DRGPIX = 2;                   // minimal drag move in pixel
    static const int DRGLOG = 53;                  // minimal drag move in mm100


    virtual void DoExecute( SfxRequest& rReq );

    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    DrawDocShell* GetDocSh() { return mpDocSh; }

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();
    virtual void DoPasteUnformatted();

    // mouse & key events; return value = sal_True: event has been handled
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& );
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);

    // moved from inline to *.cxx
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual bool Command(const CommandEvent& rCEvt);
    virtual bool RequestHelp(const HelpEvent& rHEvt);
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();        ///< activates the function
    virtual void Deactivate();      ///< deactivates the function

    void SetWindow(::sd::Window* pWin);

    virtual void SelectionHasChanged();

    sal_uInt16  GetSlotID() const { return nSlotId; }

    void StartDelayToScrollTimer ();

    virtual SdrObjectUniquePtr CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle);

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if an active function was aborted
    */
    virtual bool cancel();

    // #i33136#
    /** Decide if the object to be created should be created
        orthogonal. Default implementation uses nSlotID
        to decide. May be overridden to use other criteria
        for this decision

        @returns true if the to be created object should be orthogonal.
    */
    virtual bool doConstructOrthogonal() const;

protected:
    /**
        @param pViewSh
            May be NULL.
    */
    FuPoor (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuPoor() override;

    DECL_LINK( DelayHdl, Timer *, void );

    static void ImpForceQuadratic(::tools::Rectangle& rRect);

    /** Switch to another layer.  The layer to switch to is specified by an
        offset relative to the active layer.  With respect to the layer bar
        control at the lower left of the document window positive values
        move to the right and negative values move to the left.

        <p>Switching the layer is independent of the view's layer mode.  The
        layers are switched even when the layer mode is turned off and the
        layer control is not visible.</p>
        @param nOffset
           If the offset is positive skip that many layers in selecting the
           next layer.  If it is negative then select a previous one.  An
           offset or zero does not change the current layer.  If the
           resulting index lies outside the valid range of indices then it
           is set to either the minimal or maximal valid index, whichever
           is nearer.
    */
    void SwitchLayer (sal_Int32 nOffset);

    ::sd::View* mpView;
    ViewShell* mpViewShell;
    VclPtr< ::sd::Window> mpWindow;
    DrawDocShell* mpDocSh;
    SdDrawDocument* mpDoc;

    sal_uInt16          nSlotId;

    Timer               aScrollTimer;           ///< for auto-scrolling
    DECL_LINK( ScrollHdl, Timer *, void );
    void ForceScroll(const Point& aPixPos);

    Timer               aDragTimer;             ///< for Drag&Drop
    DECL_LINK(DragHdl, Timer *, void);
    bool            bIsInDragMode;
    Point               aMDPos;                 ///< position of MouseButtonDown

    /// Flag to prevent auto-scrolling until one drags from outside into the window
    bool            bNoScrollUntilInside;

    /// timer to delay scrolling (~ 1 sec) when dragging out of the window
    Timer               aDelayToScrollTimer;
    bool            bScrollable;
    bool            bDelayActive;
    bool            bFirstMouseMove;

    /// member to hold state of the mouse buttons for creation of own MouseEvents (like in ScrollHdl)

private:
    sal_uInt16      mnCode;

};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
