/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsheetannotation.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>

#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 5

class ScAnnontationObj : public CalcUnoApiTest, apitest::XSheetAnnotation
{
public:
    ScAnnontationObj();

    virtual void setUp();
    virtual void tearDown();

    virtual uno::Reference< uno::XInterface > init();
    virtual uno::Reference< sheet::XSheetAnnotation> getAnnotation(table::CellAddress&);

    CPPUNIT_TEST_SUITE(ScAnnontationObj);
    CPPUNIT_TEST(testGetPosition);
    CPPUNIT_TEST(testGetAuthor);
    CPPUNIT_TEST(testGetDate);
    CPPUNIT_TEST(testGetIsVisible);
    CPPUNIT_TEST(testSetIsVisible);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScAnnontationObj::nTest = 0;
uno::Reference< lang::XComponent > ScAnnontationObj::mxComponent;

ScAnnontationObj::ScAnnontationObj()
       : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< sheet::XSheetAnnotation> ScAnnontationObj::getAnnotation(table::CellAddress& xCellAddress)
{
    
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(xCellAddress.Sheet), UNO_QUERY_THROW);

    
    uno::Reference< table::XCell > xCell( xSheet->getCellByPosition(xCellAddress.Column, xCellAddress.Row), UNO_QUERY_THROW);

    
    uno::Reference< sheet::XSheetAnnotationAnchor > xAnnotationAnchor(xCell, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotation > xSheetAnnotation( xAnnotationAnchor->getAnnotation(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xSheetAnnotation.is());

    return xSheetAnnotation;
}

uno::Reference< uno::XInterface > ScAnnontationObj::init()
{

    
    OUString aFileURL;
    createFileURL(OUString("ScAnnotationObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    
    table::CellAddress xCellAddress;
    xCellAddress.Sheet = 0;
    xCellAddress.Row = 1;
    xCellAddress.Column = 2;

    return getAnnotation(xCellAddress);
}

void ScAnnontationObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScAnnontationObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnontationObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
