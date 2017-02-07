/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>

#include <vcl/svapp.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format") {}

    bool mustTestImportOf(const char* filename) const override
    {
        // Don't test the first import of these, for some reason those tests fail
        const char* aBlacklist[] =
        {
            "math-eqarray.rtf",
            "math-escaping.rtf",
            "math-lim.rtf",
            "math-mso2007.rtf",
            "math-nary.rtf",
            "math-rad.rtf",
            "math-vertical-stacks.rtf",
            "math-runs.rtf",
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".rtf") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }

    bool CjkNumberedListTestHelper(sal_Int16& rValue)
    {
        bool isNumber = false;
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
        properties->getPropertyValue("NumberingIsNumber") >>= isNumber;
        if (!isNumber)
            return false;
        uno::Reference<container::XIndexAccess> xLevels(properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        uno::Sequence< beans::PropertyValue > aPropertyValue;
        xLevels->getByIndex(0) >>= aPropertyValue;
        for (int j = 0; j < aPropertyValue.getLength(); ++j)
        {
            beans::PropertyValue aProp= aPropertyValue[j];
            if (aProp.Name == "NumberingType")
            {
                rValue = aProp.Value.get<sal_Int16>();
                return true;
            }
        }
        return false;

    }

    virtual void postLoad(const char* pFilename) override
    {
        if (OString(pFilename) == "tdf90421.fodt")
        {
            // Change the hyperlink, so its URL is empty.
            uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2), uno::UNO_QUERY);
            xRun->setPropertyValue("HyperLinkURL", uno::makeAny(OUString()));
        }
    }
};

DECLARE_RTFEXPORT_TEST(testZoom, "zoom.rtf")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_RTFEXPORT_TEST(testFdo38176, "fdo38176.rtf")
{
    CPPUNIT_ASSERT_EQUAL(9, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo49683, "fdo49683.rtf")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("one"), aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), aKeywords[1]);
}

DECLARE_RTFEXPORT_TEST(testFdo44174, "fdo44174.rtf")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aValue);
}

DECLARE_RTFEXPORT_TEST(testFdo50087, "fdo50087.rtf")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(OUString("Subject"), xDocumentProperties->getSubject());
    CPPUNIT_ASSERT_EQUAL(OUString("First line.\nSecond line."), xDocumentProperties->getDescription());
}

DECLARE_RTFEXPORT_TEST(testFdo50831, "fdo50831.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    float fValue = 0;
    xPropertySet->getPropertyValue("CharHeight") >>= fValue;
    CPPUNIT_ASSERT_EQUAL(10.f, fValue);
}

DECLARE_RTFEXPORT_TEST(testFdo48335, "fdo48335.odt")
{
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
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    for (int i = 0; i < 2; i++)
        xParaEnum->nextElement();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
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
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));
}

DECLARE_RTFEXPORT_TEST(testCommentsNested, "comments-nested.odt")
{
    uno::Reference<beans::XPropertySet> xOuter(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content").trim());

    uno::Reference<beans::XPropertySet> xInner(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content").trim());
}

DECLARE_RTFEXPORT_TEST(testMathAccents, "math-accents.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a} widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathEqarray, "math-eqarray.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("y = left lbrace stack { 0, x < 0 # 1, x = 0 # {x} ^ {2} , x > 0 } right none");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathD, "math-d.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("left (x mline y mline z right ) left (1 right ) left [2 right ] left ldbracket 3 right rdbracket left lline 4 right rline left ldline 5 right rdline left langle 6 right rangle left langle a mline b right rangle left ({x} over {y} right )");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathEscaping, "math-escaping.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("\xc3\xa1 \\{", 5, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathLim, "math-lim.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("lim from {x \xe2\x86\x92 1} {x}", 22, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathMatrix, "math-matrix.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("left [matrix {1 # 2 ## 3 # 4} right ]");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathBox, "math-mbox.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("a");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathMso2007, "math-mso2007.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("A = \xcf\x80 {r} ^ {2}", 16, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(2), 1));
    aExpected = OUString("{left (x + a right )} ^ {n} = sum from {k = 0} to {n} {left (stack { n # k } right ) {x} ^ {k} {a} ^ {n \xe2\x88\x92 k}}", 111, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(3), 1));
    aExpected = OUString("{left (1 + x right )} ^ {n} = 1 + {nx} over {1 !} + {n left (n \xe2\x88\x92 1 right ) {x} ^ {2}} over {2 !} + \xe2\x80\xa6", 104, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(4), 1));
    aExpected = OUString("f left (x right ) = {a} rsub {0} + sum from {n = 1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {n\xcf\x80x} over {L} + {b} rsub {n} sin {n\xcf\x80x} over {L} right )}", 144,
                         RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(5), 1));
    aExpected = "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}";
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(6), 1));
    aExpected = OUString("x = {\xe2\x88\x92 b \xc2\xb1 sqrt {{b} ^ {2} \xe2\x88\x92 4 ac}} over {2 a}", 51, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(7), 1));
    aExpected = OUString("{e} ^ {x} = 1 + {x} over {1 !} + {{x} ^ {2}} over {2 !} + {{x} ^ {3}} over {3 !} + \xe2\x80\xa6 , \xe2\x88\x92 \xe2\x88\x9e < x < \xe2\x88\x9e", 106, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(8), 1));
    aExpected = OUString("sin \xce\xb1 \xc2\xb1 sin \xce\xb2 = 2 sin {1} over {2} left (\xce\xb1 \xc2\xb1 \xce\xb2 right ) cos {1} over {2} left (\xce\xb1 \xe2\x88\x93 \xce\xb2 right )", 101, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(9), 1));
    aExpected = OUString("cos \xce\xb1 + cos \xce\xb2 = 2 cos {1} over {2} left (\xce\xb1 + \xce\xb2 right ) cos {1} over {2} left (\xce\xb1 \xe2\x88\x92 \xce\xb2 right )", 99, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathNary, "math-nary.rtf")
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("lllint from {1} to {2} {x + 1} prod from {a} {b} sum to {2} {x}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
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

DECLARE_RTFEXPORT_TEST(testMathRad, "math-rad.rtf")
{
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
    OUString aExpected("{x} ^ {y} + {e} ^ {x} {x} ^ {b} {x} rsub {b} {a} rsub {c} rsup {b} {x} lsub {2} lsup {1} {{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testMathVerticalstacks, "math-vertical-stacks.rtf")
{
    CPPUNIT_ASSERT_EQUAL(OUString("{a} over {b}"), getFormula(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("{a} / {b}"), getFormula(getRun(getParagraph(2), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # b }"), getFormula(getRun(getParagraph(3), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # stack { b # c } }"), getFormula(getRun(getParagraph(4), 1)));
}

DECLARE_RTFEXPORT_TEST(testTdf49073, "tdf49073.rtf")
{
    // test case for Asisan phontic guide ( ruby text.)
    sal_Unicode aRuby[3] = {0x304D,0x3082,0x3093};
    OUString sRuby = OUString(aRuby, SAL_N_ELEMENTS(aRuby));
    CPPUNIT_ASSERT_EQUAL(sRuby,getProperty<OUString>(getParagraph(1)->getStart(), "RubyText"));
    OUString sStyle = getProperty<OUString>(getParagraph(1)->getStart(), "RubyCharStyleName");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("CharacterStyles")->getByName(sStyle), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(5.f, getProperty<float>(xPropertySet, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_CENTER) ,getProperty<sal_Int16>(getParagraph(2)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_BLOCK)  ,getProperty<sal_Int16>(getParagraph(3)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_INDENT_BLOCK),getProperty<sal_Int16>(getParagraph(4)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_LEFT)   ,getProperty<sal_Int16>(getParagraph(5)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_RIGHT)  ,getProperty<sal_Int16>(getParagraph(6)->getStart(),"RubyAdjust"));
}

DECLARE_RTFEXPORT_TEST(testMathRuns, "math-runs.rtf")
{
    // was [](){}, i.e. first curly bracket had an incorrect position
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_RTFEXPORT_TEST(testFdo77979, "fdo77979.odt")
{
    // font name is encoded with \fcharset of font
    OUString aExpected("\xE5\xBE\xAE\xE8\xBD\xAF\xE9\x9B\x85\xE9\xBB\x91", 12, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo53113, "fdo53113.odt")
{
    /*
     * The problem was that a custom shape was missing its second (and all the other remaining) coordinates.
     *
     * oShape = ThisComponent.DrawPage(0)
     * oPathPropVec = oShape.CustomShapeGeometry(1).Value
     * oCoordinates = oPathPropVec(0).Value
     * xray oCoordinates(1).First.Value ' 535
     * xray oCoordinates(1).Second.Value ' 102
     */

    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            rProp.Value >>= aPairs;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aPairs.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(535), aPairs[1].First.Value.get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(102), aPairs[1].Second.Value.get<sal_Int32>());
}

DECLARE_RTFEXPORT_TEST(testFdo55939, "fdo55939.odt")
{
    // The problem was that the exported RTF was invalid.
    // Also, the 'Footnote text.' had an additional newline at its end.
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    getRun(xParagraph, 1, "Main text before footnote.");
    // Why the tab has to be removed here?
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote text."),
                         getProperty< uno::Reference<text::XTextRange> >(getRun(xParagraph, 2), "Footnote")->getText()->getString().replaceAll("\t", ""));
    getRun(xParagraph, 3, " Text after the footnote."); // However, this leading space is intentional and OK.
}

DECLARE_RTFEXPORT_TEST(testTextFrames, "textframes.odt")
{
    // The output was simply invalid, so let's check if all 3 frames were imported back.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo53604, "fdo53604.odt")
{
    // Invalid output on empty footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo52286, "fdo52286.odt")
{
    // The problem was that font size wasn't reduced in sub/super script.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(1), 2), "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(2), 2), "CharEscapementHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo61507, "fdo61507.rtf")
{
    /*
     * Unicode-only characters in \title confused Wordpad. Once the exporter
     * was fixed to guard the problematic characters with \upr and \ud, the
     * importer didn't cope with these new keywords.
     */

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    OUString aExpected = OUString("\xc3\x89\xc3\x81\xc5\x90\xc5\xb0\xe2\x88\xad", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xDocumentProperties->getTitle());

    // Only "Hello.", no additional characters.
    CPPUNIT_ASSERT_EQUAL(6, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo30983, "fdo30983.rtf")
{
    // These were 'page text area', not 'entire page', i.e. both the horizontal
    // and vertical positions were incorrect.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
}

DECLARE_RTFEXPORT_TEST(testPlaceholder, "placeholder.odt")
{
    // Only the field text was exported, make sure we still have a field with the correct Hint text.
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 2));
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xRun, "TextPortionType"));
    uno::Reference<beans::XPropertySet> xField = getProperty< uno::Reference<beans::XPropertySet> >(xRun, "TextField");
    CPPUNIT_ASSERT_EQUAL(OUString("place holder"), getProperty<OUString>(xField, "Hint"));
}

DECLARE_RTFEXPORT_TEST(testMnor, "mnor.rtf")
{
    // \mnor wasn't handled, leading to missing quotes around "divF" and so on.
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("iiint from {V} to <?> {\"divF\"} dV = llint from {S} to <?> {\"F\" \xe2\x88\x99 \"n\" dS}", 74, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testI120928, "i120928.rtf")
{
    // \listpicture and \levelpicture0 was ignored, leading to missing graphic bullet in numbering.
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bIsGraphic = false;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "NumberingType")
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, rProp.Value.get<sal_Int16>());
        else if (rProp.Name == "GraphicURL")
            bIsGraphic = true;
    }
    CPPUNIT_ASSERT_EQUAL(true, bIsGraphic);
}

DECLARE_RTFEXPORT_TEST(testBookmark, "bookmark.rtf")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xBookmark(xBookmarksSupplier->getBookmarks()->getByName("firstword"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xBookmark->getAnchor()->getString());
}

DECLARE_RTFEXPORT_TEST(testHyperlink, "hyperlink.rtf")
{
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(getRun(getParagraph(1), 1, "Hello"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("http://en.wikipedia.org/wiki/World"), getProperty<OUString>(getRun(getParagraph(1), 2, "world"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(getRun(getParagraph(1), 3, "!"), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(testHyperlinkTdf100105, "hyperlink_empty.rtf")
{
    // export of empty link was invalid, group was closed before it was opened
    uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor(xTextDoc->getText()->createTextCursor());
    xCursor->gotoStart(false);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.net"), getProperty<OUString>(xCursor, "HyperLinkURL"));
    // getRun doesn't provide a 0-length hyperlink
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(getRun(getParagraph(1), 1, "foobar"), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(test78758, "fdo78758.rtf")
{
    CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc264438068"),
                         getProperty<OUString>(getRun(getParagraph(2), 1, "EE5E EeEEE5EE"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc264438068"),
                         getProperty<OUString>(getRun(getParagraph(2), 2, "e"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc264438068"),
                         getProperty<OUString>(getRun(getParagraph(2), 3, "\t46"), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(testTextFrameBorders, "textframe-borders.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), getProperty<sal_Int32>(xFrame, "BackColor"));

    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xFrame, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), aBorder.Color);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), aBorder.LineWidth);

    table::ShadowFormat aShadowFormat = getProperty<table::ShadowFormat>(xFrame, "ShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadowFormat.Location);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(48), aShadowFormat.ShadowWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), aShadowFormat.Color);
}

DECLARE_RTFEXPORT_TEST(testTextframeGradient, "textframe-gradient.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient aGradient = getProperty<awt::Gradient>(xFrame, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);

    xFrame.set(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    aGradient = getProperty<awt::Gradient>(xFrame, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x666666), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
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
    CPPUNIT_ASSERT_EQUAL(OUString("A"), uno::Reference<text::XTextRange>(xTable->getCellByName("A1"), uno::UNO_QUERY)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), uno::Reference<text::XTextRange>(xTable->getCellByName("B1"), uno::UNO_QUERY)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Last para."), getParagraphOfText(3, xText)->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo66682, "fdo66682.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(" "), aSuffix);
}

DECLARE_RTFEXPORT_TEST(testParaShadow, "para-shadow.rtf")
{
    // The problem was that \brdrsh was ignored.
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getParagraph(2), "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(60)), aShadow.ShadowWidth);
}

DECLARE_RTFEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // RTF has just one border attribute (chbrdr) for text border so all side has
    // the same border with the same padding
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Padding (brsp)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun,"CharTopBorderDistance");
        // In the original ODT file the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // Shadow (brdrsh)
    /* RTF use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

DECLARE_RTFEXPORT_TEST(testFdo66743, "fdo66743.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    // This was too dark, 0x7f7f7f.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xd8d8d8), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo68787, "fdo68787.rtf")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 0, the 'lack of \chftnsep' <-> '0 line width' mapping was missing in the RTF tokenizer / exporter.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_RTFEXPORT_TEST(testFdo74709, "fdo74709.rtf")
{
    uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(1), "B1");
    // This was 0, as top/bottom/left/right padding wasn't imported.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(360)), getProperty<sal_Int32>(xCell, "RightBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testTdf84832, "tdf84832.docx")
{
    uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(2), "A1");
    // This was 0, as left padding wasn't exported.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(113)), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testRelsize, "relsize.rtf")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(40), getProperty<sal_Int16>(xShape, "RelativeWidth"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, "RelativeHeight"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));
}

DECLARE_RTFEXPORT_TEST(testLineNumbering, "linenumbering.rtf")
{
    uno::Reference<text::XLineNumberingProperties> xLineNumberingProperties(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet = xLineNumberingProperties->getLineNumberingProperties();
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xPropertySet, "IsOn")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), getProperty<sal_Int32>(xPropertySet, "Interval"));
}

DECLARE_RTFEXPORT_TEST(testFdo77600, "fdo77600.rtf")
{
    // This was 'Liberation Serif'.
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(getRun(getParagraph(1), 3), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo80167, "fdo80167.rtf")
{
    // Problem was that after export, the page break was missing, so this was 1.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo32613, "fdo32613.odt")
{
    // This was AS_CHARACTER, RTF export did not support writing anchored pictures.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testPictureWrapPolygon, "picture-wrap-polygon.rtf")
{
    // The problem was that the wrap polygon was ignored during import.
    drawing::PointSequenceSequence aSeqSeq = getProperty<drawing::PointSequenceSequence>(getShape(1), "ContourPolyPolygon");
    // This was 0: the polygon list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeqSeq.getLength());

    drawing::PointSequence aSeq = aSeqSeq[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aSeq.getLength());

    // The shape also didn't have negative top / left coordinates.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-1177)), getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-67)), getProperty<sal_Int32>(getShape(1), "VertOrientPosition"));
}

DECLARE_RTFEXPORT_TEST(testAbi10039, "abi10039.odt")
{
    // Make sure we don't just crash on export, and additionally the shape should not be inline (as it's at-page anchored originally).
    CPPUNIT_ASSERT(text::TextContentAnchorType_AS_CHARACTER != getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testAbi10076, "abi10076.odt")
{
    // Just make sure that we don't crash after exporting a fully calculated layout.
}

DECLARE_RTFEXPORT_TEST(testEm, "em.rtf")
{
    // Test all possible \acc* control words.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::NONE, getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::ACCENT_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 3), "CharEmphasis"));
    // This was missing.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::CIRCLE_ABOVE, getProperty<sal_Int16>(getRun(getParagraph(1), 4), "CharEmphasis"));
    // This one, too.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_BELOW, getProperty<sal_Int16>(getRun(getParagraph(1), 5), "CharEmphasis"));
}

DECLARE_RTFEXPORT_TEST(testNumberingFont, "numbering-font.rtf")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName("ListLabel 1"), uno::UNO_QUERY);
    // This was Liberation Serif, i.e. custom font of the numbering itself ("1.\t") was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("Verdana"), getProperty<OUString>(xStyle, "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo82860, "fdo82860.odt")
{
    // The problem was that:
    // 1) The import tried to use fieldmarks for SHAPE fields
    // 2) The exporter did not handle "shape with textbox" text.
    uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("hello"), getParagraphOfText(1, xText)->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo82858, "fdo82858.docx")
{
    // This was table::BorderLineStyle::SOLID, exporter failed to write explicit no line when line color was written.
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineStyle);
}

DECLARE_RTFEXPORT_TEST(testCjklist12, "cjklist12.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::AIU_HALFWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist13, "cjklist13.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::IROHA_HALFWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist16, "cjklist16.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_TRADITIONAL_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist20, "cjklist20.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::AIU_FULLWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist21, "cjklist21.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::IROHA_FULLWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist24, "cjklist24.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::HANGUL_SYLLABLE_KO, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist25, "cjklist25.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::HANGUL_JAMO_KO, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist30, "cjklist30.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist31, "cjklist31.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist34, "cjklist34.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist38, "cjklist38.rtf")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH, numFormat);
}

DECLARE_RTFEXPORT_TEST(testTableRtl, "table-rtl.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was text::WritingMode2::LR_TB, i.e. direction of the table was ignored.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, "WritingMode"));
}

DECLARE_RTFEXPORT_TEST(testNumOverrideStart, "num-override-start.rtf")
{
    // The numbering on the second level was "3.1", not "1.3".
    uno::Reference<container::XIndexAccess> xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), comphelper::SequenceAsHashMap(xRules->getByIndex(0))["StartWith"].get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), comphelper::SequenceAsHashMap(xRules->getByIndex(1))["StartWith"].get<sal_Int16>());
}

DECLARE_RTFEXPORT_TEST(testFdo82006, "fdo82006.rtf")
{
    // These were 176 (100 twips), as \sbauto and \sbbefore were ignored.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf88583, "tdf88583.odt")
{
    // This was FillStyle_NONE, as background color was missing from the color table during export.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x00cc00), getProperty<sal_Int32>(getParagraph(1), "FillColor"));
}

DECLARE_RTFEXPORT_TEST(testMargmirror, "margmirror.rtf")
{
    // \margmirror was not handled, this was PageStyleLayout_ALL.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(style::PageStyleLayout_MIRRORED, getProperty<style::PageStyleLayout>(xPageStyle, "PageStyleLayout"));
}

DECLARE_RTFEXPORT_TEST(testSautoupd, "sautoupd.rtf")
{
    // \sautoupd was ignored during import and export.
    uno::Reference<beans::XPropertySet> xHeading1(getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xHeading1, "IsAutoUpdate"));
    uno::Reference<beans::XPropertySet> xHeading2(getStyles("ParagraphStyles")->getByName("Heading 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xHeading2, "IsAutoUpdate"));
}

DECLARE_RTFEXPORT_TEST(testHyphauto, "hyphauto.rtf")
{
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getParagraph(1), "ParaIsHyphenation"));
}

DECLARE_RTFEXPORT_TEST(testHyphpar, "hyphpar.rtf")
{
    // Hyphenation was enabled for all 3 paragraphs, but it should be disabled for the 2nd one.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getParagraph(2), "ParaIsHyphenation"));
}

DECLARE_RTFEXPORT_TEST(testTdf80708, "tdf80708.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // This was 2, i.e. the second table had 3 cols, now 2 as expected.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFEXPORT_TEST(testTdf90421, "tdf90421.fodt")
{
    if (mbExported)
    {
        SvMemoryStream aMemoryStream;
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        aStream.ReadStream(aMemoryStream);
        OString aData(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
        // This was some positive number, i.e. we exported a hyperlink with an empty URL.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aData.indexOf("HYPERLINK"));
    }
}

DECLARE_RTFEXPORT_TEST(testTdf92521, "tdf92521.odt")
{
    // There should be a page break that's in the middle of the document: right after the table.
    // But there wasn't, so this was 1.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf94043, "tdf94043.rtf")
{
    auto xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    auto xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    // This was 0, the separator line was not visible due to 0 width.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), getProperty<sal_Int32>(xTextColumns, "SeparatorLineWidth"));
}

DECLARE_RTFEXPORT_TEST(testTdf94377, "tdf94377.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
    auto xText = getProperty< uno::Reference<text::XText> >(xPropertySet, "TextRange");
    // This failed, as:
    // 1) multiple paragraphs were not exported, so the text was "Asdf10asdf12".
    // 2) direct formatting of runs were not exported, so this was 12 (the document default).
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(getRun(getParagraphOfText(1, xText, "Asdf10"), 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getRun(getParagraphOfText(2, xText, "asdf12"), 1), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testPageBackground, "page-background.rtf")
{
    // The problem was that \background was ignored.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf96175, "tdf96175.rtf")
{
    // The problem that a user defined property named "Company" was lost on export.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertyContainer> xUserDefinedProperties = xDocumentProperties->getUserDefinedProperties();
    // This resulted in a beans::UnknownPropertyException.
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), getProperty<OUString>(xUserDefinedProperties, "Company"));
}

DECLARE_RTFEXPORT_TEST(testRedline, "redline.rtf")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Rebecca Lopez"), getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("Dorothy Jones"), getProperty<OUString>(getRun(getParagraph(2), 2), "RedlineAuthor"));
}

DECLARE_RTFEXPORT_TEST(testCustomDocProps, "custom-doc-props.rtf")
{
    // Custom document properties were not improved, this resulted in a beans::UnknownPropertyException.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertyContainer> xUserDefinedProperties = xDocumentProperties->getUserDefinedProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("2016-03-08T10:55:18,531376147"), getProperty<OUString>(xUserDefinedProperties, "urn:bails:IntellectualProperty:Authorization:StartValidity"));
    CPPUNIT_ASSERT_EQUAL(OUString("None"), getProperty<OUString>(xUserDefinedProperties, "urn:bails:IntellectualProperty:Authorization:StopValidity"));
    // Test roundtrip of numbers. This failed as getProperty() did not find "n".
    CPPUNIT_ASSERT_EQUAL(42.0, getProperty<double>(xUserDefinedProperties, "n"));
    // Test boolean "yes".
    CPPUNIT_ASSERT(getProperty<bool>(xUserDefinedProperties, "by"));
    // Test boolean "no".
    CPPUNIT_ASSERT(!getProperty<bool>(xUserDefinedProperties, "bn"));

    // Test roundtrip of date in general, and year/month/day in particular.
    util::DateTime aDate = getProperty<util::DateTime>(xUserDefinedProperties, "d");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2016), aDate.Year);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aDate.Month);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(30), aDate.Day);

    // Test real number.
    CPPUNIT_ASSERT_EQUAL(3.14, getProperty<double>(xUserDefinedProperties, "pi"));
}

DECLARE_RTFEXPORT_TEST(testTdf65642, "tdf65642.rtf")
{

    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHARS_UPPER_LETTER_N, getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
    // The second page's restart value: this was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), getProperty<sal_Int32>(getParagraph(2), "PageNumberOffset"));
}

DECLARE_RTFEXPORT_TEST(testPgnlcltr, "pgnlcltr.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHARS_LOWER_LETTER_N, getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testPgnucrm, "pgnucrm.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::ROMAN_UPPER, getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testPgnlcrm, "pgnlcrm.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::ROMAN_LOWER, getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testPgndec, "pgndec.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ROMAN_LOWER.
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::ARABIC, getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testTdf98806, "tdf98806.rtf")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xBookmark(xBookmarksSupplier->getBookmarks()->getByName("bookmark"), uno::UNO_QUERY);
    // This was empty, bookmark in table wasn't imported correctly.
    CPPUNIT_ASSERT_EQUAL(OUString("BBB"), xBookmark->getAnchor()->getString());
}

DECLARE_RTFEXPORT_TEST(testTdf61901, "tdf61901.rtf")
{
    // Test the file directly, as current RTF import gives the correct font name with and without the fix.
    if (mbExported)
    {
        SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
        OString sLine;
        while (pStream->ReadLine(sLine))
        {
            sal_Int32 nIndex = sLine.indexOf("\\loch\\loch");
            if (nIndex != -1)
            {
                // Make sure that \hich is always written after a \loch\loch.
                OString sRemaining = sLine.copy(nIndex);
                CPPUNIT_ASSERT(sRemaining.indexOf("\\hich") != -1);
            }
        }
    }
}

DECLARE_RTFEXPORT_TEST(testTdf103925, "tdf103925.rtf")
{
    // This was true, \animtext0 resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
}

DECLARE_RTFEXPORT_TEST(testTdf104228, "tdf104228.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("C1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCell->getText());
    // This was 2103, implicit 0 as direct formatting was ignored on the
    // paragraph (and the style had this larger value).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
}


DECLARE_RTFEXPORT_TEST(testTdf104085, "tdf104085.rtf")
{
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        if (aProps[i].Name == "BulletChar")
            return;
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_RTFEXPORT_TEST(testLeveljcCenter, "leveljc-center.rtf")
{
    // Tests that \leveljc1 is mapped to Adjust=Center for a numbering rule.
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        if (aProps[i].Name == "Adjust")
        {
            sal_Int16 nValue = 0;
            CPPUNIT_ASSERT(aProps[i].Value >>= nValue);
            CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, nValue);
            return;
        }
    }
    CPPUNIT_FAIL("no Adjust property");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
