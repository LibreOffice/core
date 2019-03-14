/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/table/xtablecolumns.hxx>
#include <cppu/unotype.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScTableColumnsObj : public CalcUnoApiTest,
                          public apitest::XElementAccess,
                          public apitest::XEnumerationAccess,
                          public apitest::XIndexAccess,
                          public apitest::XNameAccess,
                          public apitest::XServiceInfo,
                          public apitest::XTableColumns
{
public:
    ScTableColumnsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTableColumnsObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

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

    // XTableColumns
    CPPUNIT_TEST(testInsertByIndex);
    CPPUNIT_TEST(testInsertByIndexWithNegativeIndex);
    CPPUNIT_TEST(testInsertByIndexWithNoColumn);
    CPPUNIT_TEST(testInsertByIndexWithOutOfBoundIndex);
    CPPUNIT_TEST(testRemoveByIndex);
    CPPUNIT_TEST(testRemoveByIndexWithNegativeIndex);
    CPPUNIT_TEST(testRemoveByIndexWithNoColumn);
    CPPUNIT_TEST(testRemoveByIndexWithOutOfBoundIndex);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScTableColumnsObj::ScTableColumnsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<table::XCellRange>::get())
    , XIndexAccess(1024)
    , XNameAccess("ABC")
    , XServiceInfo("ScTableColumnsObj", "com.sun.star.table.TableColumns")
{
}

uno::Reference<uno::XInterface> ScTableColumnsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XColumnRowRange> xCRR(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableColumns> xTC(xCRR->getColumns(), uno::UNO_QUERY_THROW);
    setXSpreadsheet(xSheet0);

    uno::Reference<table::XCellRange> xCR(xSheet0, uno::UNO_QUERY_THROW);
    for (auto i = 0; i < xTC->getCount() - 1 && i < 3; ++i)
    {
        uno::Reference<text::XSimpleText> xST0(xCR->getCellByPosition(i, 0), uno::UNO_QUERY_THROW);
        xST0->setString(OUString::number(i) + "a");
        uno::Reference<text::XSimpleText> xST1(xCR->getCellByPosition(i, 1), uno::UNO_QUERY_THROW);
        xST1->setString(OUString::number(i) + "b");
    }
    for (auto i = 3; i < xTC->getCount() - 1 && i < 10; ++i)
    {
        uno::Reference<text::XSimpleText> xST0(xCR->getCellByPosition(i, 0), uno::UNO_QUERY_THROW);
        xST0->setString("");
        uno::Reference<text::XSimpleText> xST1(xCR->getCellByPosition(i, 1), uno::UNO_QUERY_THROW);
        xST1->setString("");
    }
    return xTC;
}

void ScTableColumnsObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScTableColumnsObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableColumnsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
