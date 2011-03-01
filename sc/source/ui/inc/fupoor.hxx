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

#ifndef _SC_FUPOOR_HXX
#define _SC_FUPOOR_HXX

#include <vcl/event.hxx>
#include <vcl/timer.hxx>
#include <sfx2/request.hxx>

class ScDrawView;
class ScTabViewShell;
class Window;
class SdrModel;
class Dialog;

// Create default drawing objects via keyboard
class SdrObject;

//  Return-Werte fuer Command
#define SC_CMD_NONE     0
#define SC_CMD_USED     1
#define SC_CMD_IGNORE   2

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuPoor
{
protected:
    ScDrawView*     pView;
    ScTabViewShell* pViewShell;
    Window*         pWindow;
    SdrModel*       pDrDoc;

    SfxRequest      aSfxRequest;
    Dialog*         pDialog;

    Timer           aScrollTimer;           // fuer Autoscrolling
    DECL_LINK( ScrollHdl, Timer * );
    void ForceScroll(const Point& aPixPos);

    Timer           aDragTimer;             // fuer Drag&Drop
    DECL_LINK( DragTimerHdl, Timer * );
    DECL_LINK( DragHdl, void * );
    BOOL            bIsInDragMode;
    Point           aMDPos;                 // Position von MouseButtonDown

    // member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)
private:
    sal_uInt16      mnCode;

public:
    FuPoor(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuPoor();

    // see member
    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    virtual void Paint(const Rectangle&, Window*) {}

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent&) { return FALSE; }

    // moved from inline to *.cxx
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt); // { return FALSE; }

    // moved from inline to *.cxx
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt); // { return FALSE; }

    virtual BYTE Command(const CommandEvent& rCEvt);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

    virtual void ScrollStart() {}   // diese Funktionen werden von
    virtual void ScrollEnd() {}     // ForceScroll aufgerufen

    void SetWindow(Window* pWin) { pWindow = pWin; }

    USHORT GetSlotID() const { return( aSfxRequest.GetSlot() ); }

    BOOL    IsDetectiveHit( const Point& rLogicPos );

    void    StopDragTimer();

    // Create default drawing objects via keyboard
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

protected:
    void ImpForceQuadratic(Rectangle& rRect);

public:
    // #i33136#
    virtual bool doConstructOrthogonal() const;
};



#endif      // _SD_FUPOOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
