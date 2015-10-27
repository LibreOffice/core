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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_VIEWSETTINGS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_VIEWSETTINGS_HXX

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "addressconverter.hxx"
#include "stylesbuffer.hxx"
#include "worksheethelper.hxx"

namespace oox {
namespace xls {

/** Contains all settings for a selection in a single pane of a sheet. */
struct PaneSelectionModel
{
    css::table::CellAddress maActiveCell;  /// Position of active cell (cursor).
    ApiCellRangeList    maSelection;                    /// Selected cell ranges.
    sal_Int32           mnActiveCellId;                 /// Index of active cell in selection list.

    explicit            PaneSelectionModel();
};

/** Contains all view settings for a single sheet. */
struct SheetViewModel
{
    typedef RefMap< sal_Int32, PaneSelectionModel > PaneSelectionModelMap;

    PaneSelectionModelMap maPaneSelMap;                 /// Selections of all panes.
    Color               maGridColor;                    /// Grid color.
    css::table::CellAddress maFirstPos;    /// First visible cell.
    css::table::CellAddress maSecondPos;   /// First visible cell in additional panes.
    sal_Int32           mnWorkbookViewId;               /// Index into list of workbookView elements.
    sal_Int32           mnViewType;                     /// View type (normal, page break, layout).
    sal_Int32           mnActivePaneId;                 /// Active pane (with cell cursor).
    sal_Int32           mnPaneState;                    /// Pane state (frozen, split).
    double              mfSplitX;                       /// Split X position (twips), or number of frozen columns.
    double              mfSplitY;                       /// Split Y position (twips), or number of frozen rows.
    sal_Int32           mnCurrentZoom;                  /// Zoom factor for current view.
    sal_Int32           mnNormalZoom;                   /// Zoom factor for normal view.
    sal_Int32           mnSheetLayoutZoom;              /// Zoom factor for pagebreak preview.
    sal_Int32           mnPageLayoutZoom;               /// Zoom factor for page layout view.
    bool                mbSelected;                     /// True = sheet is selected.
    bool                mbRightToLeft;                  /// True = sheet in right-to-left mode.
    bool                mbDefGridColor;                 /// True = default grid color.
    bool                mbShowFormulas;                 /// True = show formulas instead of results.
    bool                mbShowGrid;                     /// True = show cell grid.
    bool                mbShowHeadings;                 /// True = show column/row headings.
    bool                mbShowZeros;                    /// True = show zero value zells.
    bool                mbShowOutline;                  /// True = show outlines.
    bool                mbZoomToFit;                    /// True = zoom chart sheet to fit window.

    explicit            SheetViewModel();

    /** Returns true, if page break preview is active. */
    bool                isPageBreakPreview() const;
    /** Returns the zoom in normal view (returns default, if current value is 0). */
    sal_Int32           getNormalZoom() const;
    /** Returns the zoom in pagebreak preview (returns default, if current value is 0). */
    sal_Int32           getPageBreakZoom() const;
    /** Returns the grid color as RGB value. */
    sal_Int32           getGridColor( const ::oox::core::FilterBase& rFilter ) const;

    /** Returns the selection data, if available, otherwise 0. */
    const PaneSelectionModel* getPaneSelection( sal_Int32 nPaneId ) const;
    /** Returns the selection data of the active pane. */
    const PaneSelectionModel* getActiveSelection() const;
    /** Returns read/write access to the selection data of the specified pane. */
    PaneSelectionModel& createPaneSelection( sal_Int32 nPaneId );
};

typedef std::shared_ptr< SheetViewModel > SheetViewModelRef;

class SheetViewSettings : public WorksheetHelper
{
public:
    explicit            SheetViewSettings( const WorksheetHelper& rHelper );

    /** Imports the sheetView element containing sheet view settings. */
    void                importSheetView( const AttributeList& rAttribs );
    /** Imports the pane element containing sheet pane settings. */
    void                importPane( const AttributeList& rAttribs );
    /** Imports the selection element containing selection settings for a pane. */
    void                importSelection( const AttributeList& rAttribs );
    /** Imports the sheetView element containing view settings of a chart sheet. */
    void                importChartSheetView( const AttributeList& rAttribs );

    /** Imports the SHEETVIEW record containing sheet view settings. */
    void                importSheetView( SequenceInputStream& rStrm );
    /** Imports the PANE record containing sheet pane settings. */
    void                importPane( SequenceInputStream& rStrm );
    /** Imports the SELECTION record containing selection settings for a pane. */
    void                importSelection( SequenceInputStream& rStrm );
    /** Imports the CHARTSHEETVIEW record containing view settings of a chart sheet. */
    void                importChartSheetView( SequenceInputStream& rStrm );

    /** Converts all imported sheet view settings. */
    void                finalizeImport();

    /** Returns true, if the sheet layout is set to right-to-left. */
    bool                isSheetRightToLeft() const;

private:
    SheetViewModelRef   createSheetView();

private:
    typedef RefVector< SheetViewModel > SheetViewModelVec;
    SheetViewModelVec   maSheetViews;
};

/** Contains all view settings for the entire document. */
struct WorkbookViewModel
{
    sal_Int32           mnWinX;             /// X position of the workbook window (twips).
    sal_Int32           mnWinY;             /// Y position of the workbook window (twips).
    sal_Int32           mnWinWidth;         /// Width of the workbook window (twips).
    sal_Int32           mnWinHeight;        /// Height of the workbook window (twips).
    sal_Int32           mnActiveSheet;      /// Displayed (active) sheet.
    sal_Int32           mnFirstVisSheet;    /// First visible sheet in sheet tabbar.
    sal_Int32           mnTabBarWidth;      /// Width of sheet tabbar (1/1000 of window width).
    sal_Int32           mnVisibility;       /// Visibility state of workbook window.
    bool                mbShowTabBar;       /// True = show sheet tabbar.
    bool                mbShowHorScroll;    /// True = show horizontal sheet scrollbars.
    bool                mbShowVerScroll;    /// True = show vertical sheet scrollbars.
    bool                mbMinimized;        /// True = workbook window is minimized.

    explicit            WorkbookViewModel();
};

typedef std::shared_ptr< WorkbookViewModel > WorkbookViewModelRef;

class ViewSettings : public WorkbookHelper
{
public:
    explicit            ViewSettings( const WorkbookHelper& rHelper );

    /** Imports the workbookView element containing workbook view settings. */
    void                importWorkbookView( const AttributeList& rAttribs );
    /** Imports the oleSize element containing the visible size of the workbook. */
    void                importOleSize( const AttributeList& rAttribs );
    /** Imports the WORKBOOKVIEW record containing workbook view settings. */
    void                importWorkbookView( SequenceInputStream& rStrm );
    /** Imports the OLESIZE record containing the visible size of the workbook. */
    void                importOleSize( SequenceInputStream& rStrm );

    /** Stores converted view settings for a specific worksheet. */
    void                setSheetViewSettings( sal_Int16 nSheet,
                            const SheetViewModelRef& rxSheetView,
                            const css::uno::Any& rProperties );
    /** Stores the used area for a specific worksheet. */
    void                setSheetUsedArea(
                            const css::table::CellRangeAddress& rUsedArea );

    /** Converts all imported document view settings. */
    void                finalizeImport();

    /** Returns the Calc index of the active sheet. */
    sal_Int16           getActiveCalcSheet() const;

private:
    WorkbookViewModel&  createWorkbookView();

private:
    typedef RefVector< WorkbookViewModel >                                      WorkbookViewModelVec;
    typedef RefMap< sal_Int16, SheetViewModel >                                 SheetViewModelMap;
    typedef ::std::map< sal_Int16, css::uno::Any >                 SheetPropertiesMap;
    typedef ::std::map< sal_Int16, css::table::CellRangeAddress >  SheetUsedAreaMap;

    WorkbookViewModelVec maBookViews;       /// Workbook view models.
    SheetViewModelMap   maSheetViews;       /// Active view model for each sheet.
    SheetPropertiesMap  maSheetProps;       /// Converted property sequences for each sheet.
    SheetUsedAreaMap    maSheetUsedAreas;   /// Used area (cell range) of every sheet.
    css::table::CellRangeAddress
                        maOleSize;          /// Visible area if this is an embedded OLE object.
    bool                mbValidOleSize;     /// True = imported OLE size is a valid cell range.
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
