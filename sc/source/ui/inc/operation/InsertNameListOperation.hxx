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
/** Inserts a list of defined named ranges into the spreadsheet at the given position. */
class InsertNameListOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScAddress maStartPos;

    bool runImplementation() override;

public:
    InsertNameListOperation(ScDocShell& rDocShell, const ScAddress& rStartPos, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
