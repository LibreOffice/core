/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewsettings.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:09:30 $
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

#ifndef OOX_XLS_VIEWSETTINGS_HXX
#define OOX_XLS_VIEWSETTINGS_HXX

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Contains all settings for a selection in a single pane of a sheet. */
struct OoxSheetSelectionData
{
    ::com::sun::star::table::CellAddress maActiveCell;  /// Position of active cell (cursor).
    ApiCellRangeList    maSelection;                    /// Selected cell ranges.
    sal_Int32           mnActiveCellId;                 /// Index of active cell in selection list.

    explicit            OoxSheetSelectionData();
};

// ----------------------------------------------------------------------------

/** Contains all view settings for a single sheet. */
struct OoxSheetViewData
{
    typedef RefMap< sal_Int32, OoxSheetSelectionData > OoxSelectionDataMap;

    OoxSelectionDataMap maSelMap;                       /// Selections of all panes.
    OoxColor            maGridColor;                    /// Grid color.
    ::com::sun::star::table::CellAddress maFirstPos;    /// First visible cell.
    ::com::sun::star::table::CellAddress maSecondPos;   /// First visible cell in additional panes.
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

    explicit            OoxSheetViewData();

    /** Returns true, if page break preview is active. */
    bool                isPageBreakPreview() const;
    /** Returns the zoom in normal view (returns default, if current value is 0). */
    sal_Int32           getNormalZoom() const;
    /** Returns the zoom in pagebreak preview (returns default, if current value is 0). */
    sal_Int32           getPageBreakZoom() const;

    /** Returns the selection data, if available, otherwise 0. */
    const OoxSheetSelectionData* getSelectionData( sal_Int32 nPaneId ) const;
    /** Returns the selection data of the active pane. */
    const OoxSheetSelectionData* getActiveSelectionData() const;
    /** Returns read/write access to the selection data of the specified pane. */
    OoxSheetSelectionData& createSelectionData( sal_Int32 nPaneId );
};

typedef ::boost::shared_ptr< OoxSheetViewData > OoxSheetViewDataRef;

// ----------------------------------------------------------------------------

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
    void                importSheetView( RecordInputStream& rStrm );
    /** Imports the PANE record containing sheet pane settings. */
    void                importPane( RecordInputStream& rStrm );
    /** Imports the SELECTION record containing selection settings for a pane. */
    void                importSelection( RecordInputStream& rStrm );
    /** Imports the CHARTSHEETVIEW record containing view settings of a chart sheet. */
    void                importChartSheetView( RecordInputStream& rStrm );

    /** Imports the WINDOW2 record containing sheet view settings. */
    void                importWindow2( BiffInputStream& rStrm );
    /** Imports the PANE record containing sheet pane settings. */
    void                importPane( BiffInputStream& rStrm );
    /** Imports the SCL record containing sheet zoom settings. */
    void                importScl( BiffInputStream& rStrm );
    /** Imports the SELECTION record containing selection settings for a pane. */
    void                importSelection( BiffInputStream& rStrm );

    /** Converts all imported sheet view settings. */
    void                finalizeImport();

private:
    OoxSheetViewDataRef createSheetViewData();

private:
    typedef RefVector< OoxSheetViewData > SheetViewDataVec;
    SheetViewDataVec    maSheetDatas;
};

// ============================================================================

/** Contains all view settings for the entire document. */
struct OoxWorkbookViewData
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

    explicit            OoxWorkbookViewData();
};

typedef ::boost::shared_ptr< OoxWorkbookViewData > OoxWorkbookViewDataRef;

// ----------------------------------------------------------------------------

class ViewSettings : public WorkbookHelper
{
public:
    explicit            ViewSettings( const WorkbookHelper& rHelper );

    /** Imports the workbookView element containing workbook view settings. */
    void                importWorkbookView( const AttributeList& rAttribs );
    /** Imports the WORKBOOKVIEW record containing workbook view settings. */
    void                importWorkbookView( RecordInputStream& rStrm );
    /** Imports the WINDOW1 record containing workbook view settings. */
    void                importWindow1( BiffInputStream& rStrm );

    /** Stores converted view settings for a specific worksheet. */
    void                setSheetViewSettings( sal_Int32 nSheet,
                            const OoxSheetViewDataRef& rxViewData,
                            const ::com::sun::star::uno::Any& rProperties );

    /** Converts all imported document view settings. */
    void                finalizeImport();

    /** Returns the index of the active sheet. */
    sal_Int32           getActiveSheetIndex() const;

private:
    OoxWorkbookViewData& createWorkbookViewData();

private:
    typedef RefVector< OoxWorkbookViewData >                    WorkbookViewDataVec;
    typedef RefMap< sal_Int32, OoxSheetViewData >               SheetViewDataMap;
    typedef ::std::map< sal_Int32, ::com::sun::star::uno::Any > SheetPropertiesMap;

    WorkbookViewDataVec maBookDatas;        /// Workbook view datas.
    SheetViewDataMap    maSheetDatas;       /// Active view data for each sheet.
    SheetPropertiesMap  maSheetProps;       /// Converted property sequences for each sheet.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

