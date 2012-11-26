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
#include "precompiled_sd.hxx"

#include "fuconstr.hxx"

#include <svx/svxids.hrc>
#include <svl/aeitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "fudraw.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "FrameView.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "stlpool.hxx"
#include <svx/globl3d.hxx>

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct::FuConstruct (
    ViewShell*      pViewSh,
    ::sd::Window*           pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq),
      bSelectionChanged(false)
{
}

void FuConstruct::DoExecute( SfxRequest& rReq )
{
    FuDraw::DoExecute( rReq );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    bMBDown = true;
    bSelectionChanged = false;

    if ( mpView->IsAction() )
    {
        // #90235# this extra triggering is an error and leads to
        // erasing the last two points when creating a polygon.
        // if ( rMEvt.IsRight() )
        //  mpView->BckAction();
        return true;
    }

    bFirstMouseMove = true;
    aDragTimer.Start();

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    aMDPos = mpWindow->GetInverseViewTransformation() * aPixelPos;
    // TTTT: evtl. unify fHitLog/fDrgLog/fHitTol at SdrView?
    const double fHitLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(HITPIX, 0.0)).getLength());

    if (rMEvt.IsLeft() && mpView->IsExtendedMouseEventDispatcherEnabled())
    {
        mpWindow->CaptureMouse();

        SdrHdl* pHdl = mpView->PickHandle(aMDPos);

        if ( pHdl != NULL || mpView->IsMarkedObjHit(aMDPos, fHitLog) )
        {
            const double fTolerance(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(DRGPIX, 0.0)).getLength());
            mpView->BegDragObj(aMDPos, pHdl, fTolerance);
            bReturn = true;
        }
        else if ( mpView->areSdrObjectsSelected() )
        {
            mpView->UnmarkAll();
            bReturn = true;
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        if( bFirstMouseMove )
            bFirstMouseMove = false;
        else
            aDragTimer.Stop();
    }

    if ( mpView->IsAction() )
    {
        const basegfx::B2DPoint aPix(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPix);

        ForceScroll(aPix);
        mpView->MovAction(aLogicPos);
    }

    return true;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = true;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = false;
    }

    FuDraw::MouseButtonUp(rMEvt);

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aPnt(mpWindow->GetInverseViewTransformation() * aPixelPos);

    if ( mpView && mpView->IsDragObj() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();
        bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !mpView->IsPresObjSelected(false, true);
        }

        mpView->SetDragWithCopy(bDragWithCopy);
        mpView->EndDragObj( mpView->IsDragWithCopy() );
    }
    else if ( mpView && mpView->IsMarkObj() )
    {
        mpView->EndMarkObj();
    }
    else
    {
        bReturn = false;
    }

    if ( mpView &&  !mpView->IsAction() )
    {
        mpWindow->ReleaseMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if ( !mpView->areSdrObjectsSelected() )
        {
            SdrObject* pObj;

            if (!mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj))
            {
                const double fHitLog(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(HITPIX, 0.0)).getLength());
                mpView->MarkObj(aPnt, fHitLog);
            }

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
        }
        else if (rMEvt.IsLeft() && !rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 !bSelectionChanged                   &&
                 fabs(aPnt.getX() - aMDPos.getX()) < nDrgLog &&
                 fabs(aPnt.getY() - aMDPos.getY()) < nDrgLog)
        {
            /**************************************************************
            * Toggle zw. Selektion und Rotation
            **************************************************************/
            const SdrObject* pSingleObj = mpView->getSelectedIfSingle();

            if (mpView->GetDragMode() == SDRDRAG_MOVE && mpView->IsRotateAllowed() &&
                (mpViewShell->GetFrameView()->IsClickChangeRotation() ||
                 (pSingleObj && pSingleObj->GetObjInventor()==E3dInventor)))
            {
                mpView->SetDragMode(SDRDRAG_ROTATE);
            }
            else
            {
                mpView->SetDragMode(SDRDRAG_MOVE);
            }
        }
    }

    sal_uInt16 nClicks = rMEvt.GetClicks();

    if (nClicks == 2 && rMEvt.IsLeft() && bMBDown &&
        !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift() )
    {
        DoubleClick(rMEvt);
    }

    bMBDown = false;

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    if ( !bReturn )
        bReturn = FuDraw::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstruct::Activate()
{
    mpView->SetViewEditMode(SDREDITMODE_CREATE);
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
    mpView->SetViewEditMode(SDREDITMODE_EDIT);
}

/*************************************************************************
|*
|* StyleSheet fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

void FuConstruct::SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj)
{
    sal_Bool bUseFillStyle, bUseNoFillStyle;
    bUseFillStyle = bUseNoFillStyle = sal_False;

    switch( nSlotId )
    {
    case SID_DRAW_RECT:
    case SID_DRAW_RECT_ROUND:
    case SID_DRAW_SQUARE:
    case SID_DRAW_SQUARE_ROUND:
    case SID_DRAW_ELLIPSE:
    case SID_DRAW_PIE:
    case SID_DRAW_ELLIPSECUT:
    case SID_DRAW_CIRCLE:
    case SID_DRAW_CIRCLEPIE:
    case SID_DRAW_CIRCLECUT:
    case SID_DRAW_POLYGON:
    case SID_DRAW_XPOLYGON:
    case SID_DRAW_FREELINE:
    case SID_DRAW_BEZIER_FILL:
    {
        bUseFillStyle = sal_True;
        break;
    }
    case  SID_DRAW_RECT_NOFILL:
    case SID_DRAW_RECT_ROUND_NOFILL:
    case SID_DRAW_SQUARE_NOFILL:
    case SID_DRAW_SQUARE_ROUND_NOFILL:
    case SID_DRAW_ELLIPSE_NOFILL:
    case SID_DRAW_PIE_NOFILL:
    case SID_DRAW_ELLIPSECUT_NOFILL:
    case SID_DRAW_CIRCLE_NOFILL:
    case SID_DRAW_CIRCLEPIE_NOFILL:
    case SID_DRAW_CIRCLECUT_NOFILL:
    case SID_DRAW_POLYGON_NOFILL:
    case SID_DRAW_XPOLYGON_NOFILL:
    case SID_DRAW_FREELINE_NOFILL:
    case SID_DRAW_LINE:
    case SID_DRAW_XLINE:
    case SID_CONNECTOR_ARROW_START:
    case SID_CONNECTOR_ARROW_END:
    case SID_CONNECTOR_ARROWS:
    case SID_CONNECTOR_CIRCLE_START:
    case SID_CONNECTOR_CIRCLE_END:
    case SID_CONNECTOR_CIRCLES:
    case SID_CONNECTOR_LINE:
    case SID_CONNECTOR_LINE_ARROW_START:
    case SID_CONNECTOR_LINE_ARROW_END:
    case SID_CONNECTOR_LINE_ARROWS:
    case SID_CONNECTOR_LINE_CIRCLE_START:
    case SID_CONNECTOR_LINE_CIRCLE_END:
    case SID_CONNECTOR_LINE_CIRCLES:
    case SID_CONNECTOR_CURVE:
    case SID_CONNECTOR_CURVE_ARROW_START:
    case SID_CONNECTOR_CURVE_ARROW_END:
    case SID_CONNECTOR_CURVE_ARROWS:
    case SID_CONNECTOR_CURVE_CIRCLE_START:
    case SID_CONNECTOR_CURVE_CIRCLE_END:
    case SID_CONNECTOR_CURVE_CIRCLES:
    case SID_CONNECTOR_LINES:
    case SID_CONNECTOR_LINES_ARROW_START:
    case SID_CONNECTOR_LINES_ARROW_END:
    case SID_CONNECTOR_LINES_ARROWS:
    case SID_CONNECTOR_LINES_CIRCLE_START:
    case SID_CONNECTOR_LINES_CIRCLE_END:
    case SID_CONNECTOR_LINES_CIRCLES:
    case SID_DRAW_BEZIER_NOFILL:
    case SID_LINE_ARROW_END:
    {
        bUseNoFillStyle = sal_True;
        break;
    }
    }
    SetStyleSheet( rAttr, pObj, bUseFillStyle, bUseNoFillStyle );
}

void FuConstruct::SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
        const sal_Bool bForceFillStyle, const sal_Bool bForceNoFillStyle )
{
    SdrPageView* pPV = mpView->GetSdrPageView();

    if(pPV)
    {
        SdPage& rPage = (SdPage&)pPV->getSdrPageFromSdrPageView();
        if ( rPage.IsMasterPage() && rPage.GetPageKind() == PK_STANDARD &&
         mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        /**********************************************
        * Objects was created on the slide master page
        ***********************************************/
            String aName( rPage.GetLayoutName() );
        String aSep = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ) );
        sal_uInt16 n = aName.Search(aSep);
        n = n + aSep.Len();
        aName.Erase(n);
        aName.Append( String ( SdResId( STR_LAYOUT_BACKGROUNDOBJECTS ) ) );
            SfxStyleSheet* pSheet = (SfxStyleSheet*)rPage.getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
        DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");

        if (pSheet)
        {
            // applying style sheet for background objects
                pObj->SetStyleSheet(pSheet, false);
            SfxItemSet& rSet = pSheet->GetItemSet();
            const XFillStyleItem& rFillStyle = (const XFillStyleItem&)rSet.Get(XATTR_FILLSTYLE);
            if ( bForceFillStyle )
            {
                if (rFillStyle.GetValue() == XFILL_NONE)
                    rAttr.Put(XFillStyleItem(XFILL_SOLID));
            }
            else if ( bForceNoFillStyle )
            {
                if (rFillStyle.GetValue() != XFILL_NONE)
                    rAttr.Put(XFillStyleItem(XFILL_NONE));
            }
        }
    }
    else
    {
        /***********************************
        * object was created on normal page
        ************************************/
        if ( bForceNoFillStyle )
        {
            String aName(SdResId(STR_POOLSHEET_OBJWITHOUTFILL));
                SfxStyleSheet* pSheet = (SfxStyleSheet*)rPage.getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_GRAPHICS);
            DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");

            if (pSheet)
            {
                    pObj->SetStyleSheet(pSheet, false);
                    SfxItemSet aAttr(*mpView->GetDefaultAttr().Clone());
                    aAttr.Put(pSheet->GetItemSet().Get(XATTR_FILLSTYLE));
                    pObj->SetMergedItemSet(aAttr);
                }
                else
                {
                    SfxItemSet aAttr(*mpView->GetDefaultAttr().Clone());
                    rAttr.Put(XFillStyleItem(XFILL_NONE));
                    pObj->SetMergedItemSet(aAttr);
                }
            }
        }
    }
}

} // end of namespace sd
