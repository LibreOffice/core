/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: frmview.cxx,v $
 * $Revision: 1.38 $
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

#include "FrameView.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <sfx2/topfrm.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <rtl/ustrbuf.hxx>
#include "unokywds.hxx"

#include <vector>
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "optsitem.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "app.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "glob.hrc"
#include "sdiocmpt.hxx"
#include "framework/FrameworkHelper.hxx"
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::std;
using ::rtl::OUString;

namespace sd {

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

FrameView::FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView /* = NULK */)
  : SdrView(pDrawDoc, (OutputDevice*) NULL),
    mnRefCount(0),
    mnPresViewShellId(SID_VIEWSHELL0),
    mnSlotId(SID_OBJECT_SELECT),
    mbIsNavigatorShowingAllShapes(false)
{
    EndListening(*pDrawDoc);

    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);

    SetGridFront( FALSE );
    SetHlplFront( FALSE );
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
                // Count the FrameViews and remember the type of the main
                // view shell.
                pSfxViewSh = pSfxViewFrame->GetViewShell();
                pBase = PTR_CAST(ViewShellBase, pSfxViewSh );

                if (pBase != NULL)
                {
                    nSdViewShellCount++;

                    ::rtl::OUString sViewURL;
                    Reference<drawing::framework::XView> xView (
                        framework::FrameworkHelper::Instance(*pBase)->GetView(
                            drawing::framework::ResourceId::create(
                                comphelper_getProcessComponentContext(),
                                framework::FrameworkHelper::msCenterPaneURL)));
                    if (xView.is())
                        sViewURL = xView->getResourceId()->getResourceURL();

                    switch (framework::FrameworkHelper::GetViewId(sViewURL))
                    {
                        default:
//                        case ViewShell::ST_IMPRESS:
//                        case ViewShell::ST_NOTES:
//                        case ViewShell::ST_HANDOUT:
                            mnPresViewShellId = SID_VIEWSHELL0;
                            break;

                        case ViewShell::ST_SLIDE_SORTER:
                            mnPresViewShellId = SID_VIEWSHELL1;
                            break;

                        case ViewShell::ST_OUTLINE:
                            mnPresViewShellId = SID_VIEWSHELL2;
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

        SetSolidMarkHdl( pFrameView->IsSolidMarkHdl() );
        SetSolidDragging( pFrameView->IsSolidDragging() );

        maVisibleLayers = pFrameView->GetVisibleLayers();
        maPrintableLayers = pFrameView->GetPrintableLayers();
        maLockedLayers = pFrameView->GetLockedLayers();
        maStandardHelpLines = pFrameView->GetStandardHelpLines();
        maNotesHelpLines = pFrameView->GetNotesHelpLines();
        maHandoutHelpLines = pFrameView->GetHandoutHelpLines();
        SetActiveLayer( pFrameView->GetActiveLayer() );
        mbNoColors = pFrameView->IsNoColors();
        mbNoAttribs = pFrameView->IsNoAttribs() ;
        maVisArea = pFrameView->GetVisArea();
        mePageKind = pFrameView->GetPageKind();
        mePageKindOnLoad = pFrameView->GetPageKindOnLoad();
        mnSelectedPage = pFrameView->GetSelectedPage();
        mnSelectedPageOnLoad = pFrameView->GetSelectedPageOnLoad();
        meStandardEditMode = pFrameView->GetViewShEditMode(PK_STANDARD);
        meNotesEditMode = pFrameView->GetViewShEditMode(PK_NOTES);
        meHandoutEditMode = pFrameView->GetViewShEditMode(PK_HANDOUT);
        SetViewShEditModeOnLoad(pFrameView->GetViewShEditModeOnLoad());
        mbLayerMode = pFrameView->IsLayerMode();
        mbQuickEdit = pFrameView->IsQuickEdit();

        // #i26631#
        SetMasterPagePaintCaching( pFrameView->IsMasterPagePaintCaching() );

        SetDragWithCopy( pFrameView->IsDragWithCopy() );
        mbBigHandles         = pFrameView->IsBigHandles();
        mbDoubleClickTextEdit = pFrameView->IsDoubleClickTextEdit();
        mbClickChangeRotation = pFrameView->IsClickChangeRotation();
        mnSlidesPerRow = pFrameView->GetSlidesPerRow();
        mnDrawMode = pFrameView->GetDrawMode();
        mnTabCtrlPercent = pFrameView->GetTabCtrlPercent();
        mbIsNavigatorShowingAllShapes = pFrameView->IsNavigatorShowingAllShapes();
        SetPreviousViewShellType (pFrameView->GetPreviousViewShellType());
        SetViewShellTypeOnLoad (pFrameView->GetViewShellTypeOnLoad());
    }
    else
    {
        /**********************************************************************
        * FrameView mit den Applikationsdaten initialisieren
        **********************************************************************/
        maVisibleLayers.SetAll();
        maPrintableLayers.SetAll();
        SetGridCoarse( Size( 1000, 1000 ) );
        SetSnapGridWidth(Fraction(1000, 1), Fraction(1000, 1));
        SetActiveLayer( String( SdResId(STR_LAYER_LAYOUT) ) );
        mbNoColors = TRUE;
        mbNoAttribs = FALSE;
        maVisArea = Rectangle( Point(), Size(0, 0) );
        mePageKind = PK_STANDARD;
        mePageKindOnLoad = PK_STANDARD;
        mnSelectedPage = 0;
        mnSelectedPageOnLoad = 0;
        meStandardEditMode = EM_PAGE;
        meNotesEditMode = EM_PAGE;
        meHandoutEditMode = EM_MASTERPAGE;
        SetViewShEditModeOnLoad(EM_PAGE);
        mbLayerMode = FALSE;
        SetEliminatePolyPoints(FALSE);
        mbBigHandles = FALSE;
        mbDoubleClickTextEdit = FALSE;
        mbClickChangeRotation = FALSE;
        mnSlidesPerRow = 4;

        {
            bool bUseContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
            mnDrawMode = bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR;
        }
        mnTabCtrlPercent = 0.0;
        mbIsNavigatorShowingAllShapes = false;
        SetPreviousViewShellType (ViewShell::ST_NONE);
        SetViewShellTypeOnLoad (ViewShell::ST_IMPRESS);

        // get default for design mode
        sal_Bool bInitDesignMode = pDrawDoc->GetOpenInDesignMode();
        if( pDrawDoc->OpenInDesignModeIsDefaulted() )
        {
            bInitDesignMode = sal_True;
        }

        SfxObjectShell* pObjShell = pDrawDoc->GetObjectShell();
        if( pObjShell && pObjShell->IsReadOnly() )
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
    mnRefCount++;
}


/*************************************************************************
|*
|* Verbindung loesen
|*
\************************************************************************/

void FrameView::Disconnect()
{
    if (mnRefCount > 0)
    {
        mnRefCount--;
    }

    if (mnRefCount == 0)
    {
        delete this;
    }
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
        mbRuler = pOptions->IsRulerVisible();
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

        SetSolidMarkHdl( pOptions->IsSolidMarkHdl() );
        SetSolidDragging( pOptions->IsSolidDragging() );

        SetGridCoarse( Size( pOptions->GetFldDrawX(), pOptions->GetFldDrawY() ) );
        SetGridFine( Size( pOptions->GetFldDivisionX(), pOptions->GetFldDivisionY() ) );
        Fraction aFractX(pOptions->GetFldDrawX(), pOptions->GetFldDrawX() / ( pOptions->GetFldDivisionX() ? pOptions->GetFldDivisionX() : 1 ));
        Fraction aFractY(pOptions->GetFldDrawY(), pOptions->GetFldDrawY() / ( pOptions->GetFldDivisionY() ? pOptions->GetFldDivisionY() : 1 ));
        SetSnapGridWidth(aFractX, aFractY);
        SetQuickEdit(pOptions->IsQuickEdit());

        // #i26631#
        SetMasterPagePaintCaching( pOptions->IsMasterPagePaintCaching() );

        SetDragWithCopy(pOptions->IsDragWithCopy());
        SetBigHandles( pOptions->IsBigHandles() );
        SetDoubleClickTextEdit( pOptions->IsDoubleClickTextEdit() );
        SetClickChangeRotation( pOptions->IsClickChangeRotation() );
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
        meStandardEditMode = eMode;
    }
    else if (eKind == PK_NOTES)
    {
        meNotesEditMode = eMode;
    }
    else if (eKind == PK_HANDOUT)
    {
        meHandoutEditMode = eMode;
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
        eMode = meStandardEditMode;
    }
    else if (eKind == PK_NOTES)
    {
        eMode = meNotesEditMode;
    }
    else if (eKind == PK_HANDOUT)
    {
        eMode = meHandoutEditMode;
    }

    return (eMode);
}




void FrameView::SetViewShEditModeOnLoad (EditMode eMode)
{
    meEditModeOnLoad = eMode;
}




EditMode FrameView::GetViewShEditModeOnLoad (void) const
{
    return meEditModeOnLoad;
}




static OUString createHelpLinesString( const SdrHelpLineList& rHelpLines )
{
    ::rtl::OUStringBuffer aLines;

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
void FrameView::WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rValues, sal_Bool )
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

void FrameView::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool )
{
    const sal_Int32 nLength = rSequence.getLength();
    if (nLength)
    {
        const bool bImpress = dynamic_cast< SdDrawDocument* >(GetModel())->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;

        sal_Bool bBool = sal_False;
        sal_Int32 nInt32 = 0;
        sal_Int16 nInt16 = 0;
        rtl::OUString aString;

        sal_Int32 aSnapGridWidthXNum = GetSnapGridWidthX().GetNumerator();
        sal_Int32 aSnapGridWidthXDom = GetSnapGridWidthX().GetDenominator();

        sal_Int32 aSnapGridWidthYNum = GetSnapGridWidthY().GetNumerator();
        sal_Int32 aSnapGridWidthYDom = GetSnapGridWidthY().GetDenominator();

        EditMode eStandardEditMode;
        EditMode eNotesEditMode;
        EditMode eHandoutEditMode;

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
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SFX_CREATE_MODE_EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetPageKind( (PageKind)nInt16 );

                    SetPageKindOnLoad( (PageKind)nInt16 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SelectedPage ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SFX_CREATE_MODE_EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetSelectedPage( (USHORT)nInt16 );

                    SetSelectedPageOnLoad( (USHORT)nInt16 );
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
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeStandard ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SFX_CREATE_MODE_EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetViewShEditMode( (EditMode)nInt32, PK_STANDARD );
                    eStandardEditMode = (EditMode)nInt32;
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeNotes ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SFX_CREATE_MODE_EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetViewShEditMode( (EditMode)nInt32, PK_NOTES );
                    eNotesEditMode = (EditMode)nInt32;
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeHandout ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SFX_CREATE_MODE_EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetViewShEditMode( (EditMode)nInt32, PK_HANDOUT );
                    eHandoutEditMode = (EditMode)nInt32;
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaTop ) ) )
            {
                sal_Int32 nTop = 0;
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
                sal_Int32 nLeft = 0;
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
                sal_Int32 nWidth = 0;
                if( pValue->Value >>= nWidth )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.nRight = aVisArea.nLeft + nWidth - 1;
                    SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaHeight ) ) )
            {
                sal_Int32 nHeight = 0;
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
            else if (!bImpress && pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetVisibleLayers( aSetOfBytes );
            }
            else if (!bImpress && pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PrintableLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetPrintableLayers( aSetOfBytes );
            }
            else if (!bImpress && pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_LockedLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetLockedLayers( aSetOfBytes );
            }
        }

        switch (GetPageKindOnLoad())
        {
            case PK_STANDARD: SetViewShEditModeOnLoad(meStandardEditMode); break;
            case PK_NOTES: SetViewShEditModeOnLoad(meNotesEditMode); break;
            case PK_HANDOUT: SetViewShEditModeOnLoad(meHandoutEditMode); break;
            default: SetViewShEditModeOnLoad(EM_PAGE); break;
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




void FrameView::SetViewShellTypeOnLoad (ViewShell::ShellType eType)
{
    meViewShellTypeOnLoad = eType;
}




ViewShell::ShellType FrameView::GetViewShellTypeOnLoad (void) const
{
    return meViewShellTypeOnLoad;
}




void FrameView::SetSelectedPage(USHORT nPage)
{
    mnSelectedPage = nPage;
}




const USHORT FrameView::GetSelectedPage (void) const
{
    return mnSelectedPage;
}




void FrameView::SetIsNavigatorShowingAllShapes (const bool bIsNavigatorShowingAllShapes)
{
    mbIsNavigatorShowingAllShapes = bIsNavigatorShowingAllShapes;
}




bool FrameView::IsNavigatorShowingAllShapes (void) const
{
    return mbIsNavigatorShowingAllShapes;
}


} // end of namespace sd
