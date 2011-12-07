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
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>

#include <rtl/oustringostreaminserter.hxx>

namespace ScDataPilotFieldObj {

class ScDataPilotField : public UnoApiTest
{

    uno::Reference< beans::XPropertySet > init();

    void testSortInfo();
    void testLayoutInfo();
    void testAutoShowInfo();
    void testReference();
    void testIsGroupField();

    CPPUNIT_TEST_SUITE(ScDataPilotField);
    CPPUNIT_TEST(testSortInfo);
    CPPUNIT_TEST(testLayoutInfo);
    CPPUNIT_TEST(testAutoShowInfo);
    CPPUNIT_TEST(testReference);
    CPPUNIT_TEST(testIsGroupField);
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

void ScDataPilotField::testLayoutInfo()
{
    uno::Reference< beans::XPropertySet > xPropSet = init();
    sheet::DataPilotFieldLayoutInfo aLayoutInfoValue;
    rtl::OUString aLayoutInfo(RTL_CONSTASCII_USTRINGPARAM("LayoutInfo"));
    aLayoutInfoValue.AddEmptyLines = false;
    aLayoutInfoValue.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
    uno::Any xValue;
    xValue <<= aLayoutInfoValue;
    xPropSet->setPropertyValue(aLayoutInfo, xValue);

    sheet::DataPilotFieldLayoutInfo aNewLayoutInfoValue;
    xValue = xPropSet->getPropertyValue(aLayoutInfo);
    CPPUNIT_ASSERT( xValue >>= aNewLayoutInfoValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as the got value", aNewLayoutInfoValue.LayoutMode == aLayoutInfoValue.LayoutMode &&
            aNewLayoutInfoValue.AddEmptyLines == aLayoutInfoValue.AddEmptyLines);

    //setting HasLayoutInfo only makes sense for false, tor true the uno implementation does nothing
    sal_Bool bHasLayoutInfo;
    rtl::OUString aHasLayoutInfo(RTL_CONSTASCII_USTRINGPARAM("HasLayoutInfo"));
    xValue = xPropSet->getPropertyValue(aHasLayoutInfo);
    CPPUNIT_ASSERT( xValue >>= bHasLayoutInfo );
    CPPUNIT_ASSERT_MESSAGE("should have layout information", bHasLayoutInfo);

    bHasLayoutInfo = false;
    xValue <<= bHasLayoutInfo;
    xPropSet->setPropertyValue(aHasLayoutInfo, xValue);

    xValue = xPropSet->getPropertyValue(aHasLayoutInfo);
    CPPUNIT_ASSERT( xValue >>= bHasLayoutInfo );
    CPPUNIT_ASSERT_MESSAGE("should have no longer sort information", !bHasLayoutInfo);
}

void ScDataPilotField::testAutoShowInfo()
{
    uno::Reference< beans::XPropertySet > xPropSet = init();
    sheet::DataPilotFieldAutoShowInfo aAutoShowInfoValue;
    aAutoShowInfoValue.DataField = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Col1"));
    aAutoShowInfoValue.IsEnabled = true;
    rtl::OUString aAutoShowInfo(RTL_CONSTASCII_USTRINGPARAM("AutoShowInfo"));
    uno::Any xValue;
    xValue <<= aAutoShowInfoValue;
    xPropSet->setPropertyValue(aAutoShowInfo, xValue);

    sheet::DataPilotFieldAutoShowInfo aNewAutoShowInfoValue;
    xValue = xPropSet->getPropertyValue(aAutoShowInfo);
    CPPUNIT_ASSERT( xValue >>= aNewAutoShowInfoValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as the got value", aNewAutoShowInfoValue.DataField == aAutoShowInfoValue.DataField &&
            aNewAutoShowInfoValue.IsEnabled == aAutoShowInfoValue.IsEnabled);

    //setting HasLayoutInfo only makes sense for false, tor true the uno implementation does nothing
    sal_Bool bHasAutoShowInfo;
    rtl::OUString aHasAutoShowInfo(RTL_CONSTASCII_USTRINGPARAM("HasAutoShowInfo"));
    xValue = xPropSet->getPropertyValue(aHasAutoShowInfo);
    CPPUNIT_ASSERT( xValue >>= bHasAutoShowInfo );
    CPPUNIT_ASSERT_MESSAGE("should have AutoShow information", bHasAutoShowInfo);

    bHasAutoShowInfo = false;
    xValue <<= bHasAutoShowInfo;
    xPropSet->setPropertyValue(aHasAutoShowInfo, xValue);

    xValue = xPropSet->getPropertyValue(aHasAutoShowInfo);
    CPPUNIT_ASSERT( xValue >>= bHasAutoShowInfo );
    CPPUNIT_ASSERT_MESSAGE("should have no longer AutoShow information", !bHasAutoShowInfo);
}

void ScDataPilotField::testReference()
{
    uno::Reference< beans::XPropertySet > xPropSet = init();
    sheet::DataPilotFieldReference aReferenceValue;
    aReferenceValue.ReferenceField = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Col1"));
    aReferenceValue.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NAMED;
    rtl::OUString aReference(RTL_CONSTASCII_USTRINGPARAM("Reference"));
    uno::Any xValue;
    xValue <<= aReferenceValue;
    xPropSet->setPropertyValue(aReference, xValue);

    sheet::DataPilotFieldReference aNewReferenceValue;
    xValue = xPropSet->getPropertyValue(aReference);
    CPPUNIT_ASSERT( xValue >>= aNewReferenceValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as the got value", aReferenceValue.ReferenceField == aNewReferenceValue.ReferenceField
            && aReferenceValue.ReferenceItemType == aNewReferenceValue.ReferenceItemType);

    //setting HasReference only makes sense for false, tor true the uno implementation does nothing
    sal_Bool bHasReference;
    rtl::OUString aHasReference(RTL_CONSTASCII_USTRINGPARAM("HasReference"));
    xValue = xPropSet->getPropertyValue(aHasReference);
    CPPUNIT_ASSERT( xValue >>= bHasReference );
    CPPUNIT_ASSERT_MESSAGE("should have Reference information", bHasReference);

    bHasReference = false;
    xValue <<= bHasReference;
    xPropSet->setPropertyValue(aHasReference, xValue);

    xValue = xPropSet->getPropertyValue(aHasReference);
    CPPUNIT_ASSERT( xValue >>= bHasReference );
    CPPUNIT_ASSERT_MESSAGE("should have no longer reference information", !bHasReference);
}

void ScDataPilotField::testIsGroupField()
{
    uno::Reference< beans::XPropertySet > xPropSet = init();
    uno::Any xValue;
    rtl::OUString aIsGroupField(RTL_CONSTASCII_USTRINGPARAM("IsGroupField"));
    sal_Bool bIsGroupField;

    xValue = xPropSet->getPropertyValue(aIsGroupField);
    CPPUNIT_ASSERT( xValue >>= bIsGroupField);
    //only setting to false is supported
    if (bIsGroupField)
    {
        bIsGroupField = false;
        xValue <<= bIsGroupField;

        xPropSet->setPropertyValue(aIsGroupField, xValue);
        xValue = xPropSet->getPropertyValue(aIsGroupField);
        CPPUNIT_ASSERT(xValue >>= bIsGroupField);
        CPPUNIT_ASSERT_MESSAGE("setting IsGroupField is supported and should have happened", !bIsGroupField);
    }
    else
        std::cout << "Could not test IsGroupField" << std::endl;
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
