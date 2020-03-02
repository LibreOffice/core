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
#include <test/container/xnamecontainer.hxx>
#include <test/container/xnamereplace.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xspreadsheets.hxx>
#include <test/sheet/xspreadsheets2.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScTableSheetsObj : public CalcUnoApiTest,
                         public ::apitest::XElementAccess,
                         public ::apitest::XEnumerationAccess,
                         public ::apitest::XIndexAccess,
                         public ::apitest::XNameAccess,
                         public ::apitest::XNameContainer,
                         public ::apitest::XNameReplace,
                         public ::apitest::XServiceInfo,
                         public ::apitest::XSpreadsheets,
                         public ::apitest::XSpreadsheets2
{
public:
    ScTableSheetsObj();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(ScTableSheetsObj);

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

    // XNameContainer
    CPPUNIT_TEST(testInsertByName);
    CPPUNIT_TEST(testInsertByNameEmptyName);
    CPPUNIT_TEST(testInsertByNameDuplicate);
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST(testRemoveByNameNoneExistingElement);

    // XNameReplace
    CPPUNIT_TEST(testReplaceByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

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
    CPPUNIT_TEST(testLastAfterInsertCopy);

    CPPUNIT_TEST_SUITE_END();

    virtual uno::Reference<lang::XComponent> getComponent() override;
    virtual void createFileURL(const OUString& rFileBase, OUString& rFileURL) override;
    virtual uno::Reference<lang::XComponent> loadFromDesktop(const OUString& rString) override;

protected:
    uno::Reference<lang::XComponent> mxComponent;
};

ScTableSheetsObj::ScTableSheetsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , ::apitest::XElementAccess(cppu::UnoType<sheet::XSpreadsheet>::get())
    , ::apitest::XIndexAccess(3)
    , ::apitest::XNameAccess("Sheet1")
    , ::apitest::XNameContainer("Sheet2")
    , ::apitest::XNameReplace("Sheet2")
    , ::apitest::XServiceInfo("ScTableSheetsObj", "com.sun.star.sheet.Spreadsheets")
{
}

uno::Reference<lang::XComponent> ScTableSheetsObj::getComponent() { return mxComponent; }

void ScTableSheetsObj::createFileURL(const OUString& rFileBase, OUString& rFileURL)
{
    CalcUnoApiTest::createFileURL(rFileBase, rFileURL);
}

uno::Reference<lang::XComponent> ScTableSheetsObj::loadFromDesktop(const OUString& rString)
{
    return CalcUnoApiTest::loadFromDesktop(rString);
}

uno::Reference<uno::XInterface> ScTableSheetsObj::init()
{
    xDocument.set(mxComponent, UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xReturn(xDocument->getSheets(), UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
    XNameContainer::setElement(
        uno::makeAny(xMSF->createInstance("com.sun.star.sheet.Spreadsheet")));
    // XNameReplace
    XNameReplace::setElement(uno::makeAny(xMSF->createInstance("com.sun.star.sheet.Spreadsheet")));

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
} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
