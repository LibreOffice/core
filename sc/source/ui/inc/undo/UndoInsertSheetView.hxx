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
#include <address.hxx>
#include <SheetViewTypes.hxx>
#include <memory>

class SdrUndoAction;

namespace sc
{
/** Undo action for inserting a sheet view. */
class UndoInsertSheetView : public ScSimpleUndo
{
private:
    std::unique_ptr<SdrUndoAction> mpDrawUndo;
    SCTAB mnTab; // the default/source tab
    SCTAB mnSheetViewTab; // the sheet view tab
    SheetViewID mnSheetViewID;

public:
    UndoInsertSheetView(ScDocShell& rDocShell, SCTAB nTab, SCTAB nSheetViewTab,
                        SheetViewID nSheetViewID);
    virtual ~UndoInsertSheetView() override;

    void Undo() override;
    void Redo() override;
    void Repeat(SfxRepeatTarget& rTarget) override;
    bool CanRepeat(SfxRepeatTarget& rTarget) const override;
    OUString GetComment() const override;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
