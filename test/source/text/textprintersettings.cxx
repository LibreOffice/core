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

    testBooleanProperty(xPrinterSettings, u"PrintGraphics"_ustr);
    testBooleanProperty(xPrinterSettings, u"PrintLeftPages"_ustr);
    testBooleanProperty(xPrinterSettings, u"PrintRightPages"_ustr);
    testBooleanProperty(xPrinterSettings, u"PrintControls"_ustr);
    testStringProperty(xPrinterSettings, u"PrintFaxName"_ustr, u"FaxName"_ustr);
    testBooleanProperty(xPrinterSettings, u"PrintProspect"_ustr);
    testBooleanProperty(xPrinterSettings, u"PrintPageBackground"_ustr);
    testBooleanProperty(xPrinterSettings, u"PrintBlackFonts"_ustr);
    testBooleanOptionalProperty(xPrinterSettings, u"PrintEmptyPages"_ustr);
}
} // end namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
