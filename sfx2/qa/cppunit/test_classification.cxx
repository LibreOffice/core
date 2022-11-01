/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>

using namespace ::com::sun::star;

namespace
{

/// Tests the handling of the .uno:ClassificationApply command in various applications.
class ClassificationTest : public UnoApiTest
{
    void testClassification();

public:
    ClassificationTest()
        : UnoApiTest("/sfx2/qa/cppunit/data/")
    {
    }

    void testWriter();
    void testCalc();
    void testImpress();

    CPPUNIT_TEST_SUITE(ClassificationTest);
    CPPUNIT_TEST(testWriter);
    CPPUNIT_TEST(testCalc);
    CPPUNIT_TEST(testImpress);
    CPPUNIT_TEST_SUITE_END();
};

void ClassificationTest::testClassification()
{
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(OUString("Non-Business"))},
        {"Type", uno::Any(OUString("urn:bails:ExportControl:"))},
    }));
    dispatchCommand(mxComponent, ".uno:ClassificationApply", aPropertyValues);

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocumentPropertiesSupplier.is());
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet> xPropertySet(xDocumentProperties->getUserDefinedProperties(), uno::UNO_QUERY);
    uno::Any aAny = xPropertySet->getPropertyValue("urn:bails:ExportControl:BusinessAuthorizationCategory:Identifier");
    CPPUNIT_ASSERT_EQUAL(OUString("urn:example:tscp:1:non-business"), aAny.get<OUString>());

    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(OUString("Confidential"))},
        {"Type", uno::Any(OUString("urn:bails:NationalSecurity:"))},
    });
    dispatchCommand(mxComponent, ".uno:ClassificationApply", aPropertyValues);
    aAny = xPropertySet->getPropertyValue("urn:bails:NationalSecurity:BusinessAuthorizationCategory:Identifier");
    CPPUNIT_ASSERT_EQUAL(OUString("urn:example:tscp:1:confidential"), aAny.get<OUString>());

    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(OUString("Internal Only"))},
        {"Type", uno::Any(OUString("urn:bails:IntellectualProperty:"))},
    });
    dispatchCommand(mxComponent, ".uno:ClassificationApply", aPropertyValues);
    aAny = xPropertySet->getPropertyValue("urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Identifier");
    CPPUNIT_ASSERT_EQUAL(OUString("urn:example:tscp:1:internal-only"), aAny.get<OUString>());
}

void ClassificationTest::testWriter()
{
    // Test SID_CLASSIFICATION_APPLY handling in SwDocShell::Execute().
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    // This resulted in a beans::UnknownPropertyException when the request wasn't handled.
    testClassification();
}

void ClassificationTest::testCalc()
{
    // Test SID_CLASSIFICATION_APPLY handling in ScFormatShell::ExecuteStyle().
    mxComponent = loadFromDesktop("private:factory/scalc", "com.sun.star.sheet.SpreadsheetDocument");
    // This resulted in a beans::UnknownPropertyException when the request wasn't handled.
    testClassification();
}

void ClassificationTest::testImpress()
{
    // Test SID_CLASSIFICATION_APPLY handling in sd::DrawViewShell::FuTemporary().
    mxComponent = loadFromDesktop("private:factory/simpress", "com.sun.star.presentation.PresentationDocument");
    // This resulted in a beans::UnknownPropertyException when the request wasn't handled.
    testClassification();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ClassificationTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
