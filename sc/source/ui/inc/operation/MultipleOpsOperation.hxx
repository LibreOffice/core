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
#include <paramisc.hxx>

class ScDocShell;
class ScMarkData;

namespace sc
{
/** Operation which fills a range with formulas substituting varying input values (Multiple Operations). */
class MultipleOpsOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    ScTabOpParam maParam;

    ScRefAddress convertRefAddress(ScRefAddress const& rRefAddress);
    bool runImplementation() override;

public:
    MultipleOpsOperation(ScDocShell& rDocShell, const ScRange& rRange, const ScMarkData* pTabMark,
                         const ScTabOpParam& rParam, bool bRecord, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
