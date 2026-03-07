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
#include <types.hxx>
#include <rangelst.hxx>
#include <memory>

class ScDocShell;

namespace sc
{
class Sparkline;

/** Changes the sparkline's input data range. */
class ChangeSparklineOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    std::shared_ptr<Sparkline> mpSparkline;
    SCTAB mnTab;
    ScRangeList maDataRange;

    bool runImplementation() override;

public:
    ChangeSparklineOperation(ScDocShell& rDocShell, std::shared_ptr<Sparkline> const& rpSparkline,
                             SCTAB nTab, ScRangeList const& rDataRange);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
