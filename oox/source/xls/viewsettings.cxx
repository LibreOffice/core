/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewsettings.cxx,v $
 * $Revision: 1.5.20.6 $
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

#include "oox/xls/viewsettings.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/unitconverter.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetbuffer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::table::CellAddress;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_Int32 OOX_BOOKVIEW_TABBARRATIO_DEF    = 600;      /// Default tabbar ratio.
const sal_Int32 OOX_SHEETVIEW_NORMALZOOM_DEF    = 100;      /// Default zoom for normal view.
const sal_Int32 OOX_SHEETVIEW_SHEETLAYZOOM_DEF  = 60;       /// Default zoom for pagebreak preview.
const sal_Int32 OOX_SHEETVIEW_PAGELAYZOOM_DEF   = 100;      /// Default zoom for page layout view.

const sal_uInt8 OOBIN_PANE_FROZEN               = 0x01;
const sal_uInt8 OOBIN_PANE_FROZENNOSPLIT        = 0x02;

const sal_uInt16 OOBIN_SHEETVIEW_WINPROTECTED   = 0x0001;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWFORMULAS   = 0x0002;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWGRID       = 0x0004;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWHEADINGS   = 0x0008;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWZEROS      = 0x0010;
const sal_uInt16 OOBIN_SHEETVIEW_RIGHTTOLEFT    = 0x0020;
const sal_uInt16 OOBIN_SHEETVIEW_SELECTED       = 0x0040;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWRULER      = 0x0080;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWOUTLINE    = 0x0100;
const sal_uInt16 OOBIN_SHEETVIEW_DEFGRIDCOLOR   = 0x0200;
const sal_uInt16 OOBIN_SHEETVIEW_SHOWWHITESPACE = 0x0400;

const sal_uInt16 OOBIN_CHARTSHEETVIEW_SELECTED  = 0x0001;
const sal_uInt16 OOBIN_CHARTSHEETVIEW_ZOOMTOFIT = 0x0002;

const sal_uInt8 OOBIN_WBVIEW_HIDDEN             = 0x01;
const sal_uInt8 OOBIN_WBVIEW_MINIMIZED          = 0x02;
const sal_uInt8 OOBIN_WBVIEW_SHOWHORSCROLL      = 0x08;
const sal_uInt8 OOBIN_WBVIEW_SHOWVERSCROLL      = 0x10;
const sal_uInt8 OOBIN_WBVIEW_SHOWTABBAR         = 0x20;
const sal_uInt8 OOBIN_WBVIEW_AUTOFILTERGROUP    = 0x40;

const sal_uInt8 BIFF_PANE_BOTTOMRIGHT           = 0;        /// Bottom-right pane.
const sal_uInt8 BIFF_PANE_TOPRIGHT              = 1;        /// Right, or top-right pane.
const sal_uInt8 BIFF_PANE_BOTTOMLEFT            = 2;        /// Bottom, or bottom-left pane.
const sal_uInt8 BIFF_PANE_TOPLEFT               = 3;        /// Single, top, left, or top-left pane.

const sal_uInt16 BIFF_WINDOW1_HIDDEN            = 0x0001;
const sal_uInt16 BIFF_WINDOW1_MINIMIZED         = 0x0002;
const sal_uInt16 BIFF_WINDOW1_SHOWHORSCROLL     = 0x0008;
const sal_uInt16 BIFF_WINDOW1_SHOWVERSCROLL     = 0x0010;
const sal_uInt16 BIFF_WINDOW1_SHOWTABBAR        = 0x0020;

const sal_uInt16 BIFF_WINDOW2_SHOWFORMULAS      = 0x0001;
const sal_uInt16 BIFF_WINDOW2_SHOWGRID          = 0x0002;
const sal_uInt16 BIFF_WINDOW2_SHOWHEADINGS      = 0x0004;
const sal_uInt16 BIFF_WINDOW2_FROZEN            = 0x0008;
const sal_uInt16 BIFF_WINDOW2_SHOWZEROS         = 0x0010;
const sal_uInt16 BIFF_WINDOW2_DEFGRIDCOLOR      = 0x0020;
const sal_uInt16 BIFF_WINDOW2_RIGHTTOLEFT       = 0x0040;
const sal_uInt16 BIFF_WINDOW2_SHOWOUTLINE       = 0x0080;
const sal_uInt16 BIFF_WINDOW2_FROZENNOSPLIT     = 0x0100;
const sal_uInt16 BIFF_WINDOW2_SELECTED          = 0x0200;
const sal_uInt16 BIFF_WINDOW2_DISPLAYED         = 0x0400;
const sal_uInt16 BIFF_WINDOW2_PAGEBREAKMODE     = 0x0800;

// Attention: view settings in Calc do not use com.sun.star.view.DocumentZoomType!
const sal_Int16 API_ZOOMTYPE_PERCENT            = 0;        /// Zoom value in percent.

const sal_Int32 API_ZOOMVALUE_MIN               = 20;       /// Minimum zoom in Calc.
const sal_Int32 API_ZOOMVALUE_MAX               = 400;      /// Maximum zoom in Calc.

// no predefined constants for split mode
const sal_Int16 API_SPLITMODE_NONE              = 0;        /// No splits in window.
const sal_Int16 API_SPLITMODE_SPLIT             = 1;        /// Window is split.
const sal_Int16 API_SPLITMODE_FREEZE            = 2;        /// Window has frozen panes.

// no predefined constants for pane idetifiers
const sal_Int16 API_SPLITPANE_TOPLEFT           = 0;        /// Top-left, or top pane.
const sal_Int16 API_SPLITPANE_TOPRIGHT          = 1;        /// Top-right pane.
const sal_Int16 API_SPLITPANE_BOTTOMLEFT        = 2;        /// Bottom-left, bottom, left, or single pane.
const sal_Int16 API_SPLITPANE_BOTTOMRIGHT       = 3;        /// Bottom-right, or right pane.

// ----------------------------------------------------------------------------

/** Returns the OOXML pane identifier from the passed OOBIN or BIFF pane id. */
sal_Int32 lclGetOoxPaneId( sal_Int32 nBinPaneId, sal_Int32 nDefaultPaneId )
{
    static const sal_Int32 spnPaneIds[] = { XML_bottomRight, XML_topRight, XML_bottomLeft, XML_topLeft };
    return STATIC_ARRAY_SELECT( spnPaneIds, nBinPaneId, nDefaultPaneId );
}

} // namespace

// ============================================================================

PaneSelectionModel::PaneSelectionModel() :
    mnActiveCellId( 0 )
{
}

// ----------------------------------------------------------------------------

SheetViewModel::SheetViewModel() :
    mnWorkbookViewId( 0 ),
    mnViewType( XML_normal ),
    mnActivePaneId( XML_topLeft ),
    mnPaneState( XML_split ),
    mfSplitX( 0.0 ),
    mfSplitY( 0.0 ),
    mnCurrentZoom( 0 ),
    mnNormalZoom( 0 ),
    mnSheetLayoutZoom( 0 ),
    mnPageLayoutZoom( 0 ),
    mbSelected( false ),
    mbRightToLeft( false ),
    mbDefGridColor( true ),
    mbShowFormulas( false ),
    mbShowGrid( true ),
    mbShowHeadings( true ),
    mbShowZeros( true ),
    mbShowOutline( true ),
    mbZoomToFit( false )
{
    maGridColor.setIndexed( OOX_COLOR_WINDOWTEXT );
}

bool SheetViewModel::isPageBreakPreview() const
{
    return mnViewType == XML_pageBreakPreview;
}

sal_Int32 SheetViewModel::getNormalZoom() const
{
    const sal_Int32& rnZoom = isPageBreakPreview() ? mnNormalZoom : mnCurrentZoom;
    sal_Int32 nZoom = (rnZoom > 0) ? rnZoom : OOX_SHEETVIEW_NORMALZOOM_DEF;
    return getLimitedValue< sal_Int32 >( nZoom, API_ZOOMVALUE_MIN, API_ZOOMVALUE_MAX );
}

sal_Int32 SheetViewModel::getPageBreakZoom() const
{
    const sal_Int32& rnZoom = isPageBreakPreview() ? mnCurrentZoom : mnSheetLayoutZoom;
    sal_Int32 nZoom = (rnZoom > 0) ? rnZoom : OOX_SHEETVIEW_SHEETLAYZOOM_DEF;
    return getLimitedValue< sal_Int32 >( nZoom, API_ZOOMVALUE_MIN, API_ZOOMVALUE_MAX );
}

const PaneSelectionModel* SheetViewModel::getPaneSelection( sal_Int32 nPaneId ) const
{
    return maPaneSelMap.get( nPaneId ).get();
}

const PaneSelectionModel* SheetViewModel::getActiveSelection() const
{
    return getPaneSelection( mnActivePaneId );
}

PaneSelectionModel& SheetViewModel::createPaneSelection( sal_Int32 nPaneId )
{
    PaneSelectionModelMap::mapped_type& rxPaneSel = maPaneSelMap[ nPaneId ];
    if( !rxPaneSel )
        rxPaneSel.reset( new PaneSelectionModel );
    return *rxPaneSel;
}

// ----------------------------------------------------------------------------

SheetViewSettings::SheetViewSettings( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void SheetViewSettings::importSheetView( const AttributeList& rAttribs )
{
    SheetViewModel& rModel = *createSheetView();
    rModel.maGridColor.setIndexed( rAttribs.getInteger( XML_colorId, OOX_COLOR_WINDOWTEXT ) );
    rModel.maFirstPos        = getAddressConverter().createValidCellAddress( rAttribs.getString( XML_topLeftCell, OUString() ), getSheetIndex(), false );
    rModel.mnWorkbookViewId  = rAttribs.getToken( XML_workbookViewId, 0 );
    rModel.mnViewType        = rAttribs.getToken( XML_view, XML_normal );
    rModel.mnCurrentZoom     = rAttribs.getInteger( XML_zoomScale, 100 );
    rModel.mnNormalZoom      = rAttribs.getInteger( XML_zoomScaleNormal, 0 );
    rModel.mnSheetLayoutZoom = rAttribs.getInteger( XML_zoomScaleSheetLayoutView, 0 );
    rModel.mnPageLayoutZoom  = rAttribs.getInteger( XML_zoomScalePageLayoutView, 0 );
    rModel.mbSelected        = rAttribs.getBool( XML_tabSelected, false );
    rModel.mbRightToLeft     = rAttribs.getBool( XML_rightToLeft, false );
    rModel.mbDefGridColor    = rAttribs.getBool( XML_defaultGridColor, true );
    rModel.mbShowFormulas    = rAttribs.getBool( XML_showFormulas, false );
    rModel.mbShowGrid        = rAttribs.getBool( XML_showGridLines, true );
    rModel.mbShowHeadings    = rAttribs.getBool( XML_showRowColHeaders, true );
    rModel.mbShowZeros       = rAttribs.getBool( XML_showZeros, true );
    rModel.mbShowOutline     = rAttribs.getBool( XML_showOutlineSymbols, true );
}

void SheetViewSettings::importPane( const AttributeList& rAttribs )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importPane - missing sheet view model" );
    if( !maSheetViews.empty() )
    {
        SheetViewModel& rModel = *maSheetViews.back();
        rModel.maSecondPos    = getAddressConverter().createValidCellAddress( rAttribs.getString( XML_topLeftCell, OUString() ), getSheetIndex(), false );
        rModel.mnActivePaneId = rAttribs.getToken( XML_activePane, XML_topLeft );
        rModel.mnPaneState    = rAttribs.getToken( XML_state, XML_split );
        rModel.mfSplitX       = rAttribs.getDouble( XML_xSplit, 0.0 );
        rModel.mfSplitY       = rAttribs.getDouble( XML_ySplit, 0.0 );
    }
}

void SheetViewSettings::importSelection( const AttributeList& rAttribs )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importSelection - missing sheet view model" );
    if( !maSheetViews.empty() )
    {
        // pane this selection belongs to
        sal_Int32 nPaneId = rAttribs.getToken( XML_pane, XML_topLeft );
        PaneSelectionModel& rSelData = maSheetViews.back()->createPaneSelection( nPaneId );
        // cursor position
        rSelData.maActiveCell = getAddressConverter().createValidCellAddress( rAttribs.getString( XML_activeCell, OUString() ), getSheetIndex(), false );
        rSelData.mnActiveCellId = rAttribs.getInteger( XML_activeCellId, 0 );
        // selection
        rSelData.maSelection.clear();
        getAddressConverter().convertToCellRangeList( rSelData.maSelection, rAttribs.getString( XML_sqref, OUString() ), getSheetIndex(), false );
    }
}

void SheetViewSettings::importChartSheetView( const AttributeList& rAttribs )
{
    SheetViewModel& rModel = *createSheetView();
    rModel.mnWorkbookViewId = rAttribs.getToken( XML_workbookViewId, 0 );
    rModel.mnCurrentZoom    = rAttribs.getInteger( XML_zoomScale, 100 );
    rModel.mbSelected       = rAttribs.getBool( XML_tabSelected, false );
    rModel.mbZoomToFit      = rAttribs.getBool( XML_zoomToFit, false );
}

void SheetViewSettings::importSheetView( RecordInputStream& rStrm )
{
    SheetViewModel& rModel = *createSheetView();
    sal_uInt16 nFlags;
    sal_Int32 nViewType;
    BinAddress aFirstPos;
    rStrm >> nFlags >> nViewType >> aFirstPos;
    rModel.maGridColor.importColorId( rStrm );
    rModel.mnCurrentZoom = rStrm.readuInt16();
    rModel.mnNormalZoom = rStrm.readuInt16();
    rModel.mnSheetLayoutZoom = rStrm.readuInt16();
    rModel.mnPageLayoutZoom = rStrm.readuInt16();
    rStrm >> rModel.mnWorkbookViewId;

    rModel.maFirstPos = getAddressConverter().createValidCellAddress( aFirstPos, getSheetIndex(), false );
    static const sal_Int32 spnViewTypes[] = { XML_normal, XML_pageBreakPreview, XML_pageLayout };
    rModel.mnViewType = STATIC_ARRAY_SELECT( spnViewTypes, nViewType, XML_normal );
    rModel.mbSelected     = getFlag( nFlags, OOBIN_SHEETVIEW_SELECTED );
    rModel.mbRightToLeft  = getFlag( nFlags, OOBIN_SHEETVIEW_RIGHTTOLEFT );
    rModel.mbDefGridColor = getFlag( nFlags, OOBIN_SHEETVIEW_DEFGRIDCOLOR );
    rModel.mbShowFormulas = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWFORMULAS );
    rModel.mbShowGrid     = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWGRID );
    rModel.mbShowHeadings = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWHEADINGS );
    rModel.mbShowZeros    = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWZEROS );
    rModel.mbShowOutline  = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWOUTLINE );
}

void SheetViewSettings::importPane( RecordInputStream& rStrm )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importPane - missing sheet view model" );
    if( !maSheetViews.empty() )
    {
        SheetViewModel& rModel = *maSheetViews.back();

        BinAddress aSecondPos;
        sal_Int32 nActivePaneId;
        sal_uInt8 nFlags;
        rStrm >> rModel.mfSplitX >> rModel.mfSplitY >> aSecondPos >> nActivePaneId >> nFlags;

        rModel.maSecondPos    = getAddressConverter().createValidCellAddress( aSecondPos, getSheetIndex(), false );
        rModel.mnActivePaneId = lclGetOoxPaneId( nActivePaneId, XML_topLeft );
        rModel.mnPaneState    = getFlagValue( nFlags, OOBIN_PANE_FROZEN, getFlagValue( nFlags, OOBIN_PANE_FROZENNOSPLIT, XML_frozen, XML_frozenSplit ), XML_split );
    }
}

void SheetViewSettings::importSelection( RecordInputStream& rStrm )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importSelection - missing sheet view model" );
    if( !maSheetViews.empty() )
    {
        // pane this selection belongs to
        sal_Int32 nPaneId = rStrm.readInt32();
        PaneSelectionModel& rPaneSel = maSheetViews.back()->createPaneSelection( lclGetOoxPaneId( nPaneId, -1 ) );
        // cursor position
        BinAddress aActiveCell;
        rStrm >> aActiveCell >> rPaneSel.mnActiveCellId;
        rPaneSel.maActiveCell = getAddressConverter().createValidCellAddress( aActiveCell, getSheetIndex(), false );
        // selection
        BinRangeList aSelection;
        rStrm >> aSelection;
        rPaneSel.maSelection.clear();
        getAddressConverter().convertToCellRangeList( rPaneSel.maSelection, aSelection, getSheetIndex(), false );
    }
}

void SheetViewSettings::importChartSheetView( RecordInputStream& rStrm )
{
    SheetViewModel& rModel = *createSheetView();
    sal_uInt16 nFlags;
    rStrm >> nFlags >> rModel.mnCurrentZoom >> rModel.mnWorkbookViewId;

    rModel.mbSelected  = getFlag( nFlags, OOBIN_CHARTSHEETVIEW_SELECTED );
    rModel.mbZoomToFit = getFlag( nFlags, OOBIN_CHARTSHEETVIEW_ZOOMTOFIT );
}

void SheetViewSettings::importWindow2( BiffInputStream& rStrm )
{
    OSL_ENSURE( maSheetViews.empty(), "SheetViewSettings::importWindow2 - multiple WINDOW2 records" );
    SheetViewModel& rModel = *createSheetView();
    if( getBiff() == BIFF2 )
    {
        rModel.mbShowFormulas = rStrm.readuInt8() != 0;
        rModel.mbShowGrid = rStrm.readuInt8() != 0;
        rModel.mbShowHeadings = rStrm.readuInt8() != 0;
        rModel.mnPaneState = (rStrm.readuInt8() == 0) ? XML_split : XML_frozen;
        rModel.mbShowZeros = rStrm.readuInt8() != 0;
        BinAddress aFirstPos;
        rStrm >> aFirstPos;
        rModel.maFirstPos = getAddressConverter().createValidCellAddress( aFirstPos, getSheetIndex(), false );
        rModel.mbDefGridColor = rStrm.readuInt8() != 0;
        rModel.maGridColor.importColorRgb( rStrm );
    }
    else
    {
        sal_uInt16 nFlags;
        BinAddress aFirstPos;
        rStrm >> nFlags >> aFirstPos;

        rModel.maFirstPos     = getAddressConverter().createValidCellAddress( aFirstPos, getSheetIndex(), false );
        rModel.mnPaneState    = getFlagValue( nFlags, BIFF_WINDOW2_FROZEN, getFlagValue( nFlags, BIFF_WINDOW2_FROZENNOSPLIT, XML_frozen, XML_frozenSplit ), XML_split );
        rModel.mbSelected     = getFlag( nFlags, BIFF_WINDOW2_SELECTED );
        rModel.mbRightToLeft  = getFlag( nFlags, BIFF_WINDOW2_RIGHTTOLEFT );
        rModel.mbDefGridColor = getFlag( nFlags, BIFF_WINDOW2_DEFGRIDCOLOR );
        rModel.mbShowFormulas = getFlag( nFlags, BIFF_WINDOW2_SHOWFORMULAS );
        rModel.mbShowGrid     = getFlag( nFlags, BIFF_WINDOW2_SHOWGRID );
        rModel.mbShowHeadings = getFlag( nFlags, BIFF_WINDOW2_SHOWHEADINGS );
        rModel.mbShowZeros    = getFlag( nFlags, BIFF_WINDOW2_SHOWZEROS );
        rModel.mbShowOutline  = getFlag( nFlags, BIFF_WINDOW2_SHOWOUTLINE );

        if( getBiff() == BIFF8 )
        {
            rModel.mnViewType = getFlagValue( nFlags, BIFF_WINDOW2_PAGEBREAKMODE, XML_pageBreakPreview, XML_normal );

            rModel.maGridColor.importColorId( rStrm );
            // zoom data not included in chart sheets
            if( (getSheetType() != SHEETTYPE_CHARTSHEET) && (rStrm.getRemaining() >= 6) )
            {
                rStrm.skip( 2 );
                sal_uInt16 nPageZoom, nNormalZoom;
                rStrm >> nPageZoom >> nNormalZoom;
                rModel.mnSheetLayoutZoom = nPageZoom;
                rModel.mnNormalZoom = nNormalZoom;
            }
        }
        else
        {
            rModel.maGridColor.importColorRgb( rStrm );
        }
    }
}

void SheetViewSettings::importPane( BiffInputStream& rStrm )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importPane - missing leading WINDOW2 record" );
    if( !maSheetViews.empty() )
    {
        sal_uInt8 nActivePaneId;
        sal_uInt16 nSplitX, nSplitY;
        BinAddress aSecondPos;
        rStrm >> nSplitX >> nSplitY >> aSecondPos >> nActivePaneId;

        SheetViewModel& rModel = *maSheetViews.back();
        rModel.mfSplitX = nSplitX;
        rModel.mfSplitY = nSplitY;
        rModel.maSecondPos = getAddressConverter().createValidCellAddress( aSecondPos, getSheetIndex(), false );
        rModel.mnActivePaneId = lclGetOoxPaneId( nActivePaneId, XML_topLeft );
    }
}

void SheetViewSettings::importScl( BiffInputStream& rStrm )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importScl - missing leading WINDOW2 record" );
    if( !maSheetViews.empty() )
    {
        sal_uInt16 nNum, nDenom;
        rStrm >> nNum >> nDenom;
        OSL_ENSURE( nDenom > 0, "SheetViewSettings::importScl - invalid denominator" );
        if( nDenom > 0 )
            maSheetViews.back()->mnCurrentZoom = getLimitedValue< sal_Int32, sal_uInt16 >( (nNum * 100) / nDenom, 10, 400 );
    }
}

void SheetViewSettings::importSelection( BiffInputStream& rStrm )
{
    OSL_ENSURE( !maSheetViews.empty(), "SheetViewSettings::importPane - missing leading WINDOW2 record" );
    if( !maSheetViews.empty() )
    {
        // pane this selection belongs to
        sal_uInt8 nPaneId = rStrm.readuInt8();
        PaneSelectionModel& rPaneSel = maSheetViews.back()->createPaneSelection( lclGetOoxPaneId( nPaneId, -1 ) );
        // cursor position
        BinAddress aActiveCell;
        sal_uInt16 nActiveCellId;
        rStrm >> aActiveCell >> nActiveCellId;
        rPaneSel.maActiveCell = getAddressConverter().createValidCellAddress( aActiveCell, getSheetIndex(), false );
        rPaneSel.mnActiveCellId = nActiveCellId;
        // selection
        rPaneSel.maSelection.clear();
        BinRangeList aSelection;
        aSelection.read( rStrm, false );
        getAddressConverter().convertToCellRangeList( rPaneSel.maSelection, aSelection, getSheetIndex(), false );
    }
}

void SheetViewSettings::finalizeImport()
{
    // force creation of sheet view model to get the Excel defaults
    SheetViewModelRef xModel = maSheetViews.empty() ? createSheetView() : maSheetViews.front();

    // #i59590# #158194# special handling for chart sheets (Excel ignores some settings in chart sheets)
    if( getSheetType() == SHEETTYPE_CHARTSHEET )
    {
        xModel->maPaneSelMap.clear();
        xModel->maFirstPos = xModel->maSecondPos = CellAddress( getSheetIndex(), 0, 0 );
        xModel->mnViewType = XML_normal;
        xModel->mnActivePaneId = XML_topLeft;
        xModel->mnPaneState = XML_split;
        xModel->mfSplitX = xModel->mfSplitY = 0.0;
        xModel->mbRightToLeft = false;
        xModel->mbDefGridColor = true;
        xModel->mbShowFormulas = false;
        xModel->mbShowGrid = true;
        xModel->mbShowHeadings = true;
        xModel->mbShowZeros = true;
        xModel->mbShowOutline = true;
    }

    // mirrored sheet (this is not a view setting in Calc)
    if( xModel->mbRightToLeft )
    {
        PropertySet aPropSet( getSheet() );
        aPropSet.setProperty( PROP_TableLayout, ::com::sun::star::text::WritingMode2::RL_TB );
    }

    // sheet selected (active sheet must be selected)
    bool bSelected = xModel->mbSelected || (getSheetIndex() == getViewSettings().getActiveSheetIndex());

    // visible area and current cursor position (selection not supported via API)
    CellAddress aFirstPos = xModel->maFirstPos;
    const PaneSelectionModel* pPaneSel = xModel->getActiveSelection();
    CellAddress aCursor = pPaneSel ? pPaneSel->maActiveCell : aFirstPos;

    // freeze/split position default
    sal_Int16 nHSplitMode = API_SPLITMODE_NONE;
    sal_Int16 nVSplitMode = API_SPLITMODE_NONE;
    sal_Int32 nHSplitPos = 0;
    sal_Int32 nVSplitPos = 0;
    // active pane default
    sal_Int16 nActivePane = API_SPLITPANE_BOTTOMLEFT;

    // freeze/split position
    if( (xModel->mnPaneState == XML_frozen) || (xModel->mnPaneState == XML_frozenSplit) )
    {
        /*  Frozen panes: handle split position as row/column positions.
            #i35812# Excel uses number of visible rows/columns in the
                frozen area (rows/columns scolled outside are not incuded),
                Calc uses absolute position of first unfrozen row/column. */
        const CellAddress& rMaxApiPos = getAddressConverter().getMaxApiAddress();
        if( (xModel->mfSplitX >= 1.0) && (xModel->maFirstPos.Column + xModel->mfSplitX <= rMaxApiPos.Column) )
            nHSplitPos = static_cast< sal_Int32 >( xModel->maFirstPos.Column + xModel->mfSplitX );
        nHSplitMode = (nHSplitPos > 0) ? API_SPLITMODE_FREEZE : API_SPLITMODE_NONE;
        if( (xModel->mfSplitY >= 1.0) && (xModel->maFirstPos.Row + xModel->mfSplitY <= rMaxApiPos.Row) )
            nVSplitPos = static_cast< sal_Int32 >( xModel->maFirstPos.Row + xModel->mfSplitY );
        nVSplitMode = (nVSplitPos > 0) ? API_SPLITMODE_FREEZE : API_SPLITMODE_NONE;
    }
    else if( xModel->mnPaneState == XML_split )
    {
        // split window: view settings API uses twips...
        nHSplitPos = getLimitedValue< sal_Int32, double >( xModel->mfSplitX + 0.5, 0, SAL_MAX_INT32 );
        nHSplitMode = (nHSplitPos > 0) ? API_SPLITMODE_SPLIT : API_SPLITMODE_NONE;
        nVSplitPos = getLimitedValue< sal_Int32, double >( xModel->mfSplitY + 0.5, 0, SAL_MAX_INT32 );
        nVSplitMode = (nVSplitPos > 0) ? API_SPLITMODE_SPLIT : API_SPLITMODE_NONE;
    }

    // active pane
    switch( xModel->mnActivePaneId )
    {
        // no horizontal split -> always use left panes
        // no vertical split -> always use *bottom* panes
        case XML_topLeft:
            nActivePane = (nVSplitMode == API_SPLITMODE_NONE) ? API_SPLITPANE_BOTTOMLEFT : API_SPLITPANE_TOPLEFT;
        break;
        case XML_topRight:
            nActivePane = (nHSplitMode == API_SPLITMODE_NONE) ?
                ((nVSplitMode == API_SPLITMODE_NONE) ? API_SPLITPANE_BOTTOMLEFT : API_SPLITPANE_TOPLEFT) :
                ((nVSplitMode == API_SPLITMODE_NONE) ? API_SPLITPANE_BOTTOMRIGHT : API_SPLITPANE_TOPRIGHT);
        break;
        case XML_bottomLeft:
            nActivePane = API_SPLITPANE_BOTTOMLEFT;
        break;
        case XML_bottomRight:
            nActivePane = (nHSplitMode == API_SPLITMODE_NONE) ? API_SPLITPANE_BOTTOMLEFT : API_SPLITPANE_BOTTOMRIGHT;
        break;
    }

    // automatic grid color
    if( xModel->mbDefGridColor )
        xModel->maGridColor.setAuto();

    // write the sheet view settings into the property sequence
    PropertyMap aPropMap;
    aPropMap[ PROP_TableSelected ]                <<= bSelected;
    aPropMap[ PROP_CursorPositionX ]              <<= aCursor.Column;
    aPropMap[ PROP_CursorPositionY ]              <<= aCursor.Row;
    aPropMap[ PROP_HorizontalSplitMode ]          <<= nHSplitMode;
    aPropMap[ PROP_VerticalSplitMode ]            <<= nVSplitMode;
    aPropMap[ PROP_HorizontalSplitPositionTwips ] <<= nHSplitPos;
    aPropMap[ PROP_VerticalSplitPositionTwips ]   <<= nVSplitPos;
    aPropMap[ PROP_ActiveSplitRange ]             <<= nActivePane;
    aPropMap[ PROP_PositionLeft ]                 <<= aFirstPos.Column;
    aPropMap[ PROP_PositionTop ]                  <<= aFirstPos.Row;
    aPropMap[ PROP_PositionRight ]                <<= xModel->maSecondPos.Column;
    aPropMap[ PROP_PositionBottom ]               <<= ((nVSplitPos > 0) ? xModel->maSecondPos.Row : xModel->maFirstPos.Row);
    aPropMap[ PROP_ZoomType ]                     <<= API_ZOOMTYPE_PERCENT;
    aPropMap[ PROP_ZoomValue ]                    <<= static_cast< sal_Int16 >( xModel->getNormalZoom() );
    aPropMap[ PROP_PageViewZoomValue ]            <<= static_cast< sal_Int16 >( xModel->getPageBreakZoom() );
    aPropMap[ PROP_GridColor ]                    <<= xModel->maGridColor.getColor( *this );
    aPropMap[ PROP_ShowPageBreakPreview ]         <<= xModel->isPageBreakPreview();
    aPropMap[ PROP_ShowFormulas ]                 <<= xModel->mbShowFormulas;
    aPropMap[ PROP_ShowGrid ]                     <<= xModel->mbShowGrid;
    aPropMap[ PROP_HasColumnRowHeaders ]          <<= xModel->mbShowHeadings;
    aPropMap[ PROP_ShowZeroValues ]               <<= xModel->mbShowZeros;
    aPropMap[ PROP_IsOutlineSymbolsSet ]          <<= xModel->mbShowOutline;

    // store sheet view settings in global view settings object
    getViewSettings().setSheetViewSettings( getSheetIndex(), xModel, Any( aPropMap.makePropertyValueSequence() ) );
}

// private --------------------------------------------------------------------

SheetViewModelRef SheetViewSettings::createSheetView()
{
    SheetViewModelRef xModel( new SheetViewModel );
    maSheetViews.push_back( xModel );
    return xModel;
}

// ============================================================================

WorkbookViewModel::WorkbookViewModel() :
    mnWinX( 0 ),
    mnWinY( 0 ),
    mnWinWidth( 0 ),
    mnWinHeight( 0 ),
    mnActiveSheet( 0 ),
    mnFirstVisSheet( 0 ),
    mnTabBarWidth( OOX_BOOKVIEW_TABBARRATIO_DEF ),
    mnVisibility( XML_visible ),
    mbShowTabBar( true ),
    mbShowHorScroll( true ),
    mbShowVerScroll( true ),
    mbMinimized( false )
{
}

// ----------------------------------------------------------------------------

ViewSettings::ViewSettings( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void ViewSettings::importWorkbookView( const AttributeList& rAttribs )
{
    WorkbookViewModel& rModel = createWorkbookView();
    rModel.mnWinX          = rAttribs.getInteger( XML_xWindow, 0 );
    rModel.mnWinY          = rAttribs.getInteger( XML_yWindow, 0 );
    rModel.mnWinWidth      = rAttribs.getInteger( XML_windowWidth, 0 );
    rModel.mnWinHeight     = rAttribs.getInteger( XML_windowHeight, 0 );
    rModel.mnActiveSheet   = rAttribs.getInteger( XML_activeTab, 0 );
    rModel.mnFirstVisSheet = rAttribs.getInteger( XML_firstSheet, 0 );
    rModel.mnTabBarWidth   = rAttribs.getInteger( XML_tabRatio, 600 );
    rModel.mnVisibility    = rAttribs.getToken( XML_visibility, XML_visible );
    rModel.mbShowTabBar    = rAttribs.getBool( XML_showSheetTabs, true );
    rModel.mbShowHorScroll = rAttribs.getBool( XML_showHorizontalScroll, true );
    rModel.mbShowVerScroll = rAttribs.getBool( XML_showVerticalScroll, true );
    rModel.mbMinimized     = rAttribs.getBool( XML_minimized, false );
}

void ViewSettings::importWorkbookView( RecordInputStream& rStrm )
{
    WorkbookViewModel& rModel = createWorkbookView();
    sal_uInt8 nFlags;
    rStrm >> rModel.mnWinX >> rModel.mnWinY >> rModel.mnWinWidth >> rModel.mnWinHeight >> rModel.mnTabBarWidth >> rModel.mnFirstVisSheet >> rModel.mnActiveSheet >> nFlags;
    rModel.mnVisibility    = getFlagValue( nFlags, OOBIN_WBVIEW_HIDDEN, XML_hidden, XML_visible );
    rModel.mbShowTabBar    = getFlag( nFlags, OOBIN_WBVIEW_SHOWTABBAR );
    rModel.mbShowHorScroll = getFlag( nFlags, OOBIN_WBVIEW_SHOWHORSCROLL );
    rModel.mbShowVerScroll = getFlag( nFlags, OOBIN_WBVIEW_SHOWVERSCROLL );
    rModel.mbMinimized     = getFlag( nFlags, OOBIN_WBVIEW_MINIMIZED );
}

void ViewSettings::importWindow1( BiffInputStream& rStrm )
{
    sal_uInt16 nWinX, nWinY, nWinWidth, nWinHeight;
    rStrm >> nWinX >> nWinY >> nWinWidth >> nWinHeight;

    // WINDOW1 record occures in every sheet in BIFF4W
    OSL_ENSURE( maBookViews.empty() || ((getBiff() == BIFF4) && isWorkbookFile()),
        "ViewSettings::importWindow1 - multiple WINDOW1 records" );
    WorkbookViewModel& rModel = createWorkbookView();
    rModel.mnWinX = nWinX;
    rModel.mnWinY = nWinY;
    rModel.mnWinWidth = nWinWidth;
    rModel.mnWinHeight = nWinHeight;

    if( getBiff() <= BIFF4 )
    {
        sal_uInt8 nHidden;
        rStrm >> nHidden;
        rModel.mnVisibility = (nHidden == 0) ? XML_visible : XML_hidden;
    }
    else
    {
        sal_uInt16 nFlags, nActiveTab, nFirstVisTab, nSelectCnt, nTabBarWidth;
        rStrm >> nFlags >> nActiveTab >> nFirstVisTab >> nSelectCnt >> nTabBarWidth;

        rModel.mnActiveSheet = nActiveTab;
        rModel.mnFirstVisSheet = nFirstVisTab;
        rModel.mnTabBarWidth = nTabBarWidth;
        rModel.mnVisibility = getFlagValue( nFlags, BIFF_WINDOW1_HIDDEN, XML_hidden, XML_visible );
        rModel.mbMinimized = getFlag( nFlags, BIFF_WINDOW1_MINIMIZED );
        rModel.mbShowHorScroll = getFlag( nFlags, BIFF_WINDOW1_SHOWHORSCROLL );
        rModel.mbShowVerScroll = getFlag( nFlags, BIFF_WINDOW1_SHOWVERSCROLL );
        rModel.mbShowTabBar = getFlag( nFlags, BIFF_WINDOW1_SHOWTABBAR );
    }
}

void ViewSettings::setSheetViewSettings( sal_Int32 nSheet, const SheetViewModelRef& rxSheetView, const Any& rProperties )
{
    maSheetViews[ nSheet ] = rxSheetView;
    maSheetProps[ nSheet ] = rProperties;
}

void ViewSettings::finalizeImport()
{
    const WorksheetBuffer& rWorksheets = getWorksheets();
    if( rWorksheets.getSheetCount() <= 0 ) return;

    // force creation of workbook view model to get the Excel defaults
    const WorkbookViewModel& rModel = maBookViews.empty() ? createWorkbookView() : *maBookViews.front();

    // show object mode is part of workbook settings
    sal_Int16 nShowMode = getWorkbookSettings().getApiShowObjectMode();

    // view settings for all sheets
    Reference< XNameContainer > xSheetsNC = ContainerHelper::createNameContainer( getBaseFilter().getGlobalFactory() );
    if( !xSheetsNC.is() ) return;
    for( SheetPropertiesMap::const_iterator aIt = maSheetProps.begin(), aEnd = maSheetProps.end(); aIt != aEnd; ++aIt )
        ContainerHelper::insertByName( xSheetsNC, rWorksheets.getCalcSheetName( aIt->first ), aIt->second );

    // use active sheet to set sheet properties that are document-global in Calc
    sal_Int32 nActiveSheet = getActiveSheetIndex();
    SheetViewModelRef& rxActiveSheetView = maSheetViews[ nActiveSheet ];
    OSL_ENSURE( rxActiveSheetView.get(), "ViewSettings::finalizeImport - missing active sheet view settings" );
    if( !rxActiveSheetView )
        rxActiveSheetView.reset( new SheetViewModel );

    Reference< XIndexContainer > xContainer = ContainerHelper::createIndexContainer( getBaseFilter().getGlobalFactory() );
    if( xContainer.is() ) try
    {
        PropertyMap aPropMap;
        aPropMap[ PROP_Tables ]                        <<= xSheetsNC;
        aPropMap[ PROP_ActiveTable ]                   <<= rWorksheets.getCalcSheetName( nActiveSheet );
        aPropMap[ PROP_HasHorizontalScrollBar ]        <<= rModel.mbShowHorScroll;
        aPropMap[ PROP_HasVerticalScrollBar ]          <<= rModel.mbShowVerScroll;
        aPropMap[ PROP_HasSheetTabs ]                  <<= rModel.mbShowTabBar;
        aPropMap[ PROP_RelativeHorizontalTabbarWidth ] <<= double( rModel.mnTabBarWidth / 1000.0 );
        aPropMap[ PROP_ShowObjects ]                   <<= nShowMode;
        aPropMap[ PROP_ShowCharts ]                    <<= nShowMode;
        aPropMap[ PROP_ShowDrawing ]                   <<= nShowMode;
        aPropMap[ PROP_GridColor ]                     <<= rxActiveSheetView->maGridColor.getColor( *this );
        aPropMap[ PROP_ShowPageBreakPreview ]          <<= rxActiveSheetView->isPageBreakPreview();
        aPropMap[ PROP_ShowFormulas ]                  <<= rxActiveSheetView->mbShowFormulas;
        aPropMap[ PROP_ShowGrid ]                      <<= rxActiveSheetView->mbShowGrid;
        aPropMap[ PROP_HasColumnRowHeaders ]           <<= rxActiveSheetView->mbShowHeadings;
        aPropMap[ PROP_ShowZeroValues ]                <<= rxActiveSheetView->mbShowZeros;
        aPropMap[ PROP_IsOutlineSymbolsSet ]           <<= rxActiveSheetView->mbShowOutline;

        xContainer->insertByIndex( 0, Any( aPropMap.makePropertyValueSequence() ) );
        Reference< XIndexAccess > xIAccess( xContainer, UNO_QUERY_THROW );
        Reference< XViewDataSupplier > xViewDataSuppl( getDocument(), UNO_QUERY_THROW );
        xViewDataSuppl->setViewData( xIAccess );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "ViewSettings::finalizeImport - cannot create document view settings" );
    }
}

sal_Int32 ViewSettings::getActiveSheetIndex() const
{
    sal_Int32 nSheetCount = getLimitedValue< sal_Int32, sal_Int32 >( getWorksheets().getSheetCount(), 1, SAL_MAX_INT32 );
    return maBookViews.empty() ? 0 : getLimitedValue< sal_Int32, sal_Int32 >( maBookViews.front()->mnActiveSheet, 0, nSheetCount - 1 );
}

// private --------------------------------------------------------------------

WorkbookViewModel& ViewSettings::createWorkbookView()
{
    WorkbookViewModelRef xModel( new WorkbookViewModel );
    maBookViews.push_back( xModel );
    return *xModel;
}

// ============================================================================

} // namespace xls
} // namespace oox

