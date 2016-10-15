/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{

class ScTabViewObj : public CalcUnoApiTest
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    ScTabViewObj();

    CPPUNIT_TEST_SUITE(ScTabViewObj);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

ScTabViewObj::ScTabViewObj()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

sal_Int32 ScTabViewObj::nTest = 0;
uno::Reference< lang::XComponent > ScTabViewObj::mxComponent;

uno::Reference< uno::XInterface > ScTabViewObj::init()
{
    OUString aFileURL;
    createFileURL("ScTabViewObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    return mxComponent;
}

void ScTabViewObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScTabViewObj::tearDown()
{
    closeDocument(mxComponent);
    mxComponent.clear();

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTabViewObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
