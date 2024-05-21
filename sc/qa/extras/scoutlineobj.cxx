/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xsheetoutline.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScOutlineObj : public UnoApiTest, public apitest::XSheetOutline
{
public:
    ScOutlineObj();

    virtual void setUp() override;

    virtual uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(ScOutlineObj);

    // XSheetOutline
    CPPUNIT_TEST(testHideDetail);
    CPPUNIT_TEST(testShowDetail);
    CPPUNIT_TEST(testShowLevel);
    CPPUNIT_TEST(testUngroup);
    CPPUNIT_TEST(testGroup);
    //CPPUNIT_TEST(testAutoOutline);
    CPPUNIT_TEST(testClearOutline);

    CPPUNIT_TEST_SUITE_END();
};

ScOutlineObj::ScOutlineObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScOutlineObj::init()
{
    // get the first sheet
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

void ScOutlineObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    loadFromFile(u"ScOutlineObj.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOutlineObj);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
