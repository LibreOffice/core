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
            css::beans::NamedValue(u"nodepath"_ustr, css::uno::Any(regNodeName))) };
        css::uno::Reference<beans::XPropertySet> xRegNodeRO(
            configProvider->createInstanceWithArguments(
                u"com.sun.star.configuration.ConfigurationAccess"_ustr, args),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<beans::XPropertySet> xRegNodeRW(
            configProvider->createInstanceWithArguments(
                u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr, args),
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

    OUString node = u"/org.openoffice.Office.Calc/Input"_ustr;
    DoCheck(u"MoveSelection"_ustr, true, false, node, u"MoveSelection"_ustr);
    DoCheck(u"MoveDirection"_ustr, sal_Int16(0), sal_Int16(1), node,
            u"MoveSelectionDirection"_ustr);
    DoCheck(u"EnterEdit"_ustr, false, true, node, u"SwitchToEditMode"_ustr);
    DoCheck(u"ExtendFormat"_ustr, false, true, node, u"ExpandFormatting"_ustr);
    DoCheck(u"RangeFinder"_ustr, true, false, node, u"ShowReference"_ustr);
    DoCheck(u"ExpandReferences"_ustr, false, true, node, u"ExpandReference"_ustr);
    DoCheck(u"MarkHeader"_ustr, true, false, node, u"HighlightSelection"_ustr);
    DoCheck(u"UseTabCol"_ustr, false, true, node, u"UseTabCol"_ustr);
    DoCheck(u"ReplaceCellsWarning"_ustr, true, false, node, u"ReplaceCellsWarning"_ustr);

    node = "/org.openoffice.Office.Calc/Layout/Other/MeasureUnit";
    DoCheck(u"Metric"_ustr, sal_Int16(8), sal_Int16(1), node,
            u"NonMetric"_ustr); // Test uses en-US locale

    node = "/org.openoffice.Office.Calc/Input";
    DoCheck(u"DoAutoComplete"_ustr, true, false, node, u"AutoInput"_ustr);

    node = "/org.openoffice.Office.Calc/Content/Update";
    DoCheck(u"LinkUpdateMode"_ustr, sal_Int16(2), sal_Int16(1), node, u"Link"_ustr);

    node = "/org.openoffice.Office.Calc/Print/";
    DoCheck(u"PrintAllSheets"_ustr, false, true, node + "Other", u"AllSheets"_ustr);
    DoCheck(u"PrintEmptyPages"_ustr, false, true, node + "Page", u"EmptyPages"_ustr);

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
        u"Sun,Mon,Tue,Wed,Thu,Fri,Sat"_ustr,
        u"Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday"_ustr,
        u"Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec"_ustr,
        u"January,February,March,April,May,June,July,August,September,October,November,December"_ustr,
        u"Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Shabbat"_ustr,
        u"Nissan,Iyar,Sivan,Tammuz,Av,Elul,Tishri,Heshvan,Kislev,Tevet,Shevat,Adar,Adar B"_ustr
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
