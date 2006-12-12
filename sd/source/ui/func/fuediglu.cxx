/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuediglu.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:17:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "fuediglu.hxx"

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#include <svx/dialogs.hrc>
#ifndef _SVDGLUE_HXX //autogen
#include <svx/svdglue.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif


#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_TOOL_BAR_MANAGER_HXX
#include "ToolBarManager.hxx"
#endif

namespace sd {

TYPEINIT1( FuEditGluePoints, FuDraw );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuEditGluePoints::FuEditGluePoints (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuEditGluePoints::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuEditGluePoints* pFunc;
    FunctionReference xFunc( pFunc = new FuEditGluePoints( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent( bPermanent );
    return xFunc;
}

void FuEditGluePoints::DoExecute( SfxRequest& rReq )
{
    FuDraw::DoExecute( rReq );
    mpView->SetInsGluePointMode(FALSE);
    mpViewShell->GetViewShellBase().GetToolBarManager().AddToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msGluePointsToolBar);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuEditGluePoints::~FuEditGluePoints()
{
    mpView->BrkAction();
    mpView->UnmarkAllGluePoints();
    mpView->SetInsGluePointMode(FALSE);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuEditGluePoints::MouseButtonDown(const MouseEvent& rMEvt)
{
    mpView->SetActualWin( mpWindow );

    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    if (mpView->IsAction())
    {
        if (rMEvt.IsRight())
            mpView->BckAction();

        return TRUE;
    }

    if (rMEvt.IsLeft())
    {
        bReturn = TRUE;
        USHORT nHitLog = USHORT ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        USHORT nDrgLog = USHORT ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        mpWindow->CaptureMouse();

        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_HANDLE)
        {
            /******************************************************************
            * Handle draggen
            ******************************************************************/
            SdrHdl* pHdl = aVEvt.pHdl;

            if (mpView->IsGluePointMarked(aVEvt.pObj, aVEvt.nGlueId) && rMEvt.IsShift())
            {
                mpView->UnmarkGluePoint(aVEvt.pObj, aVEvt.nGlueId, aVEvt.pPV);
                pHdl = NULL;
            }

            if (pHdl)
            {
                // Handle draggen
                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
            }
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && mpView->IsInsGluePointMode())
        {
            /******************************************************************
            * Klebepunkt einfuegen
            ******************************************************************/
            mpView->BegInsGluePoint(aMDPos);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
                mpView->UnmarkAllGluePoints();

            mpView->BegMarkGluePoints(aMDPos);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Objekt verschieben
            ******************************************************************/
            mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, NULL, nDrgLog);
        }
        else if (eHit == SDRHIT_GLUEPOINT)
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
                mpView->UnmarkAllGluePoints();

            mpView->MarkGluePoint(aVEvt.pObj, aVEvt.nGlueId, aVEvt.pPV);
            SdrHdl* pHdl = mpView->GetGluePointHdl(aVEvt.pObj, aVEvt.nGlueId);

            if (pHdl)
            {
                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            }
        }
        else
        {
            /******************************************************************
            * Objekt selektieren oder draggen
            ******************************************************************/
            if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SDRHIT_UNMARKEDOBJECT)
            {
               mpView->UnmarkAllObj();
            }

            BOOL bMarked = FALSE;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = mpView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
                else
                {
                    bMarked = mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
            {
                // Objekt verschieben
                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
            }
            else if (mpView->AreObjectsMarked())
            {
                /**************************************************************
                * Klebepunkt selektieren
                **************************************************************/
                if (!rMEvt.IsShift())
                    mpView->UnmarkAllGluePoints();

                mpView->BegMarkGluePoints(aMDPos);
            }
            else
            {
                /**************************************************************
                * Objekt selektieren
                **************************************************************/
                mpView->BegMarkObj(aMDPos);
            }
        }

        ForcePointer(&rMEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuEditGluePoints::MouseMove(const MouseEvent& rMEvt)
{
    mpView->SetActualWin( mpWindow );

    FuDraw::MouseMove(rMEvt);

    if (mpView->IsAction())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt( mpWindow->PixelToLogic(aPix) );
        ForceScroll(aPix);
        mpView->MovAction(aPnt);
    }

    ForcePointer(&rMEvt);

    return TRUE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuEditGluePoints::MouseButtonUp(const MouseEvent& rMEvt)
{
    mpView->SetActualWin( mpWindow );

    BOOL bReturn = FALSE;

    if (mpView->IsAction())
    {
        bReturn = TRUE;
        mpView->EndAction();
    }

    FuDraw::MouseButtonUp(rMEvt);

    USHORT nDrgLog = USHORT ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
    Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if (Abs(aMDPos.X() - aPos.X()) < nDrgLog &&
        Abs(aMDPos.Y() - aPos.Y()) < nDrgLog &&
        !rMEvt.IsShift() && !rMEvt.IsMod2())
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_NONE)
        {
            // Klick auf der Stelle: deselektieren
            mpView->UnmarkAllObj();
        }
    }

    mpWindow->ReleaseMouse();

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuEditGluePoints::KeyInput(const KeyEvent& rKEvt)
{
    mpView->SetActualWin( mpWindow );

    BOOL bReturn = FuDraw::KeyInput(rKEvt);

    return bReturn;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

BOOL FuEditGluePoints::Command(const CommandEvent& rCEvt)
{
    mpView->SetActualWin( mpWindow );
    return FuPoor::Command( rCEvt );
}

/*************************************************************************
|*
|* Funktion aktivieren
|*
\************************************************************************/

void FuEditGluePoints::Activate()
{
    mpView->SetGluePointEditMode();
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Funktion deaktivieren
|*
\************************************************************************/

void FuEditGluePoints::Deactivate()
{
    mpView->SetGluePointEditMode( FALSE );
    FuDraw::Deactivate();
}

/*************************************************************************
|*
|* Request verarbeiten
|*
\************************************************************************/

void FuEditGluePoints::ReceiveRequest(SfxRequest& rReq)
{
    switch (rReq.GetSlot())
    {
        case SID_GLUE_INSERT_POINT:
        {
            mpView->SetInsGluePointMode(!mpView->IsInsGluePointMode());
        }
        break;

        case SID_GLUE_ESCDIR_LEFT:
        {
            mpView->SetMarkedGluePointsEscDir( SDRESC_LEFT,
                    !mpView->IsMarkedGluePointsEscDir( SDRESC_LEFT ) );
        }
        break;

        case SID_GLUE_ESCDIR_RIGHT:
        {
            mpView->SetMarkedGluePointsEscDir( SDRESC_RIGHT,
                    !mpView->IsMarkedGluePointsEscDir( SDRESC_RIGHT ) );
        }
        break;

        case SID_GLUE_ESCDIR_TOP:
        {
            mpView->SetMarkedGluePointsEscDir( SDRESC_TOP,
                    !mpView->IsMarkedGluePointsEscDir( SDRESC_TOP ) );
        }
        break;

        case SID_GLUE_ESCDIR_BOTTOM:
        {
            mpView->SetMarkedGluePointsEscDir( SDRESC_BOTTOM,
                    !mpView->IsMarkedGluePointsEscDir( SDRESC_BOTTOM ) );
        }
        break;

        case SID_GLUE_PERCENT:
        {
            const SfxItemSet* pSet = rReq.GetArgs();
            const SfxPoolItem& rItem = pSet->Get(SID_GLUE_PERCENT);
            BOOL bPercent = ((const SfxBoolItem&) rItem).GetValue();
            mpView->SetMarkedGluePointsPercent(bPercent);
        }
        break;

        case SID_GLUE_HORZALIGN_CENTER:
        {
            mpView->SetMarkedGluePointsAlign(FALSE, SDRHORZALIGN_CENTER);
        }
        break;

        case SID_GLUE_HORZALIGN_LEFT:
        {
            mpView->SetMarkedGluePointsAlign(FALSE, SDRHORZALIGN_LEFT);
        }
        break;

        case SID_GLUE_HORZALIGN_RIGHT:
        {
            mpView->SetMarkedGluePointsAlign(FALSE, SDRHORZALIGN_RIGHT);
        }
        break;

        case SID_GLUE_VERTALIGN_CENTER:
        {
            mpView->SetMarkedGluePointsAlign(TRUE, SDRVERTALIGN_CENTER);
        }
        break;

        case SID_GLUE_VERTALIGN_TOP:
        {
            mpView->SetMarkedGluePointsAlign(TRUE, SDRVERTALIGN_TOP);
        }
        break;

        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            mpView->SetMarkedGluePointsAlign(TRUE, SDRVERTALIGN_BOTTOM);
        }
        break;
    }

    // Zum Schluss Basisklasse rufen
    FuPoor::ReceiveRequest(rReq);
}


} // end of namespace sd
