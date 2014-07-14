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

#include <editeng/outliner.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>

#include "fupoor.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "detfunc.hxx"
#include "document.hxx"
#include <vcl/svapp.hxx>
#include <svx/sdrhittesthelper.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPoor::FuPoor(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq) :
    pView(pViewP),
    pViewShell(pViewSh),
    pWindow(pWin),
    pDrDoc(pDoc),
    aSfxRequest(rReq),
    pDialog(NULL),
    bIsInDragMode(sal_False),
    // #95491# remember MouseButton state
    mnCode(0)
{
    aScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetTimeoutHdl( LINK(this, FuPoor, DragTimerHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();

    if (pDialog)
        delete pDialog;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();

    if (pDialog)
    {
        pDialog->Hide();
    }
}

/*************************************************************************
|*
|* Scrollen bei Erreichen des Fensterrandes; wird von
|* MouseMove aufgerufen
|*
\************************************************************************/

void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    Size aSize = pWindow->GetSizePixel();
    SCsCOL dx = 0;
    SCsROW dy = 0;

    if ( aPixPos.X() <= 0              ) dx = -1;
    if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
    if ( aPixPos.Y() <= 0              ) dy = -1;
    if ( aPixPos.Y() >= aSize.Height() ) dy =  1;

    ScViewData* pViewData = pViewShell->GetViewData();
    if ( pViewData->GetDocument()->IsNegativePage( pViewData->GetTabNo() ) )
        dx = -dx;

    ScSplitPos eWhich = pViewData->GetActivePart();
    if ( dx > 0 && pViewData->GetHSplitMode() == SC_SPLIT_FIX && WhichH(eWhich) == SC_SPLIT_LEFT )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
        dx = 0;
    }
    if ( dy > 0 && pViewData->GetVSplitMode() == SC_SPLIT_FIX && WhichV(eWhich) == SC_SPLIT_TOP )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
        dy = 0;
    }

    if ( dx != 0 || dy != 0 )
    {
        ScrollStart();                          // Scrollaktion in abgeleiteter Klasse
        pViewShell->ScrollLines(2*dx, 4*dy);
        ScrollEnd();
        aScrollTimer.Start();
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, ScrollHdl, Timer *, EMPTYARG )
{
    Point aPosPixel = pWindow->GetPointerPosPixel();

    // #95491# use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPosPixel, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

// #95491# moved from inline to *.cxx
sal_Bool FuPoor::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return sal_False;
}

// #95491# moved from inline to *.cxx
sal_Bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return sal_False;
}

/*************************************************************************
|*
|* String in Applikations-Statuszeile ausgeben
|*
\************************************************************************/

//  WriteStatus gibt's nicht mehr

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool FuPoor::KeyInput(const KeyEvent& /* rKEvt */)
{
    sal_Bool bReturn = sal_False;

    return(bReturn);
}

sal_uInt8 FuPoor::Command(const CommandEvent& rCEvt)
{
    if ( COMMAND_STARTDRAG == rCEvt.GetCommand() )
    {
        //!!! sollte Joe eigentlich machen:
        // nur, wenn im Outliner was selektiert ist, darf
        // Command sal_True zurueckliefern:

        OutlinerView* pOutView = pView->GetTextEditOutlinerView();

        if ( pOutView )
            return pOutView->HasSelection() ? pView->Command(rCEvt,pWindow) : SC_CMD_NONE;
        else
            return pView->Command(rCEvt,pWindow);
    }
    else
        return pView->Command(rCEvt,pWindow);
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuPoor::DoCut()
{
    if (pView)
    {
//!     pView->DoCut(pWindow);
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuPoor::DoCopy()
{
    if (pView)
    {
//!     pView->DoCopy(pWindow);
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuPoor::DoPaste()
{
    if (pView)
    {
//!     pView->DoPaste(pWindow);
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/

IMPL_LINK( FuPoor, DragTimerHdl, Timer *, EMPTYARG )
{
    //  ExecuteDrag (und das damit verbundene Reschedule) direkt aus dem Timer
    //  aufzurufen, bringt die VCL-Timer-Verwaltung durcheinander, wenn dabei
    //  (z.B. im Drop) wieder ein Timer gestartet wird (z.B. ComeBack-Timer der
    //  DrawView fuer Solid Handles / ModelHasChanged) - der neue Timer laeuft
    //  dann um die Dauer des Drag&Drop zu spaet ab.
    //  Darum Drag&Drop aus eigenem Event:

    Application::PostUserEvent( LINK( this, FuPoor, DragHdl ) );
    return 0;
}

IMPL_LINK( FuPoor, DragHdl, void *, EMPTYARG )
{
    SdrHdl* pHdl = pView->PickHandle(aMDPos);

    if ( pHdl==NULL && pView->IsMarkedObjHit(aMDPos) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = sal_True;

//      pView->BeginDrag(pWindow, aMDPos);
        pViewShell->GetScDrawView()->BeginDrag(pWindow, aMDPos);
    }
    return 0;
}

//  Detektiv-Linie

bool FuPoor::IsDetectiveHit( const basegfx::B2DPoint& rLogicPos )
{
    sal_Bool bFound = sal_False;

    if(pView->GetSdrPageView())
    {
        SdrObjListIter aIter( *pView->GetSdrPageView()->GetCurrentObjectList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
                const double fHitLog(basegfx::B2DVector(pWindow->GetInverseViewTransformation() * basegfx::B2DVector(pView->GetHitTolerancePixel(), 0.0)).getLength());

                if(SdrObjectPrimitiveHit(*pObject, rLogicPos, fHitLog, *pView, false, 0))
            {
                bFound = sal_True;
            }
        }

        pObject = aIter.Next();
    }
    }

    return bFound;
}

void FuPoor::StopDragTimer()
{
    if (aDragTimer.IsActive() )
        aDragTimer.Stop();
}

/*************************************************************************
|*
|* #98185# Create default drawing objects via keyboard
|*
\************************************************************************/

SdrObject* FuPoor::CreateDefaultObject(const sal_uInt16 /* nID */, const basegfx::B2DRange& /* rRange */)
{
    // empty base implementation
    return 0L;
}

void FuPoor::ImpForceQuadratic(basegfx::B2DRange& rRange)
{
    basegfx::B2DPoint aNewTopLeft(0.0, 0.0);

    if(rRange.getWidth() > rRange.getHeight())
    {
        aNewTopLeft = basegfx::B2DPoint(rRange.getMinX() + ((rRange.getWidth() - rRange.getHeight()) * 0.5), rRange.getMinY());
    }
    else
    {
        aNewTopLeft = basegfx::B2DPoint(rRange.getMinX(), rRange.getMinY() + ((rRange.getHeight() - rRange.getWidth()) * 0.5));
    }

    rRange = basegfx::B2DRange(aNewTopLeft, aNewTopLeft + rRange.getRange());
}

// #i33136#
bool FuPoor::doConstructOrthogonal() const
{
    return false;
}

// eof
