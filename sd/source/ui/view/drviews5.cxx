/*************************************************************************
 *
 *  $RCSfile: drviews5.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:12:21 $
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

#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif


#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "app.hrc"

#include "optsitem.hxx"
#include "optsite2.hxx"
#include "app.hxx"
#include "frmview.hxx"
#include "sdattr.hxx"
#include "futext.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "prntopts.hxx"
#include "sdresid.hxx"
#include "sdwindow.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "drviewsh.hxx"
#include "sdoutl.hxx"
#include "sdclient.hxx"
#include "fuslshow.hxx"

#define TABCONTROL_INITIAL_SIZE     350

/*************************************************************************
|*
|* Wird gerufen, wenn sich das Model aendert
|*
\************************************************************************/

void SdDrawViewShell::ModelHasChanged()
{
    Invalidate();
    // Damit der Navigator auch einen aktuellen Status bekommt
    GetViewFrame()->GetBindings().Invalidate( SID_NAVIGATOR_STATE, TRUE, FALSE );

    // Damit das Effekte-Window die Reihenfolge updaten kann
    UpdateEffectWindow();

    //Update3DWindow();
    SfxBoolItem aItem( SID_3D_STATE, TRUE );
    GetViewFrame()->GetDispatcher()->Execute(
        SID_3D_STATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

    // jetzt den von der Drawing Engine neu erzeugten TextEditOutliner
    // initialisieren
    Outliner* pOutliner       = pDrView->GetTextEditOutliner();
    if (pOutliner)
    {
        SfxStyleSheetPool* pSPool = (SfxStyleSheetPool*) pDocSh->GetStyleSheetPool();
        pOutliner->SetStyleSheetPool(pSPool);
    }
}

/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void SdDrawViewShell::InnerResizePixel(const Point &rPos, const Size &rSize)
{
    if (!pFuSlideShow)
    {
        // Da die innere Groesse vorgegeben wird, ist ein Zoomen auf
        // Seitenbreite nicht erlaubt
        bZoomOnPage = FALSE;

        SdViewShell::InnerResizePixel(rPos, rSize);

        SetZoomRect( pDocSh->GetVisArea(ASPECT_CONTENT) );
    }
}

/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void SdDrawViewShell::OuterResizePixel(const Point &rPos, const Size &rSize)
{
    if (!pFuSlideShow || !pFuSlideShow->IsFullScreen())
    {
        SdViewShell::OuterResizePixel(rPos, rSize);

        if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            SetZoomRect( pDocSh->GetVisArea(ASPECT_CONTENT) );
        }
    }

    if (pFuSlideShow && !pFuSlideShow->IsFullScreen())
    {
        pFuSlideShow->Resize(rSize);
    }
}

/*************************************************************************
|*
|* View-Groesse (und -Position bei OLE) aendern durch Aufruf der
|* Basisklassenmethode; danach TabControl an die Position des ersten
|* horizontalen ScrollBars anpassen
|*
\************************************************************************/

void SdDrawViewShell::AdjustPosSizePixel(const Point &rNewPos,
                                         const Size &rNewSize)
{
    if ( rNewSize.Width() && rNewSize.Height() )
    {
        Size aTabSize = aTabControl.GetSizePixel();

#ifdef UNX
        if( ( aTabSize.Width() == 0 ) || ( aTabSize.Height() ) )
#else
        if ( aTabSize.Width() == 0 )
#endif
            aTabSize.Width() = TABCONTROL_INITIAL_SIZE;
        aTabSize.Height() = aScrBarWH.Height();
        Point aHPos = rNewPos;
        aHPos.Y() += rNewSize.Height() - aTabSize.Height();

        Size aBtnSize(aScrBarWH);
        aPageBtn.SetPosSizePixel(aHPos, aBtnSize);
        aHPos.X() += aBtnSize.Width();
        aMasterPageBtn.SetPosSizePixel(aHPos, aBtnSize);
        aHPos.X() += aBtnSize.Width();
        aLayerBtn.SetPosSizePixel(aHPos, aBtnSize);
        aHPos.X() += aBtnSize.Width();

        aTabControl.SetSizePixel(aTabSize);

        SdViewShell::AdjustPosSizePixel(rNewPos, rNewSize);

        aTabSize.Width() = pHScrlArray[0]->GetPosPixel().X() - aHPos.X();
        if ( aTabSize.Width() < 0 )
            aTabSize.Width() = 0;

        aTabControl.SetPosSizePixel(aHPos, aTabSize);
        aLayerTab.SetPosSizePixel(aHPos, aTabSize);

        SdClient*   pIPClient = (SdClient*)GetIPClient();
        BOOL bClientActive = FALSE;
        if ( pIPClient && pIPClient->IsInPlaceActive() )
            bClientActive = TRUE;

        BOOL bInPlaceActive = pDocSh->GetProtocol().IsInPlaceActive();

//        if ( bZoomOnPage && pDocSh->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        if ( bZoomOnPage && !bInPlaceActive && !bClientActive )
        {
            // bei Split immer erstes Fenster resizen
            pWindow = pWinArray[0][0];
            SfxRequest aReq(SID_SIZE_PAGE, 0, pDoc->GetItemPool());
            ExecuteSlot( aReq );
        }
    }
}

/*************************************************************************
|*
|* Daten der FrameView auf die aktuelle View uebertragen
|*
\************************************************************************/

void SdDrawViewShell::ReadFrameViewData(FrameView* pView)
{
    BOOL bChanged = pDoc->IsChanged();      // merken

    // Diese Option wird am Model eingestellt
    pDoc->SetPickThroughTransparentTextFrames(
             SD_MOD()->GetSdOptions(pDoc->GetDocumentType())->IsPickThrough());

    // Initialisierungen der Zeichen-(Bildschirm-)Attribute
    if (HasRuler() != pView->HasRuler())
        SetRuler( pView->HasRuler() );

    if (pDrView->GetGridCoarse() != pView->GetGridCoarse())
        pDrView->SetGridCoarse( pView->GetGridCoarse() );

    if (pDrView->GetGridFine() != pView->GetGridFine())
        pDrView->SetGridFine( pView->GetGridFine() );

    if (pDrView->GetSnapGrid() != pView->GetSnapGrid())
        pDrView->SetSnapGrid( pView->GetSnapGrid() );

    if (pDrView->GetSnapGridWidthX() != pView->GetSnapGridWidthX() || pDrView->GetSnapGridWidthY() != pView->GetSnapGridWidthY())
        pDrView->SetSnapGridWidth(pView->GetSnapGridWidthX(), pView->GetSnapGridWidthY());

    if (pDrView->IsGridVisible() != pView->IsGridVisible())
        pDrView->SetGridVisible( pView->IsGridVisible() );

    if (pDrView->IsGridFront() != pView->IsGridFront())
        pDrView->SetGridFront( pView->IsGridFront() );

    if (pDrView->GetSnapAngle() != pView->GetSnapAngle())
        pDrView->SetSnapAngle( pView->GetSnapAngle() );

    if (pDrView->IsGridSnap() !=  pView->IsGridSnap() )
        pDrView->SetGridSnap( pView->IsGridSnap() );

    if (pDrView->IsBordSnap() !=  pView->IsBordSnap() )
        pDrView->SetBordSnap( pView->IsBordSnap() );

    if (pDrView->IsHlplSnap() !=  pView->IsHlplSnap() )
        pDrView->SetHlplSnap( pView->IsHlplSnap() );

    if (pDrView->IsOFrmSnap() !=  pView->IsOFrmSnap() )
        pDrView->SetOFrmSnap( pView->IsOFrmSnap() );

    if (pDrView->IsOPntSnap() !=  pView->IsOPntSnap() )
        pDrView->SetOPntSnap( pView->IsOPntSnap() );

    if (pDrView->IsOConSnap() !=  pView->IsOConSnap() )
        pDrView->SetOConSnap( pView->IsOConSnap() );

    if (pDrView->IsHlplVisible() != pView->IsHlplVisible() )
        pDrView->SetHlplVisible( pView->IsHlplVisible() );

    if (pDrView->IsDragStripes() != pView->IsDragStripes() )
        pDrView->SetDragStripes( pView->IsDragStripes() );

    if (pDrView->IsPlusHandlesAlwaysVisible() != pView->IsPlusHandlesAlwaysVisible() )
        pDrView->SetPlusHandlesAlwaysVisible( pView->IsPlusHandlesAlwaysVisible() );

    if (pDrView->GetSnapMagneticPixel() != pView->GetSnapMagneticPixel() )
        pDrView->SetSnapMagneticPixel( pView->GetSnapMagneticPixel() );

    if (pDrView->IsMarkedHitMovesAlways() != pView->IsMarkedHitMovesAlways() )
        pDrView->SetMarkedHitMovesAlways( pView->IsMarkedHitMovesAlways() );

    if (pDrView->IsMoveOnlyDragging() != pView->IsMoveOnlyDragging() )
        pDrView->SetMoveOnlyDragging( pView->IsMoveOnlyDragging() );

//    pDrView->SetSlantButShear( pView->IsSlantButShear() );

    if (pDrView->IsNoDragXorPolys() != pView->IsNoDragXorPolys() )
        pDrView->SetNoDragXorPolys( pView->IsNoDragXorPolys() );

    if (pDrView->IsCrookNoContortion() != pView->IsCrookNoContortion() )
        pDrView->SetCrookNoContortion( pView->IsCrookNoContortion() );

    if (pDrView->IsAngleSnapEnabled() != pView->IsAngleSnapEnabled() )
        pDrView->SetAngleSnapEnabled( pView->IsAngleSnapEnabled() );

    if (pDrView->IsBigOrtho() != pView->IsBigOrtho() )
        pDrView->SetBigOrtho( pView->IsBigOrtho() );

    if (pDrView->IsOrtho() != pView->IsOrtho() )
        pDrView->SetOrtho( pView->IsOrtho() );

    if (pDrView->GetEliminatePolyPointLimitAngle() != pView->GetEliminatePolyPointLimitAngle() )
        pDrView->SetEliminatePolyPointLimitAngle( pView->GetEliminatePolyPointLimitAngle() );

    if (pDrView->IsEliminatePolyPoints() != pView->IsEliminatePolyPoints() )
        pDrView->SetEliminatePolyPoints( pView->IsEliminatePolyPoints() );

    if (pDrView->IsLineDraft() != pView->IsLineDraft() )
        pDrView->SetLineDraft( pView->IsLineDraft() );

    if (pDrView->IsFillDraft() != pView->IsFillDraft() )
    {
        pDrView->ReleaseMasterPagePaintCache();
        pDrView->SetFillDraft( pView->IsFillDraft() );
    }

    if (pDrView->IsTextDraft() != pView->IsTextDraft() )
        pDrView->SetTextDraft( pView->IsTextDraft() );

    if (pDrView->IsGrafDraft() != pView->IsGrafDraft() )
        pDrView->SetGrafDraft( pView->IsGrafDraft() );

    if (pDrView->IsSolidMarkHdl() != pView->IsSolidMarkHdl() )
        pDrView->SetSolidMarkHdl( pView->IsSolidMarkHdl() );

    if (pDrView->IsSolidDragging() != pView->IsSolidDragging() )
        pDrView->SetSolidDragging( pView->IsSolidDragging() );

    if (pDrView->IsQuickTextEditMode() != pView->IsQuickEdit())
        pDrView->SetQuickTextEditMode( pView->IsQuickEdit() );

    // Definition:
    //  grosse Handles: 9
    //  kleine Handles: 7
    USHORT nTmp = pDrView->GetMarkHdlSizePixel();
    //DBG_ASSERT(nTmp != 7, "HandleSize != 7 oder 9");
    if( nTmp == 9 && !pView->IsBigHandles() )
        pDrView->SetMarkHdlSizePixel( 7 );
    else if( nTmp == 7 && pView->IsBigHandles() )
        pDrView->SetMarkHdlSizePixel( 9 );


    SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

    if (pPageView)
    {
        if ( pPageView->GetVisibleLayers() != pView->GetVisibleLayers() )
            pPageView->SetVisibleLayers( pView->GetVisibleLayers() );

        if ( pPageView->GetPrintableLayers() != pView->GetPrintableLayers() )
            pPageView->SetPrintableLayers( pView->GetPrintableLayers() );

        if ( pPageView->GetLockedLayers() != pView->GetLockedLayers() )
            pPageView->SetLockedLayers( pView->GetLockedLayers() );

        if (ePageKind == PK_NOTES)
        {
            if (pPageView->GetHelpLines() != pView->GetNotesHelpLines())
                pPageView->SetHelpLines( pView->GetNotesHelpLines() );
        }
        else if (ePageKind == PK_HANDOUT)
        {
            if (pPageView->GetHelpLines() != pView->GetHandoutHelpLines())
                pPageView->SetHelpLines( pView->GetHandoutHelpLines() );
        }
        else
        {
            if (pPageView->GetHelpLines() != pView->GetStandardHelpLines())
                pPageView->SetHelpLines( pView->GetStandardHelpLines() );
        }
    }

    if ( pDrView->GetActiveLayer() != pView->GetActiveLayer() )
        pDrView->SetActiveLayer( pView->GetActiveLayer() );

    USHORT nSelectedPage = 0;

    if (ePageKind != PK_HANDOUT)
    {
        nSelectedPage = pView->GetSelectedPage();
    }

    EditMode eNewEditMode = pView->GetViewShEditMode(ePageKind);
    BOOL bNewLayerMode = pView->IsLayerMode();
    ChangeEditMode(eNewEditMode, bNewLayerMode);
    SwitchPage(nSelectedPage);

    // DrawMode fuer 'Normales' Fenster wiederherstellen
    if(pWindow->GetDrawMode() != pView->GetDrawMode())
      pWindow->SetDrawMode(pView->GetDrawMode());

    // Muss am Ende gerufen werden, da ein WriteFrameViewData() ausgeloest wird
    if (pDrView->IsFrameDragSingles() != pView->IsFrameDragSingles() )
        pDrView->SetFrameDragSingles( pView->IsFrameDragSingles() );

    pDoc->SetChanged( bChanged );
}

/*************************************************************************
|*
|* Daten der aktuellen View auf die FrameView uebertragen
|*
\************************************************************************/

void SdDrawViewShell::WriteFrameViewData()
{
    // Zeichen-(Bildschirm-)Attribute an FrameView merken
    pFrameView->SetRuler( HasRuler() );
    pFrameView->SetGridCoarse( pDrView->GetGridCoarse() );
    pFrameView->SetGridFine( pDrView->GetGridFine() );
    pFrameView->SetSnapGrid( pDrView->GetSnapGrid() );
    pFrameView->SetSnapGridWidth(pDrView->GetSnapGridWidthX(), pDrView->GetSnapGridWidthY());
    pFrameView->SetGridVisible( pDrView->IsGridVisible() );
    pFrameView->SetGridFront( pDrView->IsGridFront() );
    pFrameView->SetSnapAngle( pDrView->GetSnapAngle() );
    pFrameView->SetGridSnap( pDrView->IsGridSnap() );
    pFrameView->SetBordSnap( pDrView->IsBordSnap() );
    pFrameView->SetHlplSnap( pDrView->IsHlplSnap() );
    pFrameView->SetOFrmSnap( pDrView->IsOFrmSnap() );
    pFrameView->SetOPntSnap( pDrView->IsOPntSnap() );
    pFrameView->SetOConSnap( pDrView->IsOConSnap() );
    pFrameView->SetHlplVisible( pDrView->IsHlplVisible() );
    pFrameView->SetDragStripes( pDrView->IsDragStripes() );
    pFrameView->SetPlusHandlesAlwaysVisible( pDrView->IsPlusHandlesAlwaysVisible() );
    pFrameView->SetFrameDragSingles( pDrView->IsFrameDragSingles() );
    pFrameView->SetMarkedHitMovesAlways( pDrView->IsMarkedHitMovesAlways() );
    pFrameView->SetMoveOnlyDragging( pDrView->IsMoveOnlyDragging() );
    pFrameView->SetNoDragXorPolys( pDrView->IsNoDragXorPolys() );
    pFrameView->SetCrookNoContortion( pDrView->IsCrookNoContortion() );
    pFrameView->SetBigOrtho( pDrView->IsBigOrtho() );
    pFrameView->SetEliminatePolyPointLimitAngle( pDrView->GetEliminatePolyPointLimitAngle() );
    pFrameView->SetEliminatePolyPoints( pDrView->IsEliminatePolyPoints() );

    pFrameView->SetLineDraft( pDrView->IsLineDraft() );
    pFrameView->SetFillDraft( pDrView->IsFillDraft() );
    pFrameView->SetTextDraft( pDrView->IsTextDraft() );
    pFrameView->SetGrafDraft( pDrView->IsGrafDraft() );
    pFrameView->SetSolidMarkHdl( pDrView->IsSolidMarkHdl() );
    pFrameView->SetSolidDragging( pDrView->IsSolidDragging() );
    pFrameView->SetQuickEdit( pDrView->IsQuickTextEditMode() );

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisArea = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    pFrameView->SetVisArea(aVisArea);

    if (ePageKind == PK_HANDOUT)
    {
        pFrameView->SetSelectedPage(0);
    }
    else
    {
        pFrameView->SetSelectedPage(aTabControl.GetCurPageId() - 1);
    }

    pFrameView->SetViewShEditMode(eEditMode, ePageKind);
    pFrameView->SetLayerMode(bLayerMode);

    SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

    if (pPageView)
    {
        if ( pFrameView->GetVisibleLayers() != pPageView->GetVisibleLayers() )
            pFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );

        if ( pFrameView->GetPrintableLayers() != pPageView->GetPrintableLayers() )
            pFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );

        if ( pFrameView->GetLockedLayers() != pPageView->GetLockedLayers() )
            pFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

        if (ePageKind == PK_NOTES)
        {
            pFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
        }
        else if (ePageKind == PK_HANDOUT)
        {
            pFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
        }
        else
        {
            pFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
        }
    }

    if ( pFrameView->GetActiveLayer() != pDrView->GetActiveLayer() )
        pFrameView->SetActiveLayer( pDrView->GetActiveLayer() );

    // DrawMode fuer 'Normales' Fenster merken
    if(pFrameView->GetDrawMode() != pWindow->GetDrawMode())
      pFrameView->SetDrawMode(pWindow->GetDrawMode());

}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird vom Fenster pWin an
|* die Viewshell und die aktuelle Funktion weitergeleitet
|*
|* Anmerkung: pWin==NULL, wenn Paint() vom ShowWindow gerufen wird!
|*
\************************************************************************/

void SdDrawViewShell::Paint(const Rectangle& rRect, SdWindow* pWin)
{
    if (pWin)
    {
        /**********************************************************************
        * Wiese um die Seite herum zeichnen - es sei denn, das Paint kommt
        * vom Fenster der DiaShow, da soll keine Wiese zu sehen sein
        **********************************************************************/

        if (!pFuSlideShow || pWin != (SdWindow*)pFuSlideShow->GetShowWindow())
        {

            const Color aOldLineColor( pWin->GetLineColor() );
            const Color aOldFillColor( pWin->GetFillColor() );
            const ULONG nOldDrawMode( pWin->GetDrawMode() );

            pWin->SetDrawMode( DRAWMODE_DEFAULT );
            pWin->SetLineColor();
            pWin->SetFillColor( Color( 234, 234, 234 ) );

            Rectangle aSdRect( pWin->PixelToLogic( Point(0, 0) ), pWin->GetOutputSize() );
            Rectangle aPgRect( Point(0, 0), pActualPage->GetSize() );
            Polygon aWinPoly(aSdRect);
            Polygon aPagePoly(aPgRect);
            PolyPolygon aPolyPoly(aWinPoly);
            aPolyPoly.Insert(aPagePoly);
            pWin->DrawPolyPolygon(aPolyPoly);

            pWin->SetLineColor( aOldLineColor );
            pWin->SetFillColor( aOldFillColor );
            pWin->SetDrawMode( nOldDrawMode );
        }
    }

    /**************************************************************************
    * Seite zeichnen
    **************************************************************************/
    pDrView->InitRedraw(pWin, Region (rRect));

    if (pWin)
    {
        if ( pDocSh->GetActualFunction() )
        {
            /******************************************************************
            * Funktion der DocShell zeichnen (z.B. Selektion, Hilfsfiguren o.a.)
            ******************************************************************/
            pDocSh->GetActualFunction()->Paint(rRect, pWin);
        }

        if (pFuActual)
        {
            /******************************************************************
            * Funktion zeichnen (z.B. Selektion, Hilfsfiguren o.a.)
            ******************************************************************/
            pFuActual->Paint(rRect, pWin);
        }
    }
}

/*************************************************************************
|*
|* Zoom-Faktor fuer InPlace einstellen
|*
\************************************************************************/

void SdDrawViewShell::SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY)
{
    SdViewShell::SetZoomFactor(rZoomX, rZoomY);
    bZoomOnPage = FALSE;
    Point aOrigin = pWindow->GetViewOrigin();
    pWindow->SetWinViewPos(aOrigin);
}

/*************************************************************************
|*
|* Optimale Groesse zurueckgeben
|*
\************************************************************************/

Size SdDrawViewShell::GetOptimalSizePixel() const
{
    Size aSize;

    SdrPageView* pPV = pDrView->GetPageViewPvNum(0);

    if (pPV)
    {
        SdPage* pPage = (SdPage*) pPV->GetPage();

        if (pPage)
        {
            if (!bZoomOnPage)
            {
                // Gegenwaertigen MapMode beruecksichtigen
                aSize = pWindow->LogicToPixel( pPage->GetSize() );
            }
            else
            {
                // 1:1 Darstellung
                MapMode aMapMode(MAP_100TH_MM);
                aSize = pWindow->LogicToPixel( pPage->GetSize(), aMapMode );
                ( (SdDrawViewShell*) this)->bZoomOnPage = TRUE;
            }
        }
    }

    return(aSize);
}


/*************************************************************************
|*
|* Seite wird gehided
|*
\************************************************************************/

void SdDrawViewShell::HidePage(SdrPageView* pPV)
{
    FmFormShell* pShell = (FmFormShell*) aShellTable.Get(RID_FORMLAYER_TOOLBOX);

    if (pShell)
        pShell->PrepareClose(FALSE);
}



