/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoGroupSparklines.hxx>

#include <globstr.hrc>
#include <scresid.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <utility>

namespace sc
{
UndoGroupSparklines::UndoGroupSparklines(ScDocShell& rDocShell, ScRange const& rRange,
                                         std::shared_ptr<sc::SparklineGroup> pSparklineGroup)
    : ScSimpleUndo(&rDocShell)
    , m_aRange(rRange)
    , m_pSparklineGroup(std::move(pSparklineGroup))
{
}

UndoGroupSparklines::~UndoGroupSparklines() = default;

void UndoGroupSparklines::Undo()
{
    BeginUndo();

    ScDocument& rDocument = pDocShell->GetDocument();

    for (auto& rUndoData : m_aUndoData)
    {
        rDocument.DeleteSparkline(rUndoData.m_aAddress);
        auto* pCreated
            = rDocument.CreateSparkline(rUndoData.m_aAddress, rUndoData.m_pSparklineGroup);
        pCreated->setInputRange(rUndoData.m_aDataRangeList);
    }

    m_aUndoData.clear();

    pDocShell->PostPaint(m_aRange, PaintPartFlags::All);

    EndUndo();
}

void UndoGroupSparklines::Redo()
{
    BeginRedo();

    ScDocument& rDocument = pDocShell->GetDocument();

    for (ScAddress aAddress = m_aRange.aStart; aAddress.Col() <= m_aRange.aEnd.Col();
         aAddress.IncCol())
    {
        aAddress.SetRow(m_aRange.aStart.Row());
        for (; aAddress.Row() <= m_aRange.aEnd.Row(); aAddress.IncRow())
        {
            if (auto pSparkline = rDocument.GetSparkline(aAddress))
            {
                m_aUndoData.emplace_back(aAddress, pSparkline->getInputRange(),
                                         pSparkline->getSparklineGroup());

                rDocument.DeleteSparkline(aAddress);
                auto* pCreated = rDocument.CreateSparkline(aAddress, m_pSparklineGroup);
                pCreated->setInputRange(pSparkline->getInputRange());
            }
        }
    }

    pDocShell->PostPaint(m_aRange, PaintPartFlags::All);

    EndRedo();
}

void UndoGroupSparklines::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoGroupSparklines::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoGroupSparklines::GetComment() const { return ScResId(STR_UNDO_GROUP_SPARKLINES); }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
