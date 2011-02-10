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

/** Manages the cell contents and cell formatting of a sheet.
 */
class SheetDataBuffer : public WorksheetHelper
{
public:
    explicit            SheetDataBuffer( const WorksheetHelper& rHelper );

    /** Imports a shared formula from a OOXML formula string. */
    void                importSharedFmla( const ::rtl::OUString& rFormula,
                            const ::rtl::OUString& rSharedRange, sal_Int32 nSharedId,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );
    /** Imports a shared formula from a SHAREDFORMULA record in the passed stream */
    void                importSharedFmla( SequenceInputStream& rStrm,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );
    /** Imports a shared formula from a SHAREDFMLA record in the passed stream. */
    void                importSharedFmla( BiffInputStream& rStrm,
                            const ::com::sun::star::table::CellAddress& rBaseAddr );

    /** Sets the passed value to the cell. */
    void                setValueCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            double fValue );
    /** Sets the passed string to the cell. */
    void                setStringCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ::rtl::OUString& rText );
    /** Sets the passed rich-string to the cell. */
    void                setStringCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const RichString& rString, sal_Int32 nXfId );
    /** Sets the shared string with the passed identifier to the cell. */
    void                setStringCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_Int32 nStringId, sal_Int32 nXfId );
    /** Sets the passed date/time value to the cell and adjusts number format. */
    void                setDateTimeCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ::com::sun::star::util::DateTime& rDateTime );
    /** Sets the passed boolean value to the cell and adjusts number format. */
    void                setBooleanCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            bool bValue );
    /** Sets the passed BIFF error code to the cell (by converting it to a formula). */
    void                setErrorCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ::rtl::OUString& rErrorCode );
    /** Sets the passed BIFF error code to the cell (by converting it to a formula). */
    void                setErrorCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_uInt8 nErrorCode );
    /** Sets the passed formula token sequence to the cell. */
    void                setFormulaCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            const ApiTokenSequence& rTokens );
    /** Sets the shared formula with the passed identifier to the cell (OOXML only). */
    void                setFormulaCell(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >& rxCell,
                            const ::com::sun::star::table::CellAddress& rCellAddr,
                            sal_Int32 nSharedId );

    /** Inserts the passed token array as array formula. */
    void                setArrayFormula(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const ApiTokenSequence& rTokens );
    /** Sets a multiple table operation to the passed range. */
    void                setTableOperation(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            const DataTableModel& rModel );

    /** Sets default cell formatting for the specified range of rows. */
    void                setRowFormat( sal_Int32 nFirstRow, sal_Int32 nLastRow, sal_Int32 nXfId, bool bCustomFormat );
    /** Processes the cell formatting data of the passed cell. */
    void                setCellFormat( const CellModel& rModel );
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

    /** Creates and returns an empty named range with a special name for a
        shared formula with the specified base position. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XNamedRange >
                        createSharedFormulaName( const BinAddress& rMapKey );

    /** Creates a formula token array representing the shared formula with the
        passed identifier (OOXML only). */
    ApiTokenSequence    resolveSharedFormula( sal_Int32 nSharedId ) const;
    /** Creates a formula token array representing the shared formula at the
        passed base address. */
    ApiTokenSequence    resolveSharedFormula( const ::com::sun::star::table::CellAddress& rBaseAddr ) const;

    /** Retries to insert a shared formula that has not been set in the last
        call to setFormulaCell() due to the missing formula definition. */
    void                retryPendingSharedFormulaCell();

    /** Writes all cell formatting attributes to the passed row range. */
    void                writeXfIdRowRangeProperties( const XfIdRowRange& rXfIdRowRange ) const;
    /** Writes all cell formatting attributes to the passed cell range. */
    void                writeXfIdRangeProperties( const XfIdRange& rXfIdRange ) const;
    /** Tries to merge the ranges last inserted in maXfIdRanges with existing ranges. */
    void                mergeXfIdRanges();

    /** Merges the passed merged range and updates right/bottom cell borders. */
    void                finalizeMergedRange( const ::com::sun::star::table::CellRangeAddress& rRange );

private:
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
        sal_Int32           mnNumFmtId;         /// Number format id overriding the XF.

        void                set( const CellModel& rModel );
        bool                tryExpand( const CellModel& rModel );
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

    typedef ::std::map< BinAddress, sal_Int32 > TokenIndexMap;
    typedef ::std::map< BinAddress, XfIdRange > XfIdRangeMap;
    typedef ::std::list< MergedRange >          MergedRangeList;

    TokenIndexMap       maTokenIndexes;         /// Maps shared formula base address to defined name token index.
    ::com::sun::star::table::CellAddress
                        maSharedFmlaAddr;       /// Address of a cell containing a pending shared formula.
    ::com::sun::star::table::CellAddress
                        maSharedBaseAddr;       /// Base address of the pending shared formula.
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
