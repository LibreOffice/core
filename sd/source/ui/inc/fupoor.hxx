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

#ifndef SD_FU_POOR_HXX
#define SD_FU_POOR_HXX

#include <tools/rtti.hxx>
#include <vcl/timer.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/event.hxx>
#include <rtl/ref.hxx>

#include "helper/simplereferencecomponent.hxx"

class SdDrawDocument;
class SfxRequest;
class Dialog;
class SdrObject;

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
    static const int HITPIX = 2;                   // Hit-Toleranz in Pixel
    static const int DRGPIX = 2;                   // Drag MinMove in Pixel

    TYPEINFO();

    virtual void DoExecute( SfxRequest& rReq );

    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    DrawDocShell* GetDocSh() { return mpDocSh; }
    SdDrawDocument* GetDoc() { return mpDoc; }

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    // Mouse- & Key-Events; Returnwert=sal_True: Event wurde bearbeitet
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& );
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);

    // moved from inline to *.cxx
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual sal_Bool Command(const CommandEvent& rCEvt);
    virtual sal_Bool RequestHelp(const HelpEvent& rHEvt);
    virtual void Paint(const Rectangle&, ::sd::Window* );
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();        ///< activates the function
    virtual void Deactivate();      ///< deactivates the function

    void SetWindow(::sd::Window* pWin) { mpWindow = pWin; }

    virtual void SelectionHasChanged();

    sal_uInt16  GetSlotID() const { return( nSlotId ); }
    sal_uInt16  GetSlotValue() const { return( nSlotValue ); }

    void    SetNoScrollUntilInside(sal_Bool bNoScroll = sal_True)
            { bNoScrollUntilInside = bNoScroll; }

    void StartDelayToScrollTimer ();

    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

    // #i33136#
    /** Decide if the object to be created should be created
        orthogonal. Default implementation uses nSlotID
        to decide. May be overloaded to use other criterias
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
    virtual ~FuPoor (void);

    DECL_LINK( DelayHdl, void * );

    void ImpForceQuadratic(Rectangle& rRect);

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
           is set to either the minimal or maximal valid index, whitchever
           is nearer.
    */
    void SwitchLayer (sal_Int32 nOffset);

    ::sd::View* mpView;
    ViewShell* mpViewShell;
    ::sd::Window* mpWindow;
    DrawDocShell* mpDocSh;
    SdDrawDocument* mpDoc;

    sal_uInt16          nSlotId;
    sal_uInt16          nSlotValue;

    Dialog*             pDialog;

    Timer               aScrollTimer;           ///< for auto-scrolling
    DECL_LINK( ScrollHdl, void * );
    void ForceScroll(const Point& aPixPos);

    Timer               aDragTimer;             ///< for Drag&Drop
    DECL_LINK(DragHdl, void *);
    sal_Bool            bIsInDragMode;
    Point               aMDPos;                 ///< position of MouseButtonDown

    /// Flag to prevent auto-scrolling until one drags from outside into the window
    sal_Bool            bNoScrollUntilInside;

    /// timer to delay scrolling (~ 1 sec) when dragging out of the window
    Timer               aDelayToScrollTimer;
    sal_Bool            bScrollable;
    sal_Bool            bDelayActive;
    sal_Bool            bFirstMouseMove;

    /// member to hold state of the mouse buttons for creation of own MouseEvents (like in ScrollHdl)

private:
    sal_uInt16      mnCode;

};

typedef rtl::Reference< FuPoor > FunctionReference;

} // end of namespace sd

#endif      // _SD_FUPOOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
