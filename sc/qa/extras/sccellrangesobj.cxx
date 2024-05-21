/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/sheet/sheetcellranges.hxx>
#include <test/sheet/xformulaquery.hxx>
#include <test/sheet/xsheetcellrangecontainer.hxx>
#include <test/sheet/xsheetcellranges.hxx>
#include <test/sheet/xsheetoperation.hxx>
#include <test/util/xindent.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScCellRangesObj : public UnoApiTest,
                        public apitest::SheetCellRanges,
                        public apitest::XEnumerationAccess,
                        public apitest::XFormulaQuery,
                        public apitest::XIndent,
                        public apitest::XSheetCellRangeContainer,
                        public apitest::XSheetCellRanges,
                        public apitest::XSheetOperation
{
public:
    ScCellRangesObj();

    virtual void setUp() override;

    virtual uno::Reference<uno::XInterface> getXSpreadsheet() override;
    virtual uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(ScCellRangesObj);

    // SheetCellRanges
    CPPUNIT_TEST(testSheetCellRangesProperties);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XFormulaQuery
    CPPUNIT_TEST(testQueryDependents);
    CPPUNIT_TEST(testQueryPrecedents);

    // XIndent
    CPPUNIT_TEST(testIncrementIndent);
    CPPUNIT_TEST(testDecrementIndent);

    // XSheetCellRangeContainer
    CPPUNIT_TEST(testAddRemoveRangeAddress);
    CPPUNIT_TEST(testAddRemoveRangeAddresses);

    // XSheetCellRanges
    CPPUNIT_TEST(testGetCells);
    CPPUNIT_TEST(testGetRangeAddresses);
    CPPUNIT_TEST(testGetRangeAddressesAsString);

    // XSheetOperation
    CPPUNIT_TEST(testComputeFunction);
    CPPUNIT_TEST(testClearContents);

    CPPUNIT_TEST_SUITE_END();
};

ScCellRangesObj::ScCellRangesObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , apitest::XFormulaQuery(table::CellRangeAddress(0, 4, 1, 5, 4),
                             table::CellRangeAddress(0, 4, 1, 5, 4))
{
}

uno::Reference<uno::XInterface> ScCellRangesObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndexAccess(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameContainer> xRanges(
        xMSF->createInstance(u"com.sun.star.sheet.SheetCellRanges"_ustr), uno::UNO_QUERY_THROW);

    uno::Any xCellRange;
    xCellRange <<= xSheet->getCellRangeByName(u"C1:D4"_ustr);
    xRanges->insertByName(u"Range1"_ustr, xCellRange);
    xCellRange <<= xSheet->getCellRangeByName(u"E2:F5"_ustr);
    xRanges->insertByName(u"Range2"_ustr, xCellRange);
    xCellRange <<= xSheet->getCellRangeByName(u"G2:H3"_ustr);
    xRanges->insertByName(u"Range3"_ustr, xCellRange);
    xCellRange <<= xSheet->getCellRangeByName(u"I7:J8"_ustr);
    xRanges->insertByName(u"Range4"_ustr, xCellRange);

    for (int i = 0; i < 10; i++)
    {
        for (int j = 5; j < 10; j++)
        {
            xSheet->getCellByPosition(i, j)->setValue(i + j);
        }
    }

    return xRanges;
}

uno::Reference<uno::XInterface> ScCellRangesObj::getXSpreadsheet()
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xSheetDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    setXCell(xSheet->getCellByPosition(15, 15));

    return xSheet;
}

void ScCellRangesObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellRangesObj);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
