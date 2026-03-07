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
#include <memory>

class ScDocShell;

namespace sc
{
class SparklineGroup;

/** Groups the sparklines in the input range together. */
class GroupSparklinesOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maRange;
    std::shared_ptr<SparklineGroup> mpGroup;

    bool runImplementation() override;

public:
    GroupSparklinesOperation(ScDocShell& rDocShell, ScRange const& rRange,
                             std::shared_ptr<SparklineGroup> const& rpGroup);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
