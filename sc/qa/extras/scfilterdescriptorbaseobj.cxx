/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsheetfilterdescriptor.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSheetFilterable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScFilterDescriptorBaseObj : public CalcUnoApiTest, public apitest::XSheetFilterDescriptor
{
public:
    ScFilterDescriptorBaseObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    CPPUNIT_TEST_SUITE(ScFilterDescriptorBaseObj);

    // XSheetFilterDescriptor
    CPPUNIT_TEST(testGetSetFilterFields);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScFilterDescriptorBaseObj::nTest = 0;
uno::Reference< lang::XComponent > ScFilterDescriptorBaseObj::mxComponent;

ScFilterDescriptorBaseObj::ScFilterDescriptorBaseObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScFilterDescriptorBaseObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< container::XIndexAccess > xIndexAccess(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 5)->setValue(15);
    xSheet->getCellByPosition(1, 4)->setValue(10);
    xSheet->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference< sheet::XSheetFilterable > xSF(xSheet, uno::UNO_QUERY_THROW);

    return xSF->createFilterDescriptor(true);
}

void ScFilterDescriptorBaseObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScFilterDescriptorBaseObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFilterDescriptorBaseObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
