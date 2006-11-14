/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuconstr.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:28:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "fuconstr.hxx"

#include <svx/svxids.hrc>
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#include "app.hrc"
#include "glob.hrc"
#ifndef SD_FU_DRAW_HXX
#include "fudraw.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
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

FunctionReference FuConstruct::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuConstruct( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
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

    if ( pView->IsAction() )
    {
        // #90235# this extra triggering is an error and leads to
        // erasing the last two points when creating a polygon.
        // if ( rMEvt.IsRight() )
        //  pView->BckAction();
        return TRUE;
    }

    bFirstMouseMove = TRUE;
    aDragTimer.Start();

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );
    USHORT nHitLog = USHORT (pWindow->PixelToLogic(Size(HITPIX,0)).Width());

    if (rMEvt.IsLeft() && pView->IsExtendedMouseEventDispatcherEnabled())
    {
        pWindow->CaptureMouse();

        SdrHdl* pHdl = pView->PickHandle(aMDPos);

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos, nHitLog) )
        {
            USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            bReturn = TRUE;
        }
        else if ( pView->AreObjectsMarked() )
        {
            pView->UnmarkAll();
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
    Point aPnt( pWindow->PixelToLogic(aPix) );

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aPnt);
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

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView && pView->IsDragObj() )
    {
        FrameView* pFrameView = pViewShell->GetFrameView();
        BOOL bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !pView->IsPresObjSelected(FALSE, TRUE);
        }

        pView->SetDragWithCopy(bDragWithCopy);
        pView->EndDragObj( pView->IsDragWithCopy() );
    }
    else if ( pView && pView->IsMarkObj() )
    {
        pView->EndMarkObj();
    }
    else
    {
        bReturn = FALSE;
    }

    if ( pView &&  !pView->IsAction() )
    {
        pWindow->ReleaseMouse();
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        SdrHdl* pHdl = pView->PickHandle(aPnt);

        if ( !pView->AreObjectsMarked() )
        {
            SdrObject* pObj;
            SdrPageView* pPV;
            USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );

            if (!pView->PickObj(aPnt, pObj, pPV))
            {
                pView->MarkObj(aPnt, nHitLog);
            }

            pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
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
            ULONG nMarkCount = pView->GetMarkedObjectList().GetMarkCount();

            if (nMarkCount==1)
            {
                pSingleObj = pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            }

            if (pView->GetDragMode() == SDRDRAG_MOVE && pView->IsRotateAllowed() &&
                (pViewShell->GetFrameView()->IsClickChangeRotation() ||
                 (pSingleObj && pSingleObj->GetObjInventor()==E3dInventor)))
            {
                pView->SetDragMode(SDRDRAG_ROTATE);
            }
            else
            {
                pView->SetDragMode(SDRDRAG_MOVE);
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
    pView->SetEditMode(SDREDITMODE_CREATE);
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
    pView->SetEditMode(SDREDITMODE_EDIT);
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
    SdPage* pPage = (SdPage*)pView->GetSdrPageView()->GetPage();
    if ( pPage->IsMasterPage() && pPage->GetPageKind() == PK_STANDARD &&
         pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        /**********************************************
        * Objects was created on the slide master page
        ***********************************************/
        String aName( pPage->GetLayoutName() );
        String aSep = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ) );
        USHORT n = aName.Search(aSep);
        n += aSep.Len();
        aName.Erase(n);
        aName.Append( String ( SdResId( STR_LAYOUT_BACKGROUNDOBJECTS ) ) );
        SfxStyleSheet* pSheet = (SfxStyleSheet*)pPage->GetModel()->
                                                GetStyleSheetPool()->
                                                Find(aName, SD_LT_FAMILY);
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
                                         Find(aName, SFX_STYLE_FAMILY_PARA);
            DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");
            if (pSheet)
            {
                pObj->SetStyleSheet(pSheet, FALSE);
                SfxItemSet aAttr(*pView->GetDefaultAttr().Clone());
                aAttr.Put(pSheet->GetItemSet().Get(XATTR_FILLSTYLE));
                pObj->SetMergedItemSet(aAttr);
            }
            else
            {
                SfxItemSet aAttr(*pView->GetDefaultAttr().Clone());
                rAttr.Put(XFillStyleItem(XFILL_NONE));
                pObj->SetMergedItemSet(aAttr);
            }
        }
    }
}

} // end of namespace sd
