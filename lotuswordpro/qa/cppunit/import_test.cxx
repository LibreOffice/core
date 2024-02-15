/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

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

class LotusWordProTest : public UnoApiTest
{
public:
    LotusWordProTest()
        : UnoApiTest("/lotuswordpro/qa/cppunit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(LotusWordProTest, testtdf33787OrderedBullets)
{
    // Test if ordered bullets are incrementing correctly:
    //         A. Lettered bullet uppercase 1 (A.)
    //         B. Lettered bullet uppercase 2 (B.)
    //         C. Lettered bullet uppercase 3 (C.)
    // It is also make sure that nested and skipped bullets are working correctly:
    //         1) Numbered bullet w/ bracket one (1))
    //            Skipped bullet
    //         2) Numbered bullet w/ bracket two (2))
    //             a) Lettered bullet w/ bracket lowercase 1 (a))
    //             b) Lettered bullet w/ bracket lowercase 2 (b))
    loadFromFile(u"tdf33787-ordered-bullets.lwp");
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    OUString aListLabelString;
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Make sure the list start from 1.
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aListLabelString);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "1.".
    CPPUNIT_ASSERT_EQUAL(OUString("2."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "1.".
    CPPUNIT_ASSERT_EQUAL(OUString("3."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Make sure the list start from i.
    CPPUNIT_ASSERT_EQUAL(OUString("i."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "i.".
    CPPUNIT_ASSERT_EQUAL(OUString("ii."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "i.".
    CPPUNIT_ASSERT_EQUAL(OUString("iii."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Make sure the list start from I.
    CPPUNIT_ASSERT_EQUAL(OUString("I."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "I.".
    CPPUNIT_ASSERT_EQUAL(OUString("II."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "I.".
    CPPUNIT_ASSERT_EQUAL(OUString("III."), aListLabelString);

    for (int i = 0; i < 7; ++i)
        xParaEnum->nextElement();

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "A.".
    CPPUNIT_ASSERT_EQUAL(OUString("C."), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Make sure the nested list (with indentation) starts from 1).
    CPPUNIT_ASSERT_EQUAL(OUString("1)"), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Make sure skipped element has no prefix
    CPPUNIT_ASSERT_EQUAL(OUString(""), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Make sure the value after Skip is incremented properly
    // Without the accompanying fix in place, this test would have failed, the list label was "1)".
    CPPUNIT_ASSERT_EQUAL(OUString("2)"), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Next nested list
    CPPUNIT_ASSERT_EQUAL(OUString("a)"), aListLabelString);

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara->getPropertyValue("ListLabelString") >>= aListLabelString;
    // Without the accompanying fix in place, this test would have failed, the list label was "a)".
    CPPUNIT_ASSERT_EQUAL(OUString("b)"), aListLabelString);
}

CPPUNIT_TEST_FIXTURE(LotusWordProTest, testTdf129993)
{
    loadFromFile(u"tdf129993.lwp");
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
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
    loadFromFile(u"paragraphProperties.lwp");
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nParaBackColor);

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
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, nCharColor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
