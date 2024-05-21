/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xelementaccess.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTextSections.
 */
class SwXTextSections final : public UnoApiTest,
                              public apitest::XElementAccess,
                              public apitest::XIndexAccess,
                              public apitest::XNameAccess
{
public:
    SwXTextSections()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XTextSection>::get())
        , XIndexAccess(2)
        , XNameAccess(u"TextSection"_ustr)
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

        Reference<text::XTextContent> xContent(
            xMSF->createInstance(u"com.sun.star.text.TextSection"_ustr), UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xContent, false);

        Reference<text::XTextContent> xContent2(
            xMSF->createInstance(u"com.sun.star.text.TextSection"_ustr), UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xContent2, false);

        Reference<container::XNameAccess> xNameAccess;
        try
        {
            Reference<text::XTextSectionsSupplier> xSupplier(xTextDocument, UNO_QUERY_THROW);
            xNameAccess = xSupplier->getTextSections();
        }
        catch (Exception&)
        {
        }

        return Reference<XInterface>(xNameAccess, UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXTextSections);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextSections);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
