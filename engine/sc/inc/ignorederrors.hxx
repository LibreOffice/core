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

#include "rangelst.hxx"

/** Cells of a sheet in which a spreadsheet application does not mark possible errors, and the
    kinds of possible error that it does not mark there. Calc does not mark these errors itself,
    so the data only travels between OOXML import and export. */
struct ScIgnoredErrors
{
    ScRangeList maRanges;
    /// A formula that results in an error value.
    bool mbEvalError = false;
    /// A date written as text with a two digit year.
    bool mbTwoDigitTextYear = false;
    /// A number stored as text.
    bool mbNumberStoredAsText = false;
    /// A formula that differs from the formulas of the cells around it.
    bool mbFormula = false;
    /// A formula that leaves out cells next to the range that it refers to.
    bool mbFormulaRange = false;
    /// An unlocked cell that contains a formula.
    bool mbUnlockedFormula = false;
    /// A formula that refers to empty cells.
    bool mbEmptyCellReference = false;
    /// A value that the data validation of the cell does not allow.
    bool mbListDataValidation = false;
    /// A formula that differs from the formula of its table column.
    bool mbCalculatedColumn = false;

    /// Whether both entries ignore the same kinds of possible error.
    bool HasSameKinds(const ScIgnoredErrors& rOther) const
    {
        return mbEvalError == rOther.mbEvalError && mbTwoDigitTextYear == rOther.mbTwoDigitTextYear
               && mbNumberStoredAsText == rOther.mbNumberStoredAsText
               && mbFormula == rOther.mbFormula && mbFormulaRange == rOther.mbFormulaRange
               && mbUnlockedFormula == rOther.mbUnlockedFormula
               && mbEmptyCellReference == rOther.mbEmptyCellReference
               && mbListDataValidation == rOther.mbListDataValidation
               && mbCalculatedColumn == rOther.mbCalculatedColumn;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
