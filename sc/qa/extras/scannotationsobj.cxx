/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsheetannotations.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 4

class ScAnnontationsObj : public CalcUnoApiTest, apitest::XSheetAnnotations
{
public:
    ScAnnontationsObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< sheet::XSheetAnnotations > getAnnotations(long nIndex) override;

    CPPUNIT_TEST_SUITE(ScAnnontationsObj);
    CPPUNIT_TEST(testInsertNew);
    CPPUNIT_TEST(testRemoveByIndex);
    CPPUNIT_TEST(testCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScAnnontationsObj::nTest = 0;
uno::Reference< lang::XComponent > ScAnnontationsObj::mxComponent;

ScAnnontationsObj::ScAnnontationsObj()
       : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< sheet::XSheetAnnotations> ScAnnontationsObj::getAnnotations(long nIndex)
{
    // get the sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(nIndex), UNO_QUERY_THROW);

    // get the annotations collection
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnotationSupplier(xSheet, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotations > xSheetAnnotations( xAnnotationSupplier->getAnnotations(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xSheetAnnotations.is());

    return xSheetAnnotations;
}

uno::Reference< uno::XInterface > ScAnnontationsObj::init()
{
    // get the test file
    OUString aFileURL;
    createFileURL(OUString("ScAnnotationObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    return getAnnotations(0);
}
void ScAnnontationsObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScAnnontationsObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnontationsObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
