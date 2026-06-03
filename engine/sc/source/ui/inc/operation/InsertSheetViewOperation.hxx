/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <address.hxx>
#include <SheetViewTypes.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <optional>

class ScDocShell;

namespace sc
{
/** Operation which inserts a new sheet view for a given tab. */
class InsertSheetViewOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    SCTAB mnTab;
    SheetViewID mnSheetViewID = InvalidSheetViewID;
    SCTAB mnSheetViewTab = -1;

    // Optional identity (ID, name, GUIDs) of a previously-removed sheet
    // view. Empty for a fresh insert.
    std::optional<SheetViewID> moRestoreID;
    OUString maRestoreName;
    OString maRestoreGUID;
    OString maRestoreFilterGUID;

    bool runImplementation() override;

public:
    InsertSheetViewOperation(ScDocShell& rDocShell, SCTAB nTab, bool bRecord);

    /** Set the identity (ID, name, GUIDs) of a previously-removed sheet
     *  view that this operation should reinstate. Leave unset for a
     *  fresh insert.
     */
    void setRestoreIdentity(SheetViewID nID, OUString const& rName, OString const& rGUID,
                            OString const& rFilterGUID);

    SheetViewID getSheetViewID() const { return mnSheetViewID; }
    SCTAB getSheetViewTab() const { return mnSheetViewTab; }
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
