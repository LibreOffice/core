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
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_ScenariosEnumeration : public UnoApiTest, public apitest::XEnumeration
{
public:
    ScIndexEnumeration_ScenariosEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_ScenariosEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();
};

ScIndexEnumeration_ScenariosEnumeration::ScIndexEnumeration_ScenariosEnumeration()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_ScenariosEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet0->getCellByPosition(5, 5)->setValue(15);
    xSheet0->getCellByPosition(1, 4)->setValue(10);
    xSheet0->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference<sheet::XScenariosSupplier> xScenariosSupplier(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange0(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange1(xCellRange0->getCellRangeByName(u"A1:N4"_ustr),
                                                  uno::UNO_SET_THROW);
    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCellRange1, uno::UNO_QUERY_THROW);

    xScenariosSupplier->getScenarios()->addNewByName(u"ScScenario"_ustr,
                                                     { xCRA->getRangeAddress() }, u"Range"_ustr);
    uno::Reference<container::XEnumerationAccess> xEA(xScenariosSupplier->getScenarios(),
                                                      uno::UNO_QUERY_THROW);

    return xEA->createEnumeration();
}

void ScIndexEnumeration_ScenariosEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_ScenariosEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
