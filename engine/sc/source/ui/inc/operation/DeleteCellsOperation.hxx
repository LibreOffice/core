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
#include <global.hxx>
#include <address.hxx>

class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which deletes cells, rows, or columns. */
class SC_DLLPUBLIC DeleteCellsOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    DelCellCmd meCmd;

    bool canRunTheOperation() const override;
    bool runImplementation() override;

    static OperationType toOperationType(DelCellCmd eCmd);

public:
    DeleteCellsOperation(ScDocShell& rDocShell, ScRange const& rRange, ScMarkData const* pTabMark,
                         DelCellCmd eCmd, bool bApi);
};

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
