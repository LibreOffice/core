/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/lang/xcomponent.hxx>
#include <test/text/textcontent.hxx>
#include <test/text/xtextcontent.hxx>
#include <test/text/xtextfield.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTextField.
 */
struct SwXTextField final : public UnoApiTest,
                            public apitest::XPropertySet,
                            public apitest::XComponent,
                            public apitest::TextContent,
                            public apitest::XTextContent,
                            public apitest::XTextField
{
    SwXTextField()
        : UnoApiTest(u""_ustr)
        , TextContent(text::TextContentAnchorType_AS_CHARACTER,
                      text::TextContentAnchorType_AS_CHARACTER, text::WrapTextMode_NONE,
                      text::WrapTextMode_NONE)
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

        Reference<beans::XPropertySet> xFieldMaster(
            xMSF->createInstance(u"com.sun.star.text.FieldMaster.Database"_ustr), UNO_QUERY_THROW);

        xFieldMaster->setPropertyValue(u"DataBaseName"_ustr, Any(u"Address Book File"_ustr));
        xFieldMaster->setPropertyValue(u"DataTableName"_ustr, Any(u"address"_ustr));
        xFieldMaster->setPropertyValue(u"DataColumnName"_ustr, Any(u"FIRSTNAME"_ustr));

        Reference<text::XDependentTextField> xField(
            xMSF->createInstance(u"com.sun.star.text.TextField.Database"_ustr), UNO_QUERY_THROW);
        xField->attachTextFieldMaster(xFieldMaster);

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        Reference<text::XTextContent> xFieldAsContent(xField, UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xFieldAsContent, false);

        mxTextRange = Reference<text::XTextRange>(xCursor, UNO_QUERY_THROW);
        mxTextContent = Reference<text::XTextContent>(
            xMSF->createInstance(u"com.sun.star.text.TextField.DateTime"_ustr), UNO_QUERY_THROW);

        return Reference<XInterface>(xField, UNO_QUERY_THROW);
    }

    void triggerDesktopTerminate() override { mxDesktop->terminate(); };
    bool isAttachSupported() override { return true; };
    Reference<text::XTextRange> getTextRange() override { return mxTextRange; };
    Reference<text::XTextContent> getTextContent() override { return mxTextContent; };

    CPPUNIT_TEST_SUITE(SwXTextField);
    CPPUNIT_TEST(testDispose);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testTextContentProperties);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<text::XTextRange> mxTextRange;
    Reference<text::XTextContent> mxTextContent;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextField);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
