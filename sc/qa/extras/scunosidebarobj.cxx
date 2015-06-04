/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/ui/xsidebarprovider.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScUnoSidebarObj : public CalcUnoApiTest, apitest::XSidebarProvider
{
public:
    ScUnoSidebarObj();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    virtual uno::Reference< uno::XInterface > init() SAL_OVERRIDE;
    virtual uno::Reference< lang::XComponent  > getComponent() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(ScUnoSidebarObj);
    CPPUNIT_TEST(testSidebar);
    CPPUNIT_TEST_SUITE_END();

private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;


};

sal_Int32 ScUnoSidebarObj::nTest = 0;
uno::Reference< lang::XComponent > ScUnoSidebarObj::mxComponent;


ScUnoSidebarObj::ScUnoSidebarObj()
       : CalcUnoApiTest("sc/qa/extras/testdocuments")
{
}


uno::Reference< uno::XInterface > ScUnoSidebarObj::init()
{
  return getComponent();
}


uno::Reference< lang::XComponent > ScUnoSidebarObj::getComponent(){
  // target is always an empty document

    if (mxComponent.is())
        closeDocument(mxComponent);

    mxComponent = loadFromDesktop("private:factory/scalc");

    return mxComponent;
}

void ScUnoSidebarObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScUnoSidebarObj::tearDown()
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

CPPUNIT_TEST_SUITE_REGISTRATION(ScUnoSidebarObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
