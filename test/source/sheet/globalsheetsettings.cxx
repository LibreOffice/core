/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/globalsheetsettings.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <comphelper/processfactory.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void GlobalSheetSettings::testGlobalSheetSettingsProperties()
{
    uno::Reference<beans::XPropertySet> xGlobalSheetSettings(init(), UNO_QUERY_THROW);
    auto configProvider
        = css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext());
    css::uno::Sequence<css::uno::Any> args{ css::uno::Any(css::beans::NamedValue(
        "nodepath", css::uno::makeAny(OUString("/org.openoffice.Office.Calc/Input")))) };
    css::uno::Reference<beans::XPropertySet> xRegNodeRO(
        configProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess", args),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference<beans::XPropertySet> xRegNodeRW(
        configProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess", args),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::util::XChangesBatch> xBatch(xRegNodeRW, css::uno::UNO_QUERY_THROW);

    auto DoCheck = [&xGlobalSheetSettings, &xRegNodeRO, &xRegNodeRW,
                    &xBatch](const OUString& propName, const auto& origValue, const auto& newValue,
                             const OUString& regValueName) {
        OString sMessage = "PropertyValue " + propName.toUtf8();
        css::uno::Any aOrigValue(origValue), aNewValue(newValue);

        // 1. Check initial value
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aOrigValue,
                                     xGlobalSheetSettings->getPropertyValue(propName));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aOrigValue,
                                     xRegNodeRO->getPropertyValue(regValueName));

        // 2. Check setting the value through GlobalSheetSettings
        xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aNewValue,
                                     xGlobalSheetSettings->getPropertyValue(propName));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aNewValue,
                                     xRegNodeRO->getPropertyValue(regValueName));

        // 3. Check setting the value through ConfigurationUpdateAccess
        xRegNodeRW->setPropertyValue(regValueName, aOrigValue);
        xBatch->commitChanges();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aOrigValue,
                                     xRegNodeRO->getPropertyValue(regValueName));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aOrigValue,
                                     xGlobalSheetSettings->getPropertyValue(propName));
    };

    DoCheck("MoveSelection", true, false, "MoveSelection");
    DoCheck("MoveDirection", sal_Int16(0), sal_Int16(1), "MoveSelectionDirection");
    DoCheck("EnterEdit", false, true, "SwitchToEditMode");
    DoCheck("ExtendFormat", false, true, "ExpandFormatting");
    DoCheck("RangeFinder", true, false, "ShowReference");
    DoCheck("ExpandReferences", false, true, "ExpandReference");
    DoCheck("MarkHeader", true, false, "HighlightSelection");
    DoCheck("UseTabCol", false, true, "UseTabCol");
    DoCheck("UsePrinterMetrics", false, true, "UsePrinterMetrics");
    DoCheck("ReplaceCellsWarning", true, false, "ReplaceCellsWarning");

    OUString propName;
    uno::Any aNewValue;

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
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
