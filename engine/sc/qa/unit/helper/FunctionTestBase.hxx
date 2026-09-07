/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "qahelper.hxx"

#include <address.hxx>
#include <document.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <cppunit/TestAssert.h>

// Base fixture for the function tests that load a self-checking workbook from
// sc/qa/unit/data/functions.
class FunctionTestBase : public ScModelTestBase
{
public:
    FunctionTestBase()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }

protected:
    // Walk column C from row 5 down to nLastRow on the given sheet. Every non-empty C cell is an
    // equality comparison for that row and has to evaluate to 1. The whole column is read before
    // anything is reported, so one run lists every row that disagreed instead of stopping at the
    // first one, and the report says at which stage of the test they disagreed.
    static void assertRowChecks(ScDocument& rDocument, SCTAB nTab, SCROW nLastRow,
                                const char* pStage = "")
    {
        OString aRows;
        for (SCROW nRow = 4; nRow <= nLastRow; ++nRow)
        {
            ScAddress aPosition(2, nRow, nTab);
            if (rDocument.GetCellType(aPosition) == CELLTYPE_NONE)
                continue;
            if (rDocument.GetValue(aPosition) == 1.0)
                continue;
            if (!aRows.isEmpty())
                aRows += ", ";
            // Print the two values the comparison used, so a failing row already says what we
            // computed and what was expected.
            OString aOurs = OUStringToOString(rDocument.GetString(0, nRow, nTab),
                                              RTL_TEXTENCODING_UTF8);
            OString aExpected = OUStringToOString(rDocument.GetString(1, nRow, nTab),
                                                  RTL_TEXTENCODING_UTF8);
            aRows += "C" + OString::number(nRow + 1) + " (A=" + aOurs + " B=" + aExpected;
            // The tested cell is in column F or G, depending on the layout of the row. Print
            // its value and the formula it reads back as.
            for (SCCOL nColumn = 5; nColumn <= 6; ++nColumn)
            {
                if (rDocument.GetCellType(ScAddress(nColumn, nRow, nTab)) == CELLTYPE_NONE)
                    continue;
                aRows += OString::Concat(nColumn == 5 ? " F=" : " G=")
                         + OUStringToOString(rDocument.GetString(nColumn, nRow, nTab),
                                             RTL_TEXTENCODING_UTF8);
                OUString aFormula = rDocument.GetFormula(nColumn, nRow, nTab);
                if (!aFormula.isEmpty())
                    aRows += " [" + OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8) + "]";
            }
            aRows += ")";
        }
        if (aRows.isEmpty())
            return;

        OString aLabel = "Sheet" + OString::number(nTab + 1) + " verdict cells that did not "
                         "evaluate to TRUE";
        if (*pStage)
            aLabel += OString::Concat(" ") + pStage;
        aLabel += ": " + aRows;
        CPPUNIT_FAIL(aLabel.getStr());
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
