/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/lang/xcomponent.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTextTable.
 */
struct SwXTextTable final : public test::BootstrapFixture,
                            public unotest::MacrosTest,
                            public apitest::XComponent
{
    virtual void setUp() override;

    Reference<XInterface> init() override;
    void triggerDesktopTerminate() override;

    CPPUNIT_TEST_SUITE(SwXTextTable);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testDisposedByDesktopTerminate);
    CPPUNIT_TEST_SUITE_END();
};

void SwXTextTable::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SwXTextTable::triggerDesktopTerminate() { mxDesktop->terminate(); }

Reference<XInterface> SwXTextTable::init()
{
    auto xComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(xComponent.is());
    Reference<text::XTextDocument> xTextDocument(xComponent, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(xComponent, UNO_QUERY_THROW);
    Reference<text::XText> xText = xTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    Reference<text::XTextTable> xTable(xMSF->createInstance("com.sun.star.text.TextTable"),
                                       UNO_QUERY_THROW);
    xTable->initialize(4, 3);
    xText->insertTextContent(xCursor, xTable, false);
    CPPUNIT_ASSERT(xCursor.is());
    return Reference<XInterface>(xTable, UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextTable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
