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
#include <test/lang/xcomponent.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextFrame.hpp>

using namespace css;

namespace
{
/**
 * Test for Java API test of file com.sun.star.comp.office.SwXTextFrame.csv
 */
class SwXTextFrame final : public test::BootstrapFixture,
                           public unotest::MacrosTest,
                           public apitest::XServiceInfo,
                           public apitest::XComponent
{
    uno::Reference<text::XTextDocument> mxTextDocument;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    SwXTextFrame()
        : apitest::XServiceInfo("SwXTextFrame", "com.sun.star.text.TextFrame"){};
    uno::Reference<uno::XInterface> init() override;
    const uno::Reference<text::XTextDocument>& getTextDocument() const { return mxTextDocument; }
    void triggerDesktopTerminate() override { mxDesktop->terminate(); }

    CPPUNIT_TEST_SUITE(SwXTextFrame);
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST_SUITE_END();
};

void SwXTextFrame::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mxTextDocument = uno::Reference<text::XTextDocument>(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"),
        uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(mxTextDocument.is());
}

void SwXTextFrame::tearDown()
{
    if (mxTextDocument.is())
        mxTextDocument->dispose();

    test::BootstrapFixture::tearDown();
}

uno::Reference<uno::XInterface> SwXTextFrame::init()
{
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxTextDocument, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextFrame> xTextFrame(xMSF->createInstance("com.sun.star.text.TextFrame"),
                                                uno::UNO_QUERY_THROW);
    auto xText = getTextDocument()->getText();
    auto xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xTextFrame, false);
    xTextCursor->gotoEnd(false);
    return xTextFrame;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextFrame);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
