/*************************************************************************
 *
 *  $RCSfile: fuediglu.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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
#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "fuediglu.hxx"
#include "frmview.hxx"


TYPEINIT1( FuEditGluePoints, FuDraw );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuEditGluePoints::FuEditGluePoints(SdViewShell*     pViewSh,
                             SdWindow*          pWin,
                             SdView*            pView,
                             SdDrawDocument*    pDoc,
                             SfxRequest&        rReq) :
    FuDraw(pViewSh, pWin, pView, pDoc, rReq)
{
    pViewShell->SwitchObjectBar(RID_GLUEPOINTS_TOOLBOX);
    pView->SetInsGluePointMode(FALSE);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuEditGluePoints::~FuEditGluePoints()
{
    pView->BrkAction();
    pView->UnmarkAllGluePoints();
    pView->SetInsGluePointMode(FALSE);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuEditGluePoints::MouseButtonDown(const MouseEvent& rMEvt)
{
    pView->SetActualWin( pWindow );

    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    if (pView->IsAction())
    {
        if (rMEvt.IsRight())
            pView->BckAction();

        return TRUE;
    }

    if (rMEvt.IsLeft())
    {
        bReturn = TRUE;
        USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        pWindow->CaptureMouse();

        SdrViewEvent aVEvt;
        SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_HANDLE)
        {
            /******************************************************************
            * Handle draggen
            ******************************************************************/
            SdrHdl* pHdl = aVEvt.pHdl;

            if (pView->IsGluePointMarked(aVEvt.pObj, aVEvt.nGlueId) && rMEvt.IsShift())
            {
                pView->UnmarkGluePoint(aVEvt.pObj, aVEvt.nGlueId, aVEvt.pPV);
                pHdl = NULL;
            }

            if (pHdl)
            {
                // Handle draggen
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
            }
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && pView->IsInsGluePointMode())
        {
            /******************************************************************
            * Klebepunkt einfuegen
            ******************************************************************/
            pView->BegInsGluePoint(aMDPos);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
                pView->UnmarkAllGluePoints();

            pView->BegMarkGluePoints(aMDPos, (OutputDevice*) NULL);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Objekt verschieben
            ******************************************************************/
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, NULL, nDrgLog);
        }
        else if (eHit == SDRHIT_GLUEPOINT)
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
                pView->UnmarkAllGluePoints();

            pView->MarkGluePoint(aVEvt.pObj, aVEvt.nGlueId, aVEvt.pPV);
            SdrHdl* pHdl = pView->GetGluePointHdl(aVEvt.pObj, aVEvt.nGlueId);

            if (pHdl)
            {
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            }
        }
        else
        {
            /******************************************************************
            * Objekt selektieren oder draggen
            ******************************************************************/
            if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SDRHIT_UNMARKEDOBJECT)
            {
               pView->UnmarkAllObj();
            }

            BOOL bMarked = FALSE;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = pView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
                else
                {
                    bMarked = pView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
            {
                // Objekt verschieben
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
            }
            else if (pView->HasMarkedObj())
            {
                /**************************************************************
                * Klebepunkt selektieren
                **************************************************************/
                if (!rMEvt.IsShift())
                    pView->UnmarkAllGluePoints();

                pView->BegMarkGluePoints(aMDPos, (OutputDevice*) NULL);
            }
            else
            {
                /**************************************************************
                * Objekt selektieren
                **************************************************************/
                pView->BegMarkObj(aMDPos);
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
    pView->SetActualWin( pWindow );

    FuDraw::MouseMove(rMEvt);

    if (pView->IsAction())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt( pWindow->PixelToLogic(aPix) );
        ForceScroll(aPix);
        pView->MovAction(aPnt);
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
    pView->SetActualWin( pWindow );

    BOOL bReturn = FALSE;

    if (pView->IsAction())
    {
        bReturn = TRUE;
        pView->EndAction();
    }

    FuDraw::MouseButtonUp(rMEvt);

    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
    Point aPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if (Abs(aMDPos.X() - aPos.X()) < nDrgLog &&
        Abs(aMDPos.Y() - aPos.Y()) < nDrgLog &&
        !rMEvt.IsShift() && !rMEvt.IsMod2())
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_NONE)
        {
            // Klick auf der Stelle: deselektieren
            pView->UnmarkAllObj();
        }
    }

    pWindow->ReleaseMouse();

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
    pView->SetActualWin( pWindow );

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
    pView->SetActualWin( pWindow );

    BOOL bReturn = FALSE;

    if ( rCEvt.GetCommand() == COMMAND_STARTDRAG )
    {
        // Drag&Drop gibt es hier nicht
        bReturn = TRUE;
    }
    else
    {
        bReturn = FuPoor::Command(rCEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* Funktion aktivieren
|*
\************************************************************************/

void FuEditGluePoints::Activate()
{
    pView->SetGluePointEditMode();
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Funktion deaktivieren
|*
\************************************************************************/

void FuEditGluePoints::Deactivate()
{
    pView->SetGluePointEditMode( FALSE );
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
            pView->SetInsGluePointMode(!pView->IsInsGluePointMode());
        }
        break;

        case SID_GLUE_ESCDIR_LEFT:
        {
            pView->SetMarkedGluePointsEscDir( SDRESC_LEFT,
                    !pView->IsMarkedGluePointsEscDir( SDRESC_LEFT ) );
        }
        break;

        case SID_GLUE_ESCDIR_RIGHT:
        {
            pView->SetMarkedGluePointsEscDir( SDRESC_RIGHT,
                    !pView->IsMarkedGluePointsEscDir( SDRESC_RIGHT ) );
        }
        break;

        case SID_GLUE_ESCDIR_TOP:
        {
            pView->SetMarkedGluePointsEscDir( SDRESC_TOP,
                    !pView->IsMarkedGluePointsEscDir( SDRESC_TOP ) );
        }
        break;

        case SID_GLUE_ESCDIR_BOTTOM:
        {
            pView->SetMarkedGluePointsEscDir( SDRESC_BOTTOM,
                    !pView->IsMarkedGluePointsEscDir( SDRESC_BOTTOM ) );
        }
        break;

        case SID_GLUE_PERCENT:
        {
            const SfxItemSet* pSet = rReq.GetArgs();
            const SfxPoolItem& rItem = pSet->Get(SID_GLUE_PERCENT);
            BOOL bPercent = ((const SfxBoolItem&) rItem).GetValue();
            pView->SetMarkedGluePointsPercent(bPercent);
        }
        break;

        case SID_GLUE_HORZALIGN_CENTER:
        {
            pView->SetMarkedGluePointsAlign(FALSE, SDRHORZALIGN_CENTER);
        }
        break;

        case SID_GLUE_HORZALIGN_LEFT:
        {
            pView->SetMarkedGluePointsAlign(FALSE, SDRHORZALIGN_LEFT);
        }
        break;

        case SID_GLUE_HORZALIGN_RIGHT:
        {
            pView->SetMarkedGluePointsAlign(FALSE, SDRHORZALIGN_RIGHT);
        }
        break;

        case SID_GLUE_VERTALIGN_CENTER:
        {
            pView->SetMarkedGluePointsAlign(TRUE, SDRVERTALIGN_CENTER);
        }
        break;

        case SID_GLUE_VERTALIGN_TOP:
        {
            pView->SetMarkedGluePointsAlign(TRUE, SDRVERTALIGN_TOP);
        }
        break;

        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            pView->SetMarkedGluePointsAlign(TRUE, SDRVERTALIGN_BOTTOM);
        }
        break;
    }

    // Zum Schluss Basisklasse rufen
    FuPoor::ReceiveRequest(rReq);
}


