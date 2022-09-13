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

    auto DoCheck = [&xGlobalSheetSettings, &configProvider](
                       const OUString& propName, const auto& origValue, const auto& newValue,
                       const OUString& regNodeName, const OUString& regValueName) {
        OString sMessage = "PropertyValue " + propName.toUtf8();
        css::uno::Any aOrigValue(origValue), aNewValue(newValue);

        css::uno::Sequence<css::uno::Any> args{ css::uno::Any(
            css::beans::NamedValue("nodepath", css::uno::Any(regNodeName))) };
        css::uno::Reference<beans::XPropertySet> xRegNodeRO(
            configProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess", args),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<beans::XPropertySet> xRegNodeRW(
            configProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationUpdateAccess", args),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::util::XChangesBatch> xBatch(xRegNodeRW, css::uno::UNO_QUERY_THROW);

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

    OUString node = "/org.openoffice.Office.Calc/Input";
    DoCheck("MoveSelection", true, false, node, "MoveSelection");
    DoCheck("MoveDirection", sal_Int16(0), sal_Int16(1), node, "MoveSelectionDirection");
    DoCheck("EnterEdit", false, true, node, "SwitchToEditMode");
    DoCheck("ExtendFormat", false, true, node, "ExpandFormatting");
    DoCheck("RangeFinder", true, false, node, "ShowReference");
    DoCheck("ExpandReferences", false, true, node, "ExpandReference");
    DoCheck("MarkHeader", true, false, node, "HighlightSelection");
    DoCheck("UseTabCol", false, true, node, "UseTabCol");
    DoCheck("ReplaceCellsWarning", true, false, node, "ReplaceCellsWarning");

    node = "/org.openoffice.Office.Calc/Layout/Other/MeasureUnit";
    DoCheck("Metric", sal_Int16(8), sal_Int16(1), node, "NonMetric"); // Test uses en-US locale

    node = "/org.openoffice.Office.Calc/Input";
    DoCheck("DoAutoComplete", true, false, node, "AutoInput");

    node = "/org.openoffice.Office.Calc/Content/Update";
    DoCheck("LinkUpdateMode", sal_Int16(2), sal_Int16(1), node, "Link");

    node = "/org.openoffice.Office.Calc/Print/";
    DoCheck("PrintAllSheets", false, true, node + "Other", "AllSheets");
    DoCheck("PrintEmptyPages", false, true, node + "Page", "EmptyPages");

    OUString propName;
    uno::Any aNewValue;

    propName = "Scale";
    sal_Int16 aScale = 42;
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aScale);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue Scale", sal_Int16(100), aScale);

    aNewValue <<= sal_Int16(-1);
    xGlobalSheetSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xGlobalSheetSettings->getPropertyValue(propName) >>= aScale);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue Scale", sal_Int16(-1), aScale);

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
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
