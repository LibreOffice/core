/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xsubtotalfield.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScSubTotalFieldObj : public UnoApiTest, public apitest::XSubTotalField
{
public:
    ScSubTotalFieldObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScSubTotalFieldObj);

    // XSubTotalField
    CPPUNIT_TEST(testGetSetGroupColumn);
    CPPUNIT_TEST(testGetSetTotalColumns);

    CPPUNIT_TEST_SUITE_END();
};

ScSubTotalFieldObj::ScSubTotalFieldObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScSubTotalFieldObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSubTotalCalculatable> xSubTotalCalc(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSubTotalDescriptor> xSubTotalDesc
        = xSubTotalCalc->createSubTotalDescriptor(true);

    uno::Sequence<sheet::SubTotalColumn> xCols{ { /* Column   */ 5,
                                                  /* Function */ sheet::GeneralFunction_SUM } };
    xSubTotalDesc->addNew(xCols, 1);

    uno::Reference<container::XIndexAccess> xDescIndex(xSubTotalDesc, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSubTotalField> xSTF(xDescIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    return xSTF;
}

void ScSubTotalFieldObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSubTotalFieldObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
