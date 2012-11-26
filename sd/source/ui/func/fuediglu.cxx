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
#include "precompiled_sd.hxx"

#include "fuediglu.hxx"
#include <svl/eitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdglue.hxx>
#include <sfx2/request.hxx>


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
#include "View.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"

namespace sd {

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
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq),
    meLastSdrViewEditMode(SDREDITMODE_EDIT)
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
    mpView->SetInsGluePointMode(false);
    mpViewShell->GetViewShellBase().GetToolBarManager()->AddToolBar(
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
    mpView->MarkGluePoints(0, true);
    mpView->SetInsGluePointMode(false);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuEditGluePoints::MouseButtonDown(const MouseEvent& rMEvt)
{
    mpView->SetActualOutDev( mpWindow );

    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    if (mpView->IsAction())
    {
        if (rMEvt.IsRight())
            mpView->BckAction();

        return true;
    }

    if (rMEvt.IsLeft())
    {
        bReturn = true;
        const double fHitLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(HITPIX, 0.0)).getLength());
        const double fDrgLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(DRGPIX, 0.0)).getLength());
        mpWindow->CaptureMouse();

        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_HANDLE)
        {
            /******************************************************************
            * Handle draggen
            ******************************************************************/
            const SdrHdl* pHdl = aVEvt.mpHdl;

            if (mpView->IsGluePointMarked(*aVEvt.mpObj, aVEvt.mnGlueId) && rMEvt.IsShift())
            {
                mpView->MarkGluePoint(aVEvt.mpObj, aVEvt.mnGlueId, true); // unmark
                pHdl = NULL;
            }

            if (pHdl)
            {
                // Handle draggen
                mpView->BegDragObj(aMDPos, aVEvt.mpHdl, fDrgLog);
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
                mpView->MarkGluePoints(0, true);

            mpView->BegMarkGluePoints(aMDPos);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Objekt verschieben
            ******************************************************************/
            mpView->BegDragObj(aMDPos, NULL, fDrgLog);
        }
        else if (eHit == SDRHIT_GLUEPOINT)
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
                mpView->MarkGluePoints(0, true);

            mpView->MarkGluePoint(aVEvt.mpObj, aVEvt.mnGlueId);
            SdrHdl* pHdl = mpView->GetGluePointHdl(aVEvt.mpObj, aVEvt.mnGlueId);

            if (pHdl)
            {
                mpView->BegDragObj(aMDPos, pHdl, fDrgLog);
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

            bool bMarked = false;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = mpView->MarkNextObj(aMDPos, fHitLog, rMEvt.IsShift());
                }
                else
                {
                    bMarked = mpView->MarkObj(aMDPos, fHitLog, rMEvt.IsShift());
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
            {
                // Objekt verschieben
                mpView->BegDragObj(aMDPos, aVEvt.mpHdl, fDrgLog);
            }
            else if (mpView->areSdrObjectsSelected())
            {
                /**************************************************************
                * Klebepunkt selektieren
                **************************************************************/
                if (!rMEvt.IsShift())
                    mpView->MarkGluePoints(0, true);

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

bool FuEditGluePoints::MouseMove(const MouseEvent& rMEvt)
{
    mpView->SetActualOutDev( mpWindow );

    FuDraw::MouseMove(rMEvt);

    if (mpView->IsAction())
    {
        const basegfx::B2DPoint aPix(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPix);

        ForceScroll(aPix);
        mpView->MovAction(aLogicPos);
    }

    ForcePointer(&rMEvt);

    return true;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuEditGluePoints::MouseButtonUp(const MouseEvent& rMEvt)
{
    mpView->SetActualOutDev( mpWindow );

    bool bReturn = false;

    if (mpView->IsAction())
    {
        bReturn = true;
        mpView->EndAction();
    }

    FuDraw::MouseButtonUp(rMEvt);

    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
    Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if (fabs(aMDPos.getX() - aPos.getX()) < nDrgLog &&
        fabs(aMDPos.getY() - aPos.getY()) < nDrgLog &&
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuEditGluePoints::KeyInput(const KeyEvent& rKEvt)
{
    mpView->SetActualOutDev( mpWindow );

    bool bReturn = FuDraw::KeyInput(rKEvt);

    return bReturn;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

bool FuEditGluePoints::Command(const CommandEvent& rCEvt)
{
    mpView->SetActualOutDev( mpWindow );
    return FuPoor::Command( rCEvt );
}

/*************************************************************************
|*
|* Funktion aktivieren
|*
\************************************************************************/

void FuEditGluePoints::Activate()
{
    meLastSdrViewEditMode = mpView->GetViewEditMode();
    mpView->SetViewEditMode(SDREDITMODE_GLUEPOINTEDIT);
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Funktion deaktivieren
|*
\************************************************************************/

void FuEditGluePoints::Deactivate()
{
    mpView->SetViewEditMode(meLastSdrViewEditMode);
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
            bool bPercent = ((const SfxBoolItem&) rItem).GetValue();
            mpView->SetMarkedGluePointsPercent(bPercent);
        }
        break;

        case SID_GLUE_HORZALIGN_CENTER:
        {
            mpView->SetMarkedGluePointsAlign(false, SDRHORZALIGN_CENTER);
        }
        break;

        case SID_GLUE_HORZALIGN_LEFT:
        {
            mpView->SetMarkedGluePointsAlign(false, SDRHORZALIGN_LEFT);
        }
        break;

        case SID_GLUE_HORZALIGN_RIGHT:
        {
            mpView->SetMarkedGluePointsAlign(false, SDRHORZALIGN_RIGHT);
        }
        break;

        case SID_GLUE_VERTALIGN_CENTER:
        {
            mpView->SetMarkedGluePointsAlign(true, SDRVERTALIGN_CENTER);
        }
        break;

        case SID_GLUE_VERTALIGN_TOP:
        {
            mpView->SetMarkedGluePointsAlign(true, SDRVERTALIGN_TOP);
        }
        break;

        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            mpView->SetMarkedGluePointsAlign(true, SDRVERTALIGN_BOTTOM);
        }
        break;
    }

    // Zum Schluss Basisklasse rufen
    FuPoor::ReceiveRequest(rReq);
}


} // end of namespace sd
