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
#include <test/text/footnote.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/text/xtextrange.hxx>
#include <test/text/xtextcontent.hxx>
#include <test/text/xtext.hxx>
#include <test/text/xfootnote.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XFootnote.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXFootnote.
 */
class SwXFootnote final : public UnoApiTest,
                          public apitest::XComponent,
                          public apitest::XSimpleText,
                          public apitest::XTextRange,
                          public apitest::XTextContent,
                          public apitest::XText,
                          public apitest::XFootnote,
                          public apitest::Footnote
{
public:
    SwXFootnote();

    Reference<XInterface> init() override;
    Reference<text::XTextRange> getTextRange() override;
    Reference<text::XTextContent> getTextContent() override;
    bool isAttachSupported() override { return true; }
    void triggerDesktopTerminate() override {}

    CPPUNIT_TEST_SUITE(SwXFootnote);
    CPPUNIT_TEST(testDispose);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertString);
    CPPUNIT_TEST(testInsertControlCharacter);
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testGetSetString);
    CPPUNIT_TEST(testGetStart);
    CPPUNIT_TEST(testGetText);
    CPPUNIT_TEST(testFootnoteProperties);
    // CPPUNIT_TEST(testInsertRemoveTextContent);
    CPPUNIT_TEST(testGetSetLabel);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<text::XTextRange> mxTextRange;
    Reference<text::XTextContent> mxTextContent;
};

SwXFootnote::SwXFootnote()
    : UnoApiTest(u""_ustr)
{
}

Reference<XInterface> SwXFootnote::init()
{
    mxComponent
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    Reference<text::XTextDocument> xTextDocument(mxComponent, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(mxComponent, UNO_QUERY_THROW);
    Reference<text::XFootnote> xFootnote(xMSF->createInstance(u"com.sun.star.text.Footnote"_ustr),
                                         UNO_QUERY_THROW);

    Reference<text::XText> xText = xTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    xText->insertTextContent(xCursor, xFootnote, false);

    mxTextRange = Reference<text::XTextRange>(xCursor, UNO_QUERY_THROW);
    mxTextContent = Reference<text::XTextContent>(
        xMSF->createInstance(u"com.sun.star.text.Footnote"_ustr), UNO_QUERY_THROW);

    return Reference<XInterface>(xFootnote, UNO_QUERY_THROW);
}

Reference<text::XTextRange> SwXFootnote::getTextRange() { return mxTextRange; }

Reference<text::XTextContent> SwXFootnote::getTextContent() { return mxTextContent; }

CPPUNIT_TEST_SUITE_REGISTRATION(SwXFootnote);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
