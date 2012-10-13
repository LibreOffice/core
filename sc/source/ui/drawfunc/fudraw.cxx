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

#include <editeng/editeng.hxx>  // EditEngine::IsSimpleCharInput
#include <editeng/outlobj.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "sc.hrc"
#include "fudraw.hxx"
#include "futext.hxx"
#include "tabvwsh.hxx"
#include "drwlayer.hxx"
#include "scresid.hxx"
#include "userdat.hxx"
#include "docsh.hxx"
#include "postit.hxx"
#include "globstr.hrc"
#include "drawview.hxx"

/*************************************************************************
|*
|* Basisklasse fuer alle Drawmodul-spezifischen Funktionen
|*
\************************************************************************/

FuDraw::FuDraw(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq) :
    FuPoor      (pViewSh, pWin, pViewP, pDoc, rReq),
    aNewPointer ( POINTER_ARROW ),
    aOldPointer ( POINTER_ARROW )
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDraw::~FuDraw()
{
}

/*************************************************************************
|*
|* Modifier-Tasten auswerten
|*
\************************************************************************/

void FuDraw::DoModifiers(const MouseEvent& rMEvt)
{
    //  Shift   = Ortho und AngleSnap
    //  Control = Snap (Toggle)
    //  Alt     = zentrisch

    sal_Bool bShift = rMEvt.IsShift();
    sal_Bool bAlt   = rMEvt.IsMod2();

    bool bOrtho     = bShift;
    sal_Bool bAngleSnap = bShift;
    sal_Bool bCenter    = bAlt;

    // #i33136#
    if(doConstructOrthogonal())
    {
        bOrtho = !bShift;
    }

    if (pView->IsOrtho() != bOrtho)
        pView->SetOrtho(bOrtho);
    if (pView->IsAngleSnapEnabled() != bAngleSnap)
        pView->SetAngleSnapEnabled(bAngleSnap);

    if (pView->IsCreate1stPointAsCenter() != bCenter)
        pView->SetCreate1stPointAsCenter(bCenter);
    if (pView->IsResizeAtCenter() != bCenter)
        pView->SetResizeAtCenter(bCenter);

}

void FuDraw::ResetModifiers()
{
    ScViewData* pViewData = pViewShell->GetViewData();
    const ScViewOptions& rOpt = pViewData->GetOptions();
    const ScGridOptions& rGrid = rOpt.GetGridOptions();
    sal_Bool bGridOpt = rGrid.GetUseGridSnap();

    if (pView->IsOrtho())
        pView->SetOrtho(false);
    if (pView->IsAngleSnapEnabled())
        pView->SetAngleSnapEnabled(false);

    if (pView->IsGridSnap() != bGridOpt)
        pView->SetGridSnap(bGridOpt);
    if (pView->IsSnapEnabled() != bGridOpt)
        pView->SetSnapEnabled(bGridOpt);

    if (pView->IsCreate1stPointAsCenter())
        pView->SetCreate1stPointAsCenter(false);
    if (pView->IsResizeAtCenter())
        pView->SetResizeAtCenter(false);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    DoModifiers( rMEvt );
    return false;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    //  evaluate modifiers only if in a drawing layer action
    //  (don't interfere with keyboard shortcut handling)
    if (pView->IsAction())
        DoModifiers( rMEvt );

    return false;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    ResetModifiers();
    return false;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

static sal_Bool lcl_KeyEditMode( SdrObject* pObj, ScTabViewShell* pViewShell, const KeyEvent* pInitialKey )
{
    sal_Bool bReturn = false;
    if ( pObj && pObj->ISA(SdrTextObj) && !pObj->ISA(SdrUnoObj) )
    {
        // start text edit - like FuSelection::MouseButtonUp,
        // but with bCursorToEnd instead of mouse position

        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
        sal_Bool bVertical = ( pOPO && pOPO->IsVertical() );
        sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

        // don't switch shells if text shell is already active
        FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
        if ( !pPoor || pPoor->GetSlotID() != nTextSlotId )
        {
            pViewShell->GetViewData()->GetDispatcher().
                Execute(nTextSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
        }

        // get the resulting FuText and set in edit mode
        pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
        if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // no RTTI
        {
            FuText* pText = (FuText*)pPoor;
            pText->SetInEditMode( pObj, NULL, sal_True, pInitialKey );
            //! set cursor to end of text
        }
        bReturn = sal_True;
    }
    return bReturn;
}

sal_Bool FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = false;
    ScViewData& rViewData = *pViewShell->GetViewData();

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( pViewShell->IsDrawTextShell() || aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
            {
                // in normale Draw-Shell, wenn Objekt selektiert, sonst Zeichnen aus
                rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                bReturn = sal_True;
            }
            else if ( pViewShell->IsDrawSelMode() )
            {
                pView->UnmarkAll();
                rViewData.GetDispatcher().Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                bReturn = sal_True;
            }
            else if ( pView->AreObjectsMarked() )
            {
                // III
                SdrHdlList& rHdlList = const_cast< SdrHdlList& >( pView->GetHdlList() );
                if( rHdlList.GetFocusHdl() )
                    rHdlList.ResetFocusHdl();
                else
                    pView->UnmarkAll();

                //  Beim Bezier-Editieren ist jetzt wieder das Objekt selektiert
                if (!pView->AreObjectsMarked())
                    pViewShell->SetDrawShell( false );

                bReturn = sal_True;
            }
            break;

        case KEY_DELETE:                    //! ueber Accelerator
            pView->DeleteMarked();
            bReturn = sal_True;
        break;

        case KEY_RETURN:
        {
            if( rKEvt.GetKeyCode().GetModifier() == 0 )
            {
                // activate OLE object on RETURN for selected object
                // put selected text object in edit mode
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    sal_Bool bOle = pViewShell->GetViewFrame()->GetFrame().IsInPlace();
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( pObj && pObj->ISA( SdrOle2Obj ) && !bOle )
                    {
                        pViewShell->ActivateObject( static_cast< SdrOle2Obj* >( pObj ), 0 );

                        // consumed
                        bReturn = sal_True;
                    }
                    else if ( lcl_KeyEditMode( pObj, pViewShell, NULL ) )       // start text edit for suitable object
                        bReturn = sal_True;
                }
            }
        }
        break;

        case KEY_F2:
        {
            if( rKEvt.GetKeyCode().GetModifier() == 0 )
            {
                // put selected text object in edit mode
                // (this is not SID_SETINPUTMODE, but F2 hardcoded, like in Writer)
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if ( lcl_KeyEditMode( pObj, pViewShell, NULL ) )            // start text edit for suitable object
                        bReturn = sal_True;
                }
            }
        }
        break;

        // #97016#
        case KEY_TAB:
        {
            // in calc do NOT start draw object selection using TAB/SHIFT-TAB when
            // there is not yet a object selected
            if(pView->AreObjectsMarked())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( !aCode.IsMod1() && !aCode.IsMod2() )
                {
                    // changeover to the next object
                    if(!pView->MarkNextObj( !aCode.IsShift() ))
                    {
                        // No next object: go over open end and
                        // get first from the other side
                        pView->UnmarkAllObj();
                        pView->MarkNextObj(!aCode.IsShift());
                    }

                    // II
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = sal_True;
                }

                // handle Mod1 and Mod2 to get travelling running on different systems
                if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
                {
                    // II do something with a selected handle?
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

                    ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                    // guarantee visibility of focused handle
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        Point aHdlPosition(pHdl->GetPos());
                        Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                        pView->MakeVisible(aVisRect, *pWindow);
                    }

                    // consumed
                    bReturn = sal_True;
                }
            }
        }
        break;

        // #97016#
        case KEY_END:
        {
            // in calc do NOT select the last draw object when
            // there is not yet a object selected
            if(pView->AreObjectsMarked())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    // mark last object
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(false);

                    // II
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = sal_True;
                }
            }
        }
        break;

        // #97016#
        case KEY_HOME:
        {
            // in calc do NOT select the first draw object when
            // there is not yet a object selected
            if(pView->AreObjectsMarked())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    // mark first object
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(sal_True);

                    // II
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = sal_True;
                }
            }
        }
        break;

        // #97016#
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            // in calc do cursor travelling of draw objects only when
            // there is a object selected yet
            if(pView->AreObjectsMarked())
            {

                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if(rMarkList.GetMarkCount() == 1)
                {
                    // disable cursor travelling on note objects as the tail connector position
                    // must not move.
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( ScDrawLayer::IsNoteCaption( pObj ) )
                        break;
                }

                long nX = 0;
                long nY = 0;
                sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    nX = 1;
                    nY = 0;
                }

                sal_Bool bReadOnly = rViewData.GetDocShell()->IsReadOnly();

                if(!rKEvt.GetKeyCode().IsMod1() && !bReadOnly)
                {
                    if(rKEvt.GetKeyCode().IsMod2())
                    {
                        // move in 1 pixel distance
                        Size aLogicSizeOnePixel = (pWindow) ? pWindow->PixelToLogic(Size(1,1)) : Size(100, 100);
                        nX *= aLogicSizeOnePixel.Width();
                        nY *= aLogicSizeOnePixel.Height();
                    }
                    else
                    {
                        // old, fixed move distance
                        nX *= 100;
                        nY *= 100;
                    }

                    // is there a movement to do?
                    if(0 != nX || 0 != nY)
                    {
                        // II
                        const SdrHdlList& rHdlList = pView->GetHdlList();
                        SdrHdl* pHdl = rHdlList.GetFocusHdl();

                        if(0L == pHdl)
                        {
                            // only take action when move is allowed
                            if(pView->IsMoveAllowed())
                            {
                                // restrict movement to WorkArea
                                const Rectangle& rWorkArea = pView->GetWorkArea();

                                if(!rWorkArea.IsEmpty())
                                {
                                    Rectangle aMarkRect(pView->GetMarkedObjRect());
                                    aMarkRect.Move(nX, nY);

                                    if(!aMarkRect.IsInside(rWorkArea))
                                    {
                                        if(aMarkRect.Left() < rWorkArea.Left())
                                        {
                                            nX += rWorkArea.Left() - aMarkRect.Left();
                                        }

                                        if(aMarkRect.Right() > rWorkArea.Right())
                                        {
                                            nX -= aMarkRect.Right() - rWorkArea.Right();
                                        }

                                        if(aMarkRect.Top() < rWorkArea.Top())
                                        {
                                            nY += rWorkArea.Top() - aMarkRect.Top();
                                        }

                                        if(aMarkRect.Bottom() > rWorkArea.Bottom())
                                        {
                                            nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                                        }
                                    }
                                }

                                // now move the selected draw objects
                                pView->MoveAllMarked(Size(nX, nY));

                                // II
                                pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                                bReturn = sal_True;
                            }
                        }
                        else
                        {
                            // move handle with index nHandleIndex
                            if(pHdl && (nX || nY))
                            {
                                // now move the Handle (nX, nY)
                                Point aStartPoint(pHdl->GetPos());
                                Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                                const SdrDragStat& rDragStat = pView->GetDragStat();

                                // start dragging
                                pView->BegDragObj(aStartPoint, 0, pHdl, 0);

                                if(pView->IsDragObj())
                                {
                                    bool bWasNoSnap = rDragStat.IsNoSnap();
                                    sal_Bool bWasSnapEnabled = pView->IsSnapEnabled();

                                    // switch snapping off
                                    if(!bWasNoSnap)
                                        ((SdrDragStat&)rDragStat).SetNoSnap(sal_True);
                                    if(bWasSnapEnabled)
                                        pView->SetSnapEnabled(false);

                                    pView->MovAction(aEndPoint);
                                    pView->EndDragObj();

                                    // restore snap
                                    if(!bWasNoSnap)
                                        ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                                    if(bWasSnapEnabled)
                                        pView->SetSnapEnabled(bWasSnapEnabled);
                                }

                                // make moved handle visible
                                Rectangle aVisRect(aEndPoint - Point(100, 100), Size(200, 200));
                                pView->MakeVisible(aVisRect, *pWindow);

                                bReturn = sal_True;
                            }
                        }
                    }
                }
            }
        }
        break;

        // #97016#
        case KEY_SPACE:
        {
            // in calc do only something when draw objects are selected
            if(pView->AreObjectsMarked())
            {
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    if(pHdl->GetKind() == HDL_POLY)
                    {
                        // rescue ID of point with focus
                        sal_uInt32 nPol(pHdl->GetPolyNum());
                        sal_uInt32 nPnt(pHdl->GetPointNum());

                        if(pView->IsPointMarked(*pHdl))
                        {
                            if(rKEvt.GetKeyCode().IsShift())
                            {
                                pView->UnmarkPoint(*pHdl);
                            }
                        }
                        else
                        {
                            if(!rKEvt.GetKeyCode().IsShift())
                            {
                                pView->UnmarkAllPoints();
                            }

                            pView->MarkPoint(*pHdl);
                        }

                        if(0L == rHdlList.GetFocusHdl())
                        {
                            // restore point with focus
                            SdrHdl* pNewOne = 0L;

                            for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
                            {
                                SdrHdl* pAct = rHdlList.GetHdl(a);

                                if(pAct
                                    && pAct->GetKind() == HDL_POLY
                                    && pAct->GetPolyNum() == nPol
                                    && pAct->GetPointNum() == nPnt)
                                {
                                    pNewOne = pAct;
                                }
                            }

                            if(pNewOne)
                            {
                                ((SdrHdlList&)rHdlList).SetFocusHdl(pNewOne);
                            }
                        }

                        bReturn = sal_True;
                    }
                }
            }
        }
        break;
    }

    if (!bReturn)
    {
        bReturn = FuPoor::KeyInput(rKEvt);
    }

    if (!bReturn)
    {
        // allow direct typing into a selected text object

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() && EditEngine::IsSimpleCharInput(rKEvt) )
        {
            SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

            // start text edit for suitable object, pass key event to OutlinerView
            if ( lcl_KeyEditMode( pObj, pViewShell, &rKEvt ) )
                bReturn = sal_True;
        }
    }

    return (bReturn);
}

// II
void FuDraw::SelectionHasChanged()
{
    const SdrHdlList& rHdlList = pView->GetHdlList();
    ((SdrHdlList&)rHdlList).ResetFocusHdl();
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuDraw::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuDraw::Deactivate()
{
    FuPoor::Deactivate();
}

/*************************************************************************
|*
|* Maus-Pointer umschalten
|*
\************************************************************************/

static sal_Bool lcl_UrlHit( SdrView* pView, const Point& rPosPixel, Window* pWindow )
{
    SdrViewEvent aVEvt;
    MouseEvent aMEvt( rPosPixel, 1, 0, MOUSE_LEFT );
    SdrHitKind eHit = pView->PickAnything( aMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

    if ( eHit != SDRHIT_NONE && aVEvt.pObj != NULL )
    {
        if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) && ScDrawLayer::GetHitIMapObject(
                                aVEvt.pObj, pWindow->PixelToLogic(rPosPixel), *pWindow ) )
            return sal_True;

        if ( aVEvt.eEvent == SDREVENT_EXECUTEURL )
            return sal_True;
    }

    return false;
}

void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    if ( !pView->IsAction() )
    {
        Point aPosPixel = pWindow->GetPointerPosPixel();
        sal_Bool bAlt       = pMEvt && pMEvt->IsMod2();
        Point aPnt      = pWindow->PixelToLogic( aPosPixel );
        SdrHdl* pHdl    = pView->PickHandle(aPnt);
        SdrObject* pObj;
        SdrPageView* pPV;

        ScMacroInfo* pInfo = 0;
        if ( pView->PickObj(aPnt, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER) )
        {
            if ( pObj->IsGroupObject() )
            {
                SdrObject* pHit = 0;
                if ( pView->PickObj(aMDPos, pView->getHitTolLog(), pHit, pPV, SDRSEARCH_DEEP ) )
                    pObj = pHit;
            }
            pInfo = ScDrawLayer::GetMacroInfo( pObj );
        }

        if ( pView->IsTextEdit() )
        {
            pViewShell->SetActivePointer(Pointer(POINTER_TEXT));        // kann nicht sein ?
        }
        else if ( pHdl )
        {
            pViewShell->SetActivePointer(
                pView->GetPreferedPointer( aPnt, pWindow ) );
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            pViewShell->SetActivePointer( Pointer(POINTER_MOVE) );
        }
        else if ( !bAlt && ( !pMEvt || !pMEvt->GetButtons() )
                        && lcl_UrlHit( pView, aPosPixel, pWindow ) )
        {
            //  kann mit ALT unterdrueckt werden
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );          // Text-URL / ImageMap
        }
        else if ( !bAlt && pView->PickObj(aPnt, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMACRO) )
        {
            //  kann mit ALT unterdrueckt werden
            SdrObjMacroHitRec aHitRec;  //! muss da noch irgendwas gesetzt werden ????
            pViewShell->SetActivePointer( pObj->GetMacroPointer(aHitRec) );
        }
        else if ( !bAlt && pInfo && (!pInfo->GetMacro().isEmpty() || !pInfo->GetHlink().isEmpty()) )
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );
        else if ( IsDetectiveHit( aPnt ) )
            pViewShell->SetActivePointer( Pointer( POINTER_DETECTIVE ) );
        else
            pViewShell->SetActivePointer( aNewPointer );            //! in Gridwin?
    }
}

sal_Bool FuDraw::IsSizingOrMovingNote( const MouseEvent& rMEvt ) const
{
    sal_Bool bIsSizingOrMoving = false;
    if ( rMEvt.IsLeft() )
    {
        const SdrMarkList& rNoteMarkList = pView->GetMarkedObjectList();
        if(rNoteMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rNoteMarkList.GetMark( 0 )->GetMarkedSdrObj();
            if ( ScDrawLayer::IsNoteCaption( pObj ) )
            {
                Point aMPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );
                bIsSizingOrMoving =
                    pView->PickHandle( aMPos ) ||      // handles to resize the note
                    pView->IsTextEditFrameHit( aMPos );         // frame for moving the note
            }
        }
    }
    return bIsSizingOrMoving;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
