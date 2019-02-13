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
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <rtl/ustrbuf.hxx>
#include <unokywds.hxx>
#include <vcl/settings.hxx>

#include <vector>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <optsitem.hxx>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <sdmod.hxx>
#include <pres.hxx>
#include <strings.hrc>
#include <sdiocmpt.hxx>
#include <framework/FrameworkHelper.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::std;

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
        mnSlidesPerRow = 4;

        {
            bool bUseContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
            mnDrawMode = bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR;
        }
        mbIsNavigatorShowingAllShapes = false;
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
    GetModel()->SetPickThroughTransparentTextFrames( pOptions->IsPickThrough() );

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
EditMode FrameView::GetViewShEditMode()
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

#define addValue( n, v ) push_back( std::pair< OUString, Any >( OUString( n ), v ) )
void FrameView::WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& rValues )
{
    std::vector< std::pair< OUString, Any > > aUserData;

    aUserData.addValue( sUNO_View_GridIsVisible, makeAny( IsGridVisible() ) );
    aUserData.addValue( sUNO_View_GridIsFront, makeAny( IsGridFront() ) );
    aUserData.addValue( sUNO_View_IsSnapToGrid, makeAny( IsGridSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToPageMargins, makeAny( IsBordSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToSnapLines, makeAny( IsHlplSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToObjectFrame, makeAny( IsOFrmSnap() ) );
    aUserData.addValue( sUNO_View_IsSnapToObjectPoints, makeAny( IsOPntSnap() ) );

    aUserData.addValue( sUNO_View_IsPlusHandlesAlwaysVisible, makeAny( IsPlusHandlesAlwaysVisible() ) );
    aUserData.addValue( sUNO_View_IsFrameDragSingles, makeAny( IsFrameDragSingles() ) );

    aUserData.addValue( sUNO_View_EliminatePolyPointLimitAngle, makeAny( static_cast<sal_Int32>(GetEliminatePolyPointLimitAngle()) ) );
    aUserData.addValue( sUNO_View_IsEliminatePolyPoints, makeAny( IsEliminatePolyPoints() ) );

    if ( officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::get() )
    {
        SdrLayerAdmin& rLayerAdmin = getSdrModelFromSdrView().GetLayerAdmin();
        Any aAny;
        rLayerAdmin.QueryValue(GetVisibleLayers(), aAny);
        aUserData.addValue( sUNO_View_VisibleLayers, aAny );

        rLayerAdmin.QueryValue(GetPrintableLayers(), aAny);
        aUserData.addValue( sUNO_View_PrintableLayers, aAny );

        rLayerAdmin.QueryValue(GetLockedLayers(), aAny);
        aUserData.addValue( sUNO_View_LockedLayers, aAny );
    }

    aUserData.addValue( sUNO_View_NoAttribs, makeAny( IsNoAttribs() ) );
    aUserData.addValue( sUNO_View_NoColors, makeAny( IsNoColors() ) );

    if( GetStandardHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesDrawing, makeAny( createHelpLinesString( GetStandardHelpLines() ) ) );

    if( GetNotesHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesNotes, makeAny( createHelpLinesString( GetNotesHelpLines() ) ) );

    if( GetHandoutHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesHandout, makeAny( createHelpLinesString( GetHandoutHelpLines() ) ) );

    aUserData.addValue( sUNO_View_RulerIsVisible, makeAny( HasRuler() ) );
    aUserData.addValue( sUNO_View_PageKind, makeAny( static_cast<sal_Int16>(GetPageKind()) ) );
    aUserData.addValue( sUNO_View_SelectedPage, makeAny( static_cast<sal_Int16>(GetSelectedPage()) ) );
    aUserData.addValue( sUNO_View_IsLayerMode, makeAny( IsLayerMode() ) );

    aUserData.addValue( sUNO_View_IsDoubleClickTextEdit,  makeAny( IsDoubleClickTextEdit() ) );
    aUserData.addValue( sUNO_View_IsClickChangeRotation, makeAny( IsClickChangeRotation() ) );

    aUserData.addValue( sUNO_View_SlidesPerRow, makeAny( static_cast<sal_Int16>(GetSlidesPerRow()) ) );
    aUserData.addValue( sUNO_View_EditMode, makeAny( static_cast<sal_Int32>(GetViewShEditMode()) ) );
    // aUserData.addValue( sUNO_View_EditModeStandard, makeAny( (sal_Int32)GetViewShEditMode( PageKind::Standard ) ) );
    // aUserData.addValue( sUNO_View_EditModeNotes, makeAny( (sal_Int32)GetViewShEditMode( PageKind::Notes ) ) );
    // aUserData.addValue( sUNO_View_EditModeHandout, makeAny( (sal_Int32)GetViewShEditMode( PageKind::Handout ) ) );

    {
        const ::tools::Rectangle aVisArea = GetVisArea();

        aUserData.addValue( sUNO_View_VisibleAreaTop, makeAny( static_cast<sal_Int32>(aVisArea.Top()) ) );
        aUserData.addValue( sUNO_View_VisibleAreaLeft, makeAny( static_cast<sal_Int32>(aVisArea.Left()) ) );
        aUserData.addValue( sUNO_View_VisibleAreaWidth, makeAny( static_cast<sal_Int32>(aVisArea.GetWidth()) ) );
        aUserData.addValue( sUNO_View_VisibleAreaHeight, makeAny( static_cast<sal_Int32>(aVisArea.GetHeight()) ) );
    }

    aUserData.addValue( sUNO_View_GridCoarseWidth, makeAny( static_cast<sal_Int32>(GetGridCoarse().Width()) ) );
    aUserData.addValue( sUNO_View_GridCoarseHeight, makeAny( static_cast<sal_Int32>(GetGridCoarse().Height()) ) );
    aUserData.addValue( sUNO_View_GridFineWidth, makeAny( static_cast<sal_Int32>(GetGridFine().Width()) ) );
    aUserData.addValue( sUNO_View_GridFineHeight, makeAny( static_cast<sal_Int32>(GetGridFine().Height()) ) );
    aUserData.addValue( sUNO_View_GridSnapWidthXNumerator, makeAny( GetSnapGridWidthX().GetNumerator() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthXDenominator, makeAny( GetSnapGridWidthX().GetDenominator() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthYNumerator, makeAny( GetSnapGridWidthY().GetNumerator() ) );
    aUserData.addValue( sUNO_View_GridSnapWidthYDenominator, makeAny( GetSnapGridWidthY().GetDenominator() ) );
    aUserData.addValue( sUNO_View_IsAngleSnapEnabled, makeAny( IsAngleSnapEnabled() ) );
    aUserData.addValue( sUNO_View_SnapAngle, makeAny( static_cast<sal_Int32>(GetSnapAngle()) ) );

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
#undef addValue

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

        sal_Int32 nValue = sBuffer.makeStringAndClear().toInt32();

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

                aPoint.setY( sBuffer.makeStringAndClear().toInt32() );

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

    SdDrawDocument* pDrawDocument = dynamic_cast<SdDrawDocument*>(GetModel());
    const bool bImpress = pDrawDocument && pDrawDocument->GetDocumentType() == DocumentType::Impress;

    bool bBool = false;
    sal_Int32 nInt32 = 0;
    sal_Int16 nInt16 = 0;
    OUString aString;

    sal_Int32 aSnapGridWidthXNum = GetSnapGridWidthX().GetNumerator();
    sal_Int32 aSnapGridWidthXDom = GetSnapGridWidthX().GetDenominator();

    sal_Int32 aSnapGridWidthYNum = GetSnapGridWidthY().GetNumerator();
    sal_Int32 aSnapGridWidthYDom = GetSnapGridWidthY().GetDenominator();

    const css::beans::PropertyValue *pValue = rSequence.getConstArray();
    for (sal_Int32 i = 0 ; i < nLength; i++, pValue++ )
    {
        if ( pValue->Name == sUNO_View_ViewId )
        {
        }
        else if ( pValue->Name == sUNO_View_SnapLinesDrawing )
        {
            if( pValue->Value >>= aString )
            {
                SdrHelpLineList aHelpLines;
                createHelpLinesFromString( aString, aHelpLines );
                SetStandardHelpLines( aHelpLines );
            }
        }
        else if ( pValue->Name == sUNO_View_SnapLinesNotes )
        {
            if( pValue->Value >>= aString )
            {
                SdrHelpLineList aHelpLines;
                createHelpLinesFromString( aString, aHelpLines );
                SetNotesHelpLines( aHelpLines );
            }
        }
        else if ( pValue->Name == sUNO_View_SnapLinesHandout )
        {
            if( pValue->Value >>= aString )
            {
                SdrHelpLineList aHelpLines;
                createHelpLinesFromString( aString, aHelpLines );
                SetHandoutHelpLines( aHelpLines );
            }
        }
        else if ( pValue->Name == sUNO_View_RulerIsVisible )
        {
            if( pValue->Value >>= bBool )
            {
                SetRuler( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_PageKind )
        {
            if( pValue->Value >>= nInt16 )
            {
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetPageKind( static_cast<PageKind>(nInt16) );

                SetPageKindOnLoad( static_cast<PageKind>(nInt16) );
            }
        }
        else if ( pValue->Name == sUNO_View_SelectedPage )
        {
            if( pValue->Value >>= nInt16 )
            {
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetSelectedPage( static_cast<sal_uInt16>(nInt16) );

                SetSelectedPageOnLoad( static_cast<sal_uInt16>(nInt16) );
            }
        }
        else if ( pValue->Name == sUNO_View_IsLayerMode )
        {
            if( pValue->Value >>= bBool )
            {
                SetLayerMode( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsDoubleClickTextEdit )
        {
            if( pValue->Value >>= bBool )
            {
                SetDoubleClickTextEdit( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsClickChangeRotation )
        {
            if( pValue->Value >>= bBool )
            {
                SetClickChangeRotation( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_SlidesPerRow )
        {
            if( pValue->Value >>= nInt16 )
            {
                SetSlidesPerRow( static_cast<sal_uInt16>(nInt16) );
            }
        }
        else if ( pValue->Name == sUNO_View_EditMode )
        {
            if( pValue->Value >>= nInt32 )
            {
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetViewShEditMode( static_cast<EditMode>(nInt32) );
            }
        }
        // This one is kept for compatibility. Old value read from sUNO_View_EditModeStandard
        // is used. New value will be written into sUNO_View_EditMode.
        // Values from sUNO_View_EditModeNotes and sUNO_View_EditModeHangout will be ignored.
        else if ( pValue->Name == sUNO_View_EditModeStandard )
        {
            if( pValue->Value >>= nInt32 )
            {
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                    SetViewShEditMode( static_cast<EditMode>(nInt32) );
            }
        }
        else if ( pValue->Name == sUNO_View_VisibleAreaTop )
        {
            sal_Int32 nTop = 0;
            if( pValue->Value >>= nTop )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.AdjustBottom(nTop - aVisArea.Top() );
                aVisArea.SetTop( nTop );
                SetVisArea( aVisArea );
            }
        }
        else if ( pValue->Name == sUNO_View_VisibleAreaLeft )
        {
            sal_Int32 nLeft = 0;
            if( pValue->Value >>= nLeft )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.AdjustRight(nLeft - aVisArea.Left() );
                aVisArea.SetLeft( nLeft );
                SetVisArea( aVisArea );
            }
        }
        else if ( pValue->Name == sUNO_View_VisibleAreaWidth )
        {
            sal_Int32 nWidth = 0;
            if( pValue->Value >>= nWidth )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.SetRight( aVisArea.Left() + nWidth - 1 );
                SetVisArea( aVisArea );
            }
        }
        else if ( pValue->Name == sUNO_View_VisibleAreaHeight )
        {
            sal_Int32 nHeight = 0;
            if( pValue->Value >>= nHeight )
            {
                ::tools::Rectangle aVisArea( GetVisArea() );
                aVisArea.SetBottom( nHeight + aVisArea.Top() - 1 );
                SetVisArea( aVisArea );
            }
        }

        else if ( pValue->Name == sUNO_View_GridIsVisible )
        {
            if( pValue->Value >>= bBool )
            {
                SetGridVisible( bBool );
            }
        }

        else if ( pValue->Name == sUNO_View_IsSnapToGrid )
        {
            if( pValue->Value >>= bBool )
            {
                SetGridSnap( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_GridIsFront )
        {
            if( pValue->Value >>= bBool )
            {
                SetGridFront( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsSnapToPageMargins )
        {
            if( pValue->Value >>= bBool )
            {
                SetBordSnap( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsSnapToSnapLines )
        {
            if( pValue->Value >>= bBool )
            {
                SetHlplSnap( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsSnapToObjectFrame )
        {
            if( pValue->Value >>= bBool )
            {
                SetOFrmSnap( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsSnapToObjectPoints )
        {
            if( pValue->Value >>= bBool )
            {
                SetOPntSnap( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsPlusHandlesAlwaysVisible )
        {
            if( pValue->Value >>= bBool )
            {
                SetPlusHandlesAlwaysVisible( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_IsFrameDragSingles )
        {
            if( pValue->Value >>= bBool )
            {
                SetFrameDragSingles( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_EliminatePolyPointLimitAngle )
        {
            if( pValue->Value >>= nInt32 )
            {
                SetEliminatePolyPointLimitAngle( nInt32 );
            }
        }
        else if ( pValue->Name == sUNO_View_IsEliminatePolyPoints )
        {
            if( pValue->Value >>= bBool )
            {
                SetEliminatePolyPoints( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_ActiveLayer )
        {
            if( pValue->Value >>= aString )
            {
                SetActiveLayer( aString );
            }
        }
        else if ( pValue->Name == sUNO_View_NoAttribs )
        {
            if( pValue->Value >>= bBool )
            {
                SetNoAttribs( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_NoColors )
        {
            if( pValue->Value >>= bBool )
            {
                SetNoColors( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_GridCoarseWidth )
        {
            if( pValue->Value >>= nInt32 )
            {
                const Size aCoarse( nInt32, GetGridCoarse().Height() );
                SetGridCoarse( aCoarse );
            }
        }
        else if ( pValue->Name == sUNO_View_GridCoarseHeight )
        {
            if( pValue->Value >>= nInt32 )
            {
                const Size aCoarse( GetGridCoarse().Width(), nInt32 );
                SetGridCoarse( aCoarse );
            }
        }
        else if ( pValue->Name == sUNO_View_GridFineWidth )
        {
            if( pValue->Value >>= nInt32 )
            {
                const Size aCoarse( nInt32, GetGridFine().Height() );
                SetGridFine( aCoarse );
            }
        }
        else if ( pValue->Name == sUNO_View_GridFineHeight )
        {
            if( pValue->Value >>= nInt32 )
            {
                const Size aCoarse( GetGridFine().Width(), nInt32 );
                SetGridFine( aCoarse );
            }
        }
        else if ( pValue->Name == sUNO_View_IsAngleSnapEnabled )
        {
            if( pValue->Value >>= bBool )
            {
                SetAngleSnapEnabled( bBool );
            }
        }
        else if ( pValue->Name == sUNO_View_SnapAngle )
        {
            if( pValue->Value >>= nInt32 )
            {
                SetSnapAngle( nInt32 );
            }
        }
        else if ( pValue->Name == sUNO_View_GridSnapWidthXNumerator )
        {
            pValue->Value >>= aSnapGridWidthXNum;
        }
        else if ( pValue->Name == sUNO_View_GridSnapWidthXDenominator )
        {
            pValue->Value >>= aSnapGridWidthXDom;
        }
        else if ( pValue->Name == sUNO_View_GridSnapWidthYNumerator )
        {
            pValue->Value >>= aSnapGridWidthYNum;
        }
        else if ( pValue->Name == sUNO_View_GridSnapWidthYDenominator )
        {
            pValue->Value >>= aSnapGridWidthYDom;
        }
        else if (!bImpress && pValue->Name == sUNO_View_VisibleLayers  )
        {
            SdrLayerIDSet aSdrLayerIDSets;
            aSdrLayerIDSets.PutValue( pValue->Value );
            SetVisibleLayers( aSdrLayerIDSets );
        }
        else if (!bImpress && pValue->Name == sUNO_View_PrintableLayers )
        {
            SdrLayerIDSet aSdrLayerIDSets;
            aSdrLayerIDSets.PutValue( pValue->Value );
            SetPrintableLayers( aSdrLayerIDSets );
        }
        else if (!bImpress && pValue->Name == sUNO_View_LockedLayers )
        {
            SdrLayerIDSet aSdrLayerIDSets;
            aSdrLayerIDSets.PutValue( pValue->Value );
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
