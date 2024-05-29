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

#include <fuconstr.hxx>

#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/debug.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <docmodel/theme/Theme.hxx>

#include <app.hrc>
#include <strings.hrc>
#include <strings.hxx>
#include <fudraw.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <FrameView.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <glob.hxx>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

namespace sd {


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

bool FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    bMBDown = true;
    bSelectionChanged = false;

    if ( mpView->IsAction() )
    {
        return true;
    }

    bFirstMouseMove = true;
    aDragTimer.Start();

    aMDPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );
    sal_uInt16 nHitLog = sal_uInt16 (mpWindow->PixelToLogic(Size(HITPIX,0)).Width());

    if (rMEvt.IsLeft() && mpView->IsExtendedMouseEventDispatcherEnabled())
    {
        mpWindow->CaptureMouse();

        SdrHdl* pHdl = mpView->PickHandle(aMDPos);

        if ( pHdl != nullptr || mpView->IsMarkedHit(aMDPos, nHitLog) )
        {
            sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(mpView->GetDragThresholdPixels(),0)).Width() );
            mpView->BegDragObj(aMDPos, nullptr, pHdl, nDrgLog);
            bReturn = true;
        }
        else if ( mpView->GetMarkedObjectList().GetMarkCount() != 0 )
        {
            mpView->UnmarkAll();
            bReturn = true;
        }
    }

    return bReturn;
}

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

    Point aPix(rMEvt.GetPosPixel());
    Point aPnt( mpWindow->PixelToLogic(aPix) );

    if ( mpView->IsAction() )
    {
        ForceScroll(aPix);
        mpView->MovAction(aPnt);
    }

    return true;
}

bool FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = true;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = false;
    }

    FuDraw::MouseButtonUp(rMEvt);

    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView && mpView->IsDragObj() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();
        bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !mpView->IsPresObjSelected(false);
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
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(mpView->GetDragThresholdPixels(),0)).Width() );

        if ( mpView->GetMarkedObjectList().GetMarkCount() == 0 )
        {
            SdrPageView* pPV;
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

            SdrObject* pObj = mpView->PickObj(aPnt, mpView->getHitTolLog(), pPV);
            if (!pObj)
            {
                mpView->MarkObj(aPnt, nHitLog);
            }

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
        }
        else if (rMEvt.IsLeft() && !rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 !bSelectionChanged                   &&
                 std::abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                 std::abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // toggle between selection and rotation
            SdrObject* pSingleObj = nullptr;

            if (mpView->GetMarkedObjectList().GetMarkCount()==1)
            {
                pSingleObj = mpView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            }

            const bool bTiledRendering = comphelper::LibreOfficeKit::isActive();
            if (!bTiledRendering && (mpView->GetDragMode() == SdrDragMode::Move && mpView->IsRotateAllowed() &&
                (mpViewShell->GetFrameView()->IsClickChangeRotation() ||
                 (pSingleObj && pSingleObj->GetObjInventor()==SdrInventor::E3d))))
            {
                mpView->SetDragMode(SdrDragMode::Rotate);
            }
            else
            {
                mpView->SetDragMode(SdrDragMode::Move);
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

void FuConstruct::Activate()
{
    mpView->SetEditMode(SdrViewEditMode::Create);
    FuDraw::Activate();
}

void FuConstruct::Deactivate()
{
    FuDraw::Deactivate();
    mpView->SetEditMode(SdrViewEditMode::Edit);
}

bool FuConstruct::IsIgnoreUnexpectedMouseButtonUp()
{
    // tdf#153446 if there is a MouseButtonUp without a previous MouseButtonDown event,
    // the MouseButtonDown was probably swallowed by a gain-focus action,
    // and then this MouseButtonUp should be ignored

    if (bMBDown || bIsInDragMode)
        return false;

    // Don't ignore if there are pending mouse-initiated tasks to complete.
    return !mpView->IsDragObj() && !mpWindow->IsMouseCaptured() && !mpView->IsAction();
}

/**
 * set style sheet for the object to be created
 */
void FuConstruct::SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj)
{
    bool bUseFillStyle, bUseNoFillStyle;
    bUseFillStyle = bUseNoFillStyle = false;

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
        bUseFillStyle = true;
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
        bUseNoFillStyle = true;
        break;
    }
    }
    SetStyleSheet( rAttr, pObj, bUseFillStyle, bUseNoFillStyle );
}

void FuConstruct::SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
        const bool bForceFillStyle, const bool bForceNoFillStyle )
{
    SdPage* pPage = static_cast<SdPage*>(mpView->GetSdrPageView()->GetPage());
    if ( pPage->IsMasterPage() && pPage->GetPageKind() == PageKind::Standard &&
         mpDoc->GetDocumentType() == DocumentType::Impress )
    {
        /**********************************************
        * Objects was created on the slide master page
        ***********************************************/
        OUString aName( pPage->GetLayoutName() );
        sal_Int32 n = aName.indexOf(SD_LT_SEPARATOR) + SD_LT_SEPARATOR.getLength();
        aName = OUString::Concat(aName.subView(0, n)) + STR_LAYOUT_BACKGROUNDOBJECTS;
        SfxStyleSheet* pSheet(
            static_cast< SfxStyleSheet* >(
                pPage->getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SfxStyleFamily::Page)));
        DBG_ASSERT(pSheet, "StyleSheet missing");
        if (pSheet)
        {
            // applying style sheet for background objects
            pObj->SetStyleSheet(pSheet, false);
            SfxItemSet& rSet = pSheet->GetItemSet();
            const XFillStyleItem& rFillStyle = rSet.Get(XATTR_FILLSTYLE);
            if ( bForceFillStyle )
            {
                if (rFillStyle.GetValue() == drawing::FillStyle_NONE)
                    rAttr.Put(XFillStyleItem(drawing::FillStyle_SOLID));
            }
            else if ( bForceNoFillStyle )
            {
                if (rFillStyle.GetValue() != drawing::FillStyle_NONE)
                    rAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
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
            OUString aName(SdResId(STR_POOLSHEET_OBJWITHOUTFILL));
            SfxStyleSheet* pSheet(
                static_cast< SfxStyleSheet* >(
                    pPage->getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SfxStyleFamily::Para)));
            DBG_ASSERT(pSheet, "Stylesheet missing");
            if (pSheet)
            {
                pObj->SetStyleSheet(pSheet, false);
                SfxItemSet aAttr(mpView->GetDefaultAttr());
                aAttr.Put(pSheet->GetItemSet().Get(XATTR_FILLSTYLE));
                pObj->SetMergedItemSet(aAttr);
            }
            else
            {
                SfxItemSet aAttr(mpView->GetDefaultAttr());
                rAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
                pObj->SetMergedItemSet(aAttr);
            }
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
