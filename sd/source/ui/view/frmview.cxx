/*************************************************************************
 *
 *  $RCSfile: frmview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-08 11:23:24 $
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

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFX_TOPFRM_HXX //autogen wg. SfxTopViewFrame
#include <sfx2/topfrm.hxx>
#endif

#pragma hdrstop

#include "frmview.hxx"
#include "viewshel.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "optsitem.hxx"
#include "drviewsh.hxx"
#include "outlnvsh.hxx"
#include "slidvish.hxx"
#include "app.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "glob.hrc"
#include "sdiocmpt.hxx"


/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

FrameView::FrameView(SdDrawDocument* pDrawDoc)
  : SdrView(pDrawDoc, (OutputDevice*) NULL),
  nRefCount(0),
  nPresViewShellId(SID_VIEWSHELL0),
  nSlotId(SID_OBJECT_SELECT)
{
    EndListening(*pDrawDoc);

    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);

    SetGridFront( TRUE );
    SetOConSnap( FALSE );
    SetFrameDragSingles( TRUE );
    SetSlidesPerRow(4);

    FrameView* pFrameView = NULL;
    SdDrawDocShell* pDocShell = pDrawDoc->GetDocSh();

    if ( pDocShell )
    {
        /**********************************************************************
        * Das Dokument wurde geladen, ist eine FrameView vorhanden?
        **********************************************************************/
        ULONG nSdViewShellCount = 0;
        SdViewShell* pViewSh = NULL;
        SfxViewShell* pSfxViewSh = NULL;
        SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(pDocShell,
                                                             TYPE(SfxTopViewFrame));

        while (pSfxViewFrame)
        {
            // Anzahl FrameViews ermitteln
            pSfxViewSh = pSfxViewFrame->GetViewShell();
            pViewSh = PTR_CAST( SdViewShell, pSfxViewSh );

            if (pViewSh)
            {
                nSdViewShellCount++;

                if (pViewSh->ISA(SdDrawViewShell))
                {
                    nPresViewShellId = SID_VIEWSHELL0;
                }
                else if (pViewSh->ISA(SdSlideViewShell))
                {
                    nPresViewShellId = SID_VIEWSHELL1;
                }
                else if (pViewSh->ISA(SdOutlineViewShell))
                {
                    nPresViewShellId = SID_VIEWSHELL2;
                }
            }

            pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, pDocShell,
                                                  TYPE(SfxTopViewFrame));
        }

        SdDrawDocument* pDoc = pDocShell->GetDoc();
        pFrameView = pDoc->GetFrameView(nSdViewShellCount);
    }

    if (pFrameView)
    {
        /**********************************************************************
        * FrameView mit der FrameView der DocShell initialisieren
        **********************************************************************/
        SetRuler( pFrameView->HasRuler() );
        SetGridCoarse( pFrameView->GetGridCoarse() );
        SetGridFine( pFrameView->GetGridFine() );
        SetSnapGrid( pFrameView->GetSnapGrid() );
        SetSnapGridWidth(pFrameView->GetSnapGridWidthX(), pFrameView->GetSnapGridWidthY());
        SetGridVisible( pFrameView->IsGridVisible() );
        SetGridFront( pFrameView->IsGridFront() );
        SetSnapAngle( pFrameView->GetSnapAngle() );
        SetGridSnap( pFrameView->IsGridSnap() );
        SetBordSnap( pFrameView->IsBordSnap() );
        SetHlplSnap( pFrameView->IsHlplSnap() );
        SetOFrmSnap( pFrameView->IsOFrmSnap() );
        SetOPntSnap( pFrameView->IsOPntSnap() );
        SetOConSnap( pFrameView->IsOConSnap() );
        SetHlplVisible( pFrameView->IsHlplVisible() );
        SetDragStripes( pFrameView->IsDragStripes() );
        SetPlusHandlesAlwaysVisible( pFrameView->IsPlusHandlesAlwaysVisible() );
        SetFrameDragSingles( pFrameView->IsFrameDragSingles() );
        SetSnapMagneticPixel( pFrameView->GetSnapMagneticPixel() );
        SetMarkedHitMovesAlways( pFrameView->IsMarkedHitMovesAlways() );
        SetMoveOnlyDragging( pFrameView->IsMoveOnlyDragging() );
        SetCrookNoContortion( pFrameView->IsCrookNoContortion() );
        SetSlantButShear( pFrameView->IsSlantButShear() );
        SetNoDragXorPolys( pFrameView->IsNoDragXorPolys() );
        SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
        SetBigOrtho( pFrameView->IsBigOrtho() );
        SetOrtho( pFrameView->IsOrtho() );
        SetEliminatePolyPointLimitAngle( pFrameView->GetEliminatePolyPointLimitAngle() );
        SetEliminatePolyPoints( pFrameView->IsEliminatePolyPoints() );
        SetMasterPagePaintCaching( pFrameView->IsMasterPagePaintCaching() );

        SetLineDraft( pFrameView->IsLineDraft() );
        SetFillDraft( pFrameView->IsFillDraft() );
        SetTextDraft( pFrameView->IsTextDraft() );
        SetGrafDraft( pFrameView->IsGrafDraft() );
        SetSolidMarkHdl( pFrameView->IsSolidMarkHdl() );
        SetSolidDragging( pFrameView->IsSolidDragging() );

        aVisibleLayers = pFrameView->GetVisibleLayers();
        aPrintableLayers = pFrameView->GetPrintableLayers();
        aLockedLayers = pFrameView->GetLockedLayers();
        aStandardHelpLines = pFrameView->GetStandardHelpLines();
        aNotesHelpLines = pFrameView->GetNotesHelpLines();
        aHandoutHelpLines = pFrameView->GetHandoutHelpLines();
        SetActiveLayer( pFrameView->GetActiveLayer() );
        bNoColors = pFrameView->IsNoColors();
        bNoAttribs = pFrameView->IsNoAttribs() ;
        aVisArea = pFrameView->GetVisArea();
        ePageKind = pFrameView->GetPageKind();
        nSelectedPage = pFrameView->GetSelectedPage();
        eStandardEditMode = pFrameView->GetViewShEditMode(PK_STANDARD);
        eNotesEditMode = pFrameView->GetViewShEditMode(PK_NOTES);
        eHandoutEditMode = pFrameView->GetViewShEditMode(PK_HANDOUT);
        bLayerMode = pFrameView->IsLayerMode();
        bQuickEdit = pFrameView->IsQuickEdit();
        bDragWithCopy = pFrameView->IsDragWithCopy();
        bBigHandles          = pFrameView->IsBigHandles();
        bDoubleClickTextEdit = pFrameView->IsDoubleClickTextEdit();
        bClickChangeRotation = pFrameView->IsClickChangeRotation();
        nSlidesPerRow = pFrameView->GetSlidesPerRow();
        nDrawMode = pFrameView->GetDrawMode();
        nPreviewDrawMode = pFrameView->GetPreviewDrawMode();
        bShowPreviewInPageMode = pFrameView->IsShowPreviewInPageMode() != 0;
        bShowPreviewInMasterPageMode = pFrameView->IsShowPreviewInMasterPageMode() != 0;
        bShowPreviewInOutlineMode = pFrameView->IsShowPreviewInOutlineMode() != 0;
    }
    else
    {
        /**********************************************************************
        * FrameView mit den Applikationsdaten initialisieren
        **********************************************************************/
        aVisibleLayers.SetAll();
        aPrintableLayers.SetAll();
        SetGridCoarse( Size( 1000, 1000 ) );
        SetSnapGrid( Size( 1000, 1000 ) );
        SetSnapGridWidth(Fraction(1000, 1), Fraction(1000, 1));
        SetActiveLayer( String( SdResId(STR_LAYER_LAYOUT) ) );
        bNoColors = TRUE;
        bNoAttribs = FALSE;
        aVisArea = Rectangle( Point(), Size(0, 0) );
        ePageKind = PK_STANDARD;
        nSelectedPage = 0;
        eStandardEditMode = EM_PAGE;
        eNotesEditMode = EM_PAGE;
        eHandoutEditMode = EM_MASTERPAGE;
        bLayerMode = FALSE;
        SetEliminatePolyPoints(FALSE);
        nDrawMode = DRAWMODE_DEFAULT;
        nPreviewDrawMode = DRAWMODE_DEFAULT;
        bShowPreviewInPageMode = FALSE;
        bShowPreviewInMasterPageMode = TRUE;
        bShowPreviewInOutlineMode = TRUE;

        Update( SD_MOD()->GetSdOptions(pDrawDoc->GetDocumentType()) );
    }

}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

FrameView::~FrameView()
{
}


/*************************************************************************
|*
|* Verbindung herstellen
|*
\************************************************************************/

void FrameView::Connect()
{
    nRefCount++;
}


/*************************************************************************
|*
|* Verbindung loesen
|*
\************************************************************************/

void FrameView::Disconnect()
{
    if (nRefCount > 0)
    {
        nRefCount--;
    }

    if (nRefCount == 0)
    {
        delete this;
    }
}


/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/

SvStream& operator << (SvStream& rOut, const FrameView& rView)
{
    ULONG nULTemp;
    rOut << (SdrView&) rView;

    // Letzter Parameter ist die aktuelle Versionsnummer des Codes
    SdIOCompat aIO(rOut, STREAM_WRITE, 11);

    rOut << rView.bRuler;
    rOut << rView.aVisibleLayers;
    rOut << rView.aLockedLayers;
    rOut << rView.aPrintableLayers;
    rOut << rView.aStandardHelpLines;
    rOut << rView.aNotesHelpLines;
    rOut << rView.aHandoutHelpLines;
    rOut << rView.bNoColors;
    rOut << rView.bNoAttribs;
    rOut << rView.aVisArea;
    nULTemp = (ULONG) rView.ePageKind;            rOut << nULTemp;
    rOut << rView.nSelectedPage;
    nULTemp = (ULONG) rView.eStandardEditMode;    rOut << nULTemp;
    rOut << rView.bLayerMode;
    rOut << rView.bQuickEdit;
    rOut << rView.bDragWithCopy;
    rOut << (UINT16)rView.nSlidesPerRow;

    rOut << rView.bBigHandles;
    rOut << rView.bDoubleClickTextEdit;
    rOut << rView.bClickChangeRotation;

    nULTemp = (ULONG) rView.eNotesEditMode;       rOut << nULTemp;
    nULTemp = (ULONG) rView.eHandoutEditMode;     rOut << nULTemp;

    rOut << rView.nDrawMode;
    rOut << rView.nPreviewDrawMode;

    rOut << rView.bShowPreviewInPageMode;
    rOut << rView.bShowPreviewInMasterPageMode;
    rOut << rView.bShowPreviewInOutlineMode;

    return rOut;
}

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

SvStream& operator >> (SvStream& rIn, FrameView& rView)
{
    rIn >> (SdrView&) rView;

    SdIOCompat aIO(rIn, STREAM_READ);

    rIn >> rView.bRuler;
    rIn >> rView.aVisibleLayers;
    rIn >> rView.aLockedLayers;
    rIn >> rView.aPrintableLayers;
    rIn >> rView.aStandardHelpLines;

    if (aIO.GetVersion() >= 1)
    {
        // Daten der Versionen >= 1 einlesen
        rIn >> rView.aNotesHelpLines;
        rIn >> rView.aHandoutHelpLines;
    }

    if (aIO.GetVersion() >= 2)
    {
        // Daten der Versionen >= 2 einlesen
        rIn >> rView.bNoColors;
        rIn >> rView.bNoAttribs;
    }

    if (aIO.GetVersion() >= 3)
    {
        ULONG nULTemp;
        rIn >> rView.aVisArea;
        rIn >> nULTemp;          rView.ePageKind = (PageKind) nULTemp;
        rIn >> rView.nSelectedPage;
        rIn >> nULTemp;          rView.eStandardEditMode = (EditMode) nULTemp;
        rView.eNotesEditMode   = rView.eStandardEditMode;
        rView.eHandoutEditMode = rView.eStandardEditMode;
        rIn >> rView.bLayerMode;
    }

    if (aIO.GetVersion() >= 4)
    {
        rIn >> rView.bQuickEdit;
    }

    if (aIO.GetVersion() >= 5)
    {
        rIn >> rView.bDragWithCopy;
    }

    if (aIO.GetVersion() >= 6)
    {
        UINT16 nTemp;
        rIn >> nTemp; rView.nSlidesPerRow = (USHORT)nTemp;
    }

    if (aIO.GetVersion() >= 7)
    {
        rIn >> rView.bBigHandles;
        rIn >> rView.bDoubleClickTextEdit;
        rIn >> rView.bClickChangeRotation;
    }

    if (aIO.GetVersion() >= 8)
    {
        ULONG nULTemp;
        rIn >> nULTemp; rView.eNotesEditMode   = (EditMode) nULTemp;
        rIn >> nULTemp; rView.eHandoutEditMode = (EditMode) nULTemp;
    }

    if (aIO.GetVersion() >= 9)
    {
        rIn >> rView.nDrawMode;
        rIn >> rView.nPreviewDrawMode;
    }

    if (aIO.GetVersion() >= 10)
    {
        rIn >> rView.bShowPreviewInPageMode;
        rIn >> rView.bShowPreviewInMasterPageMode;
    }

    if (aIO.GetVersion() >= 11)
    {
        rIn >> rView.bShowPreviewInOutlineMode;
    }

    // Falls die UniqueLayerNames vorhanden sind, werden die Default-Namen
    // verwendet
    String aLayerName(rView.GetActiveLayer());

    if (aLayerName == String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_LAYOUT" )))
    {
        rView.SetActiveLayer(String(SdResId(STR_LAYER_LAYOUT)));
    }
    else if (aLayerName == String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )))
    {
        rView.SetActiveLayer(String(SdResId(STR_LAYER_BCKGRND)));
    }
    else if (aLayerName == String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )))
    {
        rView.SetActiveLayer(String(SdResId(STR_LAYER_BCKGRNDOBJ)));
    }
    else if (aLayerName == String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_CONTROLS" )))
    {
        rView.SetActiveLayer(String(SdResId(STR_LAYER_CONTROLS)));
    }
    else if (aLayerName == String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_MEASURELINES" )))
    {
        rView.SetActiveLayer(String(SdResId(STR_LAYER_MEASURELINES)));
    }

    if (rView.GetModel())
    {
        USHORT nMaxPages = ((SdDrawDocument* )rView.GetModel())->
                                      GetSdPageCount(rView.ePageKind);

        if (rView.nSelectedPage >= nMaxPages)
        {
            // Ggf. auf die letzte Seite selektieren
            rView.nSelectedPage = nMaxPages - 1;
        }
    }

    return rIn;
}

/*************************************************************************
|*
|* Update mit Daten der SdOptions
|*
\************************************************************************/

void FrameView::Update(SdOptions* pOptions)
{
    if (pOptions)
    {
        bRuler = pOptions->IsRulerVisible();
        SetGridVisible( pOptions->GetGridVisible() );
        SetSnapAngle( pOptions->GetAngle() );
        SetGridSnap( pOptions->GetUseGridSnap() );
        SetBordSnap( pOptions->IsSnapBorder()  );
        SetHlplSnap( pOptions->IsSnapHelplines() );
        SetOFrmSnap( pOptions->IsSnapFrame() );
        SetOPntSnap( pOptions->IsSnapPoints() );
        SetHlplVisible( pOptions->IsHelplines() );
        SetDragStripes( pOptions->IsDragStripes() );
        SetPlusHandlesAlwaysVisible( pOptions->IsHandlesBezier() );
        SetSnapMagneticPixel( pOptions->GetSnapArea() );
        SetMarkedHitMovesAlways( pOptions->IsMarkedHitMovesAlways() );
        SetMoveOnlyDragging( pOptions->IsMoveOnlyDragging() );
        SetSlantButShear( pOptions->IsMoveOnlyDragging() );
        SetNoDragXorPolys ( !pOptions->IsMoveOutline() );
        SetCrookNoContortion( pOptions->IsCrookNoContortion() );
        SetAngleSnapEnabled( pOptions->IsRotate() );
        SetBigOrtho( pOptions->IsBigOrtho() );
        SetOrtho( pOptions->IsOrtho() );
        SetEliminatePolyPointLimitAngle( pOptions->GetEliminatePolyPointLimitAngle() );
        SetMasterPagePaintCaching( pOptions->IsMasterPagePaintCaching() );
        GetModel()->SetPickThroughTransparentTextFrames( pOptions->IsPickThrough() );

        SetLineDraft( pOptions->IsHairlineMode() );
        SetFillDraft( pOptions->IsOutlineMode() );
        SetTextDraft( pOptions->IsNoText() );
        SetGrafDraft( pOptions->IsExternGraphic() );
        SetSolidMarkHdl( pOptions->IsSolidMarkHdl() );
        SetSolidDragging( pOptions->IsSolidDragging() );

        SetGridCoarse( Size( pOptions->GetFldDrawX(), pOptions->GetFldDrawY() ) );
        SetGridFine( Size( pOptions->GetFldDivisionX(), pOptions->GetFldDivisionY() ) );
//      SetSnapGrid( Size( pOptions->GetFldSnapX(), pOptions->GetFldSnapY() ) );
        Fraction aFractX(pOptions->GetFldDrawX(), pOptions->GetFldDrawX() / ( pOptions->GetFldDivisionX() ? pOptions->GetFldDivisionX() : 1 ));
        Fraction aFractY(pOptions->GetFldDrawY(), pOptions->GetFldDrawY() / ( pOptions->GetFldDivisionY() ? pOptions->GetFldDivisionY() : 1 ));
        SetSnapGridWidth(aFractX, aFractY);
        SetQuickEdit(pOptions->IsQuickEdit());
        SetDragWithCopy(pOptions->IsDragWithCopy());

        SetBigHandles( pOptions->IsBigHandles() );
        SetDoubleClickTextEdit( pOptions->IsDoubleClickTextEdit() );
        SetClickChangeRotation( pOptions->IsClickChangeRotation() );
        SetPreviewDrawMode( pOptions->GetPreviewQuality() );
    }
}


/*************************************************************************
|*
|* EditMode (Page oder MasterPage) des Arbeitsmodus setzen
|*
\************************************************************************/

void FrameView::SetViewShEditMode(EditMode eMode, PageKind eKind)
{
    if (eKind == PK_STANDARD)
    {
        eStandardEditMode = eMode;
    }
    else if (eKind == PK_NOTES)
    {
        eNotesEditMode = eMode;
    }
    else if (eKind == PK_HANDOUT)
    {
        eHandoutEditMode = eMode;
    }
}


/*************************************************************************
|*
|* EditMode (Page oder MasterPage) des Arbeitsmodus zurueckgeben
|*
\************************************************************************/

EditMode FrameView::GetViewShEditMode(PageKind eKind)
{
    EditMode eMode = EM_PAGE;

    if (eKind == PK_STANDARD)
    {
        eMode = eStandardEditMode;
    }
    else if (eKind == PK_NOTES)
    {
        eMode = eNotesEditMode;
    }
    else if (eKind == PK_HANDOUT)
    {
        eMode = eHandoutEditMode;
    }

    return (eMode);
}



