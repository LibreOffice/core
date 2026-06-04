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

#include <formula/grammar.hxx>
#include <rtl/ustring.hxx>

class ScDocShell;
class ScAddress;
class ScDocFunc;

namespace sc
{
/** Operation which writes a text into a cell, optionally interpreting
 *  it as a formula, number or string.
 */
class SetCellTextOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScAddress const& mrPosition;
    OUString const& mrText;
    bool mbInterpret;
    bool mbEnglish;
    formula::FormulaGrammar::Grammar meGrammar;

    bool runImplementation() override;

public:
    SetCellTextOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScAddress& rPosition,
                         OUString const& rText, bool bInterpret, bool bEnglish, bool bApi,
                         formula::FormulaGrammar::Grammar eGrammar);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
