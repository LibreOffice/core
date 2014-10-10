/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <test/sheet/perf/xcalcsearch.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScSearchObj : public CalcUnoApiTest, apitest::XCalcSearch
{
public:
    ScSearchObj();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    virtual uno::Reference< uno::XInterface > init() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(ScSearchObj);
    CPPUNIT_TEST(testSheetFindAll);
    CPPUNIT_TEST_SUITE_END();

private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;

};

sal_Int32 ScSearchObj::nTest = 0;
uno::Reference< lang::XComponent > ScSearchObj::mxComponent;

ScSearchObj::ScSearchObj()
       : CalcUnoApiTest("sc/qa/perf/testdocuments/")
{
}

uno::Reference< uno::XInterface > ScSearchObj::init()
{
    if (mxComponent.is())
        closeDocument(mxComponent);

    OUString aFileURL;
    createFileURL(OUString("scBigFile.ods"), aFileURL);
    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT(mxComponent.is());

    return mxComponent;
}

void ScSearchObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScSearchObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
      if (mxComponent.is())
      {
          closeDocument(mxComponent);
          mxComponent.clear();
      }
    }
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSearchObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
