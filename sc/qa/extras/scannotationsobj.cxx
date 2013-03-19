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
#include <test/sheet/xsheetannotations.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 2

class ScAnnontationsObj : public UnoApiTest, apitest::XSheetAnnotations
{
public:
    ScAnnontationsObj();

    virtual void setUp();
    virtual void tearDown();

    virtual uno::Reference< uno::XInterface > init();
    virtual uno::Reference< sheet::XSheetAnnotations> getAnnotations();

    CPPUNIT_TEST_SUITE(ScAnnontationsObj);
    CPPUNIT_TEST(testInsertNew);
    CPPUNIT_TEST(testRemoveByIndex);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScAnnontationsObj::nTest = 0;
uno::Reference< lang::XComponent > ScAnnontationsObj::mxComponent;

ScAnnontationsObj::ScAnnontationsObj()
       : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< sheet::XSheetAnnotations> ScAnnontationsObj::getAnnotations()
{
    // get the sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    // get the annotations collection
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnotationSupplier(xSheet, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotations > xSheetAnnotations( xAnnotationSupplier->getAnnotations(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xSheetAnnotations.is());

    return xSheetAnnotations;
}

uno::Reference< uno::XInterface > ScAnnontationsObj::init()
{
    // get the test file
    rtl::OUString aFileURL;
    createFileURL(rtl::OUString("ScAnnotationObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    return getAnnotations();
}

void ScAnnontationsObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScAnnontationsObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnontationsObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
