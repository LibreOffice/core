/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <tools/UnitConversion.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

#include <oox/drawingml/drawingmltypes.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testPageGraphicBackground)
{
    loadAndReload("page-graphic-background.odt");
    validate(maTempFile.GetFileName(), test::OOXML);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // No idea how the graphic background should be exported (seems there is no
    // way to do a non-tiling export to OOXML), but at least the background
    // color shouldn't be black.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPageStyle, u"BackColor"_ustr));
}


CPPUNIT_TEST_FIXTURE(Test, testCustomProperties)
{
    loadAndSave("custom-properties.docx");
    // tdf#133377  tdf#103987 FILESAVE XLSX: Make sure the custom/core/application
    // file properties are stored correctly after roundtrip to .docx

    // Extended file properties - specific to Office package,
    // eg. docx - Number of Pages, pptx - Number of Slides
    xmlDocUniquePtr pXmlDoc = parseExport(u"docProps/app.xml"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Paragraphs"_ostr, u"1"_ustr);
    //assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Lines", "1");
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Pages"_ostr, u"1"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Words"_ostr, u"3"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Characters"_ostr, u"22"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:CharactersWithSpaces"_ostr, u"24"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Company"_ostr, u"hhhhkompany"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Manager"_ostr, u"ffffmenadzer;iiiiisecondmanager"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:HyperlinkBase"_ostr, u"gggghiperlink"_ustr);
    //assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:DocSecurity", "2");
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:AppVersion"_ostr, u"15.0000"_ustr);

    // Custom file properties - defined by user
    xmlDocUniquePtr pCustomXml = parseExport(u"docProps/custom.xml"_ustr);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property"_ostr, 9);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[1]"_ostr,
                "name"_ostr, u"testDateProperty"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[1]/vt:filetime"_ostr,
                       u"1982-04-19T10:00:00Z"_ustr);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[3]"_ostr,
                "name"_ostr, u"testNegativeNumberProperty"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[3]/vt:r8"_ostr,
                       u"-100"_ustr);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[4]"_ostr,
                "name"_ostr, u"testNumberProperty"_ustr);
    //assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[4]/vt:i4",
    //                   "256");
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[5]"_ostr,
                "name"_ostr, u"testRealNumberProperty"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[5]/vt:r8"_ostr,
                       u"-128.1"_ustr);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[6]"_ostr,
                "name"_ostr, u"testScientificNumber"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[6]/vt:r8"_ostr,
                       u"1.23456789E+023"_ustr);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[7]"_ostr,
                "name"_ostr, u"testTextProperty"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[7]/vt:lpwstr"_ostr,
                       u"testPropertyValue"_ustr);
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[8]"_ostr,
                "name"_ostr, u"testYesNoProperty"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[8]/vt:bool"_ostr,
                       u"1"_ustr);
    // Hidden Custom File Property. With Final set, MS Office notifies recipients that the document is final, and sets the document to read-only.
    assertXPath(pCustomXml, "/custom-properties:Properties/custom-properties:property[9]"_ostr,
                "name"_ostr, u"_MarkAsFinal"_ustr);
    assertXPathContent(pCustomXml, "/custom-properties:Properties/custom-properties:property[9]/vt:bool"_ostr,
                       u"1"_ustr);

    // Core file properties - common for all packages (eg. creation date, modify date)
    pXmlDoc = parseExport(u"docProps/core.xml"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/dc:creator"_ostr, u"Bartosz Kosiorek;secondauthor"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/dc:description"_ostr, u"cccckomentarzglowny"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, u"2020-10-15T07:42:00Z"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/dcterms:created"_ostr, u"2020-10-14T16:23:00Z"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/cp:category"_ostr, u"eeeetokategoria"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/cp:version"_ostr, u"xxxxversion"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/cp:contentStatus"_ostr, u"ddddstatusnieznany"_ustr);
    assertXPathContent(pXmlDoc, "/cp:coreProperties/dc:identifier"_ostr, u"zzzzidentifier"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testUTF8CustomProperties, "tdf127864.docx")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet> xPropertySet(
        xDocumentProperties->getUserDefinedProperties(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"первый"_ustr, getProperty<OUString>(xPropertySet, u"первый"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testKeywords, "tdf143175.docx")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(u"One"_ustr, aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(u"Two"_ustr, aKeywords[1]);
    CPPUNIT_ASSERT_EQUAL(u"Three"_ustr, aKeywords[2]);
}

DECLARE_OOXMLEXPORT_TEST(testZoom, "zoom.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue(u"ZoomValue"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);

    // Validation test: order of elements were wrong.
    if (!isExported())
        return;
    validate(maTempFile.GetFileName(), test::OOXML);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Order was: rsid, next.
    int nNext = getXPathPosition(pXmlDoc, "/w:styles/w:style[3]"_ostr, "next");
    int nRsid = getXPathPosition(pXmlDoc, "/w:styles/w:style[3]"_ostr, "rsid");
    CPPUNIT_ASSERT(nNext < nRsid);

    pXmlDoc = parseExport(u"docProps/app.xml"_ustr);
    // One paragraph in the document.
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Paragraphs"_ostr, u"1"_ustr);
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Company"_ostr, u"Example Ltd"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, defaultTabStopNotInStyles)
{
    loadAndReload("empty.odt");
    validate(maTempFile.GetFileName(), test::OOXML);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
// The default tab stop was mistakenly exported to a style.
// xray ThisComponent.StyleFamilies(1)(0).ParaTabStop
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles( u"ParagraphStyles"_ustr );
    uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence< style::TabStop > >(
        paragraphStyles->getByName( u"Standard"_ustr ), u"ParaTabStops"_ustr );
// There actually be one tab stop, but it will be the default.
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(1), stops.getLength());
    CPPUNIT_ASSERT_EQUAL( style::TabAlign_DEFAULT, stops[ 0 ].Alignment );
}

DECLARE_OOXMLEXPORT_TEST(testFdo38244, "fdo38244.docx")
{
    /*
     * Comments attached to a range was imported without the range, check for the annotation mark start/end positions.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * oRun = oRuns.nextElement 'Annotation
     * oRun = oRuns.nextElement
     * oRun = oRuns.nextElement 'AnnotationEnd
     * xray oRun.TextPortionType
     */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));

    /*
     * Initials were not imported.
     *
     * oFields = ThisComponent.TextFields.createEnumeration
     * oField = oFields.nextElement
     * xray oField.Initials
     */
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"M"_ustr, getProperty<OUString>(xPropertySet, u"Initials"_ustr));

    /*
     * There was a fake empty paragraph at the end of the comment text.
     *
     * oFields = ThisComponent.TextFields.createEnumeration
     * oField = oFields.nextElement
     * oParas = oField.TextRange.createEnumeration
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     */

    xParaEnumAccess = getProperty< uno::Reference<container::XEnumerationAccess> >(xPropertySet, u"TextRange"_ustr);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    CPPUNIT_ASSERT(!xParaEnum->hasMoreElements());

    if (isExported())
        validate(maTempFile.GetFileName(), test::OOXML);
}

CPPUNIT_TEST_FIXTURE(Test, testCommentsNested)
{
    loadAndReload("comments-nested.odt");
    validate(maTempFile.GetFileName(), test::OOXML);
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xOuter = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Outer"_ustr, getProperty<OUString>(xOuter, u"Content"_ustr));

    uno::Reference<beans::XPropertySet> xInner = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Inner"_ustr, getProperty<OUString>(xInner, u"Content"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testMathEscape)
{
    loadAndReload("math-escape.docx");
    CPPUNIT_ASSERT_EQUAL(u"\\{ left [ right ] \\( \\) \\}"_ustr, getFormula(getRun(getParagraph(1), 1)));
}

// Saving left and right for parentheses when importing not from the m:t tag (docx)
CPPUNIT_TEST_FIXTURE(Test, testTdf158023Export)
{
    loadAndReload("tdf158023_export.docx");
    CPPUNIT_ASSERT_EQUAL(u"left [ right ] left ( right ) left lbrace  right rbrace"_ustr, getFormula(getRun(getParagraph(1), 1)));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo51034)
{
    loadAndReload("fdo51034.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that the 'l' param of the HYPERLINK field was parsed with = "#", not += "#".
    CPPUNIT_ASSERT_EQUAL(u"http://Www.google.com/#a"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
}

// Remove all spaces, as LO export/import may change that.
// Replace symbol - (i.e. U+2212) with ASCII - , LO does this change and it shouldn't matter.
static void CHECK_FORMULA(OUString const & expected, OUString const & actual) {
    CPPUNIT_ASSERT_EQUAL(
        expected.replaceAll( " ", "" ).replaceAll( u"\u2212", "-" ),
        actual.replaceAll( " ", "" ).replaceAll( u"\u2212", "-" ));
}

DECLARE_OOXMLEXPORT_TEST(testMathAccents, "math-accents.docx")
{
    CHECK_FORMULA(
        u"acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a}"
            " widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}"_ustr,
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathD, "math-d.docx")
{
    CHECK_FORMULA( u"left (x mline y mline z right )"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( u"left (1 right )"_ustr, getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( u"left [2 right ]"_ustr, getFormula( getRun( getParagraph( 1 ), 3 )));
    CHECK_FORMULA( u"left ldbracket 3 right rdbracket"_ustr, getFormula( getRun( getParagraph( 1 ), 4 )));
    CHECK_FORMULA( u"left lline 4 right rline"_ustr, getFormula( getRun( getParagraph( 1 ), 5 )));
    CHECK_FORMULA( u"left ldline 5 right rdline"_ustr, getFormula( getRun( getParagraph( 1 ), 6 )));
    CHECK_FORMULA( u"left langle 6 right rangle"_ustr, getFormula( getRun( getParagraph( 1 ), 7 )));
    CHECK_FORMULA( u"left langle a mline b right rangle"_ustr, getFormula( getRun( getParagraph( 1 ), 8 )));
    CHECK_FORMULA( u"left ({x} over {y} right )"_ustr, getFormula( getRun( getParagraph( 1 ), 9 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathEscaping, "math-escaping.docx")
{
    CHECK_FORMULA( u"\u2212 \u221E < x < \u221E"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathLim, "math-lim.docx")
{
    CHECK_FORMULA( u"lim from {x \u2192 1} {x}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathMatrix, "math-matrix.docx")
{
    CHECK_FORMULA( u"left [matrix {1 # 2 ## 3 # 4} right ]"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
}

CPPUNIT_TEST_FIXTURE(Test, testMathMso2k7)
{
    loadAndReload("math-mso2k7.docx");
    CHECK_FORMULA( u"A = \u03C0 {r} ^ {2}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
// TODO check the stack/binom difference
//    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (binom {n} {k} right ) {x} ^ {k} {a} ^ {n-k}}",
    CHECK_FORMULA( u"{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (stack {n # k} right ) {x} ^ {k} {a} ^ {n-k}}"_ustr,
        getFormula( getRun( getParagraph( 2 ), 1 )));
    CHECK_FORMULA( u"{left (1+x right )} ^ {n} =1+ {nx} over {1!} + {n left (n-1 right ) {x} ^ {2}} over {2!} +\u2026"_ustr,
        getFormula( getRun( getParagraph( 3 ), 1 )));
// TODO check (cos/sin miss {})
//    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {{n\xcf\x80x} over {L}} + {b} rsub {n} sin {{n\xcf\x80x} over {L}} right )}",
    CHECK_FORMULA( u"f left (x right ) = {a} rsub {0} + sum from {n=1} to {\u221E} {left ({a} rsub {n} cos {n\u03C0x} over {L} + {b} rsub {n} sin {n\u03C0x} over {L} right )}"_ustr,
        getFormula( getRun( getParagraph( 4 ), 1 )));
    CHECK_FORMULA( u"{a} ^ {2} + {b} ^ {2} = {c} ^ {2}"_ustr, getFormula( getRun( getParagraph( 5 ), 1 )));
    CHECK_FORMULA( u"x = {- b \u00B1 sqrt {{b} ^ {2} -4 ac}} over {2 a}"_ustr,
        getFormula( getRun( getParagraph( 6 ), 1 )));
    CHECK_FORMULA(
        u"{e} ^ {x} =1+ {x} over {1!} + {{x} ^ {2}} over {2!} + {{x} ^ {3}} over {3!} +\u2026,    -\u221E<x<\u221E"_ustr,
        getFormula( getRun( getParagraph( 7 ), 1 )));
    CHECK_FORMULA(
//        "sin {\xce\xb1} \xc2\xb1 sin {\xce\xb2} =2 sin {{1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )}",
// TODO check (cos/in miss {})
        u"sin \u03B1 \u00B1 sin \u03B2 =2 sin {1} over {2} left (\u03B1\u00B1\u03B2 right ) cos {1} over {2} left (\u03B1\u2213\u03B2 right )"_ustr,
        getFormula( getRun( getParagraph( 8 ), 1 )));
    CHECK_FORMULA(
//        "cos {\xce\xb1} + cos {\xce\xb2} =2 cos {{1} over {2} left (\xce\xb1+\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1-\xce\xb2 right )}",
// TODO check (cos/sin miss {})
        u"cos \u03B1 + cos \u03B2 =2 cos {1} over {2} left (\u03B1+\u03B2 right ) cos {1} over {2} left (\u03B1-\u03B2 right )"_ustr,
        getFormula( getRun( getParagraph( 9 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathNary, "math-nary.docx")
{
    CHECK_FORMULA( u"lllint from {1} to {2} {x + 1}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( u"prod from {a} {b}"_ustr, getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( u"sum to {2} {x}"_ustr, getFormula( getRun( getParagraph( 1 ), 3 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathOverbraceUnderbrace, "math-overbrace_underbrace.docx")
{
    CHECK_FORMULA( u"{abcd} overbrace {4}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( u"{xyz} underbrace {3}"_ustr, getFormula( getRun( getParagraph( 2 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathOverstrike, "math-overstrike.docx")
{
    CHECK_FORMULA( u"overstrike {abc}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathPlaceholders, "math-placeholders.docx")
{
    CHECK_FORMULA( u"sum from <?> to <?> <?>"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathRad, "math-rad.docx")
{
    CHECK_FORMULA( u"sqrt {4}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( u"nroot {3} {x + 1}"_ustr, getFormula( getRun( getParagraph( 1 ), 2 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathSubscripts, "math-subscripts.docx")
{
    CHECK_FORMULA( u"{x} ^ {y} + {e} ^ {x}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( u"{x} ^ {b}"_ustr, getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( u"{x} rsub {b}"_ustr, getFormula( getRun( getParagraph( 1 ), 3 )));
    CHECK_FORMULA( u"{a} rsub {c} rsup {b}"_ustr, getFormula( getRun( getParagraph( 1 ), 4 )));
    CHECK_FORMULA( u"{x} lsub {2} lsup {1}"_ustr, getFormula( getRun( getParagraph( 1 ), 5 )));
    CHECK_FORMULA( u"{{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}"_ustr,
        getFormula( getRun( getParagraph( 1 ), 6 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathVerticalStacks, "math-vertical_stacks.docx")
{
    CHECK_FORMULA( u"{a} over {b}"_ustr, getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( u"{a} / {b}"_ustr, getFormula( getRun( getParagraph( 2 ), 1 )));
// TODO check these
//    CHECK_FORMULA( "binom {a} {b}", getFormula( getRun( getParagraph( 3 ), 1 )));
//    CHECK_FORMULA( "binom {a} {binom {b} {c}}", getFormula( getRun( getParagraph( 4 ), 1 )));
}

CPPUNIT_TEST_FIXTURE(Test, testTable)
{
    loadAndReload("table.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure we write qFormat for well-known style names.
    xmlDocUniquePtr pXmlDocCT = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocCT);
    assertXPath(pXmlDocCT, "//w:style[@w:styleId='Normal']/w:qFormat"_ostr, 1);
}

namespace {

struct SingleLineBorders {
    sal_Int16 top, bottom, left, right;
    SingleLineBorders(int t=0, int b=0, int l=0, int r=0)
        : top(t), bottom(b), left(l), right(r) {}
    sal_Int16 getBorder(int i) const
    {
        switch (i) {
            case 0: return top;
            case 1: return bottom;
            case 2: return left;
            case 3: return right;
            default: assert(false); return 0;
        }
    }
};

}

DECLARE_OOXMLEXPORT_TEST(testTableBorders, "table-borders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTextTable (xTables->getByIndex(0), uno::UNO_QUERY);

    std::map<OUString, SingleLineBorders> cellBorders;
    cellBorders[u"A1"_ustr] = SingleLineBorders(106, 106, 106, 106);
    cellBorders[u"B1"_ustr] = SingleLineBorders(106, 0, 106, 35);
    cellBorders[u"C1"_ustr] = SingleLineBorders(106, 106, 35, 106);
    cellBorders[u"A2"_ustr] = SingleLineBorders(106, 35, 106, 0);
    cellBorders[u"B2"_ustr] = SingleLineBorders(0, 0, 0, 0);
    cellBorders[u"C2"_ustr] = SingleLineBorders(106, 106, 0, 106);
    cellBorders[u"A3"_ustr] = SingleLineBorders(35, 35, 106, 106);
    cellBorders[u"B3"_ustr] = SingleLineBorders(0, 106, 106, 106);
    cellBorders[u"C3"_ustr] = SingleLineBorders(106, 106, 106, 106);
    cellBorders[u"A4"_ustr] = SingleLineBorders(35, 106, 106, 35);
    cellBorders[u"B4"_ustr] = SingleLineBorders(106, 106, 35, 106);
    cellBorders[u"C4"_ustr] = SingleLineBorders(106, 106, 106, 106);

    const OUString borderNames[] = {
        u"TopBorder"_ustr,
        u"BottomBorder"_ustr,
        u"LeftBorder"_ustr,
        u"RightBorder"_ustr,
    };

    uno::Sequence<OUString> const cells = xTextTable->getCellNames();
    sal_Int32 nLength = cells.getLength();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(cellBorders.size()), nLength);

    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference<table::XCell> xCell = xTextTable->getCellByName(cells[i]);
        uno::Reference< beans::XPropertySet > xPropSet(xCell, uno::UNO_QUERY_THROW);
        const SingleLineBorders& borders = cellBorders[cells[i]];

        for (sal_Int32 j = 0; j < 4; ++j)
        {
            uno::Any aBorder = xPropSet->getPropertyValue(borderNames[j]);
            table::BorderLine aBorderLine;
            if (aBorder >>= aBorderLine)
            {
                std::stringstream message;
                message << cells[i] << "'s " << borderNames[j] << " is incorrect";
                CPPUNIT_ASSERT_EQUAL_MESSAGE(message.str(),
                        borders.getBorder(j), aBorderLine.OuterLineWidth);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFdo51550)
{
    loadAndReload("fdo51550.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that we lacked the fallback to export the replacement
    // graphic for OLE objects.  But we can actually export the OLE itself now,
    // so check that instead.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEmbeddedObjects->getCount());

    xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);

    if (!pXmlDocCT)
       return; // initial import

    assertXPath(pXmlDocCT, "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.xlsx']"_ostr, "ContentType"_ostr, u"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"_ustr);

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']"_ostr,
        "Type"_ostr,
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package"_ustr);
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p/w:r/w:object/o:OLEObject"_ostr,
        "ProgID"_ostr,
        u"Excel.Sheet.12"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(test1Table1Page, "1-table-1-page.docx")
{
    // 2 problem for this document after export:
    //   - invalid sectPr inserted at the beginning of the page
    //   - font of empty cell is not preserved, leading to change in rows height
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTextFrames)
{
    loadAndReload("textframes.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The frames were simply missing, so let's check if all 3 frames were imported back.
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTextFrameBorders, "textframe-borders.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // After import, a TextFrame is created by the VML import.
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0xD99594), getProperty<Color>(xFrame, u"BackColor"_ustr));

        table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xFrame, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(Color(0xC0504D), Color(ColorTransparency, aBorder.Color));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), aBorder.LineWidth);

        table::ShadowFormat aShadowFormat = getProperty<table::ShadowFormat>(xFrame, u"ShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadowFormat.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(48), aShadowFormat.ShadowWidth);
        CPPUNIT_ASSERT_EQUAL(Color(0x622423), Color(ColorTransparency, aShadowFormat.Color));
    }
    else
    {
        // After export and import, the result is a shape.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0xD99594), getProperty<Color>(xShape, u"FillColor"_ustr));

        CPPUNIT_ASSERT_EQUAL(Color(0xC0504D), getProperty<Color>(xShape, u"LineColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), getProperty<sal_Int32>(xShape, u"LineWidth"_ustr));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), getProperty<sal_Int32>(xShape, u"ShadowXDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), getProperty<sal_Int32>(xShape, u"ShadowYDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x622423), getProperty<Color>(xShape, u"ShadowColor"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTextframeGradient, "textframe-gradient.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    awt::Gradient2 aGradient(getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xC0504D), Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(Color(0xD99594), Color(ColorTransparency, aGradient.EndColor));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);;

    xFrame.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_GRAY7, Color(ColorTransparency, aGradient.EndColor));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);

    // Left / right margin was incorrect: the attribute was missing and we
    // didn't have the right default (had 0 instead of the below one).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, u"LeftMargin"_ustr), 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, u"RightMargin"_ustr), 2);
}

CPPUNIT_TEST_FIXTURE(Test, testCellBtlr)
{
    loadAndSave("cell-btlr.docx");
    /*
     * The problem was that the exporter didn't mirror the workaround of the
     * importer, regarding the btLr text direction: the <w:textDirection
     * w:val="btLr"/> token was completely missing in the output.
     */

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:textDirection"_ostr, "val"_ostr, u"btLr"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTableStylerPrSz, "table-style-rPr-sz.docx")
{
    // Verify that font size inside the table is 20pt, despite the sz attribute in the table size.
    // Also check that other rPr attribute are used: italic, bold, underline
    // Office has the same behavior
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
// disabled temporarily, next commit enables it again
#if 0
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(20.f, getProperty<float>(getRun(xPara, 1), "CharHeight"));
#endif
//    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<short>(getRun(xPara, 1), "CharUnderline"));
//    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xPara, 1), "CharWeight"));
//    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(getRun(xPara, 1), "CharPosture"));
}

DECLARE_OOXMLEXPORT_TEST(testMathLiteral, "math-literal.docx")
{
    CHECK_FORMULA( u"iiint from {V} to <?> {\"div\" \"F\"}  dV= llint from {S} to <?> {\"F\" \u2219 \"n \" dS}"_ustr,
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48557)
{
    loadAndReload("fdo48557.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Inner margins of the textframe wasn't exported.
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, u"TextLeftDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, u"TextRightDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, u"TextUpperDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, u"TextLowerDistance"_ustr));

    //tdf#140967 frame border was too small. Expected 0 (hairline), actual was 2
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), getProperty<sal_uInt32>(xFrame, u"LineWidth"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testI120928, "i120928.docx")
{
    // w:numPicBullet was ignored, leading to missing graphic bullet in numbering.
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
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

DECLARE_OOXMLEXPORT_TEST(testFdo64826, "fdo64826.docx")
{
    // 'Track-Changes' (Track Revisions) wasn't exported.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(mxComponent, u"RecordChanges"_ustr));
    // 'Show-Changes' should not be exported - default is true.
    if (isExported())
    {
        xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
        assertXPath(pXmlSettings, "/w:settings/w:revisionView"_ostr, 0);
    }
}

DECLARE_OOXMLEXPORT_TEST(testPageBackground, "page-background.docx")
{
    // 'Document Background' wasn't exported.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getProperty<Color>(xPageStyle, u"BackColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65265, "fdo65265.docx")
{
    // Redline (tracked changes) of text formatting were not exported
    uno::Reference<text::XTextRange> xParagraph1 = getParagraph(1);
    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);

    CPPUNIT_ASSERT_EQUAL(u"Format"_ustr, getProperty<OUString>(getRun(xParagraph1, 3), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Format"_ustr, getProperty<OUString>(getRun(xParagraph2, 2), u"RedlineType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFDO63053, "fdo63053.docx")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(u"test1&test2"_ustr, xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(u"test1&test2"_ustr, xDocumentProperties->getSubject());
}

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // 1st problem: last character was missing
    CPPUNIT_ASSERT_EQUAL(u"SAMPLE"_ustr, xShape->getString());

    const uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xShape, u"CustomShapeGeometry"_ustr);
    bool bFound = false;
    for (beans::PropertyValue const & prop : aProps)
        if (prop.Name == "TextPath")
            bFound = true;
    // 2nd problem: v:textpath wasn't imported
    CPPUNIT_ASSERT_EQUAL(true, bFound);

    // 3rd problem: rotation angle was 315, not 45.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45 * 100), getProperty<sal_Int32>(xShape, u"RotateAngle"_ustr));

    // 4th problem: mso-position-vertical-relative:margin was ignored, VertOrientRelation was text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));

    // These problems were in the exporter
    // The textpath wasn't semi-transparent.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, u"FillTransparence"_ustr));
    // The textpath had a stroke.
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_NONE, getProperty<drawing::LineStyle>(xShape, u"LineStyle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testWatermarkFont, "watermark-font.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"TestFont"_ustr, xShape->getString());

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    OUString aFont;
    float nFontSize;

    // Check font family
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"CharFontName"_ustr) >>= aFont);
    CPPUNIT_ASSERT_EQUAL(u"DejaVu Serif"_ustr, aFont);

    // Check font size
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize);
    CPPUNIT_ASSERT_EQUAL(float(72), nFontSize);
}

DECLARE_OOXMLEXPORT_TEST(testFdo43093, "fdo43093.docx")
{
    // The problem was that the alignment are not exchange when the paragraph are RTL.
    uno::Reference<uno::XInterface> xParaRtlLeft(getParagraph( 1, u"RTL Left"_ustr));
    sal_Int32 nRtlLeft = getProperty< sal_Int32 >( xParaRtlLeft, u"ParaAdjust"_ustr );
    // test the text Direction value for the paragraph
    sal_Int16 nRLDir  = getProperty< sal_Int32 >( xParaRtlLeft, u"WritingMode"_ustr );

    uno::Reference<uno::XInterface> xParaRtlRight(getParagraph( 3, u"RTL Right"_ustr));
    sal_Int32 nRtlRight = getProperty< sal_Int32 >( xParaRtlRight, u"ParaAdjust"_ustr );
    sal_Int16 nRRDir  = getProperty< sal_Int32 >( xParaRtlRight, u"WritingMode"_ustr );

    uno::Reference<uno::XInterface> xParaLtrLeft(getParagraph( 5, u"LTR Left"_ustr));
    sal_Int32 nLtrLeft = getProperty< sal_Int32 >( xParaLtrLeft, u"ParaAdjust"_ustr );
    sal_Int16 nLLDir  = getProperty< sal_Int32 >( xParaLtrLeft, u"WritingMode"_ustr );

    uno::Reference<uno::XInterface> xParaLtrRight(getParagraph( 7, u"LTR Right"_ustr));
    sal_Int32 nLtrRight = getProperty< sal_Int32 >( xParaLtrRight, u"ParaAdjust"_ustr );
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLtrRight, u"WritingMode"_ustr );

    // this will test the both the text direction and alignment for each paragraph
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nRtlLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRLDir);

    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), nRtlRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRRDir);

    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nLtrLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLLDir);

    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), nLtrRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);
}

DECLARE_OOXMLEXPORT_TEST(testFdo56679, "fdo56679.docx")
{
    // The problem was that the DOCX importer and exporter did not handle the 'color' of an underline
    // (not the color of the text, the color of the underline itself)
    uno::Reference< text::XTextRange > xParagraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > xText = getRun( xParagraph, 2, u"This is a simple sentence."_ustr);

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xText, u"CharUnderlineHasColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xText, u"CharUnderlineColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65400, "fdo65400.docx")
{
    // The problem was that if in Word you choose 'Character Shading' - then the text portion
    // is marked with 'w:shd val=pct15'. LO did not store this value and so when importing and exporting
    // this value was lost (and so Word did not show 'Character Shading' was on)
    uno::Reference< text::XTextRange > paragraph1 = getParagraph( 1 );
    uno::Reference< text::XTextRange > shaded = getRun( paragraph1, 2, u"normal"_ustr );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0x0026 ), getProperty< sal_Int32 >( shaded, u"CharShadingValue"_ustr ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0xd8d8d8 ), getProperty< sal_Int32 >( shaded, u"CharBackColor"_ustr ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66543, "fdo66543.docx")
{
    // The problem was that when importing DOCX with 'line numbers' - the 'start value' was imported
    // but nothing was done with it.

    uno::Reference< text::XTextRange > paragraph1 = getParagraph( 1 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         getProperty<sal_Int32>(paragraph1, u"ParaLineNumberStartValue"_ustr));

    if (!isExported())
        return;

    // ensure unnecessary suppressLineNumbers entry is not created.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:p[1]/w:pPr/w:suppressLineNumbers"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testN822175)
{
    loadAndReload("n822175.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    // Was text::WrapTextMode_THROUGH, due to missing Surround handling in the exporter.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, getProperty<text::WrapTextMode>(xFrame, u"Surround"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66688, "fdo66688.docx")
{
    // The problem was that TextFrame imported and exported the wrong value for transparency
    // (was stored as 'FillTransparence' instead of 'BackColorTransparency'
    uno::Reference<beans::XPropertySet> xFrame(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 80 ), getProperty< sal_Int32 >( xFrame, u"FillTransparence"_ustr ) );
}

DECLARE_OOXMLEXPORT_TEST(testFdo66773, "fdo66773.docx")
{
    // The problem was the line spacing was interpreted by Word as 'Multiple 1.08' if no default settings were written.
    // Now after the 'docDefaults' section is written in <styles.xml> - there is no more problem.
    // (Word does not try to calculate some arbitrary value for line spacing).
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());

    style::LineSpacing alineSpacing = getProperty<style::LineSpacing>(xParaEnum->nextElement(), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP, alineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(100), static_cast<sal_Int32>(alineSpacing.Height));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58577)
{
    loadAndReload("fdo58577.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The second frame was simply missing, so let's check if both frames were imported back.
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testBnc581614)
{
    loadAndReload("bnc581614.doc");
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66929, "fdo66929.docx")
{
    // The problem was that the default 'inset' attribute of the 'textbox' node was exported incorrectly.
    // A node like '<v:textbox inset="0">' was exported back as '<v:textbox inset="0pt,0pt,0pt,0pt">'
    // This is wrong because the original node denotes a specific 'left' inset, and a default 'top','right','bottom' inset
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // VML import -> TextFrame
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 )  , getProperty< sal_Int32 >( xFrame, u"LeftBorderDistance"_ustr ) );
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 127 ), getProperty< sal_Int32 >( xFrame, u"TopBorderDistance"_ustr ) );
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 254 ), getProperty< sal_Int32 >( xFrame, u"RightBorderDistance"_ustr ) );
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 127 ), getProperty< sal_Int32 >( xFrame, u"BottomBorderDistance"_ustr ) );
    }
    else
    {
        // drawingML import -> shape with TextBox
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"TextLeftDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xShape, u"TextUpperDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(254), getProperty<sal_Int32>(xShape, u"TextRightDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xShape, u"TextLowerDistance"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testPageBorderSpacingExportCase2)
{
    loadAndSave("page-borders-export-case-2.docx");
    // The problem was that the exporter didn't mirror the workaround of the
    // importer, regarding the page border's spacing : the <w:pgBorders w:offsetFrom="page">
    // and the inner nodes like <w:top w:space="24" ... />
    //
    // The exporter ALWAYS exported 'w:offsetFrom="text"' even when the spacing values where too large
    // for Word to handle (larger than 31 points)

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Assert the XPath expression - page borders
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders"_ostr, "offsetFrom"_ostr, u"page"_ustr);

    // Assert the XPath expression - 'left' border
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders/w:left"_ostr, "space"_ostr, u"24"_ustr);

    // Assert the XPath expression - 'right' border
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders/w:right"_ostr, "space"_ostr, u"24"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testGrabBag)
{
    loadAndSave("grabbag.docx");
    // w:mirrorIndents was lost on roundtrip, now should be handled as a grab bag property
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:mirrorIndents"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testFdo66781, "fdo66781.docx")
{
    // The problem was that bullets with level=0 were shown in LO as normal bullets,
    // and when saved back to DOCX were saved with level=1 (so hidden bullets became visible)
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "BulletChar")
        {
            CPPUNIT_ASSERT_EQUAL(OUString("\x0", 1, RTL_TEXTENCODING_ASCII_US), rProp.Value.get<OUString>());
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find bullet with level 0");
}

CPPUNIT_TEST_FIXTURE(Test, testFdo60990)
{
    loadAndReload("fdo60990.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The shape had no background, no paragraph adjust and no font color.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x00CFE7F5), getProperty<Color>(xShape, u"FillColor"_ustr));
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, u"ParaAdjust"_ustr)));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(getRun(xParagraph, 1), u"CharColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65718, "fdo65718.docx")
{
    // The problem was that the exporter always exported values of "0" for an images distance from text.
    // the actual attributes where 'distT', 'distB', 'distL', 'distR'
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(0), getProperty<sal_Int32>(xPropertySet, u"TopMargin"_ustr) );
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(0), getProperty<sal_Int32>(xPropertySet, u"BottomMargin"_ustr) );

    // 'getProperty' return 318 (instead of 317.5)
    // I think this is because it returns an integer, instead of a float.
    // The actual exporting to DOCX exports the correct value (114300 = 317.5 * 360)
    // The exporting to DOCX uses the 'SvxLRSpacing' that stores the value in TWIPS (180 TWIPS)
    // However, the 'LeftMargin' property is an integer property that holds that value in 'MM100' (should hold 317.5, but it is 318)
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(114300), getProperty<sal_Int32>(xPropertySet, u"LeftMargin"_ustr) );
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(114300), getProperty<sal_Int32>(xPropertySet, u"RightMargin"_ustr) );
}

DECLARE_OOXMLEXPORT_TEST(testFdo64350, "fdo64350.docx")
{
    // The problem was that page border shadows were not exported
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"ShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
}

DECLARE_OOXMLEXPORT_TEST(testFdo67013, "fdo67013.docx")
{
    /*
     * The problem was that borders inside headers \ footers were not exported
     * This was checked in xray using these commands:
     *
     * xHeaderText = ThisComponent.getStyleFamilies().getByName("PageStyles").getByName("Standard").HeaderText
     * xHeaderEnum = xHeaderText.createEnumeration()
     * xHeaderFirstParagraph = xHeaderEnum.nextElement()
     * xHeaderBottomBorder = xHeaderFirstParagraph.BottomBorder
     *
     * xFooterText = ThisComponent.getStyleFamilies().getByName("PageStyles").getByName("Standard").FooterText
     * xFooterEnum = xFooterText.createEnumeration()
     * xFooterFirstParagraph = xFooterEnum.nextElement()
     * xFooterTopBorder = xFooterFirstParagraph.TopBorder
     */
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
    uno::Reference< text::XTextRange > xHeaderParagraph = getParagraphOfText( 1, xHeaderText );
    table::BorderLine2 aHeaderBottomBorder = getProperty<table::BorderLine2>(xHeaderParagraph, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(Color(0x622423), Color(ColorTransparency, aHeaderBottomBorder.Color));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(106), aHeaderBottomBorder.InnerLineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.LineDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), aHeaderBottomBorder.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aHeaderBottomBorder.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.OuterLineWidth);

    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"FooterText"_ustr);
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );
    table::BorderLine2 aFooterTopBorder = getProperty<table::BorderLine2>(xFooterParagraph, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(Color(0x622423), Color(ColorTransparency, aFooterTopBorder.Color));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aFooterTopBorder.InnerLineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aFooterTopBorder.LineDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), aFooterTopBorder.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aFooterTopBorder.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(106), aFooterTopBorder.OuterLineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testParaShadow, "para-shadow.docx")
{
    // The problem was that in w:pBdr, child elements had a w:shadow attribute, but that was ignored.
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getParagraph(2), u"ParaShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    // w:sz="48" is in eights of a point, 1 pt is 20 twips.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(24/8*20)), aShadow.ShadowWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTableFloating, "table-floating.docx")
{
    // Both the size and the position of the table was incorrect.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Second table was too wide: 16249, i.e. as wide as the first table.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11248), getProperty<sal_Int32>(xTables->getByIndex(1), u"Width"_ustr));

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // After import, table is inside a TextFrame.
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        // This was 0, should be the opposite of (left margin + half of the border width).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xFrame, u"HoriOrientPosition"_ustr));
        // Was 0 as well, should be the right margin.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xFrame, u"RightMargin"_ustr));
    }
    else
    {
        // After import, table is inside a TextFrame.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        // This was 0, should be the opposite of (left margin + half of the border width).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xShape, u"HoriOrientPosition"_ustr));
        // Was 0 as well, should be the right margin.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xShape, u"RightMargin"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_0, "fdo44689_start_page_0.docx")
{
    // The problem was that the import & export process did not analyze the 'start from page' attribute of a section
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xPara, u"PageNumberOffset"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_7, "fdo44689_start_page_7.docx")
{
    // The problem was that the import & export process did not analyze the 'start from page' attribute of a section
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), getProperty<sal_Int16>(xPara, u"PageNumberOffset"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo67737, "fdo67737.docx")
{
    // The problem was that imported shapes did not import and render the 'flip:x' and 'flip:y' attributes
    uno::Reference<drawing::XShape> xArrow = getShape(1);
    const uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xArrow, u"CustomShapeGeometry"_ustr);
    for (beans::PropertyValue const & rProp : aProps)
    {
        if (rProp.Name == "MirroredY")
        {
            CPPUNIT_ASSERT_EQUAL( true, rProp.Value.get<bool>() );
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find MirroredY=true property");
}

DECLARE_OOXMLEXPORT_TEST(testTransparentShadow, "transparent-shadow.docx")
{
    uno::Reference<drawing::XShape> xPicture = getShape(1);
    sal_Int16 nShadowTransparence = getProperty<sal_Int16>(xPicture, u"ShadowTransparence"_ustr);
    CPPUNIT_ASSERT_EQUAL(COL_GRAY, getProperty<Color>(xPicture, u"ShadowColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), nShadowTransparence);
}

CPPUNIT_TEST_FIXTURE(Test, NoFillAttrInImagedata)
{
    loadAndSave("NoFillAttrInImagedata.docx");
    //problem was that type and color2 which are v:fill attributes were written in 'v:imagedata'. The
    //source file has v:fill and no v:imagedata. Same should be in the file written by LO.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:imagedata"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:fill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:fill"_ostr, "type"_ostr, u"tile"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testBnc837302, "bnc837302.docx")
{
    // The problem was that text with empty author was not inserted as a redline
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);

    // previously 'AAA' was not an own run
    getRun(xParagraph, 3, u"AAA"_ustr);
    // interestingly the 'Insert' is set on the _previous_ run
    CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr, getProperty<OUString>(getRun(xParagraph, 2), u"RedlineType"_ustr));

    // make sure we don't introduce a redlined delete in the 2nd paragraph
    xParagraph = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(xParagraph, 1), u"RedlineType"_ustr));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
