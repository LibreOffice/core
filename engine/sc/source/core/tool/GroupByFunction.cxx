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
#include <callable.hxx>
#include <cellvalue.hxx>
#include <dociter.hxx>
#include <formulacell.hxx>
#include <global.hxx>
#include <globstr.hrc>
#include <scmatrix.hxx>
#include <scresid.hxx>
#include <token.hxx>

#include <svl/sharedstringpool.hxx>
#include <unotools/collatorwrapper.hxx>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

using namespace formula;

namespace
{
/// The sort class of a matrix element. Groups sort numbers first, then text, then logical values,
/// then errors, and empty cells last.
enum class CellClass
{
    Number,
    Text,
    Logical,
    Error,
    Empty
};

CellClass getCellClass(const ScMatrix& rMatrix, SCSIZE nColumn, SCSIZE nRow)
{
    if (rMatrix.IsEmpty(nColumn, nRow))
        return CellClass::Empty;
    if (rMatrix.IsStringOrEmpty(nColumn, nRow))
        return CellClass::Text;
    if (rMatrix.GetError(nColumn, nRow) != FormulaError::NONE)
        return CellClass::Error;
    if (rMatrix.IsBoolean(nColumn, nRow))
        return CellClass::Logical;
    return CellClass::Number;
}

/// Compares two matrix elements in ascending order. Text compares without case, so two elements
/// that compare equal belong to the same group.
sal_Int32 compareCells(const ScMatrix& rLeft, SCSIZE nLeftColumn, SCSIZE nLeftRow,
                       const ScMatrix& rRight, SCSIZE nRightColumn, SCSIZE nRightRow)
{
    const CellClass eLeft = getCellClass(rLeft, nLeftColumn, nLeftRow);
    const CellClass eRight = getCellClass(rRight, nRightColumn, nRightRow);
    if (eLeft != eRight)
        return eLeft < eRight ? -1 : 1;

    switch (eLeft)
    {
        case CellClass::Number:
        case CellClass::Logical:
        {
            const double fLeft = rLeft.GetDouble(nLeftColumn, nLeftRow);
            const double fRight = rRight.GetDouble(nRightColumn, nRightRow);
            if (fLeft < fRight)
                return -1;
            return fLeft > fRight ? 1 : 0;
        }
        case CellClass::Text:
            return ScGlobal::GetCollator(false).compareString(
                rLeft.GetString(nLeftColumn, nLeftRow).getString(),
                rRight.GetString(nRightColumn, nRightRow).getString());
        case CellClass::Error:
        {
            const auto nLeft = static_cast<sal_uInt16>(rLeft.GetError(nLeftColumn, nLeftRow));
            const auto nRight = static_cast<sal_uInt16>(rRight.GetError(nRightColumn, nRightRow));
            if (nLeft < nRight)
                return -1;
            return nLeft > nRight ? 1 : 0;
        }
        case CellClass::Empty:
            break;
    }
    return 0;
}

/// Compares two matrix elements for a sort in the given direction. Empty cells come last in both
/// directions.
sal_Int32 compareCellsForSort(const ScMatrix& rLeft, SCSIZE nLeftColumn, SCSIZE nLeftRow,
                              const ScMatrix& rRight, SCSIZE nRightColumn, SCSIZE nRightRow,
                              bool bAscending)
{
    const bool bLeftEmpty = rLeft.IsEmpty(nLeftColumn, nLeftRow);
    const bool bRightEmpty = rRight.IsEmpty(nRightColumn, nRightRow);
    if (bLeftEmpty || bRightEmpty)
    {
        if (bLeftEmpty == bRightEmpty)
            return 0;
        return bLeftEmpty ? 1 : -1;
    }
    const sal_Int32 nResult
        = compareCells(rLeft, nLeftColumn, nLeftRow, rRight, nRightColumn, nRightRow);
    return bAscending ? nResult : -nResult;
}

/// One group of rows that share the same values in the first mnLevel key columns.
struct GroupNode
{
    /// The number of key columns that the group fixes. Level 0 is the group of all rows.
    SCSIZE mnLevel;
    /// The group one level up, as an index into the node list. The group of all rows is its own
    /// parent.
    size_t mnParent;
    /// The rows of the group in input order. The first row spells the key of the group.
    std::vector<SCSIZE> maRows;
    /// The groups one level deeper, as indices into the node list.
    std::vector<size_t> maChildren;
};

/// Builds the tree of groups over the given rows of rKeys. Node 0 is the group of all rows. The
/// children of a node are in the order in which their key first appears.
std::vector<GroupNode> buildGroups(const ScMatrix& rKeys, const std::vector<SCSIZE>& rRows)
{
    SCSIZE nKeyColumns, nKeyRows;
    rKeys.GetDimensions(nKeyColumns, nKeyRows);

    using ChildLookup = std::map<SCSIZE, size_t, std::function<bool(SCSIZE, SCSIZE)>>;
    // For each node, its children are found by a row that has the same key cell. The lookup of a
    // leaf is never searched, so its column only has to be valid.
    auto makeLookup = [&rKeys, nKeyColumns](SCSIZE nColumn)
    {
        const SCSIZE nLookupColumn = std::min(nColumn, nKeyColumns - 1);
        return ChildLookup(
            [&rKeys, nLookupColumn](SCSIZE nLeftRow, SCSIZE nRightRow)
            {
                return compareCells(rKeys, nLookupColumn, nLeftRow, rKeys, nLookupColumn, nRightRow)
                       < 0;
            });
    };

    std::vector<GroupNode> aNodes{ { 0, 0, rRows, {} } };
    std::vector<ChildLookup> aLookups;
    aLookups.push_back(makeLookup(0));
    for (SCSIZE nRow : rRows)
    {
        size_t nNode = 0;
        for (SCSIZE nColumn = 0; nColumn < nKeyColumns; ++nColumn)
        {
            size_t nChild;
            auto aFound = aLookups[nNode].find(nRow);
            if (aFound != aLookups[nNode].end())
                nChild = aFound->second;
            else
            {
                nChild = aNodes.size();
                aNodes.push_back({ nColumn + 1, nNode, {}, {} });
                aNodes[nNode].maChildren.push_back(nChild);
                aLookups[nNode].emplace(nRow, nChild);
                aLookups.push_back(makeLookup(nColumn + 1));
            }
            aNodes[nChild].maRows.push_back(nRow);
            nNode = nChild;
        }
    }
    return aNodes;
}

/// The rows that spell the keys of a group, one for each key column that the group fixes. Each key
/// comes from the group at that level, so all rows of a group spell its key the same way.
std::vector<SCSIZE> getKeyRows(const std::vector<GroupNode>& rNodes, size_t nNode)
{
    std::vector<SCSIZE> aKeyRows(rNodes[nNode].mnLevel);
    for (size_t nAncestor = nNode; rNodes[nAncestor].mnLevel > 0;
         nAncestor = rNodes[nAncestor].mnParent)
        aKeyRows[rNodes[nAncestor].mnLevel - 1] = rNodes[nAncestor].maRows[0];
    return aKeyRows;
}

/// Whether the values start with a header row: text in the first row and a number in the second.
bool startsWithHeaderRow(const ScMatrix& rValues)
{
    SCSIZE nColumns, nRows;
    rValues.GetDimensions(nColumns, nRows);
    return nRows >= 2 && getCellClass(rValues, 0, 0) == CellClass::Text
           && getCellClass(rValues, 0, 1) == CellClass::Number;
}

/// Collects the rows after the header row that the filter keeps. The filter has one entry per input
/// row, including the header row, and its entry for the header row is ignored. An empty entry
/// leaves the row out like FALSE does. Returns false when the filter has another length or holds
/// text or an error.
bool selectRows(const ScMatrix* pFilter, SCSIZE nRows, bool bHasHeaderRow,
                std::vector<SCSIZE>& rSelectedRows)
{
    if (pFilter)
    {
        SCSIZE nFilterColumns, nFilterRows;
        pFilter->GetDimensions(nFilterColumns, nFilterRows);
        if (nFilterColumns != 1 || nFilterRows != nRows)
            return false;
    }
    for (SCSIZE nRow = bHasHeaderRow ? 1 : 0; nRow < nRows; ++nRow)
    {
        if (pFilter)
        {
            const CellClass eFilterClass = getCellClass(*pFilter, 0, nRow);
            if (eFilterClass == CellClass::Text || eFilterClass == CellClass::Error)
                return false;
            if (eFilterClass == CellClass::Empty || pFilter->GetDouble(0, nRow) == 0.0)
                continue;
        }
        rSelectedRows.push_back(nRow);
    }
    return true;
}

/// The totals of the result.
struct TotalLayout
{
    bool mbGrandTotal;
    /// Each total comes before the groups it sums.
    bool mbTotalsAtTop;
    /// The number of key columns, from the first one, whose groups get a subtotal.
    SCSIZE mnSubtotalLevels;
};

/// Reads a total depth: 0 is no total, 1 the grand total, 2 adds subtotals for the first key
/// column, and so on. A negative depth puts the totals at the top. Without the argument there is
/// only the grand total. Returns nothing when the depth asks for more levels than there are key
/// columns.
std::optional<TotalLayout> getTotalLayout(std::optional<sal_Int32> oTotalDepth, SCSIZE nKeyColumns,
                                          bool bSubtotalsAllowed)
{
    const sal_Int64 nTotalDepth = oTotalDepth.value_or(1);
    const sal_Int64 nDepth = std::abs(nTotalDepth);
    const sal_Int64 nMaximumDepth = bSubtotalsAllowed ? static_cast<sal_Int64>(nKeyColumns) : 1;
    if (nDepth > nMaximumDepth)
        return std::nullopt;
    return TotalLayout{ nDepth > 0, nTotalDepth < 0,
                        nDepth > 1 ? static_cast<SCSIZE>(nDepth - 1) : 0 };
}

/// A sort order of the result.
struct SortOrder
{
    /// The direction of each key column.
    std::vector<bool> maKeyAscending;
    /// The sort entries in their order, as 0-based columns: the key columns, then the value
    /// columns. Each is paired with whether it sorts ascending.
    std::vector<std::pair<SCSIZE, bool>> maColumns;
    /// The value column to sort by and its direction, when the only entry names a value column.
    std::optional<std::pair<SCSIZE, bool>> moValueSort;
};

/// Reads a sort order. Each entry is a 1-based column: the key columns, then the value columns. A
/// negative entry sorts in descending order. Several entries may only name key columns, and each
/// column at most once. Returns nothing for an invalid sort order.
std::optional<SortOrder> getSortOrder(const ScMatrix* pSortOrder, SCSIZE nKeyColumns,
                                      SCSIZE nValueColumns)
{
    SortOrder aSortOrder;
    aSortOrder.maKeyAscending.assign(nKeyColumns, true);
    if (!pSortOrder)
        return aSortOrder;

    SCSIZE nSortColumns, nSortRows;
    pSortOrder->GetDimensions(nSortColumns, nSortRows);
    if (nSortColumns != 1 && nSortRows != 1)
        return std::nullopt;
    const SCSIZE nEntries = nSortColumns * nSortRows;
    std::vector<bool> aUsed(nKeyColumns + nValueColumns, false);
    for (SCSIZE nEntry = 0; nEntry < nEntries; ++nEntry)
    {
        const SCSIZE nEntryColumn = nSortColumns == 1 ? 0 : nEntry;
        const SCSIZE nEntryRow = nSortColumns == 1 ? nEntry : 0;
        if (getCellClass(*pSortOrder, nEntryColumn, nEntryRow) != CellClass::Number)
            return std::nullopt;
        const double fEntry = pSortOrder->GetDouble(nEntryColumn, nEntryRow);
        const double fColumn = std::abs(fEntry);
        if (fColumn != std::trunc(fColumn) || fColumn < 1
            || fColumn > static_cast<double>(nKeyColumns + nValueColumns))
            return std::nullopt;
        const SCSIZE nColumn = static_cast<SCSIZE>(fColumn) - 1;
        if (aUsed[nColumn] || (nEntries > 1 && nColumn >= nKeyColumns))
            return std::nullopt;
        aUsed[nColumn] = true;
        aSortOrder.maColumns.emplace_back(nColumn, fEntry > 0);
        if (nColumn < nKeyColumns)
            aSortOrder.maKeyAscending[nColumn] = fEntry > 0;
        else
            aSortOrder.moValueSort.emplace(nColumn - nKeyColumns, fEntry > 0);
    }
    return aSortOrder;
}

/// Sorts the groups of every level among their siblings: by rCompareValues when it is set, then by
/// the key column of the level.
void sortHierarchy(std::vector<GroupNode>& rNodes, const ScMatrix& rKeys,
                   const std::vector<bool>& rKeyAscending,
                   const std::function<sal_Int32(size_t, size_t)>& rCompareValues)
{
    auto aLessInHierarchy = [&](size_t nLeft, size_t nRight)
    {
        if (rCompareValues)
        {
            const sal_Int32 nResult = rCompareValues(nLeft, nRight);
            if (nResult != 0)
                return nResult < 0;
        }
        const SCSIZE nColumn = rNodes[nLeft].mnLevel - 1;
        return compareCellsForSort(rKeys, nColumn, rNodes[nLeft].maRows[0], rKeys, nColumn,
                                   rNodes[nRight].maRows[0], rKeyAscending[nColumn])
               < 0;
    };
    for (GroupNode& rNode : rNodes)
        std::stable_sort(rNode.maChildren.begin(), rNode.maChildren.end(), aLessInHierarchy);
}

/// Lists the groups of a hierarchy in output order, with the subtotals and the grand total of
/// rTotals. Node 0 stands for the grand total.
std::vector<size_t> listHierarchy(const std::vector<GroupNode>& rNodes, const TotalLayout& rTotals)
{
    std::vector<size_t> aOutputNodes;
    if (rTotals.mbGrandTotal && rTotals.mbTotalsAtTop)
        aOutputNodes.push_back(0);
    std::function<void(size_t)> aAppendGroup = [&](size_t nNode)
    {
        const GroupNode& rNode = rNodes[nNode];
        if (rNode.maChildren.empty())
        {
            aOutputNodes.push_back(nNode);
            return;
        }
        const bool bSubtotal = rNode.mnLevel <= rTotals.mnSubtotalLevels;
        if (bSubtotal && rTotals.mbTotalsAtTop)
            aOutputNodes.push_back(nNode);
        for (size_t nChild : rNode.maChildren)
            aAppendGroup(nChild);
        if (bSubtotal && !rTotals.mbTotalsAtTop)
            aOutputNodes.push_back(nNode);
    };
    for (size_t nChild : rNodes[0].maChildren)
        aAppendGroup(nChild);
    if (rTotals.mbGrandTotal && !rTotals.mbTotalsAtTop)
        aOutputNodes.push_back(0);
    return aOutputNodes;
}

/// Whether the function receives the values of a group together with all values. PERCENTOF is the
/// built-in function that compares a part with the whole. A LAMBDA receives both when it has two
/// required parameters.
bool takesAllValues(const FormulaCallable& rCallable)
{
    const OpCode eOpCode = rCallable.GetOpCode();
    if (eOpCode == ocLambda)
        return static_cast<const ScFormulaFunction&>(rCallable).GetNumRequiredParams() >= 2;
    return eOpCode == ocPercentOf;
}

/// The generated header name of a key or value column, for example "Value 2".
OUString getGeneratedName(TranslateId aNameId, SCSIZE nColumn)
{
    return ScResId(aNameId).replaceFirst("%1", OUString::number(nColumn + 1));
}
}

namespace sc
{
struct GroupByFunction::Grouping
{
    FormulaCallableRef mpCallable;
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
    /// The result of the function for each group node, one column per value column. A group whose
    /// result is not needed holds an empty element.
    ScMatrixRef mxAggregates;
};

std::optional<sal_Int32> GroupByFunction::PopOptionalInt32()
{
    if (mrInterpreter.IsMissing())
    {
        mrInterpreter.Pop();
        return std::nullopt;
    }
    return mrInterpreter.GetInt32();
}

ScMatrixRef GroupByFunction::PopOptionalMatrix()
{
    if (mrInterpreter.IsMissing())
    {
        mrInterpreter.Pop();
        return nullptr;
    }
    return mrInterpreter.GetMatrix();
}

ScMatrixRef GroupByFunction::GetKeyMatrix()
{
    const StackVar eType = mrInterpreter.GetStackType();
    if (eType != svSingleRef && eType != svDoubleRef)
        return mrInterpreter.GetMatrix();
    const FormulaToken* pToken = mrInterpreter.pStack[mrInterpreter.sp - 1];
    const ScRange aRange
        = eType == svSingleRef
              ? ScRange(static_cast<const ScSingleRefToken*>(pToken)->GetSingleRef().toAbs(
                    mrInterpreter.mrDoc, mrInterpreter.aPos))
              : static_cast<const ScDoubleRefToken*>(pToken)->GetDoubleRef().toAbs(
                    mrInterpreter.mrDoc, mrInterpreter.aPos);
    ScMatrixRef xKeys = mrInterpreter.GetMatrix();
    if (!xKeys || mrInterpreter.nGlobalError != FormulaError::NONE
        || aRange.aStart.Tab() != aRange.aEnd.Tab())
        return xKeys;
    SCSIZE nColumns, nRows;
    xKeys->GetDimensions(nColumns, nRows);
    if (nColumns != static_cast<SCSIZE>(aRange.aEnd.Col() - aRange.aStart.Col() + 1)
        || nRows != static_cast<SCSIZE>(aRange.aEnd.Row() - aRange.aStart.Row() + 1))
        return xKeys;

    // A logical value is a number with a boolean format here, so the cell format or the result type
    // of its formula tells it apart. The matrix is copied before the first change, as it may be
    // shared.
    bool bCopied = false;
    ScCellIterator aIterator(mrInterpreter.mrDoc, aRange);
    for (bool bHasCell = aIterator.first(); bHasCell; bHasCell = aIterator.next())
    {
        const ScAddress& rPosition = aIterator.GetPos();
        ScRefCellValue aCell = aIterator.getRefCellValue();
        bool bLogical;
        if (aCell.getType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFormulaCell = aCell.getFormula();
            if (pFormulaCell->GetErrCode() != FormulaError::NONE || !pFormulaCell->IsValue())
                continue;
            bLogical = pFormulaCell->GetFormatType() == SvNumFormatType::LOGICAL;
        }
        else if (aCell.hasNumeric())
            bLogical = false;
        else
            continue;
        if (!bLogical)
            bLogical = mrInterpreter.mrContext.NFGetType(
                           mrInterpreter.mrDoc.GetNumberFormat(mrInterpreter.mrContext, rPosition))
                       == SvNumFormatType::LOGICAL;
        if (!bLogical)
            continue;
        if (!bCopied)
        {
            xKeys = xKeys->Clone();
            bCopied = true;
        }
        const SCSIZE nColumn = rPosition.Col() - aRange.aStart.Col();
        const SCSIZE nRow = rPosition.Row() - aRange.aStart.Row();
        xKeys->PutBoolean(xKeys->GetDouble(nColumn, nRow) != 0.0, nColumn, nRow);
    }
    return xKeys;
}

void GroupByFunction::FillKeyCell(const ScMatrixRef& xKeys, const ScMatrixRef& xOutput,
                                  SCSIZE nKeyColumn, SCSIZE nKeyRow, SCSIZE nColumn, SCSIZE nRow)
{
    if (xKeys->IsBoolean(nKeyColumn, nKeyRow))
        xOutput->PutBoolean(xKeys->GetDouble(nKeyColumn, nKeyRow) != 0.0, nColumn, nRow);
    else
        ScInterpreter::FillMatrixCell(xKeys, xOutput, nKeyColumn, nKeyRow, nColumn, nRow);
}

ScMatrixRef GroupByFunction::GetGroupValues(const ScMatrixRef& xValues, SCSIZE nColumn,
                                            const std::vector<SCSIZE>& rRows)
{
    ScMatrixRef xColumn = mrInterpreter.GetNewMat(1, rRows.size(), /*bEmpty*/ true);
    if (xColumn)
    {
        // An empty cell reaches the function as empty text, so COUNTA counts it and COUNT does not.
        // An error value reaches the function as one element of the column, so COUNTA counts it
        // once like any other element.
        for (SCSIZE nIndex = 0; nIndex < rRows.size(); ++nIndex)
        {
            const ScMatrixValue aValue = xValues->Get(nColumn, rRows[nIndex]);
            if (ScMatrix::IsValueType(aValue.nType))
                xColumn->PutDouble(aValue.fVal, 0, nIndex);
            else if (ScMatrix::IsRealStringType(aValue.nType))
                xColumn->PutString(aValue.GetString(), 0, nIndex);
            else
                xColumn->PutString(svl::SharedString::getEmptyString(), 0, nIndex);
        }
    }
    return xColumn;
}

bool GroupByFunction::PutGroupAggregate(const FormulaCallableRef& pCallable,
                                        const ScMatrixRef& xGroupValues,
                                        const ScMatrixRef& xAllValues, const ScMatrixRef& xTarget,
                                        SCSIZE nColumn, SCSIZE nRow)
{
    std::vector<FormulaConstTokenRef> aArguments;
    aArguments.push_back(new ScMatrixToken(xGroupValues));
    if (xAllValues)
        aArguments.push_back(new ScMatrixToken(xAllValues));
    mrInterpreter.ScCall(pCallable, aArguments);
    const FormulaConstTokenRef pResult = mrInterpreter.PopToken();
    bool bArray = false;
    switch (pResult->GetType())
    {
        case svMatrix:
        case svMatrixCell:
        {
            const ScMatrix* pMatrix
                = pResult->GetType() == svMatrix
                      ? static_cast<const ScMatrixToken*>(pResult.get())->GetMatrix()
                      : static_cast<const ScMatrixCellResultToken*>(pResult.get())->GetMatrix();
            SCSIZE nColumns = 0;
            SCSIZE nRows = 0;
            if (pMatrix)
                pMatrix->GetDimensions(nColumns, nRows);
            bArray = nColumns * nRows > 1;
            break;
        }
        case svDoubleRef:
        {
            const ScRange aRange = static_cast<const ScDoubleRefToken*>(pResult.get())
                                       ->GetDoubleRef()
                                       .toAbs(mrInterpreter.mrDoc, mrInterpreter.aPos);
            bArray = aRange.aStart != aRange.aEnd;
            break;
        }
        default:
            break;
    }
    if (!bArray)
        mrInterpreter.PutMatrixValue(pResult, xTarget, nColumn, nRow);
    // The result of this group is stored, so clear any error it produced and let the next group
    // compute from a clean state.
    mrInterpreter.nGlobalError = FormulaError::NONE;
    return !bArray;
}

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
    rGrouping.mpCallable = mrInterpreter.GetCallable();
    rGrouping.mxValues = mrInterpreter.GetMatrix();
    rGrouping.mxKeys = GetKeyMatrix();

    if (mrInterpreter.nGlobalError != FormulaError::NONE)
    {
        mrInterpreter.PushError(mrInterpreter.nGlobalError);
        return false;
    }
    if (!rGrouping.mpCallable || !rGrouping.mxKeys || !rGrouping.mxValues)
    {
        mrInterpreter.PushIllegalArgument();
        return false;
    }

    SCSIZE nKeyRows, nValueRows;
    rGrouping.mxKeys->GetDimensions(rGrouping.mnKeyColumns, nKeyRows);
    rGrouping.mxValues->GetDimensions(rGrouping.mnValueColumns, nValueRows);
    if (nKeyRows != nValueRows || nKeyRows == 0 || rGrouping.mnKeyColumns == 0
        || rGrouping.mnValueColumns == 0
        || (oFieldHeaders && (*oFieldHeaders < 0 || *oFieldHeaders > 3))
        || (nRelationship != 0 && nRelationship != 1))
    {
        mrInterpreter.PushNoValue();
        return false;
    }
    rGrouping.mbTable = nRelationship == 1;

    // Field headers: 0 means no header row, 1 a header row that is not shown, 2 no header row with
    // generated names shown, and 3 a header row that is shown. When the argument is left out, the
    // header row is detected and not shown.
    rGrouping.mbHasHeaderRow = oFieldHeaders ? *oFieldHeaders == 1 || *oFieldHeaders == 3
                                             : startsWithHeaderRow(*rGrouping.mxValues);
    rGrouping.mbShowHeaders = oFieldHeaders && *oFieldHeaders >= 2;

    const std::optional<SortOrder> oSortOrder
        = getSortOrder(xSortOrder.get(), rGrouping.mnKeyColumns, rGrouping.mnValueColumns);
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
        = mrInterpreter.GetNewMat(rGrouping.mnValueColumns, rNodes.size(), /*bEmpty*/ true);
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
    const bool bTakesAllValues = takesAllValues(*rGrouping.mpCallable);
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
            // A function gives one value per group. An array result has no cell to go to, and the
            // whole result is the #CALC! error.
            if (!PutGroupAggregate(rGrouping.mpCallable, xGroupValues,
                                   bTakesAllValues ? xAllValues : nullptr, rGrouping.mxAggregates,
                                   nColumn, nNode))
            {
                mrInterpreter.PushError(FormulaError::Calculation);
                return false;
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
            const SCSIZE nValueColumn = rSortOrder.moValueSort->first;
            const bool bAscending = rSortOrder.moValueSort->second;
            aCompareValues
                = [&aCompareAggregates, nValueColumn, bAscending](size_t nLeft, size_t nRight)
            { return aCompareAggregates(nLeft, nRight, nValueColumn, bAscending); };
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

void GroupByFunction::PutHeaderRow(const Grouping& rGrouping, const ScMatrixRef& xOutput)
{
    svl::SharedStringPool& rStringPool = mrInterpreter.mrStrPool;
    for (SCSIZE nColumn = 0; nColumn < rGrouping.mnKeyColumns; ++nColumn)
    {
        if (rGrouping.mbHasHeaderRow)
            FillKeyCell(rGrouping.mxKeys, xOutput, nColumn, 0, nColumn, 0);
        else
            xOutput->PutString(rStringPool.intern(getGeneratedName(STR_GROUPBY_ROW_FIELD, nColumn)),
                               nColumn, 0);
    }
    for (SCSIZE nColumn = 0; nColumn < rGrouping.mnValueColumns; ++nColumn)
    {
        const SCSIZE nOutputColumn = rGrouping.mnKeyColumns + nColumn;
        if (rGrouping.mbHasHeaderRow)
            ScInterpreter::FillMatrixCell(rGrouping.mxValues, xOutput, nColumn, 0, nOutputColumn,
                                          0);
        else
            xOutput->PutString(rStringPool.intern(getGeneratedName(STR_GROUPBY_VALUE, nColumn)),
                               nOutputColumn, 0);
    }
}

void GroupByFunction::PutGroupRows(const Grouping& rGrouping,
                                   const std::vector<size_t>& rOutputNodes,
                                   const ScMatrixRef& xOutput, SCSIZE nFirstRow)
{
    // A subtotal row shows the keys that its group fixes and empty text in the deeper key columns.
    // The grand total row shows its label in the first key column.
    const svl::SharedString aTotalLabel = mrInterpreter.mrStrPool.intern(ScResId(
        rGrouping.maTotals.mnSubtotalLevels > 0 ? STR_GROUPBY_GRAND_TOTAL : STR_GROUPBY_TOTAL));
    const svl::SharedString& rEmptyText = svl::SharedString::getEmptyString();
    const SCSIZE nKeyColumns = rGrouping.mnKeyColumns;
    for (SCSIZE nIndex = 0; nIndex < rOutputNodes.size(); ++nIndex)
    {
        const size_t nNode = rOutputNodes[nIndex];
        const SCSIZE nOutputRow = nFirstRow + nIndex;
        const std::vector<SCSIZE> aKeyRows = getKeyRows(rGrouping.maNodes, nNode);
        for (SCSIZE nColumn = 0; nColumn < aKeyRows.size(); ++nColumn)
            FillKeyCell(rGrouping.mxKeys, xOutput, nColumn, aKeyRows[nColumn], nColumn, nOutputRow);
        for (SCSIZE nColumn = aKeyRows.size(); nColumn < nKeyColumns; ++nColumn)
            xOutput->PutString(rEmptyText, nColumn, nOutputRow);
        if (nNode == 0)
            xOutput->PutString(aTotalLabel, 0, nOutputRow);
        for (SCSIZE nColumn = 0; nColumn < rGrouping.mnValueColumns; ++nColumn)
            ScInterpreter::FillMatrixCell(rGrouping.mxAggregates, xOutput, nColumn, nNode,
                                          nKeyColumns + nColumn, nOutputRow);
    }
}

void GroupByFunction::Execute()
{
    Grouping aGrouping;
    if (!ReadArguments(aGrouping))
        return;
    aGrouping.maNodes = buildGroups(*aGrouping.mxKeys, aGrouping.maRows);
    if (!ComputeAggregates(aGrouping))
        return;
    const std::vector<size_t> aOutputNodes = ListOutputNodes(aGrouping);

    const SCSIZE nHeaderRows = aGrouping.mbShowHeaders ? 1 : 0;
    ScMatrixRef xOutput
        = mrInterpreter.GetNewMat(aGrouping.mnKeyColumns + aGrouping.mnValueColumns,
                                  nHeaderRows + aOutputNodes.size(), /*bEmpty*/ true);
    if (!xOutput)
    {
        mrInterpreter.PushError(FormulaError::MatrixSize);
        return;
    }
    if (aGrouping.mbShowHeaders)
        PutHeaderRow(aGrouping, xOutput);
    PutGroupRows(aGrouping, aOutputNodes, xOutput, nHeaderRows);

    // The result takes no number format from the input or from the function.
    mrInterpreter.nFuncFmtType = SvNumFormatType::NUMBER;
    mrInterpreter.PushMatrix(xOutput);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
