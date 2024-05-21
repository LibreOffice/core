/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/lang/xcomponent.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextFrame.hpp>

using namespace css;

namespace
{
/**
 * Test for Java API test of file com.sun.star.comp.office.SwXTextFrame.csv
 */
class SwXTextFrame final : public UnoApiTest,
                           public apitest::XServiceInfo,
                           public apitest::XComponent
{
public:
    SwXTextFrame()
        : UnoApiTest(u""_ustr)
        , apitest::XServiceInfo(u"SwXTextFrame"_ustr, u"com.sun.star.text.TextFrame"_ustr)
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(mxComponentContext));
        mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
        CPPUNIT_ASSERT(mxComponent.is());
    }

    uno::Reference<uno::XInterface> init() override
    {
        uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextFrame> xTextFrame(
            xMSF->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY_THROW);
        auto xText = xTextDocument->getText();
        auto xTextCursor = xText->createTextCursor();
        CPPUNIT_ASSERT(xTextCursor.is());
        xText->insertTextContent(xTextCursor, xTextFrame, false);
        xTextCursor->gotoEnd(false);
        return xTextFrame;
    }

    void triggerDesktopTerminate() override { mxDesktop->terminate(); }

    CPPUNIT_TEST_SUITE(SwXTextFrame);
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextFrame);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
