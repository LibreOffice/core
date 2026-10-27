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

#pragma once

#include <address.hxx>
#include <types.hxx>

#include <formula/types.hxx>
#include <rtl/ustring.hxx>
#include <unotools/resmgr.hxx>

#include <functional>
#include <optional>
#include <utility>
#include <vector>

class ScInterpreter;
class ScMatrix;
namespace svl
{
class SharedStringPool;
}

namespace sc::grouping
{
/// Compares two matrix elements for a sort in the given direction. Empty cells come last in both
/// directions.
sal_Int32 compareCellsForSort(const ScMatrix& rLeft, SCSIZE nLeftColumn, SCSIZE nLeftRow,
                              const ScMatrix& rRight, SCSIZE nRightColumn, SCSIZE nRightRow,
                              bool bAscending);

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

/// Builds the tree of groups over the given rows of pKeys. Node 0 is the group of all rows, and
/// without keys it is the only node. The children of a node are in the order in which their key
/// first appears.
std::vector<GroupNode> buildGroups(const ScMatrix* pKeys, const std::vector<SCSIZE>& rRows);

/// The rows that spell the keys of a group, one for each key column that the group fixes. Each key
/// comes from the group at that level, so all rows of a group spell its key the same way.
std::vector<SCSIZE> getKeyRows(const std::vector<GroupNode>& rNodes, size_t nNode);

/// Whether the values start with a header row: text in the first row and a number in the second.
bool startsWithHeaderRow(const ScMatrix& rValues);

/// Collects the rows after the header row that the filter keeps. The filter has one entry per input
/// row, including the header row, and its entry for the header row is ignored. An empty entry
/// leaves the row out like FALSE does. Returns false when the filter has another length or holds
/// text or an error.
bool selectRows(const ScMatrix* pFilter, SCSIZE nRows, bool bHasHeaderRow,
                std::vector<SCSIZE>& rSelectedRows);

/// The totals of one direction of the result.
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
                                          bool bSubtotalsAllowed);

/// A sort order of one direction of the result.
struct SortOrder
{
    /// The direction of each key column.
    std::vector<bool> maKeyAscending;
    /// The sort entries in their order, as 0-based columns: the key columns, then the result
    /// columns. Each is paired with whether it sorts ascending.
    std::vector<std::pair<SCSIZE, bool>> maColumns;
    /// The result column to sort by and its direction, when the only entry names a result column.
    std::optional<std::pair<SCSIZE, bool>> moValueSort;
};

/// Reads a sort order. Each entry is a 1-based column: the key columns, then the result columns. A
/// negative entry sorts in descending order. Several entries may only name key columns, and each
/// column at most once. Returns nothing for an invalid sort order.
std::optional<SortOrder> getSortOrder(const ScMatrix* pSortOrder, SCSIZE nKeyColumns,
                                      SCSIZE nResultColumns);

/// Sorts the groups of every level among their siblings: by rCompareValues when it is set, then by
/// the key column of the level.
void sortHierarchy(std::vector<GroupNode>& rNodes, const ScMatrix& rKeys,
                   const std::vector<bool>& rKeyAscending,
                   const std::function<sal_Int32(size_t, size_t)>& rCompareValues);

/// Lists the groups of a hierarchy in output order, with the subtotals and the grand total of
/// rTotals. Node 0 stands for the grand total. Without key columns the group of all rows is the
/// only entry.
std::vector<size_t> listHierarchy(const std::vector<GroupNode>& rNodes, const TotalLayout& rTotals);

/// Whether the function receives the values of a group together with the values of the total it is
/// a part of. PERCENTOF is the built-in function that compares a part with the whole. A LAMBDA
/// receives both when it has two required parameters.
bool takesAllValues(const formula::FormulaCallable& rCallable);

/// The generated header name of a key or value column, for example "Value 2".
OUString getGeneratedName(TranslateId aNameId, SCSIZE nColumn);

/// The functions of the function argument and their labels. A single function gives one result per
/// value column. Functions in a row give a column of results each, and functions in a column a row
/// of results each.
struct FunctionList
{
    std::vector<formula::FormulaCallableRef> maFunctions;
    /// The position in mxLabelSource of the label of each function. Without labels a function is
    /// labeled with its name.
    std::vector<std::pair<SCSIZE, SCSIZE>> maLabelPositions;
    ScMatrixRef mxLabelSource;
    bool mbInRows = false;
    /// The labels are shown when there are several functions or the labels are given.
    bool mbShowLabels = false;
};

/// A function, or a stack of functions in one row or one column. A second row or column of the
/// stack, beside the functions, holds a label for each function.
std::optional<FunctionList> getFunctionList(const formula::FormulaCallableRef& pCallable);

/// The function and the value column of each result. The functions pair up with the value columns,
/// whether they lie in a row or in a column: a single function applies to every value column, and
/// every function applies to a single value column. Otherwise there are as many functions as value
/// columns, and each function applies to the value column at its position.
std::optional<std::vector<std::pair<size_t, SCSIZE>>> getResultColumns(const FunctionList& rList,
                                                                       SCSIZE nValueColumns);

/// Puts the label of a function into xOutput: the given label, or else the name of the function. A
/// LAMBDA has no name of its own and is labeled CUSTOM.
void putFunctionLabel(const FunctionList& rList, size_t nFunction,
                      svl::SharedStringPool& rStringPool, const ScMatrixRef& xOutput,
                      SCSIZE nColumn, SCSIZE nRow);
}

namespace sc
{
/// The interpreter steps that the functions which group rows by key columns share: reading the
/// optional arguments and the keys, and calling the function on the values of a group.
class GroupingFunction
{
protected:
    explicit GroupingFunction(ScInterpreter& rInterpreter)
        : mrInterpreter(rInterpreter)
    {
    }

    /// Pops an optional integer argument. A left out argument gives nothing.
    std::optional<sal_Int32> PopOptionalInt32();
    /// Pops an optional array argument. A left out argument gives no matrix.
    ScMatrixRef PopOptionalMatrix();
    /// Pops the keys of a grouping as a matrix. A cell that holds a logical value gives a boolean
    /// element, so TRUE and 1 are different keys.
    ScMatrixRef GetKeyMatrix();
    /// Copies one key from xKeys to xOutput: a number, an error, text, an empty cell, or a boolean.
    static void FillKeyCell(const ScMatrixRef& xKeys, const ScMatrixRef& xOutput, SCSIZE nKeyColumn,
                            SCSIZE nKeyRow, SCSIZE nColumn, SCSIZE nRow);
    /// The values of one column of xValues at the given rows, as a matrix with one column.
    ScMatrixRef GetGroupValues(const ScMatrixRef& xValues, SCSIZE nColumn,
                               const std::vector<SCSIZE>& rRows);
    /// Calls pCallable with the values of a group, and with the values of its total when
    /// xAllValues is set, and stores the result in xTarget at nColumn, nRow. Returns false, and
    /// stores nothing, when the result is an array or a range of more than one cell.
    bool PutGroupAggregate(const formula::FormulaCallableRef& pCallable,
                           const ScMatrixRef& xGroupValues, const ScMatrixRef& xAllValues,
                           const ScMatrixRef& xTarget, SCSIZE nColumn, SCSIZE nRow);

    ScInterpreter& mrInterpreter;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
