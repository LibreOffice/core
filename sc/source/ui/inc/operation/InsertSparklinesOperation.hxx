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

/** Inserts new sparklines at the input range, and add them into the input group. */
class InsertSparklinesOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maDataRange;
    ScRange maSparklineRange;
    std::shared_ptr<SparklineGroup> mpSparklineGroup;

    bool runImplementation() override;

public:
    InsertSparklinesOperation(ScDocShell& rDocShell, ScRange const& rDataRange,
                              ScRange const& rSparklineRange,
                              std::shared_ptr<SparklineGroup> const& pSparklineGroup);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
