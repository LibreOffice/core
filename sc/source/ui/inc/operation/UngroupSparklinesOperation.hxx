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
#include <rangelst.hxx>

class ScDocShell;

namespace sc
{
/** Ungroup all the sparklines in the input range and create a new group for each sparkline. */
class UngroupSparklinesOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maRange;

    bool runImplementation() override;

public:
    UngroupSparklinesOperation(ScDocShell& rDocShell, ScRange const& rRange);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
