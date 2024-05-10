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
        : UnoApiTest(u"/sfx2/qa/cppunit/data/"_ustr)
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
        {"Name", uno::Any(u"Non-Business"_ustr)},
        {"Type", uno::Any(u"urn:bails:ExportControl:"_ustr)},
    }));
    dispatchCommand(mxComponent, u".uno:ClassificationApply"_ustr, aPropertyValues);

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocumentPropertiesSupplier.is());
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet> xPropertySet(xDocumentProperties->getUserDefinedProperties(), uno::UNO_QUERY);
    uno::Any aAny = xPropertySet->getPropertyValue(u"urn:bails:ExportControl:BusinessAuthorizationCategory:Identifier"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"urn:example:tscp:1:non-business"_ustr, aAny.get<OUString>());

    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(u"Confidential"_ustr)},
        {"Type", uno::Any(u"urn:bails:NationalSecurity:"_ustr)},
    });
    dispatchCommand(mxComponent, u".uno:ClassificationApply"_ustr, aPropertyValues);
    aAny = xPropertySet->getPropertyValue(u"urn:bails:NationalSecurity:BusinessAuthorizationCategory:Identifier"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"urn:example:tscp:1:confidential"_ustr, aAny.get<OUString>());

    aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Name", uno::Any(u"Internal Only"_ustr)},
        {"Type", uno::Any(u"urn:bails:IntellectualProperty:"_ustr)},
    });
    dispatchCommand(mxComponent, u".uno:ClassificationApply"_ustr, aPropertyValues);
    aAny = xPropertySet->getPropertyValue(u"urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Identifier"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"urn:example:tscp:1:internal-only"_ustr, aAny.get<OUString>());
}

void ClassificationTest::testWriter()
{
    // Test SID_CLASSIFICATION_APPLY handling in SwDocShell::Execute().
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    // This resulted in a beans::UnknownPropertyException when the request wasn't handled.
    testClassification();
}

void ClassificationTest::testCalc()
{
    // Test SID_CLASSIFICATION_APPLY handling in ScFormatShell::ExecuteStyle().
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr, u"com.sun.star.sheet.SpreadsheetDocument"_ustr);
    // This resulted in a beans::UnknownPropertyException when the request wasn't handled.
    testClassification();
}

void ClassificationTest::testImpress()
{
    // Test SID_CLASSIFICATION_APPLY handling in sd::DrawViewShell::FuTemporary().
    mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr, u"com.sun.star.presentation.PresentationDocument"_ustr);
    // This resulted in a beans::UnknownPropertyException when the request wasn't handled.
    testClassification();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ClassificationTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
