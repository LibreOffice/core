/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "BaseIndexTest.hxx"
#include "DocumentIndexTest.hxx"
#include "XDocumentIndexTest.hxx"
#include "XTextContentTest.hxx"

#include <test/bootstrapfixture.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/lang/xcomponent.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;

namespace
{
/**
 * Test for Java API test of file com.sun.star.comp.office.SwXDocumentIndex.csv
 */
class SwXDocumentIndexTest : public test::BootstrapFixture,
                             public unotest::MacrosTest,
                             public apitest::XDocumentIndexTest,
                             public apitest::BaseIndexTest,
                             public apitest::DocumentIndexTest,
                             public apitest::XTextContentTest,
                             public apitest::XServiceInfo,
                             public apitest::XComponent
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<text::XTextDocument> mxTextDocument;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    SwXDocumentIndexTest()
        : apitest::XServiceInfo("SwXDocumentIndex", "com.sun.star.text.BaseIndex"){};
    uno::Reference<uno::XInterface> init() override;
    uno::Reference<text::XTextDocument> getTextDocument() override { return mxTextDocument; }
    void triggerDesktopTerminate() override {}

    CPPUNIT_TEST_SUITE(SwXDocumentIndexTest);
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testBaseIndexProperties);
    CPPUNIT_TEST(testDocumentIndexProperties);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST_SUITE_END();
};

void SwXDocumentIndexTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mxTextDocument = uno::Reference<text::XTextDocument>(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"),
        uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(mxTextDocument.is());
}

void SwXDocumentIndexTest::tearDown()
{
    if (mxTextDocument.is())
        mxTextDocument->dispose();

    test::BootstrapFixture::tearDown();
}

uno::Reference<uno::XInterface> SwXDocumentIndexTest::init()
{
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxTextDocument, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDocumentIndex> xDocumentIndex(
        xMSF->createInstance("com.sun.star.text.DocumentIndex"), uno::UNO_QUERY_THROW);
    auto xText = getTextDocument()->getText();
    auto xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xDocumentIndex, false);
    xTextCursor->gotoEnd(false);
    return xDocumentIndex;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXDocumentIndexTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
