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
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XCellRange(u"K1:K1"_ustr)
    , XNamed(u"K"_ustr)
    , XPropertySet({
          u"BottomBorder"_ustr,
          u"BottomBorder2"_ustr,
          u"BottomBorderComplexColor"_ustr,
          u"CellProtection"_ustr,
          u"CharLocale"_ustr,
          u"CharLocaleAsian"_ustr,
          u"CharLocaleComplex"_ustr,
          u"CharPosture"_ustr,
          u"CharPostureAsian"_ustr,
          u"CharPostureComplex"_ustr,
          u"ConditionalFormat"_ustr,
          u"ConditionalFormatLocal"_ustr,
          u"ConditionalFormatXML"_ustr,
          u"DiagonalBLTR"_ustr,
          u"DiagonalBLTR2"_ustr,
          u"DiagonalTLBR"_ustr,
          u"DiagonalTLBR2"_ustr,
          u"HoriJustify"_ustr,
          u"LeftBorder"_ustr,
          u"LeftBorder2"_ustr,
          u"LeftBorderComplexColor"_ustr,
          u"NumberingRules"_ustr,
          u"Orientation"_ustr,
          u"RightBorder"_ustr,
          u"RightBorder2"_ustr,
          u"RightBorderComplexColor"_ustr,
          u"ShadowFormat"_ustr,
          u"TableBorder"_ustr,
          u"TableBorder2"_ustr,
          u"TopBorder"_ustr,
          u"TopBorder2"_ustr,
          u"TopBorderComplexColor"_ustr,
          u"UserDefinedAttributes"_ustr,
          u"Validation"_ustr,
          u"ValidationLocal"_ustr,
          u"ValidationXML"_ustr,
          u"WritingMode"_ustr,
      })
    , XServiceInfo(u"ScTableColumnObj"_ustr, u"com.sun.star.table.TableColumn"_ustr)
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
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableColumnObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
