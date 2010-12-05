/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
      bSelectionChanged(FALSE)
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

BOOL FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    bMBDown = TRUE;
    bSelectionChanged = FALSE;

    if ( mpView->IsAction() )
    {
        return TRUE;
    }

    bFirstMouseMove = TRUE;
    aDragTimer.Start();

    aMDPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );
    USHORT nHitLog = USHORT (mpWindow->PixelToLogic(Size(HITPIX,0)).Width());

    if (rMEvt.IsLeft() && mpView->IsExtendedMouseEventDispatcherEnabled())
    {
        mpWindow->CaptureMouse();

        SdrHdl* pHdl = mpView->PickHandle(aMDPos);

        if ( pHdl != NULL || mpView->IsMarkedHit(aMDPos, nHitLog) )
        {
            USHORT nDrgLog = USHORT ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            bReturn = TRUE;
        }
        else if ( mpView->AreObjectsMarked() )
        {
            mpView->UnmarkAll();
            bReturn = TRUE;
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        if( bFirstMouseMove )
            bFirstMouseMove = FALSE;
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

    return TRUE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = TRUE;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = FALSE;
    }

    FuDraw::MouseButtonUp(rMEvt);

    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView && mpView->IsDragObj() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();
        BOOL bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !mpView->IsPresObjSelected(FALSE, TRUE);
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
        bReturn = FALSE;
    }

    if ( mpView &&  !mpView->IsAction() )
    {
        mpWindow->ReleaseMouse();
        USHORT nDrgLog = USHORT ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if ( !mpView->AreObjectsMarked() )
        {
            SdrObject* pObj;
            SdrPageView* pPV;
            USHORT nHitLog = USHORT ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

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
            ULONG nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

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

    USHORT nClicks = rMEvt.GetClicks();

    if (nClicks == 2 && rMEvt.IsLeft() && bMBDown &&
        !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift() )
    {
        DoubleClick(rMEvt);
    }
    bMBDown = FALSE;

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

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

    if (nSlotId == SID_DRAW_RECT         || // Rechteck
        nSlotId == SID_DRAW_RECT_ROUND   || // Rechteck, rund
        nSlotId == SID_DRAW_SQUARE       || // Quadrat
        nSlotId == SID_DRAW_SQUARE_ROUND || // Quadrat, rund
        nSlotId == SID_DRAW_ELLIPSE      || // Ellipse
        nSlotId == SID_DRAW_PIE          || // Ellipsensegment
        nSlotId == SID_DRAW_ELLIPSECUT   || // Ellipsenabschnitt
        nSlotId == SID_DRAW_CIRCLE       || // Kreis
        nSlotId == SID_DRAW_CIRCLEPIE    || // Kreissegment
        nSlotId == SID_DRAW_CIRCLECUT    || // Ellipsenabschnitt
        nSlotId == SID_DRAW_POLYGON      || // Polygon
        nSlotId == SID_DRAW_XPOLYGON     || // 45ø-Polygon
        nSlotId == SID_DRAW_FREELINE     || // Freihandlinie
        nSlotId == SID_DRAW_BEZIER_FILL)    // Bezier
    {
        bUseFillStyle = sal_True;
    }
    else if
       (nSlotId == SID_DRAW_RECT_NOFILL         || // Rechteck
        nSlotId == SID_DRAW_RECT_ROUND_NOFILL   || // Rechteck, rund
        nSlotId == SID_DRAW_SQUARE_NOFILL       || // Quadrat
        nSlotId == SID_DRAW_SQUARE_ROUND_NOFILL || // Quadrat, rund
        nSlotId == SID_DRAW_ELLIPSE_NOFILL      || // Ellipse
        nSlotId == SID_DRAW_PIE_NOFILL          || // Ellipsensegment
        nSlotId == SID_DRAW_ELLIPSECUT_NOFILL   || // Ellipsenabschnitt
        nSlotId == SID_DRAW_CIRCLE_NOFILL       || // Kreis
        nSlotId == SID_DRAW_CIRCLEPIE_NOFILL    || // Kreissegment
        nSlotId == SID_DRAW_CIRCLECUT_NOFILL    || // Ellipsenabschnitt
        nSlotId == SID_DRAW_POLYGON_NOFILL      || // Polygon
        nSlotId == SID_DRAW_XPOLYGON_NOFILL     || // 45ø-Polygon
        nSlotId == SID_DRAW_FREELINE_NOFILL     || // Freihandlinie
        nSlotId == SID_DRAW_BEZIER_NOFILL)         // Bezier
    {
        bUseNoFillStyle = sal_True;
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
        USHORT n = aName.Search(aSep);
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
            pObj->SetStyleSheet(pSheet, FALSE);
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
                pObj->SetStyleSheet(pSheet, FALSE);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
