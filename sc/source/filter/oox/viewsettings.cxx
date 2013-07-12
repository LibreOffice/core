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

#include "viewsettings.hxx"

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <comphelper/mediadescriptor.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "unitconverter.hxx"
#include "workbooksettings.hxx"
#include "worksheetbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::oox::core::FilterBase;

// ============================================================================

namespace {

const sal_Int32 OOX_BOOKVIEW_TABBARRATIO_DEF        = 600;      /// Default tabbar ratio.
const sal_Int32 OOX_SHEETVIEW_NORMALZOOM_DEF        = 100;      /// Default zoom for normal view.
const sal_Int32 OOX_SHEETVIEW_SHEETLAYZOOM_DEF      = 60;       /// Default zoom for pagebreak preview.
const sal_Int32 OOX_SHEETVIEW_PAGELAYZOOM_DEF       = 100;      /// Default zoom for page layout view.

const sal_uInt8 BIFF12_PANE_FROZEN                  = 0x01;
const sal_uInt8 BIFF12_PANE_FROZENNOSPLIT           = 0x02;

const sal_uInt16 BIFF12_SHEETVIEW_WINPROTECTED      = 0x0001;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWFORMULAS      = 0x0002;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWGRID          = 0x0004;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWHEADINGS      = 0x0008;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWZEROS         = 0x0010;
const sal_uInt16 BIFF12_SHEETVIEW_RIGHTTOLEFT       = 0x0020;
const sal_uInt16 BIFF12_SHEETVIEW_SELECTED          = 0x0040;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWRULER         = 0x0080;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWOUTLINE       = 0x0100;
const sal_uInt16 BIFF12_SHEETVIEW_DEFGRIDCOLOR      = 0x0200;
const sal_uInt16 BIFF12_SHEETVIEW_SHOWWHITESPACE    = 0x0400;

const sal_uInt16 BIFF12_CHARTSHEETVIEW_SELECTED     = 0x0001;
const sal_uInt16 BIFF12_CHARTSHEETVIEW_ZOOMTOFIT    = 0x0002;

const sal_uInt8 BIFF12_WBVIEW_HIDDEN                = 0x01;
const sal_uInt8 BIFF12_WBVIEW_MINIMIZED             = 0x02;
const sal_uInt8 BIFF12_WBVIEW_SHOWHORSCROLL         = 0x08;
const sal_uInt8 BIFF12_WBVIEW_SHOWVERSCROLL         = 0x10;
const sal_uInt8 BIFF12_WBVIEW_SHOWTABBAR            = 0x20;
const sal_uInt8 BIFF12_WBVIEW_AUTOFILTERGROUP       = 0x40;

const sal_uInt8 BIFF_PANE_BOTTOMRIGHT               = 0;        /// Bottom-right pane.
const sal_uInt8 BIFF_PANE_TOPRIGHT                  = 1;        /// Right, or top-right pane.
const sal_uInt8 BIFF_PANE_BOTTOMLEFT                = 2;        /// Bottom, or bottom-left pane.
const sal_uInt8 BIFF_PANE_TOPLEFT                   = 3;        /// Single, top, left, or top-left pane.

const sal_uInt16 BIFF_WINDOW1_HIDDEN                = 0x0001;
const sal_uInt16 BIFF_WINDOW1_MINIMIZED             = 0x0002;
const sal_uInt16 BIFF_WINDOW1_SHOWHORSCROLL         = 0x0008;
const sal_uInt16 BIFF_WINDOW1_SHOWVERSCROLL         = 0x0010;
const sal_uInt16 BIFF_WINDOW1_SHOWTABBAR            = 0x0020;

const sal_uInt16 BIFF_WINDOW2_SHOWFORMULAS          = 0x0001;
const sal_uInt16 BIFF_WINDOW2_SHOWGRID              = 0x0002;
const sal_uInt16 BIFF_WINDOW2_SHOWHEADINGS          = 0x0004;
const sal_uInt16 BIFF_WINDOW2_FROZEN                = 0x0008;
const sal_uInt16 BIFF_WINDOW2_SHOWZEROS             = 0x0010;
const sal_uInt16 BIFF_WINDOW2_DEFGRIDCOLOR          = 0x0020;
const sal_uInt16 BIFF_WINDOW2_RIGHTTOLEFT           = 0x0040;
const sal_uInt16 BIFF_WINDOW2_SHOWOUTLINE           = 0x0080;
const sal_uInt16 BIFF_WINDOW2_FROZENNOSPLIT         = 0x0100;
const sal_uInt16 BIFF_WINDOW2_SELECTED              = 0x0200;
const sal_uInt16 BIFF_WINDOW2_DISPLAYED             = 0x0400;
const sal_uInt16 BIFF_WINDOW2_PAGEBREAKMODE         = 0x0800;

// Attention: view settings in Calc do not use com.sun.star.view.DocumentZoomType!
const sal_Int16 API_ZOOMTYPE_PERCENT                = 0;        /// Zoom value in percent.

const sal_Int32 API_ZOOMVALUE_MIN                   = 20;       /// Minimum zoom in Calc.
const sal_Int32 API_ZOOMVALUE_MAX                   = 400;      /// Maximum zoom in Calc.

// no predefined constants for split mode
const sal_Int16 API_SPLITMODE_NONE                  = 0;        /// No splits in window.
const sal_Int16 API_SPLITMODE_SPLIT                 = 1;        /// Window is split.
const sal_Int16 API_SPLITMODE_FREEZE                = 2;        /// Window has frozen panes.

// no predefined constants for pane idetifiers
const sal_Int16 API_SPLITPANE_TOPLEFT               = 0;        /// Top-left, or top pane.
const sal_Int16 API_SPLITPANE_TOPRIGHT              = 1;        /// Top-right pane.
const sal_Int16 API_SPLITPANE_BOTTOMLEFT            = 2;        /// Bottom-left, bottom, left, or single pane.
const sal_Int16 API_SPLITPANE_BOTTOMRIGHT           = 3;        /// Bottom-right, or right pane.

// ----------------------------------------------------------------------------

/** Returns the OOXML pane identifier from the passed BIFF pane id. */
sal_Int32 lclGetOoxPaneId( sal_Int32 nBiffPaneId, sal_Int32 nDefaultPaneId )
{
    static const sal_Int32 spnPaneIds[] = { XML_bottomRight, XML_topRight, XML_bottomLeft, XML_topLeft };
    return STATIC_ARRAY_SELECT( spnPaneIds, nBiffPaneId, nDefaultPaneId );
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

sal_Int32 SheetViewModel::getGridColor( const FilterBase& rFilter ) const
{
    return mbDefGridColor ? API_RGB_TRANSPARENT : maGridColor.getColor( rFilter.getGraphicHelper() );
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

void SheetViewSettings::importSheetView( SequenceInputStream& rStrm )
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
    rModel.mbSelected     = getFlag( nFlags, BIFF12_SHEETVIEW_SELECTED );
    rModel.mbRightToLeft  = getFlag( nFlags, BIFF12_SHEETVIEW_RIGHTTOLEFT );
    rModel.mbDefGridColor = getFlag( nFlags, BIFF12_SHEETVIEW_DEFGRIDCOLOR );
    rModel.mbShowFormulas = getFlag( nFlags, BIFF12_SHEETVIEW_SHOWFORMULAS );
    rModel.mbShowGrid     = getFlag( nFlags, BIFF12_SHEETVIEW_SHOWGRID );
    rModel.mbShowHeadings = getFlag( nFlags, BIFF12_SHEETVIEW_SHOWHEADINGS );
    rModel.mbShowZeros    = getFlag( nFlags, BIFF12_SHEETVIEW_SHOWZEROS );
    rModel.mbShowOutline  = getFlag( nFlags, BIFF12_SHEETVIEW_SHOWOUTLINE );
}

void SheetViewSettings::importPane( SequenceInputStream& rStrm )
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
        rModel.mnPaneState    = getFlagValue( nFlags, BIFF12_PANE_FROZEN, getFlagValue( nFlags, BIFF12_PANE_FROZENNOSPLIT, XML_frozen, XML_frozenSplit ), XML_split );
    }
}

void SheetViewSettings::importSelection( SequenceInputStream& rStrm )
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

void SheetViewSettings::importChartSheetView( SequenceInputStream& rStrm )
{
    SheetViewModel& rModel = *createSheetView();
    sal_uInt16 nFlags;
    rStrm >> nFlags >> rModel.mnCurrentZoom >> rModel.mnWorkbookViewId;

    rModel.mbSelected  = getFlag( nFlags, BIFF12_CHARTSHEETVIEW_SELECTED );
    rModel.mbZoomToFit = getFlag( nFlags, BIFF12_CHARTSHEETVIEW_ZOOMTOFIT );
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

    // sheet selected (active sheet must be selected)
    bool bSelected = xModel->mbSelected || (getSheetIndex() == getViewSettings().getActiveCalcSheet());
    if ( bSelected )
    {
        // active tab/sheet cannot be hidden
        // always force it to be displayed
        PropertySet aPropSet( getSheet() );
        aPropSet.setProperty( PROP_IsVisible, sal_True );
    }
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
    aPropMap[ PROP_GridColor ]                    <<= xModel->getGridColor( getBaseFilter() );
    aPropMap[ PROP_ShowPageBreakPreview ]         <<= xModel->isPageBreakPreview();
    aPropMap[ PROP_ShowFormulas ]                 <<= xModel->mbShowFormulas;
    aPropMap[ PROP_ShowGrid ]                     <<= xModel->mbShowGrid;
    aPropMap[ PROP_HasColumnRowHeaders ]          <<= xModel->mbShowHeadings;
    aPropMap[ PROP_ShowZeroValues ]               <<= xModel->mbShowZeros;
    aPropMap[ PROP_IsOutlineSymbolsSet ]          <<= xModel->mbShowOutline;

    // store sheet view settings in global view settings object
    getViewSettings().setSheetViewSettings( getSheetIndex(), xModel, Any( aPropMap.makePropertyValueSequence() ) );
}

bool SheetViewSettings::isSheetRightToLeft() const
{
    return !maSheetViews.empty() && maSheetViews.front()->mbRightToLeft;
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
    WorkbookHelper( rHelper ),
    mbValidOleSize( false )
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

void ViewSettings::importOleSize( const AttributeList& rAttribs )
{
    OUString aRange = rAttribs.getString( XML_ref, OUString() );
    mbValidOleSize = getAddressConverter().convertToCellRange( maOleSize, aRange, 0, true, false );
}

void ViewSettings::importWorkbookView( SequenceInputStream& rStrm )
{
    WorkbookViewModel& rModel = createWorkbookView();
    sal_uInt8 nFlags;
    rStrm >> rModel.mnWinX >> rModel.mnWinY >> rModel.mnWinWidth >> rModel.mnWinHeight >> rModel.mnTabBarWidth >> rModel.mnFirstVisSheet >> rModel.mnActiveSheet >> nFlags;
    rModel.mnVisibility    = getFlagValue( nFlags, BIFF12_WBVIEW_HIDDEN, XML_hidden, XML_visible );
    rModel.mbShowTabBar    = getFlag( nFlags, BIFF12_WBVIEW_SHOWTABBAR );
    rModel.mbShowHorScroll = getFlag( nFlags, BIFF12_WBVIEW_SHOWHORSCROLL );
    rModel.mbShowVerScroll = getFlag( nFlags, BIFF12_WBVIEW_SHOWVERSCROLL );
    rModel.mbMinimized     = getFlag( nFlags, BIFF12_WBVIEW_MINIMIZED );
}

void ViewSettings::importOleSize( SequenceInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    mbValidOleSize = getAddressConverter().convertToCellRange( maOleSize, aBinRange, 0, true, false );
}

void ViewSettings::setSheetViewSettings( sal_Int16 nSheet, const SheetViewModelRef& rxSheetView, const Any& rProperties )
{
    maSheetViews[ nSheet ] = rxSheetView;
    maSheetProps[ nSheet ] = rProperties;
}

void ViewSettings::setSheetUsedArea( const CellRangeAddress& rUsedArea )
{
    maSheetUsedAreas[ rUsedArea.Sheet ] = rUsedArea;
}

void ViewSettings::finalizeImport()
{
    const WorksheetBuffer& rWorksheets = getWorksheets();
    if( rWorksheets.getWorksheetCount() <= 0 ) return;

    // force creation of workbook view model to get the Excel defaults
    const WorkbookViewModel& rModel = maBookViews.empty() ? createWorkbookView() : *maBookViews.front();

    // show object mode is part of workbook settings
    sal_Int16 nShowMode = getWorkbookSettings().getApiShowObjectMode();

    // view settings for all sheets
    Reference< XNameContainer > xSheetsNC = NamedPropertyValues::create( getBaseFilter().getComponentContext() );
    if( !xSheetsNC.is() ) return;
    for( SheetPropertiesMap::const_iterator aIt = maSheetProps.begin(), aEnd = maSheetProps.end(); aIt != aEnd; ++aIt )
        ContainerHelper::insertByName( xSheetsNC, rWorksheets.getCalcSheetName( aIt->first ), aIt->second );

    // use active sheet to set sheet properties that are document-global in Calc
    sal_Int16 nActiveSheet = getActiveCalcSheet();
    SheetViewModelRef& rxActiveSheetView = maSheetViews[ nActiveSheet ];
    OSL_ENSURE( rxActiveSheetView.get(), "ViewSettings::finalizeImport - missing active sheet view settings" );
    if( !rxActiveSheetView )
        rxActiveSheetView.reset( new SheetViewModel );

    Reference< XIndexContainer > xContainer = IndexedPropertyValues::create( getBaseFilter().getComponentContext() );
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
        aPropMap[ PROP_GridColor ]                     <<= rxActiveSheetView->getGridColor( getBaseFilter() );
        aPropMap[ PROP_ShowPageBreakPreview ]          <<= rxActiveSheetView->isPageBreakPreview();
        aPropMap[ PROP_ShowFormulas ]                  <<= rxActiveSheetView->mbShowFormulas;
        aPropMap[ PROP_ShowGrid ]                      <<= rxActiveSheetView->mbShowGrid;
        aPropMap[ PROP_HasColumnRowHeaders ]           <<= rxActiveSheetView->mbShowHeadings;
        aPropMap[ PROP_ShowZeroValues ]                <<= rxActiveSheetView->mbShowZeros;
        aPropMap[ PROP_IsOutlineSymbolsSet ]           <<= rxActiveSheetView->mbShowOutline;

        xContainer->insertByIndex( 0, Any( aPropMap.makePropertyValueSequence() ) );
        Reference< XViewDataSupplier > xViewDataSuppl( getDocument(), UNO_QUERY_THROW );
        xViewDataSuppl->setViewData( xContainer );
    }
    catch( Exception& )
    {
        OSL_FAIL( "ViewSettings::finalizeImport - cannot create document view settings" );
    }

    /*  Set visible area to be used if this document is an embedded OLE object.
        #i44077# If a new OLE object is inserted from file, there is no OLESIZE
        record in the Excel file. In this case, use the used area calculated
        from file contents (used cells and drawing objects). */
    maOleSize.Sheet = nActiveSheet;
    const CellRangeAddress* pVisibleArea = mbValidOleSize ?
        &maOleSize : ContainerHelper::getMapElement( maSheetUsedAreas, nActiveSheet );
    if( pVisibleArea )
    {
        // calculate the visible area in units of 1/100 mm
        PropertySet aRangeProp( getCellRangeFromDoc( *pVisibleArea ) );
        css::awt::Point aPos;
        css::awt::Size aSize;
        if( aRangeProp.getProperty( aPos, PROP_Position ) && aRangeProp.getProperty( aSize, PROP_Size ) )
        {
            // set the visible area as sequence of long at the media descriptor
            Sequence< sal_Int32 > aWinExtent( 4 );
            aWinExtent[ 0 ] = aPos.X;
            aWinExtent[ 1 ] = aPos.Y;
            aWinExtent[ 2 ] = aPos.X + aSize.Width;
            aWinExtent[ 3 ] = aPos.Y + aSize.Height;
            getBaseFilter().getMediaDescriptor()[ "WinExtent" ] <<= aWinExtent;
        }
    }
}

sal_Int16 ViewSettings::getActiveCalcSheet() const
{
    return maBookViews.empty() ? 0 : ::std::max< sal_Int16 >( getWorksheets().getCalcSheetIndex( maBookViews.front()->mnActiveSheet ), 0 );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
