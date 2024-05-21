/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <comphelper/processfactory.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/lang/xcomponent.hxx>
#include <unotest/macros_test.hxx>

class TerminateTest final : public test::BootstrapFixture,
                            public unotest::MacrosTest,
                            public apitest::XComponent
{
public:
    void setUp() override;

    css::uno::Reference<css::uno::XInterface> init() override;

    void triggerDesktopTerminate() override;

    CPPUNIT_TEST_SUITE(TerminateTest);
    CPPUNIT_TEST(testDisposedByDesktopTerminate);
    CPPUNIT_TEST_SUITE_END();
};

void TerminateTest::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

css::uno::Reference<css::uno::XInterface> TerminateTest::init()
{
    auto const component
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    css::uno::Reference<css::text::XTextDocument> xTextDocument(component,
                                                                css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::lang::XMultiServiceFactory> xMSF(component, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::text::XText> xText = xTextDocument->getText();
    css::uno::Reference<css::text::XTextCursor> xCursor = xText->createTextCursor();
    css::uno::Reference<css::text::XTextTable> xTable(
        xMSF->createInstance(u"com.sun.star.text.TextTable"_ustr), css::uno::UNO_QUERY_THROW);
    xTable->initialize(4, 3);
    xText->insertTextContent(xCursor, xTable, false);
    CPPUNIT_ASSERT(xCursor.is());
    return css::uno::Reference<css::uno::XInterface>(xTable, css::uno::UNO_QUERY_THROW);
}

void TerminateTest::triggerDesktopTerminate() { mxDesktop->terminate(); }

CPPUNIT_TEST_SUITE_REGISTRATION(TerminateTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
