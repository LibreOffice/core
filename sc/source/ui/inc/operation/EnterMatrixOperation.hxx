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
#include <formula/grammar.hxx>

class ScDocShell;
class ScMarkData;
class ScTokenArray;

namespace sc
{
/** Operation which enters an array (matrix/CSE) formula into a range. */
class EnterMatrixOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScRange maRange;
    ScMarkData const* mpTabMark;
    ScTokenArray const* mpTokenArray;
    OUString maString;
    bool mbEnglish;
    OUString maFormulaNmsp;
    formula::FormulaGrammar::Grammar meGrammar;

    bool runImplementation() override;

public:
    EnterMatrixOperation(ScDocShell& rDocShell, const ScRange& rRange, const ScMarkData* pTabMark,
                         const ScTokenArray* pTokenArray, const OUString& rString, bool bApi,
                         bool bEnglish, const OUString& rFormulaNmsp,
                         formula::FormulaGrammar::Grammar eGrammar);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
