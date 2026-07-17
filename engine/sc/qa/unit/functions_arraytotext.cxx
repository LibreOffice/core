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

class ArrayToTextTest : public ScModelTestBase
{
public:
    ArrayToTextTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

namespace
{
// Walk column C from row 5 down to nLastRow. Every non-empty C cell is a
// per-row comparison between our ARRAYTOTEXT result and the value the
// authoring spreadsheet program computed, and the fixture expects each to
// evaluate to 1. A failure reports the row that disagreed.
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
        const OString aLabel
            = "C" + OString::number(nRow + 1) + " verdict cell did not evaluate to TRUE";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aLabel.getStr(), 1.0, fValue);
    }
}
}

CPPUNIT_TEST_FIXTURE(ArrayToTextTest, testArrayToTextXlsxRoundTrip)
{
    createScDoc("functions/text/xlsx/ArrayToTextFixture.xlsx");

    auto checkVerdicts = [this]() {
        ScDocument* pDocument = getScDoc();
        // The single sheet has its master cell A3 = AND(C5:C204).
        assertRowChecks(*pDocument, 0, 203);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 0)));
    };

    // The self-checking workbook holds after import and after a hard
    // recalc, then again after an XLSX save and reload plus one more
    // recalc, so both the cached results and a fresh evaluation are
    // covered on the export path too.
    checkVerdicts();
    getScDoc()->CalcAll();
    checkVerdicts();
    saveAndReload(TestFilter::XLSX);
    checkVerdicts();
    getScDoc()->CalcAll();
    checkVerdicts();

    // The exported XLSX carries the ARRAYTOTEXT formula. A5 is the first
    // one, written with the _xlfn. prefix that marks a newer function.
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[@r='5']/x:c[@r='A5']/x:f",
                       u"_xlfn.ARRAYTOTEXT(H5:I5)");
}

CPPUNIT_TEST_FIXTURE(ArrayToTextTest, testArrayToTextOdsRoundTrip)
{
    createScDoc("functions/text/xlsx/ArrayToTextFixture.xlsx");

    auto checkVerdicts = [this]() {
        ScDocument* pDocument = getScDoc();
        // The single sheet has its master cell A3 = AND(C5:C204).
        assertRowChecks(*pDocument, 0, 203);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 0)));
    };

    // Load the same workbook from XLSX and check it after a hard recalc,
    // then save to ODS, reload, and recalc again, so both the cached
    // results and a fresh evaluation are covered on the ODF path.
    checkVerdicts();
    getScDoc()->CalcAll();
    checkVerdicts();
    saveAndReload(TestFilter::ODS);
    checkVerdicts();
    getScDoc()->CalcAll();
    checkVerdicts();

    // The exported ODS carries the ARRAYTOTEXT formula. A5 is the first
    // one; ODF writes it under the COM.MICROSOFT namespace.
    xmlDocUniquePtr pContent = parseExport(u"content.xml"_ustr);
    assertXPath(pContent, "(//table:table-cell[contains(@table:formula, 'ARRAYTOTEXT')])[1]",
                "formula", u"of:=COM.MICROSOFT.ARRAYTOTEXT([.H5:.I5])");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
