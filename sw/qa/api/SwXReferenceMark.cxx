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
#include <test/container/xnamed.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNamed.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXReferenceMark.
 */
class SwXReferenceMark final : public UnoApiTest,
                               public apitest::XComponent,
                               public apitest::XTextContent,
                               public apitest::XNamed

{
public:
    SwXReferenceMark()
        : UnoApiTest(u""_ustr)
        , XNamed(u"SwXReferenceMark"_ustr)
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
        Reference<text::XTextDocument> xTextDocument(mxComponent, UNO_QUERY_THROW);
        Reference<lang::XMultiServiceFactory> xMSF(mxComponent, UNO_QUERY_THROW);

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        Reference<XInterface> xRefMark
            = xMSF->createInstance(u"com.sun.star.text.ReferenceMark"_ustr);
        Reference<container::XNamed> xNamed(xRefMark, UNO_QUERY_THROW);
        xNamed->setName(u"SwXReferenceMark"_ustr);

        Reference<text::XTextContent> xTextContent(xRefMark, UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xTextContent, false);

        mxTextRange = Reference<text::XTextRange>(xCursor, uno::UNO_QUERY_THROW);
        mxTextContent = Reference<text::XTextContent>(
            xMSF->createInstance(u"com.sun.star.text.ReferenceMark"_ustr), UNO_QUERY_THROW);

        return xRefMark;
    }

    Reference<text::XTextRange> getTextRange() override { return mxTextRange; };
    Reference<text::XTextContent> getTextContent() override { return mxTextContent; };
    bool isAttachSupported() override { return true; }
    void triggerDesktopTerminate() override { mxDesktop->terminate(); };

    CPPUNIT_TEST_SUITE(SwXReferenceMark);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testDispose);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetName);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<text::XTextRange> mxTextRange;
    Reference<text::XTextContent> mxTextContent;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXReferenceMark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
