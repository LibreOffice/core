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
class ScMarkData;

namespace sc
{
/** Operation which changes the indent of marked cells. */
class ChangeIndentOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScMarkData const& mrMark;
    bool mbIncrement;

    bool runImplementation() override;

public:
    ChangeIndentOperation(ScDocShell& rDocShell, const ScMarkData& rMark, bool bIncrement,
                          bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
