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
#include <sal/types.h>
#include <address.hxx>

class ScDocShell;
struct ScQueryParam;
class ScRange;

namespace sc
{
/** Operation which filters/queries cells in a sheet. */
class QueryOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    SCTAB mnTab;
    ScQueryParam const& mrQueryParam;
    ScRange const* mpAdvSource;

    bool runImplementation() override;

public:
    QueryOperation(ScDocShell& rDocShell, SCTAB nTab, ScQueryParam const& rQueryParam,
                   ScRange const* pAdvSource, bool bRecord, bool bApi);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
