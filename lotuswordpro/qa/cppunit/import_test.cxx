/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;

class LotusWordProTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp() override;

    virtual void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }

private:
    uno::Reference<lang::XComponent> mxComponent;
};

void LotusWordProTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void LotusWordProTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

OUStringLiteral const DATA_DIRECTORY = u"/lotuswordpro/qa/cppunit/data/";

CPPUNIT_TEST_FIXTURE(LotusWordProTest, testTdf129993)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf129993.lwp";
    getComponent() = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(getComponent().is());

    uno::Reference<text::XTextDocument> textDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    for (int i = 0; i < 15; ++i)
        xParaEnum->nextElement();
    uno::Reference<lang::XServiceInfo> xServiceInfo(xParaEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_True, xServiceInfo->supportsService("com.sun.star.text.TextTable"));
}

CPPUNIT_TEST_FIXTURE(LotusWordProTest, paragraphProperties)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "paragraphProperties.lwp";
    getComponent() = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(getComponent().is());

    uno::Reference<text::XTextDocument> textDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    //Ignore the first paragraphs
    for (int i = 0; i < 4; ++i)
        xParaEnum->nextElement();

    // bold
    uno::Reference<text::XTextRange> const xParagraph(xParaEnum->nextElement(),
                                                      uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xParagraph, uno::UNO_QUERY);
    float fFontWeight;
    xPropertySet->getPropertyValue("CharWeight") >>= fFontWeight;
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, fFontWeight);

    // italic
    uno::Reference<text::XTextRange> const xParagraph2(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet2(xParagraph2, uno::UNO_QUERY);
    awt::FontSlant fFontSlant;
    xPropertySet2->getPropertyValue("CharPosture") >>= fFontSlant;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, fFontSlant);

    // underline
    uno::Reference<text::XTextRange> const xParagraph3(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet3(xParagraph3, uno::UNO_QUERY);
    sal_Int16 nCharUnderline;
    xPropertySet3->getPropertyValue("CharUnderline") >>= nCharUnderline;
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, nCharUnderline);

    // striked through
    uno::Reference<text::XTextRange> const xParagraph4(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet4(xParagraph4, uno::UNO_QUERY);
    sal_Int16 nFontStrikeout;
    xPropertySet4->getPropertyValue("CharStrikeout") >>= nFontStrikeout;
    CPPUNIT_ASSERT_EQUAL(awt::FontStrikeout::SINGLE, nFontStrikeout);

    //superscript
    uno::Reference<text::XTextRange> const xParagraph5(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet5(xParagraph5, uno::UNO_QUERY);
    sal_Int32 nCharEscapement;
    xPropertySet5->getPropertyValue("CharEscapement") >>= nCharEscapement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(33), nCharEscapement);

    sal_Int32 nCharEscapementHeight;
    xPropertySet5->getPropertyValue("CharEscapementHeight") >>= nCharEscapementHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), nCharEscapementHeight);

    //subscript
    uno::Reference<text::XTextRange> const xParagraph6(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet6(xParagraph6, uno::UNO_QUERY);
    sal_Int32 nCharEscapement2;
    xPropertySet6->getPropertyValue("CharEscapement") >>= nCharEscapement2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-33), nCharEscapement2);

    sal_Int32 nCharEscapementHeight2;
    xPropertySet6->getPropertyValue("CharEscapementHeight") >>= nCharEscapementHeight2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), nCharEscapementHeight2);

    // red background and borders
    uno::Reference<text::XTextRange> const xParagraph7(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet7(xParagraph7, uno::UNO_QUERY);
    Color nParaBackColor;
    xPropertySet7->getPropertyValue("ParaBackColor") >>= nParaBackColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xff0000), nParaBackColor);

    table::BorderLine2 aTopBorder;
    xPropertySet7->getPropertyValue("TopBorder") >>= aTopBorder;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(35), aTopBorder.OuterLineWidth);

    table::BorderLine2 aBottomBorder;
    xPropertySet7->getPropertyValue("BottomBorder") >>= aBottomBorder;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(35), aBottomBorder.OuterLineWidth);

    table::BorderLine2 aLeftBorder;
    xPropertySet7->getPropertyValue("LeftBorder") >>= aLeftBorder;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(35), aLeftBorder.OuterLineWidth);

    table::BorderLine2 aRightBorder;
    xPropertySet7->getPropertyValue("RightBorder") >>= aRightBorder;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(35), aRightBorder.OuterLineWidth);

    // centered
    uno::Reference<text::XTextRange> const xParagraph8(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet8(xParagraph8, uno::UNO_QUERY);
    sal_Int16 nParaAdjust = {}; // spurious -Werror=maybe-uninitialized
    xPropertySet8->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));

    // left margin
    uno::Reference<text::XTextRange> const xParagraph9(xParaEnum->nextElement(),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet9(xParagraph9, uno::UNO_QUERY);
    sal_Int32 nParaLeftMargin;
    xPropertySet9->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin);

    // double line spacing
    uno::Reference<text::XTextRange> const xParagraph10(xParaEnum->nextElement(),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet10(xParagraph10, uno::UNO_QUERY);
    style::LineSpacing nParaLineSpacing;
    xPropertySet10->getPropertyValue("ParaLineSpacing") >>= nParaLineSpacing;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200), nParaLineSpacing.Height);

    // time new roman 12
    uno::Reference<text::XTextRange> const xParagraph11(xParaEnum->nextElement(),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet11(xParagraph11, uno::UNO_QUERY);
    OUString sCharFontName;
    xPropertySet11->getPropertyValue("CharFontName") >>= sCharFontName;
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), sCharFontName);
    float fCharFontHeight;
    xPropertySet11->getPropertyValue("CharHeight") >>= fCharFontHeight;
    CPPUNIT_ASSERT_EQUAL(float(12.), fCharFontHeight);

    // time new roman 16
    uno::Reference<text::XTextRange> const xParagraph12(xParaEnum->nextElement(),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet12(xParagraph12, uno::UNO_QUERY);
    OUString sCharFontName2;
    xPropertySet12->getPropertyValue("CharFontName") >>= sCharFontName2;
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), sCharFontName2);
    float fCharFontHeight2;
    xPropertySet12->getPropertyValue("CharHeight") >>= fCharFontHeight2;
    CPPUNIT_ASSERT_EQUAL(float(16.), fCharFontHeight2);

    //ignore this paragraph
    xParaEnum->nextElement();

    //text color
    uno::Reference<text::XTextRange> const xParagraph13(xParaEnum->nextElement(),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet13(xParagraph13, uno::UNO_QUERY);
    Color nCharColor;
    xPropertySet13->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x00ff00), nCharColor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
