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

class ScDocShell;

namespace sc
{
/** Moves or copies a block of cells from a source range to a destination position. */
class MoveBlockOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maSource;
    ScAddress maDestPos;
    bool mbCut;
    bool mbPaint;

    bool canRunTheOperation() const override;
    bool runImplementation() override;

public:
    MoveBlockOperation(ScDocShell& rDocShell, const ScRange& rSource, const ScAddress& rDestPos,
                       bool bCut, bool bRecord, bool bPaint, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
