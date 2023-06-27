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
#include <test/text/xsimpletext.hxx>
#include <test/text/xtextrange.hxx>
#include <test/text/xtext.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXHeadFootTextText.
 */
class SwXHeadFootTextText final : public test::BootstrapFixture,
                                  public unotest::MacrosTest,
                                  public apitest::XElementAccess,
                                  public apitest::XSimpleText,
                                  public apitest::XTextRange,
                                  public apitest::XText,
                                  public apitest::XEnumerationAccess
{
public:
    SwXHeadFootTextText();
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;
    Reference<text::XTextContent> getTextContent() override { return mxTextContent; };

    CPPUNIT_TEST_SUITE(SwXHeadFootTextText);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertString);
    CPPUNIT_TEST(testInsertControlCharacter);
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testGetSetString);
    CPPUNIT_TEST(testGetStart);
    CPPUNIT_TEST(testGetText);
    // CPPUNIT_TEST(testInsertRemoveTextContent);
    CPPUNIT_TEST(testCreateEnumeration);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<lang::XComponent> component_;
    Reference<text::XTextContent> mxTextContent;
};

SwXHeadFootTextText::SwXHeadFootTextText()
    : XElementAccess(cppu::UnoType<text::XTextRange>::get())
{
}

void SwXHeadFootTextText::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SwXHeadFootTextText::tearDown()
{
    if (component_.is())
        component_->dispose();

    test::BootstrapFixture::tearDown();
}

Reference<XInterface> SwXHeadFootTextText::init()
{
    component_ = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    Reference<text::XTextDocument> xTextDocument(component_, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(component_, UNO_QUERY_THROW);

    Reference<style::XStyleFamiliesSupplier> xStyleFam(xTextDocument, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xStyleFamNames = xStyleFam->getStyleFamilies();

    Reference<container::XNameAccess> xPageStyles(xStyleFamNames->getByName("PageStyles"),
                                                  UNO_QUERY_THROW);
    Reference<style::XStyle> xStyle(xPageStyles->getByName("Standard"), UNO_QUERY_THROW);

    Reference<beans::XPropertySet> xPropSet(xStyle, UNO_QUERY_THROW);

    xPropSet->setPropertyValue("HeaderIsOn", uno::Any(true));
    xPropSet->setPropertyValue("FooterIsOn", uno::Any(true));
    Reference<text::XText> xText(xPropSet->getPropertyValue("HeaderText"), UNO_QUERY_THROW);

    return Reference<XInterface>(xText, UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXHeadFootTextText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
