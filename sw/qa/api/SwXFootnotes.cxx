/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXFootnotesText.
 */
class SwXFootnotesText final : public test::BootstrapFixture,
                               public unotest::MacrosTest,
                               public apitest::XElementAccess,
                               public apitest::XIndexAccess
{
public:
    SwXFootnotesText();
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;

    CPPUNIT_TEST_SUITE(SwXFootnotesText);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<lang::XComponent> component_;
};

SwXFootnotesText::SwXFootnotesText()
    : XElementAccess(cppu::UnoType<text::XFootnote>::get())
    , XIndexAccess(1)
{
}

void SwXFootnotesText::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SwXFootnotesText::tearDown()
{
    if (component_.is())
        component_->dispose();

    test::BootstrapFixture::tearDown();
}

Reference<XInterface> SwXFootnotesText::init()
{
    component_ = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    Reference<text::XTextDocument> xTextDocument(component_, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(component_, UNO_QUERY_THROW);

    Reference<text::XFootnote> xFootnote(xMSF->createInstance("com.sun.star.text.Footnote"),
                                         UNO_QUERY_THROW);

    Reference<text::XText> xText = xTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    xText->insertTextContent(xCursor, xFootnote, false);

    Reference<text::XFootnotesSupplier> xSupplier(xTextDocument, UNO_QUERY_THROW);

    return Reference<XInterface>(xSupplier->getFootnotes(), UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXFootnotesText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
