/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <test/htmltesttools.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <comphelper/propertyvalue.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <helper/qahelper.hxx>
#include <impex.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sc/source/filter/html/ fixes.
class Test : public ScModelTestBase, public HtmlTestTools
{
public:
    Test()
        : ScModelTestBase("/sc/qa/filter/html/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdAsText)
{
    // Given a document with an A2 cell that contains "02" as text:
    OUString aURL = createFileURL(u"text.html");

    // When loading that document to Calc:
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue("DocumentService",
                                      OUString("com.sun.star.sheet.SpreadsheetDocument")),
    };
    loadWithParams(aURL, aParams);

    // Then make sure "01" is not auto-converted to 1, as a number:
    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSheets(xDocument->getSheets(), uno::UNO_QUERY);
    uno::Reference<table::XCellRange> xSheet(xSheets->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell(xSheet->getCellByPosition(0, 1), uno::UNO_QUERY);
    table::CellContentType eType{};
    xCell->getPropertyValue("CellContentType") >>= eType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (TEXT)
    // - Actual  : 1 (VALUE)
    // i.e. data-sheets-value was ignored on import.
    CPPUNIT_ASSERT_EQUAL(table::CellContentType_TEXT, eType);
}

CPPUNIT_TEST_FIXTURE(Test, testPasteTdAsText)
{
    // Given an empty document:
    createScDoc();

    // When pasting HTML with an A2 cell that contains "01" as text:
    ScDocument* pDoc = getScDoc();
    ScAddress aCellPos(/*nColP=*/0, /*nRowP=*/0, /*nTabP=*/0);
    ScImportExport aImporter(*pDoc, aCellPos);
    SvFileStream aFile(createFileURL(u"text.html"), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    aMemory.Seek(0);
    CPPUNIT_ASSERT(aImporter.ImportStream(aMemory, OUString(), SotClipboardFormatId::HTML));

    // Then make sure "01" is not auto-converted to 1, as a number:
    aCellPos = ScAddress(/*nColP=*/0, /*nRowP=*/1, /*nTabP=*/0);
    CellType eCellType = pDoc->GetCellType(aCellPos);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (CELLTYPE_STRING)
    // - Actual  : 1 (CELLTYPE_VALUE)
    // i.e. data-sheets-value was ignored on paste.
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, eCellType);
}

CPPUNIT_TEST_FIXTURE(Test, testPasteTdAsBools)
{
    // Given an empty document:
    createScDoc();

    // When pasting HTML with bool cells:
    ScDocument* pDoc = getScDoc();
    ScAddress aCellPos(/*nColP=*/0, /*nRowP=*/0, /*nTabP=*/0);
    ScImportExport aImporter(*pDoc, aCellPos);
    SvFileStream aFile(createFileURL(u"bool.html"), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    aMemory.Seek(0);
    CPPUNIT_ASSERT(aImporter.ImportStream(aMemory, OUString(), SotClipboardFormatId::HTML));

    // Then make sure A1's type is bool, value is true:
    sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(/*col=*/0, /*row=*/0, /*tab=*/0);
    const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: BOOLEAN
    // - Actual  : General
    // i.e. data-sheets-value's bool case was ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("BOOLEAN"), pNumberFormat->GetFormatstring());
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1), pDoc->GetValue(/*col=*/0, /*row=*/0, /*tab=*/0));
    // And make sure A2's type is bool, value is true:
    nNumberFormat = pDoc->GetNumberFormat(/*col=*/0, /*row=*/1, /*tab=*/0);
    pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
    CPPUNIT_ASSERT_EQUAL(OUString("BOOLEAN"), pNumberFormat->GetFormatstring());
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(0), pDoc->GetValue(/*col=*/0, /*row=*/1, /*tab=*/0));
}

CPPUNIT_TEST_FIXTURE(Test, testPasteTdAsFormattedNumber)
{
    // Given an empty document:
    createScDoc();

    // When pasting HTML with cells containing formatted numbers:
    ScDocument* pDoc = getScDoc();
    ScAddress aCellPos(/*nColP=*/0, /*nRowP=*/0, /*nTabP=*/0);
    ScImportExport aImporter(*pDoc, aCellPos);
    SvFileStream aFile(createFileURL(u"numberformat.html"), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    aMemory.Seek(0);
    CPPUNIT_ASSERT(aImporter.ImportStream(aMemory, OUString(), SotClipboardFormatId::HTML));

    // Then make sure A1's type is a formatted number, value is 1000:
    sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(/*col=*/0, /*row=*/0, /*tab=*/0);
    const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: #,##0.00
    // - Actual  : General
    // i.e. the number was wasted without a matching number format.
    CPPUNIT_ASSERT_EQUAL(OUString("#,##0.00"), pNumberFormat->GetFormatstring());
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000),
                         pDoc->GetValue(/*col=*/0, /*row=*/0, /*tab=*/0));
}

CPPUNIT_TEST_FIXTURE(Test, testPasteTdAsFormula)
{
    // Given an empty document:
    createScDoc();

    // When pasting HTML with cells containing a formula:
    ScDocument* pDoc = getScDoc();
    ScAddress aCellPos(/*nColP=*/0, /*nRowP=*/0, /*nTabP=*/0);
    ScImportExport aImporter(*pDoc, aCellPos);
    SvFileStream aFile(createFileURL(u"formula.html"), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    aMemory.Seek(0);
    CPPUNIT_ASSERT(aImporter.ImportStream(aMemory, OUString(), SotClipboardFormatId::HTML));

    // Then make sure C1 is a sum and it evaluates to 3:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: =SUM(A1:B1)
    // - Actual  :
    // i.e. only the formula result was imported, not the formula.
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:B1)"),
                         pDoc->GetFormula(/*col=*/2, /*row=*/0, /*tab=*/0));
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(3), pDoc->GetValue(/*col=*/2, /*row=*/0, /*tab=*/0));
}

CPPUNIT_TEST_FIXTURE(Test, testPasteSingleCell)
{
    // Given a document with '1' in A1 and '2' in B1:
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    pDoc->SetValue(ScAddress(1, 0, 0), 2.0);

    // When pasting SUM(A1:B1) into C1:
    ScAddress aCellPos(/*nColP=*/2, /*nRowP=*/0, /*nTabP=*/0);
    ScImportExport aImporter(*pDoc, aCellPos);
    SvFileStream aFile(createFileURL(u"single-cell.html"), StreamMode::READ);
    CPPUNIT_ASSERT(aImporter.ImportStream(aFile, OUString(), SotClipboardFormatId::HTML));

    // Then make sure C1 is a sum and it evaluates to 3:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: =SUM(A1:B1)
    // - Actual  :
    // i.e. data-sheets-* on <td> worked, but not on <span>.
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:B1)"),
                         pDoc->GetFormula(/*col=*/2, /*row=*/0, /*tab=*/0));
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(3), pDoc->GetValue(/*col=*/2, /*row=*/0, /*tab=*/0));
}

CPPUNIT_TEST_FIXTURE(Test, testCopyText)
{
    // Given a document with 01 in A1:
    createScDoc();
    ScDocument* pDoc = getScDoc();
    ScAddress aCellPos(/*nColP=*/0, /*nRowP=*/0, /*nTabP=*/0);
    pDoc->SetString(aCellPos, "'01");

    // When copying that text from A1:
    ScImportExport aExporter(*pDoc, aCellPos);
    SvMemoryStream aStream;
    CPPUNIT_ASSERT(aExporter.ExportStream(aStream, OUString(), SotClipboardFormatId::HTML));

    // Then make sure A1 is text:
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//td' no attribute 'data-sheets-value' exist
    // i.e. metadata was missing to avoid converting 01 to 1 (number).
    aStream.Seek(0);
    htmlDocUniquePtr pHtmlDoc = parseHtmlStream(&aStream);
    assertXPath(pHtmlDoc, "//td", "data-sheets-value", "{ \"1\": 2, \"2\": \"01\"}");
}

CPPUNIT_TEST_FIXTURE(Test, testCopyBoolean)
{
    // Given a document with boolean values in A1-A2:
    createScDoc();
    ScDocument* pDoc = getScDoc();
    ScAddress aCellPos1(/*nColP=*/0, /*nRowP=*/0, /*nTabP=*/0);
    pDoc->SetString(aCellPos1, "TRUE");
    ScAddress aCellPos2(/*nColP=*/0, /*nRowP=*/1, /*nTabP=*/0);
    pDoc->SetString(aCellPos2, "FALSE");

    // When copying those values:
    ScImportExport aExporter(*pDoc, ScRange(aCellPos1, aCellPos2));
    SvMemoryStream aStream;
    CPPUNIT_ASSERT(aExporter.ExportStream(aStream, OUString(), SotClipboardFormatId::HTML));

    // Then make sure the values are booleans:
    aStream.Seek(0);
    htmlDocUniquePtr pHtmlDoc = parseHtmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//td' no attribute 'data-sheets-value' exist
    // i.e. metadata was missing to avoid converting TRUE to text.
    assertXPath(pHtmlDoc, "(//td)[1]", "data-sheets-value", "{ \"1\": 4, \"4\": 1}");
    assertXPath(pHtmlDoc, "(//td)[2]", "data-sheets-value", "{ \"1\": 4, \"4\": 0}");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
