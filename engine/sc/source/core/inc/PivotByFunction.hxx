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

#include "GroupingFunction.hxx"

#include <vector>

namespace sc
{
/// PIVOTBY groups the rows of a range by row keys and by column keys at the same time, and calls a
/// function on the values that each row group and column group share.
class PivotByFunction : public GroupingFunction
{
public:
    explicit PivotByFunction(ScInterpreter& rInterpreter)
        : GroupingFunction(rInterpreter)
    {
    }

    /// Pops the arguments from the interpreter stack and pushes the result table or an error.
    void Execute();

private:
    /// The arguments of one call, the row groups and column groups that come from them, and the
    /// results of their cells.
    struct Pivot;

    /// Pops the arguments and selects the input rows. Pushes an error and returns false for invalid
    /// arguments.
    bool ReadArguments(Pivot& rPivot);
    /// Calls the function on the rows that a row group and a column group share, unless that cell
    /// is already computed. Returns the column of the cell in the cell matrix.
    SCSIZE ComputeCell(Pivot& rPivot, size_t nRowNode, size_t nColumnNode, SCSIZE nResultColumn);
    /// Sorts the row groups and the column groups among their siblings.
    void SortGroups(Pivot& rPivot);
    /// Puts the header rows into xOutput: the keys of the column groups, the labels of functions in
    /// a row, the names of the value columns and the names of the row keys.
    void PutHeaderRows(const Pivot& rPivot, const std::vector<size_t>& rColumnOutput,
                       const ScMatrixRef& xOutput);
    /// Puts the rows of each listed row group into xOutput, after the header rows: the keys of the
    /// group and its cells.
    void PutGroupRows(Pivot& rPivot, const std::vector<size_t>& rRowOutput,
                      const std::vector<size_t>& rColumnOutput, const ScMatrixRef& xOutput);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
