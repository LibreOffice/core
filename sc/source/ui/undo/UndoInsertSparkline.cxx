/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoInsertSparkline.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <SparklineData.hxx>
#include <utility>

namespace sc
{
UndoInsertSparkline::UndoInsertSparkline(ScDocShell& rShell,
                                         std::vector<SparklineData> aSparklineDataVector,
                                         std::shared_ptr<sc::SparklineGroup> pSparklineGroup)
    : ScSimpleUndo(rShell)
    , maSparklineDataVector(std::move(aSparklineDataVector))
    , mpSparklineGroup(std::move(pSparklineGroup))
{
}

UndoInsertSparkline::~UndoInsertSparkline() {}

void UndoInsertSparkline::Undo()
{
    BeginUndo();

    ScDocument& rDocument = rDocShell.GetDocument();
    ScRangeList aRanges;
    for (auto const& rSparklineData : maSparklineDataVector)
    {
        rDocument.DeleteSparkline(rSparklineData.maPosition);

        ScRange aCurrRange(rSparklineData.maPosition);

        if (aCurrRange.aStart == aCurrRange.aEnd
            && rDocument.HasAttrib(aCurrRange, HasAttrFlags::Merged))
        {
            rDocument.ExtendMerge(aCurrRange);
        }

        aRanges.push_back(aCurrRange);
    }

    rDocShell.PostPaint(aRanges, PaintPartFlags::All, SC_PF_TESTMERGE);

    EndUndo();
}

void UndoInsertSparkline::Redo()
{
    BeginRedo();

    ScDocument& rDocument = rDocShell.GetDocument();
    ScRangeList aRanges;
    for (auto const& rSparklineData : maSparklineDataVector)
    {
        auto* pCreated = rDocument.CreateSparkline(rSparklineData.maPosition, mpSparklineGroup);
        pCreated->setInputRange(rSparklineData.maData);

        ScRange aCurrRange(rSparklineData.maPosition);

        if (aCurrRange.aStart == aCurrRange.aEnd
            && rDocument.HasAttrib(aCurrRange, HasAttrFlags::Merged))
        {
            rDocument.ExtendMerge(aCurrRange);
        }

        aRanges.push_back(aCurrRange);
    }

    rDocShell.PostPaint(aRanges, PaintPartFlags::All, SC_PF_TESTMERGE);

    EndRedo();
}

void UndoInsertSparkline::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoInsertSparkline::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoInsertSparkline::GetComment() const
{
    return ScResId(STR_UNDO_INSERT_SPARKLINE_GROUP);
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
