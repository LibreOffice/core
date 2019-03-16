/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/text/textdocumentsettings.hxx>
#include <test/text/textprintersettings.hxx>
#include <test/text/textsettings.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;

namespace
{
/**
 * Test for Java API test of file com.sun.star.comp.Writer.DocumentSettings.csv
 */
class SwXDocumentSettings final : public test::BootstrapFixture,
                                  public unotest::MacrosTest,
                                  public apitest::TextDocumentSettings,
                                  public apitest::TextSettings,
                                  public apitest::TextPrinterSettings,
                                  public apitest::XServiceInfo
{
private:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    SwXDocumentSettings()
        : apitest::XServiceInfo("SwXDocumentSettings", "com.sun.star.text.DocumentSettings"){};
    uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(SwXDocumentSettings);
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);
    CPPUNIT_TEST(testDocumentSettingsProperties);
    CPPUNIT_TEST(testSettingsProperties);
    CPPUNIT_TEST(testPrinterSettingsProperties);
    CPPUNIT_TEST_SUITE_END();
};

void SwXDocumentSettings::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SwXDocumentSettings::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

uno::Reference<uno::XInterface> SwXDocumentSettings::init()
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xFactory(xTextDocument, uno::UNO_QUERY_THROW);

    uno::Reference<uno::XInterface> xDocumentSettings(
        xFactory->createInstance("com.sun.star.text.DocumentSettings"), uno::UNO_QUERY_THROW);

    return xDocumentSettings;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXDocumentSettings);

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
