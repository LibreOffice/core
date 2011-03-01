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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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

//------------------------------------------------------------------

/*************************************************************************
|*
|* Funktion zum Aufziehen eines Rechtecks
|*
\************************************************************************/

FuMarkRect::FuMarkRect(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pViewP, pDoc, rReq),
    bVisible(FALSE),
    bStartDrag(FALSE)
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

BOOL FuMarkRect::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    pWindow->CaptureMouse();
    pView->UnmarkAll();         // der Einheitlichkeit halber und wegen #50558#
    bStartDrag = TRUE;

    aBeginPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );
    aZoomRect = Rectangle( aBeginPos, Size() );
    return TRUE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuMarkRect::MouseMove(const MouseEvent& rMEvt)
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
        bVisible = TRUE;
    }

    ForcePointer(&rMEvt);

    return bStartDrag;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuMarkRect::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if ( bVisible )
    {
        // Hide ZoomRect
        pViewShell->DrawMarkRect(aZoomRect);
        bVisible = FALSE;
    }

    Size aZoomSizePixel = pWindow->LogicToPixel(aZoomRect).GetSize();

    USHORT nMinMove = pView->GetMinMoveDistancePixel();
    if ( aZoomSizePixel.Width() < nMinMove || aZoomSizePixel.Height() < nMinMove )
    {
        // Klick auf der Stelle

        aZoomRect.SetSize(Size());      // dann ganz leer
    }

    bStartDrag = FALSE;
    pWindow->ReleaseMouse();

    pViewShell->GetViewData()->GetDispatcher().
        Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

        //  Daten an der View merken

    pViewShell->SetChartArea( aSourceRange, aZoomRect );

        //  Chart-Dialog starten:

//  USHORT nId  = ScChartDlgWrapper::GetChildWindowId();
//  SfxChildWindow* pWnd = pViewShell->GetViewFrame()->GetChildWindow( nId );
//  SC_MOD()->SetRefDialog( nId, pWnd ? FALSE : TRUE );

    return TRUE;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

BYTE FuMarkRect::Command(const CommandEvent& rCEvt)
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuMarkRect::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            //  beenden
            pViewShell->GetViewData()->GetDispatcher().
                Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            bReturn = TRUE;
            break;
    }

    if (!bReturn)
    {
        bReturn = FuPoor::KeyInput(rKEvt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Vor dem Scrollen Selektionsdarstellung ausblenden
|*
\************************************************************************/

void FuMarkRect::ScrollStart()
{
}

/*************************************************************************
|*
|* Nach dem Scrollen Selektionsdarstellung wieder anzeigen
|*
\************************************************************************/

void FuMarkRect::ScrollEnd()
{
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
        pViewShell->MarkDataArea( TRUE );

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
        bVisible = FALSE;
        bStartDrag = FALSE;
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
