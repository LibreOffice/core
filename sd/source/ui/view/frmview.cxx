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

#include <FrameView.hxx>

#include <svx/svxids.hrc>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <rtl/ustrbuf.hxx>
#include <unokywds.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>

#include <vector>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <optsitem.hxx>
#include <ViewShellBase.hxx>
#include <sdmod.hxx>
#include <pres.hxx>
#include <framework/FrameworkHelper.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Draw.hxx>
#include <officecfg/Office/Impress.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace sd {

FrameView::FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView /* = NULL */)
:   SdrView(*pDrawDoc, nullptr), // TTTT SdDrawDocument* -> should be reference
    mnRefCount(0),
    mnPresViewShellId(SID_VIEWSHELL0),
    mbIsNavigatorShowingAllShapes(false)
{
    EndListening(*pDrawDoc);

    EnableExtendedKeyInputDispatcher(false);
    EnableExtendedMouseEventDispatcher(false);

    SetGridFront( false );
    SetHlplFront( false );
    SetOConSnap( false );
    SetFrameDragSingles();
    SetSlidesPerRow(4);

    if( nullptr == pFrameView )
    {
        DrawDocShell* pDocShell = pDrawDoc->GetDocSh();

        if ( pDocShell )
        {
            // document is loaded, is there a FrameView?
            sal_uLong nSdViewShellCount = 0;
            SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(pDocShell);

            while (pSfxViewFrame)
            {
                // Count the FrameViews and remember the type of the main
                // view shell.
                SfxViewShell* pSfxViewSh = pSfxViewFrame->GetViewShell();
                ViewShellBase* pBase = dynamic_cast<ViewShellBase*>( pSfxViewSh  );

                if (pBase != nullptr)
                {
                    nSdViewShellCount++;

                    OUString sViewURL;
                    Reference<drawing::framework::XView> xView (
                        framework::FrameworkHelper::Instance(*pBase)->GetView(
                            drawing::framework::ResourceId::create(
                                ::comphelper::getProcessComponentContext(),
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

                pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, pDocShell);
            }

            SdDrawDocument* pDoc = pDocShell->GetDoc();
            pFrameView = pDoc->GetFrameView(nSdViewShellCount);
        }
    }

    if (pFrameView)
    {
        // initialize FrameView with the FrameView of the DocShell
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
        SetDesignMode( pFrameView->IsDesignMode() );

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
        mePageEditMode = pFrameView->GetViewShEditMode();
        // meStandardEditMode = pFrameView->GetViewShEditMode(PageKind::Standard);
        // meNotesEditMode = pFrameView->GetViewShEditMode(PageKind::Notes);
        // meHandoutEditMode = pFrameView->GetViewShEditMode(PageKind::Handout);
        SetViewShEditModeOnLoad(pFrameView->GetViewShEditModeOnLoad());
        mbLayerMode = pFrameView->IsLayerMode();
        mbQuickEdit = pFrameView->IsQuickEdit();

        // #i26631#
        SetMasterPagePaintCaching( pFrameView->IsMasterPagePaintCaching() );

        SetDragWithCopy( pFrameView->IsDragWithCopy() );
        mbDoubleClickTextEdit = pFrameView->IsDoubleClickTextEdit();
        mbClickChangeRotation = pFrameView->IsClickChangeRotation();
        mnSlidesPerRow = pFrameView->GetSlidesPerRow();
        mnDrawMode = pFrameView->GetDrawMode();
        mbIsNavigatorShowingAllShapes = pFrameView->IsNavigatorShowingAllShapes();
        SetPreviousViewShellType (pFrameView->GetPreviousViewShellType());
        SetViewShellTypeOnLoad (pFrameView->GetViewShellTypeOnLoad());
    }
    else
    {
        // initialize FrameView with the application data

        // Layers need to be set, otherwise they are not visible and not printable in
        // Impress documents. The document contains already the actual layers and their
        // settings for visible, printable and locked. In case not read from <draw:layer-set>,
        // ODF defaults are used.
        SdrLayerAdmin rLayerAdmin = pDrawDoc -> GetLayerAdmin();
        rLayerAdmin.getVisibleLayersODF(maVisibleLayers);
        rLayerAdmin.getPrintableLayersODF(maPrintableLayers);
        rLayerAdmin.getLockedLayersODF(maLockedLayers);
        SetGridCoarse( Size( 1000, 1000 ) );
        SetSnapGridWidth(Fraction(1000, 1), Fraction(1000, 1));
        SetActiveLayer(sUNO_LayerName_layout);
        mbNoColors = true;
        mbNoAttribs = false;
        maVisArea = ::tools::Rectangle( Point(), Size(0, 0) );
        mePageKind = PageKind::Standard;
        mePageKindOnLoad = PageKind::Standard;
        mnSelectedPage = 0;
        mnSelectedPageOnLoad = 0;
        mePageEditMode = EditMode::Page;
        // meStandardEditMode = EditMode::Page;
        // meNotesEditMode = EditMode::Page;
        // meHandoutEditMode = EditMode::MasterPage;
        SetViewShEditModeOnLoad(EditMode::Page);
        mbLayerMode = false;
        SetEliminatePolyPoints(false);
        mbDoubleClickTextEdit = false;
        mbClickChangeRotation = false;
        mnSlidesPerRow = officecfg::Office::Impress::Misc::SorterSlidesPerRow::get();

        {
            bool bUseContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
            mnDrawMode = bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR;
        }
        mbIsNavigatorShowingAllShapes = true;
        SetPreviousViewShellType (ViewShell::ST_NONE);
        SetViewShellTypeOnLoad (ViewShell::ST_IMPRESS);

        // get default for design mode
        bool bInitDesignMode = pDrawDoc->GetOpenInDesignMode();
        if( pDrawDoc->OpenInDesignModeIsDefaulted() )
        {
            bInitDesignMode = true;
        }

        SfxObjectShell* pObjShell = pDrawDoc->GetObjectShell();
        if( pObjShell && pObjShell->IsReadOnly() )
            bInitDesignMode = false;
        SetDesignMode( bInitDesignMode );

        Update( SD_MOD()->GetSdOptions(pDrawDoc->GetDocumentType()) );
    }

}

FrameView::~FrameView()
{
}

void FrameView::Connect()
{
    mnRefCount++;
}

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

/**
 * Update with data from the specified SdOptions
 */
void FrameView::Update(SdOptions const * pOptions)
{
    if (!pOptions)
        return;

    SdDrawDocument* pDrawDocument = dynamic_cast<SdDrawDocument*>(&GetModel());
    const bool bImpress = pDrawDocument && pDrawDocument->GetDocumentType() == DocumentType::Impress;

    if (bImpress)
    {
        mbRuler = officecfg::Office::Impress::Layout::Display::Ruler::get();
        SetDragStripes( officecfg::Office::Impress::Layout::Display::Guide::get() );
        SetHlplVisible( officecfg::Office::Impress::Layout::Display::Helpline::get() );
        SetNoDragXorPolys ( !officecfg::Office::Impress::Layout::Display::Contour::get() );
        SetPlusHandlesAlwaysVisible( officecfg::Office::Impress::Layout::Display::Bezier::get() );
    }
    else
    {
        mbRuler = officecfg::Office::Draw::Layout::Display::Ruler::get();
        SetDragStripes( officecfg::Office::Draw::Layout::Display::Guide::get() );
        SetHlplVisible( officecfg::Office::Draw::Layout::Display::Helpline::get() );
        SetNoDragXorPolys ( !officecfg::Office::Draw::Layout::Display::Contour::get() );
        SetPlusHandlesAlwaysVisible( officecfg::Office::Draw::Layout::Display::Bezier::get() );
    }

    SetGridVisible( pOptions->IsGridVisible() );
    SetSnapAngle( pOptions->GetAngle() );
    SetGridSnap( pOptions->IsUseGridSnap() );
    SetBordSnap( pOptions->IsSnapBorder()  );
    SetHlplSnap( pOptions->IsSnapHelplines() );
    SetOFrmSnap( pOptions->IsSnapFrame() );
    SetOPntSnap( pOptions->IsSnapPoints() );
    SetSnapMagneticPixel( pOptions->GetSnapArea() );
    SetMarkedHitMovesAlways( pOptions->IsMarkedHitMovesAlways() );
    SetMoveOnlyDragging( pOptions->IsMoveOnlyDragging() );
    SetSlantButShear( pOptions->IsMoveOnlyDragging() );
    SetCrookNoContortion( pOptions->IsCrookNoContortion() );
    SetAngleSnapEnabled( pOptions->IsRotate() );
    SetBigOrtho( pOptions->IsBigOrtho() );
    SetOrtho( pOptions->IsOrtho() );
    SetEliminatePolyPointLimitAngle( pOptions->GetEliminatePolyPointLimitAngle() );
    GetModel().SetPickThroughTransparentTextFrames( pOptions->IsPickThrough() );

    SetSolidDragging( pOptions->IsSolidDragging() );

    SetGridCoarse( Size( pOptions->GetFieldDrawX(), pOptions->GetFieldDrawY() ) );
    SetGridFine( Size( pOptions->GetFieldDivisionX(), pOptions->GetFieldDivisionY() ) );
    Fraction aFractX(pOptions->GetFieldDrawX(), pOptions->GetFieldDrawX() / ( pOptions->GetFieldDivisionX() ? pOptions->GetFieldDivisionX() : 1 ));
    Fraction aFractY(pOptions->GetFieldDrawY(), pOptions->GetFieldDrawY() / ( pOptions->GetFieldDivisionY() ? pOptions->GetFieldDivisionY() : 1 ));
    SetSnapGridWidth(aFractX, aFractY);
    SetQuickEdit(pOptions->IsQuickEdit());

    // #i26631#
    SetMasterPagePaintCaching( pOptions->IsMasterPagePaintCaching() );

    SetDragWithCopy(pOptions->IsDragWithCopy());
    SetDragThresholdPixels(pOptions->GetDragThresholdPixels());
    SetDoubleClickTextEdit( pOptions->IsDoubleClickTextEdit() );
    SetClickChangeRotation( pOptions->IsClickChangeRotation() );
}

/**
 * Set EditMode (Page or MasterPage) of working mode
 */
void FrameView::SetViewShEditMode(EditMode eMode)
{
    mePageEditMode = eMode;
}

/**
 * Return EditMode (Page or MasterPage) of working mode
 */
EditMode FrameView::GetViewShEditMode() const
{
    return mePageEditMode;
}

void FrameView::SetViewShEditModeOnLoad (EditMode eMode)
{
    meEditModeOnLoad = eMode;
}

static OUString createHelpLinesString( const SdrHelpLineList& rHelpLines )
{
    OUStringBuffer aLines;

    const sal_uInt16 nCount = rHelpLines.GetCount();
    for( sal_uInt16 nHlpLine = 0; nHlpLine < nCount; nHlpLine++ )
    {
        const SdrHelpLine& rHelpLine = rHelpLines[nHlpLine];
        const Point& rPos = rHelpLine.GetPos();

        switch( rHelpLine.GetKind() )
        {
            case SdrHelpLineKind::Point:
                aLines.append( 'P' );
                aLines.append( static_cast<sal_Int32>(rPos.X()) );
                aLines.append( ',' );
                aLines.append( static_cast<sal_Int32>(rPos.Y()) );
                break;
            case SdrHelpLineKind::Vertical:
                aLines.append( 'V' );
                aLines.append( static_cast<sal_Int32>(rPos.X()) );
                break;
            case SdrHelpLineKind::Horizontal:
                aLines.append( 'H' );
                aLines.append( static_cast<sal_Int32>(rPos.Y()) );
                break;
            default:
                OSL_FAIL( "Unsupported helpline Kind!" );
        }
    }

    return aLines.makeStringAndClear();
}

void FrameView::WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& rValues )
{
    std::vector< std::pair< OUString, Any > > aUserData;
    aUserData.reserve(41); // worst case

    aUserData.emplace_back( sUNO_View_GridIsVisible, Any( IsGridVisible() ) );
    aUserData.emplace_back( sUNO_View_GridIsFront, Any( IsGridFront() ) );
    aUserData.emplace_back( sUNO_View_IsSnapToGrid, Any( IsGridSnap() ) );
    aUserData.emplace_back( sUNO_View_IsSnapToPageMargins, Any( IsBordSnap() ) );
    aUserData.emplace_back( sUNO_View_IsSnapToSnapLines, Any( IsHlplSnap() ) );
    aUserData.emplace_back( sUNO_View_IsSnapToObjectFrame, Any( IsOFrmSnap() ) );
    aUserData.emplace_back( sUNO_View_IsSnapToObjectPoints, Any( IsOPntSnap() ) );

    aUserData.emplace_back( sUNO_View_IsPlusHandlesAlwaysVisible, Any( IsPlusHandlesAlwaysVisible() ) );
    aUserData.emplace_back( sUNO_View_IsFrameDragSingles, Any( IsFrameDragSingles() ) );

    aUserData.emplace_back( sUNO_View_EliminatePolyPointLimitAngle, Any( static_cast<sal_Int32>(GetEliminatePolyPointLimitAngle()) ) );
    aUserData.emplace_back( sUNO_View_IsEliminatePolyPoints, Any( IsEliminatePolyPoints() ) );

    if ( officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::get() )
    {
        SdrLayerAdmin& rLayerAdmin = getSdrModelFromSdrView().GetLayerAdmin();
        Any aAny;
        rLayerAdmin.QueryValue(GetVisibleLayers(), aAny);
        aUserData.emplace_back( sUNO_View_VisibleLayers, aAny );

        rLayerAdmin.QueryValue(GetPrintableLayers(), aAny);
        aUserData.emplace_back( sUNO_View_PrintableLayers, aAny );

        rLayerAdmin.QueryValue(GetLockedLayers(), aAny);
        aUserData.emplace_back( sUNO_View_LockedLayers, aAny );
    }

    aUserData.emplace_back( sUNO_View_NoAttribs, Any( IsNoAttribs() ) );
    aUserData.emplace_back( sUNO_View_NoColors, Any( IsNoColors() ) );

    if( GetStandardHelpLines().GetCount() )
        aUserData.emplace_back( sUNO_View_SnapLinesDrawing, Any( createHelpLinesString( GetStandardHelpLines() ) ) );

    if( GetNotesHelpLines().GetCount() )
        aUserData.emplace_back( sUNO_View_SnapLinesNotes, Any( createHelpLinesString( GetNotesHelpLines() ) ) );

    if( GetHandoutHelpLines().GetCount() )
        aUserData.emplace_back( sUNO_View_SnapLinesHandout, Any( createHelpLinesString( GetHandoutHelpLines() ) ) );

    aUserData.emplace_back( sUNO_View_RulerIsVisible, Any( HasRuler() ) );
    aUserData.emplace_back( sUNO_View_PageKind, Any( static_cast<sal_Int16>(GetPageKind()) ) );
    aUserData.emplace_back( sUNO_View_SelectedPage, Any( static_cast<sal_Int16>(GetSelectedPage()) ) );
    aUserData.emplace_back( sUNO_View_IsLayerMode, Any( IsLayerMode() ) );

    aUserData.emplace_back( sUNO_View_IsDoubleClickTextEdit,  Any( IsDoubleClickTextEdit() ) );
    aUserData.emplace_back( sUNO_View_IsClickChangeRotation, Any( IsClickChangeRotation() ) );

    aUserData.emplace_back( sUNO_View_SlidesPerRow, Any( static_cast<sal_Int16>(GetSlidesPerRow()) ) );
    aUserData.emplace_back( sUNO_View_EditMode, Any( static_cast<sal_Int32>(GetViewShEditMode()) ) );
    // aUserData.emplace_back( sUNO_View_EditModeStandard, makeAny( (sal_Int32)GetViewShEditMode( PageKind::Standard ) ) );
    // aUserData.emplace_back( sUNO_View_EditModeNotes, makeAny( (sal_Int32)GetViewShEditMode( PageKind::Notes ) ) );
    // aUserData.emplace_back( sUNO_View_EditModeHandout, makeAny( (sal_Int32)GetViewShEditMode( PageKind::Handout ) ) );

    {
        const ::tools::Rectangle aVisArea = GetVisArea();

        aUserData.emplace_back( sUNO_View_VisibleAreaTop, Any( static_cast<sal_Int32>(aVisArea.Top()) ) );
        aUserData.emplace_back( sUNO_View_VisibleAreaLeft, Any( static_cast<sal_Int32>(aVisArea.Left()) ) );
        aUserData.emplace_back( sUNO_View_VisibleAreaWidth, Any( static_cast<sal_Int32>(aVisArea.GetWidth()) ) );
        aUserData.emplace_back( sUNO_View_VisibleAreaHeight, Any( static_cast<sal_Int32>(aVisArea.GetHeight()) ) );
    }

    aUserData.emplace_back( sUNO_View_GridCoarseWidth, Any( static_cast<sal_Int32>(GetGridCoarse().Width()) ) );
    aUserData.emplace_back( sUNO_View_GridCoarseHeight, Any( static_cast<sal_Int32>(GetGridCoarse().Height()) ) );
    aUserData.emplace_back( sUNO_View_GridFineWidth, Any( static_cast<sal_Int32>(GetGridFine().Width()) ) );
    aUserData.emplace_back( sUNO_View_GridFineHeight, Any( static_cast<sal_Int32>(GetGridFine().Height()) ) );
    aUserData.emplace_back( sUNO_View_GridSnapWidthXNumerator, Any( GetSnapGridWidthX().GetNumerator() ) );
    aUserData.emplace_back( sUNO_View_GridSnapWidthXDenominator, Any( GetSnapGridWidthX().GetDenominator() ) );
    aUserData.emplace_back( sUNO_View_GridSnapWidthYNumerator, Any( GetSnapGridWidthY().GetNumerator() ) );
    aUserData.emplace_back( sUNO_View_GridSnapWidthYDenominator, Any( GetSnapGridWidthY().GetDenominator() ) );
    aUserData.emplace_back( sUNO_View_IsAngleSnapEnabled, Any( IsAngleSnapEnabled() ) );
    aUserData.emplace_back( sUNO_View_SnapAngle, Any( static_cast<sal_Int32>(GetSnapAngle()) ) );

    const sal_Int32 nOldLength = rValues.getLength();
    rValues.realloc( nOldLength + aUserData.size() );

    PropertyValue* pValue = &(rValues.getArray()[nOldLength]);

    for( const auto& rItem : aUserData )
    {
        pValue->Name = rItem.first;
        pValue->Value = rItem.second;
        ++pValue;
    }
}

static void createHelpLinesFromString( const OUString& rLines, SdrHelpLineList& rHelpLines )
{
    const sal_Unicode * pStr = rLines.getStr();
    SdrHelpLine aNewHelpLine;
    OUStringBuffer sBuffer;

    while( *pStr )
    {
        Point aPoint;

        switch( *pStr )
        {
        case 'P':
            aNewHelpLine.SetKind( SdrHelpLineKind::Point );
            break;
        case 'V':
            aNewHelpLine.SetKind( SdrHelpLineKind::Vertical );
            break;
        case 'H':
            aNewHelpLine.SetKind( SdrHelpLineKind::Horizontal );
            break;
        default:
            OSL_FAIL( "syntax error in snap lines settings string" );
            return;
        }

        pStr++;

        while( (*pStr >= '0' && *pStr <= '9') || (*pStr == '+') || (*pStr == '-') )
        {
            sBuffer.append( *pStr++ );
        }

        sal_Int32 nValue = o3tl::toInt32(sBuffer);
        sBuffer.setLength(0);

        if( aNewHelpLine.GetKind() == SdrHelpLineKind::Horizontal )
        {
            aPoint.setY( nValue );
        }
        else
        {
            aPoint.setX( nValue );

            if( aNewHelpLine.GetKind() == SdrHelpLineKind::Point )
            {
                if( *pStr++ != ',' )
                    return;

                while( (*pStr >= '0' && *pStr <= '9') || (*pStr == '+') || (*pStr == '-')  )
                {
                    sBuffer.append( *pStr++ );
                }

                aPoint.setY( o3tl::toInt32(sBuffer) );
                sBuffer.setLength(0);

            }
        }

        aNewHelpLine.SetPos( aPoint );
        rHelpLines.Insert( aNewHelpLine );
    }
}

void FrameView::ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& rSequence )
{
    const sal_Int32 nLength = rSequence.getLength();
    if (!nLength)
        return;

    SdDrawDocument* pDrawDocument = dynamic_cast<SdDrawDocument*>(&GetModel());
    const bool bImpress = pDrawDocument && pDrawDocument->GetDocumentType() == DocumentType::Impress;

    bool bBool = false;
    sal_Int32 nInt32 = 0;
    sal_Int16 nInt16 = 0;
    OUString aString;

    sal_Int32 aSnapGridWidthXNum = GetSnapGridWidthX().GetNumerator();
    sal_Int32 aSnapGridWidthXDom = GetSnapGridWidthX().GetDenominator();

    sal_Int32 aSnapGridWidthYNum = GetSnapGridWidthY().GetNumerator();
    sal_Int32 aSnapGridWidthYDom = GetSnapGridWidthY().GetDenominator();

    for (const css::beans::PropertyValue& rValue : rSequence)
    {
        if ( rValue.Name == sUNO_View_ViewId )
        {
        }
        else if ( rValue.Name == sUNO_View_SnapLinesDrawing )
        {
            if( rValue.Value >>= aString )
            {
                SdrHelpLineList aHelpLines;
                createHelpLinesFromString( aString, aHelpLines );
                SetStandardHelpLines( aHelpLines );
            }
        }
        else if ( rValue.Name == sUNO_View_SnapLinesNotes )
        {
            if( rValue.Value >>= aString )
            {
                SdrHelpLineList aHelpLines;
                createHelpLinesFromString( aString, aHelpLines );
                SetNotesHelpLines( aHelpLines );
            }
        }
        else if ( rValue.Name == sUNO_View_SnapLinesHandout )
        {
            if( rValue.Value >>= aString )
            {
                SdrHelpLineList aHelpLines;
                createHelpLinesFromString( aString, aHelpLines );
                SetHandoutHelpLines( aHelpLines );
            }
        }
        else if ( rValue.Name == sUNO_View_RulerIsVisible )
        {
            if( rValue.Value >>= bBool )
            {
                SetRuler( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_PageKind )
        {
            if( rValue.Value >>= nInt16 )
            {
                SdDrawDocument* pDoc = dynamic_cast<SdDrawDocument*>(&GetModel());
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetPageKind( static_cast<PageKind>(nInt16) );

                SetPageKindOnLoad( static_cast<PageKind>(nInt16) );
            }
        }
        else if ( rValue.Name == sUNO_View_SelectedPage )
        {
            if( rValue.Value >>= nInt16 )
            {
                SdDrawDocument* pDoc = dynamic_cast<SdDrawDocument*>(&GetModel());
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetSelectedPage( static_cast<sal_uInt16>(nInt16) );

                SetSelectedPageOnLoad( static_cast<sal_uInt16>(nInt16) );
            }
        }
        else if ( rValue.Name == sUNO_View_IsLayerMode )
        {
            if( rValue.Value >>= bBool )
            {
                SetLayerMode( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsDoubleClickTextEdit )
        {
            if( rValue.Value >>= bBool )
            {
                SetDoubleClickTextEdit( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsClickChangeRotation )
        {
            if( rValue.Value >>= bBool )
            {
                SetClickChangeRotation( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_SlidesPerRow )
        {
            if( rValue.Value >>= nInt16 )
            {
                SetSlidesPerRow( static_cast<sal_uInt16>(nInt16) );
            }
        }
        else if ( rValue.Name == sUNO_View_EditMode )
        {
            if( rValue.Value >>= nInt32 )
            {
                SdDrawDocument* pDoc = dynamic_cast<SdDrawDocument*>(&GetModel());
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetViewShEditMode( static_cast<EditMode>(nInt32) );
            }
        }
        // This one is kept for compatibility. Old value read from sUNO_View_EditModeStandard
        // is used. New value will be written into sUNO_View_EditMode.
        // Values from sUNO_View_EditModeNotes and sUNO_View_EditModeHangout will be ignored.
        else if ( rValue.Name == sUNO_View_EditModeStandard )
        {
            if( rValue.Value >>= nInt32 )
            {
                SdDrawDocument* pDoc = dynamic_cast<SdDrawDocument*>(&GetModel());
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetViewShEditMode( static_cast<EditMode>(nInt32) );
            }
        }
        else if ( rValue.Name == sUNO_View_VisibleAreaTop )
        {
            sal_Int32 nTop = 0;
            if( rValue.Value >>= nTop )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.AdjustBottom(nTop - aVisArea.Top() );
                aVisArea.SetTop( nTop );
                SetVisArea( aVisArea );
            }
        }
        else if ( rValue.Name == sUNO_View_VisibleAreaLeft )
        {
            sal_Int32 nLeft = 0;
            if( rValue.Value >>= nLeft )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.AdjustRight(nLeft - aVisArea.Left() );
                aVisArea.SetLeft( nLeft );
                SetVisArea( aVisArea );
            }
        }
        else if ( rValue.Name == sUNO_View_VisibleAreaWidth )
        {
            sal_Int32 nWidth = 0;
            if( rValue.Value >>= nWidth )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.SetRight( aVisArea.Left() + nWidth - 1 );
                SetVisArea( aVisArea );
            }
        }
        else if ( rValue.Name == sUNO_View_VisibleAreaHeight )
        {
            sal_Int32 nHeight = 0;
            if( rValue.Value >>= nHeight )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.SetBottom( nHeight + aVisArea.Top() - 1 );
                SetVisArea( aVisArea );
            }
        }

        else if ( rValue.Name == sUNO_View_GridIsVisible )
        {
            if( rValue.Value >>= bBool )
            {
                SetGridVisible( bBool );
            }
        }

        else if ( rValue.Name == sUNO_View_IsSnapToGrid )
        {
            if( rValue.Value >>= bBool )
            {
                SetGridSnap( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_GridIsFront )
        {
            if( rValue.Value >>= bBool )
            {
                SetGridFront( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsSnapToPageMargins )
        {
            if( rValue.Value >>= bBool )
            {
                SetBordSnap( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsSnapToSnapLines )
        {
            if( rValue.Value >>= bBool )
            {
                SetHlplSnap( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsSnapToObjectFrame )
        {
            if( rValue.Value >>= bBool )
            {
                SetOFrmSnap( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsSnapToObjectPoints )
        {
            if( rValue.Value >>= bBool )
            {
                SetOPntSnap( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsPlusHandlesAlwaysVisible )
        {
            if( rValue.Value >>= bBool )
            {
                SetPlusHandlesAlwaysVisible( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_IsFrameDragSingles )
        {
            if( rValue.Value >>= bBool )
            {
                SetFrameDragSingles( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_EliminatePolyPointLimitAngle )
        {
            if( rValue.Value >>= nInt32 )
            {
                SetEliminatePolyPointLimitAngle( Degree100(nInt32) );
            }
        }
        else if ( rValue.Name == sUNO_View_IsEliminatePolyPoints )
        {
            if( rValue.Value >>= bBool )
            {
                SetEliminatePolyPoints( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_ActiveLayer )
        {
            if( rValue.Value >>= aString )
            {
                SetActiveLayer( aString );
            }
        }
        else if ( rValue.Name == sUNO_View_NoAttribs )
        {
            if( rValue.Value >>= bBool )
            {
                SetNoAttribs( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_NoColors )
        {
            if( rValue.Value >>= bBool )
            {
                SetNoColors( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_GridCoarseWidth )
        {
            if( rValue.Value >>= nInt32 )
            {
                const Size aCoarse( nInt32, GetGridCoarse().Height() );
                SetGridCoarse( aCoarse );
            }
        }
        else if ( rValue.Name == sUNO_View_GridCoarseHeight )
        {
            if( rValue.Value >>= nInt32 )
            {
                const Size aCoarse( GetGridCoarse().Width(), nInt32 );
                SetGridCoarse( aCoarse );
            }
        }
        else if ( rValue.Name == sUNO_View_GridFineWidth )
        {
            if( rValue.Value >>= nInt32 )
            {
                const Size aCoarse( nInt32, GetGridFine().Height() );
                SetGridFine( aCoarse );
            }
        }
        else if ( rValue.Name == sUNO_View_GridFineHeight )
        {
            if( rValue.Value >>= nInt32 )
            {
                const Size aCoarse( GetGridFine().Width(), nInt32 );
                SetGridFine( aCoarse );
            }
        }
        else if ( rValue.Name == sUNO_View_IsAngleSnapEnabled )
        {
            if( rValue.Value >>= bBool )
            {
                SetAngleSnapEnabled( bBool );
            }
        }
        else if ( rValue.Name == sUNO_View_SnapAngle )
        {
            if( rValue.Value >>= nInt32 )
            {
                SetSnapAngle( Degree100(nInt32) );
            }
        }
        else if ( rValue.Name == sUNO_View_GridSnapWidthXNumerator )
        {
            rValue.Value >>= aSnapGridWidthXNum;
        }
        else if ( rValue.Name == sUNO_View_GridSnapWidthXDenominator )
        {
            rValue.Value >>= aSnapGridWidthXDom;
        }
        else if ( rValue.Name == sUNO_View_GridSnapWidthYNumerator )
        {
            rValue.Value >>= aSnapGridWidthYNum;
        }
        else if ( rValue.Name == sUNO_View_GridSnapWidthYDenominator )
        {
            rValue.Value >>= aSnapGridWidthYDom;
        }
        else if (!bImpress && rValue.Name == sUNO_View_VisibleLayers  )
        {
            SdrLayerIDSet aSdrLayerIDSets;
            aSdrLayerIDSets.PutValue( rValue.Value );
            SetVisibleLayers( aSdrLayerIDSets );
        }
        else if (!bImpress && rValue.Name == sUNO_View_PrintableLayers )
        {
            SdrLayerIDSet aSdrLayerIDSets;
            aSdrLayerIDSets.PutValue( rValue.Value );
            SetPrintableLayers( aSdrLayerIDSets );
        }
        else if (!bImpress && rValue.Name == sUNO_View_LockedLayers )
        {
            SdrLayerIDSet aSdrLayerIDSets;
            aSdrLayerIDSets.PutValue( rValue.Value );
            SetLockedLayers( aSdrLayerIDSets );
        }
    }

    SetViewShEditModeOnLoad(EditMode::Page);

    const Fraction aSnapGridWidthX( aSnapGridWidthXNum, aSnapGridWidthXDom );
    const Fraction aSnapGridWidthY( aSnapGridWidthYNum, aSnapGridWidthYDom );

    SetSnapGridWidth( aSnapGridWidthX, aSnapGridWidthY );
}

void FrameView::SetPreviousViewShellType (ViewShell::ShellType eType)
{
    mePreviousViewShellType = eType;
}

void FrameView::SetViewShellTypeOnLoad (ViewShell::ShellType eType)
{
    meViewShellTypeOnLoad = eType;
}

void FrameView::SetSelectedPage(sal_uInt16 nPage)
{
    mnSelectedPage = nPage;
}

void FrameView::SetIsNavigatorShowingAllShapes (const bool bIsNavigatorShowingAllShapes)
{
    mbIsNavigatorShowingAllShapes = bIsNavigatorShowingAllShapes;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
