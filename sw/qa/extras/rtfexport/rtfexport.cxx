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
        : SwModelTestBase(u"/sw/qa/extras/rtfexport/data/"_ustr, u"Rich Text Format"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testZoom)
{
    auto verify = [this]() {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(
            xModel->getCurrentController(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
        sal_Int16 nValue = 0;
        xPropertySet->getPropertyValue(u"ZoomValue"_ustr) >>= nValue;
        CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
    };
    createSwDoc("zoom.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo38176)
{
    auto verify = [this]() { CPPUNIT_ASSERT_EQUAL(u"foo ‑­bar"_ustr, getBodyText()); };
    createSwDoc("fdo38176.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49683)
{
    auto verify = [this]() {
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xDocumentProperties(
            xDocumentPropertiesSupplier->getDocumentProperties());
        uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aKeywords.getLength());
        CPPUNIT_ASSERT_EQUAL(u"one"_ustr, aKeywords[0]);
        CPPUNIT_ASSERT_EQUAL(u"two"_ustr, aKeywords[1]);
    };
    createSwDoc("fdo49683.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

// TODO Use case not clear!
CPPUNIT_TEST_FIXTURE(Test, testFdo44174)
{
    auto verify = [this]() {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
            xModel->getCurrentController(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xTextViewCursorSupplier->getViewCursor(),
                                                         uno::UNO_QUERY);
        OUString aValue;
        xPropertySet->getPropertyValue(u"PageStyleName"_ustr) >>= aValue;
        CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, aValue);
    };
    createSwDoc("fdo44174.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo50087)
{
    auto verify = [this]() {
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xDocumentProperties(
            xDocumentPropertiesSupplier->getDocumentProperties());
        CPPUNIT_ASSERT_EQUAL(u"Title"_ustr, xDocumentProperties->getTitle());
        CPPUNIT_ASSERT_EQUAL(u"Subject"_ustr, xDocumentProperties->getSubject());
        CPPUNIT_ASSERT_EQUAL(u"First line.\nSecond line."_ustr,
                             xDocumentProperties->getDescription());
    };
    createSwDoc("fdo50087.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo50831)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
        float fValue = 0;
        xPropertySet->getPropertyValue(u"CharHeight"_ustr) >>= fValue;
        CPPUNIT_ASSERT_EQUAL(10.f, fValue);
    };
    createSwDoc("fdo50831.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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
    xPropertySet->getPropertyValue(u"TextPortionType"_ustr) >>= aValue;
    CPPUNIT_ASSERT_EQUAL(u"SoftPageBreak"_ustr, aValue);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo38244)
{
    auto verify = [this]() {
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
        CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                             getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
        xRunEnum->nextElement();
        xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                             getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));

        // Test initials.
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(
            xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"M"_ustr, getProperty<OUString>(xPropertySet, u"Initials"_ustr));
    };
    createSwDoc("fdo38244.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCommentsNested)
{
    loadAndReload("comments-nested.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xOuter = getProperty<uno::Reference<beans::XPropertySet>>(
        getRun(getParagraph(1), 2), u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Outer"_ustr, getProperty<OUString>(xOuter, u"Content"_ustr).trim());

    uno::Reference<beans::XPropertySet> xInner = getProperty<uno::Reference<beans::XPropertySet>>(
        getRun(getParagraph(1), 4), u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Inner"_ustr, getProperty<OUString>(xInner, u"Content"_ustr).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testMathAccents)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} "
                             "widetilde {a} widehat {a} dot {a} widevec {a} widevec {a} widetilde "
                             "{a} underline {a}"_ustr,
                             aActual);
    };
    createSwDoc("math-accents.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathEqarray)
{
    loadAndReload("math-eqarray.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(
        u"y = left lbrace stack { 0 , x < 0 # 1 , x = 0 # {x} ^ {2} , x > 0 } right none"_ustr,
        aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathD)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(
            u"left (x mline y mline z right ) left (1 right ) left [2 right ] left "
            "ldbracket 3 right rdbracket left lline 4 right rline left ldline 5 "
            "right rdline left langle 6 right rangle left langle a mline b right "
            "rangle left ({x} over {y} right )"_ustr,
            aActual);
    };
    createSwDoc("math-d.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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

CPPUNIT_TEST_FIXTURE(Test, testMathMatrix)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"left [matrix {1 # 2 ## 3 # 4} right ]"_ustr, aActual);
    };
    createSwDoc("math-matrix.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathBox)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, aActual);
    };
    createSwDoc("math-mbox.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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
    CPPUNIT_ASSERT_EQUAL(u"lllint from {1} to {2} {x + 1} prod from {a} {b} sum to {2} {x}"_ustr,
                         aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathLimupp)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"{abcd} overbrace {4}"_ustr, aActual);

        aActual = getFormula(getRun(getParagraph(2), 1));
        CPPUNIT_ASSERT_EQUAL(u"{xyz} underbrace {3}"_ustr, aActual);
    };
    createSwDoc("math-limupp.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathStrikeh)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"overstrike {abc}"_ustr, aActual);
    };
    createSwDoc("math-strikeh.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathPlaceholders)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"sum from <?> to <?> <?>"_ustr, aActual);
    };
    createSwDoc("math-placeholders.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathRad)
{
    loadAndReload("math-rad.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(u"sqrt {4} nroot {3} {x + 1}"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testMathSepchr)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"AxByBzC"_ustr, aActual);
    };
    createSwDoc("math-sepchr.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathSubscripts)
{
    auto verify = [this]() {
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(u"{x} ^ {y} + {e} ^ {x} {x} ^ {b} {x} rsub {b} {a} rsub {c} rsup {b} "
                             "{x} lsub {2} lsup {1} {{x csup {6} csub {3}} lsub {4} lsup {5}} rsub "
                             "{2} rsup {1}"_ustr,
                             aActual);
    };
    createSwDoc("math-subscripts.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathVerticalstacks)
{
    loadAndReload("math-vertical-stacks.rtf");
    CPPUNIT_ASSERT_EQUAL(u"{a} over {b}"_ustr, getFormula(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT_EQUAL(u"{a} / {b}"_ustr, getFormula(getRun(getParagraph(2), 1)));
    CPPUNIT_ASSERT_EQUAL(u"stack { a # b }"_ustr, getFormula(getRun(getParagraph(3), 1)));
    CPPUNIT_ASSERT_EQUAL(u"stack { a # stack { b # c } }"_ustr,
                         getFormula(getRun(getParagraph(4), 1)));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf49073)
{
    auto verify = [this]() {
        // test case for Asian phontic guide (ruby text.)
        sal_Unicode aRuby[3] = { 0x304D, 0x3082, 0x3093 };
        OUString sRuby(aRuby, SAL_N_ELEMENTS(aRuby));
        CPPUNIT_ASSERT_EQUAL(sRuby,
                             getProperty<OUString>(getParagraph(1)->getStart(), u"RubyText"_ustr));
        OUString sStyle
            = getProperty<OUString>(getParagraph(1)->getStart(), u"RubyCharStyleName"_ustr);
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"CharacterStyles"_ustr)->getByName(sStyle), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(5.f, getProperty<float>(xPropertySet, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(text::RubyAdjust_CENTER),
            getProperty<sal_Int16>(getParagraph(2)->getStart(), u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(text::RubyAdjust_BLOCK),
            getProperty<sal_Int16>(getParagraph(3)->getStart(), u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(text::RubyAdjust_INDENT_BLOCK),
            getProperty<sal_Int16>(getParagraph(4)->getStart(), u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(text::RubyAdjust_LEFT),
            getProperty<sal_Int16>(getParagraph(5)->getStart(), u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(text::RubyAdjust_RIGHT),
            getProperty<sal_Int16>(getParagraph(6)->getStart(), u"RubyAdjust"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(text::RubyPosition::INTER_CHARACTER),
            getProperty<sal_Int16>(getParagraph(7)->getStart(), u"RubyPosition"_ustr));
    };
    createSwDoc("tdf49073.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testMathRuns)
{
    loadAndReload("math-runs.rtf");
    // was [](){}, i.e. first curly bracket had an incorrect position
    CPPUNIT_ASSERT_EQUAL(u"\\{ left [ right ] \\( \\) \\}"_ustr,
                         getFormula(getRun(getParagraph(1), 1)));
}

// Saving left and right for parentheses when importing not from the m:t tag (rtf)
CPPUNIT_TEST_FIXTURE(Test, testTdf158023_rtf)
{
    loadAndReload("tdf158023.rtf");
    CPPUNIT_ASSERT_EQUAL(u"left [a right ] left (b right ) left lbrace c right rbrace"_ustr,
                         getFormula(getRun(getParagraph(1), 1)));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77979)
{
    loadAndReload("fdo77979.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // font name is encoded with \fcharset of font
    CPPUNIT_ASSERT_EQUAL(u"\u5FAE\u8F6F\u96C5\u9ED1"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 1), u"CharFontName"_ustr));
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
        = getProperty<uno::Sequence<beans::PropertyValue>>(getShape(1),
                                                           u"CustomShapeGeometry"_ustr);
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
    getRun(xParagraph, 1, u"Main text before footnote."_ustr);
    // Why the tab has to be removed here?
    CPPUNIT_ASSERT_EQUAL(u"Footnote text."_ustr, getProperty<uno::Reference<text::XTextRange>>(
                                                     getRun(xParagraph, 2), u"Footnote"_ustr)
                                                     ->getText()
                                                     ->getString()
                                                     .replaceAll("\t", ""));
    getRun(xParagraph, 3,
           u" Text after the footnote."_ustr); // However, this leading space is intentional and OK.
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(1), 2),
                                                               u"CharEscapementHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(2), 2),
                                                               u"CharEscapementHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo61507)
{
    auto verify = [this]() {
        /*
         * Unicode-only characters in \title confused Wordpad. Once the exporter
         * was fixed to guard the problematic characters with \upr and \ud, the
         * importer didn't cope with these new keywords.
         */

        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xDocumentProperties(
            xDocumentPropertiesSupplier->getDocumentProperties());
        CPPUNIT_ASSERT_EQUAL(u"\u00C9\u00C1\u0150\u0170\u222D"_ustr,
                             xDocumentProperties->getTitle());

        // Only "Hello.", no additional characters.
        CPPUNIT_ASSERT_EQUAL(u"Hello."_ustr, getBodyText());
    };
    createSwDoc("fdo61507.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo30983)
{
    auto verify = [this]() {
        // These were 'page text area', not 'entire page', i.e. both the horizontal
        // and vertical positions were incorrect.
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                             getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                             getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));
    };
    createSwDoc("fdo30983.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testPlaceholder)
{
    loadAndReload("placeholder.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Only the field text was exported, make sure we still have a field with the correct Hint text.
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 2));
    CPPUNIT_ASSERT_EQUAL(u"TextField"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
    uno::Reference<beans::XPropertySet> xField
        = getProperty<uno::Reference<beans::XPropertySet>>(xRun, u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"place holder"_ustr, getProperty<OUString>(xField, u"Hint"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testMnor)
{
    auto verify = [this]() {
        // \mnor wasn't handled, leading to missing quotes around "divF" and so on.
        OUString aActual = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT_EQUAL(
            u"iiint from {V} to <?> {\"divF\"} dV = llint from {S} to <?> {\"F\" \u2219 \"n\" dS}"_ustr,
            aActual);
    };
    createSwDoc("mnor.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testI120928)
{
    auto verify = [this]() {
        // \listpicture and \levelpicture0 was ignored, leading to missing graphic bullet in numbering.
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xLevels(
            xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
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
    };
    createSwDoc("i120928.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testBookmark)
{
    auto verify = [this]() {
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xBookmarksSupplier->getBookmarks()->getByName(u"firstword"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xBookmark->getAnchor()->getString());
    };
    createSwDoc("bookmark.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlink)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(
            OUString(),
            getProperty<OUString>(getRun(getParagraph(1), 1, u"Hello"_ustr), u"HyperLinkURL"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            u"http://en.wikipedia.org/wiki/World"_ustr,
            getProperty<OUString>(getRun(getParagraph(1), 2, u"world"_ustr), u"HyperLinkURL"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            OUString(),
            getProperty<OUString>(getRun(getParagraph(1), 3, u"!"_ustr), u"HyperLinkURL"_ustr));
    };
    createSwDoc("hyperlink.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkTdf100105)
{
    auto verify = [this]() {
        // export of empty link was invalid, group was closed before it was opened
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursor(xTextDoc->getText()->createTextCursor());
        xCursor->gotoStart(false);
        CPPUNIT_ASSERT_EQUAL(u"http://example.net"_ustr,
                             getProperty<OUString>(xCursor, u"HyperLinkURL"_ustr));
        // getRun doesn't provide a 0-length hyperlink
        CPPUNIT_ASSERT_EQUAL(OUString(),
                             getProperty<OUString>(getRun(getParagraph(1), 1, u"foobar"_ustr),
                                                   u"HyperLinkURL"_ustr));
    };
    createSwDoc("hyperlink_empty.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, test78758)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(
            u"#__RefHeading___Toc264438068"_ustr,
            getProperty<OUString>(getRun(getParagraph(2), 1, u"EE5E EeEEE5EE"_ustr),
                                  u"HyperLinkURL"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            u"#__RefHeading___Toc264438068"_ustr,
            getProperty<OUString>(getRun(getParagraph(2), 2, u"e"_ustr), u"HyperLinkURL"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            u"#__RefHeading___Toc264438068"_ustr,
            getProperty<OUString>(getRun(getParagraph(2), 3, u"\t46"_ustr), u"HyperLinkURL"_ustr));
    };
    createSwDoc("fdo78758.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTextFrameBorders)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0xD99594), getProperty<Color>(xFrame, u"BackColor"_ustr));

        table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xFrame, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(Color(0xC0504D), Color(ColorTransparency, aBorder.Color));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), aBorder.LineWidth);

        table::ShadowFormat aShadowFormat
            = getProperty<table::ShadowFormat>(xFrame, u"ShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadowFormat.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(48), aShadowFormat.ShadowWidth);
        CPPUNIT_ASSERT_EQUAL(Color(0x622423), Color(ColorTransparency, aShadowFormat.Color));
    };
    createSwDoc("textframe-borders.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTextframeGradient)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                             getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
        awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

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
                             getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
        aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(aColD, Color(ColorTransparency, aGradient.StartColor));
        CPPUNIT_ASSERT_EQUAL(aColC, Color(ColorTransparency, aGradient.EndColor));
    };
    createSwDoc("textframe-gradient.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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
                         getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

    const Color aColorGreen(0x127622); // green
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(aColorGreen, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(ColorTransparency, aGradient.EndColor));
}

CPPUNIT_TEST_FIXTURE(Test, testRecordChanges)
{
    auto verify = [this]() {
        // \revisions wasn't imported/exported.
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(mxComponent, u"RecordChanges"_ustr));
    };
    createSwDoc("record-changes.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTextframeTable)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextRange->getText();
        CPPUNIT_ASSERT_EQUAL(u"First para."_ustr, getParagraphOfText(1, xText)->getString());
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"A"_ustr, uno::Reference<text::XTextRange>(
                                            xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW)
                                            ->getString());
        CPPUNIT_ASSERT_EQUAL(u"B"_ustr, uno::Reference<text::XTextRange>(
                                            xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY_THROW)
                                            ->getString());
        CPPUNIT_ASSERT_EQUAL(u"Last para."_ustr, getParagraphOfText(3, xText)->getString());
    };
    createSwDoc("textframe-table.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo66682)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xLevels(
            xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aProps;
        xLevels->getByIndex(0) >>= aProps; // 1st level

        OUString aListFormat;
        for (beans::PropertyValue const& rProp : aProps)
        {
            if (rProp.Name == "ListFormat")
                aListFormat = rProp.Value.get<OUString>();
        }
        // Suffix was '\0' instead of ' '.
        CPPUNIT_ASSERT_EQUAL(u" %1% "_ustr, aListFormat);
    };
    createSwDoc("fdo66682.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testParaShadow)
{
    auto verify = [this]() {
        // The problem was that \brdrsh was ignored.
        table::ShadowFormat aShadow
            = getProperty<table::ShadowFormat>(getParagraph(2), u"ParaShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(60)), aShadow.ShadowWidth);
    };
    createSwDoc("para-shadow.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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
            = getProperty<table::BorderLine2>(xRun, u"CharTopBorder"_ustr);
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600, 0, 318, 0, 0, 318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, u"CharLeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(
            aTopBorder, getProperty<table::BorderLine2>(xRun, u"CharBottomBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, u"CharRightBorder"_ustr));
    }

    // Padding (brsp)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun, u"CharTopBorderDistance"_ustr);
        // In the original ODT file the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding,
                             getProperty<sal_Int32>(xRun, u"CharLeftBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(nTopPadding,
                             getProperty<sal_Int32>(xRun, u"CharBottomBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(nTopPadding,
                             getProperty<sal_Int32>(xRun, u"CharRightBorderDistance"_ustr));
    }

    // Shadow (brdrsh)
    /* RTF use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow
            = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFdo66743)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
        // This was too dark, 0x7f7f7f.
        CPPUNIT_ASSERT_EQUAL(Color(0xd8d8d8), getProperty<Color>(xCell, u"BackColor"_ustr));
    };
    createSwDoc("fdo66743.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo68787)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        // This was 0, the 'lack of \chftnsep' <-> '0 line width' mapping was missing in the RTF tokenizer / exporter.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(25),
                             getProperty<sal_Int32>(xPageStyle, u"FootnoteLineRelativeWidth"_ustr));
    };
    createSwDoc("fdo68787.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo74709)
{
    auto verify = [this]() {
        uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(1), u"B1"_ustr);
        // This was 0, as top/bottom/left/right padding wasn't imported.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(360)),
                             getProperty<sal_Int32>(xCell, u"RightBorderDistance"_ustr));
    };
    createSwDoc("fdo74709.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf84832)
{
    loadAndReload("tdf84832.docx");
    uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(2), u"A1"_ustr);
    // This was 0, as left padding wasn't exported.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(108)),
                         getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testRelsize)
{
    auto verify = [this]() {
        uno::Reference<drawing::XShape> xShape = getShape(1);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(40), getProperty<sal_Int16>(xShape, u"RelativeWidth"_ustr));
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                             getProperty<sal_Int16>(xShape, u"RelativeWidthRelation"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, u"RelativeHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME,
                             getProperty<sal_Int16>(xShape, u"RelativeHeightRelation"_ustr));
    };
    createSwDoc("relsize.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testLineNumbering)
{
    auto verify = [this]() {
        uno::Reference<text::XLineNumberingProperties> xLineNumberingProperties(
            mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropertySet
            = xLineNumberingProperties->getLineNumberingProperties();
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, u"IsOn"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), getProperty<sal_Int32>(xPropertySet, u"Interval"_ustr));
    };
    createSwDoc("linenumbering.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77600)
{
    auto verify = [this]() {
        // This was 'Liberation Serif'.
        CPPUNIT_ASSERT_EQUAL(
            u"Arial"_ustr, getProperty<OUString>(getRun(getParagraph(1), 3), u"CharFontName"_ustr));
    };
    createSwDoc("fdo77600.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo80167)
{
    auto verify = [this]() {
        // Problem was that after export, the page break was missing, so this was 1.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("fdo80167.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo32613)
{
    loadAndReload("fdo32613.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This was AS_CHARACTER, RTF export did not support writing anchored pictures.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
