/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/InsertSparklinesOperation.hxx>

#include <docsh.hxx>
#include <SparklineData.hxx>
#include <SparklineGroup.hxx>
#include <undo/UndoInsertSparkline.hxx>

namespace sc
{
InsertSparklinesOperation::InsertSparklinesOperation(
    ScDocShell& rDocShell, ScRange const& rDataRange, ScRange const& rSparklineRange,
    std::shared_ptr<SparklineGroup> const& pSparklineGroup)
    : Operation(OperationType::SparklineInsert, false, false)
    , mrDocShell(rDocShell)
    , maDataRange(rDataRange)
    , maSparklineRange(rSparklineRange)
    , mpSparklineGroup(pSparklineGroup)
{
}

bool InsertSparklinesOperation::runImplementation()
{
    if (!checkSheetViewProtection())
        return false;

    std::vector<SparklineData> aSparklineDataVector;

    if (maSparklineRange.aStart.Col() == maSparklineRange.aEnd.Col())
    {
        sal_Int32 nOutputRowSize = maSparklineRange.aEnd.Row() - maSparklineRange.aStart.Row();

        auto eInputOrientation = calculateOrientation(nOutputRowSize, maDataRange);

        if (eInputOrientation == RangeOrientation::Unknown)
            return false;

        sal_Int32 nIndex = 0;

        for (ScAddress aAddress = maSparklineRange.aStart;
             aAddress.Row() <= maSparklineRange.aEnd.Row(); aAddress.IncRow())
        {
            ScRange aInputRangeSlice = maDataRange;
            if (eInputOrientation == RangeOrientation::Row)
            {
                aInputRangeSlice.aStart.SetRow(maDataRange.aStart.Row() + nIndex);
                aInputRangeSlice.aEnd.SetRow(maDataRange.aStart.Row() + nIndex);
            }
            else
            {
                aInputRangeSlice.aStart.SetCol(maDataRange.aStart.Col() + nIndex);
                aInputRangeSlice.aEnd.SetCol(maDataRange.aStart.Col() + nIndex);
            }

            aSparklineDataVector.emplace_back(aAddress, aInputRangeSlice);

            nIndex++;
        }
    }
    else if (maSparklineRange.aStart.Row() == maSparklineRange.aEnd.Row())
    {
        sal_Int32 nOutputColSize = maSparklineRange.aEnd.Col() - maSparklineRange.aStart.Col();

        auto eInputOrientation = calculateOrientation(nOutputColSize, maDataRange);

        if (eInputOrientation == RangeOrientation::Unknown)
            return false;

        sal_Int32 nIndex = 0;

        for (ScAddress aAddress = maSparklineRange.aStart;
             aAddress.Col() <= maSparklineRange.aEnd.Col(); aAddress.IncCol())
        {
            ScRange aInputRangeSlice = maDataRange;
            if (eInputOrientation == RangeOrientation::Row)
            {
                aInputRangeSlice.aStart.SetRow(maDataRange.aStart.Row() + nIndex);
                aInputRangeSlice.aEnd.SetRow(maDataRange.aStart.Row() + nIndex);
            }
            else
            {
                aInputRangeSlice.aStart.SetCol(maDataRange.aStart.Col() + nIndex);
                aInputRangeSlice.aEnd.SetCol(maDataRange.aStart.Col() + nIndex);
            }

            aSparklineDataVector.emplace_back(aAddress, aInputRangeSlice);

            nIndex++;
        }
    }

    if (aSparklineDataVector.empty())
        return false;

    auto pUndoInsertSparkline
        = std::make_unique<UndoInsertSparkline>(mrDocShell, aSparklineDataVector, mpSparklineGroup);
    // insert the sparkline by "redoing"
    pUndoInsertSparkline->Redo();
    mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoInsertSparkline));

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
