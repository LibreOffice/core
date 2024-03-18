/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoUngroupSparklines.hxx>

#include <globstr.hrc>
#include <scresid.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

namespace sc
{
UndoUngroupSparklines::UndoUngroupSparklines(ScDocShell& rDocShell, ScRange const& rRange)
    : ScSimpleUndo(&rDocShell)
    , m_aRange(rRange)
{
}

UndoUngroupSparklines::~UndoUngroupSparklines() = default;

void UndoUngroupSparklines::Undo()
{
    BeginUndo();

    ScDocument& rDocument = pDocShell->GetDocument();

    for (SparklineUndoData& rUndoData : m_aUndoData)
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

void UndoUngroupSparklines::Redo()
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
                auto const& rpGroup = pSparkline->getSparklineGroup();
                m_aUndoData.emplace_back(aAddress, pSparkline->getInputRange(), rpGroup);
                auto pSparklineGroupCopy
                    = std::make_shared<sc::SparklineGroup>(rpGroup->getAttributes());
                rDocument.DeleteSparkline(aAddress);
                auto* pCreated = rDocument.CreateSparkline(aAddress, pSparklineGroupCopy);
                pCreated->setInputRange(pSparkline->getInputRange());
            }
        }
    }

    pDocShell->PostPaint(m_aRange, PaintPartFlags::All);

    EndRedo();
}

void UndoUngroupSparklines::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoUngroupSparklines::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoUngroupSparklines::GetComment() const { return ScResId(STR_UNDO_UNGROUP_SPARKLINES); }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
