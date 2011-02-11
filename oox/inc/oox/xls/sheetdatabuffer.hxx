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

/** Manages the cell contents and cell formatting of a sheet.
 */
class SheetDataBuffer : public WorksheetHelper
{
public:
    explicit            SheetDataBuffer( const WorksheetHelper& rHelper );

    /** Sets the passed value to the cell. */
    void                setValueCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            double fValue );
    /** Sets the passed string to the cell. */
    void                setStringCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ::rtl::OUString& rText );
    /** Sets the passed rich-string to the cell. */
    void                setStringCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const RichString& rString, sal_Int32 nXfId );
    /** Sets the shared string with the passed identifier to the cell. */
    void                setStringCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_Int32 nStringId, sal_Int32 nXfId );
    /** Sets the passed date/time value to the cell and adjusts number format. */
    void                setDateTimeCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ::com::sun::star::util::DateTime& rDateTime );
    /** Sets the passed boolean value to the cell and adjusts number format. */
    void                setBooleanCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            bool bValue );
    /** Sets the passed BIFF error code to the cell (by converting it to a formula). */
    void                setErrorCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ::rtl::OUString& rErrorCode );
    /** Sets the passed BIFF error code to the cell (by converting it to a formula). */
    void                setErrorCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_uInt8 nErrorCode );
    /** Sets the passed formula token sequence to the cell. */
    void                setFormulaCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ApiTokenSequence& rTokens );
    /** Sets the shared formula with the passed identifier to the cell (OOXML only). */
    void                setFormulaCell(
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_Int32 nSharedId );

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
    void                setRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId, bool bCustomFormat );
    /** Processes the cell formatting data of the passed cell.
        @param nNumFmtId  If set, overrides number format of the cell XF. */
    void                setCellFormat( const CellModel& rModel, sal_Int32 nNumFmtId = -1 );
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

    /** Inserts the passed array formula into the sheet. */
    void                finalizeArrayFormula(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const ApiTokenSequence& rTokens );
    /** Inserts the passed table operation into the sheet. */
    void                finalizeTableOperation(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const DataTableModel& rModel );

    /** Creates a named range with a special name for a shared formula with the
        specified base address and formula definition. */
    void                createSharedFormula( const BinAddress& rMapKey, const ApiTokenSequence& rTokens );
    /** Creates a formula token array representing the shared formula with the
        passed identifier. */
    ApiTokenSequence    resolveSharedFormula( const BinAddress& rMapKey ) const;

    /** Writes all cell formatting attributes to the passed row range. */
    void                writeXfIdRowRangeProperties( const XfIdRowRange& rXfIdRowRange ) const;
    /** Writes all cell formatting attributes to the passed cell range. */
    void                writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const;
    /** Tries to merge the ranges last inserted in maXfIdRanges with existing ranges. */
    void                mergeXfIdRanges();

    /** Merges the passed merged range and updates right/bottom cell borders. */
    void                finalizeMergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );

private:
    typedef ::std::pair< ::com::sun::star::table::CellRangeAddress, ApiTokenSequence >  ArrayFormula;
    typedef ::std::pair< ::com::sun::star::table::CellRangeAddress, DataTableModel >    TableOperation;

    struct XfIdRowRange
    {
        sal_Int32           mnFirstRow;         /// Index of first row.
        sal_Int32           mnLastRow;          /// Index of last row.
        sal_Int32           mnXfId;             /// XF identifier for the row range.

        explicit            XfIdRowRange();
        bool                intersects( const ::com::sun::star::table::CellRangeAddress& rRange ) const;
        void                set( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId );
        bool                tryExpand( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId );
    };

    struct XfIdRange
    {
        ::com::sun::star::table::CellRangeAddress
                            maRange;            /// The formatted cell range.
        sal_Int32           mnXfId;             /// XF identifier for the range.
        sal_Int32           mnNumFmtId;         /// Number format overriding the XF.

        void                set( const CellModel& rModel, sal_Int32 nNumFmtId );
        bool                tryExpand( const CellModel& rModel, sal_Int32 nNumFmtId );
        bool                tryMerge( const XfIdRange& rXfIdRange );
    };

    struct MergedRange
    {
        ::com::sun::star::table::CellRangeAddress
                            maRange;            /// The formatted cell range.
        sal_Int32           mnHorAlign;         /// Horizontal alignment in the range.

        explicit            MergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );
        explicit            MergedRange( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nHorAlign );
        bool                tryExpand( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nHorAlign );
    };

    typedef ::std::list< ArrayFormula >         ArrayFormulaList;
    typedef ::std::list< TableOperation >       TableOperationList;
    typedef ::std::map< BinAddress, sal_Int32 > SharedFormulaMap;
    typedef ::std::map< BinAddress, XfIdRange > XfIdRangeMap;
    typedef ::std::list< MergedRange >          MergedRangeList;

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
