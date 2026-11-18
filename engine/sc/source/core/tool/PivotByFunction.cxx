/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <PivotByFunction.hxx>

#include <interpre.hxx>
#include <globstr.hrc>
#include <scmatrix.hxx>
#include <scresid.hxx>

#include <svl/sharedstringpool.hxx>

#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <vector>

using namespace formula;
using namespace sc::grouping;

namespace
{
std::vector<SCSIZE> intersectRows(const std::vector<SCSIZE>& rLeft,
                                  const std::vector<SCSIZE>& rRight)
{
    std::vector<SCSIZE> aRows;
    std::set_intersection(rLeft.begin(), rLeft.end(), rRight.begin(), rRight.end(),
                          std::back_inserter(aRows));
    return aRows;
}
}

namespace sc
{
struct PivotByFunction::Pivot
{
    FormulaCallableRef mpCallable;
    ScMatrixRef mxValues;
    /// The row keys and the column keys. Either may be left out.
    ScMatrixRef mxRowKeys;
    ScMatrixRef mxColumnKeys;
    SCSIZE mnValueColumns = 0;
    SCSIZE mnRowKeyColumns = 0;
    SCSIZE mnColumnKeyColumns = 0;
    /// Relative to: 0 is the column total, 1 the row total, 2 the grand total, 3 the total of the
    /// parent column group in the same row, and 4 the total of the parent row group in the same
    /// column.
    sal_Int32 mnRelativeTo = 0;
    bool mbHasHeaderRow = false;
    bool mbShowHeaders = false;
    TotalLayout maRowTotals{};
    TotalLayout maColumnTotals{};
    SortOrder maRowSortOrder;
    SortOrder maColumnSortOrder;
    /// The input rows that the filter keeps, without the header row.
    std::vector<SCSIZE> maRows;
    std::vector<GroupNode> maRowNodes;
    std::vector<GroupNode> maColumnNodes;
    bool mbTakesAllValues = false;
    /// Each cell holds the function result of the rows that a row group and a column group share,
    /// one cell per value column. Its rows are the row nodes, and its columns the column nodes
    /// times the value columns.
    ScMatrixRef mxCells;
    SCSIZE mnCellColumns = 0;
    std::vector<bool> maCellComputed;
    bool mbMatrixSizeError = false;
    bool mbArrayResult = false;
};

bool PivotByFunction::ReadArguments(Pivot& rPivot)
{
    sal_uInt8 nParameterCount = mrInterpreter.GetByte();
    if (!mrInterpreter.MustHaveParamCount(nParameterCount, 4, 11))
        return false;

    if (nParameterCount >= 11)
        rPivot.mnRelativeTo = mrInterpreter.GetInt32WithDefault(0);
    ScMatrixRef xFilter;
    if (nParameterCount >= 10)
        xFilter = PopOptionalMatrix();
    ScMatrixRef xColumnSortOrder;
    if (nParameterCount >= 9)
        xColumnSortOrder = PopOptionalMatrix();
    std::optional<sal_Int32> oColumnTotalDepth;
    if (nParameterCount >= 8)
        oColumnTotalDepth = PopOptionalInt32();
    ScMatrixRef xRowSortOrder;
    if (nParameterCount >= 7)
        xRowSortOrder = PopOptionalMatrix();
    std::optional<sal_Int32> oRowTotalDepth;
    if (nParameterCount >= 6)
        oRowTotalDepth = PopOptionalInt32();
    std::optional<sal_Int32> oFieldHeaders;
    if (nParameterCount >= 5)
        oFieldHeaders = PopOptionalInt32();
    rPivot.mpCallable = mrInterpreter.GetCallable();
    rPivot.mxValues = mrInterpreter.GetMatrix();
    if (mrInterpreter.IsMissing())
        mrInterpreter.Pop();
    else
        rPivot.mxColumnKeys = GetKeyMatrix();
    if (mrInterpreter.IsMissing())
        mrInterpreter.Pop();
    else
        rPivot.mxRowKeys = GetKeyMatrix();

    if (mrInterpreter.nGlobalError != FormulaError::NONE)
    {
        mrInterpreter.PushError(mrInterpreter.nGlobalError);
        return false;
    }
    if (!rPivot.mpCallable || !rPivot.mxValues)
    {
        mrInterpreter.PushIllegalArgument();
        return false;
    }

    // Either the row keys or the column keys may be left out, and each must have a row for every
    // row of the values.
    SCSIZE nValueRows;
    rPivot.mxValues->GetDimensions(rPivot.mnValueColumns, nValueRows);
    bool bValid = (rPivot.mxRowKeys || rPivot.mxColumnKeys) && nValueRows > 0
                  && rPivot.mnValueColumns > 0
                  && (!oFieldHeaders || (*oFieldHeaders >= 0 && *oFieldHeaders <= 3))
                  && rPivot.mnRelativeTo >= 0 && rPivot.mnRelativeTo <= 4;
    if (rPivot.mxRowKeys)
    {
        SCSIZE nRows;
        rPivot.mxRowKeys->GetDimensions(rPivot.mnRowKeyColumns, nRows);
        bValid = bValid && nRows == nValueRows;
    }
    if (rPivot.mxColumnKeys)
    {
        SCSIZE nRows;
        rPivot.mxColumnKeys->GetDimensions(rPivot.mnColumnKeyColumns, nRows);
        bValid = bValid && nRows == nValueRows;
    }
    if (!bValid)
    {
        mrInterpreter.PushNoValue();
        return false;
    }

    // Field headers work as in GROUPBY. When the argument is left out, the detected header row is
    // shown when the rows or the columns have more than one key level.
    rPivot.mbHasHeaderRow = oFieldHeaders ? *oFieldHeaders == 1 || *oFieldHeaders == 3
                                          : startsWithHeaderRow(*rPivot.mxValues);
    rPivot.mbShowHeaders
        = oFieldHeaders ? *oFieldHeaders >= 2
                        : rPivot.mbHasHeaderRow
                              && (rPivot.mnRowKeyColumns > 1 || rPivot.mnColumnKeyColumns > 1);

    const std::optional<TotalLayout> oRowTotals
        = getTotalLayout(oRowTotalDepth, rPivot.mnRowKeyColumns, true);
    const std::optional<TotalLayout> oColumnTotals
        = getTotalLayout(oColumnTotalDepth, rPivot.mnColumnKeyColumns, true);
    const std::optional<SortOrder> oRowSortOrder
        = getSortOrder(xRowSortOrder.get(), rPivot.mnRowKeyColumns, rPivot.mnValueColumns);
    const std::optional<SortOrder> oColumnSortOrder
        = getSortOrder(xColumnSortOrder.get(), rPivot.mnColumnKeyColumns, rPivot.mnValueColumns);
    if (!selectRows(xFilter.get(), nValueRows, rPivot.mbHasHeaderRow, rPivot.maRows)
        || rPivot.maRows.empty() || !oRowTotals || !oColumnTotals || !oRowSortOrder
        || !oColumnSortOrder)
    {
        mrInterpreter.PushNoValue();
        return false;
    }
    rPivot.maRowTotals = *oRowTotals;
    rPivot.maColumnTotals = *oColumnTotals;
    rPivot.maRowSortOrder = *oRowSortOrder;
    rPivot.maColumnSortOrder = *oColumnSortOrder;
    return true;
}

SCSIZE PivotByFunction::ComputeCell(Pivot& rPivot, size_t nRowNode, size_t nColumnNode,
                                    SCSIZE nValueColumn)
{
    const SCSIZE nCellColumn = nColumnNode * rPivot.mnValueColumns + nValueColumn;
    const size_t nCellIndex = nRowNode * rPivot.mnCellColumns + nCellColumn;
    if (rPivot.maCellComputed[nCellIndex])
        return nCellColumn;
    rPivot.maCellComputed[nCellIndex] = true;

    // A row group and a column group without shared rows give empty text.
    const std::vector<GroupNode>& rRowNodes = rPivot.maRowNodes;
    const std::vector<GroupNode>& rColumnNodes = rPivot.maColumnNodes;
    const std::vector<SCSIZE> aCellRows
        = intersectRows(rRowNodes[nRowNode].maRows, rColumnNodes[nColumnNode].maRows);
    if (aCellRows.empty())
    {
        rPivot.mxCells->PutString(svl::SharedString::getEmptyString(), nCellColumn, nRowNode);
        return nCellColumn;
    }
    ScMatrixRef xGroupValues = GetGroupValues(rPivot.mxValues, nValueColumn, aCellRows);
    ScMatrixRef xAllValues;
    if (rPivot.mbTakesAllValues)
    {
        size_t nTotalRowNode = nRowNode;
        size_t nTotalColumnNode = nColumnNode;
        switch (rPivot.mnRelativeTo)
        {
            case 0:
                nTotalRowNode = 0;
                break;
            case 1:
                nTotalColumnNode = 0;
                break;
            case 2:
                nTotalRowNode = 0;
                nTotalColumnNode = 0;
                break;
            case 3:
                nTotalColumnNode = rColumnNodes[nColumnNode].mnParent;
                break;
            default:
                nTotalRowNode = rRowNodes[nRowNode].mnParent;
                break;
        }
        xAllValues = GetGroupValues(
            rPivot.mxValues, nValueColumn,
            intersectRows(rRowNodes[nTotalRowNode].maRows, rColumnNodes[nTotalColumnNode].maRows));
    }
    if (!xGroupValues || (rPivot.mbTakesAllValues && !xAllValues))
    {
        rPivot.mbMatrixSizeError = true;
        return nCellColumn;
    }
    if (!PutGroupAggregate(rPivot.mpCallable, xGroupValues, xAllValues, rPivot.mxCells, nCellColumn,
                           nRowNode))
        rPivot.mbArrayResult = true;
    return nCellColumn;
}

void PivotByFunction::SortGroups(Pivot& rPivot)
{
    // A value sort orders the rows by their cell in the grand total column and the columns by their
    // cell in the grand total row. When that total is turned off, the groups keep the order of
    // their keys. Without column keys the only column is the grand total column, and without row
    // keys the only row is the grand total row.
    const ScMatrix& rCells = *rPivot.mxCells;
    if (rPivot.mxRowKeys)
    {
        const SortOrder& rSortOrder = rPivot.maRowSortOrder;
        std::function<sal_Int32(size_t, size_t)> aCompareValues;
        if (rSortOrder.moValueSort && (!rPivot.mxColumnKeys || rPivot.maColumnTotals.mbGrandTotal))
        {
            const SCSIZE nValueColumn = rSortOrder.moValueSort->first;
            const bool bAscending = rSortOrder.moValueSort->second;
            aCompareValues = [&, nValueColumn, bAscending](size_t nLeft, size_t nRight)
            {
                const SCSIZE nCellColumn = ComputeCell(rPivot, nLeft, 0, nValueColumn);
                ComputeCell(rPivot, nRight, 0, nValueColumn);
                return compareCellsForSort(rCells, nCellColumn, nLeft, rCells, nCellColumn, nRight,
                                           bAscending);
            };
        }
        sortHierarchy(rPivot.maRowNodes, *rPivot.mxRowKeys, rSortOrder.maKeyAscending,
                      aCompareValues);
    }
    if (rPivot.mxColumnKeys)
    {
        const SortOrder& rSortOrder = rPivot.maColumnSortOrder;
        std::function<sal_Int32(size_t, size_t)> aCompareValues;
        if (rSortOrder.moValueSort && (!rPivot.mxRowKeys || rPivot.maRowTotals.mbGrandTotal))
        {
            const SCSIZE nValueColumn = rSortOrder.moValueSort->first;
            const bool bAscending = rSortOrder.moValueSort->second;
            aCompareValues = [&, nValueColumn, bAscending](size_t nLeft, size_t nRight)
            {
                const SCSIZE nLeftColumn = ComputeCell(rPivot, 0, nLeft, nValueColumn);
                const SCSIZE nRightColumn = ComputeCell(rPivot, 0, nRight, nValueColumn);
                return compareCellsForSort(rCells, nLeftColumn, 0, rCells, nRightColumn, 0,
                                           bAscending);
            };
        }
        sortHierarchy(rPivot.maColumnNodes, *rPivot.mxColumnKeys, rSortOrder.maKeyAscending,
                      aCompareValues);
    }
}

void PivotByFunction::PutHeaderRows(const Pivot& rPivot, const std::vector<size_t>& rColumnOutput,
                                    bool bValueHeaderRow, const ScMatrixRef& xOutput)
{
    // Each column group spans one output column per value column, and every one of them shows the
    // keys of the group. The grand total column shows its label in the first header row.
    svl::SharedStringPool& rStringPool = mrInterpreter.mrStrPool;
    const SCSIZE nValueColumns = rPivot.mnValueColumns;
    const SCSIZE nRowKeyColumns = rPivot.mnRowKeyColumns;
    const SCSIZE nColumnKeyColumns = rPivot.mnColumnKeyColumns;
    const svl::SharedString aColumnTotalLabel = rStringPool.intern(ScResId(
        rPivot.maColumnTotals.mnSubtotalLevels > 0 ? STR_GROUPBY_GRAND_TOTAL : STR_GROUPBY_TOTAL));
    for (SCSIZE nIndex = 0; nIndex < rColumnOutput.size(); ++nIndex)
    {
        const size_t nColumnNode = rColumnOutput[nIndex];
        const std::vector<SCSIZE> aKeyRows = getKeyRows(rPivot.maColumnNodes, nColumnNode);
        for (SCSIZE nValueColumn = 0; nValueColumn < nValueColumns; ++nValueColumn)
        {
            const SCSIZE nOutputColumn = nRowKeyColumns + nIndex * nValueColumns + nValueColumn;
            if (nColumnNode == 0 && nColumnKeyColumns > 0)
                xOutput->PutString(aColumnTotalLabel, nOutputColumn, 0);
            for (SCSIZE nLevel = 0; nLevel < aKeyRows.size(); ++nLevel)
                FillKeyCell(rPivot.mxColumnKeys, xOutput, nLevel, aKeyRows[nLevel], nOutputColumn,
                            nLevel);
            if (!bValueHeaderRow)
                continue;
            if (rPivot.mbHasHeaderRow)
                ScInterpreter::FillMatrixCell(rPivot.mxValues, xOutput, nValueColumn, 0,
                                              nOutputColumn, nColumnKeyColumns);
            else
                xOutput->PutString(
                    rStringPool.intern(getGeneratedName(STR_GROUPBY_VALUE, nValueColumn)),
                    nOutputColumn, nColumnKeyColumns);
        }
    }
    if (rPivot.mbShowHeaders)
    {
        for (SCSIZE nColumn = 0; nColumn < nRowKeyColumns; ++nColumn)
        {
            if (rPivot.mbHasHeaderRow)
                FillKeyCell(rPivot.mxRowKeys, xOutput, nColumn, 0, nColumn, nColumnKeyColumns);
            else
                xOutput->PutString(
                    rStringPool.intern(getGeneratedName(STR_GROUPBY_ROW_FIELD, nColumn)), nColumn,
                    nColumnKeyColumns);
        }
    }
}

void PivotByFunction::PutGroupRows(Pivot& rPivot, const std::vector<size_t>& rRowOutput,
                                   const std::vector<size_t>& rColumnOutput,
                                   const ScMatrixRef& xOutput, SCSIZE nFirstRow)
{
    // Each row shows the keys of its row group, a subtotal row only the keys that its group fixes.
    // The grand total row shows its label in the first key column.
    const SCSIZE nValueColumns = rPivot.mnValueColumns;
    const SCSIZE nRowKeyColumns = rPivot.mnRowKeyColumns;
    const svl::SharedString aRowTotalLabel = mrInterpreter.mrStrPool.intern(ScResId(
        rPivot.maRowTotals.mnSubtotalLevels > 0 ? STR_GROUPBY_GRAND_TOTAL : STR_GROUPBY_TOTAL));
    for (SCSIZE nRowIndex = 0; nRowIndex < rRowOutput.size(); ++nRowIndex)
    {
        const size_t nRowNode = rRowOutput[nRowIndex];
        const SCSIZE nOutputRow = nFirstRow + nRowIndex;
        if (nRowNode == 0 && nRowKeyColumns > 0)
            xOutput->PutString(aRowTotalLabel, 0, nOutputRow);
        const std::vector<SCSIZE> aKeyRows = getKeyRows(rPivot.maRowNodes, nRowNode);
        for (SCSIZE nColumn = 0; nColumn < aKeyRows.size(); ++nColumn)
            FillKeyCell(rPivot.mxRowKeys, xOutput, nColumn, aKeyRows[nColumn], nColumn, nOutputRow);
        for (SCSIZE nColumnIndex = 0; nColumnIndex < rColumnOutput.size(); ++nColumnIndex)
        {
            for (SCSIZE nValueColumn = 0; nValueColumn < nValueColumns; ++nValueColumn)
            {
                const SCSIZE nCellColumn
                    = ComputeCell(rPivot, nRowNode, rColumnOutput[nColumnIndex], nValueColumn);
                ScInterpreter::FillMatrixCell(
                    rPivot.mxCells, xOutput, nCellColumn, nRowNode,
                    nRowKeyColumns + nColumnIndex * nValueColumns + nValueColumn, nOutputRow);
            }
        }
    }
}

void PivotByFunction::Execute()
{
    Pivot aPivot;
    if (!ReadArguments(aPivot))
        return;
    aPivot.maRowNodes = buildGroups(aPivot.mxRowKeys.get(), aPivot.maRows);
    aPivot.maColumnNodes = buildGroups(aPivot.mxColumnKeys.get(), aPivot.maRows);

    // The cells are computed when the output or a sort needs them.
    aPivot.mnCellColumns = aPivot.maColumnNodes.size() * aPivot.mnValueColumns;
    aPivot.mxCells
        = mrInterpreter.GetNewMat(aPivot.mnCellColumns, aPivot.maRowNodes.size(), /*bEmpty*/ true);
    if (!aPivot.mxCells)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return;
    }
    aPivot.maCellComputed.assign(aPivot.mnCellColumns * aPivot.maRowNodes.size(), false);
    aPivot.mbTakesAllValues = takesAllValues(*aPivot.mpCallable);

    SortGroups(aPivot);
    const std::vector<size_t> aRowOutput = listHierarchy(aPivot.maRowNodes, aPivot.maRowTotals);
    const std::vector<size_t> aColumnOutput
        = listHierarchy(aPivot.maColumnNodes, aPivot.maColumnTotals);

    // The header rows hold one row per column key level, then the names of the value columns when
    // there are several value columns or the headers are shown. The header cells without a name
    // hold empty text, apart from the names of the row keys when the headers are shown.
    const bool bValueHeaderRow = aPivot.mbShowHeaders || aPivot.mnValueColumns > 1;
    const SCSIZE nHeaderRows = aPivot.mnColumnKeyColumns + (bValueHeaderRow ? 1 : 0);
    const SCSIZE nOutputColumns
        = aPivot.mnRowKeyColumns + aColumnOutput.size() * aPivot.mnValueColumns;
    const SCSIZE nOutputRows = nHeaderRows + aRowOutput.size();
    ScMatrixRef xOutput = mrInterpreter.GetNewMat(nOutputColumns, nOutputRows, /*bEmpty*/ true);
    if (!xOutput)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return;
    }
    // Every cell starts as empty text. Only an empty key leaves an empty cell.
    for (SCSIZE nRow = 0; nRow < nOutputRows; ++nRow)
    {
        for (SCSIZE nColumn = 0; nColumn < nOutputColumns; ++nColumn)
            xOutput->PutString(svl::SharedString::getEmptyString(), nColumn, nRow);
    }
    PutHeaderRows(aPivot, aColumnOutput, bValueHeaderRow, xOutput);
    PutGroupRows(aPivot, aRowOutput, aColumnOutput, xOutput, nHeaderRows);

    if (aPivot.mbMatrixSizeError)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return;
    }
    // A function gives one value per cell. An array result has no cell to go to, and the whole
    // result is the #CALC! error.
    if (aPivot.mbArrayResult)
    {
        mrInterpreter.PushError(FormulaError::Calculation);
        return;
    }

    // The result takes no number format from the input or from the function.
    mrInterpreter.nFuncFmtType = SvNumFormatType::NUMBER;
    mrInterpreter.PushMatrix(xOutput);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
