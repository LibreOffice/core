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
/// GROUPBY groups the rows of a range by one or more key columns and calls a function on the values
/// of each group.
class GroupByFunction : public GroupingFunction
{
public:
    explicit GroupByFunction(ScInterpreter& rInterpreter)
        : GroupingFunction(rInterpreter)
    {
    }

    /// Pops the arguments from the interpreter stack and pushes the result table or an error.
    void Execute();

private:
    /// The arguments of one call, and the groups and the results that come from them.
    struct Grouping;

    /// Pops the arguments and selects the input rows. Pushes an error and returns false for invalid
    /// arguments.
    bool ReadArguments(Grouping& rGrouping);
    /// Calls the function for each group that the result shows or that a value sort needs. Pushes
    /// an error and returns false when that fails.
    bool ComputeAggregates(Grouping& rGrouping);
    /// Sorts the groups and lists them in output order, with their totals. Node 0 stands for the
    /// grand total.
    static std::vector<size_t> ListOutputNodes(Grouping& rGrouping);
    /// Puts the name of a value column into xOutput: its header, or a generated name.
    void PutValueName(const Grouping& rGrouping, SCSIZE nValueColumn, const ScMatrixRef& xOutput,
                      SCSIZE nColumn, SCSIZE nRow);
    /// Puts the header rows into xOutput: the labels of functions in a row, and the names of the
    /// key and value columns when they are shown.
    void PutHeaderRows(const Grouping& rGrouping, const ScMatrixRef& xOutput);
    /// Puts the rows of each listed group into xOutput, after the header rows: the keys of the
    /// group and its results.
    void PutGroupRows(const Grouping& rGrouping, const std::vector<size_t>& rOutputNodes,
                      const ScMatrixRef& xOutput);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
