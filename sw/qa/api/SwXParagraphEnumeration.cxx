/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXParagraphEnumeration.
 */
class SwXParagraphEnumeration final : public UnoApiTest, public apitest::XEnumeration
{
public:
    SwXParagraphEnumeration()
        : UnoApiTest(u""_ustr)
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

        for (int i = 0; i < 5; i++)
        {
            try
            {
                xText->insertString(xCursor, u"The quick brown fox jumps over the lazy dog"_ustr,
                                    false);
                xText->insertControlCharacter(xCursor, text::ControlCharacter::LINE_BREAK, false);
                xText->insertString(xCursor, u"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG"_ustr,
                                    false);
                xText->insertControlCharacter(xCursor, text::ControlCharacter::LINE_BREAK, false);
            }
            catch (lang::IllegalArgumentException&)
            {
            }

            try
            {
                xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK,
                                              false);
            }
            catch (lang::IllegalArgumentException&)
            {
            }
        }

        Reference<container::XEnumerationAccess> xEnumAccess(xText, UNO_QUERY_THROW);
        Reference<container::XEnumeration> xEnum = xEnumAccess->createEnumeration();

        return Reference<XInterface>(xEnum, UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXParagraphEnumeration);
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXParagraphEnumeration);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
