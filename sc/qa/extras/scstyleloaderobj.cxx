/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xstyleloader.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScStyleLoaderObj : public CalcUnoApiTest, public apitest::XStyleLoader
{
public:
    ScStyleLoaderObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< lang::XComponent > getTargetComponent() override;
    virtual uno::Reference< lang::XComponent > getSourceComponent() override;
    virtual OUString getTestURL() override;

    CPPUNIT_TEST_SUITE(ScStyleLoaderObj);

    // XStyleLoader
    CPPUNIT_TEST(testLoadStylesFromURL);
    CPPUNIT_TEST(testLoadStylesFromDocument);

    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference< lang::XComponent > mxSourceComponent;
    uno::Reference< lang::XComponent > mxTargetComponent;
};

ScStyleLoaderObj::ScStyleLoaderObj()
    : CalcUnoApiTest("sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScStyleLoaderObj::init()
{
    return getTargetComponent();
}

uno::Reference< lang::XComponent > ScStyleLoaderObj::getTargetComponent()
{
    // target is always an empty document
    return mxTargetComponent;
}

uno::Reference< lang::XComponent > ScStyleLoaderObj::getSourceComponent()
{
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxSourceComponent.is());
    return mxSourceComponent;
}

OUString ScStyleLoaderObj::getTestURL()
{
      OUString aFileURL;
      createFileURL("ScStyleLoaderObj.ods", aFileURL);
      return aFileURL;
}

void ScStyleLoaderObj::setUp()
{
    CalcUnoApiTest::setUp();

    mxTargetComponent = loadFromDesktop("private:factory/scalc");
    // get the test file url
    OUString aFileURL = getTestURL();
    mxSourceComponent = loadFromDesktop(aFileURL);
}

void ScStyleLoaderObj::tearDown()
{
    closeDocument(mxSourceComponent);
    closeDocument(mxTargetComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScStyleLoaderObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
