/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xnamedrange.hxx>
#include <test/container/xnamed.hxx>
#include <test/sheet/xcellrangereferrer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 9

class ScNamedRangeObj : public CalcUnoApiTest, apitest::XNamedRange, apitest::XNamed, apitest::XCellRangeReferrer
{
public:
    ScNamedRangeObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< sheet::XNamedRange > getNamedRange(const OUString& rRangeName) override;

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
        CalcUnoApiTest("/sc/qa/extras/testdocuments"),
        apitest::XNamed(OUString("NamedRange")),
        apitest::XCellRangeReferrer(table::CellRangeAddress(0,1,7,1,7))
{
}

uno::Reference< sheet::XNamedRanges > ScNamedRangeObj::init_impl()
{
    OUString aFileURL;
    createFileURL("ScNamedRangeObj.ods", aFileURL);
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
    return getNamedRange("NamedRange");
}

void ScNamedRangeObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScNamedRangeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangeObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
