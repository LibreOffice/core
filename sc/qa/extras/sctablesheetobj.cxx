/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/util/xreplaceable.hxx>
#include <test/util/xsearchable.hxx>
#include <test/sheet/xprintareas.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{

class ScTableSheetObj : public CalcUnoApiTest, apitest::XSearchable, apitest::XReplaceable, apitest::XPrintAreas
{
public:
    ScTableSheetObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    CPPUNIT_TEST_SUITE(ScTableSheetObj);
    CPPUNIT_TEST(testFindAll);
    CPPUNIT_TEST(testFindNext);
    CPPUNIT_TEST(testFindFirst);
    CPPUNIT_TEST(testReplaceAll);
    CPPUNIT_TEST(testCreateReplaceDescriptor);
    // XPrintAreas
    CPPUNIT_TEST(testSetAndGetPrintTitleColumns);
    CPPUNIT_TEST(testSetAndGetPrintTitleRows);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScTableSheetObj::ScTableSheetObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments"),
    apitest::XSearchable(OUString("test"), 4),
    apitest::XReplaceable(OUString("searchReplaceString"), OUString("replaceReplaceString"))
{
}

uno::Reference< uno::XInterface > ScTableSheetObj::init()
{
    OUString aFileURL;
    createFileURL(OUString("ScTableSheetObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

void ScTableSheetObj::setUp()
{
    CalcUnoApiTest::setUp();
}

void ScTableSheetObj::tearDown()
{
    closeDocument(mxComponent);
    mxComponent.clear();

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableSheetObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
