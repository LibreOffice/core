/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/datapilotfield.hxx>
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

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"
#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest {

void DataPilotField::testSortInfo()
{
    uno::Reference< beans::XPropertySet> xPropSet(init(),UNO_QUERY_THROW);
    sheet::DataPilotFieldSortInfo aSortInfoValue;
    OUString aSortInfo("SortInfo");
    aSortInfoValue.Field = "Col1";
    aSortInfoValue.IsAscending = false;
    aSortInfoValue.Mode = sheet::DataPilotFieldSortMode::MANUAL;
    uno::Any aValue;
    aValue <<= aSortInfoValue;
    xPropSet->setPropertyValue(aSortInfo, aValue);

    sheet::DataPilotFieldSortInfo aNewSortInfoValue;
    aValue = xPropSet->getPropertyValue(aSortInfo);
    CPPUNIT_ASSERT( aValue >>= aNewSortInfoValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as got value", aNewSortInfoValue.Field == aSortInfoValue.Field
            && aNewSortInfoValue.Mode == aSortInfoValue.Mode && aNewSortInfoValue.IsAscending == aSortInfoValue.IsAscending);

    //setting HasSortInfo only makes sense for false, for true the uno implementation does nothing
    bool bHasSortInfo(false);
    OUString aHasSortInfo("HasSortInfo");
    aValue = xPropSet->getPropertyValue(aHasSortInfo);
    CPPUNIT_ASSERT( aValue >>= bHasSortInfo );
    CPPUNIT_ASSERT_MESSAGE("should have sort info", bHasSortInfo);

    bHasSortInfo = false;
    aValue <<= bHasSortInfo;
    xPropSet->setPropertyValue(aHasSortInfo, aValue);

    aValue = xPropSet->getPropertyValue(aHasSortInfo);
    CPPUNIT_ASSERT( aValue >>= bHasSortInfo );
    CPPUNIT_ASSERT_MESSAGE("should have no sort info", !bHasSortInfo);
}

void DataPilotField::testLayoutInfo()
{
    uno::Reference< beans::XPropertySet > xPropSet(init(),UNO_QUERY_THROW);
    sheet::DataPilotFieldLayoutInfo aLayoutInfoValue;
    OUString aLayoutInfo("LayoutInfo");
    aLayoutInfoValue.AddEmptyLines = false;
    aLayoutInfoValue.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
    uno::Any aValue;
    aValue <<= aLayoutInfoValue;
    xPropSet->setPropertyValue(aLayoutInfo, aValue);

    sheet::DataPilotFieldLayoutInfo aNewLayoutInfoValue;
    aValue = xPropSet->getPropertyValue(aLayoutInfo);
    CPPUNIT_ASSERT( aValue >>= aNewLayoutInfoValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as the got value", aNewLayoutInfoValue.LayoutMode == aLayoutInfoValue.LayoutMode &&
            aNewLayoutInfoValue.AddEmptyLines == aLayoutInfoValue.AddEmptyLines);

    //setting HasLayoutInfo only makes sense for false, tor true the uno implementation does nothing
    bool bHasLayoutInfo(false);
    OUString aHasLayoutInfo("HasLayoutInfo");
    aValue = xPropSet->getPropertyValue(aHasLayoutInfo);
    CPPUNIT_ASSERT( aValue >>= bHasLayoutInfo );
    CPPUNIT_ASSERT_MESSAGE("should have layout information", bHasLayoutInfo);

    bHasLayoutInfo = false;
    aValue <<= bHasLayoutInfo;
    xPropSet->setPropertyValue(aHasLayoutInfo, aValue);

    aValue = xPropSet->getPropertyValue(aHasLayoutInfo);
    CPPUNIT_ASSERT( aValue >>= bHasLayoutInfo );
    CPPUNIT_ASSERT_MESSAGE("should have no longer sort information", !bHasLayoutInfo);
}

void DataPilotField::testAutoShowInfo()
{
    uno::Reference< beans::XPropertySet > xPropSet(init(),UNO_QUERY_THROW);
    sheet::DataPilotFieldAutoShowInfo aAutoShowInfoValue;
    aAutoShowInfoValue.DataField = "Col1";
    aAutoShowInfoValue.IsEnabled = true;
    OUString aAutoShowInfo("AutoShowInfo");
    uno::Any aValue;
    aValue <<= aAutoShowInfoValue;
    xPropSet->setPropertyValue(aAutoShowInfo, aValue);

    sheet::DataPilotFieldAutoShowInfo aNewAutoShowInfoValue;
    aValue = xPropSet->getPropertyValue(aAutoShowInfo);
    CPPUNIT_ASSERT( aValue >>= aNewAutoShowInfoValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as the got value", aNewAutoShowInfoValue.DataField == aAutoShowInfoValue.DataField &&
            aNewAutoShowInfoValue.IsEnabled == aAutoShowInfoValue.IsEnabled);

    //setting HasLayoutInfo only makes sense for false, tor true the uno implementation does nothing
    bool bHasAutoShowInfo(false);
    OUString aHasAutoShowInfo("HasAutoShowInfo");
    aValue = xPropSet->getPropertyValue(aHasAutoShowInfo);
    CPPUNIT_ASSERT( aValue >>= bHasAutoShowInfo );
    CPPUNIT_ASSERT_MESSAGE("should have AutoShow information", bHasAutoShowInfo);

    bHasAutoShowInfo = false;
    aValue <<= bHasAutoShowInfo;
    xPropSet->setPropertyValue(aHasAutoShowInfo, aValue);

    aValue = xPropSet->getPropertyValue(aHasAutoShowInfo);
    CPPUNIT_ASSERT( aValue >>= bHasAutoShowInfo );
    CPPUNIT_ASSERT_MESSAGE("should have no longer AutoShow information", !bHasAutoShowInfo);
}

void DataPilotField::testReference()
{
    uno::Reference< beans::XPropertySet > xPropSet(init(),UNO_QUERY_THROW);
    sheet::DataPilotFieldReference aReferenceValue;
    aReferenceValue.ReferenceField = "Col1";
    aReferenceValue.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NAMED;
    OUString aReference("Reference");
    uno::Any aValue;
    aValue <<= aReferenceValue;
    xPropSet->setPropertyValue(aReference, aValue);

    sheet::DataPilotFieldReference aNewReferenceValue;
    aValue = xPropSet->getPropertyValue(aReference);
    CPPUNIT_ASSERT( aValue >>= aNewReferenceValue );
    CPPUNIT_ASSERT_MESSAGE("set value should be the same as the got value", aReferenceValue.ReferenceField == aNewReferenceValue.ReferenceField
            && aReferenceValue.ReferenceItemType == aNewReferenceValue.ReferenceItemType);

    //setting HasReference only makes sense for false, tor true the uno implementation does nothing
    bool bHasReference(false);
    OUString aHasReference("HasReference");
    aValue = xPropSet->getPropertyValue(aHasReference);
    CPPUNIT_ASSERT( aValue >>= bHasReference );
    CPPUNIT_ASSERT_MESSAGE("should have Reference information", bHasReference);

    bHasReference = false;
    aValue <<= bHasReference;
    xPropSet->setPropertyValue(aHasReference, aValue);

    aValue = xPropSet->getPropertyValue(aHasReference);
    CPPUNIT_ASSERT( aValue >>= bHasReference );
    CPPUNIT_ASSERT_MESSAGE("should have no longer reference information", !bHasReference);
}

void DataPilotField::testIsGroupField()
{
    uno::Reference< beans::XPropertySet > xPropSet(init(),UNO_QUERY_THROW);
    uno::Any aValue;
    OUString aIsGroupField("IsGroupField");
    bool bIsGroupField(false);

    aValue = xPropSet->getPropertyValue(aIsGroupField);
    CPPUNIT_ASSERT( aValue >>= bIsGroupField);
    //only setting to false is supported
    if (bIsGroupField)
    {
        bIsGroupField = false;
        aValue <<= bIsGroupField;

        xPropSet->setPropertyValue(aIsGroupField, aValue);
        aValue = xPropSet->getPropertyValue(aIsGroupField);
        CPPUNIT_ASSERT(aValue >>= bIsGroupField);
        CPPUNIT_ASSERT_MESSAGE("setting IsGroupField is supported and should have happened", !bIsGroupField);
    }
    else
        std::cout << "Could not test IsGroupField" << std::endl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
