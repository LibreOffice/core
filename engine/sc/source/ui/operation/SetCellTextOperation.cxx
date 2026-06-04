/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetCellTextOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <address.hxx>
#include <document.hxx>
#include <externalrefmgr.hxx>
#include <formulacell.hxx>
#include <stringutil.hxx>

#include <i18nlangtag/lang.h>

#include <optional>

namespace sc
{
SetCellTextOperation::SetCellTextOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                           const ScAddress& rPosition, OUString const& rText,
                                           bool bInterpret, bool bEnglish, bool bApi,
                                           formula::FormulaGrammar::Grammar eGrammar)
    : Operation(OperationType::SetCellText, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrText(rText)
    , mbInterpret(bInterpret)
    , mbEnglish(bEnglish)
    , meGrammar(eGrammar)
{
}

bool SetCellTextOperation::runImplementation()
{
    bool bSet = false;
    if (mbInterpret)
    {
        if (mbEnglish)
        {
            ScDocument& rDoc = mrDocShell.GetDocument();

            ::std::optional<ScExternalRefManager::ApiGuard> pExtRefGuard;
            if (mbApi)
                pExtRefGuard.emplace(rDoc);

            ScInputStringType aRes = ScStringUtil::parseInputString(rDoc.GetNonThreadedContext(),
                                                                    mrText, LANGUAGE_ENGLISH_US);

            switch (aRes.meType)
            {
                case ScInputStringType::Formula:
                    bSet = mrDocFunc.SetFormulaCell(
                        mrPosition, new ScFormulaCell(rDoc, mrPosition, aRes.maText, meGrammar),
                        !mbApi);
                    break;
                case ScInputStringType::Number:
                    bSet = mrDocFunc.SetValueCell(mrPosition, aRes.mfValue, !mbApi);
                    break;
                case ScInputStringType::Text:
                    bSet = mrDocFunc.SetStringOrEditCell(mrPosition, aRes.maText, !mbApi);
                    break;
                default:;
            }
        }
        // otherwise keep Null -> SetString with local formulas/number formats
    }
    else if (!mrText.isEmpty())
    {
        bSet = mrDocFunc.SetStringOrEditCell(mrPosition, mrText, !mbApi);
    }

    if (!bSet)
    {
        bool bNumFmtSet = false;
        bSet = mrDocFunc.SetNormalString(bNumFmtSet, mrPosition, mrText, mbApi);
    }
    return bSet;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
