/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/table/xtablecharts.hxx>
#include <cppu/unotype.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace css;

namespace sc_apitest
{
class ScChartsObj : public UnoApiTest,
                    public apitest::XEnumerationAccess,
                    public apitest::XElementAccess,
                    public apitest::XIndexAccess,
                    public apitest::XNameAccess,
                    public apitest::XServiceInfo,
                    public apitest::XTableCharts
{
public:
    ScChartsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScChartsObj);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    // XTableCharts
    CPPUNIT_TEST(testAddNewRemoveByName);

    CPPUNIT_TEST_SUITE_END();
};

ScChartsObj::ScChartsObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XElementAccess(cppu::UnoType<table::XTableChart>::get())
    , XIndexAccess(1)
    , XNameAccess(u"ScChartsObj"_ustr)
    , XServiceInfo(u"ScChartsObj"_ustr, u"com.sun.star.table.TableCharts"_ustr)
{
}

uno::Reference<uno::XInterface> ScChartsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet0->getCellByPosition(1, 0)->setFormula(u"JAN"_ustr);
    xSheet0->getCellByPosition(2, 0)->setFormula(u"FEB"_ustr);
    xSheet0->getCellByPosition(3, 0)->setFormula(u"MAR"_ustr);
    xSheet0->getCellByPosition(4, 0)->setFormula(u"APR"_ustr);
    xSheet0->getCellByPosition(5, 0)->setFormula(u"MAY"_ustr);
    xSheet0->getCellByPosition(6, 0)->setFormula(u"JUN"_ustr);
    xSheet0->getCellByPosition(7, 0)->setFormula(u"JUL"_ustr);
    xSheet0->getCellByPosition(8, 0)->setFormula(u"AUG"_ustr);
    xSheet0->getCellByPosition(9, 0)->setFormula(u"SEP"_ustr);
    xSheet0->getCellByPosition(10, 0)->setFormula(u"OCT"_ustr);
    xSheet0->getCellByPosition(11, 0)->setFormula(u"NOV"_ustr);
    xSheet0->getCellByPosition(12, 0)->setFormula(u"DEC"_ustr);
    xSheet0->getCellByPosition(13, 0)->setFormula(u"SUM"_ustr);

    xSheet0->getCellByPosition(0, 1)->setFormula(u"Smith"_ustr);
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
    xSheet0->getCellByPosition(13, 1)->setFormula(u"SUM(B2:M2"_ustr);

    xSheet0->getCellByPosition(0, 2)->setFormula(u"Jones"_ustr);
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
    xSheet0->getCellByPosition(13, 2)->setFormula(u"SUM(B3:M3"_ustr);

    xSheet0->getCellByPosition(0, 3)->setFormula(u"Brown"_ustr);
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
    xSheet0->getCellByPosition(13, 3)->setFormula(u"SUM(A4:L4"_ustr);

    uno::Reference<table::XCellRange> xCellRange0(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange1(xCellRange0->getCellRangeByName(u"A1:N4"_ustr),
                                                  uno::UNO_SET_THROW);
    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCellRange1, uno::UNO_QUERY_THROW);

    uno::Reference<table::XTableChartsSupplier> xTCS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableCharts> xTC = xTCS->getCharts();
    xTC->addNewByName(u"ScChartsObj"_ustr, awt::Rectangle(500, 3000, 25000, 11000),
                      { xCRA->getRangeAddress() }, true, true);

    return xTC;
}

void ScChartsObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChartsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
