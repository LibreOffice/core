/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <undo/UndoEditSparklineGroup.hxx>

#include <globstr.hrc>
#include <scresid.hxx>

#include <SparklineGroup.hxx>
#include <SparklineAttributes.hxx>
#include <utility>

namespace sc
{
UndoEditSparklneGroup::UndoEditSparklneGroup(
    ScDocShell& rDocShell, std::shared_ptr<sc::SparklineGroup> const& pSparklineGroup,
    sc::SparklineAttributes aAttributes)
    : ScSimpleUndo(&rDocShell)
    , m_pSparklineGroup(pSparklineGroup)
    , m_aNewAttributes(std::move(aAttributes))
    , m_aOriginalAttributes(pSparklineGroup->getAttributes())
{
}

UndoEditSparklneGroup::~UndoEditSparklneGroup() = default;

void UndoEditSparklneGroup::Undo()
{
    BeginUndo();

    m_pSparklineGroup->setAttributes(m_aOriginalAttributes);
    pDocShell->PostPaintGridAll();

    EndUndo();
}

void UndoEditSparklneGroup::Redo()
{
    BeginRedo();

    m_pSparklineGroup->setAttributes(m_aNewAttributes);
    pDocShell->PostPaintGridAll();

    EndRedo();
}

void UndoEditSparklneGroup::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoEditSparklneGroup::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoEditSparklneGroup::GetComment() const
{
    return ScResId(STR_UNDO_EDIT_SPARKLINE_GROUP);
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
