/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xspreadsheets2.hxx>
#include <test/container/xnamecontainer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 13

class ScTableSheetsObj : public UnoApiTest, public ::apitest::XSpreadsheets2, apitest::XNameContainer
{
public:
    ScTableSheetsObj();

    virtual void setUp();
    virtual void tearDown();

    CPPUNIT_TEST_SUITE(ScTableSheetsObj);
    CPPUNIT_TEST(testImportValue);
    CPPUNIT_TEST(testImportString);
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
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST_SUITE_END();

    virtual uno::Reference< lang::XComponent > getComponent();
    virtual void createFileURL(const OUString& rFileBase, OUString& rFileURL);
    virtual uno::Reference< lang::XComponent > loadFromDesktop(const OUString& rString);
    virtual uno::Reference< uno::XInterface > init();
protected:
    static uno::Reference< lang::XComponent > mxComponent;
    static sal_Int32 nTest;
};

uno::Reference< lang::XComponent > ScTableSheetsObj::mxComponent;
sal_Int32 ScTableSheetsObj::nTest = 0;

ScTableSheetsObj::ScTableSheetsObj():
            UnoApiTest("/sc/qa/extras/testdocuments"),
            apitest::XNameContainer(OUString("Sheet2"))
{

}

uno::Reference< lang::XComponent > ScTableSheetsObj::getComponent()
{
    return mxComponent;
}

void ScTableSheetsObj::createFileURL(const OUString& rFileBase, OUString& rFileURL)
{
    UnoApiTest::createFileURL(rFileBase, rFileURL);
}

uno::Reference< lang::XComponent > ScTableSheetsObj::loadFromDesktop(const OUString& rString)
{
    return UnoApiTest::loadFromDesktop(rString);
}

uno::Reference< uno::XInterface > ScTableSheetsObj::init()
{
    OUString aFileURL;
    createFileURL(OUString("rangenamessrc.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(mxComponent.is());
    xDocument = uno::Reference< sheet::XSpreadsheetDocument >(mxComponent, UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xReturn( xDocument->getSheets(), UNO_QUERY_THROW);

    return xReturn;
}

void ScTableSheetsObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    UnoApiTest::setUp();
}

void ScTableSheetsObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableSheetsObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
