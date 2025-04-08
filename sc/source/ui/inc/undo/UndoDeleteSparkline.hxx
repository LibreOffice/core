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
#include <address.hxx>
#include <memory>

namespace sc
{
class SparklineGroup;

/** Undo action for deleting a Sparkline */
class UndoDeleteSparkline : public ScSimpleUndo
{
private:
    std::shared_ptr<sc::SparklineGroup> mpSparklineGroup;
    ScAddress maSparklinePosition;

public:
    UndoDeleteSparkline(ScDocShell& rDocShell, ScAddress const& rSparklinePosition);

    virtual ~UndoDeleteSparkline() override;

    void Undo() override;
    void Redo() override;
    bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    void Repeat(SfxRepeatTarget& rTarget) override;
    OUString GetComment() const override;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
