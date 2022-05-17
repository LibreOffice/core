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

FunctionsTest::FunctionsTest(const OUString& rPath):
    ScBootstrapFixture(rPath)
{
}

void FunctionsTest::setUp()
{
    ScBootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

bool FunctionsTest::load(const OUString& rFilter, const OUString& rURL,
        const OUString& rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
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
            SCROW maxRow = rDoc.GetLastDataRow(tab, 2, 2, rDoc.MaxRow());
            for(SCROW row = 0; row <= maxRow; ++row)
            {
                // Column C has the check result, column D has the formula text.
                if(rDoc.HasStringData(2, row, tab))
                    continue;
                if(!rtl::math::approxEqual(1.0, rDoc.GetValue(2, row, 1)))
                    CPPUNIT_FAIL( OUString( "Testing " + rURL + " failed, "
                        + rDoc.GetAllTableNames()[tab] + ".A" + OUString::number(row+1)
                        + " \'" + rDoc.GetString(3, row, 1) + "\'"
                          " result: " + OUString::number(rDoc.GetValue(0, row, 1))
                        + ", expected: " + OUString::number(rDoc.GetValue(1, row, 1)))
                        .toUtf8().getStr());
            }
        }
    }
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rDoc.GetValue(1, 2, 0), 1e-14);

    xDocShRef->DoClose();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
