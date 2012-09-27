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

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

#include <unotools/tempfile.hxx>

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

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testZoom);
    CPPUNIT_TEST(defaultTabStopNotInStyles);
    CPPUNIT_TEST(testFdo38244);
    CPPUNIT_TEST(testMathEscape);
    CPPUNIT_TEST(testFdo51034);
    CPPUNIT_TEST(testMathAccents);
    CPPUNIT_TEST(testMathD);
    CPPUNIT_TEST(testMathEscaping);
    CPPUNIT_TEST(testMathLim);
    CPPUNIT_TEST(testMathMalformedXml);
    CPPUNIT_TEST(testMathMatrix);
    CPPUNIT_TEST(testMathMso2k7);
    CPPUNIT_TEST(testMathNary);
    CPPUNIT_TEST(testMathOverbraceUnderbrace);
    CPPUNIT_TEST(testMathOverstrike);
    CPPUNIT_TEST(testMathPlaceholders);
    CPPUNIT_TEST(testMathRad);
    CPPUNIT_TEST(testMathSubscripts);
    CPPUNIT_TEST(testMathVerticalStacks);
    CPPUNIT_TEST(testTablePosition);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void roundtrip(const OUString& rURL);
};

void Test::roundtrip(const OUString& rFilename)
{
    uno::Reference<lang::XComponent> xImported = loadFromDesktop(getURLFromSrc("/sw/qa/extras/ooxmlexport/data/") + rFilename);
    uno::Reference<frame::XStorable> xStorable(xImported, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= OUString("Office Open XML Text");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);
    uno::Reference<lang::XComponent> xComponent(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

void Test::testZoom()
{
    roundtrip("zoom.docx");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

void Test::defaultTabStopNotInStyles()
{
    roundtrip( "empty.odt" );
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
    roundtrip("fdo38244.docx");

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
    roundtrip("math-escape.docx");
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

void Test::testFdo51034()
{
    // The problem was that the 'l' param of the HYPERLINK field was parsed with = "#", not += "#".
    roundtrip("fdo51034.odt");
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
    roundtrip( "math-accents.docx" );
    CHECK_FORMULA(
        "acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a}"
            " widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}",
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathD()
{
    roundtrip( "math-d.docx" );
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
    roundtrip( "math-escaping.docx" );
    CHECK_FORMULA( "− ∞ < x < ∞", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathLim()
{
    roundtrip( "math-lim.docx" );
    CHECK_FORMULA( "lim from {x → 1} {x}", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathMalformedXml()
{
    roundtrip( "math-malformed_xml.docx" );
    CPPUNIT_ASSERT_EQUAL( 0, getLength());
}

void Test::testMathMatrix()
{
    roundtrip( "math-matrix.docx" );
    CHECK_FORMULA( "left [matrix {1 # 2 ## 3 # 4} right ]", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathMso2k7()
{
    roundtrip( "math-mso2k7.docx" );
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
    roundtrip( "math-nary.docx" );
    CHECK_FORMULA( "lllint from {1} to {2} {x + 1}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "prod from {a} {b}", getFormula( getRun( getParagraph( 1 ), 2 )));
    CHECK_FORMULA( "sum to {2} {x}", getFormula( getRun( getParagraph( 1 ), 3 )));
}

void Test::testMathOverbraceUnderbrace()
{
    roundtrip( "math-overbrace_underbrace.docx" );
    CHECK_FORMULA( "{abcd} overbrace {4}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{xyz} underbrace {3}", getFormula( getRun( getParagraph( 2 ), 1 )));
}

void Test::testMathOverstrike()
{
    roundtrip( "math-overstrike.docx" );
    CHECK_FORMULA( "overstrike {abc}", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathPlaceholders()
{
    roundtrip( "math-placeholders.docx" );
    CHECK_FORMULA( "sum from <?> to <?> <?>", getFormula( getRun( getParagraph( 1 ), 1 )));
}

void Test::testMathRad()
{
    roundtrip( "math-rad.docx" );
    CHECK_FORMULA( "sqrt {4}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "nroot {3} {x + 1}", getFormula( getRun( getParagraph( 1 ), 2 )));
}

void Test::testMathSubscripts()
{
    roundtrip( "math-subscripts.docx" );
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
    roundtrip( "math-vertical_stacks.docx" );
    CHECK_FORMULA( "{a} over {b}", getFormula( getRun( getParagraph( 1 ), 1 )));
    CHECK_FORMULA( "{a} / {b}", getFormula( getRun( getParagraph( 2 ), 1 )));
// TODO check these
//    CHECK_FORMULA( "binom {a} {b}", getFormula( getRun( getParagraph( 3 ), 1 )));
//    CHECK_FORMULA( "binom {a} {binom {b} {c}}", getFormula( getRun( getParagraph( 4 ), 1 )));
}

void Test::testTablePosition()
{
    // This is the reverse test of n779957 from ooxmlimport
    // We want to verify that a full round-trip does not break formatting
    // Of course if import code is wrong, this tests will fail regardless of export code.
    roundtrip( "../../ooxmlimport/data/n779957.docx" );

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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
