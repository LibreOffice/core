/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "functions_test.hxx"
#include <document.hxx>

#include <rtl/math.hxx>

FunctionsTest::FunctionsTest():
    ScFilterTestBase()
{
}

bool FunctionsTest::load(const OUString& rFilter, const OUString& rURL,
        const OUString& rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = loadDoc(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    CPPUNIT_ASSERT(xDocShRef.is());

    xDocShRef->DoHardRecalc();

    ScDocument& rDoc = xDocShRef->GetDocument();

    if(!rtl::math::approxEqual(1.0, rDoc.GetValue(1, 2, 0)))
    {
        // Cell B3 in Sheet1 has the cumulative success/failure result.
        // Try to find the actual failure.
        for(SCTAB tab = 1; tab <= rDoc.GetMaxTableNumber(); ++tab)
        {
            // Column "Function" has the result value, column "Expected" has the expected
            // value, Column "Correct" has the check result (1 or 0), column "FunctionString" has
            // the formula text.

            SCCOL nExpectedCol = 0;
            SCCOL nCorrectCol = 0;
            SCCOL nFunctStringCol = 0;
            for(SCCOL col = 0; col <= rDoc.MaxCol(); ++col)
            {
                if(rDoc.GetString(col, 0, tab) == "Expected")
                    nExpectedCol = col;
                else if(rDoc.GetString(col, 0, tab) == "Correct")
                    nCorrectCol = col;
                else if(rDoc.GetString(col, 0, tab) == "FunctionString")
                {
                    nFunctStringCol = col;
                    break; // Should be the last one
                }
            }

            CPPUNIT_ASSERT_MESSAGE("Column \"Expected\" not found", nExpectedCol != 0);
            CPPUNIT_ASSERT_MESSAGE("Column \"Correct\" not found", nCorrectCol != 0);
            CPPUNIT_ASSERT_MESSAGE("Column \"FunctionString\" not found", nFunctStringCol != 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                    "Function columns != Expected columns", nExpectedCol, static_cast<SCCOL>(nCorrectCol - nExpectedCol));

            SCROW maxRow = rDoc.GetLastDataRow(tab, nCorrectCol, nCorrectCol, rDoc.MaxRow());
            for(SCROW row = 1; row <= maxRow; ++row)
            {
                if(!rDoc.HasData(nCorrectCol, row, tab))
                    continue;
                if (!rtl::math::approxEqual(1.0, rDoc.GetValue(nCorrectCol, row, tab)))
                {
                    OUString result;
                    OUString expected;
                    for (SCCOL nOffset = 0; nOffset < nExpectedCol; ++ nOffset)
                    {
                        if (rDoc.HasValueData(nExpectedCol + nOffset, row, tab))
                        {
                            // snprintf provides requested precision, unlike OUString::number, which
                            // rounds to 15 decimals
                            char buf[25];
                            int len = snprintf(buf, 25, "%.17G", rDoc.GetValue(0 + nOffset, row, tab));
                            result += OUString::createFromAscii(std::string_view(buf, len));
                            len = snprintf(buf, 25, "%.17G", rDoc.GetValue(nExpectedCol + nOffset, row, tab));
                            expected += OUString::createFromAscii(std::string_view(buf, len));
                        }
                        else
                        {
                            result += rDoc.GetString(0 + nOffset, row, tab);
                            expected += rDoc.GetString(nExpectedCol + nOffset, row, tab);
                        }

                        if (nOffset < nExpectedCol - 1)
                        {
                            result += ", ";
                            expected += ", ";
                        }
                    }
                    CPPUNIT_FAIL( OUString( "Testing " + rURL + " failed, "
                                + rDoc.GetAllTableNames()[tab] + ".A" + OUString::number(row+1)
                                + " \'" + rDoc.GetString(nFunctStringCol, row, tab) + "\'"
                                " result: '" + result
                                + "', expected: '" + expected + "'")
                            .toUtf8().getStr());
                }
            }
        }
    }
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rDoc.GetValue(1, 2, 0), 1e-14);

    xDocShRef->DoClose();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
