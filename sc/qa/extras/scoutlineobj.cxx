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
#include <test/sheet/xsheetoutline.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 6

class ScOutlineObj : public UnoApiTest, apitest::XSheetOutline
{
public:
    ScOutlineObj();

    virtual void setUp();
    virtual void tearDown();

    virtual uno::Reference< uno::XInterface > init();

    CPPUNIT_TEST_SUITE(ScOutlineObj);
    CPPUNIT_TEST(testHideDetail);
    CPPUNIT_TEST(testShowDetail);
    CPPUNIT_TEST(testShowLevel);
    CPPUNIT_TEST(testUngroup);
    CPPUNIT_TEST(testGroup);
  //  CPPUNIT_TEST(testAutoOutline);
    CPPUNIT_TEST(testClearOutline);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScOutlineObj::nTest = 0;
uno::Reference< lang::XComponent > ScOutlineObj::mxComponent;

ScOutlineObj::ScOutlineObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScOutlineObj::init()
{
    // get the test file
    OUString aFileURL;
    createFileURL(OUString("ScOutlineObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    // get the first sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

void ScOutlineObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScOutlineObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOutlineObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
