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
    FunctionList maFunctions;
    /// The function and the value column of each result column.
    std::vector<std::pair<size_t, SCSIZE>> maResultColumns;
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
    /// Each cell holds a function result of the rows that a row group and a column group share, one
    /// cell per result column. Its rows are the row nodes, and its columns the column nodes times
    /// the result columns.
    ScMatrixRef mxCells;
    SCSIZE mnCellColumns = 0;
    std::vector<bool> maCellComputed;
    bool mbMatrixSizeError = false;
    bool mbArrayResult = false;

    // The layout of the result. The header rows hold one row per column key level, then the labels
    // of the functions when they lie in a row, then the names of the value columns when the
    // headers are shown, or when there are several value columns and no function labels set their
    // columns apart. With a single value column the function labels take the place of the value
    // names, as in GROUPBY. Functions in a column give every row group one row per result, with
    // the labels in a column after the row keys.
    bool mbFunctionRow = false;
    bool mbReplacedByFunctions = false;
    bool mbValueHeaderRow = false;
    SCSIZE mnFunctionRow = 0;
    SCSIZE mnValueHeaderRow = 0;
    SCSIZE mnHeaderRows = 0;
    SCSIZE mnRowsPerGroup = 1;
    SCSIZE mnLabelColumns = 0;
    SCSIZE mnFirstResultColumn = 0;
    SCSIZE mnColumnsPerGroup = 1;

    /// Sets the layout of the result from the arguments.
    void ComputeLayout()
    {
        const bool bInRows = maFunctions.mbInRows;
        const SCSIZE nResultColumns = maResultColumns.size();
        mbFunctionRow = !bInRows && maFunctions.mbShowLabels;
        mbReplacedByFunctions = mbFunctionRow && mnValueColumns == 1;
        mbValueHeaderRow
            = (mbShowHeaders || (mnValueColumns > 1 && !mbFunctionRow)) && !mbReplacedByFunctions;
        mnFunctionRow = mnColumnKeyColumns;
        mnValueHeaderRow = mnFunctionRow + (mbFunctionRow ? 1 : 0);
        mnHeaderRows = mnValueHeaderRow + (mbValueHeaderRow ? 1 : 0);
        mnRowsPerGroup = bInRows ? nResultColumns : 1;
        mnLabelColumns = bInRows && maFunctions.mbShowLabels ? 1 : 0;
        mnFirstResultColumn = mnRowKeyColumns + mnLabelColumns;
        mnColumnsPerGroup = bInRows ? 1 : nResultColumns;
    }
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
    FormulaCallableRef pCallable = mrInterpreter.GetCallable();
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
    if (!pCallable || !rPivot.mxValues)
    {
        mrInterpreter.PushIllegalArgument();
        return false;
    }

    // Either the row keys or the column keys may be left out, and each must have a row for every
    // row of the values.
    const std::optional<FunctionList> oFunctions = getFunctionList(pCallable);
    SCSIZE nValueRows;
    rPivot.mxValues->GetDimensions(rPivot.mnValueColumns, nValueRows);
    std::optional<std::vector<std::pair<size_t, SCSIZE>>> oResultColumns;
    if (oFunctions)
        oResultColumns = getResultColumns(*oFunctions, rPivot.mnValueColumns);
    bool bValid = oResultColumns && (rPivot.mxRowKeys || rPivot.mxColumnKeys) && nValueRows > 0
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

    rPivot.maFunctions = *oFunctions;
    rPivot.maResultColumns = *oResultColumns;

    // The functions apply to the value columns as in GROUPBY, and a value sort refers to the
    // results of the first function when the functions lie in a column.
    const SCSIZE nSortColumns = rPivot.maFunctions.mbInRows ? 1 : rPivot.maResultColumns.size();

    const std::optional<TotalLayout> oRowTotals
        = getTotalLayout(oRowTotalDepth, rPivot.mnRowKeyColumns, true);
    const std::optional<TotalLayout> oColumnTotals
        = getTotalLayout(oColumnTotalDepth, rPivot.mnColumnKeyColumns, true);
    const std::optional<SortOrder> oRowSortOrder
        = getSortOrder(xRowSortOrder.get(), rPivot.mnRowKeyColumns, nSortColumns);
    const std::optional<SortOrder> oColumnSortOrder
        = getSortOrder(xColumnSortOrder.get(), rPivot.mnColumnKeyColumns, nSortColumns);
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
                                    SCSIZE nResultColumn)
{
    const SCSIZE nCellColumn = nColumnNode * rPivot.maResultColumns.size() + nResultColumn;
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
    const auto& [nFunction, nValueColumn] = rPivot.maResultColumns[nResultColumn];
    const FormulaCallableRef& pFunction = rPivot.maFunctions.maFunctions[nFunction];
    const bool bTakesAllValues = takesAllValues(*pFunction);
    ScMatrixRef xGroupValues = GetGroupValues(rPivot.mxValues, nValueColumn, aCellRows);
    ScMatrixRef xAllValues;
    if (bTakesAllValues)
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
    if (!xGroupValues || (bTakesAllValues && !xAllValues))
    {
        rPivot.mbMatrixSizeError = true;
        return nCellColumn;
    }
    if (!PutGroupAggregate(pFunction, xGroupValues, xAllValues, rPivot.mxCells, nCellColumn,
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
            const SCSIZE nResultColumn = rSortOrder.moValueSort->first;
            const bool bAscending = rSortOrder.moValueSort->second;
            aCompareValues = [&, nResultColumn, bAscending](size_t nLeft, size_t nRight)
            {
                const SCSIZE nCellColumn = ComputeCell(rPivot, nLeft, 0, nResultColumn);
                ComputeCell(rPivot, nRight, 0, nResultColumn);
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
            const SCSIZE nResultColumn = rSortOrder.moValueSort->first;
            const bool bAscending = rSortOrder.moValueSort->second;
            aCompareValues = [&, nResultColumn, bAscending](size_t nLeft, size_t nRight)
            {
                const SCSIZE nLeftColumn = ComputeCell(rPivot, 0, nLeft, nResultColumn);
                const SCSIZE nRightColumn = ComputeCell(rPivot, 0, nRight, nResultColumn);
                return compareCellsForSort(rCells, nLeftColumn, 0, rCells, nRightColumn, 0,
                                           bAscending);
            };
        }
        sortHierarchy(rPivot.maColumnNodes, *rPivot.mxColumnKeys, rSortOrder.maKeyAscending,
                      aCompareValues);
    }
}

void PivotByFunction::PutHeaderRows(const Pivot& rPivot, const std::vector<size_t>& rColumnOutput,
                                    const ScMatrixRef& xOutput)
{
    // Each column group spans one output column per result, and every one of them shows the keys of
    // the group. The grand total column shows its label in the first header row.
    svl::SharedStringPool& rStringPool = mrInterpreter.mrStrPool;
    const SCSIZE nColumnKeyColumns = rPivot.mnColumnKeyColumns;
    const SCSIZE nColumnsPerGroup = rPivot.mnColumnsPerGroup;
    const svl::SharedString aColumnTotalLabel = rStringPool.intern(ScResId(
        rPivot.maColumnTotals.mnSubtotalLevels > 0 ? STR_GROUPBY_GRAND_TOTAL : STR_GROUPBY_TOTAL));
    for (SCSIZE nIndex = 0; nIndex < rColumnOutput.size(); ++nIndex)
    {
        const size_t nColumnNode = rColumnOutput[nIndex];
        const std::vector<SCSIZE> aKeyRows = getKeyRows(rPivot.maColumnNodes, nColumnNode);
        for (SCSIZE nColumnInGroup = 0; nColumnInGroup < nColumnsPerGroup; ++nColumnInGroup)
        {
            const SCSIZE nOutputColumn
                = rPivot.mnFirstResultColumn + nIndex * nColumnsPerGroup + nColumnInGroup;
            const SCSIZE nValueColumn = rPivot.maResultColumns[nColumnInGroup].second;
            if (nColumnNode == 0 && nColumnKeyColumns > 0)
                xOutput->PutString(aColumnTotalLabel, nOutputColumn, 0);
            for (SCSIZE nLevel = 0; nLevel < aKeyRows.size(); ++nLevel)
                FillKeyCell(rPivot.mxColumnKeys, xOutput, nLevel, aKeyRows[nLevel], nOutputColumn,
                            nLevel);
            if (rPivot.mbFunctionRow)
                putFunctionLabel(rPivot.maFunctions, rPivot.maResultColumns[nColumnInGroup].first,
                                 rStringPool, xOutput, nOutputColumn, rPivot.mnFunctionRow);
            if (!rPivot.mbValueHeaderRow)
                continue;
            // Functions in a column on several value columns give one column with the results of
            // every value column, under empty text.
            if (rPivot.maFunctions.mbInRows && rPivot.mnValueColumns > 1)
                xOutput->PutString(svl::SharedString::getEmptyString(), nOutputColumn,
                                   rPivot.mnValueHeaderRow);
            else if (rPivot.mbHasHeaderRow)
                ScInterpreter::FillMatrixCell(rPivot.mxValues, xOutput, nValueColumn, 0,
                                              nOutputColumn, rPivot.mnValueHeaderRow);
            else
                xOutput->PutString(
                    rStringPool.intern(getGeneratedName(STR_GROUPBY_VALUE, nValueColumn)),
                    nOutputColumn, rPivot.mnValueHeaderRow);
        }
    }
    if (rPivot.mbShowHeaders && !rPivot.mbReplacedByFunctions)
    {
        const SCSIZE nFieldRow = rPivot.mnHeaderRows - 1;
        for (SCSIZE nColumn = 0; nColumn < rPivot.mnRowKeyColumns; ++nColumn)
        {
            if (rPivot.mbHasHeaderRow)
                FillKeyCell(rPivot.mxRowKeys, xOutput, nColumn, 0, nColumn, nFieldRow);
            else
                xOutput->PutString(
                    rStringPool.intern(getGeneratedName(STR_GROUPBY_ROW_FIELD, nColumn)), nColumn,
                    nFieldRow);
        }
    }
}

void PivotByFunction::PutGroupRows(Pivot& rPivot, const std::vector<size_t>& rRowOutput,
                                   const std::vector<size_t>& rColumnOutput,
                                   const ScMatrixRef& xOutput)
{
    // Each row shows the keys of its row group, a subtotal row only the keys that its group fixes.
    // The grand total row shows its label in the first key column.
    const SCSIZE nRowKeyColumns = rPivot.mnRowKeyColumns;
    const SCSIZE nRowsPerGroup = rPivot.mnRowsPerGroup;
    const SCSIZE nColumnsPerGroup = rPivot.mnColumnsPerGroup;
    const svl::SharedString aRowTotalLabel = mrInterpreter.mrStrPool.intern(ScResId(
        rPivot.maRowTotals.mnSubtotalLevels > 0 ? STR_GROUPBY_GRAND_TOTAL : STR_GROUPBY_TOTAL));
    for (SCSIZE nRowIndex = 0; nRowIndex < rRowOutput.size(); ++nRowIndex)
    {
        const size_t nRowNode = rRowOutput[nRowIndex];
        const std::vector<SCSIZE> aKeyRows = getKeyRows(rPivot.maRowNodes, nRowNode);
        for (SCSIZE nRowInGroup = 0; nRowInGroup < nRowsPerGroup; ++nRowInGroup)
        {
            const SCSIZE nOutputRow = rPivot.mnHeaderRows + nRowIndex * nRowsPerGroup + nRowInGroup;
            if (nRowNode == 0 && nRowKeyColumns > 0)
                xOutput->PutString(aRowTotalLabel, 0, nOutputRow);
            for (SCSIZE nColumn = 0; nColumn < aKeyRows.size(); ++nColumn)
                FillKeyCell(rPivot.mxRowKeys, xOutput, nColumn, aKeyRows[nColumn], nColumn,
                            nOutputRow);
            if (rPivot.mnLabelColumns > 0)
                putFunctionLabel(rPivot.maFunctions, rPivot.maResultColumns[nRowInGroup].first,
                                 mrInterpreter.mrStrPool, xOutput, nRowKeyColumns, nOutputRow);
            const SCSIZE nFirstResult = rPivot.maFunctions.mbInRows ? nRowInGroup : 0;
            for (SCSIZE nColumnIndex = 0; nColumnIndex < rColumnOutput.size(); ++nColumnIndex)
            {
                for (SCSIZE nColumnInGroup = 0; nColumnInGroup < nColumnsPerGroup; ++nColumnInGroup)
                {
                    const SCSIZE nCellColumn
                        = ComputeCell(rPivot, nRowNode, rColumnOutput[nColumnIndex],
                                      nFirstResult + nColumnInGroup);
                    ScInterpreter::FillMatrixCell(rPivot.mxCells, xOutput, nCellColumn, nRowNode,
                                                  rPivot.mnFirstResultColumn
                                                      + nColumnIndex * nColumnsPerGroup
                                                      + nColumnInGroup,
                                                  nOutputRow);
                }
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
    aPivot.mnCellColumns = aPivot.maColumnNodes.size() * aPivot.maResultColumns.size();
    aPivot.mxCells
        = mrInterpreter.GetNewMat(aPivot.mnCellColumns, aPivot.maRowNodes.size(), /*bEmpty*/ true);
    if (!aPivot.mxCells)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return;
    }
    aPivot.maCellComputed.assign(aPivot.mnCellColumns * aPivot.maRowNodes.size(), false);

    SortGroups(aPivot);
    const std::vector<size_t> aRowOutput = listHierarchy(aPivot.maRowNodes, aPivot.maRowTotals);
    const std::vector<size_t> aColumnOutput
        = listHierarchy(aPivot.maColumnNodes, aPivot.maColumnTotals);

    aPivot.ComputeLayout();
    const SCSIZE nOutputColumns
        = aPivot.mnFirstResultColumn + aColumnOutput.size() * aPivot.mnColumnsPerGroup;
    const SCSIZE nOutputRows = aPivot.mnHeaderRows + aRowOutput.size() * aPivot.mnRowsPerGroup;
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
    PutHeaderRows(aPivot, aColumnOutput, xOutput);
    PutGroupRows(aPivot, aRowOutput, aColumnOutput, xOutput);

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
