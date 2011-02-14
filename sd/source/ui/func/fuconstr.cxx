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

TYPEINIT1( FuConstruct, FuDraw );

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
      bSelectionChanged(sal_False)
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

sal_Bool FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    bMBDown = sal_True;
    bSelectionChanged = sal_False;

    if ( mpView->IsAction() )
    {
        // #90235# this extra triggering is an error and leads to
        // erasing the last two points when creating a polygon.
        // if ( rMEvt.IsRight() )
        //  mpView->BckAction();
        return sal_True;
    }

    bFirstMouseMove = sal_True;
    aDragTimer.Start();

    aMDPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );
    sal_uInt16 nHitLog = sal_uInt16 (mpWindow->PixelToLogic(Size(HITPIX,0)).Width());

    if (rMEvt.IsLeft() && mpView->IsExtendedMouseEventDispatcherEnabled())
    {
        mpWindow->CaptureMouse();

        SdrHdl* pHdl = mpView->PickHandle(aMDPos);

        if ( pHdl != NULL || mpView->IsMarkedHit(aMDPos, nHitLog) )
        {
            sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            bReturn = sal_True;
        }
        else if ( mpView->AreObjectsMarked() )
        {
            mpView->UnmarkAll();
            bReturn = sal_True;
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        if( bFirstMouseMove )
            bFirstMouseMove = sal_False;
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

    return sal_True;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_True;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = sal_False;
    }

    FuDraw::MouseButtonUp(rMEvt);

    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView && mpView->IsDragObj() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();
        sal_Bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !mpView->IsPresObjSelected(sal_False, sal_True);
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
        bReturn = sal_False;
    }

    if ( mpView &&  !mpView->IsAction() )
    {
        mpWindow->ReleaseMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if ( !mpView->AreObjectsMarked() )
        {
            SdrObject* pObj;
            SdrPageView* pPV;
            sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

            if (!mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, pPV))
            {
                mpView->MarkObj(aPnt, nHitLog);
            }

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
        }
        else if (rMEvt.IsLeft() && !rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 !bSelectionChanged                   &&
                 Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                 Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            /**************************************************************
            * Toggle zw. Selektion und Rotation
            **************************************************************/
            SdrObject* pSingleObj = NULL;
            sal_uLong nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

            if (nMarkCount==1)
            {
                pSingleObj = mpView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            }

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
    bMBDown = sal_False;

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

sal_Bool FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

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
    mpView->SetEditMode(SDREDITMODE_CREATE);
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
    mpView->SetEditMode(SDREDITMODE_EDIT);
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
    SdPage* pPage = (SdPage*)mpView->GetSdrPageView()->GetPage();
    if ( pPage->IsMasterPage() && pPage->GetPageKind() == PK_STANDARD &&
         mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        /**********************************************
        * Objects was created on the slide master page
        ***********************************************/
        String aName( pPage->GetLayoutName() );
        String aSep = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ) );
        sal_uInt16 n = aName.Search(aSep);
        n = n + aSep.Len();
        aName.Erase(n);
        aName.Append( String ( SdResId( STR_LAYOUT_BACKGROUNDOBJECTS ) ) );
        SfxStyleSheet* pSheet = (SfxStyleSheet*)pPage->GetModel()->
                                                GetStyleSheetPool()->
                                                Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
        DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");
        if (pSheet)
        {
            // applying style sheet for background objects
            pObj->SetStyleSheet(pSheet, sal_False);
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
            SfxStyleSheet* pSheet = (SfxStyleSheet*)pPage->GetModel()->
                                         GetStyleSheetPool()->
                                         Find(aName, SD_STYLE_FAMILY_GRAPHICS);
            DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");
            if (pSheet)
            {
                pObj->SetStyleSheet(pSheet, sal_False);
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

} // end of namespace sd
