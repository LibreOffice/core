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

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

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

    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& );
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);

    // moved from inline to *.cxx
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual BOOL Command(const CommandEvent& rCEvt);
    virtual BOOL RequestHelp(const HelpEvent& rHEvt);
    virtual void Paint(const Rectangle&, ::sd::Window* );
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

    virtual void ScrollStart() {}   // diese Funktionen werden von
    virtual void ScrollEnd() {}     // ForceScroll aufgerufen

    void SetWindow(::sd::Window* pWin) { mpWindow = pWin; }

    virtual void SelectionHasChanged();

    USHORT  GetSlotID() const { return( nSlotId ); }
    USHORT  GetSlotValue() const { return( nSlotValue ); }

    void    SetNoScrollUntilInside(BOOL bNoScroll = TRUE)
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

    DECL_LINK( DelayHdl, Timer * );

    void ImpForceQuadratic(Rectangle& rRect);

    /** Switch to another layer.  The layer to switch to is specified by an
        offset relative to the active layer.  With respect to the layer bar
        control at the lower left of the document window positive values
        move to the right and negative values move to the left.

        <p>Switching the layer is independant of the view's layer mode.  The
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

    USHORT          nSlotId;
    USHORT          nSlotValue;

    Dialog*         pDialog;

    Timer           aScrollTimer;           // fuer Autoscrolling
    DECL_LINK( ScrollHdl, Timer * );
    void ForceScroll(const Point& aPixPos);

    Timer           aDragTimer;             // fuer Drag&Drop
    DECL_LINK( DragHdl, Timer * );
    BOOL            bIsInDragMode;
    Point           aMDPos;                 // Position von MouseButtonDown

    // Flag, um AutoScrolling zu verhindern, bis von ausserhalb in das
    // Fenster hinein gedragt wurde
    BOOL            bNoScrollUntilInside;

    // Timer um das scrolling zu verzoegern, wenn aus dem fenster
    // herausgedraggt wird (ca. 1 sec.)
    Timer           aDelayToScrollTimer;    // fuer Verzoegerung bis scroll
    BOOL            bScrollable;
    BOOL            bDelayActive;
    BOOL            bFirstMouseMove;

    // member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)

private:
    sal_uInt16      mnCode;

};

typedef rtl::Reference< FuPoor > FunctionReference;

} // end of namespace sd

#endif      // _SD_FUPOOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
