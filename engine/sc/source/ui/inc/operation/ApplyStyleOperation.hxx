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

#include <rtl/ustring.hxx>

class ScDocShell;
class ScDocFunc;
class ScMarkData;

namespace sc
{
/** Operation which applies a named cell style to a marked selection. */
class ApplyStyleOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScMarkData const& mrMark;
    OUString const& mrStyleName;

    bool runImplementation() override;

public:
    ApplyStyleOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, ScMarkData const& rMark,
                        OUString const& rStyleName, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
