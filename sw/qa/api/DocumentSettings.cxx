/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <comphelper/processfactory.hxx>

#include "DocumentSettingsTest.hxx"
#include "SettingsTest.hxx"
#include "PrinterSettingsTest.hxx"

using namespace css;

namespace
{
/**
 * Test for Java API test of file com.sun.star.comp.Writer.DocumentSettings.csv
 */
class DocumentSettingsTest : public test::BootstrapFixture,
                             public unotest::MacrosTest,
                             public apitest::DocumentSettingsTest,
                             public apitest::SettingsTest,
                             public apitest::PrinterSettingsTest
{
private:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    std::unordered_map<OUString, uno::Reference<uno::XInterface>> init() override;

    CPPUNIT_TEST_SUITE(DocumentSettingsTest);
    CPPUNIT_TEST(testDocumentSettingsProperties);
    CPPUNIT_TEST(testSettingsProperties);
    CPPUNIT_TEST(testPrinterSettingsProperties);
    CPPUNIT_TEST_SUITE_END();
};

void DocumentSettingsTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void DocumentSettingsTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

std::unordered_map<OUString, uno::Reference<uno::XInterface>> DocumentSettingsTest::init()
{
    std::unordered_map<OUString, uno::Reference<uno::XInterface>> map;

    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xFactory(xTextDocument, uno::UNO_QUERY_THROW);

    uno::Reference<uno::XInterface> xDocumentSettings(
        xFactory->createInstance("com.sun.star.text.DocumentSettings"), uno::UNO_QUERY_THROW);

    // DocumentSettings
    map["text::DocumentSettings"] = xDocumentSettings;
    // Settings
    map["document::Settings"] = xDocumentSettings;
    // Printer Settings
    map["text::PrinterSettings"] = xDocumentSettings;
    return map;
}

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentSettingsTest);

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
