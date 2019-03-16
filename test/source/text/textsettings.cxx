/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/extensions/HelperMacros.h>

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
                        OUString const& rPropertyName)
{
    css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo(
        rxPropertySet->getPropertySetInfo());
    css::uno::Sequence<css::beans::Property> xProperties = xPropertySetInfo->getProperties();

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
    const OUString rPropertyName("PrinterName");

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
void testForbiddenCharacters(css::uno::Reference<css::beans::XPropertySet> const& rxSettings)
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
} // end namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
