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

class ScDocShell;
class ScDPObject;

namespace sc
{
/** Creates a pivot table from the input object. */
class CreatePivotTableOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScDPObject const& mrDPObj;

    bool runImplementation() override;

public:
    CreatePivotTableOperation(ScDocShell& rDocShell, ScDPObject const& rDPObj, bool bRecord,
                              bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
