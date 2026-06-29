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

#include "helper/qahelper.hxx"

#include <address.hxx>
#include <document.hxx>
#include <rtl/string.hxx>

class DynamicArrayTest : public ScModelTestBase
{
public:
    DynamicArrayTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

namespace
{
// Walk column C from row 5 down to nLastRow on the given sheet.
// Every non-empty C cell is a per-row equality comparison that the
// fixture expects to evaluate to 1. A failure reports the row that
// disagreed so the offending scenario is easy to spot.
void assertRowChecks(ScDocument& rDocument, SCTAB nTab, SCROW nLastRow)
{
    for (SCROW nRow = 4; nRow <= nLastRow; ++nRow)
    {
        ScAddress aPosition(2, nRow, nTab);
        if (rDocument.GetCellType(aPosition) == CELLTYPE_NONE)
            continue;
        const double fValue = rDocument.GetValue(aPosition);
        if (fValue == 1.0)
            continue;
        const OString aLabel = "Sheet" + OString::number(nTab + 1) + " C"
                               + OString::number(nRow + 1) + " verdict cell did not evaluate to TRUE";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aLabel.getStr(), 1.0, fValue);
    }
}
}

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
