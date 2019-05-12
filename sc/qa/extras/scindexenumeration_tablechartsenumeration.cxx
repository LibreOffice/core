/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_TableChartsEnumeration : public CalcUnoApiTest,
                                                  public apitest::XEnumeration
{
public:
    ScIndexEnumeration_TableChartsEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_TableChartsEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScIndexEnumeration_TableChartsEnumeration::ScIndexEnumeration_TableChartsEnumeration()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_TableChartsEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet0->getCellByPosition(1, 0)->setFormula("JAN");
    xSheet0->getCellByPosition(2, 0)->setFormula("FEB");
    xSheet0->getCellByPosition(3, 0)->setFormula("MAR");
    xSheet0->getCellByPosition(4, 0)->setFormula("APR");
    xSheet0->getCellByPosition(5, 0)->setFormula("MAY");
    xSheet0->getCellByPosition(6, 0)->setFormula("JUN");
    xSheet0->getCellByPosition(7, 0)->setFormula("JUL");
    xSheet0->getCellByPosition(8, 0)->setFormula("AUG");
    xSheet0->getCellByPosition(9, 0)->setFormula("SEP");
    xSheet0->getCellByPosition(10, 0)->setFormula("OCT");
    xSheet0->getCellByPosition(11, 0)->setFormula("NOV");
    xSheet0->getCellByPosition(12, 0)->setFormula("DEC");
    xSheet0->getCellByPosition(13, 0)->setFormula("SUM");

    xSheet0->getCellByPosition(0, 1)->setFormula("Smith");
    xSheet0->getCellByPosition(1, 1)->setValue(42);
    xSheet0->getCellByPosition(2, 1)->setValue(58.9);
    xSheet0->getCellByPosition(3, 1)->setValue(-66.5);
    xSheet0->getCellByPosition(4, 1)->setValue(43.4);
    xSheet0->getCellByPosition(5, 1)->setValue(44.5);
    xSheet0->getCellByPosition(6, 1)->setValue(45.3);
    xSheet0->getCellByPosition(7, 1)->setValue(-67.3);
    xSheet0->getCellByPosition(8, 1)->setValue(30.5);
    xSheet0->getCellByPosition(9, 1)->setValue(23.2);
    xSheet0->getCellByPosition(10, 1)->setValue(-97.3);
    xSheet0->getCellByPosition(11, 1)->setValue(22.4);
    xSheet0->getCellByPosition(11, 1)->setValue(23.5);
    xSheet0->getCellByPosition(13, 1)->setFormula("SUM(B2:M2");

    xSheet0->getCellByPosition(0, 2)->setFormula("Jones");
    xSheet0->getCellByPosition(1, 2)->setValue(21);
    xSheet0->getCellByPosition(2, 2)->setValue(40.9);
    xSheet0->getCellByPosition(3, 2)->setValue(-57.5);
    xSheet0->getCellByPosition(4, 2)->setValue(-23.4);
    xSheet0->getCellByPosition(5, 2)->setValue(34.5);
    xSheet0->getCellByPosition(6, 2)->setValue(59.3);
    xSheet0->getCellByPosition(7, 2)->setValue(27.3);
    xSheet0->getCellByPosition(8, 2)->setValue(-38.5);
    xSheet0->getCellByPosition(9, 2)->setValue(43.2);
    xSheet0->getCellByPosition(10, 2)->setValue(57.3);
    xSheet0->getCellByPosition(11, 2)->setValue(25.4);
    xSheet0->getCellByPosition(11, 2)->setValue(28.5);
    xSheet0->getCellByPosition(13, 2)->setFormula("SUM(B3:M3");

    xSheet0->getCellByPosition(0, 3)->setFormula("Brown");
    xSheet0->getCellByPosition(1, 3)->setValue(31.45);
    xSheet0->getCellByPosition(2, 3)->setValue(-20.9);
    xSheet0->getCellByPosition(3, 3)->setValue(-117.5);
    xSheet0->getCellByPosition(4, 3)->setValue(23.4);
    xSheet0->getCellByPosition(5, 3)->setValue(-114.5);
    xSheet0->getCellByPosition(6, 3)->setValue(115.3);
    xSheet0->getCellByPosition(7, 3)->setValue(-171.3);
    xSheet0->getCellByPosition(8, 3)->setValue(89.5);
    xSheet0->getCellByPosition(9, 3)->setValue(41.2);
    xSheet0->getCellByPosition(10, 3)->setValue(71.3);
    xSheet0->getCellByPosition(11, 3)->setValue(25.4);
    xSheet0->getCellByPosition(11, 3)->setValue(38.5);
    xSheet0->getCellByPosition(13, 3)->setFormula("SUM(A4:L4");

    uno::Reference<table::XCellRange> xCellRange0(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange1(xCellRange0->getCellRangeByName("A1:N4"),
                                                  uno::UNO_SET_THROW);
    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCellRange1, uno::UNO_QUERY_THROW);

    uno::Sequence<table::CellRangeAddress> aCRA(1);
    aCRA[0] = xCRA->getRangeAddress();

    uno::Reference<table::XTableChartsSupplier> xTCS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableCharts> xTC = xTCS->getCharts();
    xTC->addNewByName("ScChartObj", awt::Rectangle(500, 3000, 25000, 11000), aCRA, true, false);

    uno::Reference<container::XEnumerationAccess> xEA(xTC, uno::UNO_QUERY_THROW);

    return xEA->createEnumeration();
}

void ScIndexEnumeration_TableChartsEnumeration::setUp()
{
    CalcUnoApiTest::setUp();
    m_xComponent = loadFromDesktop("private:factory/scalc");
    CPPUNIT_ASSERT_MESSAGE("no component", m_xComponent.is());
}

void ScIndexEnumeration_TableChartsEnumeration::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_TableChartsEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
