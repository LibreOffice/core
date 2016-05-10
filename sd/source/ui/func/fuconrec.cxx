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

#include <svx/svdocapt.hxx>

#include <svx/svdomeas.hxx>
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include <editeng/writingmodeitem.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include "sdresid.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "stlpool.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "glob.hrc"

namespace sd {


FuConstructRectangle::FuConstructRectangle (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuConstructRectangle::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructRectangle* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstructRectangle( pViewSh, pWin, pView, pDoc, rReq ) );
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
                const SfxUInt32Item* pCenterX = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_X);
                const SfxUInt32Item* pCenterY = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_Y);
                const SfxUInt32Item* pAxisX = rReq.GetArg<SfxUInt32Item>(ID_VAL_AXIS_X);
                const SfxUInt32Item* pAxisY = rReq.GetArg<SfxUInt32Item>(ID_VAL_AXIS_Y);

                Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () - pAxisY->GetValue () / 2,
                                           pCenterX->GetValue () + pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () + pAxisY->GetValue () / 2);
                SdrCircObj  *pNewCircle = new SdrCircObj (OBJ_CIRC, aNewRectangle);
                SdrPageView *pPV = mpView->GetSdrPageView();

                mpView->InsertObjectAtView(pNewCircle, *pPV, SdrInsertFlags::SETDEFLAYER | SdrInsertFlags::SETDEFATTR);
            }
            break;

            case SID_DRAW_RECT :
            {
                const SfxUInt32Item* pMouseStartX = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSESTART_X);
                const SfxUInt32Item* pMouseStartY = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSESTART_Y);
                const SfxUInt32Item* pMouseEndX = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSEEND_X);
                const SfxUInt32Item* pMouseEndY = rReq.GetArg<SfxUInt32Item>(ID_VAL_MOUSEEND_Y);

                Rectangle   aNewRectangle (pMouseStartX->GetValue (),
                                           pMouseStartY->GetValue (),
                                           pMouseEndX->GetValue (),
                                           pMouseEndY->GetValue ());
                SdrRectObj  *pNewRect = new SdrRectObj (aNewRectangle);
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
            SetLineEnds(aAttr, pObj);
            pObj->SetMergedItemSet(aAttr);

            if( nSlotId == SID_DRAW_CAPTION_VERTICAL )
                static_cast<SdrTextObj*>(pObj)->SetVerticalWriting( true );
        }
    }
    return bReturn;
}

bool FuConstructRectangle::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

bool FuConstructRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn(false);

    if(mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();

        if(pObj && mpView->EndCreateObj(SDRCREATE_FORCEEND))
        {
            if(SID_DRAW_MEASURELINE == nSlotId)
            {
                SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
                OUString aStr(SD_RESSTR(STR_LAYER_MEASURELINES));
                pObj->SetLayer(rAdmin.GetLayerID(aStr, false));
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

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuConstructRectangle::KeyInput(const KeyEvent& rKEvt)
{
    return FuConstruct::KeyInput(rKEvt);
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
            SAL_FALLTHROUGH;
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

    mpView->SetCurrentObj((sal_uInt16)aObjKind);

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
        // line connector
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
        // curve connector
        rAttr.Put(SdrEdgeKindItem(SDREDGE_BEZIER));
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

        rAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
        rAttr.Put( makeSdrTextLeftDistItem( 100 ) );
        rAttr.Put( makeSdrTextRightDistItem( 100 ) );
        rAttr.Put( makeSdrTextUpperDistItem( 100 ) );
        rAttr.Put( makeSdrTextLowerDistItem( 100 ) );
    }
    else if (nSlotId == SID_DRAW_MEASURELINE)
    {
        // dimension line
        SdPage* pPage = static_cast<SdPage*>( mpView->GetSdrPageView()->GetPage() );
        OUString aName(SD_RESSTR(STR_POOLSHEET_MEASURE));
        SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>( pPage->GetModel()->
                                     GetStyleSheetPool()->
                                     Find(aName, SD_STYLE_FAMILY_GRAPHICS));
        DBG_ASSERT(pSheet, "StyleSheet missing");

        if (pSheet)
        {
            pObj->SetStyleSheet(pSheet, false);
        }

        SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
        OUString aStr(SD_RESSTR(STR_LAYER_MEASURELINES));
        pObj->SetLayer(rAdmin.GetLayerID(aStr, false));
    }
    else if (nSlotId == OBJ_CUSTOMSHAPE )
    {
    }
}

/**
 * set line starts and ends for the object to be created
 */
::basegfx::B2DPolyPolygon getPolygon( sal_uInt16 nResId, SdrModel* pDoc )
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndListRef pLineEndList = pDoc->GetLineEndList();

    if( pLineEndList.is() )
    {
        OUString aArrowName( SVX_RES(nResId) );
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
        // set attributes of line start and ends

        // arrowhead
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

        // Circles
        ::basegfx::B2DPolyPolygon aCircle( getPolygon( RID_SVXSTR_CIRCLE, mpDoc ) );
        if( !aCircle.count() )
        {
            ::basegfx::B2DPolygon aNewCircle;
            aNewCircle = ::basegfx::tools::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
            aNewCircle.setClosed(true);
            aCircle.append(aNewCircle);
        }

        // Square
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

        SfxItemSet aSet( mpDoc->GetPool() );
        mpView->GetAttributes( aSet );

        // #i3908# Here, the default Line Start/End width for arrow construction is
        // set. To have the same value in all situations (construction) in i3908
        // it was decided to change the default to 0.03 cm for all situations.
        long nWidth = 300; // (1/100th mm)

        // determine line width and calculate with it the line end width
        if( aSet.GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
        {
            long nValue = static_cast<const XLineWidthItem&>( aSet.Get( XATTR_LINEWIDTH ) ).GetValue();
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
                // connector with arrow start
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
                // connector with arrow end
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_CIRCLES:
            case SID_CONNECTOR_LINE_CIRCLES:
            case SID_CONNECTOR_LINES_CIRCLES:
            case SID_CONNECTOR_CURVE_CIRCLES:
            {
                // connector with circle ends
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
                // connector with circle start
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_CONNECTOR_CIRCLE_END:
            case SID_CONNECTOR_LINE_CIRCLE_END:
            case SID_CONNECTOR_LINES_CIRCLE_END:
            case SID_CONNECTOR_CURVE_CIRCLE_END:
            {
                // connector with circle ends
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
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
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_CIRCLE_ARROW:
            {
                // circle start
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_SQUARE:
            {
                // square end
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_SQUARE_ARROW:
            {
                // square start
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;
        }
    }
}

SdrObject* FuConstructRectangle::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
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

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        nullptr, mpDoc);

    if(pObj)
    {
        Rectangle aRect(rRectangle);

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
                if( dynamic_cast< const SdrPathObj *>( pObj ) !=  nullptr)
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);

                    ::basegfx::B2DPolygon aB2DPolygon;
                    aB2DPolygon.append(::basegfx::B2DPoint(aStart.X(), nYMiddle));
                    aB2DPolygon.append(::basegfx::B2DPoint(aEnd.X(), nYMiddle));
                    static_cast<SdrPathObj*>(pObj)->SetPathPoly(::basegfx::B2DPolyPolygon(aB2DPolygon));
                }
                else
                {
                    OSL_FAIL("Object is NO line object");
                }

                break;
            }

            case SID_DRAW_MEASURELINE:
            {
                if( dynamic_cast< SdrMeasureObj *>( pObj ) !=  nullptr)
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    static_cast<SdrMeasureObj*>(pObj)->SetPoint(Point(aStart.X(), nYMiddle), 0);
                    static_cast<SdrMeasureObj*>(pObj)->SetPoint(Point(aEnd.X(), nYMiddle), 1);
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
                if( dynamic_cast< SdrEdgeObj *>( pObj ) !=  nullptr)
                {
                    static_cast<SdrEdgeObj*>(pObj)->SetTailPoint(false, aStart);
                    static_cast<SdrEdgeObj*>(pObj)->SetTailPoint(true, aEnd);
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
                if( dynamic_cast< SdrCaptionObj *>( pObj ) !=  nullptr)
                {
                    bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    static_cast<SdrTextObj*>(pObj)->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pObj->SetMergedItemSet(aSet);
                    }

                    // The default text is not inserted anymore.

                    static_cast<SdrCaptionObj*>(pObj)->SetLogicRect(aRect);
                    static_cast<SdrCaptionObj*>(pObj)->SetTailPos(
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
        SetStyleSheet(aAttr, pObj);
        SetAttributes(aAttr, pObj);
        SetLineEnds(aAttr, pObj);
        pObj->SetMergedItemSet(aAttr);
    }

    return pObj;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
