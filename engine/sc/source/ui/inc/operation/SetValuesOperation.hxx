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

namespace sc
{
/** Operation which sets a contiguous vertical run of floating point values
 *  starting at a given cell.
 */
class SetValuesOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScAddress const& mrPosition;
    std::vector<double> const& mrValues;

    bool runImplementation() override;

public:
    SetValuesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScAddress& rPosition,
                       std::vector<double> const& rValues, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
