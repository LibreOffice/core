/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#include <vcl/svapp.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format") {}

    bool mustTestImportOf(const char* filename) const {
        
        const char* aBlacklist[] = {
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

        
        return (OString(filename).endsWith(".rtf") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }
};

#define DECLARE_RTFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

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

    
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));
}

DECLARE_RTFEXPORT_TEST(testCommentsNested, "comments-nested.odt")
{
    uno::Reference<beans::XPropertySet> xOuter(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content"));

    uno::Reference<beans::XPropertySet> xInner(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content"));
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

DECLARE_RTFEXPORT_TEST(testMathRuns, "math-runs.rtf")
{
    
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_RTFEXPORT_TEST(testFdo53113, "fdo53113.odt")
{
    /*
     * The problem was that a custom shape was missings its second (and all the other remaining) coordinates.
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
    
    
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    getRun(xParagraph, 1, "Main text before footnote.");
    
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote text."),
            getProperty< uno::Reference<text::XTextRange> >(getRun(xParagraph, 2), "Footnote")->getText()->getString().replaceAll("\t", ""));
    getRun(xParagraph, 3, " Text after the footnote."); 
}

DECLARE_RTFEXPORT_TEST(testTextFrames, "textframes.odt")
{
    
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo53604, "fdo53604.odt")
{
    
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo52286, "fdo52286.odt")
{
    
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

    
    CPPUNIT_ASSERT_EQUAL(6, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo30983, "fdo30983.rtf")
{
    
    
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
}

DECLARE_RTFEXPORT_TEST(testPlaceholder, "placeholder.odt")
{
    
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 2));
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xRun, "TextPortionType"));
    uno::Reference<beans::XPropertySet> xField = getProperty< uno::Reference<beans::XPropertySet> >(xRun, "TextField");
    CPPUNIT_ASSERT_EQUAL(OUString("place holder"), getProperty<OUString>(xField, "Hint"));
}

DECLARE_RTFEXPORT_TEST(testMnor, "mnor.rtf")
{
    
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("iiint from {V} to <?> {\"divF\"} dV = llint from {S} to <?> {\"F\" \xe2\x88\x99 \"n\" dS}", 74, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testI120928, "i120928.rtf")
{
    
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; 

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
    CPPUNIT_ASSERT_EQUAL(OUString("http:
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(getRun(getParagraph(1), 3, "!"), "HyperLinkURL"));
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
    xLevels->getByIndex(0) >>= aProps; 

    OUString aSuffix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
    }
    
    CPPUNIT_ASSERT_EQUAL(OUString(" "), aSuffix);
}

DECLARE_RTFEXPORT_TEST(testParaShadow, "para-shadow.rtf")
{
    
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getParagraph(2), "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(TWIP_TO_MM100(60)), aShadow.ShadowWidth);
}

DECLARE_RTFEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    
    
    
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun,"CharTopBorderDistance");
        
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    
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
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xd8d8d8), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo68787, "fdo68787.rtf")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_RTFEXPORT_TEST(testFdo74709, "fdo74709.rtf")
{
    uno::Reference<table::XCell> xCell = getCell(getParagraphOrTable(1), "B1");
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(360)), getProperty<sal_Int32>(xCell, "RightBorderDistance"));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
