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
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/oustringostreaminserter.hxx>

namespace ScDataPilotDescriptorBase
{

#define NUMBER_OF_TESTS 2

class ScXDataPilotTable : public UnoApiTest
{
public:

    virtual void setUp();
    virtual void tearDown();

    void testGetOutputRange();
    void testRefresh();
    CPPUNIT_TEST_SUITE(ScXDataPilotTable);
    CPPUNIT_TEST(testGetOutputRange);
    CPPUNIT_TEST(testRefresh);
    CPPUNIT_TEST_SUITE_END();
private:

    uno::Reference< sheet::XDataPilotTable > init();

    static int nTest;
    static uno::Reference< lang::XComponent > xComponent;

    uno::Reference< table::XCell > xCellForChange;
    uno::Reference< table::XCell > xCellForCheck;
};

int ScXDataPilotTable::nTest = 0;
uno::Reference< lang::XComponent > ScXDataPilotTable::xComponent;

void ScXDataPilotTable::testGetOutputRange()
{
    uno::Reference< sheet::XDataPilotTable > xDPTable = init();

    table::CellRangeAddress aRange = xDPTable->getOutputRange();
    CPPUNIT_ASSERT( aRange.Sheet == 0 );
    CPPUNIT_ASSERT( aRange.StartColumn == 7 );
    CPPUNIT_ASSERT( aRange.StartRow == 8 );
}

void ScXDataPilotTable::testRefresh()
{
    uno::Reference< sheet::XDataPilotTable > xDPTable = init();
    xCellForChange->setValue( 5 );

    double aOldData = xCellForCheck->getValue();
    xDPTable->refresh();
    double aNewData = xCellForCheck->getValue();
    CPPUNIT_ASSERT_MESSAGE("value needs to change", aOldData != aNewData);
}

uno::Reference< sheet::XDataPilotTable > ScXDataPilotTable::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("ScDataPilotTableObj.ods"));
    createFileURL(aFileBase, aFileURL);
    std::cout << rtl::OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    // set variables
    xCellForChange = xSheet->getCellByPosition( 1, 5 );
    xCellForCheck = xSheet->getCellByPosition( 7, 11 );

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());
    uno::Sequence<rtl::OUString> aElementNames = xDPT->getElementNames();

    uno::Reference< sheet::XDataPilotTable > xDPTable(xDPT->getByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataPilotTable"))),UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xDPTable.is());
    return xDPTable;
}

void ScXDataPilotTable::setUp()
{
    nTest += 1;
    UnoApiTest::setUp();
}

void ScXDataPilotTable::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        uno::Reference< util::XCloseable > xCloseable(xComponent, UNO_QUERY_THROW);
        xCloseable->close( false );
    }

    UnoApiTest::tearDown();

    if (nTest == NUMBER_OF_TESTS)
    {
        mxDesktop->terminate();
        uno::Reference< lang::XComponent>(m_xContext, UNO_QUERY_THROW)->dispose();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
