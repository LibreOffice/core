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

#include "fuconrec.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "app.hrc"
#include <svl/aeitem.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/sxekitm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/adjitem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdomeas.hxx>
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include <editeng/writingmodeitem.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/svdlegacy.hxx>

#include "sdresid.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "stlpool.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "glob.hrc"

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstructRectangle::FuConstructRectangle (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuConstructRectangle::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructRectangle* pFunc;
    FunctionReference xFunc( pFunc = new FuConstructRectangle( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructRectangle::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);

    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        switch (nSlotId)
        {
            case SID_DRAW_ELLIPSE :
            {
                SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X );
                SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y );
                SFX_REQUEST_ARG (rReq, pAxisX, SfxUInt32Item, ID_VAL_AXIS_X );
                SFX_REQUEST_ARG (rReq, pAxisY, SfxUInt32Item, ID_VAL_AXIS_Y );

                const sal_Int32 nWidth(pAxisX->GetValue());
                const sal_Int32 nHeight(pAxisY->GetValue());
                const basegfx::B2DHomMatrix aObjTrans(
                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                        nWidth, nHeight,
                        pCenterX->GetValue() - (nWidth / 2), pCenterY->GetValue() - (nHeight / 2)));
                SdrCircObj* pNewCircle = new SdrCircObj(
                    *GetDoc(),
                    CircleType_Circle,
                    aObjTrans);

                mpView->InsertObjectAtView(*pNewCircle, SDRINSERT_SETDEFLAYER | SDRINSERT_SETDEFATTR);
            }
            break;

            case SID_DRAW_RECT :
            {
                SFX_REQUEST_ARG (rReq, pMouseStartX, SfxUInt32Item, ID_VAL_MOUSESTART_X );
                SFX_REQUEST_ARG (rReq, pMouseStartY, SfxUInt32Item, ID_VAL_MOUSESTART_Y );
                SFX_REQUEST_ARG (rReq, pMouseEndX, SfxUInt32Item, ID_VAL_MOUSEEND_X );
                SFX_REQUEST_ARG (rReq, pMouseEndY, SfxUInt32Item, ID_VAL_MOUSEEND_Y );

                const sal_Int32 nLeft(pMouseStartX->GetValue());
                const sal_Int32 nTop(pMouseStartY->GetValue());
                const basegfx::B2DHomMatrix aObjTrans(
                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                        pMouseEndX->GetValue() - nLeft, pMouseEndY->GetValue() - nTop,
                        nLeft, nTop));
                SdrRectObj  *pNewRect = new SdrRectObj(
                    *GetDoc(),
                    aObjTrans);

                mpView->InsertObjectAtView(*pNewRect, SDRINSERT_SETDEFLAYER | SDRINSERT_SETDEFATTR);
            }
            break;
        }
    }

    if (nSlotId == SID_TOOL_CONNECTOR               ||
        nSlotId == SID_CONNECTOR_ARROW_START        ||
        nSlotId == SID_CONNECTOR_ARROW_END          ||
        nSlotId == SID_CONNECTOR_ARROWS             ||
        nSlotId == SID_CONNECTOR_CIRCLE_START       ||
        nSlotId == SID_CONNECTOR_CIRCLE_END         ||
        nSlotId == SID_CONNECTOR_CIRCLES            ||
        nSlotId == SID_CONNECTOR_LINE               ||
        nSlotId == SID_CONNECTOR_LINE_ARROW_START   ||
        nSlotId == SID_CONNECTOR_LINE_ARROW_END     ||
        nSlotId == SID_CONNECTOR_LINE_ARROWS        ||
        nSlotId == SID_CONNECTOR_LINE_CIRCLE_START  ||
        nSlotId == SID_CONNECTOR_LINE_CIRCLE_END    ||
        nSlotId == SID_CONNECTOR_LINE_CIRCLES       ||
        nSlotId == SID_CONNECTOR_CURVE              ||
        nSlotId == SID_CONNECTOR_CURVE_ARROW_START  ||
        nSlotId == SID_CONNECTOR_CURVE_ARROW_END    ||
        nSlotId == SID_CONNECTOR_CURVE_ARROWS       ||
        nSlotId == SID_CONNECTOR_CURVE_CIRCLE_START ||
        nSlotId == SID_CONNECTOR_CURVE_CIRCLE_END   ||
        nSlotId == SID_CONNECTOR_CURVE_CIRCLES      ||
        nSlotId == SID_CONNECTOR_LINES              ||
        nSlotId == SID_CONNECTOR_LINES_ARROW_START  ||
        nSlotId == SID_CONNECTOR_LINES_ARROW_END    ||
        nSlotId == SID_CONNECTOR_LINES_ARROWS       ||
        nSlotId == SID_CONNECTOR_LINES_CIRCLE_START ||
        nSlotId == SID_CONNECTOR_LINES_CIRCLE_END   ||
        nSlotId == SID_CONNECTOR_LINES_CIRCLES      ||
        nSlotId == SID_LINE_ARROW_START             ||
        nSlotId == SID_LINE_ARROW_END               ||
        nSlotId == SID_LINE_ARROWS                  ||
        nSlotId == SID_LINE_ARROW_CIRCLE            ||
        nSlotId == SID_LINE_CIRCLE_ARROW            ||
        nSlotId == SID_LINE_ARROW_SQUARE            ||
        nSlotId == SID_LINE_SQUARE_ARROW )
    {
        mpView->UnmarkAll();
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuConstructRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if(OBJ_CAPTION == mpView->getSdrObjectCreationInfo().getIdent())
        {
            bReturn = mpView->BegCreateCaptionObj(
                aLogicPos,
                basegfx::B2DVector(846.0, 846.0), // (4x2)cm
                nDrgLog);
        }
        else
        {
            mpView->BegCreateObj(
                aLogicPos,
                nDrgLog);
        }

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pObj->GetObjectItemPool());
            SetStyleSheet(aAttr, pObj);
            SetAttributes(aAttr, pObj);
            SetLineEnds(aAttr, pObj);
            pObj->SetMergedItemSet(aAttr);

            if( nSlotId == SID_DRAW_CAPTION_VERTICAL )
                ( (SdrTextObj*) pObj)->SetVerticalWriting( true );
        }
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstructRectangle::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstructRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn(false);

    if(mpView->GetCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();

        if(pObj && mpView->EndCreateObj(SDRCREATE_FORCEEND))
        {
            if(SID_DRAW_MEASURELINE == nSlotId)
            {
                SdrLayerAdmin& rAdmin = mpDoc->GetModelLayerAdmin();
                String aStr(SdResId(STR_LAYER_MEASURELINES));
                pObj->SetLayer(rAdmin.GetLayerID(aStr, false));
            }

            // #88751# init text position when vertica caption object is created
            if(SID_DRAW_CAPTION_VERTICAL == nSlotId && dynamic_cast< SdrCaptionObj* >(pObj))
            {
                // draw text object, needs to be initialized when vertical text is used
                SfxItemSet aSet(pObj->GetMergedItemSet());

                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                // #109583#
                // Correct the value of SDRATTR_TEXTDIRECTION to avoid SetItemSet
                // calling SetVerticalWriting() again since this item may not yet
                // be set at the object and thus may differ from verical state of
                // the object.
                aSet.Put(SvxWritingModeItem(com::sun::star::text::WritingMode_TB_RL, SDRATTR_TEXTDIRECTION));
                pObj->SetMergedItemSet(aSet);
            }

            bReturn = true;
        }
    }

    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

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

bool FuConstructRectangle::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructRectangle::Activate()
{
    SdrObjKind aObjKind(OBJ_NONE);
    SdrPathObjType aSdrPathObjType(PathType_Line);

    switch (nSlotId)
    {
        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROWS:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_SQUARE_ARROW:
            mpView->SetGlueVisible();
            // keine break !
        case SID_DRAW_LINE :
        case SID_DRAW_XLINE:
            aObjKind = OBJ_POLY;
            aSdrPathObjType = PathType_Line;
            break;

        case SID_DRAW_MEASURELINE:
        {
            aObjKind = OBJ_MEASURE;
        }
        break;

        case SID_DRAW_RECT             :
        case SID_DRAW_RECT_NOFILL      :
        case SID_DRAW_RECT_ROUND       :
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE           :
        case SID_DRAW_SQUARE_NOFILL    :
        case SID_DRAW_SQUARE_ROUND     :
        case SID_DRAW_SQUARE_ROUND_NOFILL:
        {
            aObjKind = OBJ_RECT;
        }
        break;

        case SID_DRAW_ELLIPSE       :
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE        :
        case SID_DRAW_CIRCLE_NOFILL :
        {
            aObjKind = OBJ_CIRC;
        }
        break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
        {
            aObjKind = OBJ_CAPTION;
        }
        break;

        case SID_TOOL_CONNECTOR:
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
        {
            aObjKind = OBJ_EDGE;
            mpView->SetGlueVisible();
        }
        break;

        default:
        {
            aObjKind = OBJ_RECT;
        }
        break;
    }

    SdrObjectCreationInfo aSdrObjectCreationInfo(static_cast< sal_uInt16 >(aObjKind));

    aSdrObjectCreationInfo.setSdrPathObjType(aSdrPathObjType);
    mpView->setSdrObjectCreationInfo(aSdrObjectCreationInfo);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstructRectangle::Deactivate()
{
    if( nSlotId == SID_TOOL_CONNECTOR               ||
        nSlotId == SID_CONNECTOR_ARROW_START        ||
        nSlotId == SID_CONNECTOR_ARROW_END          ||
        nSlotId == SID_CONNECTOR_ARROWS             ||
        nSlotId == SID_CONNECTOR_CIRCLE_START       ||
        nSlotId == SID_CONNECTOR_CIRCLE_END         ||
        nSlotId == SID_CONNECTOR_CIRCLES            ||
        nSlotId == SID_CONNECTOR_LINE               ||
        nSlotId == SID_CONNECTOR_LINE_ARROW_START   ||
        nSlotId == SID_CONNECTOR_LINE_ARROW_END     ||
        nSlotId == SID_CONNECTOR_LINE_ARROWS        ||
        nSlotId == SID_CONNECTOR_LINE_CIRCLE_START  ||
        nSlotId == SID_CONNECTOR_LINE_CIRCLE_END    ||
        nSlotId == SID_CONNECTOR_LINE_CIRCLES       ||
        nSlotId == SID_CONNECTOR_CURVE              ||
        nSlotId == SID_CONNECTOR_CURVE_ARROW_START  ||
        nSlotId == SID_CONNECTOR_CURVE_ARROW_END    ||
        nSlotId == SID_CONNECTOR_CURVE_ARROWS       ||
        nSlotId == SID_CONNECTOR_CURVE_CIRCLE_START ||
        nSlotId == SID_CONNECTOR_CURVE_CIRCLE_END   ||
        nSlotId == SID_CONNECTOR_CURVE_CIRCLES      ||
        nSlotId == SID_CONNECTOR_LINES              ||
        nSlotId == SID_CONNECTOR_LINES_ARROW_START  ||
        nSlotId == SID_CONNECTOR_LINES_ARROW_END    ||
        nSlotId == SID_CONNECTOR_LINES_ARROWS       ||
        nSlotId == SID_CONNECTOR_LINES_CIRCLE_START ||
        nSlotId == SID_CONNECTOR_LINES_CIRCLE_END   ||
        nSlotId == SID_CONNECTOR_LINES_CIRCLES      ||
        nSlotId == SID_LINE_ARROW_START             ||
        nSlotId == SID_LINE_ARROW_END               ||
        nSlotId == SID_LINE_ARROWS                  ||
        nSlotId == SID_LINE_ARROW_CIRCLE            ||
        nSlotId == SID_LINE_CIRCLE_ARROW            ||
        nSlotId == SID_LINE_ARROW_SQUARE            ||
        nSlotId == SID_LINE_SQUARE_ARROW )
    {
        mpView->SetGlueVisible( false );
    }
    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Attribute fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

void FuConstructRectangle::SetAttributes(SfxItemSet& rAttr, SdrObject* pObj)
{
    if (nSlotId == SID_DRAW_RECT_ROUND        ||
        nSlotId == SID_DRAW_RECT_ROUND_NOFILL ||
        nSlotId == SID_DRAW_SQUARE_ROUND      ||
        nSlotId == SID_DRAW_SQUARE_ROUND_NOFILL)
    {
        /**********************************************************************
        * Abgerundete Ecken
        **********************************************************************/
        rAttr.Put(SdrMetricItem(SDRATTR_ECKENRADIUS, 500));
    }
    else if (nSlotId == SID_CONNECTOR_LINE              ||
             nSlotId == SID_CONNECTOR_LINE_ARROW_START  ||
             nSlotId == SID_CONNECTOR_LINE_ARROW_END    ||
             nSlotId == SID_CONNECTOR_LINE_ARROWS       ||
             nSlotId == SID_CONNECTOR_LINE_CIRCLE_START ||
             nSlotId == SID_CONNECTOR_LINE_CIRCLE_END   ||
             nSlotId == SID_CONNECTOR_LINE_CIRCLES)
    {
        /**********************************************************************
        * Direkt-Verbinder
        **********************************************************************/
        rAttr.Put(SdrEdgeKindItem(SDREDGE_ONELINE));
    }
    else if (nSlotId == SID_CONNECTOR_LINES              ||
             nSlotId == SID_CONNECTOR_LINES_ARROW_START  ||
             nSlotId == SID_CONNECTOR_LINES_ARROW_END    ||
             nSlotId == SID_CONNECTOR_LINES_ARROWS       ||
             nSlotId == SID_CONNECTOR_LINES_CIRCLE_START ||
             nSlotId == SID_CONNECTOR_LINES_CIRCLE_END   ||
             nSlotId == SID_CONNECTOR_LINES_CIRCLES)
    {
        /**********************************************************************
        * Linien-Verbinder
        **********************************************************************/
        rAttr.Put(SdrEdgeKindItem(SDREDGE_THREELINES));
    }
    else if (nSlotId == SID_CONNECTOR_CURVE              ||
             nSlotId == SID_CONNECTOR_CURVE_ARROW_START  ||
             nSlotId == SID_CONNECTOR_CURVE_ARROW_END    ||
             nSlotId == SID_CONNECTOR_CURVE_ARROWS       ||
             nSlotId == SID_CONNECTOR_CURVE_CIRCLE_START ||
             nSlotId == SID_CONNECTOR_CURVE_CIRCLE_END   ||
             nSlotId == SID_CONNECTOR_CURVE_CIRCLES)
    {
        /**********************************************************************
        * Kurven-Verbinder
        **********************************************************************/
        rAttr.Put(SdrEdgeKindItem(SDREDGE_BEZIER));
    }
    else if ( nSlotId == SID_DRAW_CAPTION || nSlotId == SID_DRAW_CAPTION_VERTICAL )
    {
        /**********************************************************************
        * Legendenobjekt
        **********************************************************************/
        const Size aSize(sdr::legacy::GetLogicRect(*pObj).GetSize());
        rAttr.Put( SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, aSize.Height() ) );
        rAttr.Put( SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, aSize.Width() ) );
        rAttr.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, true ) );
        rAttr.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, true ) );

        // #103516# Support full with for vertical caption objects, too
        if(SID_DRAW_CAPTION == nSlotId)
            rAttr.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        else
            rAttr.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_BLOCK ) );

        rAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
        rAttr.Put( SdrMetricItem(SDRATTR_TEXT_LEFTDIST, 100 ) );
        rAttr.Put( SdrMetricItem(SDRATTR_TEXT_RIGHTDIST, 100 ) );
        rAttr.Put( SdrMetricItem(SDRATTR_TEXT_UPPERDIST, 100 ) );
        rAttr.Put( SdrMetricItem(SDRATTR_TEXT_LOWERDIST, 100 ) );
    }
    else if (nSlotId == SID_DRAW_MEASURELINE)
    {
        /**********************************************************************
        * Masslinie
        **********************************************************************/
        SdrPageView* pPV = mpView->GetSdrPageView();

        if(pPV)
        {
            SdPage& rPage = (SdPage&) pPV->getSdrPageFromSdrPageView();
        String aName(SdResId(STR_POOLSHEET_MEASURE));
            SfxStyleSheet* pSheet = (SfxStyleSheet*)rPage.getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_GRAPHICS);
        DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");

        if (pSheet)
        {
                pObj->SetStyleSheet(pSheet, false);
        }

            SdrLayerAdmin& rAdmin = mpDoc->GetModelLayerAdmin();
        String aStr(SdResId(STR_LAYER_MEASURELINES));
            pObj->SetLayer(rAdmin.GetLayerID(aStr, false));
        }
    }
    else if (nSlotId == OBJ_CUSTOMSHAPE )
    {
    }
}


/*************************************************************************
|*
|* Linienanfaenge und -enden fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

::basegfx::B2DPolyPolygon getPolygon( sal_uInt16 nResId, SdrModel* pDoc )
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndList* pLineEndList = pDoc->GetLineEndList();

    if( pLineEndList )
    {
        String aArrowName( SVX_RES(nResId) );
        long nCount = pLineEndList->Count();
        long nIndex;
        for( nIndex = 0L; nIndex < nCount; nIndex++ )
        {
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if( pEntry->GetName() == aArrowName )
            {
                aRetval = pEntry->GetLineEnd();
                break;
            }
        }
    }

    return aRetval;
}

void FuConstructRectangle::SetLineEnds(SfxItemSet& rAttr, SdrObject* pObj)
{
    if ( (pObj->GetObjIdentifier() == OBJ_EDGE &&
          nSlotId != SID_TOOL_CONNECTOR        &&
          nSlotId != SID_CONNECTOR_LINE        &&
          nSlotId != SID_CONNECTOR_LINES       &&
          nSlotId != SID_CONNECTOR_CURVE)      ||
          nSlotId == SID_LINE_ARROW_START      ||
          nSlotId == SID_LINE_ARROW_END        ||
          nSlotId == SID_LINE_ARROWS           ||
          nSlotId == SID_LINE_ARROW_CIRCLE     ||
          nSlotId == SID_LINE_CIRCLE_ARROW     ||
          nSlotId == SID_LINE_ARROW_SQUARE     ||
          nSlotId == SID_LINE_SQUARE_ARROW )
    {
        /**************************************************************
        * Linienanfaenge und -enden attributieren
        **************************************************************/

        // Pfeilspitze
        ::basegfx::B2DPolyPolygon aArrow( getPolygon( RID_SVXSTR_ARROW, mpDoc ) );
        if( !aArrow.count() )
        {
            ::basegfx::B2DPolygon aNewArrow;
            aNewArrow.append(::basegfx::B2DPoint(10.0, 0.0));
            aNewArrow.append(::basegfx::B2DPoint(0.0, 30.0));
            aNewArrow.append(::basegfx::B2DPoint(20.0, 30.0));
            aNewArrow.setClosed(true);
            aArrow.append(aNewArrow);
        }

        // Kreis
        ::basegfx::B2DPolyPolygon aCircle( getPolygon( RID_SVXSTR_CIRCLE, mpDoc ) );
        if( !aCircle.count() )
        {
            ::basegfx::B2DPolygon aNewCircle;
            aNewCircle = ::basegfx::tools::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
            aNewCircle.setClosed(true);
            aCircle.append(aNewCircle);
        }

        // Quadrat
        ::basegfx::B2DPolyPolygon aSquare( getPolygon( RID_SVXSTR_SQUARE, mpDoc ) );
        if( !aSquare.count() )
        {
            ::basegfx::B2DPolygon aNewSquare;
            aNewSquare.append(::basegfx::B2DPoint(0.0, 0.0));
            aNewSquare.append(::basegfx::B2DPoint(10.0, 0.0));
            aNewSquare.append(::basegfx::B2DPoint(10.0, 10.0));
            aNewSquare.append(::basegfx::B2DPoint(0.0, 10.0));
            aNewSquare.setClosed(true);
            aSquare.append(aNewSquare);
        }

        SfxItemSet aSet( mpDoc->GetItemPool() );
        mpView->GetAttributes( aSet );

        // #i3908# Here, the default Line Start/End width for arrow construction is
        // set. To have the same value in all situations (construction) in i3908
        // it was decided to change the default to 0.03 cm for all situations.
        long nWidth = 300; // (1/100th mm)

        // Linienstaerke ermitteln und daraus die Linienendenstaerke berechnen
        if( aSet.GetItemState( XATTR_LINEWIDTH ) != SFX_ITEM_DONTCARE )
        {
            long nValue = ( ( const XLineWidthItem& ) aSet.Get( XATTR_LINEWIDTH ) ).GetValue();
            if( nValue > 0 )
                nWidth = nValue * 3;
        }

        switch (nSlotId)
        {
            case SID_CONNECTOR_ARROWS:
            case SID_CONNECTOR_LINE_ARROWS:
            case SID_CONNECTOR_LINES_ARROWS:
            case SID_CONNECTOR_CURVE_ARROWS:
            case SID_LINE_ARROWS:
            {
                // Verbinder mit Pfeil-Enden
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineStartWidthItem(nWidth));
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_ARROW_START:
            case SID_CONNECTOR_LINE_ARROW_START:
            case SID_CONNECTOR_LINES_ARROW_START:
            case SID_CONNECTOR_CURVE_ARROW_START:
            case SID_LINE_ARROW_START:
            case SID_LINE_ARROW_CIRCLE:
            case SID_LINE_ARROW_SQUARE:
            {
                // Verbinder mit Pfeil-Anfang
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_ARROW_END:
            case SID_CONNECTOR_LINE_ARROW_END:
            case SID_CONNECTOR_LINES_ARROW_END:
            case SID_CONNECTOR_CURVE_ARROW_END:
            case SID_LINE_ARROW_END:
            case SID_LINE_CIRCLE_ARROW:
            case SID_LINE_SQUARE_ARROW:
            {
                // Verbinder mit Pfeil-Ende
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_CIRCLES:
            case SID_CONNECTOR_LINE_CIRCLES:
            case SID_CONNECTOR_LINES_CIRCLES:
            case SID_CONNECTOR_CURVE_CIRCLES:
            {
                // Verbinder mit Kreis-Enden
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_CIRCLE_START:
            case SID_CONNECTOR_LINE_CIRCLE_START:
            case SID_CONNECTOR_LINES_CIRCLE_START:
            case SID_CONNECTOR_CURVE_CIRCLE_START:
            {
                // Verbinder mit Kreis-Anfang
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_CIRCLE_END:
            case SID_CONNECTOR_LINE_CIRCLE_END:
            case SID_CONNECTOR_LINES_CIRCLE_END:
            case SID_CONNECTOR_CURVE_CIRCLE_END:
            {
                // Verbinder mit Kreis-Ende
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;
        };

        // Und nochmal fuer die noch fehlenden Enden
        switch (nSlotId)
        {
            case SID_LINE_ARROW_CIRCLE:
            {
                // Kreis-Ende
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_CIRCLE_ARROW:
            {
                // Kreis-Anfang
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_SQUARE:
            {
                // Quadrat-Ende
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_SQUARE_ARROW:
            {
                // Quadrat-Anfang
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;
        }
    }
}

// #97016#
SdrObject* FuConstructRectangle::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    DBG_ASSERT( (nID != SID_DRAW_FONTWORK) && (nID != SID_DRAW_FONTWORK_VERTICAL ), "FuConstRectangle::CreateDefaultObject can not create Fontwork shapes!" );

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->getSdrObjectCreationInfo());

    if(pObj)
    {
        basegfx::B2DRange aRange(rRange);

        if(SID_DRAW_SQUARE == nID ||
            SID_DRAW_SQUARE_NOFILL == nID ||
            SID_DRAW_SQUARE_ROUND == nID ||
            SID_DRAW_SQUARE_ROUND_NOFILL == nID ||
            SID_DRAW_CIRCLE == nID ||
            SID_DRAW_CIRCLE_NOFILL == nID)
        {
            // force quadratic
            ImpForceQuadratic(aRange);
        }

        const basegfx::B2DPoint aStart(aRange.getMinimum());
        const basegfx::B2DPoint aEnd(aRange.getMaximum());

        switch(nID)
        {
            case SID_DRAW_LINE:
            case SID_DRAW_XLINE:
            case SID_LINE_ARROW_START:
            case SID_LINE_ARROW_END:
            case SID_LINE_ARROWS:
            case SID_LINE_ARROW_CIRCLE:
            case SID_LINE_CIRCLE_ARROW:
            case SID_LINE_ARROW_SQUARE:
            case SID_LINE_SQUARE_ARROW:
            {
                SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

                if(pSdrPathObj)
                {
                    const double fYMiddle((aRange.getMinY() + aRange.getMaxY()) * 0.5);

                    ::basegfx::B2DPolygon aB2DPolygon;
                    aB2DPolygon.append(::basegfx::B2DPoint(aStart.getX(), fYMiddle));
                    aB2DPolygon.append(::basegfx::B2DPoint(aEnd.getX(), fYMiddle));
                    pSdrPathObj->setB2DPolyPolygonInObjectCoordinates(::basegfx::B2DPolyPolygon(aB2DPolygon));
                }
                else
                {
                    DBG_ERROR("Object is NO line object");
                }

                break;
            }

            case SID_DRAW_MEASURELINE:
            {
                SdrMeasureObj* pSdrMeasureObj = dynamic_cast< SdrMeasureObj* >(pObj);

                if(pSdrMeasureObj)
                {
                    const double fYMiddle((aRange.getMinY() + aRange.getMaxY()) * 0.5);

                    pSdrMeasureObj->SetObjectPoint(basegfx::B2DPoint(aStart.getX(), fYMiddle), 0);
                    pSdrMeasureObj->SetObjectPoint(basegfx::B2DPoint(aEnd.getX(), fYMiddle), 1);
                }
                else
                {
                    DBG_ERROR("Object is NO measure object");
                }

                break;
            }

            case SID_TOOL_CONNECTOR:
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
            {
                SdrEdgeObj* pSdrEdgeObj = dynamic_cast< SdrEdgeObj* >(pObj);

                if(pSdrEdgeObj)
                {
                    pSdrEdgeObj->SetTailPoint(false, aStart);
                    pSdrEdgeObj->SetTailPoint(true, aEnd);
                }
                else
                {
                    DBG_ERROR("Object is NO connector object");
                }

                break;
            }
            case SID_DRAW_CAPTION:
            case SID_DRAW_CAPTION_VERTICAL:
            {
                SdrCaptionObj* pSdrCaptionObj = dynamic_cast< SdrCaptionObj* >(pObj);

                if(pSdrCaptionObj)
                {
                    sal_Bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    static_cast< SdrTextObj* >(pObj)->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pObj->SetMergedItemSet(aSet);
                    }

                    // For task #105815# the default text is not inserted anymore.
                    //  String aText(SdResId(STR_POOLSHEET_TEXT));
                    //  pSdrCaptionObj->SetText(aText);

                    sdr::legacy::SetLogicRange(*pSdrCaptionObj, aRange);
                    pSdrCaptionObj->SetTailPos(aRange.getMinimum() - (aRange.getRange() * 0.5));
                }
                else
                {
                    DBG_ERROR("Object is NO caption object");
                }

                break;
            }

            default:
            {
                sdr::legacy::SetLogicRange(*pObj, aRange);

                break;
            }
        }

        SfxItemSet aAttr(pObj->GetObjectItemPool());
        SetStyleSheet(aAttr, pObj);
        SetAttributes(aAttr, pObj);
        SetLineEnds(aAttr, pObj);
        pObj->SetMergedItemSet(aAttr);
    }

    return pObj;
}

} // end of namespace sd
