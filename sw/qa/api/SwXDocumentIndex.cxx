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
                             public apitest::XTextContentTest
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    std::unordered_map<OUString, uno::Reference<uno::XInterface>> init() override;

    CPPUNIT_TEST_SUITE(SwXDocumentIndexTest);
    CPPUNIT_TEST(testGetServiceName);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testBaseIndexProperties);
    CPPUNIT_TEST(testDocumentIndexProperties);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST_SUITE_END();
};

void SwXDocumentIndexTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SwXDocumentIndexTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

std::unordered_map<OUString, uno::Reference<uno::XInterface>> SwXDocumentIndexTest::init()
{
    std::unordered_map<OUString, uno::Reference<uno::XInterface>> map;

    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xFactory(xTextDocument, uno::UNO_QUERY_THROW);

    uno::Reference<text::XDocumentIndex> xDocumentIndex(
        xFactory->createInstance("com.sun.star.text.DocumentIndex"), uno::UNO_QUERY_THROW);

    uno::Reference<text::XTextContent> xTextContent(xDocumentIndex, uno::UNO_QUERY_THROW);

    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xTextContent, false);
    xTextCursor->gotoEnd(false);

    uno::Reference<text::XDocumentIndex> xDocumentIndexInstance(
        xFactory->createInstance("com.sun.star.text.DocumentIndex"), uno::UNO_QUERY_THROW);

    // XDocumentIndexTest
    map["text::XDocumentIndex"] = xDocumentIndex;
    map["text::XTextDocument"] = xTextDocument;
    // BaseIndexTest
    map["text::BaseIndex"] = xDocumentIndex;
    // DocumentIndex
    map["text::DocumentIndex"] = xDocumentIndex;
    // XTextContentTest
    map["text::XTextRange"] = xTextCursor;
    map["text::XTextContent"] = xDocumentIndex;
    map["text::XTextContent#Instance"] = xDocumentIndexInstance;

    return map;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXDocumentIndexTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
