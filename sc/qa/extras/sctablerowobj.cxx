/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/table/tablerow.hxx>
#include <test/table/xcellrange.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScTableRowObj : public CalcUnoApiTest,
                      public apitest::TableRow,
                      public apitest::XCellRange,
                      public apitest::XPropertySet,
                      public apitest::XServiceInfo
{
public:
    ScTableRowObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTableRowObj);

    // TableRow
    CPPUNIT_TEST(testTableRowProperties);

    // XCellRange
    CPPUNIT_TEST(testGetCellByPosition);
    CPPUNIT_TEST(testGetCellRangeByName);
    CPPUNIT_TEST(testGetCellRangeByPosition);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScTableRowObj::ScTableRowObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XCellRange("A7:A7")
    , XPropertySet({ "BottomBorder",
                     "BottomBorder2",
                     "CellProtection",
                     "CharLocale",
                     "CharLocaleAsian",
                     "CharLocaleComplex",
                     "CharPosture",
                     "CharPostureAsian",
                     "CharPostureComplex",
                     "ConditionalFormat",
                     "ConditionalFormatLocal",
                     "ConditionalFormatXML",
                     "DiagonalBLTR",
                     "DiagonalBLTR2",
                     "DiagonalTLBR",
                     "DiagonalTLBR2",
                     "HoriJustify",
                     "LeftBorder",
                     "LeftBorder2",
                     "NumberingRules",
                     "Orientation",
                     "RightBorder",
                     "RightBorder2",
                     "ShadowFormat",
                     "TableBorder",
                     "TableBorder2",
                     "TopBorder",
                     "TopBorder2",
                     "UserDefinedAttributes",
                     "Validation",
                     "ValidationLocal",
                     "ValidationXML",
                     "WritingMode" })
    , XServiceInfo("ScTableRowObj", "com.sun.star.table.TableRow")
{
}

uno::Reference<uno::XInterface> ScTableRowObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<table::XColumnRowRange> xCRR(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableRows> xTR(xCRR->getRows(), uno::UNO_SET_THROW);

    uno::Reference<container::XIndexAccess> xIA_TR(xTR, uno::UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xReturn(xIA_TR->getByIndex(6), uno::UNO_QUERY_THROW);
    return xReturn;
}

void ScTableRowObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScTableRowObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableRowObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
