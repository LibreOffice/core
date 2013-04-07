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
#include <test/sheet/xnamedrange.hxx>
#include <test/container/xnamed.hxx>
#include <test/sheet/xcellrangereferrer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 9

class ScNamedRangeObj : public UnoApiTest, apitest::XNamedRange, apitest::XNamed, apitest::XCellRangeReferrer
{
public:
    ScNamedRangeObj();

    virtual void setUp();
    virtual void tearDown();

    virtual uno::Reference< uno::XInterface > init();
    virtual uno::Reference< sheet::XNamedRange > getNamedRange(const OUString& rRangeName);

    CPPUNIT_TEST_SUITE(ScNamedRangeObj);
    CPPUNIT_TEST(testGetContent);
    CPPUNIT_TEST(testSetContent);
    CPPUNIT_TEST(testGetType);
    CPPUNIT_TEST(testSetType);
    CPPUNIT_TEST(testGetReferencePosition);
    CPPUNIT_TEST(testSetReferencePosition);
    CPPUNIT_TEST(testSetName);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testGetReferredCells);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference< sheet::XNamedRanges > init_impl();

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScNamedRangeObj::nTest = 0;
uno::Reference< lang::XComponent > ScNamedRangeObj::mxComponent;

ScNamedRangeObj::ScNamedRangeObj():
        UnoApiTest("/sc/qa/extras/testdocuments"),
        apitest::XNamed(OUString("NamedRange")),
        apitest::XCellRangeReferrer(table::CellRangeAddress(0,1,7,1,7))
{
}

uno::Reference< sheet::XNamedRanges > ScNamedRangeObj::init_impl()
{
    OUString aFileURL;
    createFileURL(OUString("ScNamedRangeObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< beans::XPropertySet > xPropSet (mxComponent, UNO_QUERY_THROW);
    OUString aNamedRangesPropertyString("NamedRanges");
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(aNamedRangesPropertyString), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNamedRanges.is());

    return xNamedRanges;
}

uno::Reference< sheet::XNamedRange> ScNamedRangeObj::getNamedRange(const OUString& rRangeName)
{
    uno::Reference< container::XNameAccess > xNamedAccess(init_impl(), UNO_QUERY_THROW);
    uno::Reference< sheet::XNamedRange > xNamedRange(xNamedAccess->getByName(rRangeName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNamedRange.is());

    return xNamedRange;
}

uno::Reference< uno::XInterface > ScNamedRangeObj::init()
{
    return getNamedRange(OUString("NamedRange"));
}

void ScNamedRangeObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScNamedRangeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangeObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
