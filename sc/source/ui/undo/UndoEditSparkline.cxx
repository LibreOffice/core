/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoEditSparkline.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <Sparkline.hxx>
#include <utility>

namespace sc
{
UndoEditSparkline::UndoEditSparkline(ScDocShell& rDocShell,
                                     std::shared_ptr<sc::Sparkline> pSparkline, SCTAB nTab,
                                     ScRangeList aDataRange)
    : ScSimpleUndo(&rDocShell)
    , mpSparkline(std::move(pSparkline))
    , mnTab(nTab)
    , maOldDataRange(mpSparkline->getInputRange())
    , maNewDataRange(std::move(aDataRange))
{
}

UndoEditSparkline::~UndoEditSparkline() = default;

void UndoEditSparkline::Undo()
{
    BeginUndo();

    mpSparkline->setInputRange(maOldDataRange);

    pDocShell->PostPaintCell(ScAddress(mpSparkline->getColumn(), mpSparkline->getRow(), mnTab));

    EndUndo();
}

void UndoEditSparkline::Redo()
{
    BeginRedo();

    mpSparkline->setInputRange(maNewDataRange);

    pDocShell->PostPaintCell(ScAddress(mpSparkline->getColumn(), mpSparkline->getRow(), mnTab));

    EndRedo();
}

void UndoEditSparkline::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoEditSparkline::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoEditSparkline::GetComment() const { return ScResId(STR_UNDO_EDIT_SPARKLINE); }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
