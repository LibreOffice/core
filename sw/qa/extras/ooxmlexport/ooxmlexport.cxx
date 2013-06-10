/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>

#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <swmodeltestbase.hxx>

#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

class Test : public SwModelTestBase
{
public:
    void testZoom();
    void defaultTabStopNotInStyles();
    void testFdo38244();
    void testMathEscape();
    void testFdo51034();
    void testMathAccents();
    void testMathD();
    void testMathEscaping();
    void testMathLim();
    void testMathMalformedXml();
    void testMathMatrix();
    void testMathMso2k7();
    void testMathNary();
    void testMathOverbraceUnderbrace();
    void testMathOverstrike();
    void testMathPlaceholders();
    void testMathRad();
    void testMathSubscripts();
    void testMathVerticalStacks();
    void testTablePosition();
    void testFdo47669();
    void testTableBorders();
    void testFdo51550();
    void testN789482();
    void test1Table1Page();
    void testTextFrames();
    void testTextFrameBorders();
    void testTextframeGradient();
    void testCellBtlr();
    void testTableStylerPrSz();
    void testMathLiteral();
    void testFdo48557();
    void testI120928();
    void testFdo64826();
    void testPageBackground();
    void testFdo65265();

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
        {"zoom.docx", &Test::testZoom},
        {"empty.odt", &Test::defaultTabStopNotInStyles},
        {"fdo38244.docx", &Test::testFdo38244},
        {"math-escape.docx", &Test::testMathEscape},
        {"fdo51034.odt", &Test::testFdo51034},
        {"math-accents.docx", &Test::testMathAccents},
        {"math-d.docx", &Test::testMathD},
        {"math-escaping.docx", &Test::testMathEscaping},
        {"math-lim.docx", &Test::testMathLim},
        {"math-malformed_xml.docx", &Test::testMathMalformedXml},
        {"math-matrix.docx", &Test::testMathMatrix},
        {"math-mso2k7.docx", &Test::testMathMso2k7},
        {"math-nary.docx", &Test::testMathNary},
        {"math-overbrace_underbrace.docx", &Test::testMathOverbraceUnderbrace},
        {"math-overstrike.docx", &Test::testMathOverstrike},
        {"math-placeholders.docx", &Test::testMathPlaceholders},
        {"math-rad.docx", &Test::testMathRad},
        {"math-subscripts.docx", &Test::testMathSubscripts},
        {"math-vertical_stacks.docx", &Test::testMathVerticalStacks},
        {"table-position.docx", &Test::testTablePosition},
        {"fdo47669.docx", &Test::testFdo47669},
        {"table-borders.docx", &Test::testTableBorders},
        {"fdo51550.odt", &Test::testFdo51550},
        {"n789482.docx", &Test::testN789482},
//      {"1-table-1-page.docx", &Test::test1Table1Page}, // doesn't work on openSUSE12.2 at least
        {"textframes.odt", &Test::testTextFrames},
        {"textframe-borders.docx", &Test::testTextFrameBorders},
        {"textframe-gradient.docx", &Test::testTextframeGradient},
        {"cell-btlr.docx", &Test::testCellBtlr},
        {"table-style-rPr-sz.docx", &Test::testTableStylerPrSz},
        {"math-literal.docx", &Test::testMathLiteral},
        {"fdo48557.odt", &Test::testFdo48557},
        {"i120928.docx", &Test::testI120928},
        {"fdo64826.docx", &Test::testFdo64826},
        {"page-background.docx", &Test::testPageBackground},
        {"fdo65265.docx", &Test::testFdo65265},
    };
    // Don't test the first import of these, for some reason those tests fail
    const char* aBlacklist[] = {
        "math-escape.docx",
        "math-mso2k7.docx",
    };
    std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        load("/sw/qa/extras/ooxmlexport/data/", rEntry.pName);
        // If the testcase is stored in some other format, it's pointless to test.
        if (OString(rEntry.pName).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), rEntry.pName) == vBlacklist.end())
            (this->*rEntry.pMethod)();
        reload("Office Open XML Text");
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

void Test::defaultTabStopNotInStyles()
{
// The default tab stop was mistakenly exported to a style.
// xray ThisComponent.StyleFamilies(1)(0).ParaTabStop
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles( "ParagraphStyles" );
    uno::Reference< beans::XPropertySet > properties( paragraphStyles->getByName( "Standard" ), uno::UNO_QUERY );
    uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence< style::TabStop > >(
        paragraphStyles->getByName( "Standard" ), "ParaTabStops" );
// There actually be be one tab stop, but it will be the default.
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(1), stops.getLength());
    CPPUNIT_ASSERT_EQUAL( style::TabAlign_DEFAULT, stops[ 0 ].Alignment );
}

void Test::testFdo38244()
{
    /*
     * Comments attached to a range was imported without the range, check for the fieldmark start/end positions.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * oRun = oRuns.nextElement 'TextFieldStart
     * oRun = oRuns.nextElement
     * oRun = oRuns.nextElement 'TextFieldEnd
     * xray oRun.TextPortionType
     */
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

void Test::testMathEscape()
{
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

void Test::testFdo51034()
{
    // The problem was that the 'l' param of the HYPERLINK field was parsed with = "#", not += "#".
    CPPUNIT_ASSERT_EQUAL(OUString("http://Www.google.com/#a"), getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

// Construct the expected formula from UTF8, as there may be such characters.
// Remove all spaces, as LO export/import may change that.
// Replace symbol - (i.e. U+2212) with ASCII - , LO does this change and it shouldn't matter.
#define CHECK_FORMULA( expected, actual ) \
    CPPUNIT_ASSERT_EQUAL( \
        OUString( expected, strlen( expected ), RTL_TEXTENCODING_UTF8 ) \
            .replaceAll( " ", "" ).replaceAll( OUString( "−", strlen( "−" ), RTL_TEXTENCODING_UTF8 ), "-" ), \
        OUString( actual ).replaceAll( " ", "" ).replaceAll( OUString( "−", strlen( "−" ), RTL_TEXTENCODING_UTF8 ), "-" ))

void Test::testMathAccents()
{
    CHECK_FORMULA(
        "acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a}"
            " widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}",
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathD()
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

void Test::testMathEscaping()
{
    CHECK_FORMULA( "− ∞ < x < ∞", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathLim()
{
    CHECK_FORMULA( "lim from {x → 1} {x}", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathMalformedXml()
{
    CPPUNIT_ASSERT_EQUAL( 0, getLength());
}

void Test::testMathMatrix()
{
    CHECK_FORMULA( "left [matrix {1 # 2 ## 3 # 4} right ]", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathMso2k7()
{
    CHECK_FORMULA( "A = π {r} ^ {2}", getFormula( getRun( getParagraph( 1 ), 1 )));
// TODO check the stack/binom difference
//    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (binom {n} {k} right ) {x} ^ {k} {a} ^ {n-k}}",
    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (stack {n # k} right ) {x} ^ {k} {a} ^ {n-k}}",
        getFormula( getRun( getParagraph( 2 ), 1 )));
    CHECK_FORMULA( "{left (1+x right )} ^ {n} =1+ {nx} over {1!} + {n left (n-1 right ) {x} ^ {2}} over {2!} +…",
        getFormula( getRun( getParagraph( 3 ), 1 )));
// TODO check (cos/sin miss {})
//    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {∞} {left ({a} rsub {n} cos {{nπx} over {L}} + {b} rsub {n} sin {{nπx} over {L}} right )}",
    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {∞} {left ({a} rsub {n} cos {nπx} over {L} + {b} rsub {n} sin {nπx} over {L} right )}",
        getFormula( getRun( getParagraph( 4 ), 1 )));
    CHECK_FORMULA( "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}", getFormula( getRun( getParagraph( 5 ), 1 )));
    CHECK_FORMULA( "x = {- b ± sqrt {{b} ^ {2} -4 ac}} over {2 a}",
        getFormula( getRun( getParagraph( 6 ), 1 )));
    CHECK_FORMULA(
        "{e} ^ {x} =1+ {x} over {1!} + {{x} ^ {2}} over {2!} + {{x} ^ {3}} over {3!} +…,    -∞<x<∞",
        getFormula( getRun( getParagraph( 7 ), 1 )));
    CHECK_FORMULA(
//        "sin {α} ± sin {β} =2 sin {{1} over {2} left (α±β right )} cos {{1} over {2} left (α∓β right )}",
// TODO check (cos/in miss {})
        "sin α ± sin β =2 sin {1} over {2} left (α±β right ) cos {1} over {2} left (α∓β right )",
        getFormula( getRun( getParagraph( 8 ), 1 )));
    CHECK_FORMULA(
//        "cos {α} + cos {β} =2 cos {{1} over {2} left (α+β right )} cos {{1} over {2} left (α-β right )}",
// TODO check (cos/sin miss {})
        "cos α + cos β =2 cos {1} over {2} left (α+β right ) cos {1} over {2} left (α-β right )",
        getFormula( getRun( getParagraph( 9 ), 1 )));
}

void Test::testMathNary()
{
    CHECK_FORMULA( "lllint from {1} to {2} {x + 1}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "prod from {a} {b}", getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( "sum to {2} {x}", getFormula( getRun( getParagraph( 1 ), 3 )));
}

void Test::testMathOverbraceUnderbrace()
{
    CHECK_FORMULA( "{abcd} overbrace {4}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{xyz} underbrace {3}", getFormula( getRun( getParagraph( 2 ), 1 )));
}

void Test::testMathOverstrike()
{
    CHECK_FORMULA( "overstrike {abc}", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathPlaceholders()
{
    CHECK_FORMULA( "sum from <?> to <?> <?>", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathRad()
{
    CHECK_FORMULA( "sqrt {4}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "nroot {3} {x + 1}", getFormula( getRun( getParagraph( 1 ), 2 )));
}

void Test::testMathSubscripts()
{
    CHECK_FORMULA( "{x} ^ {y} + {e} ^ {x}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{x} ^ {b}", getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( "{x} rsub {b}", getFormula( getRun( getParagraph( 1 ), 3 )));
    CHECK_FORMULA( "{a} rsub {c} rsup {b}", getFormula( getRun( getParagraph( 1 ), 4 )));
    CHECK_FORMULA( "{x} lsub {2} lsup {1}", getFormula( getRun( getParagraph( 1 ), 5 )));
    CHECK_FORMULA( "{{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}",
        getFormula( getRun( getParagraph( 1 ), 6 )));
}

void Test::testMathVerticalStacks()
{
    CHECK_FORMULA( "{a} over {b}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{a} / {b}", getFormula( getRun( getParagraph( 2 ), 1 )));
// TODO check these
//    CHECK_FORMULA( "binom {a} {b}", getFormula( getRun( getParagraph( 3 ), 1 )));
//    CHECK_FORMULA( "binom {a} {binom {b} {c}}", getFormula( getRun( getParagraph( 4 ), 1 )));
}

void Test::testTablePosition()
{
    sal_Int32 xCoordsFromOffice[] = { 2500, -1000, 0, 0 };
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
            xCoordsFromOffice[i], pos.X, 1);

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

void Test::testFdo47669()
{
    /*
     * Problem: we created imbalance </w:hyperlink> which shouldn't be there,
     * resulting in loading error: missing last character of hyperlink text
     * and content after it wasn't loaded.
     */
    getParagraph(1, "This is a hyperlink with anchor. Also, this sentence should be seen.");
    getRun(getParagraph(1), 2, "hyperlink with anchor");
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.google.com/#a"), getProperty<OUString>(getRun(getParagraph(1), 2), "HyperLinkURL"));
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
void Test::testTableBorders() {
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

void Test::testFdo51550()
{
    // The problem was that we lacked the fallback to export the replacement graphic for OLE objects.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

void Test::testN789482()
{
    // The problem was that w:del was exported before w:hyperlink, resulting in an invalid XML.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    getRun(xParagraph, 1, "Before. ");

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(getRun(xParagraph, 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(sal_True, getProperty<sal_Bool>(getRun(xParagraph, 2), "IsStart"));

    getRun(xParagraph, 3, "www.test.com");
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.test.com/"), getProperty<OUString>(getRun(xParagraph, 3), "HyperLinkURL"));

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(getRun(xParagraph, 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(sal_False, getProperty<sal_Bool>(getRun(xParagraph, 4), "IsStart"));

    getRun(xParagraph, 5, " After.");
}

void Test::test1Table1Page()
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

void Test::testTextFrames()
{
    // The frames were simply missing, so let's check if all 3 frames were imported back.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
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

void Test::testCellBtlr()
{
    /*
     * The problem was that the exporter didn't mirror the workaround of the
     * importer, regarding the btLr text direction: the <w:textDirection
     * w:val="btLr"/> token was completely missing in the output.
     *
     * Given that this doesn't affect the result in the importer, we test the
     * resulting file directly, by opening the zip file, parsing an xml stream,
     * and asserting an XPath expression. This can be extracted to a helper
     * method, once it's clear what is common in such tests.
     */

    // Create the zip file.
    utl::TempFile aTempFile;
    save("Office Open XML Text", aTempFile);

    // Read the XML stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("word/document.xml"), uno::UNO_QUERY);
    boost::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, sal_True));
    pStream->Seek(STREAM_SEEK_TO_END);
    sal_Size nSize = pStream->Tell();
    pStream->Seek(0);
    OStringBuffer aDocument(nSize);
    char ch;
    for (sal_Size i = 0; i < nSize; ++i)
    {
        *pStream >> ch;
        aDocument.append(ch);
    }

    // Parse the XML.
    xmlDocPtr pXmlDoc = xmlParseMemory((const char*)aDocument.getStr(), aDocument.getLength());

    // Assert the XPath expression.
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w"), BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main"));
    OString aXPath = "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:textDirection";
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(aXPath.getStr()), pXmlXpathCtx);
    xmlNodeSetPtr pXmlNodes = pXmlXpathObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OString aAttribute = "val";
    OUString aValue = OUString::createFromAscii((const char*)xmlGetProp(pXmlNode, BAD_CAST(aAttribute.getStr())));
    CPPUNIT_ASSERT_EQUAL(OUString("btLr"), aValue);
}

void Test::testTableStylerPrSz()
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

void Test::testMathLiteral()
{
    CHECK_FORMULA( "iiint from {V} to <?> {\"div\" \"F\"}  dV= llint from {S} to <?> {\"F\" ∙ \"n \" dS}",
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testFdo48557()
{
    // Inner margins of the textframe wasn't exported.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "LeftBorderDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "RightBorderDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "TopBorderDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xFrame, "BottomBorderDistance"));
}

void Test::testI120928()
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

void Test::testFdo64826()
{
    // 'Track-Changes' (Track Revisions) wasn't exported.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(mxComponent, "RecordChanges")));
}

void Test::testPageBackground()
{
    // 'Document Background' wasn't exported.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

void Test::testFdo65265()
{
    // Redline (tracked changes) of text formatting were not exported
    uno::Reference<text::XTextRange> xParagraph1 = getParagraph(1);
    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);

    CPPUNIT_ASSERT_EQUAL(OUString("Format"), getProperty<OUString>(getRun(xParagraph1, 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"), getProperty<OUString>(getRun(xParagraph2, 2), "RedlineType"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
