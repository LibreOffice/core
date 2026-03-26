/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    ScRange aDataRange = convertRange(maDataRange);
    ScRange aSparklineRange = convertRange(maSparklineRange);

    std::vector<SparklineData> aSparklineDataVector;

    if (aSparklineRange.aStart.Col() == aSparklineRange.aEnd.Col())
    {
        sal_Int32 nOutputRowSize = aSparklineRange.aEnd.Row() - aSparklineRange.aStart.Row();

        auto eInputOrientation = calculateOrientation(nOutputRowSize, aDataRange);

        if (eInputOrientation == RangeOrientation::Unknown)
            return false;

        sal_Int32 nIndex = 0;

        for (ScAddress aAddress = aSparklineRange.aStart;
             aAddress.Row() <= aSparklineRange.aEnd.Row(); aAddress.IncRow())
        {
            ScRange aInputRangeSlice = aDataRange;
            if (eInputOrientation == RangeOrientation::Row)
            {
                aInputRangeSlice.aStart.SetRow(aDataRange.aStart.Row() + nIndex);
                aInputRangeSlice.aEnd.SetRow(aDataRange.aStart.Row() + nIndex);
            }
            else
            {
                aInputRangeSlice.aStart.SetCol(aDataRange.aStart.Col() + nIndex);
                aInputRangeSlice.aEnd.SetCol(aDataRange.aStart.Col() + nIndex);
            }

            aSparklineDataVector.emplace_back(aAddress, aInputRangeSlice);

            nIndex++;
        }
    }
    else if (aSparklineRange.aStart.Row() == aSparklineRange.aEnd.Row())
    {
        sal_Int32 nOutputColSize = aSparklineRange.aEnd.Col() - aSparklineRange.aStart.Col();

        auto eInputOrientation = calculateOrientation(nOutputColSize, aDataRange);

        if (eInputOrientation == RangeOrientation::Unknown)
            return false;

        sal_Int32 nIndex = 0;

        for (ScAddress aAddress = aSparklineRange.aStart;
             aAddress.Col() <= aSparklineRange.aEnd.Col(); aAddress.IncCol())
        {
            ScRange aInputRangeSlice = aDataRange;
            if (eInputOrientation == RangeOrientation::Row)
            {
                aInputRangeSlice.aStart.SetRow(aDataRange.aStart.Row() + nIndex);
                aInputRangeSlice.aEnd.SetRow(aDataRange.aStart.Row() + nIndex);
            }
            else
            {
                aInputRangeSlice.aStart.SetCol(aDataRange.aStart.Col() + nIndex);
                aInputRangeSlice.aEnd.SetCol(aDataRange.aStart.Col() + nIndex);
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

    syncSheetViews();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
