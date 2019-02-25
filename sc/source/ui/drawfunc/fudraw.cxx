/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include <sc.hrc>
#include <fudraw.hxx>
#include <futext.hxx>
#include <tabvwsh.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <docsh.hxx>
#include <drawview.hxx>

// base class for draw module specific functions
FuDraw::FuDraw(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, const SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pViewP, pDoc, rReq)
    , aNewPointer(PointerStyle::Arrow)
    , aOldPointer(PointerStyle::Arrow)
{
}

FuDraw::~FuDraw()
{
}

void FuDraw::DoModifiers(const MouseEvent& rMEvt)
{
    //  Shift   = Ortho and AngleSnap
    //  Control = Snap (Toggle)
    //  Alt     = centric

    bool bShift = rMEvt.IsShift();
    bool bAlt   = rMEvt.IsMod2();

    bool bOrtho     = bShift;
    bool bAngleSnap = bShift;
    bool bCenter    = bAlt;

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
    if (!pView)
        return;

    ScViewData& rViewData = rViewShell.GetViewData();
    const ScViewOptions& rOpt = rViewData.GetOptions();
    const ScGridOptions& rGrid = rOpt.GetGridOptions();
    bool bGridOpt = rGrid.GetUseGridSnap();

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

bool FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    DoModifiers( rMEvt );
    return false;
}

bool FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    //  evaluate modifiers only if in a drawing layer action
    //  (don't interfere with keyboard shortcut handling)
    if (pView->IsAction())
        DoModifiers( rMEvt );

    return false;
}

bool FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    ResetModifiers();
    return false;
}

// Process Keyboard events. Return true if an event is being handled
static bool lcl_KeyEditMode( SdrObject* pObj, ScTabViewShell& rViewShell, const KeyEvent* pInitialKey )
{
    bool bReturn = false;
    if ( dynamic_cast<const SdrTextObj*>( pObj) != nullptr && dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr )
    {
        // start text edit - like FuSelection::MouseButtonUp,
        // but with bCursorToEnd instead of mouse position

        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
        bool bVertical = ( pOPO && pOPO->IsVertical() );
        sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

        // don't switch shells if text shell is already active
        FuPoor* pPoor = rViewShell.GetViewData().GetView()->GetDrawFuncPtr();
        if ( !pPoor || pPoor->GetSlotID() != nTextSlotId )
        {
            rViewShell.GetViewData().GetDispatcher().
                Execute(nTextSlotId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
        }

        // get the resulting FuText and set in edit mode
        pPoor = rViewShell.GetViewData().GetView()->GetDrawFuncPtr();
        if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // no RTTI
        {
            FuText* pText = static_cast<FuText*>(pPoor);
            pText->SetInEditMode( pObj, nullptr, true, pInitialKey );
            //! set cursor to end of text
        }
        bReturn = true;
    }
    return bReturn;
}

bool FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;
    ScViewData& rViewData = rViewShell.GetViewData();

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( rViewShell.IsDrawTextShell() || aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
            {
                // if object selected -> normal draw-shell, else turn off drawing
                rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
                bReturn = true;
            }
            else if ( rViewShell.IsDrawSelMode() )
            {
                pView->UnmarkAll();
                rViewData.GetDispatcher().Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
                bReturn = true;
            }
            else if ( pView->AreObjectsMarked() )
            {
                // III
                SdrHdlList& rHdlList = const_cast< SdrHdlList& >( pView->GetHdlList() );
                if( rHdlList.GetFocusHdl() )
                    rHdlList.ResetFocusHdl();
                else
                    pView->UnmarkAll();

                //  while bezier editing, object is selected
                if (!pView->AreObjectsMarked())
                    rViewShell.SetDrawShell( false );

                bReturn = true;
            }
            break;

        case KEY_DELETE:                    //! via accelerator
            pView->DeleteMarked();
            bReturn = true;
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
                    bool bOle = rViewShell.GetViewFrame()->GetFrame().IsInPlace();
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( dynamic_cast<const SdrOle2Obj*>( pObj) && !bOle )
                    {
                        rViewShell.ActivateObject( static_cast< SdrOle2Obj* >( pObj ), 0 );

                        // consumed
                        bReturn = true;
                    }
                    else if ( lcl_KeyEditMode( pObj, rViewShell, nullptr ) )       // start text edit for suitable object
                        bReturn = true;
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
                    if ( lcl_KeyEditMode( pObj, rViewShell, nullptr ) )            // start text edit for suitable object
                        bReturn = true;
                }
            }
        }
        break;

        case KEY_TAB:
        {
            // in calc do NOT start draw object selection using TAB/SHIFT-TAB when
            // there is not yet a object selected
            if(pView->AreObjectsMarked())
            {
                vcl::KeyCode aCode = rKEvt.GetKeyCode();

                if ( !aCode.IsMod1() && !aCode.IsMod2() )
                {
                    // changeover to the next object
                    if(!pView->MarkNextObj( !aCode.IsShift() ))
                    {
                        //If there is only one object, don't do the UnmarkAllObj() & MarkNextObj().
                        if ( pView->HasMultipleMarkableObjects() && pView->HasMarkableObj() )
                        {
                            // No next object: go over open end and
                            // get first from the other side
                            pView->UnmarkAllObj();
                            pView->MarkNextObj(!aCode.IsShift());
                        }
                    }

                    // II
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = true;
                }

                // handle Mod1 and Mod2 to get travelling running on different systems
                if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
                {
                    // II do something with a selected handle?
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    bool bForward(!rKEvt.GetKeyCode().IsShift());

                    const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl(bForward);

                    // guarantee visibility of focused handle
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        Point aHdlPosition(pHdl->GetPos());
                        tools::Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                        pView->MakeVisible(aVisRect, *pWindow);
                    }

                    // consumed
                    bReturn = true;
                }
            }
        }
        break;

        case KEY_END:
        {
            // in calc do NOT select the last draw object when
            // there is not yet a object selected
            if(pView->AreObjectsMarked())
            {
                vcl::KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    // mark last object
                    pView->UnmarkAllObj();
                    pView->MarkNextObj();

                    // II
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = true;
                }
            }
        }
        break;

        case KEY_HOME:
        {
            // in calc do NOT select the first draw object when
            // there is not yet a object selected
            if(pView->AreObjectsMarked())
            {
                vcl::KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    // mark first object
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(true);

                    // II
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = true;
                }
            }
        }
        break;

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
                    // scroll up
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // scroll down
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // scroll left
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // scroll right
                    nX = 1;
                    nY = 0;
                }

                bool bReadOnly = rViewData.GetDocShell()->IsReadOnly();

                if(!rKEvt.GetKeyCode().IsMod1() && !bReadOnly)
                {
                    if(rKEvt.GetKeyCode().IsMod2())
                    {
                        // move in 1 pixel distance
                        Size aLogicSizeOnePixel = pWindow ? pWindow->PixelToLogic(Size(1,1)) : Size(100, 100);
                        nX *= aLogicSizeOnePixel.Width();
                        nY *= aLogicSizeOnePixel.Height();
                    }
                    else if(rKEvt.GetKeyCode().IsShift()) // #i121236# Support for shift key in calc
                    {
                        nX *= 1000;
                        nY *= 1000;
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

                        if(nullptr == pHdl)
                        {
                            // only take action when move is allowed
                            if(pView->IsMoveAllowed())
                            {
                                // restrict movement to WorkArea
                                const tools::Rectangle& rWorkArea = pView->GetWorkArea();

                                if(!rWorkArea.IsEmpty())
                                {
                                    tools::Rectangle aMarkRect(pView->GetMarkedObjRect());
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

                                bReturn = true;
                            }
                        }
                        else
                        {
                            // move handle with index nHandleIndex
                            if (nX || nY)
                            {
                                // now move the Handle (nX, nY)
                                Point aStartPoint(pHdl->GetPos());
                                Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                                const SdrDragStat& rDragStat = pView->GetDragStat();

                                // start dragging
                                pView->BegDragObj(aStartPoint, nullptr, pHdl, 0);

                                if(pView->IsDragObj())
                                {
                                    bool bWasNoSnap = rDragStat.IsNoSnap();
                                    bool bWasSnapEnabled = pView->IsSnapEnabled();

                                    // switch snapping off
                                    if(!bWasNoSnap)
                                        const_cast<SdrDragStat&>(rDragStat).SetNoSnap();
                                    if(bWasSnapEnabled)
                                        pView->SetSnapEnabled(false);

                                    pView->MovAction(aEndPoint);
                                    pView->EndDragObj();

                                    // restore snap
                                    if(!bWasNoSnap)
                                        const_cast<SdrDragStat&>(rDragStat).SetNoSnap(bWasNoSnap);
                                    if(bWasSnapEnabled)
                                        pView->SetSnapEnabled(bWasSnapEnabled);
                                }

                                // make moved handle visible
                                tools::Rectangle aVisRect(aEndPoint - Point(100, 100), Size(200, 200));
                                pView->MakeVisible(aVisRect, *pWindow);

                                bReturn = true;
                            }
                        }
                    }
                }
            }
        }
        break;

        case KEY_SPACE:
        {
            // in calc do only something when draw objects are selected
            if(pView->AreObjectsMarked())
            {
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    if(pHdl->GetKind() == SdrHdlKind::Poly)
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

                        if(nullptr == rHdlList.GetFocusHdl())
                        {
                            // restore point with focus
                            SdrHdl* pNewOne = nullptr;

                            for(size_t a = 0; !pNewOne && a < rHdlList.GetHdlCount(); ++a)
                            {
                                SdrHdl* pAct = rHdlList.GetHdl(a);

                                if(pAct
                                    && pAct->GetKind() == SdrHdlKind::Poly
                                    && pAct->GetPolyNum() == nPol
                                    && pAct->GetPointNum() == nPnt)
                                {
                                    pNewOne = pAct;
                                }
                            }

                            if(pNewOne)
                            {
                                const_cast<SdrHdlList&>(rHdlList).SetFocusHdl(pNewOne);
                            }
                        }

                        bReturn = true;
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
            if ( lcl_KeyEditMode( pObj, rViewShell, &rKEvt ) )
                bReturn = true;
        }
    }

    return bReturn;
}

// toggle mouse-pointer
static bool lcl_UrlHit( const SdrView* pView, const Point& rPosPixel, const vcl::Window* pWindow )
{
    SdrViewEvent aVEvt;
    MouseEvent aMEvt( rPosPixel, 1, MouseEventModifiers::NONE, MOUSE_LEFT );
    SdrHitKind eHit = pView->PickAnything( aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt );

    if ( eHit != SdrHitKind::NONE && aVEvt.pObj != nullptr )
    {
        if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) && ScDrawLayer::GetHitIMapObject(
                                aVEvt.pObj, pWindow->PixelToLogic(rPosPixel), *pWindow ) )
            return true;

        if ( aVEvt.eEvent == SdrEventKind::ExecuteUrl )
            return true;
    }

    return false;
}

void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    if ( !pView->IsAction() )
    {
        Point aPosPixel = pWindow->GetPointerPosPixel();
        bool bAlt       = pMEvt && pMEvt->IsMod2();
        Point aPnt      = pWindow->PixelToLogic( aPosPixel );
        SdrHdl* pHdl    = pView->PickHandle(aPnt);
        SdrPageView* pPV;

        ScMacroInfo* pInfo = nullptr;
        SdrObject* pObj = pView->PickObj(aPnt, pView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER);
        if (pObj)
        {
            if ( pObj->IsGroupObject() )
            {
                SdrObject* pHit = pView->PickObj(aMDPos, pView->getHitTolLog(), pPV, SdrSearchOptions::DEEP);
                if (pHit)
                    pObj = pHit;
            }
            pInfo = ScDrawLayer::GetMacroInfo( pObj );
        }

        if ( pView->IsTextEdit() )
        {
            rViewShell.SetActivePointer(PointerStyle::Text);        // can't be ?
        }
        else if ( pHdl )
        {
            rViewShell.SetActivePointer(
                pView->GetPreferredPointer( aPnt, pWindow ) );
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            rViewShell.SetActivePointer( PointerStyle::Move );
        }
        else if ( !bAlt && ( !pMEvt || !pMEvt->GetButtons() )
                        && lcl_UrlHit( pView, aPosPixel, pWindow ) )
        {
            //  could be suppressed with ALT
            pWindow->SetPointer( PointerStyle::RefHand );          // Text-URL / ImageMap
        }
        else if ( !bAlt && (pObj = pView->PickObj(aPnt, pView->getHitTolLog(), pPV, SdrSearchOptions::PICKMACRO)) )
        {
            //  could be suppressed with ALT
            SdrObjMacroHitRec aHitRec;  //! something missing ????
            rViewShell.SetActivePointer( pObj->GetMacroPointer(aHitRec) );
        }
        else if ( !bAlt && pInfo && (!pInfo->GetMacro().isEmpty() || !pInfo->GetHlink().isEmpty()) )
            pWindow->SetPointer( PointerStyle::RefHand );
        else if ( IsDetectiveHit( aPnt ) )
            rViewShell.SetActivePointer( PointerStyle::Detective );
        else
            rViewShell.SetActivePointer( aNewPointer );            //! in Gridwin?
    }
}

bool FuDraw::IsEditingANote() const
{
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    const size_t backval=rMarkList.GetMarkCount();
    for (size_t nlv1=0; nlv1<backval; ++nlv1)
    {
        SdrObject* pObj = rMarkList.GetMark( nlv1 )->GetMarkedSdrObj();
        if ( ScDrawLayer::IsNoteCaption( pObj ) )
        {
            return true;
        }
    }
    return false;
}

bool FuDraw::IsSizingOrMovingNote( const MouseEvent& rMEvt ) const
{
    bool bIsSizingOrMoving = false;
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
