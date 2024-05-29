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

#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/sizeitem.hxx>
#include <sfx2/bindings.hxx>
#include <svl/ptitem.hxx>
#include <osl/diagnose.h>

#include <tabvwsh.hxx>
#include <gridwin.hxx>
#include <dbfunc.hxx>
#include <viewdata.hxx>
#include <output.hxx>
#include <drawview.hxx>
#include <fupoor.hxx>

#include <drawutil.hxx>
#include <document.hxx>
#include <comphelper/lok.hxx>

bool ScGridWindow::DrawMouseButtonDown(const MouseEvent& rMEvt)
{
    bool bRet = false;
    FuPoor* pDraw = mrViewData.GetView()->GetDrawFuncPtr();
    if (pDraw && !mrViewData.IsRefMode())
    {
        MapMode aDrawMode = GetDrawMapMode();
        MapMode aOldMode = GetMapMode();
        if ( comphelper::LibreOfficeKit::isActive() && aOldMode != aDrawMode )
            SetMapMode( aDrawMode );

        pDraw->SetWindow( this );
        Point aLogicPos = PixelToLogic(rMEvt.GetPosPixel());
        if ( pDraw->IsDetectiveHit( aLogicPos ) )
        {
            // nothing on detective arrows (double click is evaluated on ButtonUp)
            bRet = true;
        }
        else
        {
            bRet = pDraw->MouseButtonDown( rMEvt );
            if ( bRet )
                UpdateStatusPosSize();
        }

        if ( comphelper::LibreOfficeKit::isActive() && aOldMode != aDrawMode )
            SetMapMode( aOldMode );
    }

    // cancel draw with right key
    ScDrawView* pDrView = mrViewData.GetScDrawView();
    if ( pDrView && !rMEvt.IsLeft() && !bRet )
    {
        pDrView->BrkAction();
        bRet = true;
    }
    return bRet;
}

bool ScGridWindow::DrawMouseButtonUp(const MouseEvent& rMEvt)
{
    ScViewFunc* pView = mrViewData.GetView();
    bool bRet = false;
    FuPoor* pDraw = pView->GetDrawFuncPtr();
    if (pDraw && !mrViewData.IsRefMode())
    {
        MapMode aDrawMode = GetDrawMapMode();
        MapMode aOldMode = GetMapMode();
        if ( comphelper::LibreOfficeKit::isActive() && aOldMode != aDrawMode )
            SetMapMode( aDrawMode );

        pDraw->SetWindow( this );
        bRet = pDraw->MouseButtonUp( rMEvt );

        // execute "format paint brush" for drawing objects
        SfxItemSet* pDrawBrush = pView->GetDrawBrushSet();
        if ( pDrawBrush )
        {
            ScDrawView* pDrView = mrViewData.GetScDrawView();
            if ( pDrView )
            {
                pDrView->SetAttrToMarked(*pDrawBrush, true/*bReplaceAll*/);
            }

            if ( !pView->IsPaintBrushLocked() )
                pView->ResetBrushDocument(); // end paint brush mode if not locked
        }

        if ( comphelper::LibreOfficeKit::isActive() && aOldMode != aDrawMode )
            SetMapMode( aOldMode );
    }

    return bRet;
}

bool ScGridWindow::DrawMouseMove(const MouseEvent& rMEvt)
{
    FuPoor* pDraw = mrViewData.GetView()->GetDrawFuncPtr();
    if (pDraw && !mrViewData.IsRefMode())
    {
        MapMode aDrawMode = GetDrawMapMode();
        MapMode aOldMode = GetMapMode();
        if ( comphelper::LibreOfficeKit::isActive() && aOldMode != aDrawMode )
            SetMapMode( aDrawMode );

        pDraw->SetWindow( this );
        bool bRet = pDraw->MouseMove( rMEvt );
        if ( bRet )
            UpdateStatusPosSize();

        if ( comphelper::LibreOfficeKit::isActive() && aOldMode != aDrawMode )
            SetMapMode( aOldMode );

        return bRet;
    }
    else
    {
        SetPointer( PointerStyle::Arrow );
        return false;
    }
}

void ScGridWindow::DrawEndAction()
{
    ScDrawView* pDrView = mrViewData.GetScDrawView();
    if ( pDrView && pDrView->IsAction() )
        pDrView->BrkAction();

    FuPoor* pDraw = mrViewData.GetView()->GetDrawFuncPtr();
    if (pDraw)
        pDraw->StopDragTimer();

    //  ReleaseMouse on call
}

bool ScGridWindow::DrawCommand(const CommandEvent& rCEvt)
{
    ScDrawView* pDrView = mrViewData.GetScDrawView();
    FuPoor* pDraw = mrViewData.GetView()->GetDrawFuncPtr();
    if (pDrView && pDraw && !mrViewData.IsRefMode())
    {
        pDraw->SetWindow( this );
        sal_uInt8 nUsed = pDraw->Command( rCEvt );
        if( nUsed == SC_CMD_USED )
            nButtonDown = 0;  // MouseButtonUp is swallowed...
        if( nUsed || pDrView->IsAction() )
            return true;
    }

    return false;
}

bool ScGridWindow::DrawKeyInput(const KeyEvent& rKEvt, vcl::Window* pWin)
{
    ScDrawView* pDrView = mrViewData.GetScDrawView();
    FuPoor* pDraw = mrViewData.GetView()->GetDrawFuncPtr();


    if (pDrView && pDrView->KeyInput(rKEvt, pWin))
        return true;

    if (pDrView && pDraw && !mrViewData.IsRefMode())
    {
        pDraw->SetWindow( this );
        bool bOldMarked = pDrView->GetMarkedObjectList().GetMarkCount() != 0;
        if (pDraw->KeyInput( rKEvt ))
        {
            bool bLeaveDraw = false;
            bool bUsed = true;
            bool bNewMarked = pDrView->GetMarkedObjectList().GetMarkCount() != 0;
            if ( !mrViewData.GetView()->IsDrawSelMode() )
                if ( !bNewMarked )
                {
                    mrViewData.GetViewShell()->SetDrawShell( false );
                    bLeaveDraw = true;
                    if ( !bOldMarked &&
                        rKEvt.GetKeyCode().GetCode() == KEY_DELETE )
                        bUsed = false; // nothing deleted
                    if(bOldMarked)
                        GetFocus();
                }
            if (!bLeaveDraw)
                UpdateStatusPosSize(); // for moving/resizing etc. by keyboard
            return bUsed;
        }
    }

    return false;
}

void ScGridWindow::DrawRedraw( ScOutputData& rOutputData, SdrLayerID nLayer )
{
    const ScViewOptions& rOpts = mrViewData.GetOptions();

    // use new flags at SdrPaintView for hiding objects
    const bool bDrawOle(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_OLE));
    const bool bDrawChart(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_CHART));
    const bool bDrawDraw(VOBJ_MODE_SHOW == rOpts.GetObjMode(VOBJ_TYPE_DRAW));

    if(!(bDrawOle || bDrawChart || bDrawDraw))
        return;

    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();

    if(pDrView)
    {
        pDrView->setHideOle(!bDrawOle);
        pDrView->setHideChart(!bDrawChart);
        pDrView->setHideDraw(!bDrawDraw);
        pDrView->setHideFormControl(!bDrawDraw);
    }

    rOutputData.DrawSelectiveObjects(nLayer);
}

void ScGridWindow::DrawSdrGrid( const tools::Rectangle& rDrawingRect, OutputDevice* pContentDev )
{
    // Draw grid lines

    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();
    if ( pDrView && pDrView->IsGridVisible() )
    {
        SdrPageView* pPV = pDrView->GetSdrPageView();
        OSL_ENSURE(pPV, "PageView not available");
        if (pPV)
        {
            pContentDev->SetLineColor(COL_GRAY);

            pPV->DrawPageViewGrid( *pContentDev, rDrawingRect );
        }
    }
}

MapMode ScGridWindow::GetDrawMapMode( bool bForce )
{
    ScDocument& rDoc = mrViewData.GetDocument();

    // FIXME this shouldn't be necessary once we change the entire Calc to
    // work in the logic coordinates (ideally 100ths of mm - so that it is
    // the same as editeng and drawinglayer), and get rid of all the
    // SetMapMode's and other unnecessary fun we have with pixels
    if (comphelper::LibreOfficeKit::isActive())
    {
        return mrViewData.GetLogicMode();
    }

    SCTAB nTab = mrViewData.GetTabNo();
    bool bNegativePage = rDoc.IsNegativePage( nTab );

    MapMode aDrawMode = mrViewData.GetLogicMode();

    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();
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
            rDoc.GetTableArea( nTab, nEndCol, nEndRow );
            if (nEndCol<20) nEndCol = 20;
            if (nEndRow<20) nEndRow = 1000;
            ScDrawUtil::CalcScale( rDoc, nTab, 0,0, nEndCol,nEndRow, GetOutDev(),
                                   mrViewData.GetZoomX(),mrViewData.GetZoomY(),
                                   mrViewData.GetPPTX(),mrViewData.GetPPTY(),
                                   aScaleX,aScaleY );
        }
        aDrawMode.SetScaleX(aScaleX);
        aDrawMode.SetScaleY(aScaleY);
    }
    aDrawMode.SetOrigin(Point());
    Point aStartPos = mrViewData.GetPixPos(eWhich);
    if ( bNegativePage )
    {
        //  RTL uses negative positions for drawing objects
        aStartPos.setX( -aStartPos.X() + GetOutputSizePixel().Width() - 1 );
    }
    aDrawMode.SetOrigin( PixelToLogic( aStartPos, aDrawMode ) );

    return aDrawMode;
}

void ScGridWindow::DrawAfterScroll()
{
    PaintImmediately(); // always, so the behaviour with and without DrawingLayer is the same

    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();
    if (pDrView)
    {
        OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();
        if (pOlView && pOlView->GetWindow() == this)
            pOlView->ShowCursor(false); // was removed at scrolling
    }
}

void ScGridWindow::CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress)
{
    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();
    if (pDrView)
    {
        const ScViewOptions& rOpts = mrViewData.GetOptions();
        if(rOpts.GetOption( VOPT_ANCHOR ))
        {
            bool bNegativePage = mrViewData.GetDocument().IsNegativePage( mrViewData.GetTabNo() );
            Point aPos = mrViewData.GetScrPos( rAddress.Col(), rAddress.Row(), eWhich, true );
            aPos = PixelToLogic(aPos);
            rHdl.AddHdl(std::make_unique<SdrHdl>(aPos, bNegativePage ? SdrHdlKind::Anchor_TR : SdrHdlKind::Anchor));
        }
    }
}

void ScGridWindow::UpdateStatusPosSize()
{
    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();
    if (!pDrView)
        return; // shouldn't be called in that case

    SdrPageView* pPV = pDrView->GetSdrPageView();
    if (!pPV)
        return; // shouldn't be called in that case either

    SfxItemSetFixed<SID_ATTR_POSITION, SID_ATTR_SIZE> aSet(mrViewData.GetViewShell()->GetPool());

    //  Fill items for position and size:
    //  show action rectangle during action,
    //  position and size of selected object(s) if something is selected,
    //  mouse position otherwise

    bool bActionItem = false;
    if ( pDrView->IsAction() ) // action rectangle
    {
        tools::Rectangle aRect;
        pDrView->TakeActionRect( aRect );
        if ( !aRect.IsEmpty() )
        {
            pPV->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() ) ) );
            bActionItem = true;
        }
    }
    if ( !bActionItem )
    {
        if ( pDrView->GetMarkedObjectList().GetMarkCount() != 0 ) // selected objects
        {
            tools::Rectangle aRect = pDrView->GetAllMarkedRect();
            pPV->LogicToPagePos(aRect);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                    Size( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() ) ) );
        }
        else // mouse position
        {
            Point aPos = PixelToLogic(aCurMousePos);
            pPV->LogicToPagePos(aPos);
            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
        }
    }

    mrViewData.GetBindings().SetState(aSet);
}

bool ScGridWindow::DrawHasMarkedObj()
{
    ScDrawView* p = mrViewData.GetScDrawView();
    return p && p->GetMarkedObjectList().GetMarkCount() != 0;
}

void ScGridWindow::DrawMarkDropObj( SdrObject* pObj )
{
    ScDrawView* pDrView = mrViewData.GetView()->GetScDrawView();
    if (pDrView)
        pDrView->MarkDropObj(pObj);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
