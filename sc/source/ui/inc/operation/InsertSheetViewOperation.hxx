/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <address.hxx>
#include <SheetViewTypes.hxx>

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

    bool runImplementation() override;

public:
    InsertSheetViewOperation(ScDocShell& rDocShell, SCTAB nTab, bool bRecord);

    SheetViewID getSheetViewID() const { return mnSheetViewID; }
    SCTAB getSheetViewTab() const { return mnSheetViewTab; }
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
