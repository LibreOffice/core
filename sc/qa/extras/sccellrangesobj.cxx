/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsheetcellrangecontainer.hxx>
#include <test/sheet/xsheetcellranges.hxx>
#include <test/sheet/xsheetoperation.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 7

class ScCellRangesObj : public CalcUnoApiTest,
                        public apitest::XSheetCellRangeContainer,
                        public apitest::XSheetCellRanges,
                        public apitest::XSheetOperation
{
public:
    ScCellRangesObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    CPPUNIT_TEST_SUITE(ScCellRangesObj);

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

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScCellRangesObj::nTest = 0;
uno::Reference< lang::XComponent > ScCellRangesObj::mxComponent;

ScCellRangesObj::ScCellRangesObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScCellRangesObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< container::XIndexAccess > xIndexAccess(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference< lang::XMultiServiceFactory > xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameContainer > xRanges(xMSF->createInstance("com.sun.star.sheet.SheetCellRanges"), uno::UNO_QUERY_THROW);

    uno::Any xCellRange;
    xCellRange <<= xSheet->getCellRangeByName("C1:D4");
    xRanges->insertByName("Range1", xCellRange);
    xCellRange <<= xSheet->getCellRangeByName("E2:F5");
    xRanges->insertByName("Range2", xCellRange);
    xCellRange <<= xSheet->getCellRangeByName("G2:H3");
    xRanges->insertByName("Range3", xCellRange);
    xCellRange <<= xSheet->getCellRangeByName("I7:J8");
    xRanges->insertByName("Range4", xCellRange);

    for ( int i = 0; i < 10; i++ )
    {
        for ( int j = 5; j < 10; j++ )
        {
            xSheet->getCellByPosition(i, j)->setValue(i + j);
        }
    }

    return xRanges;
}

void ScCellRangesObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScCellRangesObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellRangesObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
