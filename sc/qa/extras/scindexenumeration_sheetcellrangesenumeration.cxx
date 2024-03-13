/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_SheetCellRangesEnumeration : public UnoApiTest,
                                                      public apitest::XEnumeration
{
public:
    ScIndexEnumeration_SheetCellRangesEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_SheetCellRangesEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();
};

ScIndexEnumeration_SheetCellRangesEnumeration::ScIndexEnumeration_SheetCellRangesEnumeration()
    : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_SheetCellRangesEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xSCR(
        xMSF->createInstance("com.sun.star.sheet.SheetCellRanges"), uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameContainer> xNC(xSCR, uno::UNO_QUERY_THROW);

    uno::Any aRange;

    aRange <<= xSheet0->getCellRangeByName("C1:D4");
    xNC->insertByName("Range1", aRange);
    aRange <<= xSheet0->getCellRangeByName("E2:F5");
    xNC->insertByName("Range2", aRange);
    aRange <<= xSheet0->getCellRangeByName("G2:H3");
    xNC->insertByName("Range3", aRange);
    aRange <<= xSheet0->getCellRangeByName("I7:J8");
    xNC->insertByName("Range4", aRange);

    for (auto x = 0; x < 10; ++x)
    {
        for (auto y = 0; y < 5; ++y)
        {
            xSheet0->getCellByPosition(x, y)->setFormula("a");
        }
    }
    for (auto x = 0; x < 10; ++x)
    {
        for (auto y = 5; y < 10; ++y)
        {
            xSheet0->getCellByPosition(x, y)->setValue(x + y);
        }
    }
    uno::Reference<container::XEnumerationAccess> xEA(xSCR, uno::UNO_QUERY_THROW);

    return xEA->createEnumeration();
}

void ScIndexEnumeration_SheetCellRangesEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_SheetCellRangesEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
