/*************************************************************************
 *
 *  $RCSfile: gridwin3.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:36:22 $
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
    BOOL bRet = FALSE;
    FuPoor* pDraw = pViewData->GetView()->GetDrawFuncPtr();
    if (pDraw && !pViewData->IsRefMode())
    {
        pDraw->SetWindow( this );
        bRet = pDraw->MouseButtonUp( rMEvt );
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
        BOOL bOldMarked = pDrView->HasMarkedObj();
        if (pDraw->KeyInput( rKEvt ))
        {
            BOOL bUsed = TRUE;
            BOOL bNewMarked = pDrView->HasMarkedObj();
            if ( !pViewData->GetView()->IsDrawSelMode() )
                if ( !bNewMarked )
                {
                    pViewData->GetViewShell()->SetDrawShell( FALSE );
                    if ( !bOldMarked &&
                        rKEvt.GetKeyCode().GetCode() == KEY_DELETE )
                        bUsed = FALSE;                  // nichts geloescht
                }
            return bUsed;
        }
    }

    return FALSE;
}

void ScGridWindow::DrawRedraw( ScOutputData& rOutputData, const Rectangle& rDrawingRect,
                                    ScUpdateMode eMode, ULONG nLayer )
{
    //!     eMode auswerten !!!

    USHORT nObjectFlags = 0;
    const ScViewOptions& rOpts = pViewData->GetOptions();
    if ( rOpts.GetObjMode( VOBJ_TYPE_OLE ) == VOBJ_MODE_SHOW )
        nObjectFlags |= SC_OBJECTS_OLE;
    if ( rOpts.GetObjMode( VOBJ_TYPE_CHART ) == VOBJ_MODE_SHOW )
        nObjectFlags |= SC_OBJECTS_CHARTS;
    if ( rOpts.GetObjMode( VOBJ_TYPE_DRAW ) == VOBJ_MODE_SHOW )
        nObjectFlags |= SC_OBJECTS_DRAWING;

    USHORT nDummyFlags = 0;
    if ( rOpts.GetObjMode( VOBJ_TYPE_OLE ) == VOBJ_MODE_DUMMY )
        nDummyFlags |=  SC_OBJECTS_OLE;
    if ( rOpts.GetObjMode( VOBJ_TYPE_CHART ) == VOBJ_MODE_DUMMY )
        nDummyFlags |=  SC_OBJECTS_CHARTS;
    if ( rOpts.GetObjMode( VOBJ_TYPE_DRAW ) == VOBJ_MODE_DUMMY )
        nDummyFlags |=  SC_OBJECTS_DRAWING;

    if (nObjectFlags || nDummyFlags)
    {
        if ( eMode == SC_UPDATE_CHANGED )
            rOutputData.DrawingSingle( (USHORT) nLayer, nObjectFlags, nDummyFlags );
        else
            rOutputData.DrawSelectiveObjects( (USHORT) nLayer, rDrawingRect,
                                                nObjectFlags, nDummyFlags );
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
            ScDocument* pDoc = pViewData->GetDocument();
            USHORT nTab = pViewData->GetTabNo();
            USHORT nEndCol = 0;
            USHORT nEndRow = 0;
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
    aDrawMode.SetOrigin( PixelToLogic( pViewData->GetPixPos(eWhich), aDrawMode ) );

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
    return pDrView && pDrView->IsMarkHdlShown() && pDrView->HasMarkedObj();
}

void ScGridWindow::CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress)
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
    {
        const ScViewOptions& rOpts = pViewData->GetOptions();
        if(rOpts.GetOption( VOPT_ANCHOR ))
        {
            Point aPos = pViewData->GetScrPos( rAddress.Col(), rAddress.Row(), eWhich, TRUE );
            aPos = PixelToLogic(aPos);
            rHdl.AddHdl(new SdrHdl(aPos ,HDL_ANCHOR));
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
                    Rectangle aEffRect = rRect.GetIntersection( aEditRect );
                    if (bFrame)
                    {
                        //  RedrawOneLayer mit dem Text-Rechteck zeichnet nur die Outliner-View
                        //  und den Text-Rahmen (an den kommt man sonst von aussen nicht heran).

                        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
                        DBG_ASSERT(pPV, "keine PageView");
                        if (pPV)
                        {
                            SdrLayerID nLayer = pEditObj ? pEditObj->GetLayer() : SC_LAYER_FRONT;
                            pPV->RedrawOneLayer( nLayer, aEffRect, this );
                        }
                    }
                    else
                    {
                        SetFillColor( pOlView->GetBackgroundColor() );
                        SetLineColor();
                        DrawRect( aEffRect );
                        pOlView->Paint( aEffRect );
                    }
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
        return;         // sollte dann gar nicht gerufen werden

    Point aPos = PixelToLogic(aCurMousePos);
    pDrView->GetPageViewPvNum(0)->LogicToPagePos(aPos);

    SfxItemSet aSet(pViewData->GetViewShell()->GetPool(), SID_ATTR_POSITION, SID_ATTR_SIZE);

    // Position- und Groesse-Items
    if ( pDrView->IsAction() )
    {
        Rectangle aRect;
        pDrView->TakeActionRect( aRect );

        if ( aRect.IsEmpty() )
            aSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );
        else
        {
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(),
                            aRect.Bottom() - aRect.Top() ) ) );
        }
    }
    else
    {
        aSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );

        if ( pDrView->HasMarkedObj() )
        {
            Rectangle aRect = pDrView->GetAllMarkedRect();
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(),
                            aRect.Bottom() - aRect.Top()) ) );
        }
        else
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
    }

    pViewData->GetBindings().SetState(aSet);
}

BOOL ScGridWindow::DrawHasMarkedObj()
{
    ScDrawView* p = pViewData->GetScDrawView();
    return p ? p->HasMarkedObj() : FALSE;
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

        pDrView->PostPaint();
        pDrView->RestartAfterPaintTimer();
    }
}

void ScGridWindow::DrawMarkDropObj( SdrObject* pObj )
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
        pDrView->MarkDropObj(pObj);
}



