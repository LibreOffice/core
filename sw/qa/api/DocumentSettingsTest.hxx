/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_API_DOCUMENTSETTINGSTEST_HXX
#define INCLUDED_SW_QA_API_DOCUMENTSETTINGSTEST_HXX

#include "ApiTestBase.hxx"

#include <cppunit/TestAssert.h>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

namespace apitest
{
class DocumentSettingsTest : public ApiTestBase
{
public:
    void testDocumentSettingsProperties()
    {
        auto map = init();

        css::uno::Reference<css::beans::XPropertySet> xDocumentSettings(
            map["text::DocumentSettings"], css::uno::UNO_QUERY_THROW);

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
