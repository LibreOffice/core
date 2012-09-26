/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "../swmodeltestbase.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>

#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>

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

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testZoom);
    CPPUNIT_TEST(testFdo38176);
    CPPUNIT_TEST(testFdo49683);
    CPPUNIT_TEST(testFdo44174);
    CPPUNIT_TEST(testFdo50087);
    CPPUNIT_TEST(testFdo50831);
    CPPUNIT_TEST(testFdo48335);
    CPPUNIT_TEST(testFdo38244);
    CPPUNIT_TEST(testMathAccents);
    CPPUNIT_TEST(testMathEqarray);
    CPPUNIT_TEST(testMathD);
    CPPUNIT_TEST(testMathEscaping);
    CPPUNIT_TEST(testMathLim);
    CPPUNIT_TEST(testMathMatrix);
    CPPUNIT_TEST(testMathBox);
    CPPUNIT_TEST(testMathMso2007);
    CPPUNIT_TEST(testMathNary);
    CPPUNIT_TEST(testMathLimupp);
    CPPUNIT_TEST(testMathStrikeh);
    CPPUNIT_TEST(testMathPlaceholders);
    CPPUNIT_TEST(testMathRad);
    CPPUNIT_TEST(testMathSepchr);
    CPPUNIT_TEST(testMathSubscripts);
    CPPUNIT_TEST(testMathVerticalstacks);
    CPPUNIT_TEST(testMathRuns);
    CPPUNIT_TEST(testFdo53113);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void roundtrip(const OUString& rURL);
};

void Test::roundtrip(const OUString& rFilename)
{
    uno::Reference<lang::XComponent> xImported = loadFromDesktop(getURLFromSrc("/sw/qa/extras/rtfexport/data/") + rFilename);
    uno::Reference<frame::XStorable> xStorable(xImported, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= OUString("Rich Text Format");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);
    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

void Test::testZoom()
{
    roundtrip("zoom.rtf");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

void Test::testFdo38176()
{
    roundtrip("fdo38176.rtf");
    CPPUNIT_ASSERT_EQUAL(9, getLength());
}

void Test::testFdo49683()
{
    roundtrip("fdo49683.rtf");

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("one"), aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), aKeywords[1]);
}

void Test::testFdo44174()
{
    roundtrip("fdo44174.rtf");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aValue);
}

void Test::testFdo50087()
{
    roundtrip("fdo50087.rtf");

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(OUString("Subject"), xDocumentProperties->getSubject());
    CPPUNIT_ASSERT_EQUAL(OUString("First line.\nSecond line."), xDocumentProperties->getDescription());
}

void Test::testFdo50831()
{
    roundtrip("fdo50831.rtf");

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
    roundtrip("fdo48335.odt");

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
    roundtrip("fdo38244.rtf");

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
    roundtrip("math-accents.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a} widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathEqarray()
{
    roundtrip("math-eqarray.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("y = left lbrace stack { 0, x < 0 # 1, x = 0 # {x} ^ {2} , x > 0 } right none");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathD()
{
    roundtrip("math-d.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("left (x mline y mline z right ) left (1 right ) left [2 right ] left ldbracket 3 right rdbracket left lline 4 right rline left ldline 5 right rdline left langle 6 right rangle left langle a mline b right rangle left ({x} over {y} right )");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathEscaping()
{
    roundtrip("math-escaping.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("á \\{", 5, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathLim()
{
    roundtrip("math-lim.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("lim from {x → 1} {x}", 22, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathMatrix()
{
    roundtrip("math-matrix.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("left [matrix {1 # 2 ## 3 # 4} right ]");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathBox()
{
    roundtrip("math-mbox.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("a");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathMso2007()
{
    roundtrip("math-mso2007.rtf");
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
    roundtrip("math-nary.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("lllint from {1} to {2} {x + 1} prod from {a} {b} sum to {2} {x}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathLimupp()
{
    roundtrip("math-limupp.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("{abcd} overbrace {4}"), aActual);

    aActual = getFormula(getRun(getParagraph(2), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("{xyz} underbrace {3}"), aActual);
}

void Test::testMathStrikeh()
{
    roundtrip("math-strikeh.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("overstrike {abc}"), aActual);
}

void Test::testMathPlaceholders()
{
    roundtrip("math-placeholders.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("sum from <?> to <?> <?>"), aActual);
}

void Test::testMathRad()
{
    roundtrip("math-rad.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("sqrt {4} nroot {3} {x + 1}"), aActual);
}

void Test::testMathSepchr()
{
    roundtrip("math-sepchr.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT_EQUAL(OUString("AxByBzC"), aActual);
}

void Test::testMathSubscripts()
{
    roundtrip("math-subscripts.rtf");
    OUString aActual = getFormula(getRun(getParagraph(1), 1));
    OUString aExpected("{x} ^ {y} + {e} ^ {x} {x} ^ {b} {x} rsub {b} {a} rsub {c} rsup {b} {x} lsub {2} lsup {1} {{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void Test::testMathVerticalstacks()
{
    roundtrip("math-vertical-stacks.rtf");
    CPPUNIT_ASSERT_EQUAL(OUString("{a} over {b}"), getFormula(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("{a} / {b}"), getFormula(getRun(getParagraph(2), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # b }"), getFormula(getRun(getParagraph(3), 1)));
    CPPUNIT_ASSERT_EQUAL(OUString("stack { a # stack { b # c } }"), getFormula(getRun(getParagraph(4), 1)));
}

void Test::testMathRuns()
{
    roundtrip("math-runs.rtf");
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

    roundtrip("fdo53113.odt");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xDraws->getByIndex(0), "CustomShapeGeometry");
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Coordinates")
            rProp.Value >>= aPairs;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aPairs.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(535), aPairs[1].First.Value.get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(102), aPairs[1].Second.Value.get<sal_Int32>());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
