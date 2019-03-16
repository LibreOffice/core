/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/textdocumentsettings.hxx>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.hxx>

using namespace css::uno;

namespace apitest
{
TextDocumentSettings::~TextDocumentSettings() {}

void TextDocumentSettings::testDocumentSettingsProperties()
{
    css::uno::Reference<css::beans::XPropertySet> xDocumentSettings(init(),
                                                                    css::uno::UNO_QUERY_THROW);

    testBooleanOptionalProperty(xDocumentSettings, "ChartAutoUpdate");
    testBooleanOptionalProperty(xDocumentSettings, "AddParaTableSpacing");
    testBooleanOptionalProperty(xDocumentSettings, "AddParaTableSpacingAtStart");
    testBooleanOptionalProperty(xDocumentSettings, "AlignTabStopPosition");
    testBooleanOptionalProperty(xDocumentSettings, "SaveGlobalDocumentLinks");
    testBooleanOptionalProperty(xDocumentSettings, "IsLabelDocument");
    testBooleanOptionalProperty(xDocumentSettings, "UseFormerLineSpacing");
    testBooleanOptionalProperty(xDocumentSettings, "AddParaSpacingToTableCells");
    testBooleanOptionalProperty(xDocumentSettings, "UseFormerObjectPositioning");
    testBooleanOptionalProperty(xDocumentSettings, "ConsiderTextWrapOnObjPos");
    testBooleanOptionalProperty(xDocumentSettings, "MathBaselineAlignment");
}

} // end namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
