/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/globalsheetsettings.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void GlobalSheetSettings::testGlobalSheetSettingsProperties()
{
    uno::Reference<beans::XPropertySet> xGlobalSheetSettings(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "MoveSelection";
    bool aMoveSelection = false;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMoveSelection);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue MoveSelection", aMoveSelection);

    aNewValue <<= false;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMoveSelection);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue MoveSelection", !aMoveSelection);

    propName = "MoveDirection";
    sal_Int16 aMoveDirection = 42;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMoveDirection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue MoveDirection", sal_Int16(0),
                                 aMoveDirection);

    aNewValue <<= sal_Int16(1);
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMoveDirection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue MoveDirection", sal_Int16(1),
                                 aMoveDirection);

    propName = "EnterEdit";
    bool aEnterEdit = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aEnterEdit);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue EnterEdit", !aEnterEdit);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aEnterEdit);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue EnterEdit", aEnterEdit);

    propName = "ExtendFormat";
    bool aExtendFormat = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aExtendFormat);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ExtendFormat", !aExtendFormat);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aExtendFormat);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ExtendFormat", aExtendFormat);

    propName = "RangeFinder";
    bool aRangeFinder = false;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aRangeFinder);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue RangeFinder", aRangeFinder);

    aNewValue <<= false;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aRangeFinder);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue RangeFinder", !aRangeFinder);

    propName = "ExpandReferences";
    bool aExpandReferences = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aExpandReferences);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ExpandReferences", !aExpandReferences);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aExpandReferences);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ExpandReferences", aExpandReferences);

    propName = "MarkHeader";
    bool aMarkHeader = false;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMarkHeader);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue MarkHeader", aMarkHeader);

    aNewValue <<= false;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMarkHeader);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue MarkHeader", !aMarkHeader);

    propName = "UseTabCol";
    bool aUseTabCol = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aUseTabCol);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue UseTabCol", !aUseTabCol);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aUseTabCol);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue UseTabCol", aUseTabCol);

    propName = "Metric";
    sal_Int16 aMetric = 42;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMetric);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue Metric", sal_Int16(8), aMetric);

    aNewValue <<= sal_Int16(1);
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aMetric);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Metric", sal_Int16(1), aMetric);

    propName = "Scale";
    sal_Int16 aScale = 42;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aScale);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue Scale", sal_Int16(100), aScale);

    aNewValue <<= sal_Int16(-1);
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aScale);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Scale", sal_Int16(-1), aScale);

    propName = "DoAutoComplete";
    bool aDoAutoComplete = false;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aDoAutoComplete);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue DoAutoComplete", aDoAutoComplete);

    aNewValue <<= false;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aDoAutoComplete);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue DoAutoComplete", !aDoAutoComplete);

    propName = "StatusBarFunction";
    sal_Int16 aStatusBarFunction = 42;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aStatusBarFunction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue StatusBarFunction", sal_Int16(514),
                                 aStatusBarFunction);

    aNewValue <<= sal_Int16(1);
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aStatusBarFunction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue StatusBarFunction", sal_Int16(1),
                                 aStatusBarFunction);

    propName = "UserLists";
    uno::Sequence<OUString> aSeq{
        "Sun,Mon,Tue,Wed,Thu,Fri,Sat",
        "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday",
        "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec",
        "January,February,March,April,May,June,July,August,September,October,November,December",
        "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Shabbat",
        "Nissan,Iyar,Sivan,Tammuz,Av,Elul,Tishri,Heshvan,Kislev,Tevet,Shevat,Adar,Adar B"
    };

    uno::Sequence<OUString> aUserLists;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aUserLists);
    for (auto i = 0; i < aUserLists.getLength(); i++)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue UserLists[" + std::to_string(i)
                                         + "]",
                                     aSeq[i], aUserLists[i]);
    }

    aNewValue <<= uno::Sequence<OUString>();
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aUserLists);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue UserLists", !aUserLists.hasElements());

    propName = "LinkUpdateMode";
    sal_Int16 aLinkUpdateMode = 42;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aLinkUpdateMode);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue LinkUpdateMode", sal_Int16(2),
                                 aLinkUpdateMode);

    aNewValue <<= sal_Int16(1);
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aLinkUpdateMode);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue LinkUpdateMode", sal_Int16(1),
                                 aLinkUpdateMode);

    propName = "PrintAllSheets";
    bool aPrintAllSheets = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aPrintAllSheets);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue PrintAllSheets", !aPrintAllSheets);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aPrintAllSheets);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue PrintAllSheets", aPrintAllSheets);

    propName = "PrintEmptyPages";
    bool aPrintEmptyPages = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aPrintEmptyPages);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue PrintEmptyPages", !aPrintEmptyPages);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aPrintEmptyPages);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue PrintEmptyPages", aPrintEmptyPages);

    propName = "UsePrinterMetrics";
    bool aUsePrinterMetrics = true;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aUsePrinterMetrics);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue UsePrinterMetrics", !aUsePrinterMetrics);

    aNewValue <<= true;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aUsePrinterMetrics);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue UsePrinterMetrics", aUsePrinterMetrics);

    propName = "ReplaceCellsWarning";
    bool aReplaceCellsWarning = false;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aReplaceCellsWarning);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ReplaceCellsWarning", aReplaceCellsWarning);

    aNewValue <<= false;
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aReplaceCellsWarning);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ReplaceCellsWarning",
                           !aReplaceCellsWarning);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
