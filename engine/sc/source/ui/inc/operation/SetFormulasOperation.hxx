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
#include <sal/types.h>

#include <vector>

class ScDocShell;
class ScAddress;
class ScDocFunc;
class ScFormulaCell;

namespace sc
{
/** Operation which sets a contiguous vertical run of formula cells
 *  starting at a given cell.
 */
class SetFormulasOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScAddress const& mrPosition;
    std::vector<ScFormulaCell*>& mrCells;

    bool runImplementation() override;

public:
    SetFormulasOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScAddress& rPosition,
                         std::vector<ScFormulaCell*>& rCells, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
