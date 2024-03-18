/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoDeleteSparkline.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <Sparkline.hxx>

namespace sc
{
UndoDeleteSparkline::UndoDeleteSparkline(ScDocShell& rDocShell, ScAddress const& rSparklinePosition)
    : ScSimpleUndo(&rDocShell)
    , maSparklinePosition(rSparklinePosition)
{
}

UndoDeleteSparkline::~UndoDeleteSparkline() {}

void UndoDeleteSparkline::Undo()
{
    BeginUndo();

    ScDocument& rDocument = pDocShell->GetDocument();
    auto pSparkline = rDocument.GetSparkline(maSparklinePosition);
    if (!pSparkline)
    {
        rDocument.CreateSparkline(maSparklinePosition, mpSparklineGroup);
    }
    else
    {
        SAL_WARN("sc", "Can't undo deletion if the sparkline at that address already exists.");
    }

    pDocShell->PostPaintCell(maSparklinePosition);

    EndUndo();
}

void UndoDeleteSparkline::Redo()
{
    BeginRedo();

    ScDocument& rDocument = pDocShell->GetDocument();
    if (auto pSparkline = rDocument.GetSparkline(maSparklinePosition))
    {
        mpSparklineGroup = pSparkline->getSparklineGroup();
        rDocument.DeleteSparkline(maSparklinePosition);
    }
    else
    {
        SAL_WARN("sc", "Can't delete a sparkline that donesn't exist.");
    }

    pDocShell->PostPaintCell(maSparklinePosition);

    EndRedo();
}

void UndoDeleteSparkline::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoDeleteSparkline::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoDeleteSparkline::GetComment() const { return ScResId(STR_UNDO_DELETE_SPARKLINE); }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
