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
#include <sal/types.h>

class ScDocFunc;
class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which applies an autoformat table style to a selected range. */
class AutoFormatOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    sal_uInt16 mnFormatNo;

    bool isInputOnSheetViewAutoFilter() const;

    bool canRunTheOperation() const override;
    bool runImplementation() override;

public:
    AutoFormatOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScRange& rRange,
                        const ScMarkData* pTabMark, sal_uInt16 nFormatNo, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
