/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <editeng/editeng.hxx>
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
    
    
    

    sal_Bool bShift = rMEvt.IsShift();
    sal_Bool bAlt   = rMEvt.IsMod2();

    bool bOrtho     = bShift;
    sal_Bool bAngleSnap = bShift;
    sal_Bool bCenter    = bAlt;

    
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

bool FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    DoModifiers( rMEvt );
    return false;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    
    
    if (pView->IsAction())
        DoModifiers( rMEvt );

    return false;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    
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
        
        

        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
        sal_Bool bVertical = ( pOPO && pOPO->IsVertical() );
        sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

        
        FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
        if ( !pPoor || pPoor->GetSlotID() != nTextSlotId )
        {
            pViewShell->GetViewData()->GetDispatcher().
                Execute(nTextSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
        }

        
        pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
        if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    
        {
            FuText* pText = (FuText*)pPoor;
            pText->SetInEditMode( pObj, NULL, true, pInitialKey );
            
        }
        bReturn = sal_True;
    }
    return bReturn;
}

bool FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;
    ScViewData& rViewData = *pViewShell->GetViewData();

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( pViewShell->IsDrawTextShell() || aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
            {
                
                rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                bReturn = true;
            }
            else if ( pViewShell->IsDrawSelMode() )
            {
                pView->UnmarkAll();
                rViewData.GetDispatcher().Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                bReturn = true;
            }
            else if ( pView->AreObjectsMarked() )
            {
                
                SdrHdlList& rHdlList = const_cast< SdrHdlList& >( pView->GetHdlList() );
                if( rHdlList.GetFocusHdl() )
                    rHdlList.ResetFocusHdl();
                else
                    pView->UnmarkAll();

                
                if (!pView->AreObjectsMarked())
                    pViewShell->SetDrawShell( false );

                bReturn = true;
            }
            break;

        case KEY_DELETE:                    
            pView->DeleteMarked();
            bReturn = true;
        break;

        case KEY_RETURN:
        {
            if( rKEvt.GetKeyCode().GetModifier() == 0 )
            {
                
                
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    sal_Bool bOle = pViewShell->GetViewFrame()->GetFrame().IsInPlace();
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( pObj && pObj->ISA( SdrOle2Obj ) && !bOle )
                    {
                        pViewShell->ActivateObject( static_cast< SdrOle2Obj* >( pObj ), 0 );

                        
                        bReturn = true;
                    }
                    else if ( lcl_KeyEditMode( pObj, pViewShell, NULL ) )       
                        bReturn = true;
                }
            }
        }
        break;

        case KEY_F2:
        {
            if( rKEvt.GetKeyCode().GetModifier() == 0 )
            {
                
                
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if ( lcl_KeyEditMode( pObj, pViewShell, NULL ) )            
                        bReturn = true;
                }
            }
        }
        break;

        
        case KEY_TAB:
        {
            
            
            if(pView->AreObjectsMarked())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( !aCode.IsMod1() && !aCode.IsMod2() )
                {
                    
                    if(!pView->MarkNextObj( !aCode.IsShift() ))
                    {
                        
                        if ( pView->GetMarkableObjCount() > 1 && pView->HasMarkableObj() )
                        {
                            
                            
                            pView->UnmarkAllObj();
                            pView->MarkNextObj(!aCode.IsShift());
                        }
                    }

                    
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = true;
                }

                
                if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
                {
                    
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

                    ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                    
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        Point aHdlPosition(pHdl->GetPos());
                        Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                        pView->MakeVisible(aVisRect, *pWindow);
                    }

                    
                    bReturn = true;
                }
            }
        }
        break;

        
        case KEY_END:
        {
            
            
            if(pView->AreObjectsMarked())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(false);

                    
                    if(pView->AreObjectsMarked())
                        pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                    bReturn = true;
                }
            }
        }
        break;

        
        case KEY_HOME:
        {
            
            
            if(pView->AreObjectsMarked())
            {
                KeyCode aCode = rKEvt.GetKeyCode();

                if ( aCode.IsMod1() )
                {
                    
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(sal_True);

                    
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
            
            
            if(pView->AreObjectsMarked())
            {

                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if(rMarkList.GetMarkCount() == 1)
                {
                    
                    
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( ScDrawLayer::IsNoteCaption( pObj ) )
                        break;
                }

                long nX = 0;
                long nY = 0;
                sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

                if (nCode == KEY_UP)
                {
                    
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    
                    nX = 1;
                    nY = 0;
                }

                sal_Bool bReadOnly = rViewData.GetDocShell()->IsReadOnly();

                if(!rKEvt.GetKeyCode().IsMod1() && !bReadOnly)
                {
                    if(rKEvt.GetKeyCode().IsMod2())
                    {
                        
                        Size aLogicSizeOnePixel = (pWindow) ? pWindow->PixelToLogic(Size(1,1)) : Size(100, 100);
                        nX *= aLogicSizeOnePixel.Width();
                        nY *= aLogicSizeOnePixel.Height();
                    }
                    else if(rKEvt.GetKeyCode().IsShift()) 
                    {
                        nX *= 1000;
                        nY *= 1000;
                    }
                    else
                    {
                        
                        nX *= 100;
                        nY *= 100;
                    }

                    
                    if(0 != nX || 0 != nY)
                    {
                        
                        const SdrHdlList& rHdlList = pView->GetHdlList();
                        SdrHdl* pHdl = rHdlList.GetFocusHdl();

                        if(0L == pHdl)
                        {
                            
                            if(pView->IsMoveAllowed())
                            {
                                
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

                                
                                pView->MoveAllMarked(Size(nX, nY));

                                
                                pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                                bReturn = true;
                            }
                        }
                        else
                        {
                            
                            if(pHdl && (nX || nY))
                            {
                                
                                Point aStartPoint(pHdl->GetPos());
                                Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                                const SdrDragStat& rDragStat = pView->GetDragStat();

                                
                                pView->BegDragObj(aStartPoint, 0, pHdl, 0);

                                if(pView->IsDragObj())
                                {
                                    bool bWasNoSnap = rDragStat.IsNoSnap();
                                    sal_Bool bWasSnapEnabled = pView->IsSnapEnabled();

                                    
                                    if(!bWasNoSnap)
                                        ((SdrDragStat&)rDragStat).SetNoSnap(true);
                                    if(bWasSnapEnabled)
                                        pView->SetSnapEnabled(false);

                                    pView->MovAction(aEndPoint);
                                    pView->EndDragObj();

                                    
                                    if(!bWasNoSnap)
                                        ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                                    if(bWasSnapEnabled)
                                        pView->SetSnapEnabled(bWasSnapEnabled);
                                }

                                
                                Rectangle aVisRect(aEndPoint - Point(100, 100), Size(200, 200));
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
            
            if(pView->AreObjectsMarked())
            {
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    if(pHdl->GetKind() == HDL_POLY)
                    {
                        
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
        

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() && EditEngine::IsSimpleCharInput(rKEvt) )
        {
            SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

            
            if ( lcl_KeyEditMode( pObj, pViewShell, &rKEvt ) )
                bReturn = true;
        }
    }

    return bReturn;
}


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
            pViewShell->SetActivePointer(Pointer(POINTER_TEXT));        
        }
        else if ( pHdl )
        {
            pViewShell->SetActivePointer(
                pView->GetPreferredPointer( aPnt, pWindow ) );
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            pViewShell->SetActivePointer( Pointer(POINTER_MOVE) );
        }
        else if ( !bAlt && ( !pMEvt || !pMEvt->GetButtons() )
                        && lcl_UrlHit( pView, aPosPixel, pWindow ) )
        {
            
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );          
        }
        else if ( !bAlt && pView->PickObj(aPnt, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMACRO) )
        {
            
            SdrObjMacroHitRec aHitRec;  
            pViewShell->SetActivePointer( pObj->GetMacroPointer(aHitRec) );
        }
        else if ( !bAlt && pInfo && (!pInfo->GetMacro().isEmpty() || !pInfo->GetHlink().isEmpty()) )
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );
        else if ( IsDetectiveHit( aPnt ) )
            pViewShell->SetActivePointer( Pointer( POINTER_DETECTIVE ) );
        else
            pViewShell->SetActivePointer( aNewPointer );            
    }
}

bool FuDraw::IsEditingANote() const
{
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    sal_Int32 backval=rMarkList.GetMarkCount();
    for (sal_Int32 nlv1=0;nlv1<backval;nlv1++)
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
                    pView->PickHandle( aMPos ) ||      
                    pView->IsTextEditFrameHit( aMPos );         
            }
        }
    }
    return bIsSizingOrMoving;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
