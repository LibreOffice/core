/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: worksheethelper.hxx,v $
 * $Revision: 1.4.20.4 $
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

#include "oox/helper/progressbar.hxx"
#include "oox/ole/olehelper.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/formulabase.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Point; }
    namespace awt { struct Size; }
    namespace util { struct DateTime; }
    namespace drawing { class XDrawPage; }
    namespace table { class XTableColumns; }
    namespace table { class XTableRows; }
    namespace table { class XCell; }
    namespace table { class XCellRange; }
    namespace sheet { class XSpreadsheet; }
    namespace sheet { class XSheetCellRanges; }
} } }

namespace oox {
namespace xls {

struct BinAddress;
struct BinRange;
class BinRangeList;
class WorksheetSettings;
class SharedFormulaBuffer;
class CondFormatBuffer;
class CommentsBuffer;
class PageSettings;
class SheetViewSettings;
class VmlDrawing;

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

/** Stores some data about a cell. */
struct CellModel
{
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > mxCell;
    ::com::sun::star::table::CellAddress maAddress;
    ::rtl::OUString     maValueStr;         /// String containing cell value data.
    ::rtl::OUString     maFormulaRef;       /// String containing formula range for array/shared formulas.
    sal_Int32           mnCellType;         /// Data type of the cell.
    sal_Int32           mnFormulaType;      /// Type of the imported formula.
    sal_Int32           mnSharedId;         /// Shared formula identifier for current cell.
    sal_Int32           mnXfId;             /// XF identifier for the cell.
    sal_Int32           mnNumFmtId;         /// Forced number format for the cell.
    bool                mbHasValueStr;      /// True = contents of maValueStr are valid.
    bool                mbShowPhonetic;     /// True = show phonetic text.

    inline explicit     CellModel() { reset(); }
    void                reset();
};

// ----------------------------------------------------------------------------

/** Stores data about a data table a.k.a. multiple operation range. */
struct DataTableModel
{
    ::rtl::OUString     maRef1;             /// String containing first reference cell for data table formulas.
    ::rtl::OUString     maRef2;             /// String containing second reference cell for data table formulas.
    bool                mb2dTable;          /// True = 2-dimensional data table.
    bool                mbRowTable;         /// True = row oriented data table.
    bool                mbRef1Deleted;      /// True = first reference cell deleted.
    bool                mbRef2Deleted;      /// True = second reference cell deleted.

    explicit            DataTableModel();
};

// ----------------------------------------------------------------------------

/** Stores formatting data about a range of columns. */
struct ColumnModel
{
    sal_Int32           mnFirstCol;         /// 1-based (!) index of first column.
    sal_Int32           mnLastCol;          /// 1-based (!) index of last column.
    double              mfWidth;            /// Column width in number of characters.
    sal_Int32           mnXfId;             /// Column default formatting.
    sal_Int32           mnLevel;            /// Column outline level.
    bool                mbShowPhonetic;     /// True = cells in column show phonetic settings.
    bool                mbHidden;           /// True = column is hidden.
    bool                mbCollapsed;        /// True = column outline is collapsed.

    explicit            ColumnModel();

    /** Expands this entry with the passed column range, if column settings are equal. */
    bool                tryExpand( const ColumnModel& rModel );
};

// ----------------------------------------------------------------------------

/** Stores formatting data about a range of rows. */
struct RowModel
{
    sal_Int32           mnFirstRow;         /// 1-based (!) index of first row.
    sal_Int32           mnLastRow;          /// 1-based (!) index of last row.
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

    /** Expands this entry with the passed row range, if row settings are equal. */
    bool                tryExpand( const RowModel& rModel );
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

    /** Sets the passed OOBIN or BIFF validation type. */
    void                setBinType( sal_uInt8 nType );
    /** Sets the passed OOBIN or BIFF operator. */
    void                setBinOperator( sal_uInt8 nOperator );
    /** Sets the passed OOBIN or BIFF error style. */
    void                setBinErrorStyle( sal_uInt8 nErrorStyle );
};

// ============================================================================
// ============================================================================

class WorksheetData;

class WorksheetHelper : public WorkbookHelper
{
public:
    /*implicit*/        WorksheetHelper( WorksheetData& rSheetData );

    /** Returns the type of this sheet. */
    WorksheetType       getSheetType() const;
    /** Returns the index of the current sheet. */
    sal_Int16           getSheetIndex() const;
    /** Returns the XSpreadsheet interface of the current sheet. */
    const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >&
                        getSheet() const;

    /** Returns the XCell interface for the passed cell address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                        getCell(
                            const ::com::sun::star::table::CellAddress& rAddress ) const;
    /** Returns the XCell interface for the passed cell address string. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                        getCell(
                            const ::rtl::OUString& rAddressStr,
                            ::com::sun::star::table::CellAddress* opAddress = 0 ) const;
    /** Returns the XCell interface for the passed cell address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                        getCell(
                            const BinAddress& rBinAddress,
                            ::com::sun::star::table::CellAddress* opAddress = 0 ) const;

    /** Returns the XCellRange interface for the passed cell range address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getCellRange(
                            const ::com::sun::star::table::CellRangeAddress& rRange ) const;
    /** Returns the XCellRange interface for the passed range address string. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getCellRange(
                            const ::rtl::OUString& rRangeStr,
                            ::com::sun::star::table::CellRangeAddress* opRange = 0 ) const;
    /** Returns the XCellRange interface for the passed range address. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                        getCellRange(
                            const BinRange& rBinRange,
                            ::com::sun::star::table::CellRangeAddress* opRange = 0 ) const;

    /** Returns the XSheetCellRanges interface for the passed cell range addresses. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges >
                        getCellRangeList( const ApiCellRangeList& rRanges ) const;
    /** Returns the XSheetCellRanges interface for the passed space-separated range list. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges >
                        getCellRangeList(
                            const ::rtl::OUString& rRangesStr,
                            ApiCellRangeList* opRanges = 0 ) const;
    /** Returns the XSheetCellRanges interface for the passed range list. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges >
                        getCellRangeList(
                            const BinRangeList& rBinRanges,
                            ApiCellRangeList* opRanges = 0 ) const;

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
                        getColumns( sal_Int32 nFirstCol, sal_Int32 nLastCol ) const;
    /** Returns the XTableRows interface for a range of rows. */
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows >
                        getRows( sal_Int32 nFirstRow, sal_Int32 nLastRow ) const;

    /** Returns the XDrawPage interface of the draw page of the current sheet. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                        getDrawPage() const;

    /** Returns the absolute cell position in 1/100 mm. */
    ::com::sun::star::awt::Point getCellPosition( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the cell size in 1/100 mm. */
    ::com::sun::star::awt::Size getCellSize( sal_Int32 nCol, sal_Int32 nRow ) const;
    /** Returns the size of the entire drawing page in 1/100 mm. */
    ::com::sun::star::awt::Size getDrawPageSize() const;

    /** Returns the worksheet settings object. */
    WorksheetSettings&  getWorksheetSettings() const;
    /** Returns the buffer containing all shared formulas in this sheet. */
    SharedFormulaBuffer& getSharedFormulas() const;
    /** Returns the conditional formattings in this sheet. */
    CondFormatBuffer&   getCondFormats() const;
    /** Returns the buffer for all cell comments in this sheet. */
    CommentsBuffer&     getComments() const;
    /** Returns the page/print settings for this sheet. */
    PageSettings&       getPageSettings() const;
    /** Returns the view settings for this sheet. */
    SheetViewSettings&  getSheetViewSettings() const;
    /** Returns the VML drawing page for this sheet. */
    VmlDrawing&         getVmlDrawing() const;

    /** Sets the passed string to the cell. */
    void                setStringCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::rtl::OUString& rText ) const;
    /** Sets the shared string with the passed identifier to the cell. */
    void                setSharedStringCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            sal_Int32 nStringId,
                            sal_Int32 nXfId ) const;
    /** Sets the passed date/time value to the cell and adjusts number format. */
    void                setDateTimeCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::util::DateTime& rDateTime ) const;
    /** Sets the passed boolean value to the cell and adjusts number format. */
    void                setBooleanCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            bool bValue ) const;
    /** Sets the passed BIFF error code to the cell (by converting it to a formula). */
    void                setErrorCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::rtl::OUString& rErrorCode ) const;
    /** Sets the passed BIFF error code to the cell (by converting it to a formula). */
    void                setErrorCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            sal_uInt8 nErrorCode ) const;
    /** Sets cell contents to the cell specified in the passed cell model. */
    void                setCell( CellModel& orModel ) const;

    /** Sets a standard number format (constant from com.sun.star.util.NumberFormat) to the passed cell. */
    void                setStandardNumFmt(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            sal_Int16 nStdNumFmt ) const;

    /** Changes the current sheet type. */
    void                setSheetType( WorksheetType eSheetType );
    /** Sets the dimension (used area) of the sheet. */
    void                setDimension( const ::com::sun::star::table::CellRangeAddress& rRange );
    /** Stores the cell formatting data of the current cell. */
    void                setCellFormat( const CellModel& rModel );
    /** Merges the cells in the passed cell range. */
    void                setMergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );
    /** Sets a column or row page break described in the passed struct. */
    void                setPageBreak( const PageBreakModel& rModel, bool bRowBreak );
    /** Inserts the hyperlink URL into the spreadsheet. */
    void                setHyperlink( const HyperlinkModel& rModel );
    /** Inserts the data validation settings into the spreadsheet. */
    void                setValidation( const ValidationModel& rModel );
    /** Sets a multiple table operation to the passed range. */
    void                setTableOperation(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const DataTableModel& rModel ) const;
    /** Sets the passed label ranges to the current sheet. */
    void                setLabelRanges(
                            const ApiCellRangeList& rColRanges,
                            const ApiCellRangeList& rRowRanges );
    /** Sets the path to the DrawingML fragment of this sheet. */
    void                setDrawingPath( const ::rtl::OUString& rDrawingPath );
    /** Sets the path to the legacy VML drawing fragment of this sheet. */
    void                setVmlDrawingPath( const ::rtl::OUString& rVmlDrawingPath );

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

    /** Initial conversion before importing the worksheet. */
    void                initializeWorksheetImport();
    /** Final conversion after importing the worksheet. */
    void                finalizeWorksheetImport();

private:
    WorksheetData&      mrSheetData;
};

// ============================================================================

namespace prv {

typedef ::boost::shared_ptr< WorksheetData > WorksheetDataRef;

struct WorksheetDataOwner
{
    explicit            WorksheetDataOwner( WorksheetDataRef xSheetData );
    virtual             ~WorksheetDataOwner();
    WorksheetDataRef    mxSheetData;
};

} // namespace prv

// ----------------------------------------------------------------------------

class WorksheetHelperRoot : private prv::WorksheetDataOwner, public WorksheetHelper
{
public:
    /** Returns true, if this helper refers to an existing Calc sheet. */
    bool                isValidSheet() const;

protected:
    /** Constructs from the passed data, creates and owns a new data object. */
    explicit            WorksheetHelperRoot(
                            const WorkbookHelper& rHelper,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );

    /** Constructs from another sheet helper, does not create a data object. */
    explicit            WorksheetHelperRoot(
                            const WorksheetHelper& rHelper );

    /** Constructs from another sheet helper, shares ownership of the passed helper. */
    explicit            WorksheetHelperRoot(
                            const WorksheetHelperRoot& rHelper );

private:
    WorksheetHelperRoot& operator=( const WorksheetHelperRoot& );
};

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox

#endif

