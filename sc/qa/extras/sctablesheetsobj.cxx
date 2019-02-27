/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xnamecontainer.hxx>
#include <test/sheet/xspreadsheets.hxx>
#include <test/sheet/xspreadsheets2.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScTableSheetsObj : public CalcUnoApiTest, public apitest::XEnumerationAccess,
                                                public ::apitest::XSpreadsheets,
                                                public ::apitest::XSpreadsheets2,
                                                public apitest::XNameContainer
{
public:
    ScTableSheetsObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTableSheetsObj);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XSpreadsheets
    CPPUNIT_TEST(testInsertNewByName);
    CPPUNIT_TEST(testInsertNewByNameBadName);
    CPPUNIT_TEST(testCopyByName);
    CPPUNIT_TEST(testMoveByName);

    // XSpreadsheets2
    CPPUNIT_TEST(testImportedSheetNameAndIndex);
    CPPUNIT_TEST(testImportString);
    CPPUNIT_TEST(testImportValue);
    CPPUNIT_TEST(testImportFormulaBasicMath);
    CPPUNIT_TEST(testImportFormulaWithNamedRange);
    CPPUNIT_TEST(testImportOverExistingNamedRange);
    CPPUNIT_TEST(testImportNamedRangeDefinedInSource);
    CPPUNIT_TEST(testImportNamedRangeRedefinedInSource);
    CPPUNIT_TEST(testImportNewNamedRange);
    CPPUNIT_TEST(testImportCellStyle);

    // XNameContainer
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST(testRemoveByNameNoneExistingElement);

    CPPUNIT_TEST_SUITE_END();

    virtual uno::Reference< lang::XComponent > getComponent() override;
    virtual void createFileURL(const OUString& rFileBase, OUString& rFileURL) override;
    virtual uno::Reference< lang::XComponent > loadFromDesktop(const OUString& rString) override;
    virtual uno::Reference< uno::XInterface > init() override;
protected:
    uno::Reference< lang::XComponent > mxComponent;
};

ScTableSheetsObj::ScTableSheetsObj():
            CalcUnoApiTest("/sc/qa/extras/testdocuments"),
            apitest::XNameContainer("Sheet2")
{
}

uno::Reference< lang::XComponent > ScTableSheetsObj::getComponent()
{
    return mxComponent;
}

void ScTableSheetsObj::createFileURL(const OUString& rFileBase, OUString& rFileURL)
{
    CalcUnoApiTest::createFileURL(rFileBase, rFileURL);
}

uno::Reference< lang::XComponent > ScTableSheetsObj::loadFromDesktop(const OUString& rString)
{
    return CalcUnoApiTest::loadFromDesktop(rString);
}

uno::Reference< uno::XInterface > ScTableSheetsObj::init()
{
    xDocument.set(mxComponent, UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xReturn( xDocument->getSheets(), UNO_QUERY_THROW);

    return xReturn;
}

void ScTableSheetsObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    OUString aFileURL;
    createFileURL("rangenamessrc.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);
}

void ScTableSheetsObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableSheetsObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
