/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>

#include <test/unoapi_property_testers.hxx>
#include <test/text/textprintersettings.hxx>

namespace apitest
{
TextPrinterSettings::~TextPrinterSettings() {}
void TextPrinterSettings::testPrinterSettingsProperties()
{
    css::uno::Reference<css::beans::XPropertySet> xPrinterSettings(init(),
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
    testBooleanProperty(xPrinterSettings, "PrintProspect");
    testBooleanProperty(xPrinterSettings, "PrintPageBackground");
    testBooleanProperty(xPrinterSettings, "PrintBlackFonts");
    testBooleanOptionalProperty(xPrinterSettings, "PrintEmptyPages");
}
} // end namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
