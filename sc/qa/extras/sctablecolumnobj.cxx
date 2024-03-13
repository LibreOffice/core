/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/container/xnamed.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/table/tablecolumn.hxx>
#include <test/table/xcellrange.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScTableColumnObj : public UnoApiTest,
                         public apitest::TableColumn,
                         public apitest::XCellRange,
                         public apitest::XNamed,
                         public apitest::XPropertySet,
                         public apitest::XServiceInfo
{
public:
    ScTableColumnObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScTableColumnObj);

    // TableColumn
    CPPUNIT_TEST(testTableColumnProperties);

    // XCellRange
    CPPUNIT_TEST(testGetCellByPosition);
    CPPUNIT_TEST(testGetCellRangeByName);
    CPPUNIT_TEST(testGetCellRangeByPosition);

    // XNamed
    CPPUNIT_TEST(testGetName);
    // because TableColumnNames are fixed, test for an exception
    CPPUNIT_TEST(testSetNameThrowsException);

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
};

ScTableColumnObj::ScTableColumnObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XCellRange("K1:K1")
    , XNamed("K")
    , XPropertySet({
          "BottomBorder",
          "BottomBorder2",
          "BottomBorderComplexColor",
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
          "LeftBorderComplexColor",
          "NumberingRules",
          "Orientation",
          "RightBorder",
          "RightBorder2",
          "RightBorderComplexColor",
          "ShadowFormat",
          "TableBorder",
          "TableBorder2",
          "TopBorder",
          "TopBorder2",
          "TopBorderComplexColor",
          "UserDefinedAttributes",
          "Validation",
          "ValidationLocal",
          "ValidationXML",
          "WritingMode",
      })
    , XServiceInfo("ScTableColumnObj", "com.sun.star.table.TableColumn")
{
}

uno::Reference<uno::XInterface> ScTableColumnObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    setSpreadsheet(xSheet0);
    uno::Reference<table::XColumnRowRange> xCRR(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableColumns> xTC(xCRR->getColumns(), uno::UNO_SET_THROW);

    uno::Reference<container::XIndexAccess> xIA_TC(xTC, uno::UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xReturn(xIA_TC->getByIndex(10), uno::UNO_QUERY_THROW);
    return xReturn;
}

void ScTableColumnObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableColumnObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
