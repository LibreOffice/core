/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <unotools/tempfile.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <oox/drawingml/drawingmltypes.hxx>

#include <string>

#if !defined(_WIN32)

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }

    /**
     * Validation handling
     */
    bool mustValidate(const char* filename) const override
    {
        const char* aWhitelist[] = {
            "page-graphic-background.odt",
            "zoom.docx",
            "empty.odt",
            "fdo38244.docx",
            "comments-nested.odt"
        };
        std::vector<const char*> vWhitelist(aWhitelist, aWhitelist + SAL_N_ELEMENTS(aWhitelist));

        return std::find(vWhitelist.begin(), vWhitelist.end(), filename) != vWhitelist.end();
    }
};

DECLARE_OOXMLEXPORT_TEST(testPageGraphicBackground, "page-graphic-background.odt")
{
    // No idea how the graphic background should be exported (seems there is no
    // way to do a non-tiling export to OOXML), but at least the background
    // color shouldn't be black.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testZoom, "zoom.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);

    // Validation test: order of elements were wrong.
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;
    // Order was: rsid, next.
    int nNext = getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "next");
    int nRsid = getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "rsid");
    CPPUNIT_ASSERT(nNext < nRsid);

    pXmlDoc = parseExport("docProps/app.xml");
    // One paragraph in the document.
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Paragraphs", "1");
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Company", "Example Ltd");
}

DECLARE_OOXMLEXPORT_TEST(defaultTabStopNotInStyles, "empty.odt")
{
// The default tab stop was mistakenly exported to a style.
// xray ThisComponent.StyleFamilies(1)(0).ParaTabStop
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles( "ParagraphStyles" );
    uno::Reference< beans::XPropertySet > properties( paragraphStyles->getByName( "Standard" ), uno::UNO_QUERY );
    uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence< style::TabStop > >(
        paragraphStyles->getByName( "Standard" ), "ParaTabStops" );
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
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));

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
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));

    /*
     * There was a fake empty paragraph at the end of the comment text.
     *
     * oFields = ThisComponent.TextFields.createEnumeration
     * oField = oFields.nextElement
     * oParas = oField.TextRange.createEnumeration
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     */

    xParaEnumAccess.set(getProperty< uno::Reference<container::XEnumerationAccess> >(xPropertySet, "TextRange"), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    bool bCaught = false;
    try
    {
        xParaEnum->nextElement();
    }
    catch (container::NoSuchElementException&)
    {
        bCaught = true;
    }
    CPPUNIT_ASSERT_EQUAL(true, bCaught);
}

DECLARE_OOXMLEXPORT_TEST(testCommentsNested, "comments-nested.odt")
{
    uno::Reference<beans::XPropertySet> xOuter(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content"));

    uno::Reference<beans::XPropertySet> xInner(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content"));
}

DECLARE_OOXMLEXPORT_TEST(testMathEscape, "math-escape.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testFdo51034, "fdo51034.odt")
{
    // The problem was that the 'l' param of the HYPERLINK field was parsed with = "#", not += "#".
    CPPUNIT_ASSERT_EQUAL(OUString("http://Www.google.com/#a"), getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

// Remove all spaces, as LO export/import may change that.
// Replace symbol - (i.e. U+2212) with ASCII - , LO does this change and it shouldn't matter.
void CHECK_FORMULA(OUString const & expected, OUString const & actual) {
    CPPUNIT_ASSERT_EQUAL(
        expected.replaceAll( " ", "" ).replaceAll( OUString( "\xe2\x88\x92", strlen( "\xe2\x88\x92" ), RTL_TEXTENCODING_UTF8 ), "-" ),
        actual.replaceAll( " ", "" ).replaceAll( OUString( "\xe2\x88\x92", strlen( "\xe2\x88\x92" ), RTL_TEXTENCODING_UTF8 ), "-" ));
}

DECLARE_OOXMLEXPORT_TEST(testMathAccents, "math-accents.docx")
{
    CHECK_FORMULA(
        "acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a}"
            " widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}",
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathD, "math-d.docx")
{
    CHECK_FORMULA( "left (x mline y mline z right )", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "left (1 right )", getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( "left [2 right ]", getFormula( getRun( getParagraph( 1 ), 3 )));
    CHECK_FORMULA( "left ldbracket 3 right rdbracket", getFormula( getRun( getParagraph( 1 ), 4 )));
    CHECK_FORMULA( "left lline 4 right rline", getFormula( getRun( getParagraph( 1 ), 5 )));
    CHECK_FORMULA( "left ldline 5 right rdline", getFormula( getRun( getParagraph( 1 ), 6 )));
    CHECK_FORMULA( "left langle 6 right rangle", getFormula( getRun( getParagraph( 1 ), 7 )));
    CHECK_FORMULA( "left langle a mline b right rangle", getFormula( getRun( getParagraph( 1 ), 8 )));
    CHECK_FORMULA( "left ({x} over {y} right )", getFormula( getRun( getParagraph( 1 ), 9 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathEscaping, "math-escaping.docx")
{
    CHECK_FORMULA( OUString::fromUtf8("\xe2\x88\x92 \xe2\x88\x9e < x < \xe2\x88\x9e"), getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathLim, "math-lim.docx")
{
    CHECK_FORMULA( OUString::fromUtf8("lim from {x \xe2\x86\x92 1} {x}"), getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathMatrix, "math-matrix.docx")
{
    CHECK_FORMULA( "left [matrix {1 # 2 ## 3 # 4} right ]", getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathMso2k7, "math-mso2k7.docx")
{
    CHECK_FORMULA( OUString::fromUtf8("A = \xcf\x80 {r} ^ {2}"), getFormula( getRun( getParagraph( 1 ), 1 )));
// TODO check the stack/binom difference
//    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (binom {n} {k} right ) {x} ^ {k} {a} ^ {n-k}}",
    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (stack {n # k} right ) {x} ^ {k} {a} ^ {n-k}}",
        getFormula( getRun( getParagraph( 2 ), 1 )));
    CHECK_FORMULA( OUString::fromUtf8("{left (1+x right )} ^ {n} =1+ {nx} over {1!} + {n left (n-1 right ) {x} ^ {2}} over {2!} +\xe2\x80\xa6"),
        getFormula( getRun( getParagraph( 3 ), 1 )));
// TODO check (cos/sin miss {})
//    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {{n\xcf\x80x} over {L}} + {b} rsub {n} sin {{n\xcf\x80x} over {L}} right )}",
    CHECK_FORMULA( OUString::fromUtf8("f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {n\xcf\x80x} over {L} + {b} rsub {n} sin {n\xcf\x80x} over {L} right )}"),
        getFormula( getRun( getParagraph( 4 ), 1 )));
    CHECK_FORMULA( "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}", getFormula( getRun( getParagraph( 5 ), 1 )));
    CHECK_FORMULA( OUString::fromUtf8("x = {- b \xc2\xb1 sqrt {{b} ^ {2} -4 ac}} over {2 a}"),
        getFormula( getRun( getParagraph( 6 ), 1 )));
    CHECK_FORMULA(
        OUString::fromUtf8("{e} ^ {x} =1+ {x} over {1!} + {{x} ^ {2}} over {2!} + {{x} ^ {3}} over {3!} +\xe2\x80\xa6,    -\xe2\x88\x9e<x<\xe2\x88\x9e"),
        getFormula( getRun( getParagraph( 7 ), 1 )));
    CHECK_FORMULA(
//        "sin {\xce\xb1} \xc2\xb1 sin {\xce\xb2} =2 sin {{1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )}",
// TODO check (cos/in miss {})
        OUString::fromUtf8("sin \xce\xb1 \xc2\xb1 sin \xce\xb2 =2 sin {1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right ) cos {1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )"),
        getFormula( getRun( getParagraph( 8 ), 1 )));
    CHECK_FORMULA(
//        "cos {\xce\xb1} + cos {\xce\xb2} =2 cos {{1} over {2} left (\xce\xb1+\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1-\xce\xb2 right )}",
// TODO check (cos/sin miss {})
        OUString::fromUtf8("cos \xce\xb1 + cos \xce\xb2 =2 cos {1} over {2} left (\xce\xb1+\xce\xb2 right ) cos {1} over {2} left (\xce\xb1-\xce\xb2 right )"),
        getFormula( getRun( getParagraph( 9 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathNary, "math-nary.docx")
{
    CHECK_FORMULA( "lllint from {1} to {2} {x + 1}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "prod from {a} {b}", getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( "sum to {2} {x}", getFormula( getRun( getParagraph( 1 ), 3 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathOverbraceUnderbrace, "math-overbrace_underbrace.docx")
{
    CHECK_FORMULA( "{abcd} overbrace {4}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{xyz} underbrace {3}", getFormula( getRun( getParagraph( 2 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathOverstrike, "math-overstrike.docx")
{
    CHECK_FORMULA( "overstrike {abc}", getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathPlaceholders, "math-placeholders.docx")
{
    CHECK_FORMULA( "sum from <?> to <?> <?>", getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathRad, "math-rad.docx")
{
    CHECK_FORMULA( "sqrt {4}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "nroot {3} {x + 1}", getFormula( getRun( getParagraph( 1 ), 2 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathSubscripts, "math-subscripts.docx")
{
    CHECK_FORMULA( "{x} ^ {y} + {e} ^ {x}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{x} ^ {b}", getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( "{x} rsub {b}", getFormula( getRun( getParagraph( 1 ), 3 )));
    CHECK_FORMULA( "{a} rsub {c} rsup {b}", getFormula( getRun( getParagraph( 1 ), 4 )));
    CHECK_FORMULA( "{x} lsub {2} lsup {1}", getFormula( getRun( getParagraph( 1 ), 5 )));
    CHECK_FORMULA( "{{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}",
        getFormula( getRun( getParagraph( 1 ), 6 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathVerticalStacks, "math-vertical_stacks.docx")
{
    CHECK_FORMULA( "{a} over {b}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{a} / {b}", getFormula( getRun( getParagraph( 2 ), 1 )));
// TODO check these
//    CHECK_FORMULA( "binom {a} {b}", getFormula( getRun( getParagraph( 3 ), 1 )));
//    CHECK_FORMULA( "binom {a} {binom {b} {c}}", getFormula( getRun( getParagraph( 4 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testTable, "table.odt")
{
    // Validation test: order of elements were wrong.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Order was: insideH, end, insideV.
    int nEnd = getXPathPosition(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblBorders", "end");
    int nInsideH = getXPathPosition(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblBorders", "insideH");
    int nInsideV = getXPathPosition(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblBorders", "insideV");
    CPPUNIT_ASSERT(nEnd < nInsideH);
    CPPUNIT_ASSERT(nInsideH < nInsideV);

    // Make sure we write qFormat for well-known style names.
    assertXPath(parseExport("word/styles.xml"), "//w:style[@w:styleId='Normal']/w:qFormat", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTablePosition, "table-position.docx")
{
    sal_Int32 aXCoordsFromOffice[] = { 2500, -1000, 0, 0 };
    sal_Int32 cellLeftMarginFromOffice[] = { 250, 100, 0, 0 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);

    for (int i=0; i<4; i++) {
        uno::Reference<text::XTextTable> xTable1 (xTables->getByIndex(i), uno::UNO_QUERY);
        // Verify X coord
        uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(), uno::UNO_QUERY);
        xCtrl->select(uno::makeAny(xTable1));
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl, uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect X coord computed from docx",
            aXCoordsFromOffice[i], pos.X, 1);

        // Verify left margin of 1st cell :
        //  * Office left margins are measured relative to the right of the border
        //  * LO left spacing is measured from the center of the border
        uno::Reference<table::XCell> xCell = xTable1->getCellByName("A1");
        uno::Reference< beans::XPropertySet > xPropSet(xCell, uno::UNO_QUERY_THROW);
        sal_Int32 aLeftMargin = -1;
        xPropSet->getPropertyValue("LeftBorderDistance") >>= aLeftMargin;
        uno::Any aLeftBorder = xPropSet->getPropertyValue("LeftBorder");
        table::BorderLine2 aLeftBorderLine;
        aLeftBorder >>= aLeftBorderLine;
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect left spacing computed from docx cell margin",
            cellLeftMarginFromOffice[i], aLeftMargin - 0.5 * aLeftBorderLine.LineWidth, 1);
    }
}

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

DECLARE_OOXMLEXPORT_TEST(testTableBorders, "table-borders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTextTable (xTables->getByIndex(0), uno::UNO_QUERY);

    std::map<OUString, SingleLineBorders> cellBorders;
    cellBorders[OUString("A1")] = SingleLineBorders(106, 106, 106, 106);
    cellBorders[OUString("B1")] = SingleLineBorders(106, 0, 106, 35);
    cellBorders[OUString("C1")] = SingleLineBorders(106, 106, 35, 106);
    cellBorders[OUString("A2")] = SingleLineBorders(106, 35, 106, 0);
    cellBorders[OUString("B2")] = SingleLineBorders(0, 0, 0, 0);
    cellBorders[OUString("C2")] = SingleLineBorders(106, 106, 0, 106);
    cellBorders[OUString("A3")] = SingleLineBorders(35, 35, 106, 106);
    cellBorders[OUString("B3")] = SingleLineBorders(0, 106, 106, 106);
    cellBorders[OUString("C3")] = SingleLineBorders(106, 106, 106, 106);
    cellBorders[OUString("A4")] = SingleLineBorders(35, 106, 106, 35);
    cellBorders[OUString("B4")] = SingleLineBorders(106, 106, 35, 106);
    cellBorders[OUString("C4")] = SingleLineBorders(106, 106, 106, 106);

    const OUString borderNames[] = {
        OUString("TopBorder"),
        OUString("BottomBorder"),
        OUString("LeftBorder"),
        OUString("RightBorder"),
    };

    uno::Sequence<OUString> const cells = xTextTable->getCellNames();
    sal_Int32 nLength = cells.getLength();
    CPPUNIT_ASSERT_EQUAL((sal_Int32)cellBorders.size(), nLength);

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

DECLARE_OOXMLEXPORT_TEST(testFdo51550, "fdo51550.odt")
{
    // The problem was that we lacked the fallback to export the replacement
    // graphic for OLE objects.  But we can actually export the OLE itself now,
    // so check that instead.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xEmbeddedObjects->getCount());

    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");

    if (!pXmlDocCT)
       return; // initial import

    assertXPath(pXmlDocCT, "/ContentType:Types/ContentType:Override[@PartName='/word/embeddings/oleObject1.xlsx']", "ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.Sheet.12");
}

/*
 * doesn't work on openSUSE12.2 at least
DECLARE_OOXMLEXPORT_TEST(test1Table1Page, "1-table-1-page.docx")
{
    // 2 problem for this document after export:
    //   - invalid sectPr inserted at the beginning of the page
    //   - font of empty cell is not preserved, leading to change in rows height
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}
*/

DECLARE_OOXMLEXPORT_TEST(testTextFrames, "textframes.odt")
{
    // The frames were simply missing, so let's check if all 3 frames were imported back.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTextFrameBorders, "textframe-borders.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // After import, a TextFrame is created by the VML import.
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
    else
    {
        // After export and import, the result is a shape.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), getProperty<sal_Int32>(xShape, "FillColor"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), getProperty<sal_Int32>(xShape, "LineColor"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), getProperty<sal_Int32>(xShape, "LineWidth"));

        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), getProperty<sal_Int32>(xShape, "ShadowXDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(48), getProperty<sal_Int32>(xShape, "ShadowYDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), getProperty<sal_Int32>(xShape, "ShadowColor"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTextframeGradient, "textframe-gradient.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
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

    // Left / right margin was incorrect: the attribute was missing and we
    // didn't have the right default (had 0 instead of the below one).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, "RightMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testCellBtlr, "cell-btlr.docx")
{
    /*
     * The problem was that the exporter didn't mirror the workaround of the
     * importer, regarding the btLr text direction: the <w:textDirection
     * w:val="btLr"/> token was completely missing in the output.
     */

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:textDirection", "val", "btLr");
}

DECLARE_OOXMLEXPORT_TEST(testTableStylerPrSz, "table-style-rPr-sz.docx")
{
    // Verify that font size inside the table is 20pt, despite the sz attribute in the table size.
    // Also check that other rPr attribute are used: italic, bold, underline
    // Office has the same behavior
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(20.f, getProperty<float>(getRun(xPara, 1), "CharHeight"));
//    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<short>(getRun(xPara, 1), "CharUnderline"));
//    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xPara, 1), "CharWeight"));
//    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(getRun(xPara, 1), "CharPosture"));
}

DECLARE_OOXMLEXPORT_TEST(testMathLiteral, "math-literal.docx")
{
    CHECK_FORMULA( OUString::fromUtf8("iiint from {V} to <?> {\"div\" \"F\"}  dV= llint from {S} to <?> {\"F\" \xe2\x88\x99 \"n \" dS}"),
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testFdo48557, "fdo48557.odt")
{
    // Inner margins of the textframe wasn't exported.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextLeftDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextRightDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextUpperDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TextLowerDistance"));
}

DECLARE_OOXMLEXPORT_TEST(testI120928, "i120928.docx")
{
    // w:numPicBullet was ignored, leading to missing graphic bullet in numbering.
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

DECLARE_OOXMLEXPORT_TEST(testFdo64826, "fdo64826.docx")
{
    // 'Track-Changes' (Track Revisions) wasn't exported.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(mxComponent, "RecordChanges")));
}

DECLARE_OOXMLEXPORT_TEST(testPageBackground, "page-background.docx")
{
    // 'Document Background' wasn't exported.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65265, "fdo65265.docx")
{
    // Redline (tracked changes) of text formatting were not exported
    uno::Reference<text::XTextRange> xParagraph1 = getParagraph(1);
    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);

    CPPUNIT_ASSERT_EQUAL(OUString("Format"), getProperty<OUString>(getRun(xParagraph1, 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"), getProperty<OUString>(getRun(xParagraph2, 2), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65655, "fdo65655.docx")
{
    // The problem was that the DOCX had a non-blank odd footer and a blank even footer
    // The 'Different Odd & Even Pages' was turned on
    // However - LO assumed that because the 'even' footer is blank - it should ignore the 'Different Odd & Even Pages' flag
    // So it did not import it and did not export it
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    bool bValue = false;
    xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bool(bValue));
    xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bool(bValue));
}

DECLARE_OOXMLEXPORT_TEST(testFDO63053, "fdo63053.docx")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("test1&test2"), xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(OUString("test1&test2"), xDocumentProperties->getSubject());
}

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // 1st problem: last character was missing
    CPPUNIT_ASSERT_EQUAL(OUString("SAMPLE"), xShape->getString());

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xShape, "CustomShapeGeometry");
    bool bFound = false;
    for (int i = 0; i < aProps.getLength(); ++i)
        if (aProps[i].Name == "TextPath")
            bFound = true;
    // 2nd problem: v:textpath wasn't imported
    CPPUNIT_ASSERT_EQUAL(true, bFound);

    // 3rd problem: rotation angle was 315, not 45.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45 * 100), getProperty<sal_Int32>(xShape, "RotateAngle"));

    // 4th problem: mso-position-vertical-relative:margin was ignored, VertOrientRelation was text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xShape, "VertOrientRelation"));

    // These problems were in the exporter
    // The textpath wasn't semi-transparent.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, "FillTransparence"));
    // The textpath had a stroke.
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_NONE, getProperty<drawing::LineStyle>(xShape, "LineStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo43093, "fdo43093.docx")
{
    // The problem was that the alignment are not exchange when the paragraph are RTL.
    uno::Reference<uno::XInterface> xParaRtlLeft(getParagraph( 1, "RTL Left"));
    sal_Int32 nRtlLeft = getProperty< sal_Int32 >( xParaRtlLeft, "ParaAdjust" );
    // test the text Direction value for the pragraph
    sal_Int16 nRLDir  = getProperty< sal_Int32 >( xParaRtlLeft, "WritingMode" );

    uno::Reference<uno::XInterface> xParaRtlRight(getParagraph( 3, "RTL Right"));
    sal_Int32 nRtlRight = getProperty< sal_Int32 >( xParaRtlRight, "ParaAdjust" );
    sal_Int16 nRRDir  = getProperty< sal_Int32 >( xParaRtlRight, "WritingMode" );

    uno::Reference<uno::XInterface> xParaLtrLeft(getParagraph( 5, "LTR Left"));
    sal_Int32 nLtrLeft = getProperty< sal_Int32 >( xParaLtrLeft, "ParaAdjust" );
    sal_Int16 nLLDir  = getProperty< sal_Int32 >( xParaLtrLeft, "WritingMode" );

    uno::Reference<uno::XInterface> xParaLtrRight(getParagraph( 7, "LTR Right"));
    sal_Int32 nLtrRight = getProperty< sal_Int32 >( xParaLtrRight, "ParaAdjust" );
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLtrRight, "WritingMode" );

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

DECLARE_OOXMLEXPORT_TEST(testFdo64238_a, "fdo64238_a.docx")
{
    // The problem was that when 'Show Only Odd Footer' was marked in Word and the Even footer *was filled*
    // then LO would still import the Even footer and concatenate it to the odd footer.
    // This case specifically is for :
    // 'Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be blank, but instead was the 'even' footer
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xFooterParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    sal_Int32 numOfRuns = 0;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        numOfRuns++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), numOfRuns);
}

DECLARE_OOXMLEXPORT_TEST(testFdo64238_b, "fdo64238_b.docx")
{
    // The problem was that when 'Show Only Odd Footer' was marked in Word and the Even footer *was filled*
    // then LO would still import the Even footer and concatenate it to the odd footer.
    // This case specifically is for :
    // 'Non-Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be just the odd footer, but instead was the 'odd' and 'even' footers concatenated
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xFooterParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    sal_Int32 numOfRuns = 0;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        numOfRuns++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), numOfRuns);
}

DECLARE_OOXMLEXPORT_TEST(testFdo56679, "fdo56679.docx")
{
    // The problem was that the DOCX importer and exporter did not handle the 'color' of an underline
    // (not the color of the text, the color of the underline itself)
    uno::Reference< text::XTextRange > xParagraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > xText = getRun( xParagraph, 2, "This is a simple sentence.");

    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xText, "CharUnderlineHasColor")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), getProperty<sal_Int32>(xText, "CharUnderlineColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65400, "fdo65400.docx")
{
    // The problem was that if in Word you choose 'Character Shading' - then the text portion
    // is marked with 'w:shd val=pct15'. LO did not store this value and so when importing and exporting
    // this value was lost (and so Word did not show 'Character Shading' was on)
    uno::Reference< text::XTextRange > paragraph1 = getParagraph( 1 );
    uno::Reference< text::XTextRange > shaded = getRun( paragraph1, 2, "normal" );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0x0026 ), getProperty< sal_Int32 >( shaded, "CharShadingValue" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0xd8d8d8 ), getProperty< sal_Int32 >( shaded, "CharBackColor" ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66543, "fdo66543.docx")
{
    // The problem was that when importing DOCX with 'line numbers' - the 'start value' was imported
    // but nothing was done with it.

    uno::Reference< text::XTextRange > paragraph1 = getParagraph( 1 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1 ), getProperty< sal_Int32 >( paragraph1, "ParaLineNumberStartValue" ));
}

DECLARE_OOXMLEXPORT_TEST(testN822175, "n822175.odt")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    // Was text::WrapTextMode_THROUGH, due to missing Surround handling in the exporter.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, getProperty<text::WrapTextMode>(xFrame, "Surround"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66688, "fdo66688.docx")
{
    // The problem was that TextFrame imported and exported the wrong value for transparency
    // (was stored as 'FillTransparence' instead of 'BackColorTransparency'
    uno::Reference<beans::XPropertySet> xFrame(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 80 ), getProperty< sal_Int32 >( xFrame, "FillTransparence" ) );
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

    style::LineSpacing alineSpacing = getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP, alineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(100), static_cast<sal_Int32>(alineSpacing.Height));
}

DECLARE_OOXMLEXPORT_TEST(testFdo58577, "fdo58577.odt")
{
    // The second frame was simply missing, so let's check if both frames were imported back.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testBnc581614, "bnc581614.doc")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
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
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 )  , getProperty< sal_Int32 >( xFrame, "LeftBorderDistance" ) );
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 127 ), getProperty< sal_Int32 >( xFrame, "TopBorderDistance" ) );
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 254 ), getProperty< sal_Int32 >( xFrame, "RightBorderDistance" ) );
        CPPUNIT_ASSERT_EQUAL( sal_Int32( 127 ), getProperty< sal_Int32 >( xFrame, "BottomBorderDistance" ) );
    }
    else
    {
        // drawingML import -> shape with TextBox
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "TextLeftDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xShape, "TextUpperDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(254), getProperty<sal_Int32>(xShape, "TextRightDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(127), getProperty<sal_Int32>(xShape, "TextLowerDistance"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testPageBorderSpacingExportCase2, "page-borders-export-case-2.docx")
{
    // The problem was that the exporter didn't mirror the workaround of the
    // importer, regarding the page border's spacing : the <w:pgBorders w:offsetFrom="page">
    // and the inner nodes like <w:top w:space="24" .... />
    //
    // The exporter ALWAYS exported 'w:offsetFrom="text"' even when the spacing values where too large
    // for Word to handle (larger than 31 points)

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // Assert the XPath expression - page borders
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders", "offsetFrom", "page");

    // Assert the XPath expression - 'left' border
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders/w:left", "space", "24");

    // Assert the XPath expression - 'right' border
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders/w:right", "space", "24");
}

DECLARE_OOXMLEXPORT_TEST(testFdo66145, "fdo66145.docx")
{
    // The Writer ignored the 'First Is Shared' flag
    CPPUNIT_ASSERT_EQUAL(OUString("This is the FIRST page header."),
        parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("This is the header for the REST OF THE FILE."),
        parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("This is the header for the REST OF THE FILE."),
        parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testGrabBag, "grabbag.docx")
{
    // w:mirrorIndents was lost on roundtrip, now should be handled as a grab bag property
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:mirrorIndents");
}

DECLARE_OOXMLEXPORT_TEST(testFdo66781, "fdo66781.docx")
{
    // The problem was that bullets with level=0 were shown in LO as normal bullets,
    // and when saved back to DOCX were saved with level=1 (so hidden bullets became visible)
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "BulletChar")
        {
            CPPUNIT_ASSERT_EQUAL(OUString("\x0", 1, RTL_TEXTENCODING_UTF8), rProp.Value.get<OUString>());
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find bullet with level 0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo60990, "fdo60990.odt")
{
    // The shape had no background, no paragraph adjust and no font color.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00CFE7F5), getProperty<sal_Int32>(xShape, "FillColor"));
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00), getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65718, "fdo65718.docx")
{
    // The problem was that the exporter always exported values of "0" for an images distance from text.
    // the actual attributes where 'distT', 'distB', 'distL', 'distR'
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32( oox::drawingml::convertEmuToHmm(0) ), getProperty<sal_Int32>(xPropertySet, "TopMargin") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32( oox::drawingml::convertEmuToHmm(0) ), getProperty<sal_Int32>(xPropertySet, "BottomMargin") );

    // 'getProperty' return 318 (instead of 317.5)
    // I think this is because it returns an integer, instead of a float.
    // The actual exporting to DOCX exports the correct value (114300 = 317.5 * 360)
    // The exporting to DOCX uses the 'SvxLRSpacing' that stores the value in TWIPS (180 TWIPS)
    // However, the 'LeftMargin' property is an integer property that holds that value in 'MM100' (should hold 317.5, but it is 318)
    CPPUNIT_ASSERT_EQUAL(sal_Int32( oox::drawingml::convertEmuToHmm(114300) ), getProperty<sal_Int32>(xPropertySet, "LeftMargin") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32( oox::drawingml::convertEmuToHmm(114300) ), getProperty<sal_Int32>(xPropertySet, "RightMargin") );
}

DECLARE_OOXMLEXPORT_TEST(testFdo64350, "fdo64350.docx")
{
    // The problem was that page border shadows were not exported
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getStyles("PageStyles")->getByName("Standard"), "ShadowFormat");
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
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    uno::Reference< text::XTextRange > xHeaderParagraph = getParagraphOfText( 1, xHeaderText );
    table::BorderLine2 aHeaderBottomBorder = getProperty<table::BorderLine2>(xHeaderParagraph, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), aHeaderBottomBorder.Color);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(106), aHeaderBottomBorder.InnerLineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.LineDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), aHeaderBottomBorder.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aHeaderBottomBorder.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.OuterLineWidth);

    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );
    table::BorderLine2 aFooterTopBorder = getProperty<table::BorderLine2>(xFooterParagraph, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), aFooterTopBorder.Color);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aFooterTopBorder.InnerLineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aFooterTopBorder.LineDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), aFooterTopBorder.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aFooterTopBorder.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(106), aFooterTopBorder.OuterLineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testParaShadow, "para-shadow.docx")
{
    // The problem was that in w:pBdr, child elements had a w:shadow attribute, but that was ignored.
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getParagraph(2), "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11248), getProperty<sal_Int32>(xTables->getByIndex(1), "Width"));

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // After import, table is inside a TextFrame.
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        // This was 0, should be the opposite of (left margin + half of the border width).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
        // Was 0 as well, should be the right margin.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xFrame, "RightMargin"));
    }
    else
    {
        // After import, table is inside a TextFrame.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        // This was 0, should be the opposite of (left margin + half of the border width).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
        // Was 0 as well, should be the right margin.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xShape, "RightMargin"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_0, "fdo44689_start_page_0.docx")
{
    // The problem was that the import & export process did not analyze the 'start from page' attribute of a section
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xPara, "PageNumberOffset"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_7, "fdo44689_start_page_7.docx")
{
    // The problem was that the import & export process did not analyze the 'start from page' attribute of a section
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), getProperty<sal_Int16>(xPara, "PageNumberOffset"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo67737, "fdo67737.docx")
{
    // The problem was that imported shapes did not import and render the 'flip:x' and 'flip:y' attributes
    uno::Reference<drawing::XShape> xArrow = getShape(1);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xArrow, "CustomShapeGeometry");
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "MirroredY")
        {
            CPPUNIT_ASSERT_EQUAL( true, bool(rProp.Value.get<bool>()) );
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find MirroredY=true property");
}

DECLARE_OOXMLEXPORT_TEST(testTransparentShadow, "transparent-shadow.docx")
{
    uno::Reference<drawing::XShape> xPicture = getShape(1);
    sal_Int32 nShadowColor = getProperty<sal_Int32>(xPicture, "ShadowColor");
    sal_Int16 nShadowTransparence = getProperty<sal_Int16>(xPicture, "ShadowTransparence");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x808080), nShadowColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), nShadowTransparence);
}

DECLARE_OOXMLEXPORT_TEST(NoFillAttrInImagedata, "NoFillAttrInImagedata.docx")
{
    //problem was that type and color2 which are v:fill attributes were written in 'v:imagedata'
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:imagedata", "type");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect/v:imagedata", "color2");
}

DECLARE_OOXMLEXPORT_TEST(testBnc837302, "bnc837302.docx")
{
    // The problem was that text with empty author was not inserted as a redline
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);

    // previously 'AAA' was not an own run
    getRun(xParagraph, 3, "AAA");
    // interestingly the 'Insert' is set on the _previous_ run
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"), getProperty<OUString>(getRun(xParagraph, 2), "RedlineType"));

    // make sure we don't introduce a redlined delete in the 2nd paragraph
    xParagraph = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(xParagraph, 1), "RedlineType"));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
