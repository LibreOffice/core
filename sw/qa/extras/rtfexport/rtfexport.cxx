/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>

#include <tools/UnitConversion.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testZoom, "zoom.rtf")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_RTFEXPORT_TEST(testFdo38176, "fdo38176.rtf")
{
    CPPUNIT_ASSERT_EQUAL(u"foo ‑­bar"_ustr, getBodyText());
}

DECLARE_RTFEXPORT_TEST(testFdo49683, "fdo49683.rtf")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(
        xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("one"), aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), aKeywords[1]);
}

// TODO Use case not clear!
DECLARE_RTFEXPORT_TEST(testFdo44174, "fdo44174.rtf")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xTextViewCursorSupplier->getViewCursor(),
                                                     uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), aValue);
}

DECLARE_RTFEXPORT_TEST(testFdo50087, "fdo50087.rtf")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(
        xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(OUString("Subject"), xDocumentProperties->getSubject());
    CPPUNIT_ASSERT_EQUAL(OUString("First line.\nSecond line."),
                         xDocumentProperties->getDescription());
}

DECLARE_RTFEXPORT_TEST(testFdo50831, "fdo50831.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    float fValue = 0;
    xPropertySet->getPropertyValue("CharHeight") >>= fValue;
    CPPUNIT_ASSERT_EQUAL(10.f, fValue);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48335)
{
    loadAndReload("fdo48335.odt");
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    /*
     * The problem was that we exported a fake pagebreak, make sure it's just a soft one now.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.TextPortionType 'was Text, should be SoftPageBreak
     */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    for (int i = 0; i < 2; i++)
        xParaEnum->nextElement();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("TextPortionType") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("SoftPageBreak"), aValue);
}

DECLARE_RTFEXPORT_TEST(testFdo38244, "fdo38244.rtf")
{
    // See ooxmlexport's testFdo38244().
    // Test comment range feature.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(xPropertySet, "TextPortionType"));

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));
}

CPPUNIT_TEST_FIXTURE(Test, testCommentsNested)
{
    loadAndReload("comments-nested.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xOuter
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content").trim());

    uno::Reference<beans::XPropertySet> xInner
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 4), "TextField");
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content").trim());
}

DECLARE_RTFEXPORT_TEST(testMathAccents, "math-accents.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        OUString("acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} "
                 "widetilde {a} widehat {a} dot {a} widevec {a} widevec {a} widetilde "
                 "{a} underline {a}"),
        aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathEqarray)
{
    loadAndReload("math-eqarray.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        OUString("y = left lbrace stack { 0 , x < 0 # 1 , x = 0 # {x} ^ {2} , x > 0 } right none"),
        aActual);
}

DECLARE_RTFEXPORT_TEST(testMathD, "math-d.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        OUString("left (x mline y mline z right ) left (1 right ) left [2 right ] left "
                 "ldbracket 3 right rdbracket left lline 4 right rline left ldline 5 "
                 "right rdline left langle 6 right rangle left langle a mline b right "
                 "rangle left ({x} over {y} right )"),
        aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathEscaping)
{
    loadAndReload("math-escaping.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(u"\u00E1 \\{"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathLim)
{
    loadAndReload("math-lim.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(u"lim from {x \u2192 1} {x}"_ustr, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathMatrix, "math-matrix.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("left [matrix {1 # 2 ## 3 # 4} right ]"), aActual);
}

DECLARE_RTFEXPORT_TEST(testMathBox, "math-mbox.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathMso2007)
{
    loadAndReload("math-mso2007.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected(u"A = \u03C0 {r} ^ {2}"_ustr);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(2), 1));
    aExpected = u"{left (x + a right )} ^ {n} = sum from {k = 0} to {n} {left (stack { n "
                u"# k } right ) {x} ^ {k} {a} ^ {n \u2212 k}}"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(3), 1));
    aExpected = u"{left (1 + x right )} ^ {n} = 1 + {nx} over {1 !} + {n left (n \u2212 1 "
                u"right ) {x} ^ {2}} over {2 !} + \u2026"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(4), 1));
    aExpected = u"f left (x right ) = {a} rsub {0} + sum from {n = 1} to {\u221E} {left "
                u"({a} rsub {n} cos {n\u03C0x} over {L} + {b} rsub {n} sin {n\u03C0x} "
                u"over {L} right )}"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(5), 1));
    aExpected = "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}";
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(6), 1));
    aExpected = u"x = {\u2212 b \u00B1 sqrt {{b} ^ {2} \u2212 4 ac}} over {2 a}"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(7), 1));
    aExpected = u"{e} ^ {x} = 1 + {x} over {1 !} + {{x} ^ {2}} over {2 !} + {{x} ^ {3}} "
                u"over {3 !} + \u2026 , \u2212 \u221E < x < \u221E"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(8), 1));
    aExpected = u"sin \u03B1 \u00B1 sin \u03B2 = 2 sin {1} over {2} left (\u03B1 \u00B1 "
                u"\u03B2 right ) cos {1} over {2} left (\u03B1 \u2213 \u03B2 right )"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(9), 1));
    aExpected = u"cos \u03B1 + cos \u03B2 = 2 cos {1} over {2} left (\u03B1 + \u03B2 "
                u"right ) cos {1} over {2} left (\u03B1 \u2212 \u03B2 right )"_ustr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathNary)
{
    loadAndReload("math-nary.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        OUString("lllint from {1} to {2} {x + 1} prod from {a} {b} sum to {2} {x}"), aActual);
}

DECLARE_RTFEXPORT_TEST(testMathLimupp, "math-limupp.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("{abcd} overbrace {4}"), aActual);

    aActual = getFormula(getRun(getParagraph(2), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("{xyz} underbrace {3}"), aActual);
}

DECLARE_RTFEXPORT_TEST(testMathStrikeh, "math-strikeh.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("overstrike {abc}"), aActual);
}

DECLARE_RTFEXPORT_TEST(testMathPlaceholders, "math-placeholders.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("sum from <?> to <?> <?>"), aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathRad)
{
    loadAndReload("math-rad.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("sqrt {4} nroot {3} {x + 1}"), aActual);
}

DECLARE_RTFEXPORT_TEST(testMathSepchr, "math-sepchr.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("AxByBzC"), aActual);
}

DECLARE_RTFEXPORT_TEST(testMathSubscripts, "math-subscripts.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        OUString("{x} ^ {y} + {e} ^ {x} {x} ^ {b} {x} rsub {b} {a} rsub {c} rsup {b} "
                 "{x} lsub {2} lsup {1} {{x csup {6} csub {3}} lsub {4} lsup {5}} rsub "
                 "{2} rsup {1}"),
        aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathVerticalstacks)
{
    loadAndReload("math-vertical-stacks.rtf");
    CPPUNIT_ASSERT_EQUAL(OUString("{a} over {b}"), getFormula(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("{a} / {b}"), getFormula(getRun(getParagraph(2), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # b }"), getFormula(getRun(getParagraph(3), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # stack { b # c } }"),
                         getFormula(getRun(getParagraph(4), 1)));
}

DECLARE_RTFEXPORT_TEST(testTdf49073, "tdf49073.rtf")
{
    // test case for Asian phontic guide (ruby text.)
    sal_Unicode aRuby[3] = { 0x304D, 0x3082, 0x3093 };
    OUString sRuby(aRuby, SAL_N_ELEMENTS(aRuby));
    CPPUNIT_ASSERT_EQUAL(sRuby, getProperty<OUString>(getParagraph(1)->getStart(), "RubyText"));
    OUString sStyle = getProperty<OUString>(getParagraph(1)->getStart(), "RubyCharStyleName");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("CharacterStyles")->getByName(sStyle), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(5.f, getProperty<float>(xPropertySet, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_CENTER),
                         getProperty<sal_Int16>(getParagraph(2)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_BLOCK),
                         getProperty<sal_Int16>(getParagraph(3)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_INDENT_BLOCK),
                         getProperty<sal_Int16>(getParagraph(4)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_LEFT),
                         getProperty<sal_Int16>(getParagraph(5)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_RIGHT),
                         getProperty<sal_Int16>(getParagraph(6)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyPosition::INTER_CHARACTER),
                         getProperty<sal_Int16>(getParagraph(7)->getStart(), "RubyPosition"));
}

CPPUNIT_TEST_FIXTURE(Test, testMathRuns)
{
    loadAndReload("math-runs.rtf");
    // was [](){}, i.e. first curly bracket had an incorrect position
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] \\( \\) \\}"),
                         getFormula(getRun(getParagraph(1), 1)));
}

// Saving left and right for parentheses when importing not from the m:t tag (rtf)
CPPUNIT_TEST_FIXTURE(Test, testTdf158023_rtf)
{
    loadAndReload("tdf158023.rtf");
    CPPUNIT_ASSERT_EQUAL(OUString("left [a right ] left (b right ) left lbrace c right rbrace"),
                         getFormula(getRun(getParagraph(1), 1)));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77979)
{
    loadAndReload("fdo77979.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // font name is encoded with \fcharset of font
    CPPUNIT_ASSERT_EQUAL(u"\u5FAE\u8F6F\u96C5\u9ED1"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo53113)
{
    loadAndReload("fdo53113.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    /*
     * The problem was that a custom shape was missing its second (and all the other remaining) coordinates.
     *
     * oShape = ThisComponent.DrawPage(0)
     * oPathPropVec = oShape.CustomShapeGeometry(1).Value
     * oCoordinates = oPathPropVec(0).Value
     * xray oCoordinates(1).First.Value ' 535
     * xray oCoordinates(1).Second.Value ' 102
     */

    const uno::Sequence<beans::PropertyValue> aProps
        = getProperty<uno::Sequence<beans::PropertyValue>>(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "Coordinates")
            rProp.Value >>= aPairs;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aPairs.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(535), aPairs[1].First.Value.get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(102), aPairs[1].Second.Value.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo55939)
{
    loadAndReload("fdo55939.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that the exported RTF was invalid.
    // Also, the 'Footnote text.' had an additional newline at its end.
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    getRun(xParagraph, 1, "Main text before footnote.");
    // Why the tab has to be removed here?
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote text."), getProperty<uno::Reference<text::XTextRange>>(
                                                         getRun(xParagraph, 2), "Footnote")
                                                         ->getText()
                                                         ->getString()
                                                         .replaceAll("\t", ""));
    getRun(xParagraph, 3,
           " Text after the footnote."); // However, this leading space is intentional and OK.
}

CPPUNIT_TEST_FIXTURE(Test, testTextFrames)
{
    loadAndReload("textframes.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The output was simply invalid, so let's check if all 3 frames were imported back.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo53604)
{
    loadAndReload("fdo53604.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Invalid output on empty footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52286)
{
    loadAndReload("fdo52286.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that font size wasn't reduced in sub/super script.
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(1), 2), "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(2), 2), "CharEscapementHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo61507, "fdo61507.rtf")
{
    /*
     * Unicode-only characters in \title confused Wordpad. Once the exporter
     * was fixed to guard the problematic characters with \upr and \ud, the
     * importer didn't cope with these new keywords.
     */

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(
        xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(u"\u00C9\u00C1\u0150\u0170\u222D"_ustr, xDocumentProperties->getTitle());

    // Only "Hello.", no additional characters.
    CPPUNIT_ASSERT_EQUAL(OUString("Hello."), getBodyText());
}

DECLARE_RTFEXPORT_TEST(testFdo30983, "fdo30983.rtf")
{
    // These were 'page text area', not 'entire page', i.e. both the horizontal
    // and vertical positions were incorrect.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(Test, testPlaceholder)
{
    loadAndReload("placeholder.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Only the field text was exported, make sure we still have a field with the correct Hint text.
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 2));
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xRun, "TextPortionType"));
    uno::Reference<beans::XPropertySet> xField
        = getProperty<uno::Reference<beans::XPropertySet>>(xRun, "TextField");
    CPPUNIT_ASSERT_EQUAL(OUString("place holder"), getProperty<OUString>(xField, "Hint"));
}

DECLARE_RTFEXPORT_TEST(testMnor, "mnor.rtf")
{
    // \mnor wasn't handled, leading to missing quotes around "divF" and so on.
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        u"iiint from {V} to <?> {\"divF\"} dV = llint from {S} to <?> {\"F\" \u2219 \"n\" dS}"_ustr,
        aActual);
}

DECLARE_RTFEXPORT_TEST(testI120928, "i120928.rtf")
{
    // \listpicture and \levelpicture0 was ignored, leading to missing graphic bullet in numbering.
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "NumberingType")
            nNumberingType = rProp.Value.get<sal_Int16>();
        else if (rProp.Name == "GraphicBitmap")
            xBitmap = rProp.Value.get<uno::Reference<awt::XBitmap>>();
    }
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);
    CPPUNIT_ASSERT(xBitmap.is());
}

DECLARE_RTFEXPORT_TEST(testBookmark, "bookmark.rtf")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xBookmark(
        xBookmarksSupplier->getBookmarks()->getByName("firstword"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xBookmark->getAnchor()->getString());
}

DECLARE_RTFEXPORT_TEST(testHyperlink, "hyperlink.rtf")
{
    CPPUNIT_ASSERT_EQUAL(
        OUString(), getProperty<OUString>(getRun(getParagraph(1), 1, "Hello"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("http://en.wikipedia.org/wiki/World"),
        getProperty<OUString>(getRun(getParagraph(1), 2, "world"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString(),
                         getProperty<OUString>(getRun(getParagraph(1), 3, "!"), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(testHyperlinkTdf100105, "hyperlink_empty.rtf")
{
    // export of empty link was invalid, group was closed before it was opened
    uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor(xTextDoc->getText()->createTextCursor());
    xCursor->gotoStart(false);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.net"),
                         getProperty<OUString>(xCursor, "HyperLinkURL"));
    // getRun doesn't provide a 0-length hyperlink
    CPPUNIT_ASSERT_EQUAL(
        OUString(), getProperty<OUString>(getRun(getParagraph(1), 1, "foobar"), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(test78758, "fdo78758.rtf")
{
    CPPUNIT_ASSERT_EQUAL(
        OUString("#__RefHeading___Toc264438068"),
        getProperty<OUString>(getRun(getParagraph(2), 1, "EE5E EeEEE5EE"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc264438068"),
                         getProperty<OUString>(getRun(getParagraph(2), 2, "e"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc264438068"),
                         getProperty<OUString>(getRun(getParagraph(2), 3, "\t46"), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(testTextFrameBorders, "textframe-borders.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xD99594), getProperty<Color>(xFrame, "BackColor"));

    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xFrame, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(Color(0xC0504D), Color(ColorTransparency, aBorder.Color));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), aBorder.LineWidth);

    table::ShadowFormat aShadowFormat = getProperty<table::ShadowFormat>(xFrame, "ShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadowFormat.Location);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(48), aShadowFormat.ShadowWidth);
    CPPUNIT_ASSERT_EQUAL(Color(0x622423), Color(ColorTransparency, aShadowFormat.Color));
}

DECLARE_RTFEXPORT_TEST(testTextframeGradient, "textframe-gradient.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    // prepare compare colors
    const Color aColA(0xd99594);
    const Color aColB(0xc0504d);
    const Color aColC(0x666666);
    const Color aColD(COL_BLACK);

    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(aColB, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(aColA, Color(ColorTransparency, aGradient.EndColor));

    xFrame.set(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(aColD, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(aColC, Color(ColorTransparency, aGradient.EndColor));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159824_axialGradient)
{
    // given a frame with an axial gradient (white - green - white)
    loadAndReload("tdf159824_axialGradient.odt");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    const Color aColorGreen(0x127622); // green
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(aColorGreen, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(ColorTransparency, aGradient.EndColor));
}

DECLARE_RTFEXPORT_TEST(testRecordChanges, "record-changes.rtf")
{
    // \revisions wasn't imported/exported.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(mxComponent, "RecordChanges"));
}

DECLARE_RTFEXPORT_TEST(testTextframeTable, "textframe-table.rtf")
{
    uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("First para."), getParagraphOfText(1, xText)->getString());
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A"), uno::Reference<text::XTextRange>(
                                            xTable->getCellByName("A1"), uno::UNO_QUERY_THROW)
                                            ->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), uno::Reference<text::XTextRange>(
                                            xTable->getCellByName("B1"), uno::UNO_QUERY_THROW)
                                            ->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Last para."), getParagraphOfText(3, xText)->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo66682, "fdo66682.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aListFormat;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "ListFormat")
            aListFormat = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(" %1% "), aListFormat);
}

DECLARE_RTFEXPORT_TEST(testParaShadow, "para-shadow.rtf")
{
    // The problem was that \brdrsh was ignored.
    table::ShadowFormat aShadow
        = getProperty<table::ShadowFormat>(getParagraph(2), "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(60)), aShadow.ShadowWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testCharacterBorder)
{
    loadAndReload("charborder.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    // RTF has just one border attribute (chbrdr) for text border so all side has
    // the same border with the same padding
    // Border
    {
        const table::BorderLine2 aTopBorder
            = getProperty<table::BorderLine2>(xRun, "CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600, 0, 318, 0, 0, 318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, "CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, "CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, "CharRightBorder"));
    }

    // Padding (brsp)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun, "CharTopBorderDistance");
        // In the original ODT file the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun, "CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun, "CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun, "CharRightBorderDistance"));
    }

    // Shadow (brdrsh)
    /* RTF use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow
            = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

DECLARE_RTFEXPORT_TEST(testFdo66743, "fdo66743.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    // This was too dark, 0x7f7f7f.
    CPPUNIT_ASSERT_EQUAL(Color(0xd8d8d8), getProperty<Color>(xCell, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo68787, "fdo68787.rtf")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // This was 0, the 'lack of \chftnsep' <-> '0 line width' mapping was missing in the RTF tokenizer / exporter.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25),
                         getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_RTFEXPORT_TEST(testFdo74709, "fdo74709.rtf")
{
    uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(1), "B1");
    // This was 0, as top/bottom/left/right padding wasn't imported.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(360)),
                         getProperty<sal_Int32>(xCell, "RightBorderDistance"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf84832)
{
    loadAndReload("tdf84832.docx");
    uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(2), "A1");
    // This was 0, as left padding wasn't exported.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(108)),
                         getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testRelsize, "relsize.rtf")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(40), getProperty<sal_Int16>(xShape, "RelativeWidth"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, "RelativeHeight"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));
}

DECLARE_RTFEXPORT_TEST(testLineNumbering, "linenumbering.rtf")
{
    uno::Reference<text::XLineNumberingProperties> xLineNumberingProperties(mxComponent,
                                                                            uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet
        = xLineNumberingProperties->getLineNumberingProperties();
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, "IsOn"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), getProperty<sal_Int32>(xPropertySet, "Interval"));
}

DECLARE_RTFEXPORT_TEST(testFdo77600, "fdo77600.rtf")
{
    // This was 'Liberation Serif'.
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"),
                         getProperty<OUString>(getRun(getParagraph(1), 3), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo80167, "fdo80167.rtf")
{
    // Problem was that after export, the page break was missing, so this was 1.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo32613)
{
    loadAndReload("fdo32613.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This was AS_CHARACTER, RTF export did not support writing anchored pictures.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
