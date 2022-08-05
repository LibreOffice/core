/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <undobase.hxx>
#include <memory>

#include <SparklineAttributes.hxx>
#include <SparklineGroup.hxx>

namespace sc
{
/** Undo action for editing a Sparkline */
class UndoEditSparklneGroup : public ScSimpleUndo
{
private:
    std::shared_ptr<sc::SparklineGroup> m_pSparklineGroup;
    sc::SparklineAttributes m_aNewAttributes;
    sc::SparklineAttributes m_aOriginalAttributes;

public:
    UndoEditSparklneGroup(ScDocShell& rDocShell,
                          std::shared_ptr<sc::SparklineGroup> const& rSparklineGroup,
                          sc::SparklineAttributes aAttributes);
    virtual ~UndoEditSparklneGroup() override;

    void Undo() override;
    void Redo() override;
    bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    void Repeat(SfxRepeatTarget& rTarget) override;
    OUString GetComment() const override;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
