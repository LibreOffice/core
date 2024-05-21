/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/sheet/spreadsheetviewsettings.hxx>
#include <test/sheet/xactivationbroadcaster.hxx>
#include <test/sheet/xcellrangereferrer.hxx>
#include <test/sheet/xspreadsheetview.hxx>
#include <test/sheet/xviewfreezable.hxx>
#include <test/sheet/xviewsplitable.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XViewPane.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScTabViewObj : public UnoApiTest,
                     public apitest::SpreadsheetViewSettings,
                     public apitest::XActivationBroadcaster,
                     public apitest::XCellRangeReferrer,
                     public apitest::XElementAccess,
                     public apitest::XEnumerationAccess,
                     public apitest::XIndexAccess,
                     public apitest::XSpreadsheetView,
                     public apitest::XViewFreezable,
                     public apitest::XViewSplitable
{
public:
    ScTabViewObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<uno::XInterface> getXSpreadsheet(const sal_Int16 nNumber = 0) override;

    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScTabViewObj);

    // SpreadsheetViewSettings
    CPPUNIT_TEST(testSpreadsheetViewSettingsProperties);

    // XActivationBroadcaster
    CPPUNIT_TEST(testAddRemoveActivationEventListener);

    // XCellRangeReferrer
    //Disabled till it's clear why it fails on some machines.
    //CPPUNIT_TEST(testGetReferredCells);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XSpreadsheetView
    CPPUNIT_TEST(testGetSetActiveSheet);

    // XViewFreezable
    CPPUNIT_TEST(testFreeze);

    // XViewSplitable
    CPPUNIT_TEST(testSplit);

    CPPUNIT_TEST_SUITE_END();
};

ScTabViewObj::ScTabViewObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XElementAccess(cppu::UnoType<sheet::XViewPane>::get())
    , XIndexAccess(1)
{
}

uno::Reference<uno::XInterface> ScTabViewObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY_THROW);

    setCellRange(table::CellRangeAddress(0, 0, 0, 6, 23));

    return xModel->getCurrentController();
}

uno::Reference<uno::XInterface> ScTabViewObj::getXSpreadsheet(const sal_Int16 nNumber)
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    xSheets->insertNewByName(u"Sheet2"_ustr, 2);
    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(nNumber), UNO_QUERY_THROW);

    return xSheet;
}

void ScTabViewObj::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTabViewObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
