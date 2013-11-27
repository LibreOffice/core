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

//------------------------------------------------------------------------

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

    bool bShift = rMEvt.IsShift();
//    bool bCtrl  = rMEvt.IsMod1();
    bool bAlt = rMEvt.IsMod2();

//    ScViewData* pViewData = pViewShell->GetViewData();
//    const ScViewOptions& rOpt = pViewData->GetOptions();
//    const ScGridOptions& rGrid = rOpt.GetGridOptions();
//    bool bGridOpt = rGrid.GetUseGridSnap();

    bool bOrtho = bShift;
    bool bAngleSnap = bShift;
//    bool bGridSnap  = ( bGridOpt != bCtrl );        // andere Snap's nicht unterstuetzt
    bool bCenter = bAlt;

    // #i33136#
    if(doConstructOrthogonal())
    {
        bOrtho = !bShift;
    }

    if (pView->IsOrthogonal() != bOrtho)
    {
        pView->SetOrthogonal(bOrtho);
    }

    if (pView->IsAngleSnapEnabled() != bAngleSnap)
    {
        pView->SetAngleSnapEnabled(bAngleSnap);
    }

/*  Control fuer Snap beisst sich beim Verschieben mit "kopieren" !!!

    if (pView->IsGridSnap() != bGridSnap)
    {
        pView->SetGridSnap(bGridSnap);
    }

    if (pView->IsSnapEnabled() != bGridSnap)
    {
        pView->SetSnapEnabled(bGridSnap);
    }
*/
    if (pView->IsCreate1stPointAsCenter() != bCenter)
    {
        pView->SetCreate1stPointAsCenter(bCenter);
    }

    if (pView->IsResizeAtCenter() != bCenter)
    {
        pView->SetResizeAtCenter(bCenter);
    }
}

void FuDraw::ResetModifiers()
{
    ScViewData* pViewData = pViewShell->GetViewData();
    const ScViewOptions& rOpt = pViewData->GetOptions();
    const ScGridOptions& rGrid = rOpt.GetGridOptions();
    bool bGridOpt = rGrid.GetUseGridSnap();

    if (pView->IsOrthogonal())
    {
        pView->SetOrthogonal(false);
    }
    if (pView->IsAngleSnapEnabled())
    {
        pView->SetAngleSnapEnabled(false);
    }

    if (pView->IsGridSnap() != bGridOpt)
    {
        pView->SetGridSnap(bGridOpt);
    }
    if (pView->IsSnapEnabled() != bGridOpt)
    {
        pView->SetSnapEnabled(bGridOpt);
    }

    if (pView->IsCreate1stPointAsCenter())
    {
        pView->SetCreate1stPointAsCenter(false);
    }
    if (pView->IsResizeAtCenter())
    {
        pView->SetResizeAtCenter(false);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool __EXPORT FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    DoModifiers( rMEvt );
    return sal_False;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool __EXPORT FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    //  #106438# evaluate modifiers only if in a drawing layer action
    //  (don't interfere with keyboard shortcut handling)
    if (pView->IsAction())
        DoModifiers( rMEvt );

    return sal_False;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    ResetModifiers();
    return sal_False;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool lcl_KeyEditMode( SdrObject* pObj, ScTabViewShell* pViewShell, const KeyEvent* pInitialKey )
{
    sal_Bool bReturn = sal_False;
    if ( pObj && dynamic_cast< SdrTextObj* >(pObj) && !dynamic_cast< SdrUnoObj* >(pObj) )
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

sal_Bool __EXPORT FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;
    ScViewData& rViewData = *pViewShell->GetViewData();

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:

    /* 18.12.95: TextShell beibehalten nicht mehr gewuenscht...
     *
     *          if ( pView->IsAction() )
     *          {
     *              pView->BrkAction();
     *              pWindow->ReleaseMouse();
     *              bReturn = sal_True;
     *          }
     *          else if ( pView->IsTextEdit() )
     *          {
     *              pView->EndTextEdit();
     *              pView->SetViewEditMode(SDREDITMODE_CREATE);
     *              pViewShell->GetScDrawView()->InvalidateDrawTextAttrs();
     *              bReturn = sal_True;
     *          }
     *          else
     */

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
            else if ( pView->areSdrObjectsSelected() )
            {
                // #97016# III
                SdrHdlList& rHdlList = const_cast< SdrHdlList& >( pView->GetHdlList() );
                if( rHdlList.GetFocusHdl() )
                    rHdlList.ResetFocusHdl();
                else
                    pView->UnmarkAll();

                //  Beim Bezier-Editieren ist jetzt wieder das Objekt selektiert
                if (!pView->areSdrObjectsSelected())
                    pViewShell->SetDrawShell( sal_False );

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
                // #98256# activate OLE object on RETURN for selected object
                // #98198# put selected text object in edit mode
                SdrObject* pSelected = pView->getSelectedIfSingle();

                if( !pView->IsTextEdit() && pSelected )
                {
                    sal_Bool bOle = pViewShell->GetViewFrame()->GetFrame().IsInPlace();
                    SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pSelected);

                    if( pSdrOle2Obj && !bOle )
                    {
                        pViewShell->ActivateObject( pSdrOle2Obj, 0 );

                        // consumed
                        bReturn = sal_True;
                    }
                    else if ( lcl_KeyEditMode( pSelected, pViewShell, NULL ) )      // start text edit for suitable object
                        bReturn = sal_True;
                }
            }
        }
        break;

        case KEY_F2:
        {
            if( rKEvt.GetKeyCode().GetModifier() == 0 )
            {
                // #98198# put selected text object in edit mode
                // (this is not SID_SETINPUTMODE, but F2 hardcoded, like in Writer)
                SdrObject* pSelected = pView->getSelectedIfSingle();

                if( !pView->IsTextEdit() && pSelected )
                {
                    if ( lcl_KeyEditMode( pSelected, pViewShell, NULL ) )           // start text edit for suitable object
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
            if(pView->areSdrObjectsSelected())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( !aCode.IsMod1() && !aCode.IsMod2() )
                {
                    // changeover to the next object
                    if(!pView->MarkNextObj( !aCode.IsShift() ))
                    {
//IAccessibility2 Implementation 2009-----
                        //If there is only one object, don't do the UnmarkAlllObj() & MarkNextObj().
                        if ( pView->GetMarkableObjCount() > 1 && pView->HasMarkableObj() )
                        {
//-----IAccessibility2 Implementation 2009
                        // #97016# No next object: go over open end and
                        // get first from the other side
                        pView->UnmarkAllObj();
                        pView->MarkNextObj(!aCode.IsShift());
                        }
                    }

                    // #97016# II
                    if(pView->areSdrObjectsSelected())
                    {
                        pView->MakeVisibleAtView(pView->getMarkedObjectSnapRange(), *pWindow);
                    }

                    bReturn = sal_True;
                }

                // #98994# handle Mod1 and Mod2 to get travelling running on different systems
                if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
                {
                    // #97016# II do something with a selected handle?
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

                    ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                    // guarantee visibility of focused handle
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        const basegfx::B2DRange aRange(
                            pHdl->getPosition() - basegfx::B2DPoint(100.0, 100.0),
                            pHdl->getPosition() + basegfx::B2DPoint(100.0, 100.0));

                        pView->MakeVisibleAtView(aRange, *pWindow);
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
            if(pView->areSdrObjectsSelected())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    // #97016# mark last object
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(false);

                    // #97016# II
                    if(pView->areSdrObjectsSelected())
                    {
                        pView->MakeVisibleAtView(pView->getMarkedObjectSnapRange(), *pWindow);
                    }

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
            if(pView->areSdrObjectsSelected())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    // #97016# mark first object
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(true);

                    // #97016# II
                    if(pView->areSdrObjectsSelected())
                    {
                        pView->MakeVisibleAtView(pView->getMarkedObjectSnapRange(), *pWindow);
                    }

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
            if(pView->areSdrObjectsSelected())
            {
                const SdrObject* pSelected = pView->getSelectedIfSingle();

                if(pSelected)
                {
                    // disable cursor travelling on note objects as the tail connector position
                    // must not move.
                    if( ScDrawLayer::IsNoteCaption( *pSelected ) )
                        break;
                }

                basegfx::B2DVector aMove(0.0, 0.0);
                sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    aMove = basegfx::B2DPoint(0.0, -1.0);
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    aMove = basegfx::B2DPoint(0.0, 1.0);
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    aMove = basegfx::B2DPoint(-1.0, 0.0);
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    aMove = basegfx::B2DPoint(1.0, 0.0);
                }

                const bool bReadOnly(rViewData.GetDocShell()->IsReadOnly());

                if(!rKEvt.GetKeyCode().IsMod1() && !bReadOnly)
                {
                    if(rKEvt.GetKeyCode().IsMod2())
                    {
                        // #97016# move in 1 pixel distance
                        const basegfx::B2DVector aLogicOnePix(
                            pWindow->GetInverseViewTransformation() *
                            basegfx::B2DVector(100.0, 100.0));

                        aMove *= aLogicOnePix;
                    }
                    else if(rKEvt.GetKeyCode().IsShift()) // #121236# Support for shift key in calc
                    {
                        aMove *= 1000.0;
                    }
                    else
                    {
                        // old, fixed move distance
                        aMove *= 100.0;
                    }

                    // is there a movement to do?
                    if(!aMove.equalZero())
                    {
                        // #97016# II
                        const SdrHdlList& rHdlList = pView->GetHdlList();
                        SdrHdl* pHdl = rHdlList.GetFocusHdl();

                        if(!pHdl)
                        {
                            // #107086# only take action when move is allowed
                            if(pView->IsMoveAllowed())
                            {
                                // #90129# restrict movement to WorkArea
                                const basegfx::B2DRange& rWorkRange = pView->GetWorkArea();

                                if(!rWorkRange.isEmpty())
                                {
                                    basegfx::B2DRange aMarkRange(pView->getMarkedObjectSnapRange());
                                    aMarkRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aMove));

                                    if(!aMarkRange.isInside(rWorkRange))
                                    {
                                        if(aMarkRange.getMinX() < rWorkRange.getMinX())
                                        {
                                            aMove.setX(aMove.getX() + rWorkRange.getMinX() - aMarkRange.getMinX());
                                        }

                                        if(aMarkRange.getMaxX() > rWorkRange.getMaxX())
                                        {
                                            aMove.setX(aMove.getX() - aMarkRange.getMaxX() - rWorkRange.getMaxX());
                                        }

                                        if(aMarkRange.getMinY() < rWorkRange.getMinY())
                                        {
                                            aMove.setY(aMove.getY() + rWorkRange.getMinY() - aMarkRange.getMinY());
                                        }

                                        if(aMarkRange.getMaxY() > rWorkRange.getMaxY())
                                        {
                                            aMove.setY(aMove.getY() - aMarkRange.getMaxY() - rWorkRange.getMaxY());
                                        }
                                    }
                                }

                                // now move the selected draw objects
                                pView->MoveMarkedObj(aMove);

                                // #97016# II
                                pView->MakeVisibleAtView(pView->getMarkedObjectSnapRange(), *pWindow);

                                bReturn = sal_True;
                            }
                        }
                        else
                        {
                            // move handle with index nHandleIndex
                            pView->MoveHandleByVector(*pHdl, aMove, pWindow, 0);
                            bReturn = sal_True;
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
            if(pView->areSdrObjectsSelected())
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
                                pView->MarkPoint(*pHdl, true); // unmark
                            }
                        }
                        else
                        {
                            if(!rKEvt.GetKeyCode().IsShift())
                            {
                                pView->MarkPoints(0, true); // unmarkall
                            }

                            pView->MarkPoint(*pHdl);
                        }

                        if(0L == rHdlList.GetFocusHdl())
                        {
                            // restore point with focus
                            SdrHdl* pNewOne = 0L;

                            for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
                            {
                                SdrHdl* pAct = rHdlList.GetHdlByIndex(a);

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
        // #98198# allow direct typing into a selected text object
        SdrObject* pSelected = pView->getSelectedIfSingle();

        if( !pView->IsTextEdit() && pSelected && EditEngine::IsSimpleCharInput(rKEvt) )
        {
            // start text edit for suitable object, pass key event to OutlinerView
            if ( lcl_KeyEditMode( pSelected, pViewShell, &rKEvt ) )
                bReturn = sal_True;
        }
    }

    return (bReturn);
}

void FuDraw::SelectionHasChanged()
{
}

/*************************************************************************
|*
|* Vor dem Scrollen Selektionsdarstellung ausblenden
|*
\************************************************************************/

void FuDraw::ScrollStart()
{
//      HideShownXor in Gridwin
}

/*************************************************************************
|*
|* Nach dem Scrollen Selektionsdarstellung wieder anzeigen
|*
\************************************************************************/

void FuDraw::ScrollEnd()
{
//      ShowShownXor in Gridwin
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

sal_Bool lcl_UrlHit( SdrView* pView, const Point& rPosPixel, Window* pWindow )
{
    SdrViewEvent aVEvt;
    MouseEvent aMEvt( rPosPixel, 1, 0, MOUSE_LEFT );
    SdrHitKind eHit = pView->PickAnything( aMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

    if ( eHit != SDRHIT_NONE && aVEvt.mpObj != NULL )
    {
        const basegfx::B2DPoint aPixelPos(rPosPixel.X(), rPosPixel.Y());
        const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

        if ( ScDrawLayer::GetIMapInfo( aVEvt.mpObj ) && ScDrawLayer::GetHitIMapObject( *aVEvt.mpObj, aLogicPos, *pWindow ) )
            return sal_True;

        if ( aVEvt.meEvent == SDREVENT_EXECUTEURL )
            return sal_True;
    }

    return sal_False;
}

void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    if ( !pView->IsAction() )
    {
        Point aPosPixel = pWindow->GetPointerPosPixel();
        sal_Bool bAlt       = pMEvt && pMEvt->IsMod2();
        const basegfx::B2DPoint aPnt(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(aPosPixel.X(), aPosPixel.Y()));
        SdrHdl* pHdl    = pView->PickHandle(aPnt);
        SdrObject* pObj;

        ScMacroInfo* pInfo = 0;
        if ( pView->PickObj(aPnt, pView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER) )
        {
            if ( pObj->getChildrenOfSdrObject() )
            {
                SdrObject* pHit = 0;
                if ( pView->PickObj(aMDPos, pView->getHitTolLog(), pHit, SDRSEARCH_DEEP ) )
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
        else if ( pView->IsMarkedObjHit(aPnt) )
        {
            pViewShell->SetActivePointer( Pointer(POINTER_MOVE) );
        }
        else if ( !bAlt && ( !pMEvt || !pMEvt->GetButtons() )
                        && lcl_UrlHit( pView, aPosPixel, pWindow ) )
        {
            //  kann mit ALT unterdrueckt werden
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );          // Text-URL / ImageMap
        }
        else if ( !bAlt && pView->PickObj(aPnt, pView->getHitTolLog(), pObj, SDRSEARCH_PICKMACRO) )
        {
            //  kann mit ALT unterdrueckt werden
            SdrObjMacroHitRec aHitRec;  //! muss da noch irgendwas gesetzt werden ????
            pViewShell->SetActivePointer( pObj->GetMacroPointer(aHitRec) );
        }
#ifdef ISSUE66550_HLINK_FOR_SHAPES
        else if ( !bAlt && pInfo && ((pInfo->GetMacro().getLength() > 0) || (pInfo->GetHlink().getLength() > 0)) )
#else
        else if ( !bAlt && pInfo && (pInfo->GetMacro().getLength() > 0) )
#endif
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );
        else if ( IsDetectiveHit( aPnt ) )
            pViewShell->SetActivePointer( Pointer( POINTER_DETECTIVE ) );
        else
            pViewShell->SetActivePointer( aNewPointer );            //! in Gridwin?
    }
}

sal_Bool FuDraw::IsSizingOrMovingNote( const MouseEvent& rMEvt ) const
{
    sal_Bool bIsSizingOrMoving = sal_False;
    if ( rMEvt.IsLeft() )
    {
        const SdrObject* pSelected = pView->getSelectedIfSingle();

        if(pSelected)
        {
            if ( ScDrawLayer::IsNoteCaption( *pSelected ) )
            {
                const basegfx::B2DPoint aMPos(
                    pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

                bIsSizingOrMoving =
                    pView->PickHandle( aMPos ) ||      // handles to resize the note
                    pView->IsTextEditFrameHit( aMPos );         // frame for moving the note
            }
        }
    }
    return bIsSizingOrMoving;
}
