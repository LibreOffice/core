/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/lang/xcomponent.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/text/baseindex.hxx>
#include <test/text/textdocumentindex.hxx>
#include <test/text/xdocumentindex.hxx>
#include <test/text/xtextcontent.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Test for Java API test of file com.sun.star.comp.office.SwXDocumentIndex.csv
 */
class SwXDocumentIndex final : public UnoApiTest,
                               public apitest::XServiceInfo,
                               public apitest::XDocumentIndex,
                               public apitest::BaseIndex,
                               public apitest::TextDocumentIndex,
                               public apitest::XTextContent,
                               public apitest::XComponent
{
    Reference<text::XTextDocument> mxTextDocument;
    Reference<text::XTextRange> mxTextRange;
    Reference<text::XTextContent> mxTextContent;

public:
    // SwXDocumentIndex()
    //     : apitest::XServiceInfo("SwXDocumentIndex", "com.sun.star.text.BaseIndex"){};
    SwXDocumentIndex()
        : UnoApiTest(u""_ustr)
        , apitest::XServiceInfo(u"SwXDocumentIndex"_ustr, u"com.sun.star.text.BaseIndex"_ustr)
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(mxComponentContext));
        mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
        CPPUNIT_ASSERT(mxComponent.is());
    }

    Reference<XInterface> init() override
    {
        mxTextDocument = Reference<text::XTextDocument>(mxComponent, UNO_QUERY_THROW);
        Reference<lang::XMultiServiceFactory> xMSF(mxTextDocument, UNO_QUERY_THROW);
        Reference<text::XDocumentIndex> xDocumentIndex(
            xMSF->createInstance(u"com.sun.star.text.DocumentIndex"_ustr), UNO_QUERY_THROW);

        auto xText = getTextDocument()->getText();
        auto xTextCursor = xText->createTextCursor();
        CPPUNIT_ASSERT(xTextCursor.is());
        xText->insertTextContent(xTextCursor, xDocumentIndex, false);
        xTextCursor->gotoEnd(false);

        mxTextRange = Reference<text::XTextRange>(xTextCursor, UNO_QUERY_THROW);
        mxTextContent = Reference<text::XTextContent>(
            xMSF->createInstance(u"com.sun.star.text.DocumentIndex"_ustr), UNO_QUERY_THROW);

        return xDocumentIndex;
    }

    Reference<text::XTextRange> getTextRange() override { return mxTextRange; };
    Reference<text::XTextContent> getTextContent() override { return mxTextContent; };
    bool isAttachSupported() override { return true; }
    Reference<text::XTextDocument> getTextDocument() override { return mxTextDocument; }
    void triggerDesktopTerminate() override {}

    CPPUNIT_TEST_SUITE(SwXDocumentIndex);
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

CPPUNIT_TEST_SUITE_REGISTRATION(SwXDocumentIndex);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
