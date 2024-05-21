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
#include <test/container/xnameaccess.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXFramesText.
 */
class SwXFramesText final : public UnoApiTest,
                            public apitest::XElementAccess,
                            public apitest::XIndexAccess,
                            public apitest::XNameAccess
{
public:
    SwXFramesText()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XTextFrame>::get())
        , XIndexAccess(1)
        , XNameAccess(u"Frame1"_ustr)
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

        Reference<text::XTextFrame> xTextFrame(
            xMSF->createInstance(u"com.sun.star.text.TextFrame"_ustr), UNO_QUERY_THROW);

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        xText->insertTextContent(xCursor, xTextFrame, false);

        Reference<text::XTextFramesSupplier> xSupplier(xTextDocument, UNO_QUERY_THROW);

        // Reference<container::XNameAccess> xNA = xSupplier->getTextFrames();
        // Sequence<OUString> aNames = xNA->getElementNames();
        // std::cout << aNames[0] << std::endl;

        return Reference<XInterface>(xSupplier->getTextFrames(), UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXFramesText);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXFramesText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
