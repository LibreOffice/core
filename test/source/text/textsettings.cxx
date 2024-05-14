/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>

#include <test/unoapi_property_testers.hxx>
#include <test/text/textsettings.hxx>

namespace
{
bool extstsProperty(css::uno::Reference<css::beans::XPropertySet> const& rxPropertySet,
                    OUString const& rPropertyName)
{
    css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo(
        rxPropertySet->getPropertySetInfo());
    return xPropertySetInfo->hasPropertyByName(rPropertyName);
}

bool isPropertyReadOnly(css::uno::Reference<css::beans::XPropertySet> const& rxPropertySet,
                        std::u16string_view rPropertyName)
{
    css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo(
        rxPropertySet->getPropertySetInfo());
    const css::uno::Sequence<css::beans::Property> xProperties = xPropertySetInfo->getProperties();

    for (auto const& rProperty : xProperties)
    {
        if (rProperty.Name == rPropertyName)
            return (rProperty.Attributes & com::sun::star::beans::PropertyAttribute::READONLY) != 0;
    }

    return false;
}
// [property] string PrinterName;
void testPrinterName(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
{
    static constexpr OUString rPropertyName(u"PrinterName"_ustr);

    if (!extstsProperty(rxSettings, rPropertyName))
        return; // Property is sometimes not set - bug? it is not defined as optional

    OUString aPrinterName_Get;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                           rxSettings->getPropertyValue(rPropertyName) >>= aPrinterName_Get);

    OUString aPrinterName_Set;
    css::uno::Any aNewValue;
    aNewValue <<= aPrinterName_Get;
    rxSettings->setPropertyValue(rPropertyName, aNewValue);

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                           rxSettings->getPropertyValue(rPropertyName) >>= aPrinterName_Set);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue", aPrinterName_Get, aPrinterName_Set);
}

// [optional, property] short PrinterIndependentLayout;
void testPrinterIndependentLayout(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
{
    static constexpr OUString rPropertyName(u"PrinterIndependentLayout"_ustr);

    if (!extstsProperty(rxSettings, rPropertyName))
        return; // Property is optional

    sal_Int16 aValue_Get = {};

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
void testForbiddenCharacters(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
{
    static constexpr OUString rPropertyName(u"ForbiddenCharacters"_ustr);

    if (!extstsProperty(rxSettings, rPropertyName))
        return; // Property is optional

    CPPUNIT_ASSERT_MESSAGE("Property is read-only but shouldn't be",
                           !isPropertyReadOnly(rxSettings, rPropertyName));

    css::uno::Reference<css::i18n::XForbiddenCharacters> aValue_Get;

    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                           rxSettings->getPropertyValue(rPropertyName) >>= aValue_Get);
    CPPUNIT_ASSERT_MESSAGE("Empty reference to XForbiddenCharacters", aValue_Get.is());
}
}

namespace apitest
{
TextSettings::~TextSettings() {}
void TextSettings::testSettingsProperties()
{
    css::uno::Reference<css::beans::XPropertySet> xSettings(init(), css::uno::UNO_QUERY_THROW);

    testForbiddenCharacters(xSettings);
    //testShortOptionalProperty(xSettings, "LinkUpdateMode");
    testPrinterName(xSettings);
    // [property] sequence< byte > PrinterSetup;
    testBooleanOptionalProperty(xSettings, u"IsKernAsianPunctuation"_ustr);
    //testShortOptionalProperty(xSettings, "CharacterCompressionType");
    testBooleanOptionalProperty(xSettings, u"ApplyUserData"_ustr);
    testBooleanOptionalProperty(xSettings, u"SaveVersionOnClose"_ustr);
    testBooleanOptionalProperty(xSettings, u"UpdateFromTemplate"_ustr);
    testBooleanOptionalProperty(xSettings, u"FieldAutoUpdate"_ustr);
    testStringOptionalProperty(xSettings, u"CurrentDatabaseDataSource"_ustr);
    testStringOptionalProperty(xSettings, u"CurrentDatabaseCommand"_ustr);
    testLongOptionalProperty(xSettings, u"CurrentDatabaseCommandType"_ustr);
    testLongOptionalProperty(xSettings, u"DefaultTabStop"_ustr);
    testBooleanOptionalProperty(xSettings, u"IsPrintBooklet"_ustr);
    testBooleanOptionalProperty(xSettings, u"IsPrintBookletFront"_ustr);
    testBooleanOptionalProperty(xSettings, u"IsPrintBookletBack"_ustr);
    testLongOptionalProperty(xSettings, u"PrintQuality"_ustr);
    testStringOptionalProperty(xSettings, u"ColorTableURL"_ustr);
    testStringOptionalProperty(xSettings, u"DashTableURL"_ustr);
    testStringOptionalProperty(xSettings, u"LineEndTableURL"_ustr);
    testStringOptionalProperty(xSettings, u"HatchTableURL"_ustr);
    testStringOptionalProperty(xSettings, u"GradientTableURL"_ustr);
    testStringOptionalProperty(xSettings, u"BitmapTableURL"_ustr);
    testBooleanOptionalProperty(xSettings, u"AutoCalculate"_ustr);
    testPrinterIndependentLayout(xSettings);
    testBooleanOptionalProperty(xSettings, u"AddExternalLeading"_ustr);
    testBooleanOptionalProperty(xSettings, u"EmbedFonts"_ustr);
    testBooleanOptionalProperty(xSettings, u"EmbedSystemFonts"_ustr);
    testBooleanOptionalProperty(xSettings, u"EmbedOnlyUsedFonts"_ustr);
    testBooleanOptionalProperty(xSettings, u"EmbedLatinScriptFonts"_ustr);
    testBooleanOptionalProperty(xSettings, u"EmbedAsianScriptFonts"_ustr);
    testBooleanOptionalProperty(xSettings, u"EmbedComplexScriptFonts"_ustr);
}
} // end namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
