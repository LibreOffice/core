/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xgoalseek.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScModelObj : public UnoApiTest, apitest::XGoalSeek
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    ScModelObj();

    CPPUNIT_TEST_SUITE(ScModelObj);
    CPPUNIT_TEST(testSeekGoal);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

ScModelObj::ScModelObj()
     : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

sal_Int32 ScModelObj::nTest = 0;
uno::Reference< lang::XComponent > ScModelObj::mxComponent;

uno::Reference< uno::XInterface > ScModelObj::init()
{
    OUString aFileURL;
    createFileURL(OUString("ScModelObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    return mxComponent;
}

void ScModelObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScModelObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScModelObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
