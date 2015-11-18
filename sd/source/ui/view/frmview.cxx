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

#include "FrameView.hxx"

#include <svx/svxids.hrc>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <rtl/ustrbuf.hxx>
#include "unokywds.hxx"
#include <vcl/settings.hxx>

#include <vector>
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "optsitem.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "sdmod.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "glob.hrc"
#include "sdiocmpt.hxx"
#include "framework/FrameworkHelper.hxx"
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::std;

namespace sd {

FrameView::FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView /* = NULK */)
  : SdrView(pDrawDoc, nullptr),
    mnRefCount(0),
    mnPresViewShellId(SID_VIEWSHELL0),
    mnSlotId(SID_OBJECT_SELECT),
    mbIsNavigatorShowingAllShapes(false)
{
    EndListening(*pDrawDoc);

    EnableExtendedKeyInputDispatcher(false);
    EnableExtendedMouseEventDispatcher(false);
    EnableExtendedCommandEventDispatcher(false);

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
        meEditMode = pFrameView->GetViewShEditMode();
        // meStandardEditMode = pFrameView->GetViewShEditMode(PK_STANDARD);
        // meNotesEditMode = pFrameView->GetViewShEditMode(PK_NOTES);
        // meHandoutEditMode = pFrameView->GetViewShEditMode(PK_HANDOUT);
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
        maVisibleLayers.SetAll();
        maPrintableLayers.SetAll();
        SetGridCoarse( Size( 1000, 1000 ) );
        SetSnapGridWidth(Fraction(1000, 1), Fraction(1000, 1));
        SetActiveLayer( SD_RESSTR(STR_LAYER_LAYOUT) );
        mbNoColors = true;
        mbNoAttribs = false;
        maVisArea = Rectangle( Point(), Size(0, 0) );
        mePageKind = PK_STANDARD;
        mePageKindOnLoad = PK_STANDARD;
        mnSelectedPage = 0;
        mnSelectedPageOnLoad = 0;
        meEditMode = EM_PAGE;
        // meStandardEditMode = EM_PAGE;
        // meNotesEditMode = EM_PAGE;
        // meHandoutEditMode = EM_MASTERPAGE;
        SetViewShEditModeOnLoad(EM_PAGE);
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
}

/**
 * Set EditMode (Page or MasterPage) of working mode
 */
void FrameView::SetViewShEditMode(EditMode eMode)
{
    meEditMode = eMode;
}

/**
 * Return EditMode (Page or MasterPage) of working mode
 */
EditMode FrameView::GetViewShEditMode()
{
    return meEditMode;
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
            case SDRHELPLINE_POINT:
                aLines.append( 'P' );
                aLines.append( (sal_Int32)rPos.X() );
                aLines.append( ',' );
                aLines.append( (sal_Int32)rPos.Y() );
                break;
            case SDRHELPLINE_VERTICAL:
                aLines.append( 'V' );
                aLines.append( (sal_Int32)rPos.X() );
                break;
            case SDRHELPLINE_HORIZONTAL:
                aLines.append( 'H' );
                aLines.append( (sal_Int32)rPos.Y() );
                break;
            default:
                OSL_FAIL( "Unsupported helpline Kind!" );
        }
    }

    return aLines.makeStringAndClear();
}

#define addValue( n, v ) push_back( std::pair< OUString, Any >( OUString( n ), v ) )
void FrameView::WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& rValues, bool )
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

    aUserData.addValue( sUNO_View_EliminatePolyPointLimitAngle, makeAny( (sal_Int32)GetEliminatePolyPointLimitAngle() ) );
    aUserData.addValue( sUNO_View_IsEliminatePolyPoints, makeAny( IsEliminatePolyPoints() ) );

    Any aAny;
    GetVisibleLayers().QueryValue( aAny );
    aUserData.addValue( sUNO_View_VisibleLayers, aAny );

    GetPrintableLayers().QueryValue( aAny );
    aUserData.addValue( sUNO_View_PrintableLayers, aAny );

    GetLockedLayers().QueryValue( aAny );
    aUserData.addValue( sUNO_View_LockedLayers, aAny );

    aUserData.addValue( sUNO_View_NoAttribs, makeAny( IsNoAttribs() ) );
    aUserData.addValue( sUNO_View_NoColors, makeAny( IsNoColors() ) );

    if( GetStandardHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesDrawing, makeAny( createHelpLinesString( GetStandardHelpLines() ) ) );

    if( GetNotesHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesNotes, makeAny( createHelpLinesString( GetNotesHelpLines() ) ) );

    if( GetHandoutHelpLines().GetCount() )
        aUserData.addValue( sUNO_View_SnapLinesHandout, makeAny( createHelpLinesString( GetHandoutHelpLines() ) ) );

    aUserData.addValue( sUNO_View_RulerIsVisible, makeAny( HasRuler() ) );
    aUserData.addValue( sUNO_View_PageKind, makeAny( (sal_Int16)GetPageKind() ) );
    aUserData.addValue( sUNO_View_SelectedPage, makeAny( (sal_Int16)GetSelectedPage() ) );
    aUserData.addValue( sUNO_View_IsLayerMode, makeAny( IsLayerMode() ) );

    aUserData.addValue( sUNO_View_IsDoubleClickTextEdit,  makeAny( IsDoubleClickTextEdit() ) );
    aUserData.addValue( sUNO_View_IsClickChangeRotation, makeAny( IsClickChangeRotation() ) );

    aUserData.addValue( sUNO_View_SlidesPerRow, makeAny( (sal_Int16)GetSlidesPerRow() ) );
    aUserData.addValue( sUNO_View_EditMode, makeAny( (sal_Int32)GetViewShEditMode() ) );
    // aUserData.addValue( sUNO_View_EditModeStandard, makeAny( (sal_Int32)GetViewShEditMode( PK_STANDARD ) ) );
    // aUserData.addValue( sUNO_View_EditModeNotes, makeAny( (sal_Int32)GetViewShEditMode( PK_NOTES ) ) );
    // aUserData.addValue( sUNO_View_EditModeHandout, makeAny( (sal_Int32)GetViewShEditMode( PK_HANDOUT ) ) );

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
    aUserData.addValue( sUNO_View_IsAngleSnapEnabled, makeAny( IsAngleSnapEnabled() ) );
    aUserData.addValue( sUNO_View_SnapAngle, makeAny( (sal_Int32)GetSnapAngle() ) );

    const sal_Int32 nOldLength = rValues.getLength();
    rValues.realloc( nOldLength + aUserData.size() );

    PropertyValue* pValue = &(rValues.getArray()[nOldLength]);

    std::vector< std::pair< OUString, Any > >::iterator aIter( aUserData.begin() );
    for( ; aIter != aUserData.end(); ++aIter, ++pValue )
    {
        pValue->Name = (*aIter).first;
        pValue->Value = (*aIter).second;
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
            OSL_FAIL( "syntax error in snap lines settings string" );
            return;
        }

        pStr++;

        while( (*pStr >= '0' && *pStr <= '9') || (*pStr == '+') || (*pStr == '-') )
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

                while( (*pStr >= '0' && *pStr <= '9') || (*pStr == '+') || (*pStr == '-')  )
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

void FrameView::ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& rSequence, bool )
{
    const sal_Int32 nLength = rSequence.getLength();
    if (nLength)
    {
        SdDrawDocument* pDrawDocument = dynamic_cast<SdDrawDocument*>(GetModel());
        const bool bImpress = pDrawDocument && pDrawDocument->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;

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
                        SetPageKind( (PageKind)nInt16 );

                    SetPageKindOnLoad( (PageKind)nInt16 );
                }
            }
            else if ( pValue->Name == sUNO_View_SelectedPage )
            {
                if( pValue->Value >>= nInt16 )
                {
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetSelectedPage( (sal_uInt16)nInt16 );

                    SetSelectedPageOnLoad( (sal_uInt16)nInt16 );
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
                    SetSlidesPerRow( (sal_uInt16)nInt16 );
                }
            }
            else if ( pValue->Name == sUNO_View_EditMode )
            {
                if( pValue->Value >>= nInt32 )
                {
                    SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( GetModel() );
                    if( pDoc && pDoc->GetDocSh() && ( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocSh()->GetCreateMode() ) )
                        SetViewShEditMode( (EditMode)nInt32 );
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
                        SetViewShEditMode( (EditMode)nInt32 );
                }
            }
            else if ( pValue->Name == sUNO_View_VisibleAreaTop )
            {
                sal_Int32 nTop = 0;
                if( pValue->Value >>= nTop )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.Bottom() += nTop - aVisArea.Top();
                    aVisArea.Top() = nTop;
                    SetVisArea( aVisArea );
                }
            }
            else if ( pValue->Name == sUNO_View_VisibleAreaLeft )
            {
                sal_Int32 nLeft = 0;
                if( pValue->Value >>= nLeft )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.Right() += nLeft - aVisArea.Left();
                    aVisArea.Left() = nLeft;
                    SetVisArea( aVisArea );
                }
            }
            else if ( pValue->Name == sUNO_View_VisibleAreaWidth )
            {
                sal_Int32 nWidth = 0;
                if( pValue->Value >>= nWidth )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.Right() = aVisArea.Left() + nWidth - 1;
                    SetVisArea( aVisArea );
                }
            }
            else if ( pValue->Name == sUNO_View_VisibleAreaHeight )
            {
                sal_Int32 nHeight = 0;
                if( pValue->Value >>= nHeight )
                {
                    Rectangle aVisArea( GetVisArea() );
                    aVisArea.Bottom() = nHeight + aVisArea.Top() - 1;
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
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetVisibleLayers( aSetOfBytes );
            }
            else if (!bImpress && pValue->Name == sUNO_View_PrintableLayers )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetPrintableLayers( aSetOfBytes );
            }
            else if (!bImpress && pValue->Name == sUNO_View_LockedLayers )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                SetLockedLayers( aSetOfBytes );
            }
        }

        SetViewShEditModeOnLoad(EM_PAGE);

        const Fraction aSnapGridWidthX( aSnapGridWidthXNum, aSnapGridWidthXDom );
        const Fraction aSnapGridWidthY( aSnapGridWidthYNum, aSnapGridWidthYDom );

        SetSnapGridWidth( aSnapGridWidthX, aSnapGridWidthY );
    }
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
