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

#include <fuconrec.hxx>
#include <svx/svdpagv.hxx>

#include <svx/svxids.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <app.hrc>
#include <svl/aeitem.hxx>
#include <svl/itemset.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/sdtmfitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sderitm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/xtable.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xfillit.hxx>

#include <svx/svdocapt.hxx>

#include <svx/svdomeas.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <ToolBarManager.hxx>
#include <editeng/writingmodeitem.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <sdresid.hxx>
#include <View.hxx>
#include <sdpage.hxx>
#include <Window.hxx>
#include <stlpool.hxx>
#include <drawdoc.hxx>
#include <unokywds.hxx>

#include <strings.hrc>

namespace sd {


FuConstructRectangle::FuConstructRectangle (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
    , mnFillTransparence(0)
    , mnLineStyle(SAL_MAX_UINT16)
{
}

namespace{

/// Checks to see if the request has a parameter of IsSticky:bool=true
/// It means that the selected command/button will stay selected after use
bool isSticky(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs ();
    if (pArgs)
    {
        const SfxBoolItem* pIsSticky = rReq.GetArg<SfxBoolItem>(FN_PARAM_4);
        if (pIsSticky && pIsSticky->GetValue())
            return true;
    }

    return false;
}

}

rtl::Reference<FuPoor> FuConstructRectangle::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructRectangle* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstructRectangle( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent || isSticky(rReq));
    return xFunc;
}

void FuConstructRectangle::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::ToolBarGroup::Function,
        ToolBarManager::msDrawingObjectToolBar);

    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        switch (nSlotId)
        {
            case SID_DRAW_ELLIPSE :
            {
                const SfxUInt32Item* pCenterX = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_X);
                const SfxUInt32Item* pCenterY = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_Y);
                const SfxUInt32Item* pAxisX = rReq.GetArg<SfxUInt32Item>(ID_VAL_AXIS_X);
                const SfxUInt32Item* pAxisY = rReq.GetArg<SfxUInt32Item>(ID_VAL_AXIS_Y);

                if (!pCenterX || !pCenterY || !pAxisX || !pAxisY)
                    break;

                ::tools::Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () - pAxisY->GetValue () / 2,
                                           pCenterX->GetValue () + pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () + pAxisY->GetValue () / 2);
                SdrCircObj  *pNewCircle = new SdrCircObj(
                    mpView->getSdrModelFromSdrView(),
                    OBJ_CIRC,
                    aNewRectangle);
                SdrPageView *pPV = mpView->GetSdrPageView();

                mpView->InsertObjectAtView(pNewCircle, *pPV, SdrInsertFlags::SETDEFLAYER | SdrInsertFlags::SETDEFATTR);
            }
            break;

            case SID_DRAW_RECT :
            {
                const SfxUInt32Item* pMouseStartX       = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSESTART_X);
                const SfxUInt32Item* pMouseStartY       = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSESTART_Y);
                const SfxUInt32Item* pMouseEndX         = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSEEND_X);
                const SfxUInt32Item* pMouseEndY         = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSEEND_Y);
                const SfxUInt16Item* pFillTransparence  = rReq.GetArg<SfxUInt16Item>(FN_PARAM_1);
                const SfxStringItem* pFillColor         = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
                const SfxUInt16Item* pLineStyle         = rReq.GetArg<SfxUInt16Item>(FN_PARAM_3);
                const SfxStringItem* pShapeName         = rReq.GetArg<SfxStringItem>(SID_SHAPE_NAME);

                if (pFillTransparence && pFillTransparence->GetValue() > 0)
                {
                    mnFillTransparence = pFillTransparence->GetValue();
                }
                if (pFillColor && !pFillColor->GetValue().isEmpty())
                {
                    msFillColor = pFillColor->GetValue();
                }
                if (pLineStyle)
                {
                    mnLineStyle = pLineStyle->GetValue();
                }
                if (pShapeName && !pShapeName->GetValue().isEmpty())
                {
                    msShapeName = pShapeName->GetValue();
                }

                if (!pMouseStartX || !pMouseStartY || !pMouseEndX || !pMouseEndY)
                    break;

                ::tools::Rectangle   aNewRectangle (pMouseStartX->GetValue (),
                                           pMouseStartY->GetValue (),
                                           pMouseEndX->GetValue (),
                                           pMouseEndY->GetValue ());
                SdrRectObj  *pNewRect = new SdrRectObj(
                    mpView->getSdrModelFromSdrView(),
                    aNewRectangle);
                SdrPageView *pPV = mpView->GetSdrPageView();

                mpView->InsertObjectAtView(pNewRect, *pPV, SdrInsertFlags::SETDEFLAYER | SdrInsertFlags::SETDEFATTR);
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

bool FuConstructRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if (mpView->GetCurrentObjIdentifier() == OBJ_CAPTION)
        {
            Size aCaptionSize(846, 846);    // (4x2)cm
            bReturn = mpView->BegCreateCaptionObj(aPnt, aCaptionSize,
                                                nullptr, nDrgLog);
        }
        else
        {
            mpView->BegCreateObj(aPnt, nullptr, nDrgLog);
        }

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet(aAttr, pObj);
            SetAttributes(aAttr, pObj);
            SetLineEnds(aAttr, *pObj);
            pObj->SetMergedItemSet(aAttr);

            if( nSlotId == SID_DRAW_CAPTION_VERTICAL )
                static_cast<SdrTextObj*>(pObj)->SetVerticalWriting( true );
        }
    }
    return bReturn;
}

bool FuConstructRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn(false);

    if(mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();

        if(pObj && mpView->EndCreateObj(SdrCreateCmd::ForceEnd))
        {
            if(SID_DRAW_MEASURELINE == nSlotId)
            {
                SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
                pObj->SetLayer(rAdmin.GetLayerID(sUNO_LayerName_measurelines));
            }

            // init text position when vertical caption object is created
            if( dynamic_cast< const SdrCaptionObj *>( pObj ) !=  nullptr && SID_DRAW_CAPTION_VERTICAL == nSlotId)
            {
                // draw text object, needs to be initialized when vertical text is used
                SfxItemSet aSet(pObj->GetMergedItemSet());

                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                // Correct the value of SDRATTR_TEXTDIRECTION to avoid SetItemSet
                // calling SetVerticalWriting() again since this item may not yet
                // be set at the object and thus may differ from vertical state of
                // the object.
                aSet.Put(SvxWritingModeItem(css::text::WritingMode_TB_RL, SDRATTR_TEXTDIRECTION));
                pObj->SetMergedItemSet(aSet);
            }

            bReturn = true;
        }
    }

    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

    return bReturn;
}

void FuConstructRectangle::Activate()
{
    SdrObjKind aObjKind;

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
            [[fallthrough]];
        case SID_DRAW_LINE :
        case SID_DRAW_XLINE:
            aObjKind = OBJ_LINE;
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

    mpView->SetCurrentObj(static_cast<sal_uInt16>(aObjKind));

    FuConstruct::Activate();
}

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

namespace {
/// Returns the color based on the color names listed in core/include/tools/color.hxx
/// Feel free to extend with more color names from color.hxx
Color strToColor(const OUString& sColor)
{
    Color aColor = COL_AUTO;

    if (sColor == "COL_GRAY")
        aColor = COL_GRAY;
    else if (sColor == "COL_GRAY3")
        aColor = COL_GRAY3;
    else if (sColor == "COL_GRAY7")
        aColor = COL_GRAY7;

    return aColor;
}
}

/**
 * set attribute for the object to be created
 */
void FuConstructRectangle::SetAttributes(SfxItemSet& rAttr, SdrObject* pObj)
{
    if (nSlotId == SID_DRAW_RECT_ROUND        ||
        nSlotId == SID_DRAW_RECT_ROUND_NOFILL ||
        nSlotId == SID_DRAW_SQUARE_ROUND      ||
        nSlotId == SID_DRAW_SQUARE_ROUND_NOFILL)
    {
        // round corner
        rAttr.Put(makeSdrEckenradiusItem(500));
    }
    else if (nSlotId == SID_CONNECTOR_LINE              ||
             nSlotId == SID_CONNECTOR_LINE_ARROW_START  ||
             nSlotId == SID_CONNECTOR_LINE_ARROW_END    ||
             nSlotId == SID_CONNECTOR_LINE_ARROWS       ||
             nSlotId == SID_CONNECTOR_LINE_CIRCLE_START ||
             nSlotId == SID_CONNECTOR_LINE_CIRCLE_END   ||
             nSlotId == SID_CONNECTOR_LINE_CIRCLES)
    {
        // direct connector
        rAttr.Put(SdrEdgeKindItem(SdrEdgeKind::OneLine));
    }
    else if (nSlotId == SID_CONNECTOR_LINES              ||
             nSlotId == SID_CONNECTOR_LINES_ARROW_START  ||
             nSlotId == SID_CONNECTOR_LINES_ARROW_END    ||
             nSlotId == SID_CONNECTOR_LINES_ARROWS       ||
             nSlotId == SID_CONNECTOR_LINES_CIRCLE_START ||
             nSlotId == SID_CONNECTOR_LINES_CIRCLE_END   ||
             nSlotId == SID_CONNECTOR_LINES_CIRCLES)
    {
        // line connector
        rAttr.Put(SdrEdgeKindItem(SdrEdgeKind::ThreeLines));
    }
    else if (nSlotId == SID_CONNECTOR_CURVE              ||
             nSlotId == SID_CONNECTOR_CURVE_ARROW_START  ||
             nSlotId == SID_CONNECTOR_CURVE_ARROW_END    ||
             nSlotId == SID_CONNECTOR_CURVE_ARROWS       ||
             nSlotId == SID_CONNECTOR_CURVE_CIRCLE_START ||
             nSlotId == SID_CONNECTOR_CURVE_CIRCLE_END   ||
             nSlotId == SID_CONNECTOR_CURVE_CIRCLES)
    {
        // curve connector
        rAttr.Put(SdrEdgeKindItem(SdrEdgeKind::Bezier));
    }
    else if ( nSlotId == SID_DRAW_CAPTION || nSlotId == SID_DRAW_CAPTION_VERTICAL )
    {
        // legend object
        Size aSize(pObj->GetLogicRect().GetSize());
        rAttr.Put( makeSdrTextMinFrameHeightItem( aSize.Height() ) );
        rAttr.Put( makeSdrTextMinFrameWidthItem( aSize.Width() ) );
        rAttr.Put( makeSdrTextAutoGrowHeightItem( true ) );
        rAttr.Put( makeSdrTextAutoGrowWidthItem( true ) );

        // Support full with for vertical caption objects, too
        if(SID_DRAW_CAPTION == nSlotId)
            rAttr.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        else
            rAttr.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_BLOCK ) );

        rAttr.Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
        rAttr.Put( makeSdrTextLeftDistItem( 100 ) );
        rAttr.Put( makeSdrTextRightDistItem( 100 ) );
        rAttr.Put( makeSdrTextUpperDistItem( 100 ) );
        rAttr.Put( makeSdrTextLowerDistItem( 100 ) );
    }
    else if (nSlotId == SID_DRAW_MEASURELINE)
    {
        // dimension line
        SdPage* pPage = static_cast<SdPage*>( mpView->GetSdrPageView()->GetPage() );
        OUString aName(SdResId(STR_POOLSHEET_MEASURE));
        SfxStyleSheet* pSheet(
            static_cast< SfxStyleSheet* >(
                pPage->getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SfxStyleFamily::Para)));
        DBG_ASSERT(pSheet, "StyleSheet missing");

        if (pSheet)
        {
            pObj->SetStyleSheet(pSheet, false);
        }

        SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
        pObj->SetLayer(rAdmin.GetLayerID(sUNO_LayerName_measurelines));
    }
    else if (nSlotId == SID_DRAW_RECT)
    {
        if (mnFillTransparence > 0 && mnFillTransparence <= 100)
            rAttr.Put(XFillTransparenceItem(mnFillTransparence));
        if (!msFillColor.isEmpty())
            rAttr.Put(XFillColorItem(OUString(), strToColor(msFillColor)));
        if (!msShapeName.isEmpty())
            pObj->SetName(msShapeName);

        switch(mnLineStyle)
        {
        case 0:
            rAttr.Put( XLineStyleItem(css::drawing::LineStyle_NONE ) );
            break;
        case 1:
            rAttr.Put( XLineStyleItem(css::drawing::LineStyle_SOLID ) );
            break;
        case 2:
            rAttr.Put( XLineStyleItem(css::drawing::LineStyle_DASH ) );
            break;
        default:
            // Leave it to the defaults
            break;
        }
    }
}

/**
 * set line starts and ends for the object to be created
 */
static ::basegfx::B2DPolyPolygon getPolygon(const char* pResId, const SdrModel& rModel)
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndListRef pLineEndList(rModel.GetLineEndList());

    if( pLineEndList.is() )
    {
        OUString aArrowName(SvxResId(pResId));
        long nCount = pLineEndList->Count();
        long nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if( pEntry->GetName() == aArrowName )
            {
                aRetval = pEntry->GetLineEnd();
                break;
            }
        }
    }

    return aRetval;
}

void FuConstructRectangle::SetLineEnds(SfxItemSet& rAttr, SdrObject const & rObj)
{
    if ( !((rObj.GetObjIdentifier() == OBJ_EDGE &&
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
          nSlotId == SID_LINE_SQUARE_ARROW) )
        return;

    // set attributes of line start and ends
    SdrModel& rModel(rObj.getSdrModelFromSdrObject());

    // arrowhead
    ::basegfx::B2DPolyPolygon aArrow( getPolygon( RID_SVXSTR_ARROW, rModel ) );
    if( !aArrow.count() )
    {
        ::basegfx::B2DPolygon aNewArrow;
        aNewArrow.append(::basegfx::B2DPoint(10.0, 0.0));
        aNewArrow.append(::basegfx::B2DPoint(0.0, 30.0));
        aNewArrow.append(::basegfx::B2DPoint(20.0, 30.0));
        aNewArrow.setClosed(true);
        aArrow.append(aNewArrow);
    }

    // Circles
    ::basegfx::B2DPolyPolygon aCircle( getPolygon( RID_SVXSTR_CIRCLE, rModel ) );
    if( !aCircle.count() )
    {
        ::basegfx::B2DPolygon aNewCircle;
        aNewCircle = ::basegfx::utils::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
        aNewCircle.setClosed(true);
        aCircle.append(aNewCircle);
    }

    // Square
    ::basegfx::B2DPolyPolygon aSquare( getPolygon( RID_SVXSTR_SQUARE, rModel ) );
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

    SfxItemSet aSet( mpDoc->GetPool() );
    mpView->GetAttributes( aSet );

    // #i3908# Here, the default Line Start/End width for arrow construction is
    // set. To have the same value in all situations (construction) in i3908
    // it was decided to change the default to 0.03 cm for all situations.
    long nWidth = 300; // (1/100th mm)

    // determine line width and calculate with it the line end width
    if( aSet.GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
    {
        long nValue = aSet.Get( XATTR_LINEWIDTH ).GetValue();
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
            // connector with arrow ends
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineStartWidthItem(nWidth));
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
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
            // connector with arrow start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
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
            // connector with arrow end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_LINES_CIRCLES:
        case SID_CONNECTOR_CURVE_CIRCLES:
        {
            // connector with circle ends
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineStartWidthItem(nWidth));
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        {
            // connector with circle start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;

        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        {
            // connector with circle ends
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;
    }

    // and again, for the still missing ends
    switch (nSlotId)
    {
        case SID_LINE_ARROW_CIRCLE:
        {
            // circle end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_CIRCLE_ARROW:
        {
            // circle start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_SQUARE:
        {
            // square end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_SQUARE_ARROW:
        {
            // square start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;
    }
}

SdrObjectUniquePtr FuConstructRectangle::CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle)
{
    DBG_ASSERT( (nID != SID_DRAW_FONTWORK) && (nID != SID_DRAW_FONTWORK_VERTICAL ), "FuConstRectangle::CreateDefaultObject can not create Fontwork shapes!" );

    // case SID_DRAW_LINE:
    // case SID_DRAW_XLINE:
    // case SID_DRAW_MEASURELINE:
    // case SID_LINE_ARROW_START:
    // case SID_LINE_ARROW_END:
    // case SID_LINE_ARROWS:
    // case SID_LINE_ARROW_CIRCLE:
    // case SID_LINE_CIRCLE_ARROW:
    // case SID_LINE_ARROW_SQUARE:
    // case SID_LINE_SQUARE_ARROW:
    // case SID_DRAW_RECT:
    // case SID_DRAW_RECT_NOFILL:
    // case SID_DRAW_RECT_ROUND:
    // case SID_DRAW_RECT_ROUND_NOFILL:
    // case SID_DRAW_SQUARE:
    // case SID_DRAW_SQUARE_NOFILL:
    // case SID_DRAW_SQUARE_ROUND:
    // case SID_DRAW_SQUARE_ROUND_NOFILL:
    // case SID_DRAW_ELLIPSE:
    // case SID_DRAW_ELLIPSE_NOFILL:
    // case SID_DRAW_CIRCLE:
    // case SID_DRAW_CIRCLE_NOFILL:
    // case SID_DRAW_CAPTION:
    // case SID_DRAW_CAPTION_VERTICAL:
    // case SID_TOOL_CONNECTOR:
    // case SID_CONNECTOR_ARROW_START:
    // case SID_CONNECTOR_ARROW_END:
    // case SID_CONNECTOR_ARROWS:
    // case SID_CONNECTOR_CIRCLE_START:
    // case SID_CONNECTOR_CIRCLE_END:
    // case SID_CONNECTOR_CIRCLES:
    // case SID_CONNECTOR_LINE:
    // case SID_CONNECTOR_LINE_ARROW_START:
    // case SID_CONNECTOR_LINE_ARROW_END:
    // case SID_CONNECTOR_LINE_ARROWS:
    // case SID_CONNECTOR_LINE_CIRCLE_START:
    // case SID_CONNECTOR_LINE_CIRCLE_END:
    // case SID_CONNECTOR_LINE_CIRCLES:
    // case SID_CONNECTOR_CURVE:
    // case SID_CONNECTOR_CURVE_ARROW_START:
    // case SID_CONNECTOR_CURVE_ARROW_END:
    // case SID_CONNECTOR_CURVE_ARROWS:
    // case SID_CONNECTOR_CURVE_CIRCLE_START:
    // case SID_CONNECTOR_CURVE_CIRCLE_END:
    // case SID_CONNECTOR_CURVE_CIRCLES:
    // case SID_CONNECTOR_LINES:
    // case SID_CONNECTOR_LINES_ARROW_START:
    // case SID_CONNECTOR_LINES_ARROW_END:
    // case SID_CONNECTOR_LINES_ARROWS:
    // case SID_CONNECTOR_LINES_CIRCLE_START:
    // case SID_CONNECTOR_LINES_CIRCLE_END:
    // case SID_CONNECTOR_LINES_CIRCLES:

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->GetCurrentObjInventor(),
        mpView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        ::tools::Rectangle aRect(rRectangle);

        if(SID_DRAW_SQUARE == nID ||
            SID_DRAW_SQUARE_NOFILL == nID ||
            SID_DRAW_SQUARE_ROUND == nID ||
            SID_DRAW_SQUARE_ROUND_NOFILL == nID ||
            SID_DRAW_CIRCLE == nID ||
            SID_DRAW_CIRCLE_NOFILL == nID)
        {
            // force quadratic
            ImpForceQuadratic(aRect);
        }

        Point aStart = aRect.TopLeft();
        Point aEnd = aRect.BottomRight();

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
                if( auto pPathObj = dynamic_cast<SdrPathObj *>( pObj.get() ) )
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);

                    ::basegfx::B2DPolygon aB2DPolygon;
                    aB2DPolygon.append(::basegfx::B2DPoint(aStart.X(), nYMiddle));
                    aB2DPolygon.append(::basegfx::B2DPoint(aEnd.X(), nYMiddle));
                    pPathObj->SetPathPoly(::basegfx::B2DPolyPolygon(aB2DPolygon));
                }
                else
                {
                    OSL_FAIL("Object is NO line object");
                }

                break;
            }

            case SID_DRAW_MEASURELINE:
            {
                if( auto pMeasureObj = dynamic_cast< SdrMeasureObj *>( pObj.get() ) )
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    pMeasureObj->SetPoint(Point(aStart.X(), nYMiddle), 0);
                    pMeasureObj->SetPoint(Point(aEnd.X(), nYMiddle), 1);
                }
                else
                {
                    OSL_FAIL("Object is NO measure object");
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
                if( auto pEdgeObj = dynamic_cast< SdrEdgeObj *>( pObj.get() ) )
                {
                    pEdgeObj->SetTailPoint(false, aStart);
                    pEdgeObj->SetTailPoint(true, aEnd);
                }
                else
                {
                    OSL_FAIL("Object is NO connector object");
                }

                break;
            }
            case SID_DRAW_CAPTION:
            case SID_DRAW_CAPTION_VERTICAL:
            {
                if( auto pCaptionObj = dynamic_cast< SdrCaptionObj *>( pObj.get() ) )
                {
                    bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    pCaptionObj->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pObj->SetMergedItemSet(aSet);
                    }

                    // The default text is not inserted anymore.

                    pCaptionObj->SetLogicRect(aRect);
                    pCaptionObj->SetTailPos(
                        aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
                }
                else
                {
                    OSL_FAIL("Object is NO caption object");
                }

                break;
            }

            default:
            {
                pObj->SetLogicRect(aRect);

                break;
            }
        }

        SfxItemSet aAttr(mpDoc->GetPool());
        SetStyleSheet(aAttr, pObj.get());
        SetAttributes(aAttr, pObj.get());
        SetLineEnds(aAttr, *pObj);
        pObj->SetMergedItemSet(aAttr);
    }

    return pObj;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
