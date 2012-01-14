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
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotFieldGrouping.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <rtl/oustringostreaminserter.hxx>

namespace ScDataPilotFieldObj {

#define NUMBER_OF_TESTS  1

class ScXDataPilotFieldGrouping : public UnoApiTest
{
public:
    void testCreateNameGroup();
    void testCreateDateGroup();

    virtual void setUp();
    virtual void tearDown();

    CPPUNIT_TEST_SUITE(ScXDataPilotFieldGrouping);
    CPPUNIT_TEST(testCreateNameGroup);
    //broken: fdo#43609
    //CPPUNIT_TEST(testCreateDateGroup);
    CPPUNIT_TEST_SUITE_END();

    uno::Reference< sheet::XDataPilotFieldGrouping > init();

private:
    static int nTest;
    static uno::Reference< lang::XComponent > xComponent;
};

int ScXDataPilotFieldGrouping::nTest = 0;
uno::Reference< lang::XComponent > ScXDataPilotFieldGrouping::xComponent;

void ScXDataPilotFieldGrouping::testCreateNameGroup()
{
    uno::Reference< sheet::XDataPilotFieldGrouping > xDataPilotFieldGrouping = init();
    uno::Reference< sheet::XDataPilotField > xDataPilotField( xDataPilotFieldGrouping, UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess( xDataPilotField->getItems(), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xNameAccess->hasElements());

    uno::Sequence< rtl::OUString > aElements = xNameAccess->getElementNames();
    xDataPilotFieldGrouping->createNameGroup( aElements );
}

void ScXDataPilotFieldGrouping::testCreateDateGroup()
{
    uno::Reference< sheet::XDataPilotFieldGrouping > xDataPilotFieldGrouping = init();
    sheet::DataPilotFieldGroupInfo aGroupInfo;
    aGroupInfo.GroupBy = sheet::DataPilotFieldGroupBy::MONTHS;
    aGroupInfo.HasDateValues = true;
    xDataPilotFieldGrouping->createDateGroup(aGroupInfo);
}

uno::Reference< sheet::XDataPilotFieldGrouping> ScXDataPilotFieldGrouping::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("scdatapilotfieldobj.ods"));
    createFileURL(aFileBase, aFileURL);
    std::cout << rtl::OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(1), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());
    uno::Sequence<rtl::OUString> aElementNames = xDPT->getElementNames();
    for (int i = 0; i < aElementNames.getLength(); ++i)
    {
        std::cout << "PivotTable: " << aElementNames[i] << std::endl;
    }

    uno::Reference< sheet::XDataPilotDescriptor > xDPDsc(xDPT->getByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataPilot1"))),UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xDPDsc.is());
    uno::Reference< container::XIndexAccess > xIA( xDPDsc->getDataPilotFields(), UNO_QUERY_THROW);
    uno::Reference< sheet::XDataPilotFieldGrouping > xReturnValue( xIA->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xReturnValue.is());
    return xReturnValue;
}

void ScXDataPilotFieldGrouping::setUp()
{
    nTest += 1;
    UnoApiTest::setUp();
}

void ScXDataPilotFieldGrouping::tearDown()
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

CPPUNIT_TEST_SUITE_REGISTRATION(ScXDataPilotFieldGrouping);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
