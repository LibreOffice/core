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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/svdoutl.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/sizeitem.hxx>
#include <sfx2/bindings.hxx>
#include <svl/ptitem.hxx>

#include "tabvwsh.hxx"
#include "gridwin.hxx"
#include "dbfunc.hxx"
#include "viewdata.hxx"
#include "output.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"

#include "drawutil.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include <vcl/svapp.hxx>

// -----------------------------------------------------------------------

sal_Bool ScGridWindow::DrawMouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bRet = sal_False;
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        Point aLogicPos = PixelToLogic(rMEvt.GetPosPixel());
        if ( pDraw->IsDetectiveHit( aLogicPos ) )
        {
            //  auf Detektiv-Pfeilen gar nichts (Doppelklick wird bei ButtonUp ausgewertet)
            bRet = sal_True;
        }
        else
        {
            bRet = pDraw->MouseButtonDown( rMEvt );
            if ( bRet )
                UpdateStatusPosSize();
        }
    }

    //  bei rechter Taste Draw-Aktion abbrechen

    ScDrawView* pDrView = pViewData->GetScDrawView();
    if ( pDrView && !rMEvt.IsLeft() && !bRet )
    {
        pDrView->BrkAction();
        bRet = sal_True;
    }
    return bRet;
}

sal_Bool ScGridWindow::DrawMouseButtonUp(const MouseEvent& rMEvt)
{
    ScViewFunc* pView = pViewData->GetView();
    sal_Bool bRet = sal_False;
    FuPoor* pDraw = pView->GetDrawFuncPtr();
    if (pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        bRet = pDraw->MouseButtonUp( rMEvt );

        // execute "format paint brush" for drawing objects
        SfxItemSet* pDrawBrush = pView->GetDrawBrushSet();
        if ( pDrawBrush )
        {
            ScDrawView* pDrView = pViewData->GetScDrawView();
            if ( pDrView )
            {
                sal_Bool bReplaceAll = sal_True;
                pDrView->SetAttrToMarked(*pDrawBrush, bReplaceAll);
            }

            if ( !pView->IsPaintBrushLocked() )
                pView->ResetBrushDocument();        // end paint brush mode if not locked
        }
    }

    return bRet;
}

sal_Bool ScGridWindow::DrawMouseMove(const MouseEvent& rMEvt)
{
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        sal_Bool bRet = pDraw->MouseMove( rMEvt );
        if ( bRet )
            UpdateStatusPosSize();
        return bRet;
    }
    else
    {
        SetPointer( Pointer( POINTER_ARROW ) );
        return sal_False;
    }
}

void ScGridWindow::DrawEndAction()
{
    ScDrawView* pDrView = pViewData->GetScDrawView();
    if ( pDrView && pDrView->IsAction() )
        pDrView->BrkAction();

    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDraw)
        pDraw->StopDragTimer();

    //  ReleaseMouse beim Aufruf
}

sal_Bool ScGridWindow::DrawCommand(const CommandEvent& rCEvt)
{
    ScDrawView* pDrView = pViewData->GetScDrawView();
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDrView && pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        sal_uInt8 nUsed = pDraw->Command( rCEvt );
        if( nUsed == SC_CMD_USED )
            nButtonDown = 0;                    // MouseButtonUp wird verschluckt...
        if( nUsed || pDrView->IsAction() )
            return sal_True;
    }

    return sal_False;
}

sal_Bool ScGridWindow::DrawKeyInput(const KeyEvent& rKEvt)
{
    ScDrawView* pDrView = pViewData->GetScDrawView();
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDrView && pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        sal_Bool bOldMarked = pDrView->AreObjectsMarked();
        if (pDraw->KeyInput( rKEvt ))
        {
            sal_Bool bLeaveDraw = sal_False;
            sal_Bool bUsed = sal_True;
            sal_Bool bNewMarked = pDrView->AreObjectsMarked();
            if ( !pViewData->GetView()->IsDrawSelMode() )
                if ( !bNewMarked )
                {
                    pViewData->GetViewShell()->SetDrawShell( sal_False );
                    bLeaveDraw = sal_True;
                    if ( !bOldMarked &&
                        rKEvt.GetKeyCode().GetCode() == KEY_DELETE )
                        bUsed = sal_False;                  // nichts geloescht
                }
            if (!bLeaveDraw)
                UpdateStatusPosSize();      // #108137# for moving/resizing etc. by keyboard
            return bUsed;
        }
    }

    return sal_False;
}

void ScGridWindow::DrawRedraw( ScOutputData& rOutputData, ScUpdateMode eMode, sal_uLong nLayer )
{
    // #109985#
    const ScViewOptions& rOpts = pViewData->GetOptions();

    // use new flags at SdrPaintView for hiding objects
    const bool bDrawOle(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_OLE));
    const bool bDrawChart(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_CHART));
    const bool bDrawDraw(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_DRAW));

    if(bDrawOle || bDrawChart || bDrawDraw)
    {
        ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();

        if(pDrView)
        {
            pDrView->setHideOle(!bDrawOle);
            pDrView->setHideChart(!bDrawChart);
            pDrView->setHideDraw(!bDrawDraw);
            pDrView->setHideFormControl(!bDrawDraw);
        }

        if(SC_UPDATE_CHANGED == eMode)
        {
            rOutputData.DrawingSingle((sal_uInt16)nLayer);
        }
        else
        {
            rOutputData.DrawSelectiveObjects((sal_uInt16)nLayer);
        }
    }
}

void ScGridWindow::DrawSdrGrid( const Rectangle& rDrawingRect, OutputDevice* pContentDev )
{
    //  Draw-Gitterlinien

    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if ( pDrView && pDrView->IsGridVisible() )
    {
        SdrPageView* pPV = pDrView->GetSdrPageView();
        DBG_ASSERT(pPV, "keine PageView");
        if (pPV)
        {
            pContentDev->SetLineColor(COL_GRAY);

            pPV->DrawPageViewGrid( *pContentDev, rDrawingRect );
        }
    }
}

MapMode ScGridWindow::GetDrawMapMode( sal_Bool bForce )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );

    MapMode aDrawMode = pViewData->GetLogicMode();

    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if ( pDrView || bForce )
    {
        Fraction aScaleX;
        Fraction aScaleY;
        if (pDrView)
            pDrView->GetScale( aScaleX, aScaleY );
        else
        {
            SCCOL nEndCol = 0;
            SCROW nEndRow = 0;
            pDoc->GetTableArea( nTab, nEndCol, nEndRow );
            if (nEndCol<20) nEndCol = 20;
            if (nEndRow<20) nEndRow = 20;
            ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, this,
                                    pViewData->GetZoomX(),pViewData->GetZoomY(),
                                    pViewData->GetPPTX(),pViewData->GetPPTY(),
                                    aScaleX,aScaleY );
        }
        aDrawMode.SetScaleX(aScaleX);
        aDrawMode.SetScaleY(aScaleY);
    }
    aDrawMode.SetOrigin(Point());
    Point aStartPos = pViewData->GetPixPos(eWhich);
    if ( bNegativePage )
    {
        //  RTL uses negative positions for drawing objects
        aStartPos.X() = -aStartPos.X() + GetOutputSizePixel().Width() - 1;
    }
    aDrawMode.SetOrigin( PixelToLogic( aStartPos, aDrawMode ) );

    return aDrawMode;
}

//sal_Bool ScGridWindow::DrawBeforeScroll()
//{
//  ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
//
//  sal_Bool bXor = sal_False;
//  if (pDrView)
//  {
//      bXor=pDrView->IsShownXorVisible(this);
//      if (bXor) pDrView->HideShownXor(this);
//  }
//  return bXor;
//}

void ScGridWindow::DrawAfterScroll(/*sal_Bool bVal*/)
{
    Update();       // immer, damit das Verhalten mit/ohne DrawingLayer gleich ist

    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        //if (bVal)
        //  pDrView->ShowShownXor(this);

        OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            pOlView->ShowCursor(sal_False);                 // ist beim Scrollen weggekommen
    }
}

//void ScGridWindow::DrawMarks()
//{
//  ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
//  if (pDrView)
//      pDrView->DrawMarks(this);
//}

//sal_Bool ScGridWindow::NeedDrawMarks()
//{
//  ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
//  return pDrView && pDrView->IsMarkHdlShown() && pDrView->AreObjectsMarked();
//}

void ScGridWindow::CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress)
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        const ScViewOptions& rOpts = pViewData->GetOptions();
        if(rOpts.GetOption( VOPT_ANCHOR ))
        {
            sal_Bool bNegativePage = pViewData->GetDocument()->IsNegativePage( pViewData->GetTabNo() );
            Point aPos = pViewData->GetScrPos( rAddress.Col(), rAddress.Row(), eWhich, sal_True );
            aPos = PixelToLogic(aPos);
            rHdl.AddHdl(new SdrHdl(aPos, bNegativePage ? HDL_ANCHOR_TR : HDL_ANCHOR));
        }
    }
}

SdrObject* ScGridWindow::GetEditObject()
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            return pDrView->GetTextEditObject();
    }

    return NULL;
}

void ScGridWindow::UpdateStatusPosSize()
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (!pDrView)
        return;         // shouldn't be called in that case

    SdrPageView* pPV = pDrView->GetSdrPageView();
    if (!pPV)
        return;         // shouldn't be called in that case either

    SfxItemSet aSet(pViewData->GetViewShell()->GetPool(), SID_ATTR_POSITION, SID_ATTR_SIZE);

    //  Fill items for position and size:
    //  #108137# show action rectangle during action,
    //  position and size of selected object(s) if something is selected,
    //  mouse position otherwise

    sal_Bool bActionItem = sal_False;
    if ( pDrView->IsAction() )              // action rectangle
    {
        Rectangle aRect;
        pDrView->TakeActionRect( aRect );
        if ( !aRect.IsEmpty() )
        {
            pPV->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() ) ) );
            bActionItem = sal_True;
        }
    }
    if ( !bActionItem )
    {
        if ( pDrView->AreObjectsMarked() )      // selected objects
        {
            Rectangle aRect = pDrView->GetAllMarkedRect();
            pPV->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() ) ) );
        }
        else                                // mouse position
        {
            Point aPos = PixelToLogic(aCurMousePos);
            pPV->LogicToPagePos(aPos);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
        }
    }

    pViewData->GetBindings().SetState(aSet);
}

sal_Bool ScGridWindow::DrawHasMarkedObj()
{
    ScDrawView* p = pViewData->GetScDrawView();
    return p ? p->AreObjectsMarked() : sal_False;
}

//void ScGridWindow::DrawStartTimer()
//{
    //ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    //if (pDrView)
    //{
        /* jetzt in DrawMarks
        sal_uInt16 nWinNum = pDrView->FindWin(this);
        if (nWinNum!=SDRVIEWWIN_NOTFOUND)
            pDrView->AfterInitRedraw(nWinNum);
        */

        // pDrView->PostPaint();
        // pDrView->RestartAfterPaintTimer();
    //}
//}

void ScGridWindow::DrawMarkDropObj( SdrObject* pObj )
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
        pDrView->MarkDropObj(pObj);
}



