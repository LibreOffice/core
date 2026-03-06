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
#include <global.hxx>

class ScDocFunc;
class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which fills a range with a numeric or date series. */
class FillSeriesOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    FillDir meDir;
    FillCmd meCmd;
    FillDateCmd meDateCmd;
    double mfStart;
    double mfStep;
    double mfMax;

    bool runImplementation() override;

public:
    FillSeriesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScRange& rRange,
                        const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd,
                        FillDateCmd eDateCmd, double fStart, double fStep, double fMax, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
