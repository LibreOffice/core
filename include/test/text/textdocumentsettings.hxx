/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_TEXT_TEXTDOCUMENTSETTINGS_HXX
#define INCLUDED_TEST_TEXT_TEXTDOCUMENTSETTINGS_HXX

#include <test/unoapi_property_testers.hxx>
#include <test/testdllapi.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

namespace apitest
{
class OOO_DLLPUBLIC_TEST TextDocumentSettings
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual ~TextDocumentSettings() {}
    void testDocumentSettingsProperties()
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
};
} // end namespace apitest

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
