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
 * Initial tests for SwXFootnoteProperties.
 */
class SwXFootnoteProperties final : public test::BootstrapFixture,
                                    public unotest::MacrosTest,
                                    public apitest::XElementAccess,
                                    public apitest::XIndexAccess
{
public:
    SwXFootnoteProperties();
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;

    CPPUNIT_TEST_SUITE(SwXFootnoteProperties);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> component_;
};

SwXFootnoteProperties::SwXFootnoteProperties()
    : XElementAccess(cppu::UnoType<text::XFootnote>::get())
    , XIndexAccess(3)
{
}

void SwXFootnoteProperties::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SwXFootnoteProperties::tearDown()
{
    if (component_.is())
        component_->dispose();

    test::BootstrapFixture::tearDown();
}

Reference<XInterface> SwXFootnoteProperties::init()
{
    component_
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    Reference<text::XTextDocument> xTextDocument(component_, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(component_, UNO_QUERY_THROW);

    Reference<text::XFootnote> xFootnote(xMSF->createInstance(u"com.sun.star.text.Footnote"_ustr),
                                         UNO_QUERY_THROW);

    Reference<text::XText> xText = xTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    xText->insertTextContent(xCursor, xFootnote, false);

    Reference<text::XFootnotesSupplier> xFootnotesSupplier(xTextDocument, UNO_QUERY_THROW);

    return Reference<XInterface>(xFootnotesSupplier->getFootnoteSettings(), UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXFootnoteProperties);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
