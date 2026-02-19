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
#include <global.hxx>
#include <address.hxx>

class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which inserts cells, rows, or columns. */
class InsertCellsOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    InsCellCmd meCmd;
    bool mbPartOfPaste;
    size_t mnInsertCount;

    bool runImplementation() override;

    static OperationType toOperationType(InsCellCmd eCmd);

public:
    InsertCellsOperation(ScDocShell& rDocShell, ScRange const& rRange, ScMarkData const* pTabMark,
                         InsCellCmd eCmd, bool bRecord, bool bApi, bool bPartOfPaste,
                         size_t nInsertCount);
};

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
