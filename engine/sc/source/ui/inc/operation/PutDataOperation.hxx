/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"

class ScDocShell;
class ScAddress;
class ScDocFunc;
class ScEditEngineDefaulter;

namespace sc
{
/** Operation which writes the content of an EditEngine into a cell,
 *  choosing between an edit-text object, a plain string, or an empty
 *  cell, and applying any leftover cell attributes from the EditEngine.
 */
class PutDataOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScAddress const& mrPosition;
    ScEditEngineDefaulter& mrEngine;

    bool runImplementation() override;

public:
    PutDataOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScAddress& rPosition,
                     ScEditEngineDefaulter& rEngine, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
