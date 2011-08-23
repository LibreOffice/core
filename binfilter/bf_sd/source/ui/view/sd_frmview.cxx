/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SD_UNOKYWDS_HXX_
#include "unokywds.hxx"
#endif

#include <vector>

#include "frmview.hxx"
#include "drawdoc.hxx"
#include "optsitem.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "sdiocmpt.hxx"
#include "bf_sd/docshell.hxx"

namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::std;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

FrameView::FrameView(SdDrawDocument* pDrawDoc, FrameView* pFrameView )
: SdrView(pDrawDoc, (OutputDevice*) NULL)
, nRefCount(0)
, nPresViewShellId(SID_VIEWSHELL0)
, nSlotId(SID_OBJECT_SELECT)
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
        SdDrawDocShell* pDocShell = pDrawDoc->GetDocSh();

        if ( pDocShell )
        {
            /**********************************************************************
            * Das Dokument wurde geladen, ist eine FrameView vorhanden?
            **********************************************************************/
            ULONG nSdViewShellCount = 0;
            SdDrawDocument* pDoc = pDocShell->GetDoc();
        }
    }

        aVisibleLayers.SetAll();
        aPrintableLayers.SetAll();
        SetGridCoarse( Size( 1000, 1000 ) );
        SetSnapGrid( Size( 1000, 1000 ) );
        SetSnapGridWidth(Fraction(1000, 1), Fraction(1000, 1));
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
        bShowPreviewInMasterPageMode = TRUE;
        bShowPreviewInOutlineMode = TRUE;
        nTabCtrlPercent = 0.0;

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

FrameView::~FrameView()
{
}

SvStream& operator << (SvStream& rOut, const FrameView& rView)
{
    sal_uInt32 nULTemp;

    rOut << (SdrView&) rView;

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
    nULTemp = (sal_uInt32) rView.ePageKind;            rOut << nULTemp;
    rOut << rView.nSelectedPage;
    nULTemp = (sal_uInt32) rView.eStandardEditMode;    rOut << nULTemp;
    rOut << rView.bLayerMode;
    rOut << rView.bQuickEdit;
    rOut << rView.bDragWithCopy;
    rOut << (UINT16)rView.nSlidesPerRow;

    rOut << rView.bBigHandles;
    rOut << rView.bDoubleClickTextEdit;
    rOut << rView.bClickChangeRotation;

    nULTemp = (sal_uInt32) rView.eNotesEditMode;       rOut << nULTemp;
    nULTemp = (sal_uInt32) rView.eHandoutEditMode;     rOut << nULTemp;

    rOut << static_cast<sal_uInt32>(rView.nDrawMode);
    rOut << static_cast<sal_uInt32>(rView.nPreviewDrawMode);

    rOut << rView.bShowPreviewInPageMode;
    rOut << rView.bShowPreviewInMasterPageMode;
    rOut << rView.bShowPreviewInOutlineMode;

    return rOut;
}

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
        sal_uInt32 nULTemp;
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
        sal_uInt32 nULTemp;
        rIn >> nULTemp; rView.eNotesEditMode   = (EditMode) nULTemp;
        rIn >> nULTemp; rView.eHandoutEditMode = (EditMode) nULTemp;
    }

    if (aIO.GetVersion() >= 9)
    {
        sal_uInt32 nTemp;
        rIn >> nTemp;
        rView.nDrawMode = nTemp;
        rIn >> nTemp;
        rView.nPreviewDrawMode = nTemp;
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

    if (rView.GetModel())
    {
        USHORT nMaxPages = ((SdDrawDocument* )rView.GetModel())->GetSdPageCount(rView.ePageKind);

        if (rView.nSelectedPage >= nMaxPages)
            rView.nSelectedPage = nMaxPages - 1;
    }

    return rIn;
}

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
//		SetSnapGrid( Size( pOptions->GetFldSnapX(), pOptions->GetFldSnapY() ) );
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

    aUserData.addValue( sUNO_View_IsPlusHandlesAlwaysVisible, makeAny( (sal_Bool)IsPlusHandlesAlwaysVisible() ) );
    aUserData.addValue( sUNO_View_IsFrameDragSingles, makeAny( (sal_Bool)IsFrameDragSingles() ) );

    aUserData.addValue( sUNO_View_EliminatePolyPointLimitAngle, makeAny( (sal_Int32)GetEliminatePolyPointLimitAngle() ) );
    aUserData.addValue( sUNO_View_IsEliminatePolyPoints, makeAny( (sal_Bool)IsEliminatePolyPoints() ) );

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

    aUserData.addValue( sUNO_View_RulerIsVisible, makeAny( (sal_Bool)HasRuler() ) );
    aUserData.addValue( sUNO_View_PageKind, makeAny( (sal_Int16)GetPageKind() ) );
    aUserData.addValue( sUNO_View_SelectedPage, makeAny( (sal_Int16)GetSelectedPage() ) );
    aUserData.addValue( sUNO_View_IsLayerMode, makeAny( (sal_Bool)IsLayerMode() ) );

    aUserData.addValue( sUNO_View_IsBigHandles, makeAny( (sal_Bool)IsBigHandles() ) );
    aUserData.addValue( sUNO_View_IsDoubleClickTextEdit,  makeAny( (sal_Bool)IsDoubleClickTextEdit() ) );
    aUserData.addValue( sUNO_View_IsClickChangeRotation, makeAny( (sal_Bool)IsClickChangeRotation() ) );

    aUserData.addValue( sUNO_View_SlidesPerRow, makeAny( (sal_Int16)GetSlidesPerRow() ) );
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

 static void createHelpLinesFromString( const ::rtl::OUString& rLines, SdrHelpLineList& rHelpLines )
 {
     const sal_Unicode * pStr = rLines.getStr();
     SdrHelpLine aNewHelpLine;
     ::rtl::OUStringBuffer sBuffer;
 
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

/*?*/ void FrameView::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
/*?*/ {
/*?*/ 		const sal_Int32 nLength = rSequence.getLength();
     if (nLength)
     {
         sal_Bool bBool;
         sal_Int32 nInt32;
         sal_Int16 nInt16;
         ::rtl::OUString aString;
 
         sal_Int32 aSnapGridWidthXNum = GetSnapGridWidthX().GetNumerator();
         sal_Int32 aSnapGridWidthXDom = GetSnapGridWidthX().GetDenominator();
 
         sal_Int32 aSnapGridWidthYNum = GetSnapGridWidthY().GetNumerator();
         sal_Int32 aSnapGridWidthYDom = GetSnapGridWidthY().GetDenominator();
 
         const ::com::sun::star::beans::PropertyValue *pValue = rSequence.getConstArray();
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
             else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsLayerMode ) ) )
             {
                 if( pValue->Value >>= bBool )
                 {
                     SetLayerMode( bBool );
                 }
             }
             else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsBigHandles ) ) )
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
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SlidesPerRow ) ) )
             {
                 if( pValue->Value >>= nInt16 )
                 {
                     SetSlidesPerRow( (USHORT)nInt16 );
                 }
             }
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
             else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsPlusHandlesAlwaysVisible ) ) )
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
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EliminatePolyPointLimitAngle ) ) )
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
} //namespace binfilter
