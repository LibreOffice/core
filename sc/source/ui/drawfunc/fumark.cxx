/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <basegfx/range/b2drange.hxx>

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
    bVisible(sal_False),
    bStartDrag(sal_False)
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

sal_Bool FuMarkRect::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    pWindow->CaptureMouse();
    pView->UnmarkAll();         // der Einheitlichkeit halber und wegen #50558#
    bStartDrag = sal_True;

    aBeginPos = pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    aZoomRange = basegfx::B2DRange(aBeginPos);
    return sal_True;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuMarkRect::MouseMove(const MouseEvent& rMEvt)
{
    if ( bStartDrag )
    {
        if ( bVisible )
        {
            pViewShell->DrawMarkRange(aZoomRange);
        }

        const basegfx::B2DPoint aPixPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        ForceScroll(rMEvt.GetPosPixel());

        const basegfx::B2DPoint aEndPos(pWindow->GetInverseViewTransformation() * aPixPos);
        aZoomRange = basegfx::B2DRange(aBeginPos, aEndPos);
        pViewShell->DrawMarkRange(aZoomRange);
        bVisible = sal_True;
    }

    ForcePointer(&rMEvt);

    return bStartDrag;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuMarkRect::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if ( bVisible )
    {
        // Hide ZoomRect
        pViewShell->DrawMarkRange(aZoomRange);
        bVisible = sal_False;
    }

    const basegfx::B2DVector aZoomSizePixel(pWindow->GetViewTransformation() * aZoomRange.getRange());
    const double fMinMove(pView->GetMinMoveDistancePixel());

    if ( aZoomSizePixel.getX() < fMinMove || aZoomSizePixel.getY() < fMinMove )
    {
        // Klick auf der Stelle
        aZoomRange = basegfx::B2DRange(aZoomRange.getMinimum());
    }

    bStartDrag = sal_False;
    pWindow->ReleaseMouse();

    pViewShell->GetViewData()->GetDispatcher().
        Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

        //  Daten an der View merken

    pViewShell->SetChartArea(aSourceRange, aZoomRange);

        //  Chart-Dialog starten:

//  sal_uInt16 nId  = ScChartDlgWrapper::GetChildWindowId();
//  SfxChildWindow* pWnd = pViewShell->GetViewFrame()->GetChildWindow( nId );
//  SC_MOD()->SetRefDialog( nId, pWnd ? sal_False : sal_True );

    return sal_True;
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool FuMarkRect::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            //  beenden
            pViewShell->GetViewData()->GetDispatcher().
                Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            bReturn = sal_True;
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
        pViewShell->MarkDataArea( sal_True );

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
        pViewShell->DrawMarkRange(aZoomRange);
        bVisible = sal_False;
        bStartDrag = sal_False;
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




