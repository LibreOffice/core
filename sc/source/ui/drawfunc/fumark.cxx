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


#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "fumark.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "scmod.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "drawview.hxx"
#include "markdata.hxx"



/*************************************************************************
|*
|* Funktion zum Aufziehen eines Rechtecks
|*
\************************************************************************/

FuMarkRect::FuMarkRect(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pViewP, pDoc, rReq),
    bVisible(false),
    bStartDrag(false)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuMarkRect::~FuMarkRect()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuMarkRect::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    pWindow->CaptureMouse();
    pView->UnmarkAll();         // der Einheitlichkeit halber und wegen #50558#
    bStartDrag = true;

    aBeginPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );
    aZoomRect = Rectangle( aBeginPos, Size() );
    return true;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuMarkRect::MouseMove(const MouseEvent& rMEvt)
{
    if ( bStartDrag )
    {
        if ( bVisible )
            pViewShell->DrawMarkRect(aZoomRect);
        Point aPixPos= rMEvt.GetPosPixel();
        ForceScroll(aPixPos);

        Point aEndPos = pWindow->PixelToLogic(aPixPos);
        Rectangle aRect(aBeginPos, aEndPos);
        aZoomRect = aRect;
        aZoomRect.Justify();
        pViewShell->DrawMarkRect(aZoomRect);
        bVisible = true;
    }

    ForcePointer(&rMEvt);

    return bStartDrag;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuMarkRect::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if ( bVisible )
    {
        // Hide ZoomRect
        pViewShell->DrawMarkRect(aZoomRect);
        bVisible = false;
    }

    Size aZoomSizePixel = pWindow->LogicToPixel(aZoomRect).GetSize();

    sal_uInt16 nMinMove = pView->GetMinMoveDistancePixel();
    if ( aZoomSizePixel.Width() < nMinMove || aZoomSizePixel.Height() < nMinMove )
    {
        // Klick auf der Stelle

        aZoomRect.SetSize(Size());      // dann ganz leer
    }

    bStartDrag = false;
    pWindow->ReleaseMouse();

    pViewShell->GetViewData()->GetDispatcher().
        Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

        //  Daten an der View merken

    pViewShell->SetChartArea( aSourceRange, aZoomRect );

        //  Chart-Dialog starten:

//  sal_uInt16 nId  = ScChartDlgWrapper::GetChildWindowId();
//  SfxChildWindow* pWnd = pViewShell->GetViewFrame()->GetChildWindow( nId );
//  SC_MOD()->SetRefDialog( nId, pWnd ? sal_False : sal_True );

    return true;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

sal_uInt8 FuMarkRect::Command(const CommandEvent& rCEvt)
{
    if ( COMMAND_STARTDRAG == rCEvt.GetCommand() )
    {
        //  nicht anfangen, auf der Tabelle rumzudraggen,
        //  aber Maus-Status nicht zuruecksetzen
        return SC_CMD_IGNORE;
    }
    else
        return FuPoor::Command(rCEvt);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

bool FuMarkRect::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            //  beenden
            pViewShell->GetViewData()->GetDispatcher().
                Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            bReturn = true;
            break;
    }

    if (!bReturn)
    {
        bReturn = FuPoor::KeyInput(rKEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuMarkRect::Activate()
{
    FuPoor::Activate();

        //  Markierung merken, bevor evtl. Tabelle umgeschaltet wird

    ScViewData* pViewData = pViewShell->GetViewData();
    ScMarkData& rMark = pViewData->GetMarkData();

    if ( !rMark.IsMultiMarked() && !rMark.IsMarked() )
        pViewShell->MarkDataArea( true );

    pViewData->GetMultiArea( aSourceRange );        // Mehrfachselektion erlaubt

//  pViewShell->Unmark();

    ForcePointer(NULL);
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuMarkRect::Deactivate()
{
    FuPoor::Deactivate();

    if (bVisible)
    {
        // Hide ZoomRect
        pViewShell->DrawMarkRect(aZoomRect);
        bVisible = false;
        bStartDrag = false;
    }
}

/*************************************************************************
|*
|* Maus-Pointer umschalten
|*
\************************************************************************/

void FuMarkRect::ForcePointer(const MouseEvent* /* pMEvt */)
{
    pViewShell->SetActivePointer( Pointer( POINTER_CHART ) );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
