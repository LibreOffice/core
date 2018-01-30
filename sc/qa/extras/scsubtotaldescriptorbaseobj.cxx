/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/subtotaldescriptor.hxx>
#include <test/sheet/xsubtotaldescriptor.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest {

class ScSubTotalDescriptorBaseObj : public CalcUnoApiTest,
                                    public apitest::SubTotalDescriptor,
                                    public apitest::XSubTotalDescriptor
{
public:
    ScSubTotalDescriptorBaseObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScSubTotalDescriptorBaseObj);

    // SubTotalDescriptor
    CPPUNIT_TEST(testSubTotalDescriptorProperties);

    // XSubTotalDescriptor
    CPPUNIT_TEST(testAddNew);
    CPPUNIT_TEST(testClear);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScSubTotalDescriptorBaseObj::ScSubTotalDescriptorBaseObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScSubTotalDescriptorBaseObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< container::XIndexAccess > xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference< sheet::XSubTotalCalculatable > xSTC(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor > xSTD = xSTC->createSubTotalDescriptor(true);

    uno::Sequence< sheet::SubTotalColumn > xCols;
    xCols.realloc(1);
    xCols[0].Column = 5;
    xCols[0].Function = sheet::GeneralFunction_SUM;
    xSTD->addNew(xCols, 10);

    return xSTD;
}

void ScSubTotalDescriptorBaseObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScSubTotalDescriptorBaseObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSubTotalDescriptorBaseObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
