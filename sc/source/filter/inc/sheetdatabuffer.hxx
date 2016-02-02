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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_SHEETDATABUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_SHEETDATABUFFER_HXX

#include <list>
#include <map>
#include <set>

#include "richstring.hxx"
#include "worksheethelper.hxx"

namespace com { namespace sun { namespace star {
    namespace util { struct DateTime; }
} } }

namespace oox {
namespace xls {

/** Stores basic data about cell values and formatting. */
struct CellModel
{
    css::table::CellAddress
                        maCellAddr;         /// The address of the current cell.
    sal_Int32           mnCellType;         /// Data type of the cell value.
    sal_Int32           mnXfId;             /// XF (cell formatting) identifier.
    bool                mbShowPhonetic;     /// True = show phonetic text.

    explicit            CellModel();
};

/** Stores data about cell formulas. */
struct CellFormulaModel
{
    css::table::CellRangeAddress
                        maFormulaRef;       /// Formula range for array/shared formulas and data tables.
    sal_Int32           mnFormulaType;      /// Type of the formula (regular, array, shared, table).
    sal_Int32           mnSharedId;         /// Identifier of a shared formula (OOXML only).

    explicit            CellFormulaModel();

    /** Returns true, if the passed cell address is valid for an array formula. */
    bool                isValidArrayRef( const css::table::CellAddress& rCellAddr );
    /** Returns true, if the passed cell address is valid for a shared formula. */
    bool                isValidSharedRef( const css::table::CellAddress& rCellAddr );
};

/** Stores data about table operations. */
struct DataTableModel
{
    OUString     maRef1;             /// First reference cell for table operations.
    OUString     maRef2;             /// Second reference cell for table operations.
    bool                mb2dTable;          /// True = 2-dimensional data table.
    bool                mbRowTable;         /// True = row oriented data table.
    bool                mbRef1Deleted;      /// True = first reference cell deleted.
    bool                mbRef2Deleted;      /// True = second reference cell deleted.

    explicit            DataTableModel();
};

/** Manages all cell blocks currently in use. */
class CellBlockBuffer : public WorksheetHelper
{
public:
    explicit            CellBlockBuffer( const WorksheetHelper& rHelper );

    /** Sets column span information for a row. */
    void                setColSpans( sal_Int32 nRow, const ValueRangeSet& rColSpans );

private:
    typedef ::std::map< sal_Int32, ValueRangeVector >   ColSpanVectorMap;

    ColSpanVectorMap    maColSpans;             /// Buffered column spans, mapped by row index.
    sal_Int32           mnCurrRow;              /// Current row index used for buffered cell import.
};

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
    void                setStringCell( const CellModel& rModel, const OUString& rText );
    /** Inserts a rich-string cell into the sheet. */
    void                setStringCell( const CellModel& rModel, const RichStringRef& rxString );
    /** Inserts a shared string cell into the sheet. */
    void                setStringCell( const CellModel& rModel, sal_Int32 nStringId );
    /** Inserts a date/time cell into the sheet and adjusts number format. */
    void                setDateTimeCell( const CellModel& rModel, const css::util::DateTime& rDateTime );
    /** Inserts a boolean cell into the sheet and adjusts number format. */
    void                setBooleanCell( const CellModel& rModel, bool bValue );
    /** Inserts an error cell from the passed error code into the sheet. */
    void                setErrorCell( const CellModel& rModel, const OUString& rErrorCode );
    /** Inserts an error cell from the passed BIFF error code into the sheet. */
    void                setErrorCell( const CellModel& rModel, sal_uInt8 nErrorCode );
    /** Inserts a formula cell into the sheet. */
    void                setFormulaCell( const CellModel& rModel, const ApiTokenSequence& rTokens );
    /** Inserts a ISO 8601 date cell into the sheet. */
    void                setDateCell( const CellModel& rModel, const OUString& rDateString );

    void                createSharedFormula(
            const css::table::CellAddress& rRange,
            const ApiTokenSequence& rTokens);

    /** Inserts the passed token array as array formula. */
    void                createArrayFormula(
                            const css::table::CellRangeAddress& rRange,
                            const ApiTokenSequence& rTokens );
    /** Sets a multiple table operation to the passed range. */
    void                createTableOperation(
                            const css::table::CellRangeAddress& rRange,
                            const DataTableModel& rModel );

    /** Sets default cell formatting for the specified range of rows. */
    void                setRowFormat( sal_Int32 nRow, sal_Int32 nXfId, bool bCustomFormat );
    /** Merges the cells in the passed cell range. */
    void                setMergedRange( const css::table::CellRangeAddress& rRange );
    /** Sets a standard number format (constant from com.sun.star.util.NumberFormat) to the specified cell. */
    void                setStandardNumFmt(
                            const css::table::CellAddress& rCellAddr,
                            sal_Int16 nStdNumFmt );
    /** Processes the cell formatting data of the passed cell.
        @param nNumFmtId  If set, overrides number format of the cell XF. */
    void                setCellFormat( const CellModel& rModel, sal_Int32 nNumFmtId = -1 );

    /** Final processing after the sheet has been imported. */
    void                finalizeImport();

    /** Sets the passed formula token array into a cell. */
    void                setCellFormula(
                            const css::table::CellAddress& rCellAddr,
                            const ApiTokenSequence& rTokens );

private:
    struct XfIdRowRange;

    /** Creates a formula token array representing the shared formula with the
        passed identifier. */
    ApiTokenSequence    resolveSharedFormula( const css::table::CellAddress& rMapKey ) const;

    /** Inserts the passed array formula into the sheet. */
    void                finalizeArrayFormula(
                            const css::table::CellRangeAddress& rRange,
                            const ApiTokenSequence& rTokens ) const;
    /** Inserts the passed table operation into the sheet. */
    void finalizeTableOperation(
        const css::table::CellRangeAddress& rRange, const DataTableModel& rModel );

    /** Writes all cell formatting attributes to the passed cell range list. (depreciates writeXfIdRangeProperties) */
    void                applyCellMerging( const css::table::CellRangeAddress& rRange );
    void                addColXfStyle( sal_Int32 nXfId, sal_Int32 nFormatId, const css::table::CellRangeAddress& rAddress, bool bProcessRowRange = false );
private:
    /** Stores cell range address and formula token array of an array formula. */
    typedef ::std::pair< css::table::CellRangeAddress, ApiTokenSequence > ArrayFormula;
    typedef ::std::list< ArrayFormula > ArrayFormulaList;

    /** Stores cell range address and settings of a table operation. */
    typedef ::std::pair< css::table::CellRangeAddress, DataTableModel > TableOperation;
    typedef ::std::list< TableOperation > TableOperationList;

    /** Stores information about a range of rows with equal cell formatting. */
    struct XfIdRowRange
    {
        ValueRange          maRowRange;         /// Indexes of first and last row.
        sal_Int32           mnXfId;             /// XF identifier for the row range.

        explicit            XfIdRowRange();
        void                set( sal_Int32 nRow, sal_Int32 nXfId );
        bool                tryExpand( sal_Int32 nRow, sal_Int32 nXfId );
    };

    typedef ::std::pair< sal_Int32, sal_Int32 > XfIdNumFmtKey;
    typedef ::std::map< XfIdNumFmtKey, ApiCellRangeList > XfIdRangeListMap;

    typedef ::std::pair< sal_Int32, sal_Int32 > RowRange;
    struct RowRangeStyle
    {
        sal_Int32 mnStartRow;
        sal_Int32 mnEndRow;
        XfIdNumFmtKey mnNumFmt;
    };
    struct StyleRowRangeComp
    {
        bool operator() (const RowRangeStyle& lhs, const RowRangeStyle& rhs) const
        {
            return lhs.mnEndRow<rhs.mnStartRow;
        }
    };
    typedef ::std::set< RowRangeStyle, StyleRowRangeComp > RowStyles;
    typedef ::std::map< sal_Int32, RowStyles > ColStyles;
    /** Stores information about a merged cell range. */
    struct MergedRange
    {
        css::table::CellRangeAddress
                            maRange;            /// The formatted cell range.
        sal_Int32           mnHorAlign;         /// Horizontal alignment in the range.

        explicit            MergedRange( const css::table::CellRangeAddress& rRange );
        explicit            MergedRange( const css::table::CellAddress& rAddress, sal_Int32 nHorAlign );
        bool                tryExpand( const css::table::CellAddress& rAddress, sal_Int32 nHorAlign );
    };
    typedef ::std::list< MergedRange > MergedRangeList;

    ColStyles           maStylesPerColumn;      /// Stores cell styles by column ( in row ranges )
    CellBlockBuffer     maCellBlocks;           /// Manages all open cell blocks.
    ArrayFormulaList    maArrayFormulas;        /// All array formulas in the sheet.
    TableOperationList  maTableOperations;      /// All table operations in the sheet.
    ::std::map< BinAddress, ApiTokenSequence >
                        maSharedFormulas;       /// Maps shared formula base address to defined name token index.
    css::table::CellAddress
                        maSharedFmlaAddr;       /// Address of a cell containing a pending shared formula.
    css::table::CellAddress maSharedBaseAddr;       /// Base address of the pending shared formula.
    XfIdRowRange        maXfIdRowRange;         /// Cached XF identifier for a range of rows.
    XfIdRangeListMap    maXfIdRangeLists;       /// Collected XF identifiers for cell rangelists.
    MergedRangeList     maMergedRanges;         /// Merged cell ranges.
    MergedRangeList     maCenterFillRanges;     /// Merged cell ranges from 'center across' or 'fill' alignment.
    bool                mbPendingSharedFmla;    /// True = maSharedFmlaAddr and maSharedBaseAddr are valid.
    std::map< sal_Int32, std::vector< ValueRange > > maXfIdRowRangeList; /// Cached XF identifiers for a ranges of rows, we try and process rowranges with the same XF id together
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
