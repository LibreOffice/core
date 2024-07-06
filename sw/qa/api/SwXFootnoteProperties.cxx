/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xindexaccess.hxx>

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
class SwXFootnoteProperties final : public UnoApiTest,
                                    public apitest::XElementAccess,
                                    public apitest::XIndexAccess
{
public:
    SwXFootnoteProperties();

    Reference<XInterface> init() override;

    CPPUNIT_TEST_SUITE(SwXFootnoteProperties);
    CPPUNIT_TEST_SUITE_END();
};

SwXFootnoteProperties::SwXFootnoteProperties()
    : UnoApiTest(u""_ustr)
    , XElementAccess(cppu::UnoType<text::XFootnote>::get())
    , XIndexAccess(3)
{
}

Reference<XInterface> SwXFootnoteProperties::init()
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

    Reference<text::XFootnotesSupplier> xFootnotesSupplier(xTextDocument, UNO_QUERY_THROW);

    return Reference<XInterface>(xFootnotesSupplier->getFootnoteSettings(), UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXFootnoteProperties);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
