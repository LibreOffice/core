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

#include <GroupByFunction.hxx>

#include <interpre.hxx>
#include <globstr.hrc>
#include <scmatrix.hxx>
#include <scresid.hxx>

#include <svl/sharedstringpool.hxx>

#include <algorithm>
#include <functional>
#include <optional>
#include <vector>

using namespace formula;
using namespace sc::grouping;

namespace sc
{
struct GroupByFunction::Grouping
{
    FunctionList maFunctions;
    /// The function and the value column of each result column.
    std::vector<std::pair<size_t, SCSIZE>> maResultColumns;
    ScMatrixRef mxKeys;
    ScMatrixRef mxValues;
    SCSIZE mnKeyColumns = 0;
    SCSIZE mnValueColumns = 0;
    /// Field relationship 1: the groups sort as one flat table, and only the grand total is
    /// allowed.
    bool mbTable = false;
    bool mbHasHeaderRow = false;
    bool mbShowHeaders = false;
    SortOrder maSortOrder;
    TotalLayout maTotals{};
    /// The input rows that the filter keeps, without the header row.
    std::vector<SCSIZE> maRows;
    std::vector<GroupNode> maNodes;
    /// The result of each function for each group node, one column per result column. A group
    /// whose results are not needed holds empty elements.
    ScMatrixRef mxAggregates;

    // The layout of the result. Functions in a row add a header row with their labels, above the
    // field names when those are shown. Functions in a column give every group one row per
    // result, with the labels in a column after the keys. On several value columns the shown field
    // names go in a column of their own after the labels, one per row.
    bool mbFunctionRow = false;
    SCSIZE mnFieldRow = 0;
    SCSIZE mnHeaderRows = 0;
    SCSIZE mnRowsPerGroup = 1;
    SCSIZE mnLabelColumns = 0;
    SCSIZE mnValueNameColumns = 0;
    SCSIZE mnFirstResultColumn = 0;
    SCSIZE mnOutputResultColumns = 0;
    SCSIZE mnOutputColumns = 0;

    /// Sets the layout of the result from the arguments.
    void ComputeLayout()
    {
        const bool bInRows = maFunctions.mbInRows;
        const SCSIZE nResultColumns = maResultColumns.size();
        mbFunctionRow = !bInRows && maFunctions.mbShowLabels;
        mnFieldRow = mbFunctionRow ? 1 : 0;
        mnHeaderRows = mnFieldRow + (mbShowHeaders ? 1 : 0);
        mnRowsPerGroup = bInRows ? nResultColumns : 1;
        mnLabelColumns = bInRows && maFunctions.mbShowLabels ? 1 : 0;
        mnValueNameColumns = bInRows && mnValueColumns > 1 && mbShowHeaders ? 1 : 0;
        mnFirstResultColumn = mnKeyColumns + mnLabelColumns + mnValueNameColumns;
        mnOutputResultColumns = bInRows ? 1 : nResultColumns;
        mnOutputColumns = mnFirstResultColumn + mnOutputResultColumns;
    }
};

bool GroupByFunction::ReadArguments(Grouping& rGrouping)
{
    sal_uInt8 nParameterCount = mrInterpreter.GetByte();
    if (!mrInterpreter.MustHaveParamCount(nParameterCount, 3, 8))
        return false;

    sal_Int32 nRelationship = 0;
    if (nParameterCount >= 8)
        nRelationship = mrInterpreter.GetInt32WithDefault(0);
    ScMatrixRef xFilter;
    if (nParameterCount >= 7)
        xFilter = PopOptionalMatrix();
    ScMatrixRef xSortOrder;
    if (nParameterCount >= 6)
        xSortOrder = PopOptionalMatrix();
    std::optional<sal_Int32> oTotalDepth;
    if (nParameterCount >= 5)
        oTotalDepth = PopOptionalInt32();
    std::optional<sal_Int32> oFieldHeaders;
    if (nParameterCount >= 4)
        oFieldHeaders = PopOptionalInt32();
    FormulaCallableRef pCallable = mrInterpreter.GetCallable();
    rGrouping.mxValues = mrInterpreter.GetMatrix();
    rGrouping.mxKeys = GetKeyMatrix();

    if (mrInterpreter.nGlobalError != FormulaError::NONE)
    {
        mrInterpreter.PushError(mrInterpreter.nGlobalError);
        return false;
    }
    if (!pCallable || !rGrouping.mxKeys || !rGrouping.mxValues)
    {
        mrInterpreter.PushIllegalArgument();
        return false;
    }

    const std::optional<FunctionList> oFunctions = getFunctionList(pCallable);
    SCSIZE nKeyRows, nValueRows;
    rGrouping.mxKeys->GetDimensions(rGrouping.mnKeyColumns, nKeyRows);
    rGrouping.mxValues->GetDimensions(rGrouping.mnValueColumns, nValueRows);
    std::optional<std::vector<std::pair<size_t, SCSIZE>>> oResultColumns;
    if (oFunctions)
        oResultColumns = getResultColumns(*oFunctions, rGrouping.mnValueColumns);
    if (!oResultColumns || nKeyRows != nValueRows || nKeyRows == 0 || rGrouping.mnKeyColumns == 0
        || rGrouping.mnValueColumns == 0
        || (oFieldHeaders && (*oFieldHeaders < 0 || *oFieldHeaders > 3))
        || (nRelationship != 0 && nRelationship != 1))
    {
        mrInterpreter.PushNoValue();
        return false;
    }
    rGrouping.mbTable = nRelationship == 1;
    rGrouping.maFunctions = *oFunctions;
    rGrouping.maResultColumns = *oResultColumns;

    // Field headers: 0 means no header row, 1 a header row that is not shown, 2 no header row with
    // generated names shown, and 3 a header row that is shown. When the argument is left out, the
    // header row is detected and not shown.
    rGrouping.mbHasHeaderRow = oFieldHeaders ? *oFieldHeaders == 1 || *oFieldHeaders == 3
                                             : startsWithHeaderRow(*rGrouping.mxValues);
    rGrouping.mbShowHeaders = oFieldHeaders && *oFieldHeaders >= 2;

    // A value sort refers to the results of the first function when the functions lie in a column,
    // and to every result column when they lie in a row.
    const SCSIZE nSortResultColumns
        = rGrouping.maFunctions.mbInRows ? 1 : rGrouping.maResultColumns.size();
    const std::optional<SortOrder> oSortOrder
        = getSortOrder(xSortOrder.get(), rGrouping.mnKeyColumns, nSortResultColumns);
    // A value sort in a hierarchy orders the groups of every level by their subtotals. Without a
    // total depth those subtotals are shown, below their groups.
    if (!oTotalDepth && oSortOrder && oSortOrder->moValueSort && !rGrouping.mbTable)
        oTotalDepth = sal_Int32(rGrouping.mnKeyColumns);
    const std::optional<TotalLayout> oTotals
        = getTotalLayout(oTotalDepth, rGrouping.mnKeyColumns, !rGrouping.mbTable);
    if (!selectRows(xFilter.get(), nKeyRows, rGrouping.mbHasHeaderRow, rGrouping.maRows)
        || rGrouping.maRows.empty() || !oTotals || !oSortOrder)
    {
        mrInterpreter.PushNoValue();
        return false;
    }
    rGrouping.maSortOrder = *oSortOrder;
    rGrouping.maTotals = *oTotals;
    return true;
}

bool GroupByFunction::ComputeAggregates(Grouping& rGrouping)
{
    const std::vector<GroupNode>& rNodes = rGrouping.maNodes;
    rGrouping.mxAggregates
        = mrInterpreter.GetNewMat(rGrouping.maResultColumns.size(), rNodes.size(), /*bEmpty*/ true);
    if (!rGrouping.mxAggregates)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return false;
    }

    // A value sort in a hierarchy orders every level by its subtotal, also where the subtotal is
    // not shown.
    const TotalLayout& rTotals = rGrouping.maTotals;
    const bool bSortHierarchyByValue = rGrouping.maSortOrder.moValueSort && !rGrouping.mbTable;
    auto aIsAggregateNeeded = [&](const GroupNode& rNode)
    {
        if (rNode.mnLevel == rGrouping.mnKeyColumns)
            return true;
        if (rNode.mnLevel == 0)
            return rTotals.mbGrandTotal;
        return rNode.mnLevel <= rTotals.mnSubtotalLevels || bSortHierarchyByValue;
    };
    for (SCSIZE nColumn = 0; nColumn < rGrouping.mnValueColumns; ++nColumn)
    {
        ScMatrixRef xAllValues = GetGroupValues(rGrouping.mxValues, nColumn, rGrouping.maRows);
        if (!xAllValues)
        {
            mrInterpreter.PushError(FormulaError::MatrixSize);
            return false;
        }
        for (size_t nNode = 0; nNode < rNodes.size(); ++nNode)
        {
            if (!aIsAggregateNeeded(rNodes[nNode]))
                continue;
            ScMatrixRef xGroupValues
                = nNode == 0 ? xAllValues
                             : GetGroupValues(rGrouping.mxValues, nColumn, rNodes[nNode].maRows);
            if (!xGroupValues)
            {
                mrInterpreter.PushError(FormulaError::MatrixSize);
                return false;
            }
            for (SCSIZE nResultColumn = 0; nResultColumn < rGrouping.maResultColumns.size();
                 ++nResultColumn)
            {
                const auto& [nFunction, nValueColumn] = rGrouping.maResultColumns[nResultColumn];
                if (nValueColumn != nColumn)
                    continue;
                const FormulaCallableRef& pFunction = rGrouping.maFunctions.maFunctions[nFunction];
                // A function gives one value per group. An array result has no cell to go to, and
                // the whole result is the #CALC! error.
                if (!PutGroupAggregate(pFunction, xGroupValues,
                                       takesAllValues(*pFunction) ? xAllValues : nullptr,
                                       rGrouping.mxAggregates, nResultColumn, nNode))
                {
                    mrInterpreter.PushError(FormulaError::Calculation);
                    return false;
                }
            }
        }
    }
    return true;
}

std::vector<size_t> GroupByFunction::ListOutputNodes(Grouping& rGrouping)
{
    const ScMatrix& rKeys = *rGrouping.mxKeys;
    const ScMatrix& rAggregates = *rGrouping.mxAggregates;
    const SortOrder& rSortOrder = rGrouping.maSortOrder;
    const SCSIZE nKeyColumns = rGrouping.mnKeyColumns;
    std::vector<GroupNode>& rNodes = rGrouping.maNodes;
    auto aCompareKeys = [&](size_t nLeft, size_t nRight, SCSIZE nColumn, bool bAscending)
    {
        return compareCellsForSort(rKeys, nColumn, rNodes[nLeft].maRows[0], rKeys, nColumn,
                                   rNodes[nRight].maRows[0], bAscending);
    };
    auto aCompareAggregates = [&](size_t nLeft, size_t nRight, SCSIZE nColumn, bool bAscending)
    {
        return compareCellsForSort(rAggregates, nColumn, nLeft, rAggregates, nColumn, nRight,
                                   bAscending);
    };

    if (!rGrouping.mbTable)
    {
        std::function<sal_Int32(size_t, size_t)> aCompareValues;
        if (rSortOrder.moValueSort)
        {
            const SCSIZE nResultColumn = rSortOrder.moValueSort->first;
            const bool bAscending = rSortOrder.moValueSort->second;
            aCompareValues
                = [&aCompareAggregates, nResultColumn, bAscending](size_t nLeft, size_t nRight)
            { return aCompareAggregates(nLeft, nRight, nResultColumn, bAscending); };
        }
        sortHierarchy(rNodes, rKeys, rSortOrder.maKeyAscending, aCompareValues);
        return listHierarchy(rNodes, rGrouping.maTotals);
    }

    // In a table every key column sorts on its own, so the groups sort as one list by the sort
    // entries in their order. Groups that the sort entries leave equal follow the key columns from
    // right to left. A sort of the whole list by each key column in turn, and then by the sort
    // entries, gives the same order. Without sort entries the key columns order the list from left
    // to right.
    std::vector<size_t> aLeaves;
    for (size_t nNode = 0; nNode < rNodes.size(); ++nNode)
    {
        if (rNodes[nNode].mnLevel == nKeyColumns)
            aLeaves.push_back(nNode);
    }
    auto aLessInTable = [&](size_t nLeft, size_t nRight)
    {
        for (const auto& [nColumn, bAscending] : rSortOrder.maColumns)
        {
            const sal_Int32 nResult
                = nColumn < nKeyColumns
                      ? aCompareKeys(nLeft, nRight, nColumn, bAscending)
                      : aCompareAggregates(nLeft, nRight, nColumn - nKeyColumns, bAscending);
            if (nResult != 0)
                return nResult < 0;
        }
        const bool bRightToLeft = !rSortOrder.maColumns.empty();
        for (SCSIZE nIndex = 0; nIndex < nKeyColumns; ++nIndex)
        {
            const SCSIZE nColumn = bRightToLeft ? nKeyColumns - 1 - nIndex : nIndex;
            const sal_Int32 nResult
                = aCompareKeys(nLeft, nRight, nColumn, rSortOrder.maKeyAscending[nColumn]);
            if (nResult != 0)
                return nResult < 0;
        }
        return false;
    };
    std::stable_sort(aLeaves.begin(), aLeaves.end(), aLessInTable);

    const TotalLayout& rTotals = rGrouping.maTotals;
    std::vector<size_t> aOutputNodes;
    if (rTotals.mbGrandTotal && rTotals.mbTotalsAtTop)
        aOutputNodes.push_back(0);
    aOutputNodes.insert(aOutputNodes.end(), aLeaves.begin(), aLeaves.end());
    if (rTotals.mbGrandTotal && !rTotals.mbTotalsAtTop)
        aOutputNodes.push_back(0);
    return aOutputNodes;
}

void GroupByFunction::PutValueName(const Grouping& rGrouping, SCSIZE nValueColumn,
                                   const ScMatrixRef& xOutput, SCSIZE nColumn, SCSIZE nRow)
{
    if (rGrouping.mbHasHeaderRow)
        ScInterpreter::FillMatrixCell(rGrouping.mxValues, xOutput, nValueColumn, 0, nColumn, nRow);
    else
        xOutput->PutString(
            mrInterpreter.mrStrPool.intern(getGeneratedName(STR_GROUPBY_VALUE, nValueColumn)),
            nColumn, nRow);
}

void GroupByFunction::PutHeaderRows(const Grouping& rGrouping, const ScMatrixRef& xOutput)
{
    // A header cell without a name holds empty text.
    for (SCSIZE nRow = 0; nRow < rGrouping.mnHeaderRows; ++nRow)
    {
        for (SCSIZE nColumn = 0; nColumn < rGrouping.mnOutputColumns; ++nColumn)
            xOutput->PutString(svl::SharedString::getEmptyString(), nColumn, nRow);
    }
    svl::SharedStringPool& rStringPool = mrInterpreter.mrStrPool;
    if (rGrouping.mbFunctionRow)
    {
        for (SCSIZE nColumn = 0; nColumn < rGrouping.maResultColumns.size(); ++nColumn)
            putFunctionLabel(rGrouping.maFunctions, rGrouping.maResultColumns[nColumn].first,
                             rStringPool, xOutput, rGrouping.mnFirstResultColumn + nColumn, 0);
    }
    if (!rGrouping.mbShowHeaders)
        return;
    const SCSIZE nFieldRow = rGrouping.mnFieldRow;
    for (SCSIZE nColumn = 0; nColumn < rGrouping.mnKeyColumns; ++nColumn)
    {
        if (rGrouping.mbHasHeaderRow)
            FillKeyCell(rGrouping.mxKeys, xOutput, nColumn, 0, nColumn, nFieldRow);
        else
            xOutput->PutString(rStringPool.intern(getGeneratedName(STR_GROUPBY_ROW_FIELD, nColumn)),
                               nColumn, nFieldRow);
    }
    // Functions in a column on several value columns give one result column with the results of
    // every value column, and its header keeps the empty text.
    if (rGrouping.maFunctions.mbInRows && rGrouping.mnValueColumns > 1)
        return;
    for (SCSIZE nColumn = 0; nColumn < rGrouping.mnOutputResultColumns; ++nColumn)
        PutValueName(rGrouping, rGrouping.maResultColumns[nColumn].second, xOutput,
                     rGrouping.mnFirstResultColumn + nColumn, nFieldRow);
}

void GroupByFunction::PutGroupRows(const Grouping& rGrouping,
                                   const std::vector<size_t>& rOutputNodes,
                                   const ScMatrixRef& xOutput)
{
    // A subtotal row shows the keys that its group fixes and empty text in the deeper key columns.
    // The grand total row shows its label in the first key column.
    const svl::SharedString aTotalLabel = mrInterpreter.mrStrPool.intern(ScResId(
        rGrouping.maTotals.mnSubtotalLevels > 0 ? STR_GROUPBY_GRAND_TOTAL : STR_GROUPBY_TOTAL));
    const svl::SharedString& rEmptyText = svl::SharedString::getEmptyString();
    const SCSIZE nKeyColumns = rGrouping.mnKeyColumns;
    const SCSIZE nRowsPerGroup = rGrouping.mnRowsPerGroup;
    for (SCSIZE nIndex = 0; nIndex < rOutputNodes.size(); ++nIndex)
    {
        const size_t nNode = rOutputNodes[nIndex];
        const std::vector<SCSIZE> aKeyRows = getKeyRows(rGrouping.maNodes, nNode);
        for (SCSIZE nRowInGroup = 0; nRowInGroup < nRowsPerGroup; ++nRowInGroup)
        {
            const SCSIZE nOutputRow = rGrouping.mnHeaderRows + nIndex * nRowsPerGroup + nRowInGroup;
            for (SCSIZE nColumn = 0; nColumn < aKeyRows.size(); ++nColumn)
                FillKeyCell(rGrouping.mxKeys, xOutput, nColumn, aKeyRows[nColumn], nColumn,
                            nOutputRow);
            for (SCSIZE nColumn = aKeyRows.size(); nColumn < nKeyColumns; ++nColumn)
                xOutput->PutString(rEmptyText, nColumn, nOutputRow);
            if (nNode == 0)
                xOutput->PutString(aTotalLabel, 0, nOutputRow);
            if (rGrouping.mnLabelColumns > 0)
                putFunctionLabel(rGrouping.maFunctions,
                                 rGrouping.maResultColumns[nRowInGroup].first,
                                 mrInterpreter.mrStrPool, xOutput, nKeyColumns, nOutputRow);
            if (rGrouping.mnValueNameColumns > 0)
                PutValueName(rGrouping, rGrouping.maResultColumns[nRowInGroup].second, xOutput,
                             nKeyColumns + rGrouping.mnLabelColumns, nOutputRow);
            const SCSIZE nFirstAggregate = rGrouping.maFunctions.mbInRows ? nRowInGroup : 0;
            for (SCSIZE nColumn = 0; nColumn < rGrouping.mnOutputResultColumns; ++nColumn)
                ScInterpreter::FillMatrixCell(rGrouping.mxAggregates, xOutput,
                                              nFirstAggregate + nColumn, nNode,
                                              rGrouping.mnFirstResultColumn + nColumn, nOutputRow);
        }
    }
}

void GroupByFunction::Execute()
{
    Grouping aGrouping;
    if (!ReadArguments(aGrouping))
        return;
    aGrouping.maNodes = buildGroups(aGrouping.mxKeys.get(), aGrouping.maRows);
    if (!ComputeAggregates(aGrouping))
        return;
    const std::vector<size_t> aOutputNodes = ListOutputNodes(aGrouping);

    aGrouping.ComputeLayout();
    ScMatrixRef xOutput = mrInterpreter.GetNewMat(
        aGrouping.mnOutputColumns,
        aGrouping.mnHeaderRows + aOutputNodes.size() * aGrouping.mnRowsPerGroup, /*bEmpty*/ true);
    if (!xOutput)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return;
    }
    PutHeaderRows(aGrouping, xOutput);
    PutGroupRows(aGrouping, aOutputNodes, xOutput);

    // The result takes no number format from the input or from the function.
    mrInterpreter.nFuncFmtType = SvNumFormatType::NUMBER;
    mrInterpreter.PushMatrix(xOutput);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
