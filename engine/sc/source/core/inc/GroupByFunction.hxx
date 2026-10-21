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

#include <optional>
#include <vector>

class ScInterpreter;

namespace sc
{
/// GROUPBY groups the rows of a range by one or more key columns and calls a function on the values
/// of each group.
class GroupByFunction
{
public:
    explicit GroupByFunction(ScInterpreter& rInterpreter)
        : mrInterpreter(rInterpreter)
    {
    }

    /// Pops the arguments from the interpreter stack and pushes the result table or an error.
    void Execute();

private:
    /// The arguments of one call, and the groups and the results that come from them.
    struct Grouping;

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
    /// Calls pCallable with the values of a group, and with all values when xAllValues is set, and
    /// stores the result in xTarget at nColumn, nRow. Returns false, and stores nothing, when the
    /// result is an array or a range of more than one cell.
    bool PutGroupAggregate(const formula::FormulaCallableRef& pCallable,
                           const ScMatrixRef& xGroupValues, const ScMatrixRef& xAllValues,
                           const ScMatrixRef& xTarget, SCSIZE nColumn, SCSIZE nRow);

    /// Pops the arguments and selects the input rows. Pushes an error and returns false for invalid
    /// arguments.
    bool ReadArguments(Grouping& rGrouping);
    /// Calls the function for each group that the result shows or that a value sort needs. Pushes
    /// an error and returns false when that fails.
    bool ComputeAggregates(Grouping& rGrouping);
    /// Sorts the groups and lists them in output order, with their totals. Node 0 stands for the
    /// grand total.
    static std::vector<size_t> ListOutputNodes(Grouping& rGrouping);
    /// Puts the names of the key and value columns into the first row of xOutput.
    void PutHeaderRow(const Grouping& rGrouping, const ScMatrixRef& xOutput);
    /// Puts one row per listed group into xOutput, from nFirstRow on: its keys and its results.
    void PutGroupRows(const Grouping& rGrouping, const std::vector<size_t>& rOutputNodes,
                      const ScMatrixRef& xOutput, SCSIZE nFirstRow);

    ScInterpreter& mrInterpreter;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
