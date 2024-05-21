/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/text/xtextrange.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/text/xtextrangecompare.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXBodyText.
 */
class SwXBodyText final : public test::BootstrapFixture,
                          public unotest::MacrosTest,
                          public apitest::XElementAccess,
                          public apitest::XTextRange,
                          public apitest::XSimpleText,
                          public apitest::XEnumerationAccess,
                          public apitest::XTextRangeCompare
{
public:
    SwXBodyText();
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;

    CPPUNIT_TEST_SUITE(SwXBodyText);
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertString);
    CPPUNIT_TEST(testInsertControlCharacter);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetText);
    CPPUNIT_TEST(testGetStart);
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testGetSetString);
    CPPUNIT_TEST(testCreateEnumeration);
    CPPUNIT_TEST(testCompareRegionStarts);
    CPPUNIT_TEST(testCompareRegionEnds);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> component_;
};

SwXBodyText::SwXBodyText()
    : XElementAccess(cppu::UnoType<text::XTextRange>::get())
{
}

void SwXBodyText::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SwXBodyText::tearDown()
{
    if (component_.is())
        component_->dispose();

    test::BootstrapFixture::tearDown();
}

Reference<XInterface> SwXBodyText::init()
{
    component_
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    Reference<text::XTextDocument> xTextDocument(component_, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(component_, UNO_QUERY_THROW);

    Reference<text::XText> xText = xTextDocument->getText();

    return Reference<XInterface>(xText, UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXBodyText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
