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

#ifndef OOX_XLS_WORKSHEETHELPER_HXX
#define OOX_XLS_WORKSHEETHELPER_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/helper/progressbar.hxx"
#include "oox/ole/olehelper.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/formulabase.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Point; }
    namespace awt { struct Rectangle; }
    namespace awt { struct Size; }
    namespace drawing { class XDrawPage; }
    namespace sheet { class XSheetCellRanges; }
    namespace sheet { class XSpreadsheet; }
    namespace table { class XCell; }
    namespace table { class XCellRange; }
    namespace table { class XTableColumns; }
    namespace table { class XTableRows; }
} } }

namespace oox {
namespace xls {

class AutoFilterBuffer;
struct BinAddress;
struct BinRange;
class BiffSheetDrawing;
class BinRangeList;
class CommentsBuffer;
class CondFormatBuffer;
class Font;
class PageSettings;
class QueryTableBuffer;
class RichString;
class SheetDataBuffer;
class SheetViewSettings;
class VmlDrawing;
class WorksheetSettings;

// ============================================================================
// ============================================================================

/** An enumeration for all types of sheets in a workbook. */
enum WorksheetType
{
    SHEETTYPE_WORKSHEET,            /// Worksheet.
    SHEETTYPE_CHARTSHEET,           /// Chart sheet.
    SHEETTYPE_MACROSHEET,           /// Macro sheet.
    SHEETTYPE_DIALOGSHEET,          /// Dialog sheet (BIFF5+).
    SHEETTYPE_MODULESHEET,          /// VB module sheet (BIFF5 only).
    SHEETTYPE_EMPTYSHEET            /// Other (unsupported) sheet type.
};

// ============================================================================

/** Stores settings and formatting data about a range of sheet columns. */
struct ColumnModel
{
    ValueRange          maRange;            /// 1-based (!) range of the described columns.
    double              mfWidth;            /// Column width in number of characters.
    sal_Int32           mnXfId;             /// Column default formatting.
    sal_Int32           mnLevel;            /// Column outline level.
    bool                mbShowPhonetic;     /// True = cells in column show phonetic settings.
    bool                mbHidden;           /// True = column is hidden.
    bool                mbCollapsed;        /// True = column outline is collapsed.

    explicit            ColumnModel();

    /** Returns true, if this entry can be merged with the passed column range (column settings are equal). */
    bool                isMergeable( const ColumnModel& rModel ) const;
};

// ----------------------------------------------------------------------------

/** Stores settings and formatting data about a sheet row. */
struct RowModel
{
    sal_Int32           mnRow;              /// 1-based (!) index of the described row.
    ValueRangeSet       maColSpans;         /// 0-based (!) column ranges of used cells.
    double              mfHeight;           /// Row height in points.
    sal_Int32           mnXfId;             /// Row default formatting (see mbIsFormatted).
    sal_Int32           mnLevel;            /// Row outline level.
    bool                mbCustomHeight;     /// True = row has custom height.
    bool                mbCustomFormat;     /// True = cells in row have explicit formatting.
    bool                mbShowPhonetic;     /// True = cells in row show phonetic settings.
    bool                mbHidden;           /// True = row is hidden.
    bool                mbCollapsed;        /// True = row outline is collapsed.
    bool                mbThickTop;         /// True = row has extra space above text.
    bool                mbThickBottom;      /// True = row has extra space below text.

    explicit            RowModel();

    /** Inserts the passed column span into the row model. */
    void                insertColSpan( const ValueRange& rColSpan );
    /** Returns true, if this entry can be merged with the passed row range (row settings are equal). */
    bool                isMergeable( const RowModel& rModel ) const;
};

// ----------------------------------------------------------------------------

/** Stores formatting data about a page break. */
struct PageBreakModel
{
    sal_Int32           mnColRow;           /// 0-based (!) index of column/row.
    sal_Int32           mnMin;              /// Start of limited break.
    sal_Int32           mnMax;              /// End of limited break.
    bool                mbManual;           /// True = manual page break.

    explicit            PageBreakModel();
};

// ----------------------------------------------------------------------------

/** Stores data about a hyperlink range. */
struct HyperlinkModel : public ::oox::ole::StdHlinkInfo
{
    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// The cell area containing the hyperlink.
    ::rtl::OUString     maTooltip;          /// Additional tooltip text.

    explicit            HyperlinkModel();
};

// ----------------------------------------------------------------------------

/** Stores data about ranges with data validation settings. */
struct ValidationModel
{
    ApiCellRangeList    maRanges;
    ApiTokenSequence    maTokens1;
    ApiTokenSequence    maTokens2;
    ::rtl::OUString     maInputTitle;
    ::rtl::OUString     maInputMessage;
    ::rtl::OUString     maErrorTitle;
    ::rtl::OUString     maErrorMessage;
    sal_Int32           mnType;
    sal_Int32           mnOperator;
    sal_Int32           mnErrorStyle;
    bool                mbShowInputMsg;
    bool                mbShowErrorMsg;
    bool                mbNoDropDown;
    bool                mbAllowBlank;

    explicit            ValidationModel();

    /** Sets the passed BIFF validation type. */
    void                setBiffType( sal_uInt8 nType );
    /** Sets the passed BIFF operator. */
    void                setBiffOperator( sal_uInt8 nOperator );
    /** Sets the passed BIFF error style. */
    void                setBiffErrorStyle( sal_uInt8 nErrorStyle );
};

// ============================================================================
// ============================================================================

class WorksheetGlobals;
typedef ::boost::shared_ptr< WorksheetGlobals > WorksheetGlobalsRef;

class WorksheetHelper : public WorkbookHelper
{
public:
    /*implicit*/        WorksheetHelper( WorksheetGlobals& rSheetGlob );

    static WorksheetGlobalsRef constructGlobals(
                            const WorkbookHelper& rHelper,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );

    // ------------------------------------------------------------------------

    /** Returns the type of this sheet. */
    WorksheetType       getSheetType() const;
    /** Returns the index of the current sheet. */
    sal_Int16           getSheetIndex() const;
    /** Returns the XSpreadsheet interface of the current sheet. */
    const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >&
                        getSheet() const;

    /** Returns the XCell interface for the passed cell address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                        getCell( const ::com::sun::star::table::CellAddress& rAddress ) const;
    /** Returns the XCellRange interface for the passed cell range address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getCellRange( const ::com::sun::star::table::CellRangeAddress& rRange ) const;
    /** Returns the XSheetCellRanges interface for the passed cell range addresses. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges >
                        getCellRangeList( const ApiCellRangeList& rRanges ) const;

    /** Returns the address of the passed cell. The cell reference must be valid. */
    static ::com::sun::star::table::CellAddress
                        getCellAddress(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell );
    /** Returns the address of the passed cell range. The range reference must be valid. */
    static ::com::sun::star::table::CellRangeAddress
                        getRangeAddress(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >& rxRange );

    /** Returns the XCellRange interface for a column. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getColumn( sal_Int32 nCol ) const;
    /** Returns the XCellRange interface for a row. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getRow( sal_Int32 nRow ) const;

    /** Returns the XTableColumns interface for a range of columns. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns >
                        getColumns( const ValueRange& rColRange ) const;
    /** Returns the XTableRows interface for a range of rows. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows >
                        getRows( const ValueRange& rRowRange ) const;

    /** Returns the XDrawPage interface of the draw page of the current sheet. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                        getDrawPage() const;

    /** Returns the absolute cell position in 1/100 mm. */
    ::com::sun::star::awt::Point getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the cell size in 1/100 mm. */
    ::com::sun::star::awt::Size getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the size of the entire drawing page in 1/100 mm. */
    ::com::sun::star::awt::Size getDrawPageSize() const;

    /** Returns the buffer for cell contents and cell formatting. */
    SheetDataBuffer&    getSheetData() const;
    /** Returns the conditional formattings in this sheet. */
    CondFormatBuffer&   getCondFormats() const;
    /** Returns the buffer for all cell comments in this sheet. */
    CommentsBuffer&     getComments() const;
    /** Returns the auto filters for the sheet. */
    AutoFilterBuffer&   getAutoFilters() const;
    /** Returns the buffer for all web query tables in this sheet. */
    QueryTableBuffer&   getQueryTables() const;
    /** Returns the worksheet settings object. */
    WorksheetSettings&  getWorksheetSettings() const;
    /** Returns the page/print settings for this sheet. */
    PageSettings&       getPageSettings() const;
    /** Returns the view settings for this sheet. */
    SheetViewSettings&  getSheetViewSettings() const;
    /** Returns the VML drawing page for this sheet (OOXML/BIFF12 only). */
    VmlDrawing&         getVmlDrawing() const;
    /** Returns the BIFF drawing page for this sheet (BIFF2-BIFF8 only). */
    BiffSheetDrawing&   getBiffDrawing() const;

    /** Changes the current sheet type. */
    void                setSheetType( WorksheetType eSheetType );
    /** Sets a column or row page break described in the passed struct. */
    void                setPageBreak( const PageBreakModel& rModel, bool bRowBreak );
    /** Inserts the hyperlink URL into the spreadsheet. */
    void                setHyperlink( const HyperlinkModel& rModel );
    /** Inserts the data validation settings into the spreadsheet. */
    void                setValidation( const ValidationModel& rModel );
    /** Sets the passed label ranges to the current sheet. */
    void                setLabelRanges( const ApiCellRangeList& rColRanges, const ApiCellRangeList& rRowRanges );
    /** Sets the path to the DrawingML fragment of this sheet. */
    void                setDrawingPath( const ::rtl::OUString& rDrawingPath );
    /** Sets the path to the legacy VML drawing fragment of this sheet. */
    void                setVmlDrawingPath( const ::rtl::OUString& rVmlDrawingPath );

    /** Extends the used area of this sheet by the passed cell position. */
    void                extendUsedArea( const ::com::sun::star::table::CellAddress& rAddress );
    /** Extends the used area of this sheet by the passed cell range. */
    void                extendUsedArea( const ::com::sun::star::table::CellRangeAddress& rRange );
    /** Extends the shape bounding box by the position and size of the passed rectangle (in 1/100 mm). */
    void                extendShapeBoundingBox( const ::com::sun::star::awt::Rectangle& rShapeRect );

    /** Sets base width for all columns (without padding pixels). This value
        is only used, if width has not been set with setDefaultColumnWidth(). */
    void                setBaseColumnWidth( sal_Int32 nWidth );
    /** Sets default width for all columns. This function overrides the base
        width set with the setBaseColumnWidth() function. */
    void                setDefaultColumnWidth( double fWidth );
    /** Converts default cell formatting for a range of columns. */
    void                setDefaultColumnFormat( sal_Int32 nFirstCol, sal_Int32 nLastCol, sal_Int32 nXfId );
    /** Sets column settings for a specific range of columns.
        @descr  Column default formatting is converted directly, other settings
        are cached and converted in the finalizeWorksheetImport() call. */
    void                setColumnModel( const ColumnModel& rModel );

    /** Sets default height and hidden state for all unused rows in the sheet. */
    void                setDefaultRowSettings(
                            double fHeight, bool bCustomHeight,
                            bool bHidden, bool bThickTop, bool bThickBottom );
    /** Sets row settings for a specific range of rows.
        @descr  Row default formatting is converted directly, other settings
        are cached and converted in the finalizeWorksheetImport() call. */
    void                setRowModel( const RowModel& rModel );
    /** Specifies that the passed row needs to set its height manually. */
    void                setManualRowHeight( sal_Int32 nRow );

    /** Inserts a value cell directly into the Calc sheet. */
    void                putValue(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            double fValue ) const;
    /** Inserts a string cell directly into the Calc sheet. */
    void                putString(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            const ::rtl::OUString& rText ) const;
    /** Inserts a rich-string cell directly into the Calc sheet. */
    void                putRichString(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            const RichString& rString,
                            const Font* pFirstPortionFont ) const;
    /** Inserts a formula cell directly into the Calc sheet. */
    void                putFormulaTokens(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            const ApiTokenSequence& rTokens ) const;

    /** Initial conversion before importing the worksheet. */
    void                initializeWorksheetImport();
    /** Final conversion after importing the worksheet. */
    void                finalizeWorksheetImport();

private:
    WorksheetGlobals&   mrSheetGlob;
};

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox

#endif
