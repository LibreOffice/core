/*************************************************************************
 *
 *  $RCSfile: fuconrec.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:46:16 $
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

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#pragma hdrstop

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include "app.hrc"
//#include <basctl/idetemp.hxx>
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include <svx/xlnstwit.hxx>
#endif
#ifndef _SVX_XLNEDWIT_HXX //autogen
#include <svx/xlnedwit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX //autogen
#include <svx/xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX //autogen
#include <svx/xlnstit.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SDTMFITM_HXX //autogen
#include <svx/sdtmfitm.hxx>
#endif
#ifndef _SXEKITM_HXX //autogen
#include <svx/sxekitm.hxx>
#endif
#ifndef _SDERITM_HXX //autogen
#include <svx/sderitm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SVDOCIRC_HXX //autogen
#include <svx/svdocirc.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif


#include "viewshel.hxx"
#include "sdresid.hxx"
#include "sdview.hxx"
#include "sdpage.hxx"
#include "sdwindow.hxx"
#include "stlpool.hxx"
#include "drawdoc.hxx"
#include "fuconrec.hxx"
#include "res_bmp.hrc"
#include "glob.hrc"

TYPEINIT1( FuConstRectangle, FuConstruct );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstRectangle::FuConstRectangle(SdViewShell*     pViewSh,
                                   SdWindow*        pWin,
                                   SdView*          pView,
                                   SdDrawDocument*  pDoc,
                                   SfxRequest&      rReq) :
      FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
    pViewShell->SwitchObjectBar(RID_DRAW_OBJ_TOOLBOX);

    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        switch (nSlotId)
        {
            case SID_DRAW_ELLIPSE :
            {
                SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, FALSE);
                SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, FALSE);
                SFX_REQUEST_ARG (rReq, pAxisX, SfxUInt32Item, ID_VAL_AXIS_X, FALSE);
                SFX_REQUEST_ARG (rReq, pAxisY, SfxUInt32Item, ID_VAL_AXIS_Y, FALSE);

                Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () - pAxisY->GetValue () / 2,
                                           pCenterX->GetValue () + pAxisX->GetValue () / 2,
                                           pCenterY->GetValue () + pAxisY->GetValue () / 2);
                SdrCircObj  *pNewCircle = new SdrCircObj (OBJ_CIRC, aNewRectangle);
                SdrPageView *pPV = pView->GetPageViewPvNum(0);

                pView->InsertObject (pNewCircle, *pPV, SDRINSERT_SETDEFLAYER |
                                                       SDRINSERT_SETDEFATTR);
            }
            break;

            case SID_DRAW_RECT :
            {
                SFX_REQUEST_ARG (rReq, pMouseStartX, SfxUInt32Item, ID_VAL_MOUSESTART_X, FALSE);
                SFX_REQUEST_ARG (rReq, pMouseStartY, SfxUInt32Item, ID_VAL_MOUSESTART_Y, FALSE);
                SFX_REQUEST_ARG (rReq, pMouseEndX, SfxUInt32Item, ID_VAL_MOUSEEND_X, FALSE);
                SFX_REQUEST_ARG (rReq, pMouseEndY, SfxUInt32Item, ID_VAL_MOUSEEND_Y, FALSE);

                Rectangle   aNewRectangle (pMouseStartX->GetValue (),
                                           pMouseStartY->GetValue (),
                                           pMouseEndX->GetValue (),
                                           pMouseEndY->GetValue ());
                SdrRectObj  *pNewRect = new SdrRectObj (aNewRectangle);
                SdrPageView *pPV = pView->GetPageViewPvNum (0);

                pView->InsertObject (pNewRect, *pPV, SDRINSERT_SETDEFLAYER |
                                                     SDRINSERT_SETDEFATTR);
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
        pView->UnmarkAll();
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstRectangle::~FuConstRectangle()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        pWindow->CaptureMouse();
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if (pView->GetCurrentObjIdentifier() == OBJ_CAPTION)
        {
            Size aCaptionSize(846, 846);    // (4x2)cm
            bReturn = pView->BegCreateCaptionObj(aPnt, aCaptionSize,
                                                 (OutputDevice*) NULL, nDrgLog);
        }
        else
        {
            pView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);
        }

        SdrObject* pObj = pView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pDoc->GetPool());
            SetStyleSheet(aAttr, pObj);
            SetAttributes(aAttr, pObj);
            SetLineEnds(aAttr, pObj);

//-/            pObj->NbcSetAttributes(aAttr, FALSE);
            pObj->SetItemSet(aAttr);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstRectangle::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        SdrObject* pObj = pView->GetCreateObj();

        if (pView->EndCreateObj(SDRCREATE_FORCEEND) &&
            pObj && nSlotId == SID_DRAW_MEASURELINE)
        {
            SdrLayerAdmin& rAdmin = pDoc->GetLayerAdmin();
            String aStr(SdResId(STR_LAYER_MEASURELINES));
            pObj->SetLayer(rAdmin.GetLayerID(aStr, FALSE));
        }

        bReturn = TRUE;
    }

    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent)
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

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

BOOL FuConstRectangle::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstRectangle::Activate()
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
            pView->SetGlueVisible();
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
            pView->SetGlueVisible();
        }
        break;

        default:
        {
            aObjKind = OBJ_RECT;
        }
        break;
    }

    pView->SetCurrentObj(aObjKind);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstRectangle::Deactivate()
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
        pView->SetGlueVisible( FALSE );
    }
    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Attribute fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

void FuConstRectangle::SetAttributes(SfxItemSet& rAttr, SdrObject* pObj)
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
    else if (nSlotId == SID_DRAW_CAPTION)
    {
        /**********************************************************************
        * Legendenobjekt
        **********************************************************************/
        Size aSize(pObj->GetLogicRect().GetSize());
        rAttr.Put( SdrTextMinFrameHeightItem( aSize.Height() ) );
        rAttr.Put( SdrTextMinFrameWidthItem( aSize.Width() ) );
        rAttr.Put( SdrTextAutoGrowHeightItem( TRUE ) );
        rAttr.Put( SdrTextAutoGrowWidthItem( TRUE ) );
        rAttr.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        rAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
    }
    else if (nSlotId == SID_DRAW_MEASURELINE)
    {
        /**********************************************************************
        * Masslinie
        **********************************************************************/
        SdPage* pPage = (SdPage*) pView->GetPageViewPvNum(0)->GetPage();
        String aName(SdResId(STR_POOLSHEET_MEASURE));
        SfxStyleSheet* pSheet = (SfxStyleSheet*) pPage->GetModel()->
                                     GetStyleSheetPool()->
                                     Find(aName, SFX_STYLE_FAMILY_PARA);
        DBG_ASSERT(pSheet, "Objektvorlage nicht gefunden");

        if (pSheet)
        {
            pObj->SetStyleSheet(pSheet, FALSE);
        }

        SdrLayerAdmin& rAdmin = pDoc->GetLayerAdmin();
        String aStr(SdResId(STR_LAYER_MEASURELINES));
        pObj->SetLayer(rAdmin.GetLayerID(aStr, FALSE));
    }
}


/*************************************************************************
|*
|* Linienanfaenge und -enden fuer das zu erzeugende Objekt setzen
|*
\************************************************************************/

void FuConstRectangle::SetLineEnds(SfxItemSet& rAttr, SdrObject* pObj)
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
        XPolygon aArrow(4);                         //      []
        aArrow[0]=Point(125,0);                     // 0  __[]__250
        aArrow[1]=Point(250,250);                   //    \    /
        aArrow[2]=Point(0,250);                     //     \  /
        aArrow[3]=Point(125,0);                     //      \/250

        // Kreis
        XPolygon aCircle(Point(0,0), 250, 250);

        // Quadrat
        XPolygon aSquare(4);
        aSquare[0].X()= 0; aSquare[0].Y()= 0;
        aSquare[1].X()=10; aSquare[1].Y()= 0;
        aSquare[2].X()=10; aSquare[2].Y()=10;
        aSquare[3].X()= 0; aSquare[3].Y()=10;

        SfxItemSet aSet( pDoc->GetPool() );
        pView->GetAttributes( aSet );
        long nWidth = 250;
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



