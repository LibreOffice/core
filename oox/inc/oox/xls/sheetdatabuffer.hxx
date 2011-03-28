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

#ifndef OOX_XLS_SHEETDATABUFFER_HXX
#define OOX_XLS_SHEETDATABUFFER_HXX

#include <list>
#include <map>
#include "oox/xls/richstring.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XNamedRange; }
    namespace util { struct DateTime; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Stores basic data about cell values and formatting. */
struct CellModel
{
    ::com::sun::star::table::CellAddress
                        maCellAddr;         /// The address of the current cell.
    sal_Int32           mnCellType;         /// Data type of the cell value.
    sal_Int32           mnXfId;             /// XF (cell formatting) identifier.
    bool                mbShowPhonetic;     /// True = show phonetic text.

    explicit            CellModel();
};

// ----------------------------------------------------------------------------

/** Stores data about cell formulas. */
struct CellFormulaModel
{
    ::com::sun::star::table::CellRangeAddress
                        maFormulaRef;       /// Formula range for array/shared formulas and data tables.
    sal_Int32           mnFormulaType;      /// Type of the formula (regular, array, shared, table).
    sal_Int32           mnSharedId;         /// Identifier of a shared formula (OOXML only).

    explicit            CellFormulaModel();

    /** Returns true, if the passed cell address is valid for an array formula. */
    bool                isValidArrayRef( const ::com::sun::star::table::CellAddress& rCellAddr );
    /** Returns true, if the passed cell address is valid for a shared formula. */
    bool                isValidSharedRef( const ::com::sun::star::table::CellAddress& rCellAddr );
};

// ----------------------------------------------------------------------------

/** Stores data about table operations. */
struct DataTableModel
{
    ::rtl::OUString     maRef1;             /// First reference cell for table operations.
    ::rtl::OUString     maRef2;             /// Second reference cell for table operations.
    bool                mb2dTable;          /// True = 2-dimensional data table.
    bool                mbRowTable;         /// True = row oriented data table.
    bool                mbRef1Deleted;      /// True = first reference cell deleted.
    bool                mbRef2Deleted;      /// True = second reference cell deleted.

    explicit            DataTableModel();
};

// ============================================================================

/** Stores position and contents of a range of cells for optimized import. */
class CellBlock : public WorksheetHelper
{
public:
    explicit            CellBlock( const WorksheetHelper& rHelper, const ValueRange& rColSpan, sal_Int32 nRow );

    /** Returns true, if the end index of the passed colspan is greater than
        the own column end index, or if the passed range has the same end index
        but the start indexes do not match. */
    bool                isBefore( const ValueRange& rColSpan ) const;
    /** Returns true, if the cell block can be expanded with the passed colspan. */
    bool                isExpandable( const ValueRange& rColSpan ) const;
    /** Returns true, if the own colspan contains the passed column. */
    bool                contains( sal_Int32 nCol ) const;

    /** Returns the specified cell from the last row in the cell buffer array. */
    ::com::sun::star::uno::Any& getCellAny( sal_Int32 nCol );
    /** Inserts a rich-string into the cell block. */
    void                insertRichString(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            const RichStringRef& rxString,
                            const Font* pFirstPortionFont );

    /** Appends a new row to the cell buffer array. */
    void                startNextRow();
    /** Writes all buffered cells into the Calc sheet. */
    void                finalizeImport();

private:
    /** Fills unused cells before passed index with empty strings. */
    void                fillUnusedCells( sal_Int32 nIndex );

private:
    /** Stores position and string data of a rich-string cell. */
    struct RichStringCell
    {
        ::com::sun::star::table::CellAddress
                            maCellAddr;         /// The address of the rich-string cell.
        RichStringRef       mxString;           /// The string with rich formatting.
        const Font*         mpFirstPortionFont; /// Font information from cell for first text portion.

        explicit            RichStringCell(
                                const ::com::sun::star::table::CellAddress& rCellAddr,
                                const RichStringRef& rxString,
                                const Font* pFirstPortionFont );
    };
    typedef ::std::list< RichStringCell > RichStringCellList;

    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// Cell range covered by this cell block.
    RichStringCellList  maRichStrings;      /// Cached rich-string cells.
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >
                        maCellArray;        /// The array of cells of this cell block.
    ::com::sun::star::uno::Any*
                        mpCurrCellRow;      /// Pointer to first cell of current row (last row in maCellArray).
    const sal_Int32     mnRowLength;        /// Number of cells covered by row of this cell block.
    sal_Int32           mnFirstFreeIndex;   /// Relative index of first unused cell in current row.
};

// ============================================================================

/** Manages all cell blocks currently in use. */
class CellBlockBuffer : public WorksheetHelper
{
public:
    explicit            CellBlockBuffer( const WorksheetHelper& rHelper );

    /** Sets column span information for a row. */
    void                setColSpans( sal_Int32 nRow, const ValueRangeSet& rColSpans );

    /** Tries to find a cell block. Recalculates the map of cell blocks, if the
        passed cell address is located in another row than the last cell. */
    CellBlock*          getCellBlock( const ::com::sun::star::table::CellAddress& rCellAddr );

    /** Inserts all cells of all open cell blocks into the Calc document. */
    void                finalizeImport();

private:
    typedef ::std::map< sal_Int32, ValueRangeVector >   ColSpanVectorMap;
    typedef RefMap< sal_Int32, CellBlock >              CellBlockMap;

    ColSpanVectorMap    maColSpans;             /// Buffereed column spans, mapped by row index.
    CellBlockMap        maCellBlocks;           /// All open cell blocks, mapped by last (!) column of the block span.
    CellBlockMap::iterator maCellBlockIt;       /// Pointer to cell block currently in use.
    sal_Int32           mnCurrRow;              /// Current row index used for buffered cell import.
};

// ============================================================================

/** Manages the cell contents and cell formatting of a sheet.
 */
class SheetDataBuffer : public WorksheetHelper
{
public:
    explicit            SheetDataBuffer( const WorksheetHelper& rHelper );

    /** Sets column span information for a row. */
    void                setColSpans( sal_Int32 nRow, const ValueRangeSet& rColSpans );

    /** Inserts a blank cell (with formatting) into the sheet. */
    void                setBlankCell( const CellModel& rModel );
    /** Inserts a value cell into the sheet. */
    void                setValueCell( const CellModel& rModel, double fValue );
    /** Inserts a simple string cell into the sheet. */
    void                setStringCell( const CellModel& rModel, const ::rtl::OUString& rText );
    /** Inserts a rich-string cell into the sheet. */
    void                setStringCell( const CellModel& rModel, const RichStringRef& rxString );
    /** Inserts a shared string cell into the sheet. */
    void                setStringCell( const CellModel& rModel, sal_Int32 nStringId );
    /** Inserts a date/time cell into the sheet and adjusts number format. */
    void                setDateTimeCell( const CellModel& rModel, const ::com::sun::star::util::DateTime& rDateTime );
    /** Inserts a boolean cell into the sheet and adjusts number format. */
    void                setBooleanCell( const CellModel& rModel, bool bValue );
    /** Inserts an error cell from the passed error code into the sheet. */
    void                setErrorCell( const CellModel& rModel, const ::rtl::OUString& rErrorCode );
    /** Inserts an error cell from the passed BIFF error code into the sheet. */
    void                setErrorCell( const CellModel& rModel, sal_uInt8 nErrorCode );
    /** Inserts a formula cell into the sheet. */
    void                setFormulaCell( const CellModel& rModel, const ApiTokenSequence& rTokens );
    /** Inserts a shared formula cell into the sheet (OOXML only). */
    void                setFormulaCell( const CellModel& rModel, sal_Int32 nSharedId );

    /** Inserts the passed token array as array formula. */
    void                createArrayFormula(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const ApiTokenSequence& rTokens );
    /** Sets a multiple table operation to the passed range. */
    void                createTableOperation(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const DataTableModel& rModel );
    /** Creates a named range with a special name for a shared formula with the
        specified identifier and formula definition (OOXML only). */
    void                createSharedFormula(
                            sal_Int32 nSharedId,
                            const ApiTokenSequence& rTokens );
    /** Creates a named range with a special name for a shared formula with the
        specified base address and formula definition (BIFF only). */
    void                createSharedFormula(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ApiTokenSequence& rTokens );

    /** Sets default cell formatting for the specified range of rows. */
    void                setRowFormat( sal_Int32 nRow, sal_Int32 nXfId, bool bCustomFormat );
    /** Merges the cells in the passed cell range. */
    void                setMergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );
    /** Sets a standard number format (constant from com.sun.star.util.NumberFormat) to the specified cell. */
    void                setStandardNumFmt(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_Int16 nStdNumFmt );

    /** Final processing after the sheet has been imported. */
    void                finalizeImport();

private:
    struct XfIdRowRange;
    struct XfIdRange;

    /** Sets the passed formula token array into a cell. */
    void                setCellFormula(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ApiTokenSequence& rTokens );

    /** Creates a named range with a special name for a shared formula with the
        specified base address and formula definition. */
    void                createSharedFormula( const BinAddress& rMapKey, const ApiTokenSequence& rTokens );
    /** Creates a formula token array representing the shared formula with the
        passed identifier. */
    ApiTokenSequence    resolveSharedFormula( const BinAddress& rMapKey ) const;

    /** Inserts the passed array formula into the sheet. */
    void                finalizeArrayFormula(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const ApiTokenSequence& rTokens ) const;
    /** Inserts the passed table operation into the sheet. */
    void                finalizeTableOperation(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const DataTableModel& rModel ) const;

    /** Processes the cell formatting data of the passed cell.
        @param nNumFmtId  If set, overrides number format of the cell XF. */
    void                setCellFormat( const CellModel& rModel, sal_Int32 nNumFmtId = -1 );

    /** Writes all cell formatting attributes to the passed row range. */
    void                writeXfIdRowRangeProperties( const XfIdRowRange& rXfIdRowRange ) const;
    /** Writes all cell formatting attributes to the passed cell range. */
    void                writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const;
    /** Tries to merge the ranges last inserted in maXfIdRanges with existing ranges. */
    void                mergeXfIdRanges();

    /** Merges the passed merged range and updates right/bottom cell borders. */
    void                finalizeMergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );

private:
    /** Stores cell range address and formula token array of an array formula. */
    typedef ::std::pair< ::com::sun::star::table::CellRangeAddress, ApiTokenSequence > ArrayFormula;
    typedef ::std::list< ArrayFormula > ArrayFormulaList;

    /** Stores cell range address and settings of a table operation. */
    typedef ::std::pair< ::com::sun::star::table::CellRangeAddress, DataTableModel > TableOperation;
    typedef ::std::list< TableOperation > TableOperationList;

    typedef ::std::map< BinAddress, sal_Int32 > SharedFormulaMap;

    /** Stores information about a range of rows with equal cell formatting. */
    struct XfIdRowRange
    {
        ValueRange          maRowRange;         /// Indexes of first and last row.
        sal_Int32           mnXfId;             /// XF identifier for the row range.

        explicit            XfIdRowRange();
        bool                intersects( const ::com::sun::star::table::CellRangeAddress& rRange ) const;
        void                set( sal_Int32 nRow, sal_Int32 nXfId );
        bool                tryExpand( sal_Int32 nRow, sal_Int32 nXfId );
    };

    /** Stores information about a range of cells with equal formatting. */
    struct XfIdRange
    {
        ::com::sun::star::table::CellRangeAddress
                            maRange;            /// The formatted cell range.
        sal_Int32           mnXfId;             /// XF identifier for the range.
        sal_Int32           mnNumFmtId;         /// Number format overriding the XF.

        void                set( const ::com::sun::star::table::CellAddress& rCellAddr, sal_Int32 nXfId, sal_Int32 nNumFmtId );
        bool                tryExpand( const ::com::sun::star::table::CellAddress& rCellAddr, sal_Int32 nXfId, sal_Int32 nNumFmtId );
        bool                tryMerge( const XfIdRange& rXfIdRange );
    };
    typedef ::std::map< BinAddress, XfIdRange > XfIdRangeMap;

    /** Stores information about a merged cell range. */
    struct MergedRange
    {
        ::com::sun::star::table::CellRangeAddress
                            maRange;            /// The formatted cell range.
        sal_Int32           mnHorAlign;         /// Horizontal alignment in the range.

        explicit            MergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );
        explicit            MergedRange( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nHorAlign );
        bool                tryExpand( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nHorAlign );
    };
    typedef ::std::list< MergedRange > MergedRangeList;

    CellBlockBuffer     maCellBlocks;           /// Manages all open cell blocks.
    ArrayFormulaList    maArrayFormulas;        /// All array formulas in the sheet.
    TableOperationList  maTableOperations;      /// All table operations in the sheet.
    SharedFormulaMap    maSharedFormulas;       /// Maps shared formula base address to defined name token index.
    ::com::sun::star::table::CellAddress
                        maSharedFmlaAddr;       /// Address of a cell containing a pending shared formula.
    BinAddress          maSharedBaseAddr;       /// Base address of the pending shared formula.
    XfIdRowRange        maXfIdRowRange;         /// Cached XF identifier for a range of rows.
    XfIdRangeMap        maXfIdRanges;           /// Collected XF identifiers for cell ranges.
    MergedRangeList     maMergedRanges;         /// Merged cell ranges.
    MergedRangeList     maCenterFillRanges;     /// Merged cell ranges from 'center across' or 'fill' alignment.
    bool                mbPendingSharedFmla;    /// True = maSharedFmlaAddr and maSharedBaseAddr are valid.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
