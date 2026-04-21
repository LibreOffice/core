/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <undobase.hxx>
#include <address.hxx>

namespace sc
{
/** Undo action for a pivot table spill resolution. When a spilled pivot table
    is rendered (because blocking cells were cleared), this undo action can
    restore the #SPILL! state. Redo re-renders the pivot table. */
class UndoSpillPivotTable : public ScSimpleUndo
{
public:
    UndoSpillPivotTable(ScDocShell& rDocShell, const OUString& rDPName,
                        const ScRange& rOutputRange);

    void Undo() override;
    void Redo() override;
    void Repeat(SfxRepeatTarget& rTarget) override;
    bool CanRepeat(SfxRepeatTarget& rTarget) const override;

    OUString GetComment() const override;

private:
    OUString maDPName;
    ScRange maOutputRange; // output range when rendered (non-spill state)
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
