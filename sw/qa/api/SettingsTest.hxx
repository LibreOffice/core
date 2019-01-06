/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_API_SETTINGSTEST_HXX
#define INCLUDED_SW_QA_API_SETTINGSTEST_HXX

#include "ApiTestBase.hxx"

#include <cppunit/TestAssert.h>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>

namespace apitest
{
class SettingsTest : public ApiTestBase
{
    // [property] string PrinterName;
    static void testPrinterName(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
    {
        const OUString rPropertyName("PrinterName");

        if (!extstsProperty(rxSettings, rPropertyName))
            return; // Property is sometimes not set - bug?

        OUString aPrinterName_Get;

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxSettings->getPropertyValue(rPropertyName) >>= aPrinterName_Get);

        OUString aPrinterName_Set;
        css::uno::Any aNewValue;
        aNewValue <<= aPrinterName_Get;
        rxSettings->setPropertyValue(rPropertyName, aNewValue);

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxSettings->getPropertyValue(rPropertyName) >>= aPrinterName_Set);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue", aPrinterName_Get,
                                     aPrinterName_Set);
    }

    // [optional, property] short PrinterIndependentLayout;
    static void
    testPrinterIndependentLayout(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
    {
        const OUString rPropertyName("PrinterIndependentLayout");

        if (!extstsProperty(rxSettings, rPropertyName))
            return; // Property is optional

        sal_Int16 aValue_Get;

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxSettings->getPropertyValue(rPropertyName) >>= aValue_Get);

        sal_Int16 aValue_New;
        aValue_New = (aValue_Get == 1 ? 3 : 1);
        rxSettings->setPropertyValue(rPropertyName, css::uno::Any(aValue_New));

        sal_Int16 aValue_Set;

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxSettings->getPropertyValue(rPropertyName) >>= aValue_Set);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue", aValue_New, aValue_Set);
    }

    // [optional, property] com::sun::star::i18n::XForbiddenCharacters ForbiddenCharacters;
    static void testForbiddenCharacters(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
    {
        const OUString rPropertyName("ForbiddenCharacters");

        if (!extstsProperty(rxSettings, rPropertyName))
            return; // Property is optional

        CPPUNIT_ASSERT_MESSAGE("Property is read-only but shouldn't be",
                               !isPropertyReadOnly(rxSettings, rPropertyName));

        css::uno::Reference<css::i18n::XForbiddenCharacters> aValue_Get;

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxSettings->getPropertyValue(rPropertyName) >>= aValue_Get);
        CPPUNIT_ASSERT_MESSAGE("Empty reference to XForbiddenCharacters", aValue_Get.is());
    }

public:
    void testSettingsProperties()
    {
        auto map = init();

        css::uno::Reference<css::beans::XPropertySet> xSettings(map["document::Settings"],
                                                                css::uno::UNO_QUERY_THROW);

        testForbiddenCharacters(xSettings);
        //testShortOptionalProperty(xSettings, "LinkUpdateMode");
        testPrinterName(xSettings);
        // [property] sequence< byte > PrinterSetup;
        testBooleanOptionalProperty(xSettings, "IsKernAsianPunctuation");
        //testShortOptionalProperty(xSettings, "CharacterCompressionType");
        testBooleanOptionalProperty(xSettings, "ApplyUserData");
        testBooleanOptionalProperty(xSettings, "SaveVersionOnClose");
        testBooleanOptionalProperty(xSettings, "UpdateFromTemplate");
        testBooleanOptionalProperty(xSettings, "FieldAutoUpdate");
        testStringOptionalProperty(xSettings, "CurrentDatabaseDataSource");
        testStringOptionalProperty(xSettings, "CurrentDatabaseCommand");
        testLongOptionalProperty(xSettings, "CurrentDatabaseCommandType");
        testLongOptionalProperty(xSettings, "DefaultTabStop");
        testBooleanOptionalProperty(xSettings, "IsPrintBooklet");
        testBooleanOptionalProperty(xSettings, "IsPrintBookletFront");
        testBooleanOptionalProperty(xSettings, "IsPrintBookletBack");
        testLongOptionalProperty(xSettings, "PrintQuality");
        testStringOptionalProperty(xSettings, "ColorTableURL");
        testStringOptionalProperty(xSettings, "DashTableURL");
        testStringOptionalProperty(xSettings, "LineEndTableURL");
        testStringOptionalProperty(xSettings, "HatchTableURL");
        testStringOptionalProperty(xSettings, "GradientTableURL");
        testStringOptionalProperty(xSettings, "BitmapTableURL");
        testBooleanOptionalProperty(xSettings, "AutoCalculate");
        testPrinterIndependentLayout(xSettings);
        testBooleanOptionalProperty(xSettings, "AddExternalLeading");
        testBooleanOptionalProperty(xSettings, "EmbedFonts");
        testBooleanOptionalProperty(xSettings, "EmbedSystemFonts");
        testBooleanOptionalProperty(xSettings, "EmbedOnlyUsedFonts");
        testBooleanOptionalProperty(xSettings, "EmbedLatinScriptFonts");
        testBooleanOptionalProperty(xSettings, "EmbedAsianScriptFonts");
        testBooleanOptionalProperty(xSettings, "EmbedComplexScriptFonts");
    }
};
} // end namespace apitest

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
