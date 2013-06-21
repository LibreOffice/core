/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
#include <swmodeltestbase.hxx>

class Test : public SwModelTestBase
{
public:
    void testZoom();
    void testFdo38176();
    void testFdo49683();
    void testFdo44174();
    void testFdo50087();
    void testFdo50831();
    void testFdo48335();
    void testFdo38244();
    void testMathAccents();
    void testMathEqarray();
    void testMathD();
    void testMathEscaping();
    void testMathLim();
    void testMathMatrix();
    void testMathBox();
    void testMathMso2007();
    void testMathNary();
    void testMathLimupp();
    void testMathStrikeh();
    void testMathPlaceholders();
    void testMathRad();
    void testMathSepchr();
    void testMathSubscripts();
    void testMathVerticalstacks();
    void testMathRuns();
    void testFdo53113();
    void testFdo55939();
    void testTextFrames();
    void testFdo53604();
    void testFdo52286();
    void testFdo61507();
    void testFdo30983();
    void testPlaceholder();
    void testMnor();
    void testI120928();
    void testBookmark();
    void testHyperlink();
    void testTextFrameBorders();
    void testTextframeGradient();
    void testRecordChanges();
    void testTextframeTable();
    void testFdo66682();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(run);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void run();
};

void Test::run()
{
    MethodEntry<Test> aMethods[] = {
        {"zoom.rtf", &Test::testZoom},
        {"fdo38176.rtf", &Test::testFdo38176},
        {"fdo49683.rtf", &Test::testFdo49683},
        {"fdo44174.rtf", &Test::testFdo44174},
        {"fdo50087.rtf", &Test::testFdo50087},
        {"fdo50831.rtf", &Test::testFdo50831},
        {"fdo48335.odt", &Test::testFdo48335},
        {"fdo38244.rtf", &Test::testFdo38244},
        {"math-accents.rtf", &Test::testMathAccents},
        {"math-eqarray.rtf", &Test::testMathEqarray},
        {"math-d.rtf", &Test::testMathD},
        {"math-escaping.rtf", &Test::testMathEscaping},
        {"math-lim.rtf", &Test::testMathLim},
        {"math-matrix.rtf", &Test::testMathMatrix},
        {"math-mbox.rtf", &Test::testMathBox},
        {"math-mso2007.rtf", &Test::testMathMso2007},
        {"math-nary.rtf", &Test::testMathNary},
        {"math-limupp.rtf", &Test::testMathLimupp},
        {"math-strikeh.rtf", &Test::testMathStrikeh},
        {"math-placeholders.rtf", &Test::testMathPlaceholders},
        {"math-rad.rtf", &Test::testMathRad},
        {"math-sepchr.rtf", &Test::testMathSepchr},
        {"math-subscripts.rtf", &Test::testMathSubscripts},
        {"math-vertical-stacks.rtf", &Test::testMathVerticalstacks},
        {"math-runs.rtf", &Test::testMathRuns},
        {"fdo53113.odt", &Test::testFdo53113},
        {"fdo55939.odt", &Test::testFdo55939},
        {"textframes.odt", &Test::testTextFrames},
        {"fdo53604.odt", &Test::testFdo53604},
        {"fdo52286.odt", &Test::testFdo52286},
        {"fdo61507.rtf", &Test::testFdo61507},
        {"fdo30983.rtf", &Test::testFdo30983},
        {"placeholder.odt", &Test::testPlaceholder},
        {"mnor.rtf", &Test::testMnor},
        {"i120928.rtf", &Test::testI120928},
        {"bookmark.rtf", &Test::testBookmark},
        {"hyperlink.rtf", &Test::testHyperlink},
        {"textframe-borders.rtf", &Test::testTextFrameBorders},
        {"textframe-gradient.rtf", &Test::testTextframeGradient},
        {"record-changes.rtf", &Test::testRecordChanges},
        {"textframe-table.rtf", &Test::testTextframeTable},
        {"fdo66682.rtf", &Test::testFdo66682},
    };
    // Don't test the first import of these, for some reason those tests fail
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
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        load("/sw/qa/extras/rtfexport/data/", rEntry.pName);
        // If the testcase is stored in some other format, it's pointless to test.
        if (OString(rEntry.pName).endsWith(".rtf") && std::find(vBlacklist.begin(), vBlacklist.end(), rEntry.pName) == vBlacklist.end())
            (this->*rEntry.pMethod)();
        reload("Rich Text Format");
        (this->*rEntry.pMethod)();
        finish();
    }
}

void Test::testZoom()
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

void Test::testFdo38176()
{
    CPPUNIT_ASSERT_EQUAL(9, getLength());
}

void Test::testFdo49683()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("one"), aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), aKeywords[1]);
}

void Test::testFdo44174()
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aValue);
}

void Test::testFdo50087()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(OUString("Subject"), xDocumentProperties->getSubject());
    CPPUNIT_ASSERT_EQUAL(OUString("First line.\nSecond line."), xDocumentProperties->getDescription());
}

void Test::testFdo50831()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    float fValue = 0;
    xPropertySet->getPropertyValue("CharHeight") >>= fValue;
    CPPUNIT_ASSERT_EQUAL(10.f, fValue);
}

void Test::testFdo48335()
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

void Test::testFdo38244()
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
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStart"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));
}

void Test::testMathAccents()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a} widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathEqarray()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("y = left lbrace stack { 0, x < 0 # 1, x = 0 # {x} ^ {2} , x > 0 } right none");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathD()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("left (x mline y mline z right ) left (1 right ) left [2 right ] left ldbracket 3 right rdbracket left lline 4 right rline left ldline 5 right rdline left langle 6 right rangle left langle a mline b right rangle left ({x} over {y} right )");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathEscaping()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("á \\{", 5, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathLim()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("lim from {x → 1} {x}", 22, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathMatrix()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("left [matrix {1 # 2 ## 3 # 4} right ]");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathBox()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("a");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathMso2007()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("A = π {r} ^ {2}", 16, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(2), 1));
    aExpected = OUString("{left (x + a right )} ^ {n} = sum from {k = 0} to {n} {left (stack { n # k } right ) {x} ^ {k} {a} ^ {n − k}}", 111, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(3), 1));
    aExpected = OUString("{left (1 + x right )} ^ {n} = 1 + {nx} over {1 !} + {n left (n − 1 right ) {x} ^ {2}} over {2 !} + …", 104, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(4), 1));
    aExpected = OUString("f left (x right ) = {a} rsub {0} + sum from {n = 1} to {∞} {left ({a} rsub {n} cos {nπx} over {L} + {b} rsub {n} sin {nπx} over {L} right )}", 144,
            RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(5), 1));
    aExpected = "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}";
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(6), 1));
    aExpected = OUString("x = {− b ± sqrt {{b} ^ {2} − 4 ac}} over {2 a}", 51, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(7), 1));
    aExpected = OUString("{e} ^ {x} = 1 + {x} over {1 !} + {{x} ^ {2}} over {2 !} + {{x} ^ {3}} over {3 !} + … , − ∞ < x < ∞", 106, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(8), 1));
    aExpected = OUString("sin α ± sin β = 2 sin {1} over {2} left (α ± β right ) cos {1} over {2} left (α ∓ β right )", 101, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    aActual = getFormula(getRun(getParagraph(9), 1));
    aExpected = OUString("cos α + cos β = 2 cos {1} over {2} left (α + β right ) cos {1} over {2} left (α − β right )", 99, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathNary()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("lllint from {1} to {2} {x + 1} prod from {a} {b} sum to {2} {x}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathLimupp()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("{abcd} overbrace {4}"), aActual);

    aActual = getFormula(getRun(getParagraph(2), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("{xyz} underbrace {3}"), aActual);
}

void Test::testMathStrikeh()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("overstrike {abc}"), aActual);
}

void Test::testMathPlaceholders()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("sum from <?> to <?> <?>"), aActual);
}

void Test::testMathRad()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("sqrt {4} nroot {3} {x + 1}"), aActual);
}

void Test::testMathSepchr()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("AxByBzC"), aActual);
}

void Test::testMathSubscripts()
{
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("{x} ^ {y} + {e} ^ {x} {x} ^ {b} {x} rsub {b} {a} rsub {c} rsup {b} {x} lsub {2} lsup {1} {{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathVerticalstacks()
{
    CPPUNIT_ASSERT_EQUAL(OUString("{a} over {b}"), getFormula(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("{a} / {b}"), getFormula(getRun(getParagraph(2), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # b }"), getFormula(getRun(getParagraph(3), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # stack { b # c } }"), getFormula(getRun(getParagraph(4), 1)));
}

void Test::testMathRuns()
{
    // was [](){}, i.e. first curly bracket had an incorrect position
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

void Test::testFdo53113()
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

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xDraws->getByIndex(0), "CustomShapeGeometry");
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

void Test::testFdo55939()
{
    // The problem was that the exported RTF was invalid.
    uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
    getRun(xParagraph, 1, "Main text before footnote.");
    // Why the tab has to be removed here?
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote text."),
            getProperty< uno::Reference<text::XTextRange> >(getRun(xParagraph, 2), "Footnote")->getText()->getString().replaceAll("\t", ""));
    getRun(xParagraph, 3, " Text after the footnote."); // However, this leading space is intentional and OK.
}

void Test::testTextFrames()
{
    // The output was simply invalid, so let's check if all 3 frames were imported back.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

void Test::testFdo53604()
{
    // Invalid output on empty footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

void Test::testFdo52286()
{
    // The problem was that font size wasn't reduced in sub/super script.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(1), 2), "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(getRun(getParagraph(2), 2), "CharEscapementHeight"));
}

void Test::testFdo61507()
{
    /*
     * Unicode-only characters in \title confused Wordpad. Once the exporter
     * was fixed to guard the problematic characters with \upr and \ud, the
     * importer didn't cope with these new keywords.
     */

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    OUString aExpected = OUString("ÉÁŐŰ∭", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xDocumentProperties->getTitle());

    // Only "Hello.", no additional characters.
    CPPUNIT_ASSERT_EQUAL(6, getLength());
}

void Test::testFdo30983()
{
    // These were 'page text area', not 'entire page', i.e. both the horizontal
    // and vertical positions were incorrect.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xDraws->getByIndex(0), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xDraws->getByIndex(0), "VertOrientRelation"));
}

void Test::testPlaceholder()
{
    // Only the field text was exported, make sure we still have a field with the correct Hint text.
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 2));
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xRun, "TextPortionType"));
    uno::Reference<beans::XPropertySet> xField = getProperty< uno::Reference<beans::XPropertySet> >(xRun, "TextField");
    CPPUNIT_ASSERT_EQUAL(OUString("place holder"), getProperty<OUString>(xField, "Hint"));
}

void Test::testMnor()
{
    // \mnor wasn't handled, leading to missing quotes around "divF" and so on.
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("iiint from {V} to <?> {\"divF\"} dV = llint from {S} to <?> {\"F\" ∙ \"n\" dS}", 74, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testI120928()
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

void Test::testBookmark()
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xBookmark(xBookmarksSupplier->getBookmarks()->getByName("firstword"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xBookmark->getAnchor()->getString());
}

void Test::testHyperlink()
{
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(getRun(getParagraph(1), 1, "Hello"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("http://en.wikipedia.org/wiki/World"), getProperty<OUString>(getRun(getParagraph(1), 2, "world"), "HyperLinkURL"));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(getRun(getParagraph(1), 3, "!"), "HyperLinkURL"));
}

void Test::testTextFrameBorders()
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

void Test::testTextframeGradient()
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

void Test::testRecordChanges()
{
    // \revisions wasn't imported/exported.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(mxComponent, "RecordChanges"));
}

void Test::testTextframeTable()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("First para."), getParagraphOfText(1, xText)->getString());
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A"), uno::Reference<text::XTextRange>(xTable->getCellByName("A1"), uno::UNO_QUERY)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), uno::Reference<text::XTextRange>(xTable->getCellByName("B1"), uno::UNO_QUERY)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Last para."), getParagraphOfText(3, xText)->getString());
}

void Test::testFdo66682()
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
