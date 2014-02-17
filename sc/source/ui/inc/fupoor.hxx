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

//  Return values for command
#define SC_CMD_NONE     0
#define SC_CMD_USED     1
#define SC_CMD_IGNORE   2

/** Base class for all functions */
class FuPoor
{
protected:
    ScDrawView*     pView;
    ScTabViewShell* pViewShell;
    Window*         pWindow;
    SdrModel*       pDrDoc;

    SfxRequest      aSfxRequest;
    Dialog*         pDialog;

    Timer           aScrollTimer;           // for Autoscrolling
    DECL_LINK( ScrollHdl, void * );
    void ForceScroll(const Point& aPixPos);

    Timer           aDragTimer;             // for Drag&Drop
    DECL_LINK( DragTimerHdl, void * );
    DECL_LINK( DragHdl, void * );
    sal_Bool            bIsInDragMode;
    Point           aMDPos;                 // Position of MouseButtonDown

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

    // Mouse- & Key-Events; return value=TRUE: Event was processed
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent&) { return false; }

    // moved from inline to *.cxx
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt); // { return FALSE; }

    // moved from inline to *.cxx
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt); // { return FALSE; }

    virtual sal_uInt8 Command(const CommandEvent& rCEvt);

    virtual void Activate();
    virtual void Deactivate();

    void SetWindow(Window* pWin) { pWindow = pWin; }

    sal_uInt16 GetSlotID() const { return( aSfxRequest.GetSlot() ); }

    sal_Bool    IsDetectiveHit( const Point& rLogicPos );

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
