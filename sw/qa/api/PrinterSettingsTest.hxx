/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_API_PRINTERSETTINGSTEST_HXX
#define INCLUDED_SW_QA_API_PRINTERSETTINGSTEST_HXX

#include "ApiTestBase.hxx"

#include <cppunit/TestAssert.h>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/NotePrintMode.hpp>

namespace apitest
{
class PrinterSettingsTest : public ApiTestBase
{
private:
    void propertyTestNotePrintMode(
        css::uno::Reference<css::beans::XPropertySet> const& rxPrinterSettings,
        OUString const& rPropertyName)
    {
        css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo(
            rxPrinterSettings->getPropertySetInfo());

        CPPUNIT_ASSERT_MESSAGE("Property is not set",
                               !xPropertySetInfo->hasPropertyByName(rPropertyName));

        css::text::NotePrintMode aNotePrintMode_Get;

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxPrinterSettings->getPropertyValue(rPropertyName)
                               >>= aNotePrintMode_Get);

        css::text::NotePrintMode aNotePrintMode_Set;
        css::uno::Any aNewValue;
        aNewValue <<= css::text::NotePrintMode_ONLY;
        rxPrinterSettings->setPropertyValue(rPropertyName, aNewValue);

        CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue",
                               rxPrinterSettings->getPropertyValue(rPropertyName)
                               >>= aNotePrintMode_Set);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue", css::text::NotePrintMode_ONLY,
                                     aNotePrintMode_Set);
    }

public:
    void testPrinterSettingsProperties()
    {
        auto map = init();

        css::uno::Reference<css::beans::XPropertySet> xPrinterSettings(map["text::PrinterSettings"],
                                                                       css::uno::UNO_QUERY_THROW);

        testBooleanProperty(xPrinterSettings, "PrintGraphics");
        testBooleanProperty(xPrinterSettings, "PrintTables");
        testBooleanProperty(xPrinterSettings, "PrintDrawings");
        testBooleanProperty(xPrinterSettings, "PrintLeftPages");
        testBooleanProperty(xPrinterSettings, "PrintRightPages");
        testBooleanProperty(xPrinterSettings, "PrintControls");
        testBooleanProperty(xPrinterSettings, "PrintReversed");
        testBooleanProperty(xPrinterSettings, "PrintControls");
        testStringProperty(xPrinterSettings, "PrintFaxName", "FaxName");
        //propertyTestNotePrintMode(xPrinterSettings, "PrintAnnotationMode"); // Isn't set in this case
        testBooleanProperty(xPrinterSettings, "PrintProspect");
        testBooleanProperty(xPrinterSettings, "PrintPageBackground");
        testBooleanProperty(xPrinterSettings, "PrintBlackFonts");
        testBooleanOptionalProperty(xPrinterSettings, "PrintEmptyPages");
    }
};
} // end namespace apitest

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
