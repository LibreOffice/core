/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xlabelrange.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScLabelRangeObj : public UnoApiTest, public apitest::XLabelRange
{
public:
    ScLabelRangeObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScLabelRangeObj);

    // XLabelRange
    CPPUNIT_TEST(testGetSetDataArea);
    CPPUNIT_TEST(testGetSetLabelArea);

    CPPUNIT_TEST_SUITE_END();
};

ScLabelRangeObj::ScLabelRangeObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScLabelRangeObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xProp(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XLabelRanges> xLabelRanges(xProp->getPropertyValue("ColumnLabelRanges"),
                                                     uno::UNO_QUERY_THROW);

    table::CellRangeAddress aCellRanageAddr1(0, 0, 1, 0, 6);
    table::CellRangeAddress aCellRanageAddr2(0, 0, 0, 0, 1);
    xLabelRanges->addNew(aCellRanageAddr1, aCellRanageAddr2);

    uno::Reference<sheet::XLabelRange> xLabelRange(xLabelRanges->getByIndex(0),
                                                   uno::UNO_QUERY_THROW);
    return xLabelRange;
}

void ScLabelRangeObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScLabelRangeObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
