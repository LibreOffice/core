/*************************************************************************
 *
 *  $RCSfile: gridwin3.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 10:15:02 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/svdoutl.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sizeitem.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/ptitem.hxx>

#include "tabvwsh.hxx"
#include "gridwin.hxx"
#include "dbfunc.hxx"
#include "viewdata.hxx"
#include "output.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"

#include "drawutil.hxx"
#include "document.hxx"


// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------


BOOL ScGridWindow::DrawMouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bRet = FALSE;
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        Point aLogicPos = PixelToLogic(rMEvt.GetPosPixel());
        if ( pDraw->IsDetectiveHit( aLogicPos ) )
        {
            //  auf Detektiv-Pfeilen gar nichts (Doppelklick wird bei ButtonUp ausgewertet)
            bRet = TRUE;
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
        bRet = TRUE;
    }
    return bRet;
}

BOOL ScGridWindow::DrawMouseButtonUp(const MouseEvent& rMEvt)
{
    ScViewFunc* pView = pViewData->GetView();
    BOOL bRet = FALSE;
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
                BOOL bReplaceAll = TRUE;
                pDrView->SetAttrToMarked(*pDrawBrush, bReplaceAll);
            }

            if ( !pView->IsPaintBrushLocked() )
                pView->ResetBrushDocument();        // end paint brush mode if not locked
        }
    }

    return bRet;
}

BOOL ScGridWindow::DrawMouseMove(const MouseEvent& rMEvt)
{
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        BOOL bRet = pDraw->MouseMove( rMEvt );
        if ( bRet )
            UpdateStatusPosSize();
        return bRet;
    }
    else
    {
        SetPointer( Pointer( POINTER_ARROW ) );
        return FALSE;
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

BOOL ScGridWindow::DrawCommand(const CommandEvent& rCEvt)
{
    ScDrawView* pDrView = pViewData->GetScDrawView();
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDrView && pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        BYTE nUsed = pDraw->Command( rCEvt );
        if( nUsed == SC_CMD_USED )
            nButtonDown = 0;                    // MouseButtonUp wird verschluckt...
        if( nUsed || pDrView->IsAction() )
            return TRUE;
    }

    return FALSE;
}

BOOL ScGridWindow::DrawKeyInput(const KeyEvent& rKEvt)
{
    ScDrawView* pDrView = pViewData->GetScDrawView();
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDrView && pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        BOOL bOldMarked = pDrView->AreObjectsMarked();
        if (pDraw->KeyInput( rKEvt ))
        {
            BOOL bLeaveDraw = FALSE;
            BOOL bUsed = TRUE;
            BOOL bNewMarked = pDrView->AreObjectsMarked();
            if ( !pViewData->GetView()->IsDrawSelMode() )
                if ( !bNewMarked )
                {
                    pViewData->GetViewShell()->SetDrawShell( FALSE );
                    bLeaveDraw = TRUE;
                    if ( !bOldMarked &&
                        rKEvt.GetKeyCode().GetCode() == KEY_DELETE )
                        bUsed = FALSE;                  // nichts geloescht
                }
            if (!bLeaveDraw)
                UpdateStatusPosSize();      // #108137# for moving/resizing etc. by keyboard
            return bUsed;
        }
    }

    return FALSE;
}

void ScGridWindow::DrawRedraw( ScOutputData& rOutputData, const Rectangle& rDrawingRect,
                                    ScUpdateMode eMode, ULONG nLayer )
{
    // #109985#
    sal_uInt16 nPaintMode(0);
    sal_Bool bDrawAtAll(sal_False);
    const ScViewOptions& rOpts = pViewData->GetOptions();

    if(VOBJ_MODE_DUMMY == rOpts.GetObjMode(VOBJ_TYPE_OLE))
    {
        nPaintMode |= SDRPAINTMODE_SC_DRAFT_OLE;
        bDrawAtAll = sal_True;
    }
    else if(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_OLE))
    {
        bDrawAtAll = sal_True;
    }
    else
    {
        nPaintMode |= SDRPAINTMODE_SC_HIDE_OLE;
    }

    if(VOBJ_MODE_DUMMY == rOpts.GetObjMode(VOBJ_TYPE_CHART))
    {
        nPaintMode |= SDRPAINTMODE_SC_DRAFT_CHART;
        bDrawAtAll = sal_True;
    }
    else if(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_CHART))
    {
        bDrawAtAll = sal_True;
    }
    else
    {
        nPaintMode |= SDRPAINTMODE_SC_HIDE_CHART;
    }

    if(VOBJ_MODE_DUMMY == rOpts.GetObjMode(VOBJ_TYPE_DRAW))
    {
        nPaintMode |= SDRPAINTMODE_SC_DRAFT_DRAW;
        bDrawAtAll = sal_True;
    }
    else if(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_DRAW))
    {
        bDrawAtAll = sal_True;
    }
    else
    {
        nPaintMode |= SDRPAINTMODE_SC_HIDE_DRAW;
    }

    if(bDrawAtAll)
    {
        if(SC_UPDATE_CHANGED == eMode)
        {
            rOutputData.DrawingSingle((sal_uInt16)nLayer, nPaintMode);
        }
        else
        {
            rOutputData.DrawSelectiveObjects((sal_uInt16)nLayer, rDrawingRect, nPaintMode);
        }
    }
}

void ScGridWindow::DrawSdrGrid( const Rectangle& rDrawingRect )
{
    //  Draw-Gitterlinien

    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if ( pDrView && pDrView->IsGridVisible() )
    {
        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
        DBG_ASSERT(pPV, "keine PageView");
        if (pPV)
        {
            SetLineColor(COL_GRAY);

            pPV->DrawGrid( *this, rDrawingRect );
        }
    }
}

MapMode ScGridWindow::GetDrawMapMode( BOOL bForce )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    BOOL bNegativePage = pDoc->IsNegativePage( nTab );

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

BOOL ScGridWindow::DrawBeforeScroll()
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();

    BOOL bXor = FALSE;
    if (pDrView)
    {
        bXor=pDrView->IsShownXorVisible(this);
        if (bXor) pDrView->HideShownXor(this);
    }
    return bXor;
}

void ScGridWindow::DrawAfterScroll(BOOL bVal)
{
    Update();       // immer, damit das Verhalten mit/ohne DrawingLayer gleich ist

    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        if (bVal)
            pDrView->ShowShownXor(this);

        OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            pOlView->ShowCursor(FALSE);                 // ist beim Scrollen weggekommen
    }
}

void ScGridWindow::DrawMarks()
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
        pDrView->DrawMarks(this);
}

BOOL ScGridWindow::NeedDrawMarks()
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    return pDrView && pDrView->IsMarkHdlShown() && pDrView->AreObjectsMarked();
}

void ScGridWindow::CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress)
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        const ScViewOptions& rOpts = pViewData->GetOptions();
        if(rOpts.GetOption( VOPT_ANCHOR ))
        {
            BOOL bNegativePage = pViewData->GetDocument()->IsNegativePage( pViewData->GetTabNo() );
            Point aPos = pViewData->GetScrPos( rAddress.Col(), rAddress.Row(), eWhich, TRUE );
            aPos = PixelToLogic(aPos);
            rHdl.AddHdl(new SdrHdl(aPos, bNegativePage ? HDL_ANCHOR_TR : HDL_ANCHOR));
        }
    }
}

void ScGridWindow::OutlinerViewPaint( const Rectangle& rRect )
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView && pDrView->GetTextEditOutlinerView())
    {
        Outliner* pOut = pDrView->GetTextEditOutliner();
        ULONG nViewCount = pOut ? pOut->GetViewCount() : 0;
        for (ULONG nView=0; nView<nViewCount; nView++)
        {
            OutlinerView* pOlView = pOut->GetView(nView);

            if (pOlView && pOlView->GetWindow() == this)
            {
                BOOL bFrame = FALSE;
                Rectangle aEditRect = pOlView->GetOutputArea();
                SdrObject* pEditObj = GetEditObject();
                if (pEditObj && pEditObj->ISA(SdrTextObj))
                {
                    SdrTextObj* pTextObj = (SdrTextObj*)pEditObj;
                    Rectangle aMinRect;
                    pTextObj->TakeTextEditArea(NULL,NULL,NULL,&aMinRect); // PageView Offset ist 0
                    aEditRect.Union(aMinRect);
                    bFrame = pTextObj->IsTextFrame();
                }

                if ( aEditRect.IsOver( rRect ) )
                {
                    ULONG nOldDrawMode = GetDrawMode();
                    if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                    {
                        SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
                    }

                    Rectangle aEffRect = rRect.GetIntersection( aEditRect );
                    if (bFrame)
                    {
                        if ( pEditObj->GetLayer() ==SC_LAYER_INTERN )   // cell note?
                        {
                            //  #78828# always draw the edit background in case notes overlap
                            SetFillColor( pOlView->GetBackgroundColor() );
                            SetLineColor();
                            DrawRect( aEffRect );
                        }

                        //  DrawLayer mit dem Text-Rechteck zeichnet nur die Outliner-View
                        //  und den Text-Rahmen (an den kommt man sonst von aussen nicht heran).

                        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
                        DBG_ASSERT(pPV, "keine PageView");
                        if (pPV)
                        {
                            SdrLayerID nLayer = pEditObj ? pEditObj->GetLayer() : SC_LAYER_FRONT;
                            pPV->DrawLayer( nLayer, aEffRect, this );
                        }
                    }
                    else
                    {
                        SetFillColor( pOlView->GetBackgroundColor() );
                        SetLineColor();
                        DrawRect( aEffRect );
                        pOlView->Paint( aEffRect );
                    }

                    SetDrawMode( nOldDrawMode );
                }
            }
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

    SfxItemSet aSet(pViewData->GetViewShell()->GetPool(), SID_ATTR_POSITION, SID_ATTR_SIZE);

    //  Fill items for position and size:
    //  #108137# show action rectangle during action,
    //  position and size of selected object(s) if something is selected,
    //  mouse position otherwise

    BOOL bActionItem = FALSE;
    if ( pDrView->IsAction() )              // action rectangle
    {
        Rectangle aRect;
        pDrView->TakeActionRect( aRect );
        if ( !aRect.IsEmpty() )
        {
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() ) ) );
            bActionItem = TRUE;
        }
    }
    if ( !bActionItem )
    {
        if ( pDrView->AreObjectsMarked() )      // selected objects
        {
            Rectangle aRect = pDrView->GetAllMarkedRect();
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() ) ) );
        }
        else                                // mouse position
        {
            Point aPos = PixelToLogic(aCurMousePos);
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aPos);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
        }
    }

    pViewData->GetBindings().SetState(aSet);
}

BOOL ScGridWindow::DrawHasMarkedObj()
{
    ScDrawView* p = pViewData->GetScDrawView();
    return p ? p->AreObjectsMarked() : FALSE;
}

void ScGridWindow::DrawStartTimer()
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        /* jetzt in DrawMarks
        USHORT nWinNum = pDrView->FindWin(this);
        if (nWinNum!=SDRVIEWWIN_NOTFOUND)
            pDrView->AfterInitRedraw(nWinNum);
        */

        // pDrView->PostPaint();
        pDrView->RestartAfterPaintTimer();
    }
}

void ScGridWindow::DrawMarkDropObj( SdrObject* pObj )
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
        pDrView->MarkDropObj(pObj);
}



