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

#include "helper/FunctionTestBase.hxx"

#include <address.hxx>
#include <document.hxx>

class PivotByFunctionTest : public FunctionTestBase
{
protected:
    // The single sheet checks each case in column C of rows 5 to 204, and its master cell A3 is
    // AND(C5:C204).
    void checkVerdicts(const char* pStage)
    {
        ScDocument* pDocument = getScDoc();
        assertRowChecks(*pDocument, 0, 203, pStage);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 0)));
    }
};

CPPUNIT_TEST_FIXTURE(PivotByFunctionTest, testPivotByXlsxRoundTrip)
{
    createScDoc("functions/spreadsheet/xlsx/PivotByFixture.xlsx");

    checkVerdicts("on import");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation");
    saveAndReload(TestFilter::XLSX);
    checkVerdicts("after a save and reload");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation of the reloaded document");
}

CPPUNIT_TEST_FIXTURE(PivotByFunctionTest, testPivotByOdsRoundTrip)
{
    createScDoc("functions/spreadsheet/xlsx/PivotByFixture.xlsx");

    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation");
    saveAndReload(TestFilter::ODS);
    checkVerdicts("after a save to ODS and reload");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation of the reloaded document");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
