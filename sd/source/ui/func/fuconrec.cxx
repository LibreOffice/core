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
#include <svx/sdtmfitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sderitm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/adjitem.hxx>
#include <svx/xtable.hxx>

// #88751#
#include <svx/svdocapt.hxx>

// #97016#
#include <svx/svdomeas.hxx>
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
// #109583#
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

TYPEINIT1( FuConstructRectangle, FuConstruct );

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
                SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, sal_False);
                SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, sal_False);
                SFX_REQUEST_ARG (rReq, pAxisX, SfxUInt32Item, ID_VAL_AXIS_X, sal_False);
                SFX_REQUEST_ARG (rReq, pAxisY, SfxUInt32Item, ID_VAL_AXIS_Y, sal_False);

                Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () - pAxisY->GetValue () / 2,
                                           pCenterX->GetValue () + pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () + pAxisY->GetValue () / 2);
                SdrCircObj  *pNewCircle = new SdrCircObj (OBJ_CIRC, aNewRectangle);
                SdrPageView *pPV = mpView->GetSdrPageView();

                mpView->InsertObjectAtView(pNewCircle, *pPV, SDRINSERT_SETDEFLAYER | SDRINSERT_SETDEFATTR);
            }
            break;

            case SID_DRAW_RECT :
            {
                SFX_REQUEST_ARG (rReq, pMouseStartX, SfxUInt32Item, ID_VAL_MOUSESTART_X, sal_False);
                SFX_REQUEST_ARG (rReq, pMouseStartY, SfxUInt32Item, ID_VAL_MOUSESTART_Y, sal_False);
                SFX_REQUEST_ARG (rReq, pMouseEndX, SfxUInt32Item, ID_VAL_MOUSEEND_X, sal_False);
                SFX_REQUEST_ARG (rReq, pMouseEndY, SfxUInt32Item, ID_VAL_MOUSEEND_Y, sal_False);

                Rectangle   aNewRectangle (pMouseStartX->GetValue (),
                                           pMouseStartY->GetValue (),
                                           pMouseEndX->GetValue (),
                                           pMouseEndY->GetValue ());
                SdrRectObj  *pNewRect = new SdrRectObj (aNewRectangle);
                SdrPageView *pPV = mpView->GetSdrPageView();

                mpView->InsertObjectAtView(pNewRect, *pPV, SDRINSERT_SETDEFLAYER | SDRINSERT_SETDEFATTR);
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

sal_Bool FuConstructRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if (mpView->GetCurrentObjIdentifier() == OBJ_CAPTION)
        {
            Size aCaptionSize(846, 846);    // (4x2)cm
            bReturn = mpView->BegCreateCaptionObj(aPnt, aCaptionSize,
                                                (OutputDevice*) NULL, nDrgLog);
        }
        else
        {
            mpView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);
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
                ( (SdrTextObj*) pObj)->SetVerticalWriting( sal_True );
        }
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuConstructRectangle::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuConstructRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn(sal_False);

    if(mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        SdrObject* pObj = mpView->GetCreateObj();

        if(pObj && mpView->EndCreateObj(SDRCREATE_FORCEEND))
        {
            if(SID_DRAW_MEASURELINE == nSlotId)
            {
                SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
                String aStr(SdResId(STR_LAYER_MEASURELINES));
                pObj->SetLayer(rAdmin.GetLayerID(aStr, sal_False));
            }

            // #88751# init text position when vertica caption object is created
            if(pObj->ISA(SdrCaptionObj) && SID_DRAW_CAPTION_VERTICAL == nSlotId)
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

            bReturn = sal_True;
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

sal_Bool FuConstructRectangle::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

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
            // keine break !
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
        mpView->SetGlueVisible( sal_False );
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
        rAttr.Put(SdrEckenradiusItem(500));
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
        Size aSize(pObj->GetLogicRect().GetSize());
        rAttr.Put( SdrTextMinFrameHeightItem( aSize.Height() ) );
        rAttr.Put( SdrTextMinFrameWidthItem( aSize.Width() ) );
        rAttr.Put( SdrTextAutoGrowHeightItem( sal_True ) );
        rAttr.Put( SdrTextAutoGrowWidthItem( sal_True ) );

        // #103516# Support full with for vertical caption objects, too
        if(SID_DRAW_CAPTION == nSlotId)
            rAttr.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        else
            rAttr.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_BLOCK ) );

        rAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
        rAttr.Put( SdrTextLeftDistItem( 100 ) );
        rAttr.Put( SdrTextRightDistItem( 100 ) );
        rAttr.Put( SdrTextUpperDistItem( 100 ) );
        rAttr.Put( SdrTextLowerDistItem( 100 ) );
    }
    else if (nSlotId == SID_DRAW_MEASURELINE)
    {
        /**********************************************************************
        * Masslinie
        **********************************************************************/
        SdPage* pPage = (SdPage*) mpView->GetSdrPageView()->GetPage();
        String aName(SdResId(STR_POOLSHEET_MEASURE));
        SfxStyleSheet* pSheet = (SfxStyleSheet*) pPage->GetModel()->
                                     GetStyleSheetPool()->
                                     Find(aName, SD_STYLE_FAMILY_GRAPHICS);
        DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");

        if (pSheet)
        {
            pObj->SetStyleSheet(pSheet, sal_False);
        }

        SdrLayerAdmin& rAdmin = mpDoc->GetLayerAdmin();
        String aStr(SdResId(STR_LAYER_MEASURELINES));
        pObj->SetLayer(rAdmin.GetLayerID(aStr, sal_False));
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

        SfxItemSet aSet( mpDoc->GetPool() );
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
        0L, mpDoc);

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
                if(pObj->ISA(SdrPathObj))
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);

                    ::basegfx::B2DPolygon aB2DPolygon;
                    aB2DPolygon.append(::basegfx::B2DPoint(aStart.X(), nYMiddle));
                    aB2DPolygon.append(::basegfx::B2DPoint(aEnd.X(), nYMiddle));
                    ((SdrPathObj*)pObj)->SetPathPoly(::basegfx::B2DPolyPolygon(aB2DPolygon));
                }
                else
                {
                    DBG_ERROR("Object is NO line object");
                }

                break;
            }

            case SID_DRAW_MEASURELINE:
            {
                if(pObj->ISA(SdrMeasureObj))
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    ((SdrMeasureObj*)pObj)->SetPoint(Point(aStart.X(), nYMiddle), 0);
                    ((SdrMeasureObj*)pObj)->SetPoint(Point(aEnd.X(), nYMiddle), 1);
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
                if(pObj->ISA(SdrEdgeObj))
                {
                    ((SdrEdgeObj*)pObj)->SetTailPoint(sal_False, aStart);
                    ((SdrEdgeObj*)pObj)->SetTailPoint(sal_True, aEnd);
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
                if(pObj->ISA(SdrCaptionObj))
                {
                    sal_Bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    ((SdrTextObj*)pObj)->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pObj->SetMergedItemSet(aSet);
                    }

                    // For task #105815# the default text is not inserted anymore.
                    //  String aText(SdResId(STR_POOLSHEET_TEXT));
                    //  ((SdrCaptionObj*)pObj)->SetText(aText);

                    ((SdrCaptionObj*)pObj)->SetLogicRect(aRect);
                    ((SdrCaptionObj*)pObj)->SetTailPos(
                        aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
                }
                else
                {
                    DBG_ERROR("Object is NO caption object");
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
