/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoDeleteSparklineGroup.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <document.hxx>

#include <Sparkline.hxx>
#include <SparklineList.hxx>
#include <SparklineGroup.hxx>
#include <utility>

namespace sc
{
UndoDeleteSparklineGroup::UndoDeleteSparklineGroup(
    ScDocShell& rDocSh, std::shared_ptr<sc::SparklineGroup> pSparklineGroup, SCTAB nTab)
    : ScSimpleUndo(rDocSh)
    , mpSparklineGroup(std::move(pSparklineGroup))
    , mnTab(nTab)
{
}

UndoDeleteSparklineGroup::~UndoDeleteSparklineGroup() {}

void UndoDeleteSparklineGroup::Undo()
{
    BeginUndo();

    ScDocument& rDocument = rDocShell.GetDocument();

    for (auto const& pSparkline : maSparklines)
    {
        ScAddress aAddress(pSparkline->getColumn(), pSparkline->getRow(), mnTab);
        auto* pNewSparkline = rDocument.CreateSparkline(aAddress, mpSparklineGroup);
        pNewSparkline->setInputRange(pSparkline->getInputRange());
    }

    rDocShell.PostPaintGridAll();

    EndUndo();
}

void UndoDeleteSparklineGroup::Redo()
{
    BeginRedo();

    ScDocument& rDocument = rDocShell.GetDocument();
    auto* pList = rDocument.GetSparklineList(mnTab);
    if (pList)
    {
        maSparklines = pList->getSparklinesFor(mpSparklineGroup);

        for (auto const& pSparkline : maSparklines)
        {
            ScAddress aAddress(pSparkline->getColumn(), pSparkline->getRow(), mnTab);
            rDocument.DeleteSparkline(aAddress);
        }
    }
    rDocShell.PostPaintGridAll();

    EndRedo();
}

void UndoDeleteSparklineGroup::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoDeleteSparklineGroup::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoDeleteSparklineGroup::GetComment() const
{
    return ScResId(STR_UNDO_DELETE_SPARKLINE_GROUP);
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
