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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

#include <rtl/oustringostreaminserter.hxx>

namespace ScDataPilotFieldObj {

class ScDataPilotField : public UnoApiTest
{

    uno::Reference< beans::XPropertySet > init();

    void testSortInfo();

    CPPUNIT_TEST_SUITE(ScDataPilotField);
    CPPUNIT_TEST(testSortInfo);
    CPPUNIT_TEST_SUITE_END();
};

void ScDataPilotField::testSortInfo()
{
    uno::Reference< beans::XPropertySet> xPropSet = init();
    sheet::DataPilotFieldSortInfo aSortInfoValue;
    rtl::OUString aSortInfo(RTL_CONSTASCII_USTRINGPARAM("SortInfo"));
    aSortInfoValue.Field = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Col1"));
    aSortInfoValue.IsAscending = false;
    aSortInfoValue.Mode = sheet::DataPilotFieldSortMode::MANUAL;
    uno::Any xValue;
    xValue <<= aSortInfoValue;
    xPropSet->setPropertyValue(aSortInfo, xValue);

    sheet::DataPilotFieldSortInfo aNewSortInfoValue;
    xValue = xPropSet->getPropertyValue(aSortInfo);
    CPPUNIT_ASSERT( xValue >>= aNewSortInfoValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as got value", aNewSortInfoValue.Field == aSortInfoValue.Field
            && aNewSortInfoValue.Mode == aSortInfoValue.Mode && aNewSortInfoValue.IsAscending == aSortInfoValue.IsAscending);

    //setting HasSortInfo only makes sense for false, for true the uno implementation does nothing
    sal_Bool bHasSortInfo;
    rtl::OUString aHasSortInfo(RTL_CONSTASCII_USTRINGPARAM("HasSortInfo"));
    xValue = xPropSet->getPropertyValue(aHasSortInfo);
    CPPUNIT_ASSERT( xValue >>= bHasSortInfo );
    CPPUNIT_ASSERT_MESSAGE("should have sort info", bHasSortInfo);

    bHasSortInfo = false;
    xValue <<= bHasSortInfo;
    xPropSet->setPropertyValue(aHasSortInfo, xValue);

    xValue = xPropSet->getPropertyValue(aHasSortInfo);
    CPPUNIT_ASSERT( xValue >>= bHasSortInfo );
    CPPUNIT_ASSERT_MESSAGE("should have no sort info", !bHasSortInfo);
}

uno::Reference< beans::XPropertySet > ScDataPilotField::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("scdatapilotfieldobj.ods"));
    createFileURL(aFileBase, aFileURL);
    std::cout << rtl::OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    static uno::Reference< lang::XComponent > xComponent;
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
    uno::Reference< beans::XPropertySet > xReturnValue( xIA->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xReturnValue.is());
    return xReturnValue;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotField);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
