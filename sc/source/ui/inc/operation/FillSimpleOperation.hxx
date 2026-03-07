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
/** Operation which fills cells by copying data from an adjacent source row or column. */
class FillSimpleOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    FillDir meDir;

    bool canRunTheOperation() const override;
    bool runImplementation() override;

public:
    FillSimpleOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScRange& rRange,
                        const ScMarkData* pTabMark, FillDir eDir, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
