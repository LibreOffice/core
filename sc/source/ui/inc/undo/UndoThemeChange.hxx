/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <undobase.hxx>

namespace sc
{
class UndoThemeChange : public ScSimpleUndo
{
private:
    std::shared_ptr<model::ColorSet> mpOldColorSet;
    std::shared_ptr<model::ColorSet> mpNewColorSet;

public:
    UndoThemeChange(ScDocShell& rDocShell, std::shared_ptr<model::ColorSet> const& pOldColorSet,
                    std::shared_ptr<model::ColorSet> const& pNewColorSet);
    virtual ~UndoThemeChange() override;

    void Undo() override;
    void Redo() override;
    bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    void Repeat(SfxRepeatTarget& rTarget) override;
    OUString GetComment() const override;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
