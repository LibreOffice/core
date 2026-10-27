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

#include <GroupingFunction.hxx>

#include <interpre.hxx>
#include <callable.hxx>
#include <cellvalue.hxx>
#include <compiler.hxx>
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
#include <map>

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
}

namespace sc::grouping
{
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

std::vector<GroupNode> buildGroups(const ScMatrix* pKeys, const std::vector<SCSIZE>& rRows)
{
    std::vector<GroupNode> aNodes{ { 0, 0, rRows, {} } };
    if (!pKeys)
        return aNodes;

    SCSIZE nKeyColumns, nKeyRows;
    pKeys->GetDimensions(nKeyColumns, nKeyRows);

    struct KeyLess
    {
        const ScMatrix* mpKeys;
        SCSIZE mnColumn;
        bool operator()(SCSIZE nLeftRow, SCSIZE nRightRow) const
        {
            return compareCells(*mpKeys, mnColumn, nLeftRow, *mpKeys, mnColumn, nRightRow) < 0;
        }
    };
    using ChildLookup = std::map<SCSIZE, size_t, KeyLess>;
    // For each node, its children are found by a row that has the same key cell. A leaf has no
    // children and no lookup.
    std::vector<std::optional<ChildLookup>> aLookups;
    aLookups.emplace_back(std::in_place, KeyLess{ pKeys, 0 });
    for (SCSIZE nRow : rRows)
    {
        size_t nNode = 0;
        for (SCSIZE nColumn = 0; nColumn < nKeyColumns; ++nColumn)
        {
            ChildLookup& rLookup = *aLookups[nNode];
            size_t nChild;
            auto aFound = rLookup.find(nRow);
            if (aFound != rLookup.end())
                nChild = aFound->second;
            else
            {
                nChild = aNodes.size();
                aNodes.push_back({ nColumn + 1, nNode, {}, {} });
                aNodes[nNode].maChildren.push_back(nChild);
                rLookup.emplace(nRow, nChild);
                if (nColumn + 1 < nKeyColumns)
                    aLookups.emplace_back(std::in_place, KeyLess{ pKeys, nColumn + 1 });
                else
                    aLookups.emplace_back();
            }
            aNodes[nChild].maRows.push_back(nRow);
            nNode = nChild;
        }
    }
    return aNodes;
}

std::vector<SCSIZE> getKeyRows(const std::vector<GroupNode>& rNodes, size_t nNode)
{
    std::vector<SCSIZE> aKeyRows(rNodes[nNode].mnLevel);
    for (size_t nAncestor = nNode; rNodes[nAncestor].mnLevel > 0;
         nAncestor = rNodes[nAncestor].mnParent)
        aKeyRows[rNodes[nAncestor].mnLevel - 1] = rNodes[nAncestor].maRows[0];
    return aKeyRows;
}

bool startsWithHeaderRow(const ScMatrix& rValues)
{
    SCSIZE nColumns, nRows;
    rValues.GetDimensions(nColumns, nRows);
    return nRows >= 2 && getCellClass(rValues, 0, 0) == CellClass::Text
           && getCellClass(rValues, 0, 1) == CellClass::Number;
}

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

std::optional<TotalLayout> getTotalLayout(std::optional<sal_Int32> oTotalDepth, SCSIZE nKeyColumns,
                                          bool bSubtotalsAllowed)
{
    const sal_Int64 nTotalDepth = oTotalDepth.value_or(1);
    const sal_Int64 nDepth = std::abs(nTotalDepth);
    // Without key columns there is only the group of all rows, and depth 1 still allows it.
    const sal_Int64 nMaximumDepth
        = bSubtotalsAllowed ? std::max<sal_Int64>(static_cast<sal_Int64>(nKeyColumns), 1) : 1;
    if (nDepth > nMaximumDepth)
        return std::nullopt;
    return TotalLayout{ nDepth > 0, nTotalDepth < 0,
                        nDepth > 1 ? static_cast<SCSIZE>(nDepth - 1) : 0 };
}

std::optional<SortOrder> getSortOrder(const ScMatrix* pSortOrder, SCSIZE nKeyColumns,
                                      SCSIZE nResultColumns)
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
    std::vector<bool> aUsed(nKeyColumns + nResultColumns, false);
    for (SCSIZE nEntry = 0; nEntry < nEntries; ++nEntry)
    {
        const SCSIZE nEntryColumn = nSortColumns == 1 ? 0 : nEntry;
        const SCSIZE nEntryRow = nSortColumns == 1 ? nEntry : 0;
        if (getCellClass(*pSortOrder, nEntryColumn, nEntryRow) != CellClass::Number)
            return std::nullopt;
        const double fEntry = pSortOrder->GetDouble(nEntryColumn, nEntryRow);
        const double fColumn = std::abs(fEntry);
        if (fColumn != std::trunc(fColumn) || fColumn < 1
            || fColumn > static_cast<double>(nKeyColumns + nResultColumns))
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

std::vector<size_t> listHierarchy(const std::vector<GroupNode>& rNodes, const TotalLayout& rTotals)
{
    if (rNodes.size() == 1)
        return { 0 };

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

bool takesAllValues(const FormulaCallable& rCallable)
{
    const OpCode eOpCode = rCallable.GetOpCode();
    if (eOpCode == ocLambda)
        return static_cast<const ScFormulaFunction&>(rCallable).GetNumRequiredParams() >= 2;
    return eOpCode == ocPercentOf;
}

OUString getGeneratedName(TranslateId aNameId, SCSIZE nColumn)
{
    return ScResId(aNameId).replaceFirst("%1", OUString::number(nColumn + 1));
}

std::optional<FunctionList> getFunctionList(const formula::FormulaCallableRef& pCallable)
{
    FunctionList aList;
    auto pArray = dynamic_cast<const ScCallableArray*>(pCallable.get());
    if (!pArray)
    {
        aList.maFunctions.push_back(pCallable);
        return aList;
    }
    if (pArray->GetColumns() * pArray->GetRows() == 1)
    {
        aList.maFunctions.push_back(pArray->GetCallable(0, 0));
        return aList;
    }

    // A line is a row of the stack when the functions lie in a row, and a column otherwise.
    auto aReadLines = [&](bool bInRows) -> bool
    {
        const SCSIZE nFunctions = bInRows ? pArray->GetRows() : pArray->GetColumns();
        const SCSIZE nLines = bInRows ? pArray->GetColumns() : pArray->GetRows();
        auto aGetCallable = [&](SCSIZE nLine, SCSIZE nIndex)
        {
            return bInRows ? pArray->GetCallable(nLine, nIndex)
                           : pArray->GetCallable(nIndex, nLine);
        };
        auto aIsLine = [&](SCSIZE nLine, bool bFunctions)
        {
            for (SCSIZE nIndex = 0; nIndex < nFunctions; ++nIndex)
            {
                if (bool(aGetCallable(nLine, nIndex)) != bFunctions)
                    return false;
            }
            return true;
        };
        if (nLines > 2)
            return false;
        SCSIZE nFunctionLine = 0;
        if (!aIsLine(0, true))
        {
            if (nLines == 1 || !aIsLine(1, true))
                return false;
            nFunctionLine = 1;
        }
        const SCSIZE nLabelLine = 1 - nFunctionLine;
        if (nLines == 2 && !aIsLine(nLabelLine, false))
            return false;

        aList.mbInRows = bInRows;
        aList.mbShowLabels = nFunctions > 1 || nLines == 2;
        for (SCSIZE nIndex = 0; nIndex < nFunctions; ++nIndex)
        {
            aList.maFunctions.push_back(aGetCallable(nFunctionLine, nIndex));
            if (nLines == 2)
                aList.maLabelPositions.push_back(bInRows ? std::make_pair(nLabelLine, nIndex)
                                                         : std::make_pair(nIndex, nLabelLine));
        }
        if (nLines == 2)
            aList.mxLabelSource = pArray->GetValues();
        return true;
    };
    if (aReadLines(false) || aReadLines(true))
        return aList;
    return std::nullopt;
}

std::optional<std::vector<std::pair<size_t, SCSIZE>>> getResultColumns(const FunctionList& rList,
                                                                       SCSIZE nValueColumns)
{
    const size_t nFunctions = rList.maFunctions.size();
    std::vector<std::pair<size_t, SCSIZE>> aResultColumns;
    if (nFunctions > 1 && nValueColumns > 1 && nFunctions != nValueColumns)
        return std::nullopt;
    const SCSIZE nResultColumns = std::max<SCSIZE>(nFunctions, nValueColumns);
    for (SCSIZE nResultColumn = 0; nResultColumn < nResultColumns; ++nResultColumn)
        aResultColumns.emplace_back(nFunctions == 1 ? 0 : nResultColumn,
                                    nValueColumns == 1 ? 0 : nResultColumn);
    return aResultColumns;
}

void putFunctionLabel(const FunctionList& rList, size_t nFunction,
                      svl::SharedStringPool& rStringPool, const ScMatrixRef& xOutput,
                      SCSIZE nColumn, SCSIZE nRow)
{
    if (rList.mxLabelSource)
    {
        const auto& [nLabelColumn, nLabelRow] = rList.maLabelPositions[nFunction];
        ScInterpreter::FillMatrixCell(rList.mxLabelSource, xOutput, nLabelColumn, nLabelRow,
                                      nColumn, nRow);
        return;
    }
    const OpCode eOpCode = rList.maFunctions[nFunction]->GetOpCode();
    const OUString aName
        = eOpCode == ocLambda ? ScResId(STR_GROUPBY_CUSTOM) : ScCompiler::GetNativeSymbol(eOpCode);
    xOutput->PutString(rStringPool.intern(aName), nColumn, nRow);
}
}

namespace sc
{
std::optional<sal_Int32> GroupingFunction::PopOptionalInt32()
{
    if (mrInterpreter.IsMissing())
    {
        mrInterpreter.Pop();
        return std::nullopt;
    }
    return mrInterpreter.GetInt32();
}

ScMatrixRef GroupingFunction::PopOptionalMatrix()
{
    if (mrInterpreter.IsMissing())
    {
        mrInterpreter.Pop();
        return nullptr;
    }
    return mrInterpreter.GetMatrix();
}

ScMatrixRef GroupingFunction::GetKeyMatrix()
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

void GroupingFunction::FillKeyCell(const ScMatrixRef& xKeys, const ScMatrixRef& xOutput,
                                   SCSIZE nKeyColumn, SCSIZE nKeyRow, SCSIZE nColumn, SCSIZE nRow)
{
    if (xKeys->IsBoolean(nKeyColumn, nKeyRow))
        xOutput->PutBoolean(xKeys->GetDouble(nKeyColumn, nKeyRow) != 0.0, nColumn, nRow);
    else
        ScInterpreter::FillMatrixCell(xKeys, xOutput, nKeyColumn, nKeyRow, nColumn, nRow);
}

ScMatrixRef GroupingFunction::GetGroupValues(const ScMatrixRef& xValues, SCSIZE nColumn,
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

bool GroupingFunction::PutGroupAggregate(const FormulaCallableRef& pCallable,
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
