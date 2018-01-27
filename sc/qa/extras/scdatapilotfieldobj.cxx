/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <test/sheet/datapilotfield.hxx>
#include <test/sheet/xdatapilotfield.hxx>
#include <test/sheet/xdatapilotfieldgrouping.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScDataPilotFieldObj : public CalcUnoApiTest, public apitest::DataPilotField,
                                                   public apitest::XDataPilotField,
                                                   public apitest::XDataPilotFieldGrouping
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference< uno::XInterface > init() override;

    ScDataPilotFieldObj();

    CPPUNIT_TEST_SUITE(ScDataPilotFieldObj);

    // DataPilotField
    CPPUNIT_TEST(testSortInfo);
    CPPUNIT_TEST(testLayoutInfo);
    CPPUNIT_TEST(testAutoShowInfo);
    CPPUNIT_TEST(testReference);
    CPPUNIT_TEST(testIsGroupField);

    // XDataPilotField
    CPPUNIT_TEST(testGetItems);

    // XDataPilotFieldGrouping
    CPPUNIT_TEST(testCreateNameGroup);
    // see fdo#
    //CPPUNIT_TEST(testCreateDateGroup);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScDataPilotFieldObj::ScDataPilotFieldObj()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScDataPilotFieldObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(1), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());
    uno::Sequence<OUString> aElementNames = xDPT->getElementNames();
    (void) aElementNames;

    uno::Reference< sheet::XDataPilotDescriptor > xDPDsc(xDPT->getByName("DataPilot1"),UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPDsc.is());
    uno::Reference< container::XIndexAccess > xIA( xDPDsc->getDataPilotFields(), UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xReturnValue( xIA->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xReturnValue.is());
    return xReturnValue;
}

void ScDataPilotFieldObj::setUp()
{
    CalcUnoApiTest::setUp();

    OUString aFileURL;
    createFileURL("scdatapilotfieldobj.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");

}

void ScDataPilotFieldObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotFieldObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
