/*************************************************************************
 *
 *  $RCSfile: fuconstr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tbe $ $Date: 2000-11-10 16:21:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#include "fudraw.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "viewshel.hxx"
#include "drawdoc.hxx"
#include "fuconstr.hxx"
#include "frmview.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "stlpool.hxx"
#include <svx/globl3d.hxx>

TYPEINIT1( FuConstruct, FuDraw );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct::FuConstruct(SdViewShell*       pViewSh,
                         SdWindow*          pWin,
                         SdView*            pView,
                         SdDrawDocument*    pDoc,
                         SfxRequest&        rReq) :
    FuDraw(pViewSh, pWin, pView, pDoc, rReq),
    bSelectionChanged(FALSE)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstruct::~FuConstruct()
{
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
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return TRUE;
    }

    bFirstMouseMove = TRUE;
    aDragTimer.Start();

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );
    USHORT nHitLog = USHORT (pWindow->PixelToLogic(Size(HITPIX,0)).Width());

    if (rMEvt.IsLeft() && pView->IsExtendedMouseEventDispatcherEnabled())
    {
        pWindow->CaptureMouse();

        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos, nHitLog) )
        {
            USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            bReturn = TRUE;
        }
        else if ( pView->HasMarkedObj() )
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

    if ( pView->IsDragObj() )
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
    else if ( pView->IsMarkObj() )
    {
        pView->EndMarkObj();
    }
    else
    {
        bReturn = FALSE;
    }

    if ( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        SdrHdl* pHdl = pView->HitHandle(aPnt, *pWindow);

        if ( !pView->HasMarkedObj() )
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
            ULONG nMarkCount = pView->GetMarkList().GetMarkCount();

            if (nMarkCount==1)
            {
                pSingleObj = pView->GetMarkList().GetMark(0)->GetObj();
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
    SdPage* pPage = (SdPage*) pView->GetPageViewPvNum(0)->GetPage();

    if (pPage->IsMasterPage() && pPage->GetPageKind() == PK_STANDARD)
    {
        /******************************************************************
        * Objekt wurde auf Hintergrund erzeugt
        ******************************************************************/
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
            // Vorlage fuer Hintergrundobjekte zuweisen
            pObj->SetStyleSheet(pSheet, FALSE);

            SfxItemSet& rSet = pSheet->GetItemSet();
            const XFillStyleItem& rFillStyle = (const XFillStyleItem&)
                                               rSet.Get(XATTR_FILLSTYLE);

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
                /**********************************************************
                * Gefuellte Objekte
                **********************************************************/
                if (rFillStyle.GetValue() == XFILL_NONE)
                {
                    // Vorlage hat keine Fuellung, daher hart attributieren:
                    // Fuellung setzen
                    rAttr.Put(XFillStyleItem(XFILL_SOLID));
                }
            }
            else if (nSlotId == SID_DRAW_RECT_NOFILL         || // Rechteck
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
                /**********************************************************
                * Ungefuellte Objekte
                **********************************************************/
                if (rFillStyle.GetValue() != XFILL_NONE)
                {
                    // Vorlage hat eine Fuellung, daher hart attributieren:
                    // Keine Fuellung setzen
                    rAttr.Put(XFillStyleItem(XFILL_NONE));
                }
            }
        }
    }
    else
    {
        /******************************************************************
        * Objekt wurde auf Seite  erzeugt
        ******************************************************************/
        if (nSlotId == SID_DRAW_RECT_NOFILL         || // Rechteck
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
            /**************************************************************
            * Ungefuellte Objekte:
            * Vorlage fuer ungefuellte Objekte zuweisen
            **************************************************************/
            String aName(SdResId(STR_POOLSHEET_OBJWITHOUTFILL));
            SfxStyleSheet* pSheet = (SfxStyleSheet*)pPage->GetModel()->
                                         GetStyleSheetPool()->
                                         Find(aName, SFX_STYLE_FAMILY_PARA);
            DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");

            if (pSheet)
            {
                pObj->SetStyleSheet(pSheet, FALSE);

                // Default-Attr. und Fuellung aus Vorlage zuweisen
                SfxItemSet aAttr(*pView->GetDefaultAttr().Clone());
                aAttr.Put(pSheet->GetItemSet().Get(XATTR_FILLSTYLE));

//-/                pObj->NbcSetAttributes(aAttr, FALSE);
                pObj->SetItemSet(aAttr);
            }
            else
            {
                // Default-Attr. und "Leere-Fuellung" zuweisen
                SfxItemSet aAttr(*pView->GetDefaultAttr().Clone());
                rAttr.Put(XFillStyleItem(XFILL_NONE));

//-/                pObj->NbcSetAttributes(aAttr, FALSE);
                pObj->SetItemSet(aAttr);
            }
        }
    }
}




