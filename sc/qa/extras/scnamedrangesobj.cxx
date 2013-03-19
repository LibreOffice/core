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
#include <test/sheet/xnamedranges.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 3

class ScNamedRangesObj : public UnoApiTest, apitest::XNamedRanges
{
public:
    virtual void setUp();
    virtual void tearDown();

    virtual uno::Reference< uno::XInterface > init(sal_Int32 nSheet);

    ScNamedRangesObj();

    CPPUNIT_TEST_SUITE(ScNamedRangesObj);
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testAddNewFromTitles);
    //CPPUNIT_TEST_EXCEPTION(testRemoveByName, uno::RuntimeException);
    CPPUNIT_TEST(testOutputList);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScNamedRangesObj::nTest = 0;
uno::Reference< lang::XComponent > ScNamedRangesObj::mxComponent;

ScNamedRangesObj::ScNamedRangesObj()
     : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScNamedRangesObj::init(sal_Int32 nSheet)
{
    rtl::OUString aFileURL;
    createFileURL(rtl::OUString("ScNamedRangeObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< beans::XPropertySet > xPropSet (mxComponent, UNO_QUERY_THROW);
    rtl::OUString aNamedRangesPropertyString("NamedRanges");
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(aNamedRangesPropertyString), UNO_QUERY_THROW);

    //set value from xnamedranges.hxx
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
    xSheet = uno::Reference< sheet::XSpreadsheet >(xIndexAccess->getByIndex(nSheet),UNO_QUERY_THROW);

    return xNamedRanges;
}

void ScNamedRangesObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScNamedRangesObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangesObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
