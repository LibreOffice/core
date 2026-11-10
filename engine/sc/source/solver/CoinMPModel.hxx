/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <com/sun/star/sheet/SolverConstraint.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <cpo/uno/Sequence.hxx>

#include "SolverComponent.hxx"

#include <vector>

enum class CoinMpSolveStatus
{
    Optimal,
    Infeasible,
    Unbounded,
    TimeLimit,
    Other
};

// Result of solving one linear problem with Cbc.
struct CoinMpSolveResult
{
    // aSolution and fObjective hold either the optimum or, at the time limit, the best solution
    // found so far.
    bool bSuccess = false;
    CoinMpSolveStatus eStatus = CoinMpSolveStatus::Other;
    std::vector<double> aSolution;
    double fObjective = 0.0;
};

// Build a linear problem from already sampled coefficients and solve it with
// Cbc. For the objective and for each constraint cell, rCells holds a vector
// whose first entry is the value of an affine model of that cell at the origin
// (all variables zero) and whose remaining entries are the slope of each
// variable. The caller supplies the variable bounds and column types (which
// encode integer and binary variables), so the same routine serves both a
// one-shot linear solve and a linearized step of an outer nonlinear loop.
CoinMpSolveResult coinmpSolveLinearModel(
    const cpo::uno::Sequence<css::table::CellAddress>& rVariables,
    const cpo::uno::Sequence<css::sheet::SolverConstraint>& rConstraints,
    const css::table::CellAddress& rObjective, const ScSolverCellHashMap& rCells,
    const std::vector<double>& rLowerBounds, const std::vector<double>& rUpperBounds,
    const std::vector<char>& rColumnType, bool bMaximize, double fTimeoutSeconds);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
