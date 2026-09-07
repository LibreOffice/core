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

class DynamicArrayTest : public FunctionTestBase
{
};

CPPUNIT_TEST_FIXTURE(DynamicArrayTest, testDynamicArrayXlsxRoundTrip)
{
    createScDoc("functions/dynamic_array/xlsx/DynamicArrayFixture.xlsx");

    auto checkVerdicts = [this]() {
        ScDocument* pDocument = getScDoc();
        // Sheet 1 "CSE Array Spill Cases", master cell A3 = AND(C5:C400).
        assertRowChecks(*pDocument, 0, 399);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 0)));
        // Sheet 2 "Implicit intersection Operator", master cell A3 = AND(C5:C200).
        assertRowChecks(*pDocument, 1, 199);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 1)));
        // Sheet 3 "Spill Operator", master cell A3 = AND(C5:C508).
        assertRowChecks(*pDocument, 2, 507);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 2)));
    };

    // The self-checking workbook holds after import, and again after an
    // XLSX save and reload.
    checkVerdicts();
    saveAndReload(TestFilter::XLSX);
    checkVerdicts();
}

CPPUNIT_TEST_FIXTURE(DynamicArrayTest, testUnionSpillIntersectionOperatorXlsxRoundTrip)
{
    createScDoc("functions/dynamic_array/xlsx/UnionSpillIntersectionOperatorTest.xlsx");

    auto checkVerdicts = [this](const char* pStage) {
        ScDocument* pDocument = getScDoc();
        assertRowChecks(*pDocument, 0, 200, pStage);
    };

    // The workbook is another application's, so the first check is against its stored
    // results and each later one against ours, with the save and reload covering the export.
    checkVerdicts("on import");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation");
    saveAndReload(TestFilter::XLSX);
    checkVerdicts("after a save and reload");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation of the reloaded document");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
