/*************************************************************************
 *
 *  $RCSfile: frmview.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:59:02 $
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

#include "FrameView.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFX_TOPFRM_HXX //autogen wg. SfxTopViewFrame
#include <sfx2/topfrm.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SD_UNOKYWDS_HXX_
#include "unokywds.hxx"
#endif

#include <vector>

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "optsitem.hxx"
#include "PaneManager.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#include "app.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "glob.hrc"
#include "sdiocmpt.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::std;

namespace sd {

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

FrameView::FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView /* = NULK */)
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

    if( NULL == pFrameView )
    {
        DrawDocShell* pDocShell = pDrawDoc->GetDocSh();

        if ( pDocShell )
        {
            /**********************************************************************
            * Das Dokument wurde geladen, ist eine FrameView vorhanden?
            **********************************************************************/
            ULONG nSdViewShellCount = 0;
            ViewShellBase* pBase = NULL;
            SfxViewShell* pSfxViewSh = NULL;
            SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(pDocShell,
                                                                 TYPE(SfxTopViewFrame));

            while (pSfxViewFrame)
            {
                // Count the FrameViews and remember the ty
                pSfxViewSh = pSfxViewFrame->GetViewShell();
                pBase = PTR_CAST(ViewShellBase, pSfxViewSh );

                if (pBase != NULL)
                {
                    nSdViewShellCount++;

                    //AF
                    switch (pBase->GetPaneManager().GetViewShellType(
                        PaneManager::PT_CENTER))
                    {
                        case ViewShell::ST_IMPRESS:
                        case ViewShell::ST_NOTES:
                        case ViewShell::ST_HANDOUT:
                            nPresViewShellId = SID_VIEWSHELL0;
                            break;

                        case ViewShell::ST_SLIDE:
                            nPresViewShellId = SID_VIEWSHELL1;
                            break;

                        case ViewShell::ST_OUTLINE:
                            nPresViewShellId = SID_VIEWSHELL2;
                            break;
                    }
                }

                pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, pDocShell,
                                                      TYPE(SfxTopViewFrame));
            }

            SdDrawDocument* pDoc = pDocShell->GetDoc();
            pFrameView = pDoc->GetFrameView(nSdViewShellCount);
        }
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
// #110094#-7
//      SetMasterPagePaintCaching( pFrameView->IsMasterPagePaintCaching() );
        SetDesignMode( pFrameView->IsDesignMode() );

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
        nTabCtrlPercent = pFrameView->GetTabCtrlPercent();
        SetPreviousViewShellType (pFrameView->GetPreviousViewShellType());
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

        {
            bool bUseContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
            nDrawMode = bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR;
        }
        nPreviewDrawMode = nDrawMode;
        bShowPreviewInPageMode = FALSE;
        bShowPreviewInMasterPageMode = FALSE;
        bShowPreviewInOutlineMode = FALSE;
        nTabCtrlPercent = 0.0;
        SetPreviousViewShellType (ViewShell::ST_NONE);

        // get default for design mode
        sal_Bool bInitDesignMode = pDrawDoc->GetOpenInDesignMode();
        if( pDrawDoc->OpenInDesignModeIsDefaulted() )
        {
            bInitDesignMode = sal_True;
        }

        SfxObjectShell* pObjShell = pDrawDoc->GetObjectShell();
        sal_Bool bReadOnly = sal_False;
        if( pObjShell )
            bReadOnly = pObjShell->IsReadOnly();
        if( bReadOnly )
            bInitDesignMode = sal_False;
        SetDesignMode( bInitDesignMode );

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

    // #95895# translate view-layer name to standard-ASCII
    // like in MakeUniqueLayerNames()
    String aLayerName(rView.GetActiveLayer());

    String aLayerLayout(SdResId(STR_LAYER_LAYOUT));
    String aLayerBckgrnd(SdResId(STR_LAYER_BCKGRND));
    String aLayerBckgrndObj(SdResId(STR_LAYER_BCKGRNDOBJ));
    String aLayerControls(SdResId(STR_LAYER_CONTROLS));
    String aLayerMeasurelines(SdResId(STR_LAYER_MEASURELINES));

    sal_Bool bActiveLayerWasChanged(sal_False);
    String aOldLayerName(rView.GetActiveLayer());

    if (aLayerName == aLayerLayout)
    {
        ((FrameView&)rView).SetActiveLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_LAYOUT" )));
        bActiveLayerWasChanged = sal_True;
    }
    else if (aLayerName == aLayerBckgrnd)
    {
        ((FrameView&)rView).SetActiveLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )));
        bActiveLayerWasChanged = sal_True;
    }
    else if (aLayerName == aLayerBckgrndObj)
    {
        ((FrameView&)rView).SetActiveLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )));
        bActiveLayerWasChanged = sal_True;
    }
    else if (aLayerName == aLayerControls)
    {
        ((FrameView&)rView).SetActiveLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_CONTROLS" )));
        bActiveLayerWasChanged = sal_True;
    }
    else if (aLayerName == aLayerMeasurelines)
    {
        ((FrameView&)rView).SetActiveLayer( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_MEASURELINES" )));
        bActiveLayerWasChanged = sal_True;
    }

    // stream out the view
    rOut << (SdrView&) rView;

    // #95895# when active layer name was changed for export, change it back to original here
    if(bActiveLayerWasChanged)
    {
        ((FrameView&)rView).SetActiveLayer(aOldLayerName);
    }

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
    else
    {
        // #i12131#
        // If layer name was not translated this may be an old layer name in
        // translated speech version. To avoid errors (see bugid) this needs to be
        // set to a useful default. Best default-layout is 'Layout'.
        rView.SetActiveLayer(String(SdResId(STR_LAYER_LAYOUT)));
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
        SetGridVisible( pOptions->IsGridVisible() );
        SetSnapAngle( pOptions->GetAngle() );
        SetGridSnap( pOptions->IsUseGridSnap() );
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
// #110094#-7
//      SetMasterPagePaintCaching( pOptions->IsMasterPagePaintCaching() );
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

static OUString createHelpLinesString( const SdrHelpLineList& rHelpLines )
{
    OUStringBuffer aLines;

    const USHORT nCount = rHelpLines.GetCount();
    for( USHORT nHlpLine = 0; nHlpLine < nCount; nHlpLine++ )
    {
        const SdrHelpLine& rHelpLine = rHelpLines[nHlpLine];
        const Point& rPos = rHelpLine.GetPos();

        switch( rHelpLine.GetKind() )
        {
            case SDRHELPLINE_POINT:
                aLines.append( (sal_Unicode)'P' );
                aLines.append( (sal_Int32)rPos.X() );
                aLines.append( (sal_Unicode)',' );
                aLines.append( (sal_Int32)rPos.Y() );
                break;
            case SDRHELPLINE_VERTICAL:
                aLines.append( (sal_Unicode)'V' );
                aLines.append( (sal_Int32)rPos.X() );
                break;
            case SDRHELPLINE_HORIZONTAL:
                aLines.append( (sal_Unicode)'H' );
                aLines.append( (sal_Int32)rPos.Y() );
                break;
            default:
                DBG_ERROR( "Unsupported helpline Kind!" );
        }
    }

    return aLines.makeStringAndClear();
}

#define addValue( n, v ) push_back( std::pair< OUString, Any >( OUString( RTL_CONSTASCII_USTRINGPARAM( n ) ), v ) )
void FrameView::WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rValues, sal_Bool bBrowse )
{
    std::vector< std::pair< OUString, Any > > aUserData;

    aUserData.addValue( sUNO_View_GridIsVisible, makeAny( (sal_Bool)IsGridVisible() ) );
    aUserData.addValue( sUNO_View_GridIsFront, makeAny( (sal_Bool)IsGridFront() ) );
    aUserData.addValue( sUNO_View_IsSnapToGrid, makeAny( (sal_Bool)IsGridSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToPageMargins, makeAny( (sal_Bool)IsBordSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToSnapLines, makeAny( (sal_Bool)IsHlplSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToObjectFrame, makeAny( (sal_Bool)IsOFrmSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToObjectPoints, makeAny( (sal_Bool)IsOPntSnap() ) );

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapLinesVisible ) );
//  pValue->Value <<= (sal_Bool)IsHlplVisible();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsDragStripes ) );
//  pValue->Value <<= (sal_Bool)IsDragStripes();
//  pValue++;nIndex++;

    aUserData.addValue( sUNO_View_IsPlusHandlesAlwaysVisible, makeAny( (sal_Bool)IsPlusHandlesAlwaysVisible() ) );
    aUserData.addValue( sUNO_View_IsFrameDragSingles, makeAny( (sal_Bool)IsFrameDragSingles() ) );

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsMarkedHitMovesAlways ) );
//  pValue->Value <<= (sal_Bool)IsMarkedHitMovesAlways();
//  pValue++;nIndex++;

    aUserData.addValue( sUNO_View_EliminatePolyPointLimitAngle, makeAny( (sal_Int32)GetEliminatePolyPointLimitAngle() ) );
    aUserData.addValue( sUNO_View_IsEliminatePolyPoints, makeAny( (sal_Bool)IsEliminatePolyPoints() ) );

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsLineDraft ) );
//  pValue->Value <<= (sal_Bool)IsLineDraft();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsFillDraft ) );
//  pValue->Value <<= (sal_Bool)IsFillDraft();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsTextDraft ) );
//  pValue->Value <<= (sal_Bool)IsTextDraft();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsGrafDraft ) );
//  pValue->Value <<= (sal_Bool)IsGrafDraft();
//  pValue++;nIndex++;

    Any aAny;
    GetVisibleLayers().QueryValue( aAny );
    aUserData.addValue( sUNO_View_VisibleLayers, aAny );

    GetPrintableLayers().QueryValue( aAny );
    aUserData.addValue( sUNO_View_PrintableLayers, aAny );

    GetLockedLayers().QueryValue( aAny );
    aUserData.addValue( sUNO_View_LockedLayers, aAny );

    aUserData.addValue( sUNO_View_NoAttribs, makeAny( (sal_Bool)IsNoAttribs() ) );
    aUserData.addValue( sUNO_View_NoColors, makeAny( (sal_Bool)IsNoColors() ) );

    if( GetStandardHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesDrawing, makeAny( createHelpLinesString( GetStandardHelpLines() ) ) );

    if( GetNotesHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesNotes, makeAny( createHelpLinesString( GetNotesHelpLines() ) ) );

    if( GetHandoutHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesHandout, makeAny( createHelpLinesString( GetHandoutHelpLines() ) ) );

    aUserData.addValue( sUNO_View_RulerIsVisible, makeAny( (sal_Bool)HasRuler() ) );
    aUserData.addValue( sUNO_View_PageKind, makeAny( (sal_Int16)GetPageKind() ) );
    aUserData.addValue( sUNO_View_SelectedPage, makeAny( (sal_Int16)GetSelectedPage() ) );
    aUserData.addValue( sUNO_View_IsLayerMode, makeAny( (sal_Bool)IsLayerMode() ) );

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsQuickEdit ) );
//  pValue->Value <<= (sal_Bool)IsQuickEdit();
//  pValue++;nIndex++;

    aUserData.addValue( sUNO_View_IsBigHandles, makeAny( (sal_Bool)IsBigHandles() ) );
    aUserData.addValue( sUNO_View_IsDoubleClickTextEdit,  makeAny( (sal_Bool)IsDoubleClickTextEdit() ) );
    aUserData.addValue( sUNO_View_IsClickChangeRotation, makeAny( (sal_Bool)IsClickChangeRotation() ) );

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsDragWithCopy ) );
//  pValue->Value <<= (sal_Bool)IsDragWithCopy();
//  pValue++;nIndex++;

    aUserData.addValue( sUNO_View_SlidesPerRow, makeAny( (sal_Int16)GetSlidesPerRow() ) );
/* #107128# Product managment decided to not make this persistent
    aUserData.addValue( sUNO_View_DrawMode, makeAny( (sal_Int32)GetDrawMode() ) );
    aUserData.addValue( sUNO_View_PreviewDrawMode, makeAny( (sal_Int32)GetPreviewDrawMode() ) );
*/
    aUserData.addValue( sUNO_View_IsShowPreviewInPageMode, makeAny( (sal_Bool)IsShowPreviewInPageMode() ) );
    aUserData.addValue( sUNO_View_IsShowPreviewInMasterPageMode, makeAny( (sal_Bool)IsShowPreviewInMasterPageMode() ) );
    aUserData.addValue( sUNO_View_SetShowPreviewInOutlineMode, makeAny( (sal_Bool)IsShowPreviewInOutlineMode() ) );
    aUserData.addValue( sUNO_View_EditModeStandard, makeAny( (sal_Int32)GetViewShEditMode( PK_STANDARD ) ) );
    aUserData.addValue( sUNO_View_EditModeNotes, makeAny( (sal_Int32)GetViewShEditMode( PK_NOTES ) ) );
    aUserData.addValue( sUNO_View_EditModeHandout, makeAny( (sal_Int32)GetViewShEditMode( PK_HANDOUT ) ) );

    {
        const Rectangle aVisArea = GetVisArea();

        aUserData.addValue( sUNO_View_VisibleAreaTop, makeAny( (sal_Int32)aVisArea.Top() ) );
        aUserData.addValue( sUNO_View_VisibleAreaLeft, makeAny( (sal_Int32)aVisArea.Left() ) );
        aUserData.addValue( sUNO_View_VisibleAreaWidth, makeAny( (sal_Int32)aVisArea.GetWidth() ) );
        aUserData.addValue( sUNO_View_VisibleAreaHeight, makeAny( (sal_Int32)aVisArea.GetHeight() ) );
    }

    aUserData.addValue( sUNO_View_GridCoarseWidth, makeAny( (sal_Int32)GetGridCoarse().Width() ) );
    aUserData.addValue( sUNO_View_GridCoarseHeight, makeAny( (sal_Int32)GetGridCoarse().Height() ) );
    aUserData.addValue( sUNO_View_GridFineWidth, makeAny( (sal_Int32)GetGridFine().Width() ) );
    aUserData.addValue( sUNO_View_GridFineHeight, makeAny( (sal_Int32)GetGridFine().Height() ) );
    aUserData.addValue( sUNO_View_GridSnapWidth, makeAny( (sal_Int32)GetSnapGrid().Width() ) );
    aUserData.addValue( sUNO_View_GridSnapHeight, makeAny( (sal_Int32)GetSnapGrid().Height() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthXNumerator, makeAny( (sal_Int32)GetSnapGridWidthX().GetNumerator() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthXDenominator, makeAny( (sal_Int32)GetSnapGridWidthX().GetDenominator() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthYNumerator, makeAny( (sal_Int32)GetSnapGridWidthY().GetNumerator() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthYDenominator, makeAny( (sal_Int32)GetSnapGridWidthY().GetDenominator() ) );
    aUserData.addValue( sUNO_View_IsAngleSnapEnabled, makeAny( (sal_Bool)IsAngleSnapEnabled() ) );
    aUserData.addValue( sUNO_View_SnapAngle, makeAny( (sal_Int32)GetSnapAngle() ) );

    const sal_Int32 nOldLength = rValues.getLength();
    rValues.realloc( nOldLength + aUserData.size() );

    PropertyValue* pValue = &(rValues.getArray()[nOldLength]);

    std::vector< std::pair< OUString, Any > >::iterator aIter( aUserData.begin() );
    for( ; aIter != aUserData.end(); aIter++, pValue++ )
    {
        pValue->Name = (*aIter).first;
        pValue->Value = (*aIter).second;
    }
}
#undef addValue

static void createHelpLinesFromString( const rtl::OUString& rLines, SdrHelpLineList& rHelpLines )
{
    const sal_Unicode * pStr = rLines.getStr();
    SdrHelpLine aNewHelpLine;
    rtl::OUStringBuffer sBuffer;

    while( *pStr )
    {
        Point aPoint;

        switch( *pStr )
        {
        case (sal_Unicode)'P':
            aNewHelpLine.SetKind( SDRHELPLINE_POINT );
            break;
        case (sal_Unicode)'V':
            aNewHelpLine.SetKind( SDRHELPLINE_VERTICAL );
            break;
        case (sal_Unicode)'H':
            aNewHelpLine.SetKind( SDRHELPLINE_HORIZONTAL );
            break;
        default:
            DBG_ERROR( "syntax error in snap lines settings string" );
            return;
        }

        pStr++;

        while( (*pStr >= sal_Unicode('0') && *pStr <= sal_Unicode('9')) || (*pStr == '+') || (*pStr == '-') )
        {
            sBuffer.append( *pStr++ );
        }

        sal_Int32 nValue = sBuffer.makeStringAndClear().toInt32();

        if( aNewHelpLine.GetKind() == SDRHELPLINE_HORIZONTAL )
        {
            aPoint.Y() = nValue;
        }
        else
        {
            aPoint.X() = nValue;

            if( aNewHelpLine.GetKind() == SDRHELPLINE_POINT )
            {
                if( *pStr++ != ',' )
                    return;

                while( (*pStr >= sal_Unicode('0') && *pStr <= sal_Unicode('9')) || (*pStr == '+') || (*pStr == '-')  )
                {
                    sBuffer.append( *pStr++ );
                }

                aPoint.Y() = sBuffer.makeStringAndClear().toInt32();

            }
        }

        aNewHelpLine.SetPos( aPoint );
        rHelpLines.Insert( aNewHelpLine );
    }
}

void FrameView::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    const sal_Int32 nLength = rSequence.getLength();
    if (nLength)
    {
        sal_Bool bBool;
        sal_Int32 nInt32;
        sal_Int16 nInt16;
        rtl::OUString aString;

        sal_Int32 aSnapGridWidthXNum = GetSnapGridWidthX().GetNumerator();
        sal_Int32 aSnapGridWidthXDom = GetSnapGridWidthX().GetDenominator();

        sal_Int32 aSnapGridWidthYNum = GetSnapGridWidthY().GetNumerator();
        sal_Int32 aSnapGridWidthYDom = GetSnapGridWidthY().GetDenominator();

        const com::sun::star::beans::PropertyValue *pValue = rSequence.getConstArray();
        for (sal_Int16 i = 0 ; i < nLength; i++, pValue++ )
        {
            if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_ViewId ) ) )
            {
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapLinesDrawing ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SdrHelpLineList aHelpLines;
                    createHelpLinesFromString( aString, aHelpLines );
                    SetStandardHelpLines( aHelpLines );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapLinesNotes ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SdrHelpLineList aHelpLines;
                    createHelpLinesFromString( aString, aHelpLines );
                    SetNotesHelpLines( aHelpLines );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapLinesHandout ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SdrHelpLineList aHelpLines;
                    createHelpLinesFromString( aString, aHelpLines );
                    SetHandoutHelpLines( aHelpLines );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_RulerIsVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetRuler( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PageKind ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    SetPageKind( (PageKind)nInt16 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SelectedPage ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    SetSelectedPage( (USHORT)nInt16 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsLayerMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetLayerMode( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsQuickEdit ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetQuickEdit( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsBigHandles ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetBigHandles( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsDoubleClickTextEdit ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetDoubleClickTextEdit( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsClickChangeRotation ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetClickChangeRotation( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsDragWithCopy ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetDragWithCopy( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SlidesPerRow ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    SetSlidesPerRow( (USHORT)nInt16 );
                }
            }
/* #107128# Product managment decided to not make this persistent
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_DrawMode ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                    if( rStyleSettings.GetHighContrastMode() )
                        continue;
                    SetDrawMode( (ULONG)nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PreviewDrawMode ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                    SvtAccessibilityOptions aAccOptions;
                    if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
                        continue;
                    SetPreviewDrawMode( (ULONG)nInt32 );
                }
            }
*/
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsShowPreviewInPageMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetShowPreviewInPageMode( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsShowPreviewInMasterPageMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetShowPreviewInMasterPageMode( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SetShowPreviewInOutlineMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetShowPreviewInOutlineMode( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeStandard ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SetViewShEditMode( (EditMode)nInt32, PK_STANDARD );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeNotes ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SetViewShEditMode( (EditMode)nInt32, PK_NOTES );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeHandout ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SetViewShEditMode( (EditMode)nInt32, PK_HANDOUT );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaTop ) ) )
            {
                sal_Int32 nTop;
                if( pValue->Value >>= nTop )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.nBottom += nTop - aVisArea.nTop;
                    aVisArea.nTop = nTop;
                    SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaLeft ) ) )
            {
                sal_Int32 nLeft;
                if( pValue->Value >>= nLeft )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.nRight += nLeft - aVisArea.nLeft;
                    aVisArea.nLeft = nLeft;
                    SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaWidth ) ) )
            {
                sal_Int32 nWidth;
                if( pValue->Value >>= nWidth )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.nRight = aVisArea.nLeft + nWidth - 1;
                    SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaHeight ) ) )
            {
                sal_Int32 nHeight;
                if( pValue->Value >>= nHeight )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.nBottom = nHeight + aVisArea.nTop - 1;
                    SetVisArea( aVisArea );
                }
            }

            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridIsVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetGridVisible( bBool );
                }
            }

            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToGrid ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetGridSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridIsFront ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetGridFront( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToPageMargins ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetBordSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToSnapLines ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetHlplSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToObjectFrame ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetOFrmSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToObjectPoints ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetOPntSnap( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapLinesVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetHlplVisible( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsDragStripes ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetDragStripes( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsPlusHandlesAlwaysVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetPlusHandlesAlwaysVisible( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsFrameDragSingles ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetFrameDragSingles( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsMarkedHitMovesAlways ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetMarkedHitMovesAlways( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EliminatePolyPointLimitAngle ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SetEliminatePolyPointLimitAngle( nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsEliminatePolyPoints ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetEliminatePolyPoints( bBool );
                }
            }
/*
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsLineDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetLineDraft( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsFillDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetFillDraft( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsTextDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetTextDraft( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsGrafDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetGrafDraft( bBool );
                }
            }
*/
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_ActiveLayer ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SetActiveLayer( aString );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_NoAttribs ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetNoAttribs( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_NoColors ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetNoColors( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridCoarseWidth ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( nInt32, GetGridCoarse().Height() );
                    SetGridCoarse( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridCoarseHeight ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( GetGridCoarse().Width(), nInt32 );
                    SetGridCoarse( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridFineWidth ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( nInt32, GetGridFine().Height() );
                    SetGridFine( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridFineHeight ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( GetGridFine().Width(), nInt32 );
                    SetGridFine( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidth ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( nInt32, GetSnapGrid().Height() );
                    SetSnapGrid( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapHeight ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( GetSnapGrid().Width(), nInt32 );
                    SetSnapGrid( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsAngleSnapEnabled ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    SetAngleSnapEnabled( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapAngle ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SetSnapAngle( nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthXNumerator ) ) )
            {
                pValue->Value >>= aSnapGridWidthXNum;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthXDenominator ) ) )
            {
                pValue->Value >>= aSnapGridWidthXDom;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthYNumerator ) ) )
            {
                pValue->Value >>= aSnapGridWidthYNum;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthYDenominator ) ) )
            {
                pValue->Value >>= aSnapGridWidthYDom;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetVisibleLayers( aSetOfBytes );
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PrintableLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetPrintableLayers( aSetOfBytes );
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_LockedLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetLockedLayers( aSetOfBytes );
            }
        }

        const Fraction aSnapGridWidthX( aSnapGridWidthXNum, aSnapGridWidthXDom );
        const Fraction aSnapGridWidthY( aSnapGridWidthYNum, aSnapGridWidthYDom );

        SetSnapGridWidth( aSnapGridWidthX, aSnapGridWidthY );
    }
}




void FrameView::SetPreviousViewShellType (ViewShell::ShellType eType)
{
    mePreviousViewShellType = eType;
}




ViewShell::ShellType FrameView::GetPreviousViewShellType (void) const
{
    return mePreviousViewShellType;
}



void FrameView::SetSelectedPage(USHORT nPage)
{
    nSelectedPage = nPage;
}




const USHORT FrameView::GetSelectedPage (void) const
{
    return nSelectedPage;
}

} // end of namespace sd
