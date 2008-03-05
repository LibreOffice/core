/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewsettings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:08:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/viewsettings.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertysequence.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
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

/** Property names for document view settings. */
const sal_Char* const sppcDocNames[] =
{
    "Tables",
    "ActiveTable",
    "HasHorizontalScrollBar",
    "HasVerticalScrollBar",
    "HasSheetTabs",
    "RelativeHorizontalTabbarWidth",
    "ShowObjects",
    "ShowCharts",
    "ShowDrawing",
    0
};

/** Property names for sheet view settings that are document-global in Calc. */
const sal_Char* const sppcGlobalSheetNames[] =
{
    "GridColor",
    "ZoomType",
    "ZoomValue",
    "PageViewZoomValue",
    "ShowPageBreakPreview",
    "ShowFormulas",
    "ShowGrid",
    "HasColumnRowHeaders",
    "ShowZeroValues",
    "IsOutlineSymbolsSet",
    0
};

/** Property names for sheet view settings. */
const sal_Char* const sppcSheetNames[] =
{
    "TableSelected",
    "CursorPositionX",
    "CursorPositionY",
    "HorizontalSplitMode",
    "VerticalSplitMode",
    "HorizontalSplitPositionTwips",
    "VerticalSplitPositionTwips",
    "ActiveSplitRange",
    "PositionLeft",
    "PositionTop",
    "PositionRight",
    "PositionBottom",
    0
};

// ----------------------------------------------------------------------------

/** Returns the OOXML pane identifier from the passed OOBIN or BIFF pane id. */
sal_Int32 lclGetOoxPaneId( sal_Int32 nBinPaneId, sal_Int32 nDefaultPaneId )
{
    static const sal_Int32 spnPaneIds[] = { XML_bottomRight, XML_topRight, XML_bottomLeft, XML_topLeft };
    return STATIC_ARRAY_SELECT( spnPaneIds, nBinPaneId, nDefaultPaneId );
}

} // namespace

// ============================================================================

OoxSheetSelectionData::OoxSheetSelectionData() :
    mnActiveCellId( 0 )
{
}

// ----------------------------------------------------------------------------

OoxSheetViewData::OoxSheetViewData() :
    maGridColor( XML_indexed, OOX_COLOR_WINDOWTEXT ),
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
}

bool OoxSheetViewData::isPageBreakPreview() const
{
    return mnViewType == XML_pageBreakPreview;
}

sal_Int32 OoxSheetViewData::getNormalZoom() const
{
    const sal_Int32& rnZoom = isPageBreakPreview() ? mnNormalZoom : mnCurrentZoom;
    sal_Int32 nZoom = (rnZoom > 0) ? rnZoom : OOX_SHEETVIEW_NORMALZOOM_DEF;
    return getLimitedValue< sal_Int32 >( nZoom, API_ZOOMVALUE_MIN, API_ZOOMVALUE_MAX );
}

sal_Int32 OoxSheetViewData::getPageBreakZoom() const
{
    const sal_Int32& rnZoom = isPageBreakPreview() ? mnCurrentZoom : mnSheetLayoutZoom;
    sal_Int32 nZoom = (rnZoom > 0) ? rnZoom : OOX_SHEETVIEW_SHEETLAYZOOM_DEF;
    return getLimitedValue< sal_Int32 >( nZoom, API_ZOOMVALUE_MIN, API_ZOOMVALUE_MAX );
}

const OoxSheetSelectionData* OoxSheetViewData::getSelectionData( sal_Int32 nPaneId ) const
{
    return maSelMap.get( nPaneId ).get();
}

const OoxSheetSelectionData* OoxSheetViewData::getActiveSelectionData() const
{
    return getSelectionData( mnActivePaneId );
}

OoxSheetSelectionData& OoxSheetViewData::createSelectionData( sal_Int32 nPaneId )
{
    OoxSelectionDataMap::mapped_type& rxSelData = maSelMap[ nPaneId ];
    if( !rxSelData )
        rxSelData.reset( new OoxSheetSelectionData );
    return *rxSelData;
}

// ----------------------------------------------------------------------------

SheetViewSettings::SheetViewSettings( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void SheetViewSettings::importSheetView( const AttributeList& rAttribs )
{
    OoxSheetViewData& rData = *createSheetViewData();
    rData.maGridColor.set( XML_indexed, rAttribs.getInteger( XML_colorId, OOX_COLOR_WINDOWTEXT ) );
    rData.maFirstPos        = getAddressConverter().createValidCellAddress( rAttribs.getString( XML_topLeftCell ), getSheetIndex(), false );
    rData.mnWorkbookViewId  = rAttribs.getToken( XML_workbookViewId, 0 );
    rData.mnViewType        = rAttribs.getToken( XML_view, XML_normal );
    rData.mnCurrentZoom     = rAttribs.getInteger( XML_zoomScale, 100 );
    rData.mnNormalZoom      = rAttribs.getInteger( XML_zoomScaleNormal, 0 );
    rData.mnSheetLayoutZoom = rAttribs.getInteger( XML_zoomScaleSheetLayoutView, 0 );
    rData.mnPageLayoutZoom  = rAttribs.getInteger( XML_zoomScalePageLayoutView, 0 );
    rData.mbSelected        = rAttribs.getBool( XML_tabSelected, false );
    rData.mbRightToLeft     = rAttribs.getBool( XML_rightToLeft, false );
    rData.mbDefGridColor    = rAttribs.getBool( XML_defaultGridColor, true );
    rData.mbShowFormulas    = rAttribs.getBool( XML_showFormulas, false );
    rData.mbShowGrid        = rAttribs.getBool( XML_showGridLines, true );
    rData.mbShowHeadings    = rAttribs.getBool( XML_showRowColHeaders, true );
    rData.mbShowZeros       = rAttribs.getBool( XML_showZeros, true );
    rData.mbShowOutline     = rAttribs.getBool( XML_showOutlineSymbols, true );
}

void SheetViewSettings::importPane( const AttributeList& rAttribs )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importPane - missing view data" );
    if( !maSheetDatas.empty() )
    {
        OoxSheetViewData& rData = *maSheetDatas.back();
        rData.maSecondPos    = getAddressConverter().createValidCellAddress( rAttribs.getString( XML_topLeftCell ), getSheetIndex(), false );
        rData.mnActivePaneId = rAttribs.getToken( XML_activePane, XML_topLeft );
        rData.mnPaneState    = rAttribs.getToken( XML_state, XML_split );
        rData.mfSplitX       = rAttribs.getDouble( XML_xSplit, 0.0 );
        rData.mfSplitY       = rAttribs.getDouble( XML_ySplit, 0.0 );
    }
}

void SheetViewSettings::importSelection( const AttributeList& rAttribs )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importSelection - missing view data" );
    if( !maSheetDatas.empty() )
    {
        // pane this selection belongs to
        sal_Int32 nPaneId = rAttribs.getToken( XML_pane, XML_topLeft );
        OoxSheetSelectionData& rSelData = maSheetDatas.back()->createSelectionData( nPaneId );
        // cursor position
        rSelData.maActiveCell = getAddressConverter().createValidCellAddress( rAttribs.getString( XML_activeCell ), getSheetIndex(), false );
        rSelData.mnActiveCellId = rAttribs.getInteger( XML_activeCellId, 0 );
        // selection
        rSelData.maSelection.clear();
        getAddressConverter().convertToCellRangeList( rSelData.maSelection, rAttribs.getString( XML_sqref ), getSheetIndex(), false );
    }
}

void SheetViewSettings::importChartSheetView( const AttributeList& rAttribs )
{
    OoxSheetViewData& rData = *createSheetViewData();
    rData.mnWorkbookViewId = rAttribs.getToken( XML_workbookViewId, 0 );
    rData.mnCurrentZoom    = rAttribs.getInteger( XML_zoomScale, 100 );
    rData.mbSelected       = rAttribs.getBool( XML_tabSelected, false );
    rData.mbZoomToFit      = rAttribs.getBool( XML_zoomToFit, false );
}

void SheetViewSettings::importSheetView( RecordInputStream& rStrm )
{
    OoxSheetViewData& rData = *createSheetViewData();
    sal_uInt16 nFlags;
    sal_Int32 nViewType;
    BinAddress aFirstPos;
    rStrm >> nFlags >> nViewType >> aFirstPos;
    rData.maGridColor.importColorId( rStrm );
    rData.mnCurrentZoom = rStrm.readuInt16();
    rData.mnNormalZoom = rStrm.readuInt16();
    rData.mnSheetLayoutZoom = rStrm.readuInt16();
    rData.mnPageLayoutZoom = rStrm.readuInt16();
    rStrm >> rData.mnWorkbookViewId;

    rData.maFirstPos = getAddressConverter().createValidCellAddress( aFirstPos, getSheetIndex(), false );
    static const sal_Int32 spnViewTypes[] = { XML_normal, XML_pageBreakPreview, XML_pageLayout };
    rData.mnViewType = STATIC_ARRAY_SELECT( spnViewTypes, nViewType, XML_normal );
    rData.mbSelected     = getFlag( nFlags, OOBIN_SHEETVIEW_SELECTED );
    rData.mbRightToLeft  = getFlag( nFlags, OOBIN_SHEETVIEW_RIGHTTOLEFT );
    rData.mbDefGridColor = getFlag( nFlags, OOBIN_SHEETVIEW_DEFGRIDCOLOR );
    rData.mbShowFormulas = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWFORMULAS );
    rData.mbShowGrid     = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWGRID );
    rData.mbShowHeadings = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWHEADINGS );
    rData.mbShowZeros    = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWZEROS );
    rData.mbShowOutline  = getFlag( nFlags, OOBIN_SHEETVIEW_SHOWOUTLINE );
}

void SheetViewSettings::importPane( RecordInputStream& rStrm )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importPane - missing view data" );
    if( !maSheetDatas.empty() )
    {
        OoxSheetViewData& rData = *maSheetDatas.back();

        BinAddress aSecondPos;
        sal_Int32 nActivePaneId;
        sal_uInt8 nFlags;
        rStrm >> rData.mfSplitX >> rData.mfSplitY >> aSecondPos >> nActivePaneId >> nFlags;

        rData.maSecondPos    = getAddressConverter().createValidCellAddress( aSecondPos, getSheetIndex(), false );
        rData.mnActivePaneId = lclGetOoxPaneId( nActivePaneId, XML_topLeft );
        rData.mnPaneState    = getFlagValue( nFlags, OOBIN_PANE_FROZEN, getFlagValue( nFlags, OOBIN_PANE_FROZENNOSPLIT, XML_frozen, XML_frozenSplit ), XML_split );
    }
}

void SheetViewSettings::importSelection( RecordInputStream& rStrm )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importSelection - missing view data" );
    if( !maSheetDatas.empty() )
    {
        // pane this selection belongs to
        sal_Int32 nPaneId = rStrm.readInt32();
        OoxSheetSelectionData& rSelData = maSheetDatas.back()->createSelectionData( lclGetOoxPaneId( nPaneId, -1 ) );
        // cursor position
        BinAddress aActiveCell;
        rStrm >> aActiveCell >> rSelData.mnActiveCellId;
        rSelData.maActiveCell = getAddressConverter().createValidCellAddress( aActiveCell, getSheetIndex(), false );
        // selection
        BinRangeList aSelection;
        rStrm >> aSelection;
        rSelData.maSelection.clear();
        getAddressConverter().convertToCellRangeList( rSelData.maSelection, aSelection, getSheetIndex(), false );
    }
}

void SheetViewSettings::importChartSheetView( RecordInputStream& rStrm )
{
    OoxSheetViewData& rData = *createSheetViewData();
    sal_uInt16 nFlags;
    rStrm >> nFlags >> rData.mnCurrentZoom >> rData.mnWorkbookViewId;

    rData.mbSelected  = getFlag( nFlags, OOBIN_CHARTSHEETVIEW_SELECTED );
    rData.mbZoomToFit = getFlag( nFlags, OOBIN_CHARTSHEETVIEW_ZOOMTOFIT );
}

void SheetViewSettings::importWindow2( BiffInputStream& rStrm )
{
    OSL_ENSURE( maSheetDatas.empty(), "SheetViewSettings::importWindow2 - multiple WINDOW2 records" );
    OoxSheetViewData& rData = *createSheetViewData();
    if( getBiff() == BIFF2 )
    {
        rData.mbShowFormulas = rStrm.readuInt8() != 0;
        rData.mbShowGrid = rStrm.readuInt8() != 0;
        rData.mbShowHeadings = rStrm.readuInt8() != 0;
        rData.mnPaneState = (rStrm.readuInt8() == 0) ? XML_split : XML_frozen;
        rData.mbShowZeros = rStrm.readuInt8() != 0;
        BinAddress aFirstPos;
        rStrm >> aFirstPos;
        rData.maFirstPos = getAddressConverter().createValidCellAddress( aFirstPos, getSheetIndex(), false );
        rData.mbDefGridColor = rStrm.readuInt8() != 0;
        rData.maGridColor.importColorRgb( rStrm );
    }
    else
    {
        sal_uInt16 nFlags;
        BinAddress aFirstPos;
        rStrm >> nFlags >> aFirstPos;

        rData.maFirstPos     = getAddressConverter().createValidCellAddress( aFirstPos, getSheetIndex(), false );
        rData.mnViewType     = getFlagValue( nFlags, BIFF_WINDOW2_PAGEBREAKMODE, XML_pageBreakPreview, XML_normal );
        rData.mnPaneState    = getFlagValue( nFlags, BIFF_WINDOW2_FROZEN, getFlagValue( nFlags, BIFF_WINDOW2_FROZENNOSPLIT, XML_frozen, XML_frozenSplit ), XML_split );
        rData.mbSelected     = getFlag( nFlags, BIFF_WINDOW2_SELECTED );
        rData.mbRightToLeft  = getFlag( nFlags, BIFF_WINDOW2_RIGHTTOLEFT );
        rData.mbDefGridColor = getFlag( nFlags, BIFF_WINDOW2_DEFGRIDCOLOR );
        rData.mbShowFormulas = getFlag( nFlags, BIFF_WINDOW2_SHOWFORMULAS );
        rData.mbShowGrid     = getFlag( nFlags, BIFF_WINDOW2_SHOWGRID );
        rData.mbShowHeadings = getFlag( nFlags, BIFF_WINDOW2_SHOWHEADINGS );
        rData.mbShowZeros    = getFlag( nFlags, BIFF_WINDOW2_SHOWZEROS );
        rData.mbShowOutline  = getFlag( nFlags, BIFF_WINDOW2_SHOWOUTLINE );

        if( getBiff() == BIFF8 )
        {
            rData.maGridColor.importColorId( rStrm );
            // zoom data not included in chart sheets
            if( (getSheetType() != SHEETTYPE_CHARTSHEET) && (rStrm.getRecLeft() >= 6) )
            {
                rStrm.skip( 2 );
                sal_uInt16 nPageZoom, nNormalZoom;
                rStrm >> nPageZoom >> nNormalZoom;
                rData.mnSheetLayoutZoom = nPageZoom;
                rData.mnNormalZoom = nNormalZoom;
            }
        }
        else
        {
            rData.maGridColor.importColorRgb( rStrm );
        }
    }
}

void SheetViewSettings::importPane( BiffInputStream& rStrm )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importPane - missing leading WINDOW2 record" );
    if( !maSheetDatas.empty() )
    {
        sal_uInt8 nActivePaneId;
        sal_uInt16 nSplitX, nSplitY;
        BinAddress aSecondPos;
        rStrm >> nSplitX >> nSplitY >> aSecondPos >> nActivePaneId;

        OoxSheetViewData& rData = *maSheetDatas.back();
        rData.mfSplitX = nSplitX;
        rData.mfSplitY = nSplitY;
        rData.maSecondPos = getAddressConverter().createValidCellAddress( aSecondPos, getSheetIndex(), false );
        rData.mnActivePaneId = lclGetOoxPaneId( nActivePaneId, XML_topLeft );
    }
}

void SheetViewSettings::importScl( BiffInputStream& rStrm )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importScl - missing leading WINDOW2 record" );
    if( !maSheetDatas.empty() )
    {
        sal_uInt16 nNum, nDenom;
        rStrm >> nNum >> nDenom;
        OSL_ENSURE( nDenom > 0, "SheetViewSettings::importScl - invalid denominator" );
        if( nDenom > 0 )
            maSheetDatas.back()->mnCurrentZoom = getLimitedValue< sal_Int32, sal_uInt16 >( (nNum * 100) / nDenom, 10, 400 );
    }
}

void SheetViewSettings::importSelection( BiffInputStream& rStrm )
{
    OSL_ENSURE( !maSheetDatas.empty(), "SheetViewSettings::importPane - missing leading WINDOW2 record" );
    if( !maSheetDatas.empty() )
    {
        // pane this selection belongs to
        sal_uInt8 nPaneId = rStrm.readuInt8();
        OoxSheetSelectionData& rSelData = maSheetDatas.back()->createSelectionData( lclGetOoxPaneId( nPaneId, -1 ) );
        // cursor position
        BinAddress aActiveCell;
        sal_uInt16 nActiveCellId;
        rStrm >> aActiveCell >> nActiveCellId;
        rSelData.maActiveCell = getAddressConverter().createValidCellAddress( aActiveCell, getSheetIndex(), false );
        rSelData.mnActiveCellId = nActiveCellId;
        // selection
        rSelData.maSelection.clear();
        BinRangeList aSelection;
        aSelection.read( rStrm, false );
        getAddressConverter().convertToCellRangeList( rSelData.maSelection, aSelection, getSheetIndex(), false );
    }
}

void SheetViewSettings::finalizeImport()
{
    // special handling for chart sheets
    bool bChartSheet = getSheetType() == SHEETTYPE_CHARTSHEET;

    // force creation of sheet view data to get the Excel defaults
    OoxSheetViewDataRef xData = maSheetDatas.empty() ? createSheetViewData() : maSheetDatas.front();

    // mirrored sheet (this is not a view setting in Calc)
    // #i59590# real life: Excel ignores mirror flag in chart sheets
    if( !bChartSheet && xData->mbRightToLeft )
    {
        PropertySet aPropSet( getXSpreadsheet() );
        aPropSet.setProperty( CREATE_OUSTRING( "TableLayout" ), ::com::sun::star::text::WritingMode2::RL_TB );
    }

    // sheet selected (active sheet must be selected)
    bool bSelected = xData->mbSelected || (getSheetIndex() == getViewSettings().getActiveSheetIndex());

    // visible area and current cursor position (selection not supported via API)
    CellAddress aFirstPos( getSheetIndex(), 0, 0 );
    CellAddress aCursor( getSheetIndex(), 0, 0 );
    if( !bChartSheet )
    {
        aFirstPos = xData->maFirstPos;
        const OoxSheetSelectionData* pSelData = xData->getActiveSelectionData();
        aCursor = pSelData ? pSelData->maActiveCell : xData->maFirstPos;
    }

    // freeze/split position default
    sal_Int16 nHSplitMode = API_SPLITMODE_NONE;
    sal_Int16 nVSplitMode = API_SPLITMODE_NONE;
    sal_Int32 nHSplitPos = 0;
    sal_Int32 nVSplitPos = 0;
    // active pane default
    sal_Int16 nActivePane = API_SPLITPANE_BOTTOMLEFT;

    if( !bChartSheet )
    {
        // freeze/split position
        if( (xData->mnPaneState == XML_frozen) || (xData->mnPaneState == XML_frozenSplit) )
        {
            /*  Frozen panes: handle split position as row/column positions.
                #i35812# Excel uses number of visible rows/columns in the
                    frozen area (rows/columns scolled outside are not incuded),
                    Calc uses absolute position of first unfrozen row/column. */
            const CellAddress& rMaxApiPos = getAddressConverter().getMaxApiAddress();
            if( (xData->mfSplitX >= 1.0) && (xData->maFirstPos.Column + xData->mfSplitX <= rMaxApiPos.Column) )
                nHSplitPos = static_cast< sal_Int32 >( xData->maFirstPos.Column + xData->mfSplitX );
            nHSplitMode = (nHSplitPos > 0) ? API_SPLITMODE_FREEZE : API_SPLITMODE_NONE;
            if( (xData->mfSplitY >= 1.0) && (xData->maFirstPos.Row + xData->mfSplitY <= rMaxApiPos.Row) )
                nVSplitPos = static_cast< sal_Int32 >( xData->maFirstPos.Row + xData->mfSplitY );
            nVSplitMode = (nVSplitPos > 0) ? API_SPLITMODE_FREEZE : API_SPLITMODE_NONE;
        }
        else if( xData->mnPaneState == XML_split )
        {
            // split window: view settings API uses twips...
            nHSplitPos = getLimitedValue< sal_Int32, double >( xData->mfSplitX + 0.5, 0, SAL_MAX_INT32 );
            nHSplitMode = (nHSplitPos > 0) ? API_SPLITMODE_SPLIT : API_SPLITMODE_NONE;
            nVSplitPos = getLimitedValue< sal_Int32, double >( xData->mfSplitY + 0.5, 0, SAL_MAX_INT32 );
            nVSplitMode = (nVSplitPos > 0) ? API_SPLITMODE_SPLIT : API_SPLITMODE_NONE;
        }

        // active pane
        switch( xData->mnActivePaneId )
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
    }

    // automatic grid color
    if( bChartSheet || xData->mbDefGridColor )
        xData->maGridColor.set( XML_auto, 0 );

    // write the sheet view settings into the property sequence
    PropertySequence aSheetProps( sppcSheetNames, sppcGlobalSheetNames );
    aSheetProps
        << bSelected
        << aCursor.Column
        << aCursor.Row
        << nHSplitMode
        << nVSplitMode
        << nHSplitPos
        << nVSplitPos
        << nActivePane
        << aFirstPos.Column
        << aFirstPos.Row
        << xData->maSecondPos.Column
        << ((nVSplitPos > 0) ? xData->maSecondPos.Row : xData->maFirstPos.Row)
        << getStyles().getColor( xData->maGridColor, API_RGB_TRANSPARENT )
        << API_ZOOMTYPE_PERCENT
        << static_cast< sal_Int16 >( xData->getNormalZoom() )
        << static_cast< sal_Int16 >( xData->getPageBreakZoom() )
        << (!bChartSheet && xData->isPageBreakPreview())
        << xData->mbShowFormulas
        << xData->mbShowGrid
        << xData->mbShowHeadings
        << xData->mbShowZeros
        << xData->mbShowOutline;

    // store sheet view settings in global view settings object
    getViewSettings().setSheetViewSettings( getSheetIndex(), xData, Any( aSheetProps.createPropertySequence() ) );
}

// private --------------------------------------------------------------------

OoxSheetViewDataRef SheetViewSettings::createSheetViewData()
{
    OoxSheetViewDataRef xData( new OoxSheetViewData );
    maSheetDatas.push_back( xData );
    return xData;
}

// ============================================================================

OoxWorkbookViewData::OoxWorkbookViewData() :
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
    OoxWorkbookViewData& rData = createWorkbookViewData();
    rData.mnWinX          = rAttribs.getInteger( XML_xWindow, 0 );
    rData.mnWinY          = rAttribs.getInteger( XML_yWindow, 0 );
    rData.mnWinWidth      = rAttribs.getInteger( XML_windowWidth, 0 );
    rData.mnWinHeight     = rAttribs.getInteger( XML_windowHeight, 0 );
    rData.mnActiveSheet   = rAttribs.getInteger( XML_activeTab, 0 );
    rData.mnFirstVisSheet = rAttribs.getInteger( XML_firstSheet, 0 );
    rData.mnTabBarWidth   = rAttribs.getInteger( XML_tabRatio, 600 );
    rData.mnVisibility    = rAttribs.getToken( XML_visibility, XML_visible );
    rData.mbShowTabBar    = rAttribs.getBool( XML_showSheetTabs, true );
    rData.mbShowHorScroll = rAttribs.getBool( XML_showHorizontalScroll, true );
    rData.mbShowVerScroll = rAttribs.getBool( XML_showVerticalScroll, true );
    rData.mbMinimized     = rAttribs.getBool( XML_minimized, false );
}

void ViewSettings::importWorkbookView( RecordInputStream& rStrm )
{
    OoxWorkbookViewData& rData = createWorkbookViewData();
    sal_uInt8 nFlags;
    rStrm >> rData.mnWinX >> rData.mnWinY >> rData.mnWinWidth >> rData.mnWinHeight >> rData.mnTabBarWidth >> rData.mnFirstVisSheet >> rData.mnActiveSheet >> nFlags;
    rData.mnVisibility    = getFlagValue( nFlags, OOBIN_WBVIEW_HIDDEN, XML_hidden, XML_visible );
    rData.mbShowTabBar    = getFlag( nFlags, OOBIN_WBVIEW_SHOWTABBAR );
    rData.mbShowHorScroll = getFlag( nFlags, OOBIN_WBVIEW_SHOWHORSCROLL );
    rData.mbShowVerScroll = getFlag( nFlags, OOBIN_WBVIEW_SHOWVERSCROLL );
    rData.mbMinimized     = getFlag( nFlags, OOBIN_WBVIEW_MINIMIZED );
}

void ViewSettings::importWindow1( BiffInputStream& rStrm )
{
    sal_uInt16 nWinX, nWinY, nWinWidth, nWinHeight;
    rStrm >> nWinX >> nWinY >> nWinWidth >> nWinHeight;

    // WINDOW1 record occures in every sheet in BIFF4W
    OSL_ENSURE( maBookDatas.empty() || ((getBiff() == BIFF4) && isWorkbookFile()),
        "ViewSettings::importWindow1 - multiple WINDOW1 records" );
    OoxWorkbookViewData& rData = createWorkbookViewData();
    rData.mnWinX = nWinX;
    rData.mnWinY = nWinY;
    rData.mnWinWidth = nWinWidth;
    rData.mnWinHeight = nWinHeight;

    if( getBiff() <= BIFF4 )
    {
        sal_uInt8 nHidden;
        rStrm >> nHidden;
        rData.mnVisibility = (nHidden == 0) ? XML_visible : XML_hidden;
    }
    else
    {
        sal_uInt16 nFlags, nActiveTab, nFirstVisTab, nSelectCnt, nTabBarWidth;
        rStrm >> nFlags >> nActiveTab >> nFirstVisTab >> nSelectCnt >> nTabBarWidth;

        rData.mnActiveSheet = nActiveTab;
        rData.mnFirstVisSheet = nFirstVisTab;
        rData.mnTabBarWidth = nTabBarWidth;
        rData.mnVisibility = getFlagValue( nFlags, BIFF_WINDOW1_HIDDEN, XML_hidden, XML_visible );
        rData.mbMinimized = getFlag( nFlags, BIFF_WINDOW1_MINIMIZED );
        rData.mbShowHorScroll = getFlag( nFlags, BIFF_WINDOW1_SHOWHORSCROLL );
        rData.mbShowVerScroll = getFlag( nFlags, BIFF_WINDOW1_SHOWVERSCROLL );
        rData.mbShowTabBar = getFlag( nFlags, BIFF_WINDOW1_SHOWTABBAR );
    }
}

void ViewSettings::setSheetViewSettings( sal_Int32 nSheet, const OoxSheetViewDataRef& rxViewData, const Any& rProperties )
{
    maSheetDatas[ nSheet ] = rxViewData;
    maSheetProps[ nSheet ] = rProperties;
}

void ViewSettings::finalizeImport()
{
    const WorksheetBuffer& rWorksheets = getWorksheets();
    if( rWorksheets.getInternalSheetCount() <= 0 ) return;

    // force creation of workbook view data to get the Excel defaults
    const OoxWorkbookViewData& rData = maBookDatas.empty() ? createWorkbookViewData() : *maBookDatas.front();

    // show object mode is part of workbook settings
    sal_Int16 nShowMode = getWorkbookSettings().getApiShowObjectMode();

    // view settings for all sheets
    Reference< XNameContainer > xSheetsNC = ContainerHelper::createNameContainer();
    if( !xSheetsNC.is() ) return;
    for( SheetPropertiesMap::const_iterator aIt = maSheetProps.begin(), aEnd = maSheetProps.end(); aIt != aEnd; ++aIt )
        ContainerHelper::insertByName( xSheetsNC, rWorksheets.getFinalSheetName( aIt->first ), aIt->second );

    // use data of active sheet to set sheet properties that are document-global in Calc
    sal_Int32 nActiveSheet = getActiveSheetIndex();
    OoxSheetViewDataRef& rxActiveSheetData = maSheetDatas[ nActiveSheet ];
    OSL_ENSURE( rxActiveSheetData.get(), "ViewSettings::finalizeImport - missing active sheet view settings" );
    if( !rxActiveSheetData )
        rxActiveSheetData.reset( new OoxSheetViewData );

    PropertySequence aDocProps( sppcDocNames, sppcGlobalSheetNames );
    aDocProps
        << xSheetsNC
        << rWorksheets.getFinalSheetName( nActiveSheet )
        << rData.mbShowHorScroll
        << rData.mbShowVerScroll
        << rData.mbShowTabBar
        << double( rData.mnTabBarWidth / 1000.0 )
        << nShowMode << nShowMode << nShowMode
        << getStyles().getColor( rxActiveSheetData->maGridColor, API_RGB_TRANSPARENT )
        << API_ZOOMTYPE_PERCENT
        << static_cast< sal_Int16 >( rxActiveSheetData->getNormalZoom() )
        << static_cast< sal_Int16 >( rxActiveSheetData->getPageBreakZoom() )
        << rxActiveSheetData->isPageBreakPreview()
        << rxActiveSheetData->mbShowFormulas
        << rxActiveSheetData->mbShowGrid
        << rxActiveSheetData->mbShowHeadings
        << rxActiveSheetData->mbShowZeros
        << rxActiveSheetData->mbShowOutline;

    Reference< XIndexContainer > xContainer = ContainerHelper::createIndexContainer();
    if( xContainer.is() ) try
    {
        xContainer->insertByIndex( 0, Any( aDocProps.createPropertySequence() ) );
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
    sal_Int32 nSheetCount = getLimitedValue< sal_Int32, sal_Int32 >( getWorksheets().getInternalSheetCount(), 1, SAL_MAX_INT32 );
    return maBookDatas.empty() ? 0 : getLimitedValue< sal_Int32, sal_Int32 >( maBookDatas.front()->mnActiveSheet, 0, nSheetCount - 1 );
}

// private --------------------------------------------------------------------

OoxWorkbookViewData& ViewSettings::createWorkbookViewData()
{
    OoxWorkbookViewDataRef xData( new OoxWorkbookViewData );
    maBookDatas.push_back( xData );
    return *xData;
}

// ============================================================================

} // namespace xls
} // namespace oox

