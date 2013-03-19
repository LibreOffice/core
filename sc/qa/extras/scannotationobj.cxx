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
 * Copyright (C) 2013 Laurent Godard <lgodard.libre@laposte.net> (initial developer)
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
#include <test/sheet/xsheetannotation.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>

#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 5

class ScAnnontationObj : public UnoApiTest, apitest::XSheetAnnotation
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
       : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< sheet::XSheetAnnotation> ScAnnontationObj::getAnnotation(table::CellAddress& xCellAddress)
{
    // get the sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(xCellAddress.Sheet), UNO_QUERY_THROW);

    // get the cell
    uno::Reference< table::XCell > xCell( xSheet->getCellByPosition(xCellAddress.Column, xCellAddress.Row), UNO_QUERY_THROW);

    // get the annotation from cell
    uno::Reference< sheet::XSheetAnnotationAnchor > xAnnotationAnchor(xCell, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotation > xSheetAnnotation( xAnnotationAnchor->getAnnotation(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xSheetAnnotation.is());

    return xSheetAnnotation;
}

uno::Reference< uno::XInterface > ScAnnontationObj::init()
{

    // get the test file
    rtl::OUString aFileURL;
    createFileURL(rtl::OUString("ScAnnotationObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    // tested annotation is in sheet 0 cell C2
    table::CellAddress xCellAddress;
    xCellAddress.Sheet = 0;
    xCellAddress.Row = 1;
    xCellAddress.Column = 2;

    return getAnnotation(xCellAddress);
}

void ScAnnontationObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScAnnontationObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnontationObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
