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
        const basegfx::B2DPoint aLogicPos(GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

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
        sal_Bool bOldMarked = pDrView->areSdrObjectsSelected();
        if (pDraw->KeyInput( rKEvt ))
        {
            sal_Bool bLeaveDraw = sal_False;
            sal_Bool bUsed = sal_True;
            sal_Bool bNewMarked = pDrView->areSdrObjectsSelected();
            if ( !pViewData->GetView()->IsDrawSelMode() )
                if ( !bNewMarked )
                {
                    pViewData->GetViewShell()->SetDrawShell( sal_False );
                    bLeaveDraw = sal_True;
                    if ( !bOldMarked &&
                        rKEvt.GetKeyCode().GetCode() == KEY_DELETE )
                        bUsed = sal_False;                  // nichts geloescht
//IAccessibility2 Implementation 2009-----
                    if(bOldMarked)
                        GetFocus();
//-----IAccessibility2 Implementation 2009
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

            pPV->DrawPageViewGrid(*pContentDev, basegfx::B2DRange(rDrawingRect.Left(), rDrawingRect.Top(), rDrawingRect.Right(), rDrawingRect.Bottom()));
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
            pOlView->ShowCursor(false);                 // ist beim Scrollen weggekommen
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
//  return pDrView && pDrView->IsMarkHdlShown() && pDrView->areSdrObjectsSelected();
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
            const Point aPos(pViewData->GetScrPos(rAddress.Col(), rAddress.Row(), eWhich, true));
            const basegfx::B2DPoint aPixelPos(aPos.X(), aPos.Y());
            const basegfx::B2DPoint aLogicPos(GetInverseViewTransformation() * aPixelPos); // after cell edit mode is ended

            new SdrHdl(rHdl, 0, bNegativePage ? HDL_ANCHOR_TR : HDL_ANCHOR, aLogicPos);
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
        basegfx::B2DRange aRange(pDrView->TakeActionRange());

        if(!aRange.isEmpty())
        {
            aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(-pPV->GetPageOrigin()));
            const Point aOldTopLeft(basegfx::fround(aRange.getMinX()), basegfx::fround(aRange.getMinY()));
            const Size aOldSize(basegfx::fround(aRange.getWidth()), basegfx::fround(aRange.getHeight()));

            aSet.Put( SfxPointItem( SID_ATTR_POSITION, aOldTopLeft ) );
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE, aOldSize ) );

            bActionItem = sal_True;
        }
    }
    if ( !bActionItem )
    {
        if ( pDrView->areSdrObjectsSelected() )     // selected objects
        {
            basegfx::B2DRange aRange(pDrView->getMarkedObjectSnapRange());
            aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(-pPV->GetPageOrigin()));

            aSet.Put(SfxPointItem(SID_ATTR_POSITION, Point(basegfx::fround(aRange.getMinX()), basegfx::fround(aRange.getMinY()))));
            aSet.Put(SvxSizeItem(SID_ATTR_SIZE, Size(basegfx::fround(aRange.getWidth()), basegfx::fround(aRange.getHeight()))));
        }
        else                                // mouse position
        {
            const basegfx::B2DPoint aPos((GetInverseViewTransformation() * basegfx::B2DPoint(aCurMousePos.X(), aCurMousePos.Y())) - pPV->GetPageOrigin());

            aSet.Put(SfxPointItem(SID_ATTR_POSITION, Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY()))));
            aSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
        }
    }

    pViewData->GetBindings().SetState(aSet);
}

sal_Bool ScGridWindow::DrawHasMarkedObj()
{
    ScDrawView* p = pViewData->GetScDrawView();
    return p ? p->areSdrObjectsSelected() : sal_False;
}

void ScGridWindow::DrawMarkDropObj( SdrObject* pObj )
{
    ScDrawView* pDrView = pViewData->GetView()->GetScDrawView();
    if (pDrView)
        pDrView->MarkDropObj(pObj);
}



