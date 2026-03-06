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
#include <tools/solar.h>

class ScDocFunc;
class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which performs smart autofill, extending a detected pattern into adjacent cells. */
class FillAutoOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScRange& mrRange;
    ScMarkData const* mpTabMark;
    FillDir meDir;
    FillCmd meCmd;
    FillDateCmd meDateCmd;
    sal_uLong mnCount;
    double mfStep;
    double mfMax;

    bool runImplementation() override;

public:
    FillAutoOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, ScRange& rRange,
                      const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                      sal_uLong nCount, double fStep, double fMax, bool bRecord, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
