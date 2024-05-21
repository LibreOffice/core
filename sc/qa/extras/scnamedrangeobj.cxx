/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xnamed.hxx>
#include <test/sheet/xnamedrange.hxx>
#include <test/sheet/xcellrangereferrer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScNamedRangeObj : public UnoApiTest,
                        public apitest::XCellRangeReferrer,
                        public apitest::XNamed,
                        public apitest::XNamedRange
{
public:
    ScNamedRangeObj();

    virtual void setUp() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< sheet::XNamedRange > getNamedRange(const OUString& rRangeName) override;

    CPPUNIT_TEST_SUITE(ScNamedRangeObj);

    // XCellRangeReferrer
    CPPUNIT_TEST(testGetReferredCells);

    // XNamed
    CPPUNIT_TEST(testSetName);
    CPPUNIT_TEST(testGetName);

    // XNamedRange
    CPPUNIT_TEST(testGetContent);
    CPPUNIT_TEST(testSetContent);
    CPPUNIT_TEST(testGetType);
    CPPUNIT_TEST(testSetType);
    CPPUNIT_TEST(testGetReferencePosition);
    CPPUNIT_TEST(testSetReferencePosition);

    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference< sheet::XNamedRanges > init_impl();
};

ScNamedRangeObj::ScNamedRangeObj():
        UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr),
        apitest::XNamed(u"NamedRange"_ustr)
{
}

uno::Reference< sheet::XNamedRanges > ScNamedRangeObj::init_impl()
{
    CPPUNIT_ASSERT_MESSAGE("no component loaded", mxComponent.is());

    uno::Reference< beans::XPropertySet > xPropSet (mxComponent, UNO_QUERY_THROW);
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(u"NamedRanges"_ustr), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNamedRanges.is());

    setCellRange(table::CellRangeAddress(0, 1, 7, 1, 7));

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
    return getNamedRange(u"NamedRange"_ustr);
}

void ScNamedRangeObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    loadFromFile(u"ScNamedRangeObj.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangeObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
