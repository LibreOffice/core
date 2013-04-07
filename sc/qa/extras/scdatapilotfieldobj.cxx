/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xdatapilotfieldgrouping.hxx>
#include <test/sheet/datapilotfield.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 6

class ScDataPilotFieldObj : public UnoApiTest, apitest::XDataPilotFieldGrouping,
                                apitest::DataPilotField
{
public:
    virtual void setUp();
    virtual void tearDown();
    virtual uno::Reference< uno::XInterface > init();

    ScDataPilotFieldObj();

    CPPUNIT_TEST_SUITE(ScDataPilotFieldObj);
    CPPUNIT_TEST(testSortInfo);
    CPPUNIT_TEST(testLayoutInfo);
    CPPUNIT_TEST(testAutoShowInfo);
    CPPUNIT_TEST(testReference);
    CPPUNIT_TEST(testIsGroupField);
    CPPUNIT_TEST(testCreateNameGroup);
    // see fdo#
    //CPPUNIT_TEST(testCreateDateGroup);
    CPPUNIT_TEST_SUITE_END();
private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScDataPilotFieldObj::nTest = 0;
uno::Reference< lang::XComponent > ScDataPilotFieldObj::mxComponent;

ScDataPilotFieldObj::ScDataPilotFieldObj()
     : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScDataPilotFieldObj::init()
{
    OUString aFileURL;
    createFileURL(OUString("scdatapilotfieldobj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(1), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());
    uno::Sequence<OUString> aElementNames = xDPT->getElementNames();

    uno::Reference< sheet::XDataPilotDescriptor > xDPDsc(xDPT->getByName(OUString("DataPilot1")),UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPDsc.is());
    uno::Reference< container::XIndexAccess > xIA( xDPDsc->getDataPilotFields(), UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xReturnValue( xIA->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xReturnValue.is());
    return xReturnValue;
}

void ScDataPilotFieldObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScDataPilotFieldObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotFieldObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
