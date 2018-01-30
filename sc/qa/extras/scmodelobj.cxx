/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xconsolidatable.hxx>
#include <test/sheet/xgoalseek.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScModelObj : public UnoApiTest, public apitest::XConsolidatable,
                                      public apitest::XGoalSeek
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    ScModelObj();

    CPPUNIT_TEST_SUITE(ScModelObj);

    // XConsolidatable
    CPPUNIT_TEST(testCreateConsolidationDescriptor);
    CPPUNIT_TEST(testConsolidate);

    // XGoalSeek
    CPPUNIT_TEST(testSeekGoal);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScModelObj::ScModelObj()
     : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScModelObj::init()
{
    CPPUNIT_ASSERT_MESSAGE("no component loaded", mxComponent.is());

    return mxComponent;
}

void ScModelObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    OUString aFileURL;
    createFileURL("ScModelObj.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
}

void ScModelObj::tearDown()
{
    closeDocument(mxComponent);
    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScModelObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
