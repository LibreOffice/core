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
#include <editeng/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdouno.hxx>
#include <sfx2/dispatch.hxx>

#include "fuconstr.hxx"
#include "fudraw.hxx"
#include "tabvwsh.hxx"
#include "futext.hxx"
#include "sc.hrc"
#include "drawview.hxx"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

//------------------------------------------------------------------------

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct::FuConstruct(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuDraw(pViewSh, pWin, pViewP, pDoc, rReq)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstruct::~FuConstruct()
{
}

sal_uInt8 FuConstruct::Command(const CommandEvent& rCEvt)
{
    //  special code for non-VCL OS2/UNX removed

    return FuDraw::Command( rCEvt );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    if ( pView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return sal_True;
    }

    aDragTimer.Start();

    aMDPos = pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

    if ( rMEvt.IsLeft() )
    {
        pWindow->CaptureMouse();

        SdrHdl* pHdl = pView->PickHandle(aMDPos);

        if ( pHdl != NULL || pView->IsMarkedObjHit(aMDPos) )
        {
            const double fTolerance(basegfx::B2DVector(pWindow->GetInverseViewTransformation() * basegfx::B2DVector(1.0, 0.0)).getLength());
            pView->BegDragObj(aMDPos, pHdl, fTolerance);
            bReturn = sal_True;
        }
        else if ( pView->areSdrObjectsSelected() )
        {
            pView->UnmarkAll();
            bReturn = sal_True;
        }
    }

    bIsInDragMode = sal_False;

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        const basegfx::B2DPoint aOldPixel(pWindow->GetViewTransformation() * aMDPos);
        const basegfx::B2DPoint aNewPixel(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

        if ( fabs( aOldPixel.getX() - aNewPixel.getX() ) > SC_MAXDRAGMOVE ||
             fabs( aOldPixel.getY() - aNewPixel.getY() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    const Point aPix(rMEvt.GetPosPixel());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(aPix.X(), aPix.Y()));

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aLogicPos);
    }
    else
    {
        SdrHdl* pHdl = pView->PickHandle(aLogicPos);

        if ( pHdl )
        {
            pViewShell->SetActivePointer(pHdl->GetPointer());
        }
        else if ( pView->IsMarkedObjHit(aLogicPos) )
        {
            pViewShell->SetActivePointer(Pointer(POINTER_MOVE));
        }
        else
        {
            pViewShell->SetActivePointer( aNewPointer );
        }
    }
    return sal_True;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = SimpleMouseButtonUp( rMEvt );

    //      Doppelklick auf Textobjekt? (->fusel)

    sal_uInt16 nClicks = rMEvt.GetClicks();
    if ( nClicks == 2 && rMEvt.IsLeft() )
    {
        if ( pView->areSdrObjectsSelected() )
            {
            SdrObject* pSelected = pView->getSelectedIfSingle();

            if (pSelected)
            {
                //  #49458# bei Uno-Controls nicht in Textmodus
                if ( dynamic_cast< SdrTextObj* >(pSelected) && !dynamic_cast< SdrUnoObj* >(pSelected) )
                {
                    OutlinerParaObject* pOPO = pSelected->GetOutlinerParaObject();
                    sal_Bool bVertical = ( pOPO && pOPO->IsVertical() );
                    sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                    pViewShell->GetViewData()->GetDispatcher().
                        Execute(nTextSlotId, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

                    // jetzt den erzeugten FuText holen und in den EditModus setzen
                    FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                    if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // hat keine RTTI
                    {
                        FuText* pText = (FuText*)pPoor;
                        Point aMousePixel = rMEvt.GetPosPixel();
                        pText->SetInEditMode( pSelected, &aMousePixel );
                    }
                    bReturn = sal_True;
                }
            }
        }
    }

    FuDraw::MouseButtonUp(rMEvt);

    return bReturn;
}

//      SimpleMouseButtonUp - ohne Test auf Doppelklick

sal_Bool FuConstruct::SimpleMouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool    bReturn = sal_True;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aPnt(pWindow->GetInverseViewTransformation() * aPixelPos);

    if ( pView->IsDragObj() )
         pView->EndDragObj( rMEvt.IsMod1() );

    else if ( pView->IsMarkObj() )
        pView->EndMarkObj();

    else bReturn = sal_False;

    if ( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();

        if ( !pView->areSdrObjectsSelected() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, 2.0, false, rMEvt.IsMod1());

            SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
            if ( pView->areSdrObjectsSelected() )
                rDisp.Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool __EXPORT FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                pWindow->ReleaseMouse();
                bReturn = sal_True;
            }
            else                            // Zeichenmodus beenden
            {
                pViewShell->GetViewData()->GetDispatcher().
                    Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            }
            break;

        case KEY_DELETE:
            pView->DeleteMarked();
            bReturn = sal_True;
            break;
    }

    if ( !bReturn )
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstruct::Activate()
{
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstruct::Deactivate()
{
    FuDraw::Deactivate();
}




