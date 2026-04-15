/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "PivotTableOperation.hxx"

class ScDPObject;

namespace sc
{
/** Updates the pivot table identified by the input object. */
class UpdatePivotTableOperation : public PivotTableOperation
{
private:
    ScDPObject& mrDPObject;

    bool runImplementation() override;

public:
    UpdatePivotTableOperation(ScDocShell& rDocShell, ScDPObject& rDPObject, bool bRecord,
                              bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
