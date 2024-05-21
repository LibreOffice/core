/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/sheetsortdescriptor2.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/XSortable.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScSortDescriptorBaseObj : public UnoApiTest, public apitest::SheetSortDescriptor2
{
public:
    ScSortDescriptorBaseObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScSortDescriptorBaseObj);

    // SheetSortDescriptor2
    CPPUNIT_TEST(testSheetSortDescriptor2Properties);

    CPPUNIT_TEST_SUITE_END();
};

ScSortDescriptorBaseObj::ScSortDescriptorBaseObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScSortDescriptorBaseObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<util::XSortable> xSortable(xSheet, UNO_QUERY_THROW);
    return xSortable;
}

void ScSortDescriptorBaseObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSortDescriptorBaseObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
