/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/lang/xcomponent.hxx>
#include <test/container/xnamed.hxx>
#include <test/text/xtextcontent.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXBookmark.
 */
class SwXBookmark final : public test::BootstrapFixture,
                          public unotest::MacrosTest,
                          public apitest::XComponent,
                          public apitest::XNamed,
                          public apitest::XTextContent
{
public:
    SwXBookmark();
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;
    Reference<text::XTextRange> getTextRange() override;
    Reference<text::XTextContent> getTextContent() override;
    bool isAttachSupported() override { return true; }
    Reference<text::XTextDocument> getTextDocument() { return mxTextDocument; }
    void triggerDesktopTerminate() override { mxDesktop->terminate(); }

    CPPUNIT_TEST_SUITE(SwXBookmark);
    CPPUNIT_TEST(testDispose);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetName);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<text::XTextDocument> mxTextDocument;
    Reference<text::XTextRange> mxTextRange;
    Reference<text::XTextContent> mxTextContent;
};

SwXBookmark::SwXBookmark()
    : XNamed("Bookmark")
{
}

void SwXBookmark::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mxTextDocument = Reference<text::XTextDocument>(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"),
        UNO_QUERY_THROW);
    CPPUNIT_ASSERT(mxTextDocument.is());
}

void SwXBookmark::tearDown()
{
    if (mxTextDocument.is())
        mxTextDocument->dispose();

    test::BootstrapFixture::tearDown();
}

Reference<XInterface> SwXBookmark::init()
{
    Reference<lang::XMultiServiceFactory> xMSF(mxTextDocument, UNO_QUERY_THROW);

    Reference<text::XText> xText = getTextDocument()->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    Reference<text::XTextContent> xBookmark(xMSF->createInstance("com.sun.star.text.Bookmark"),
                                            UNO_QUERY_THROW);

    xText->insertTextContent(xCursor, xBookmark, false);
    mxTextRange = Reference<text::XTextRange>(xCursor, UNO_QUERY_THROW);
    mxTextContent = Reference<text::XTextContent>(
        xMSF->createInstance("com.sun.star.text.Bookmark"), UNO_QUERY_THROW);

    return Reference<XInterface>(xBookmark, UNO_QUERY_THROW);
}

Reference<text::XTextRange> SwXBookmark::getTextRange() { return mxTextRange; }

Reference<text::XTextContent> SwXBookmark::getTextContent() { return mxTextContent; }

CPPUNIT_TEST_SUITE_REGISTRATION(SwXBookmark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
