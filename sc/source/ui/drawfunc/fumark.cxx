/*************************************************************************
 *
 *  $RCSfile: fumark.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <svx/svdview.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "fumark.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "scmod.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"

//------------------------------------------------------------------

/*************************************************************************
|*
|* Funktion zum Aufziehen eines Rechtecks
|*
\************************************************************************/

FuMarkRect::FuMarkRect(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
               SdrModel* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
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

    SFX_DISPATCHER().Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

        //  Daten an der View merken

    pViewShell->SetChartArea( aSourceRange, aZoomRect );

        //  Chart-Dialog starten:

    USHORT nId  = ScChartDlgWrapper::GetChildWindowId();
    SfxChildWindow* pWnd = pViewShell->GetViewFrame()->GetChildWindow( nId );
    SC_MOD()->SetRefDialog( nId, pWnd ? FALSE : TRUE );

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
        //  #29877# nicht anfangen, auf der Tabelle rumzudraggen,
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
            SFX_DISPATCHER().Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
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

void FuMarkRect::ForcePointer(const MouseEvent* pMEvt)
{
#ifdef VCL
    pViewShell->SetActivePointer( Pointer( POINTER_CHART ) );
#else
    pViewShell->SetActivePointer( ScResId( RID_SCPTR_CHART ) );
#endif
}




