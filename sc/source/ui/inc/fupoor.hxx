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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FUPOOR_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FUPOOR_HXX

#include <vcl/timer.hxx>
#include <sfx2/request.hxx>
#include <svx/svdobj.hxx>
#include <vcl/window.hxx>

class ScDrawView;
class ScTabViewShell;
class SdrModel;
class CommandEvent;
class KeyEvent;
class MouseEvent;

//  Return values for command
#define SC_CMD_NONE     0
#define SC_CMD_USED     1

/** Base class for all functions */
class FuPoor
{
protected:
    ScDrawView*     pView;
    ScTabViewShell& rViewShell;
    VclPtr<vcl::Window>     pWindow;
    SdrModel*       pDrDoc;

    SfxRequest const      aSfxRequest;

    Timer           aScrollTimer;           // for Autoscrolling
    DECL_LINK( ScrollHdl, Timer *, void );
    void ForceScroll(const Point& aPixPos);

    Timer           aDragTimer;             // for Drag&Drop
    DECL_LINK( DragTimerHdl, Timer *, void );
    DECL_LINK( DragHdl, void *, void );
    bool            bIsInDragMode;
    Point           aMDPos;                 // Position of MouseButtonDown

    // member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)
private:
    sal_uInt16      mnCode;

public:
    FuPoor(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, const SfxRequest& rReq);
    virtual ~FuPoor();

    // see member
    void SetMouseButtonCode(sal_uInt16 nNew) { if(nNew != mnCode) mnCode = nNew; }
    sal_uInt16 GetMouseButtonCode() const { return mnCode; }

    // Mouse- & Key-Events; return value=TRUE: Event was processed
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent&) { return false; }

    // moved from inline to *.cxx
    virtual bool MouseButtonUp(const MouseEvent& rMEvt); // { return FALSE; }

    // moved from inline to *.cxx
    virtual bool MouseButtonDown(const MouseEvent& rMEvt); // { return FALSE; }

    sal_uInt8 Command(const CommandEvent& rCEvt);

    virtual void Activate();
    virtual void Deactivate();

    void SetWindow(vcl::Window* pWin) { pWindow = pWin; }

    sal_uInt16 GetSlotID() const { return aSfxRequest.GetSlot(); }

    bool    IsDetectiveHit( const Point& rLogicPos );

    void    StopDragTimer();

    // Create default drawing objects via keyboard
    virtual SdrObjectUniquePtr CreateDefaultObject(const sal_uInt16 nID, const tools::Rectangle& rRectangle);

protected:
    static void ImpForceQuadratic(tools::Rectangle& rRect);

public:
    // #i33136#
    virtual bool doConstructOrthogonal() const;
};

#endif      // _SD_FUPOOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
