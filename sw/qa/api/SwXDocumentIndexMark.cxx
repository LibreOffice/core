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
#include <test/text/xtextcontent.hxx>
#include <test/text/baseindexmark.hxx>
#include <test/text/documentindexmark.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXDocumentIndexMark.
 */
class SwXDocumentIndexMark final : public UnoApiTest,
                                   public apitest::XComponent,
                                   public apitest::XTextContent,
                                   public apitest::BaseIndexMark,
                                   public apitest::DocumentIndexMark
{
public:
    SwXDocumentIndexMark();

    Reference<XInterface> init() override;
    uno::Reference<text::XTextRange> getTextRange() override;
    uno::Reference<text::XTextContent> getTextContent() override;
    bool isAttachSupported() override { return true; }
    void triggerDesktopTerminate() override { mxDesktop->terminate(); }

    CPPUNIT_TEST_SUITE(SwXDocumentIndexMark);
    CPPUNIT_TEST(testDispose);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testBaseIndexMarkProperties);
    CPPUNIT_TEST(testDocumentIndexMarkProperties);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<text::XTextRange> mxTextRange;
    uno::Reference<text::XTextContent> mxTextContent;
};

SwXDocumentIndexMark::SwXDocumentIndexMark()
    : UnoApiTest(u""_ustr)
{
}

Reference<XInterface> SwXDocumentIndexMark::init()
{
    mxComponent
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    Reference<text::XTextDocument> xTextDocument(mxComponent, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(mxComponent, UNO_QUERY_THROW);

    Reference<text::XText> xText = xTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    Reference<text::XDocumentIndexMark> xDIM(
        xMSF->createInstance(u"com.sun.star.text.DocumentIndexMark"_ustr), UNO_QUERY_THROW);

    mxTextRange = uno::Reference<text::XTextRange>(xCursor, uno::UNO_QUERY_THROW);
    mxTextContent = uno::Reference<text::XTextContent>(
        xMSF->createInstance(u"com.sun.star.text.DocumentIndex"_ustr), uno::UNO_QUERY_THROW);
    xText->insertTextContent(xCursor, xDIM, false);

    return Reference<XInterface>(xDIM, UNO_QUERY_THROW);
}

uno::Reference<text::XTextRange> SwXDocumentIndexMark::getTextRange() { return mxTextRange; }

uno::Reference<text::XTextContent> SwXDocumentIndexMark::getTextContent() { return mxTextContent; }

CPPUNIT_TEST_SUITE_REGISTRATION(SwXDocumentIndexMark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
