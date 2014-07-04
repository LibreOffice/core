/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(WNT)

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>

#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>

#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

#define EMU_TO_MM100(EMU) (EMU / 360)

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:

    /**
     * Given that some problem doesn't affect the result in the importer, we
     * test the resulting file directly, by opening the zip file, parsing an
     * xml stream, and asserting an XPath expression. This method returns the
     * xml stream, so that you can do the asserting.
     */
    xmlDocPtr parseExport(const OUString& rStreamName = OUString("word/document.xml"));

    /**
     * Helper method to return nodes represented by rXPath.
     */
    xmlNodeSetPtr getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath);

    /**
     * Assert that rXPath exists, and returns exactly one node.
     * In case rAttribute is provided, the rXPath's attribute's value must
     * equal to the rExpected value.
     */
    void assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute = OString(), const OUString& rExpectedValue = OUString());

    /**
     * Assert that rXPath exists, and returns exactly nNumberOfNodes nodes.
     * Useful for checking that we do _not_ export some node (nNumberOfNodes == 0).
     */
    void assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes);

    /**
     * Same as the assertXPath(), but don't assert: return the string instead.
     */
    OUString getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute);

    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
            "ImageCrop.docx",
            "test_GIF_ImageCrop.docx",
            "test_PNG_ImageCrop.docx"
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }
};

xmlDocPtr Test::parseExport(const OUString& rStreamName)
{
    if (!m_bExported)
        return 0;

    // Read the XML stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), m_aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rStreamName), uno::UNO_QUERY);
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
    return xmlParseMemory((const char*)aDocument.getStr(), aDocument.getLength());
}

xmlNodeSetPtr Test::getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w"), BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml"));
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(rXPath.getStr()), pXmlXpathCtx);
    return pXmlXpathObj->nodesetval;
}

void Test::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute, const OUString& rExpectedValue)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString("Attribute '" + rAttribute + "' of '" + rXPath + "' incorrect value.").getStr(),
        rExpectedValue, aValue);
}

void Test::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
        nNumberOfNodes, xmlXPathNodeSetGetLength(pXmlNodes));
}

OUString Test::getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
        1, xmlXPathNodeSetGetLength(pXmlNodes));
    if (rAttribute.isEmpty())
        return OUString();
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    return OUString::createFromAscii((const char*)xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr())));
}

#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

// For testing during development of a tast, you want to use
// DECLARE_OOXMLEXPORT_TEST_ONLY, and change the below to #if 1
// Of course, don't forget to set back to #if 0 when you are done :-)
#if 0
#define DECLARE_OOXMLEXPORT_TEST_ONLY(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

#undef DECLARE_OOXMLEXPORT_TEST
#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) class disabled##TestName : public Test { void disabled(); }; void disabled##TestName::disabled()
#endif

DECLARE_OOXMLEXPORT_TEST(testZoom, "zoom.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_OOXMLEXPORT_TEST(defaultTabStopNotInStyles, "empty.odt")
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

DECLARE_OOXMLEXPORT_TEST(testFdo38244, "fdo38244.docx")
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

DECLARE_OOXMLEXPORT_TEST(testMathEscape, "math-escape.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"), getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testFdo51034, "fdo51034.odt")
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
            .replaceAll( " ", "" ).replaceAll( OUString( "\xe2\x88\x92", strlen( "\xe2\x88\x92" ), RTL_TEXTENCODING_UTF8 ), "-" ), \
        OUString( actual ).replaceAll( " ", "" ).replaceAll( OUString( "\xe2\x88\x92", strlen( "\xe2\x88\x92" ), RTL_TEXTENCODING_UTF8 ), "-" ))

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
    CHECK_FORMULA( "\xe2\x88\x92 \xe2\x88\x9e < x < \xe2\x88\x9e", getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathLim, "math-lim.docx")
{
    CHECK_FORMULA( "lim from {x \xe2\x86\x92 1} {x}", getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathMalformedXml, "math-malformed_xml.docx")
{
    CPPUNIT_ASSERT_EQUAL( 0, getLength());
}

DECLARE_OOXMLEXPORT_TEST(testMathMatrix, "math-matrix.docx")
{
    CHECK_FORMULA( "left [matrix {1 # 2 ## 3 # 4} right ]", getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathMso2k7, "math-mso2k7.docx")
{
    CHECK_FORMULA( "A = \xcf\x80 {r} ^ {2}", getFormula( getRun( getParagraph( 1 ), 1 )));
// TODO check the stack/binom difference
//    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (binom {n} {k} right ) {x} ^ {k} {a} ^ {n-k}}",
    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (stack {n # k} right ) {x} ^ {k} {a} ^ {n-k}}",
        getFormula( getRun( getParagraph( 2 ), 1 )));
    CHECK_FORMULA( "{left (1+x right )} ^ {n} =1+ {nx} over {1!} + {n left (n-1 right ) {x} ^ {2}} over {2!} +\xe2\x80\xa6",
        getFormula( getRun( getParagraph( 3 ), 1 )));
// TODO check (cos/sin miss {})
//    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {{n\xcf\x80x} over {L}} + {b} rsub {n} sin {{n\xcf\x80x} over {L}} right )}",
    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {n\xcf\x80x} over {L} + {b} rsub {n} sin {n\xcf\x80x} over {L} right )}",
        getFormula( getRun( getParagraph( 4 ), 1 )));
    CHECK_FORMULA( "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}", getFormula( getRun( getParagraph( 5 ), 1 )));
    CHECK_FORMULA( "x = {- b \xc2\xb1 sqrt {{b} ^ {2} -4 ac}} over {2 a}",
        getFormula( getRun( getParagraph( 6 ), 1 )));
    CHECK_FORMULA(
        "{e} ^ {x} =1+ {x} over {1!} + {{x} ^ {2}} over {2!} + {{x} ^ {3}} over {3!} +\xe2\x80\xa6,    -\xe2\x88\x9e<x<\xe2\x88\x9e",
        getFormula( getRun( getParagraph( 7 ), 1 )));
    CHECK_FORMULA(
//        "sin {\xce\xb1} \xc2\xb1 sin {\xce\xb2} =2 sin {{1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )}",
// TODO check (cos/in miss {})
        "sin \xce\xb1 \xc2\xb1 sin \xce\xb2 =2 sin {1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right ) cos {1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )",
        getFormula( getRun( getParagraph( 8 ), 1 )));
    CHECK_FORMULA(
//        "cos {\xce\xb1} + cos {\xce\xb2} =2 cos {{1} over {2} left (\xce\xb1+\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1-\xce\xb2 right )}",
// TODO check (cos/sin miss {})
        "cos \xce\xb1 + cos \xce\xb2 =2 cos {1} over {2} left (\xce\xb1+\xce\xb2 right ) cos {1} over {2} left (\xce\xb1-\xce\xb2 right )",
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

DECLARE_OOXMLEXPORT_TEST(testTablePosition, "table-position.docx")
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

DECLARE_OOXMLEXPORT_TEST(testFdo47669, "fdo47669.docx")
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
    // The problem was that we lacked the fallback to export the replacement graphic for OLE objects.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testN789482, "n789482.docx")
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

DECLARE_OOXMLEXPORT_TEST(testTextFrames, "textframes.odt")
{
    // The frames were simply missing, so let's check if all 3 frames were imported back.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTextFrameBorders, "textframe-borders.docx")
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

DECLARE_OOXMLEXPORT_TEST(testTextframeGradient, "textframe-gradient.docx")
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
    CHECK_FORMULA( "iiint from {V} to <?> {\"div\" \"F\"}  dV= llint from {S} to <?> {\"F\" \xe2\x88\x99 \"n \" dS}",
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testFdo48557, "fdo48557.odt")
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
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(mxComponent, "RecordChanges")));
}

DECLARE_OOXMLEXPORT_TEST(testPageBackground, "page-background.docx")
{
    // 'Document Background' wasn't exported.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testPageGraphicBackground, "page-graphic-background.odt")
{
    // No idea how the graphic background should be exported (seems there is no
    // way to do a non-tiling export to OOXML), but at least the background
    // color shouldn't be black.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPageStyle, "BackColor"));
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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    sal_Bool bValue = false;
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
    // then LO would still import the Even footer and concatenate it to to the odd footer.
    // This case specifically is for :
    // 'Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be blank, but instead was the 'even' footer
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "FooterText");
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
    // then LO would still import the Even footer and concatenate it to to the odd footer.
    // This case specifically is for :
    // 'Non-Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be just the odd footer, but instead was the 'odd' and 'even' footers concatenated
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "FooterText");
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

    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xText, "CharUnderlineHasColor")));
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
    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 80 ), getProperty< sal_Int32 >( xFrame, "BackColorTransparency" ) );
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
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
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
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 )  , getProperty< sal_Int32 >( xFrame, "LeftBorderDistance" ) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 127 ), getProperty< sal_Int32 >( xFrame, "TopBorderDistance" ) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 254 ), getProperty< sal_Int32 >( xFrame, "RightBorderDistance" ) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 127 ), getProperty< sal_Int32 >( xFrame, "BottomBorderDistance" ) );
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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(xPropertySet, "FirstIsShared")));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00CFE7F5), getProperty<sal_Int32>(xShape, "BackColor"));
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

    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(0) ), getProperty<sal_Int32>(xPropertySet, "TopMargin") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(0) ), getProperty<sal_Int32>(xPropertySet, "BottomMargin") );

    // Going to do '+1' because the 'getProperty' return 318 (instead of 317.5)
    // I think this is because it returns an integer, instead of a float.
    // The actual exporting to DOCX exports the correct value (114300 = 317.5 * 360)
    // The exporting to DOCX uses the 'SvxLRSpacing' that stores the value in TWIPS (180 TWIPS)
    // However, the 'LeftMargin' property is an integer property that holds that value in 'MM100' (should hold 317.5, but it is 318)
    // So I had to add the hack of the '+1' to make the test-case pass
    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(114300) + 1 ), getProperty<sal_Int32>(xPropertySet, "LeftMargin") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(114300) + 1), getProperty<sal_Int32>(xPropertySet, "RightMargin") );
}

DECLARE_OOXMLEXPORT_TEST(testFdo64350, "fdo64350.docx")
{
    // The problem was that page border shadows were not exported
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "ShadowFormat");
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
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    uno::Reference< text::XTextRange > xHeaderParagraph = getParagraphOfText( 1, xHeaderText );
    table::BorderLine2 aHeaderBottomBorder = getProperty<table::BorderLine2>(xHeaderParagraph, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), aHeaderBottomBorder.Color);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(106), aHeaderBottomBorder.InnerLineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.LineDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), aHeaderBottomBorder.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aHeaderBottomBorder.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.OuterLineWidth);

    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "FooterText");
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(TWIP_TO_MM100(24/8*20)), aShadow.ShadowWidth);
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
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    // This was 0, should be the the opposite of (left margin + half of the border width).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    // Was 0 as well, should be the right margin.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xFrame, "RightMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTableFloatingMargins, "table-floating-margins.docx")
{
    // In case the table had custom left cell margin, the horizontal position was still incorrect (too small, -199).
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-499), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    // These were 0 as well, due to lack of import.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xFrame, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xFrame, "BottomMargin"));

    // Paragraph bottom margin wasn't 0 in the A1 cell of the floating table.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:pict/v:rect/v:textbox/w:txbxContent/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "after", "0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_0, "fdo44689_start_page_0.docx")
{
    // The problem was that the import & export process did not analyze the 'start from page' attribute of a section
    uno::Reference<beans::XPropertySet> xPara(getParagraph(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xPara, "PageNumberOffset"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_7, "fdo44689_start_page_7.docx")
{
    // The problem was that the import & export process did not analyze the 'start from page' attribute of a section
    uno::Reference<beans::XPropertySet> xPara(getParagraph(0), uno::UNO_QUERY);
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
            CPPUNIT_ASSERT_EQUAL( true, bool(rProp.Value.get<sal_Bool>()) );
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find MirroredY=true property");
}

DECLARE_OOXMLEXPORT_TEST(testTransparentShadow, "transparent-shadow.docx")
{
    uno::Reference<drawing::XShape> xPicture = getShape(1);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xPicture, "ShadowFormat");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x7f808080), aShadow.Color);
}

DECLARE_OOXMLEXPORT_TEST(testBnc834035, "bnc834035.odt")
{
    // Illustration index had wrong hyperlinks: anchor was using Writer's
    // <seqname>!<index>|sequence syntax, not a bookmark name.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    // This was Figure!1|sequence.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:hyperlink", "anchor", "_Toc363553908");
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
    OUString aProperty;
    try
    {
        // throws when not present
        aProperty = getProperty<OUString>(getRun(xParagraph, 1), "RedlineType");
    }
    catch (const beans::UnknownPropertyException&) {}
    CPPUNIT_ASSERT_EQUAL(OUString(), aProperty);
}

DECLARE_OOXMLEXPORT_TEST(testFdo68418, "fdo68418.docx")
{
    // The problem was that in 'MSWordExportBase::SectionProperties' function in 'wrt8sty.cxx'
    // it checked if it 'IsPlausableSingleWordSection'.
    // The 'IsPlausableSingleWordSection' compared different aspects of 2 'SwFrmFmt' objects.
    // One of the checks was 'do both formats have the same distance from the top and bottom ?'
    // This check is correct if both have headers or both don't have headers.
    // However - if one has a header, and the other one has an empty header (no header) - it is not correct to compare
    // between them (same goes for 'footer').
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "FooterText");
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );

    // First page footer is empty, second page footer is 'aaaa'
    CPPUNIT_ASSERT_EQUAL(OUString("aaaa"), xFooterParagraph->getString());        // I get an error that it expects ''
}

DECLARE_OOXMLEXPORT_TEST(testA4AndBorders, "a4andborders.docx")
{
    /*
     * The problem was that in case of a document with borders, the pgSz attribute
     * was exported as a child of pgBorders, thus being ignored on reload.
     * We assert dimension against A4 size in mm (to avoid minor rounding errors)
     */
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Width (mm)", sal_Int32(210), getProperty<sal_Int32>(xPageStyle, "Width") / 100);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Height (mm)", sal_Int32(297), getProperty<sal_Int32>(xPageStyle, "Height") / 100);
}

DECLARE_OOXMLEXPORT_TEST(testFdo68787, "fdo68787.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    // This was 25, the 'lack of w:separator' <-> '0 line width' mapping was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // OOXML has just one border attribute (<w:bdr>) for text border so all side has
    // the same border with the same padding
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Padding (w:space)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun,"CharTopBorderDistance");
        // In the original ODT the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // Shadow (w:shadow)
    /* OOXML use just one bool value for shadow so the next conversions
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

    // Also check shadow when it is in middle of the paragraph
    // (problem can be during export with SwWW8AttrIter::HasTextItem())
    {
        uno::Reference<beans::XPropertySet> xMiddleRun(getRun(getParagraph(2),2), uno::UNO_QUERY);
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xMiddleRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

DECLARE_OOXMLEXPORT_TEST(testStyleInheritance, "style-inheritance.docx")
{
    // Check that now styleId's are more like what MSO produces
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    // the 1st style always must be Normal
    assertXPath(pXmlStyles, "/w:styles/w:style[1]", "styleId", "Normal");
    // some random style later
    assertXPath(pXmlStyles, "/w:styles/w:style[4]", "styleId", "Heading3");

    // Check that we do _not_ export w:next for styles that point to themselves.
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:next", 0);

    // Check that we roundtrip <w:next> correctly - on XML level
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:next", "val", "Normal");
    // And to be REALLY sure, check it on the API level too ;-)
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > properties(paragraphStyles->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), getProperty<OUString>(properties, "FollowStyle"));

    // This was 0, as export of w:outlineLvl was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(properties, "OutlineLevel"));

    properties = uno::Reference< beans::XPropertySet >(paragraphStyles->getByName("Heading 11"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getProperty<OUString>(properties, "FollowStyle"));

    // Make sure style #2 is Heading 1.
    assertXPath(pXmlStyles, "/w:styles/w:style[2]", "styleId", "Heading1");
    // w:ind was copied from the parent (Normal) style without a good reason.
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:pPr/w:ind", 0);

    // We output exactly 2 properties in rPrDefault, nothing else was
    // introduced as an additional default
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/*", 2);
    // Check that we output real content of rPrDefault
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "ascii", "Times New Roman");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:lang", "bidi", "ar-SA");
    // pPrDefault is empty
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/*", 0);

    // Check latent styles
    uno::Sequence<beans::PropertyValue> aGrabBag = getProperty< uno::Sequence<beans::PropertyValue> >(mxComponent, "InteropGrabBag");
    uno::Sequence<beans::PropertyValue> aLatentStyles;
    for (sal_Int32 i = 0; i < aGrabBag.getLength(); ++i)
        if (aGrabBag[i].Name == "latentStyles")
            aGrabBag[i].Value >>= aLatentStyles;
    CPPUNIT_ASSERT(aLatentStyles.getLength()); // document should have latent styles

    // Check latent style default attributes
    OUString aCount;
    uno::Sequence<beans::PropertyValue> aLatentStyleExceptions;
    for (sal_Int32 i = 0; i < aLatentStyles.getLength(); ++i)
    {
        if (aLatentStyles[i].Name == "count")
            aCount = aLatentStyles[i].Value.get<OUString>();
        else if (aLatentStyles[i].Name == "lsdExceptions")
            aLatentStyles[i].Value >>= aLatentStyleExceptions;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("371"), aCount); // This check the "count" attribute.

    // Check exceptions to the latent style defaults.
    uno::Sequence<beans::PropertyValue> aLatentStyleException;
    aLatentStyleExceptions[0].Value >>= aLatentStyleException;
    OUString aName;
    for (sal_Int32 i = 0; i < aLatentStyleException.getLength(); ++i)
        if (aLatentStyleException[i].Name == "name")
            aName = aLatentStyleException[i].Value.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"), aName); // This checks the "name" attribute of the first exception.

    // This numbering style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']/w:name", "val", "No List");

    // Table style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:tblPr/w:tblCellMar/w:left", "w", "108");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:semiHidden", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:unhideWhenUsed", 1);

    // Additional para style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListParagraph']/w:uiPriority", "val", "34");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:qFormat", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:rsid", "val", "00780346");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']", "default", "1");

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:link", "val", "Heading1Char");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:locked", 1);

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']", "customStyle", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']/w:autoRedefine", 1);

    // Additional char style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']", "default", "1");

    // Finally check the same for numbering styles.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']", "default", "1");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar1, "calendar1.docx")
{
    // Document has a non-trivial table style, test the roundtrip of it.
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:basedOn", "val", "TableNormal");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rsid", "val", "00903003");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblPr/w:tblStyleColBandSize", "val", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tcPr/w:shd", "val", "clear");

    // Table style lost its paragraph / run properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:pPr/w:spacing", "lineRule", "auto");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rPr/w:lang", "eastAsia", "ja-JP");

    // Table style lost its conditional table formatting properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:wordWrap", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:rFonts", "hAnsiTheme", "minorHAnsi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tblPr", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:vAlign", "val", "bottom");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='lastRow']/w:tcPr/w:tcBorders/w:tr2bl", "val", "nil");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcBorders/w:top", "themeColor", "text1");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar2, "calendar2.docx")
{
    // This paragraph property was missing in table style.
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:pPr/w:jc", "val", "center");

    // These run properties were missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "val", "en-US");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "bidi", "ar-SA");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:caps", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:smallCaps", "val", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color", "themeColor", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:spacing", "val", "20");

    // Table borders were also missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblPr/w:tblBorders/w:insideV", "themeTint", "99");
}

DECLARE_OOXMLEXPORT_TEST(testQuicktables, "quicktables.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    // These were missing in the Calendar3 table style.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:rFonts", "cstheme", "majorBidi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:color", "themeTint", "80");
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color", "themeShade").equalsIgnoreAsciiCase("BF"));

    // Calendar4.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:pPr/w:snapToGrid", "val", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:rPr/w:bCs", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFill", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFillShade", "80");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind", "rightChars", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind", "right", "144");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcMar/w:bottom", "w", "86");

    // LightList.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='LightList']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:spacing", "before", "0");

    // MediumList2-Accent1.
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumList2-Accent1']/w:tblStylePr[@w:type='band1Vert']/w:tcPr/w:shd", "themeFillTint").equalsIgnoreAsciiCase("3F"));

    // MediumShading2-Accent5.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumShading2-Accent5']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:tcBorders/w:top", "color", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testFdo71302, "fdo71302.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    // This got renamed to "Strong Emphasis" without a good reason.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Strong']", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    sal_Bool bTheme = sal_False;
    for(int i = 0; i < aGrabBag.getLength(); ++i)
    {
      if (aGrabBag[i].Name == "OOXTheme")
      {
        bTheme = sal_True;
        uno::Reference<xml::dom::XDocument> aThemeDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aThemeDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aThemeDom.get()); // Reference not empty
      }
    }
    CPPUNIT_ASSERT(bTheme); // Grab Bag has all the expected elements

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xGroup->getCount()); // 1 rendered bitmap from the original shapes

    uno::Reference<beans::XPropertySet> xGroupPropertySet(getShape(1), uno::UNO_QUERY);
    xGroupPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    sal_Bool bData = sal_False, bLayout = sal_False, bQStyle = sal_False, bColor = sal_False, bDrawing = sal_False;
    for(int i = 0; i < aGrabBag.getLength(); ++i)
    {
      if (aGrabBag[i].Name == "OOXData")
      {
        bData = sal_True;
        uno::Reference<xml::dom::XDocument> aDataDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aDataDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aDataDom.get()); // Reference not empty
      }
      else if (aGrabBag[i].Name == "OOXLayout")
      {
        bLayout = sal_True;
        uno::Reference<xml::dom::XDocument> aLayoutDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aLayoutDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aLayoutDom.get()); // Reference not empty
      }
      else if (aGrabBag[i].Name == "OOXStyle")
      {
        bQStyle = sal_True;
        uno::Reference<xml::dom::XDocument> aStyleDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aStyleDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aStyleDom.get()); // Reference not empty
      }
      else if (aGrabBag[i].Name == "OOXColor")
      {
        bColor = sal_True;
        uno::Reference<xml::dom::XDocument> aColorDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aColorDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aColorDom.get()); // Reference not empty
      }
      else if (aGrabBag[i].Name == "OOXDrawing")
      {
        bDrawing = sal_True;
        uno::Reference<xml::dom::XDocument> aDrawingDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aDrawingDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aDrawingDom.get()); // Reference not empty
      }
    }
    CPPUNIT_ASSERT(bData && bLayout && bQStyle && bColor && bDrawing); // Grab Bag has all the expected elements

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(0), uno::UNO_QUERY);
    OUString nValue;
    xPropertySet->getPropertyValue("Name") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(OUString("RenderedShapes"), nValue); // Rendered bitmap has the proper name
}

DECLARE_OOXMLEXPORT_TEST(testFdo69636, "fdo69636.docx")
{
    /*
     * The problem was that the exporter didn't mirror the workaround of the
     * importer, regarding the btLr text frame direction: the
     * mso-layout-flow-alt property was completely missing in the output.
     */
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:pict/v:rect/v:textbox", "style").match("mso-layout-flow-alt:bottom-to-top"));
}

DECLARE_OOXMLEXPORT_TEST(testCharHighlight, "char_highlight.docx")
{
    const uno::Reference< text::XTextRange > xPara = getParagraph(1);
    // Both highlight and background
    const sal_Int32 nBackColor(0x4F81BD);
    for( int nRun = 1; nRun <= 16; ++nRun )
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,nRun), uno::UNO_QUERY);
        sal_Int32 nHighlightColor = 0;
        switch( nRun )
        {
            case 1: nHighlightColor = 0x000000; break; //black
            case 2: nHighlightColor = 0x0000ff; break; //blue
            case 3: nHighlightColor = 0x00ffff; break; //cyan
            case 4: nHighlightColor = 0x00ff00; break; //green
            case 5: nHighlightColor = 0xff00ff; break; //magenta
            case 6: nHighlightColor = 0xff0000; break; //red
            case 7: nHighlightColor = 0xffff00; break; //yellow
            case 8: nHighlightColor = 0xffffff; break; //white
            case 9: nHighlightColor = 0x000080;  break;//dark blue
            case 10: nHighlightColor = 0x008080; break; //dark cyan
            case 11: nHighlightColor = 0x008000; break; //dark green
            case 12: nHighlightColor = 0x800080; break; //dark magenta
            case 13: nHighlightColor = 0x800000; break; //dark red
            case 14: nHighlightColor = 0x808000; break; //dark yellow
            case 15: nHighlightColor = 0x808080; break; //dark gray
            case 16: nHighlightColor = 0xC0C0C0; break; //light gray
        }
        CPPUNIT_ASSERT_EQUAL(nHighlightColor, getProperty<sal_Int32>(xRun,"CharHighlight"));
        CPPUNIT_ASSERT_EQUAL(nBackColor, getProperty<sal_Int32>(xRun,"CharBackColor"));
    }

    // Only highlight
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,17), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0C0C0), getProperty<sal_Int32>(xRun,"CharHighlight"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));
    }

    // Only background
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,18), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000ff), getProperty<sal_Int32>(xRun,"CharBackColor"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFontNameIsEmpty, "font-name-is-empty.docx")
{
    // Check no empty font name is exported
    // This test does not fail, if the document contains a font with empty name.

    xmlDocPtr pXmlFontTable = parseExport("word/fontTable.xml");
    if (!pXmlFontTable)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlFontTable, "/w:fonts/w:font");
    sal_Int32 length = xmlXPathNodeSetGetLength(pXmlNodes);
    for(sal_Int32 index = 0; index < length; index++){
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[index];
        OUString attrVal = OUString::createFromAscii((const char*)xmlGetProp(pXmlNode, BAD_CAST("name")));
         if (attrVal == ""){
            CPPUNIT_FAIL("Font name is empty.");
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testMultiColumnLineSeparator, "multi-column-line-separator-SAVED.docx")
{
    // Check for the Column Separator value.It should be FALSE as the document doesnt contains separator line.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:sectPr/w:cols","sep","false");
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlGrabBag, "customxml.docx")
{
   // The problem was that item[n].xml and itemProps[n].xml and .rels files for item[n].xml
   // files were missing from docx file after saving file.
   // This test case tests whether customxml files grabbagged properly in correct object.

   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   sal_Bool CustomXml = sal_False;
   for(int i = 0; i < aGrabBag.getLength(); ++i)
   {
       if (aGrabBag[i].Name == "OOXCustomXml" || aGrabBag[i].Name == "OOXCustomXmlProps")
       {
           CustomXml = sal_True;
           uno::Reference<xml::dom::XDocument> aCustomXmlDom;
           uno::Sequence<uno::Reference<xml::dom::XDocument> > aCustomXmlDomList;
           CPPUNIT_ASSERT(aGrabBag[i].Value >>= aCustomXmlDomList); // PropertyValue of proper type
           sal_Int32 length = aCustomXmlDomList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(1), length);
           aCustomXmlDom = aCustomXmlDomList[0];
           CPPUNIT_ASSERT(aCustomXmlDom.get()); // Reference not empty
       }
   }
   CPPUNIT_ASSERT(CustomXml); // Grab Bag has all the expected elements
}

DECLARE_OOXMLEXPORT_TEST(testActiveXGrabBag, "activex.docx")
{
   // The problem was that activeX.xml files were missing from docx file after saving file.
   // This test case tests whether activex files grabbagged properly in correct object.

   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   bool bActiveX = sal_False;
   for(int i = 0; i < aGrabBag.getLength(); ++i)
   {
       if (aGrabBag[i].Name == "OOXActiveX")
       {
           bActiveX = sal_True;
           uno::Reference<xml::dom::XDocument> aActiveXDom;
           uno::Sequence<uno::Reference<xml::dom::XDocument> > aActiveXDomList;
           CPPUNIT_ASSERT(aGrabBag[i].Value >>= aActiveXDomList); // PropertyValue of proper type
           sal_Int32 length = aActiveXDomList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(5), length);
           aActiveXDom = aActiveXDomList[0];
           CPPUNIT_ASSERT(aActiveXDom.get()); // Reference not empty
       }
   }
   CPPUNIT_ASSERT(bActiveX); // Grab Bag has all the expected elements
}

DECLARE_OOXMLEXPORT_TEST(testActiveXBinGrabBag, "activexbin.docx")
{
   // The problem was that activeX.bin files were missing from docx file after saving file.
   // This test case tests whether activex bin files grabbagged properly in correct object.

   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   bool bActiveX = sal_False;
   for(int i = 0; i < aGrabBag.getLength(); ++i)
   {
       if (aGrabBag[i].Name == "OOXActiveXBin")
       {
           bActiveX = sal_True;
           uno::Reference<io::XInputStream> aActiveXBin;
           uno::Sequence<uno::Reference<io::XInputStream> > aActiveXBinList;
           CPPUNIT_ASSERT(aGrabBag[i].Value >>= aActiveXBinList); // PropertyValue of proper type
           sal_Int32 length = aActiveXBinList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(5), length);
           aActiveXBin = aActiveXBinList[0];
           CPPUNIT_ASSERT(aActiveXBin.get()); // Reference not empty
       }
   }
   CPPUNIT_ASSERT(bActiveX); // Grab Bag has all the expected elements
}

DECLARE_OOXMLEXPORT_TEST(testFdo69644, "fdo69644.docx")
{
    // The problem was that the exporter exported the table definition
    // with only 3 columns, instead of 5 columns.
    // Check that the table grid is exported with 5 columns
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 5);
}

DECLARE_OOXMLEXPORT_TEST(testCp1000015, "cp1000015.odt")
{
    // Redline and hyperlink end got exported in an incorrect order.
    getParagraph(1, "Hello.");
    getParagraph(2, "http://www.google.com/");
}

DECLARE_OOXMLEXPORT_TEST(testFdo70812, "fdo70812.docx")
{
    // Import just crashed.
    getParagraph(1, "Sample pages document.");
}

DECLARE_OOXMLEXPORT_TEST(testPgMargin, "testPgMargin.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgMar", "left", "1440");
}

DECLARE_OOXMLEXPORT_TEST(testVMLData, "TestVMLData.docx")
{
    // The problem was exporter was exporting vml data for shape in w:rPr element.
    // vml data shoud not come under w:rPr element.
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/w:pict/v:rect", "stroked").match("f"));
}

DECLARE_OOXMLEXPORT_TEST(testImageData, "image_data.docx")
{
    // The problem was exporter was exporting v:imagedata data for shape in w:pict as v:fill w element.

    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/w:pict/v:rect/v:imagedata", "detectmouseclick").match("t"));
}

DECLARE_OOXMLEXPORT_TEST(testImageCrop, "ImageCrop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    ::com::sun::star::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2955 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 5477 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2856 ), aGraphicCropStruct.Top );
    // FIXME import test is disabled (we only check after import-export-import)
    // The reason is that after import this is 2291 -- rounding error?
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2290 ), aGraphicCropStruct.Bottom );
}

DECLARE_OOXMLEXPORT_TEST(testFdo70838, "fdo70838.docx")
{
    // The problem was that VMLExport::Commit didn't save the correct width and height,
    // and ImplEESdrWriter::ImplFlipBoundingBox made a mistake calculating the position

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // get styles of the four shapes
    OUString aStyles[4];
    aStyles[0] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/w:pict[1]/v:rect", "style");
    // original is: "position:absolute;margin-left:97.6pt;margin-top:165pt;width:283.4pt;height:141.7pt;rotation:285"
    aStyles[1] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/w:pict[2]/v:rect", "style");
    // original is: "position:absolute;margin-left:97.6pt;margin-top:164.95pt;width:283.4pt;height:141.7pt;rotation:255"
    aStyles[2] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/w:pict[3]/v:rect", "style");
    // original is: "position:absolute;margin-left:97.5pt;margin-top:164.9pt;width:283.4pt;height:141.7pt;rotation:105"
    aStyles[3] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/w:pict[4]/v:rect", "style");
    // original is: "position:absolute;margin-left:97.55pt;margin-top:164.95pt;width:283.4pt;height:141.7pt;rotation:75"

    //check the size and position of each of the shapes
    for( int i = 0; i < 4; ++i )
    {
        CPPUNIT_ASSERT(!aStyles[i].isEmpty());

        sal_Int32 nextTokenPos = 0;
        do
        {
            OUString aStyleCommand = aStyles[i].getToken( 0, ';', nextTokenPos );
            CPPUNIT_ASSERT(!aStyleCommand.isEmpty());

            OUString aStyleCommandName  = aStyleCommand.getToken( 0, ':' );
            OUString aStyleCommandValue = aStyleCommand.getToken( 1, ':' );

            if( aStyleCommandName.equals( "margin-left" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_EQUAL(0, abs(97.6 - fValue));
            }
            else if( aStyleCommandName.equals( "margin-top" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_EQUAL(0, abs(165 - fValue));
            }
            else if( aStyleCommandName.equals( "width" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_EQUAL(0, abs(283.4 - fValue));
            }
            else if( aStyleCommandName.equals( "height" ) )
            {
                float fValue = aStyleCommandValue.getToken( 0, 'p' ).toFloat();
                CPPUNIT_ASSERT_EQUAL(0, abs(141.7 - fValue));
            }

        } while( nextTokenPos != -1 );
    }
}

DECLARE_OOXMLEXPORT_TEST(testLineSpacingexport, "test_line_spacing.docx")
{
     // The Problem was that the w:line attribute value in w:spacing tag was incorrect
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());

    // FIXME The test passes on most machines (including Linux x86_64 with gcc-4.7), but fails on various configs:
    // Linux arm, Linux x86_64 with gcc-4.8 and Mac. Need to figure out what goes wrong and fix that.
#if 0
    style::LineSpacing alineSpacing = getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(13200), static_cast<sal_Int16>(alineSpacing.Height));
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "line", "31680");
#endif
}

DECLARE_OOXMLEXPORT_TEST(testHyperlineIsEnd, "hyperlink.docx")
{
    // Check  that the document.xml contents all the tag properly closed.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // If  document.xml miss any ending tag then parseExport() returns NULL which fail the test case.
    CPPUNIT_ASSERT(pXmlDoc) ;
    // Check hyperlink is properly open.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink",1);
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxGradientAngle, "fdo65295.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xIndexAccess->getCount());

    // Angle of frame#1 is 135 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame1, "FillStyle"));
    awt::Gradient aGradient1 = getProperty<awt::Gradient>(xFrame1, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(135 * 10), aGradient1.Angle);

    // Angle of frame#2 is 180 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame2, "FillStyle"));
    awt::Gradient aGradient2 = getProperty<awt::Gradient>(xFrame2, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(180 * 10), aGradient2.Angle);

    // Angle of frame#3 is  90 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame3(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame3, "FillStyle"));
    awt::Gradient aGradient3 = getProperty<awt::Gradient>(xFrame3, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 90 * 10), aGradient3.Angle);

    // Angle of frame#4 is 225 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame4(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame4, "FillStyle"));
    awt::Gradient aGradient4 = getProperty<awt::Gradient>(xFrame4, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(225 * 10), aGradient4.Angle);

    // Angle of frame#5 is 270 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame5(xIndexAccess->getByIndex(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame5, "FillStyle"));
    awt::Gradient aGradient5 = getProperty<awt::Gradient>(xFrame5, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270 * 10), aGradient5.Angle);

    // Angle of frame#6 is 315 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame6(xIndexAccess->getByIndex(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame6, "FillStyle"));
    awt::Gradient aGradient6 = getProperty<awt::Gradient>(xFrame6, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(315 * 10), aGradient6.Angle);

    // Angle of frame#7 is   0 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame7(xIndexAccess->getByIndex(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame7, "FillStyle"));
    awt::Gradient aGradient7 = getProperty<awt::Gradient>(xFrame7, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(  0 * 10), aGradient7.Angle);

    // Angle of frame#8 is  45 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame8(xIndexAccess->getByIndex(7), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame8, "FillStyle"));
    awt::Gradient aGradient8 = getProperty<awt::Gradient>(xFrame8, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 45 * 10), aGradient8.Angle);
}

DECLARE_OOXMLEXPORT_TEST(testCellGridSpan, "cell-grid-span.docx")
{
    // The problem was during export gridSpan value for 1st & 2nd cells for test document
    // used to get set wrongly to 5 and 65532 respectively which was the reason for crash during save operation
    // Varifying gridSpan element is not present in RoundTriped Document (As it's Default value is 1).
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:tcPr/w:gridSpan",0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:gridSpan",0);
}
DECLARE_OOXMLEXPORT_TEST(testFdo71646, "fdo71646.docx")
{
    // The problem was after save file created by MS the direction changed to RTL.
    uno::Reference<uno::XInterface> xParaLTRLeft(getParagraph( 1, "LTR LEFT"));
    sal_Int32 nLTRLeft = getProperty< sal_Int32 >( xParaLTRLeft, "ParaAdjust" );
    // test the text Direction value for the pragraph
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLTRLeft, "WritingMode" );

    // this will test the both the text direction and alignment for paragraph
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nLTRLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);
}

DECLARE_OOXMLEXPORT_TEST(testParaAutoSpacing, "para-auto-spacing.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "beforeAutospacing","1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "afterAutospacing","1");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "beforeAutospacing","");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "afterAutospacing","");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "before","400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "after","400");
}

DECLARE_OOXMLEXPORT_TEST(testGIFImageCrop, "test_GIF_ImageCrop.docx")
{
    // FIXME why does this fail on Mac?
#if !defined(MACOSX)
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    ::com::sun::star::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    // FIXME import test is disabled (we only check after import-export-import)
    // The reason is that after import this is 1171 -- why?
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1265 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4256 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1109 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1448 ), aGraphicCropStruct.Bottom );
#endif
}

DECLARE_OOXMLEXPORT_TEST(testPNGImageCrop, "test_PNG_ImageCrop.docx")
{
    // FIXME why does this fail on Mac?
#if !defined(MACOSX)
    /* The problem was image cropping information was not getting saved
     * after roundtrip.
     * Check for presenece of cropping parameters in exported file.
     */
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    ::com::sun::star::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    // FIXME import test is disabled (we only check after import-export-import)
    // The reason is that after import this is 1141 -- why?
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1231 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1295 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1358 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 737 ), aGraphicCropStruct.Bottom );
#endif
}

DECLARE_OOXMLEXPORT_TEST(testFootnoteParagraphTag, "testFootnote.docx")
{
    /* In footnotes.xml, the paragraph tag inside <w:footnote w:id="2"> was getting written into document.xml.
     * Check for, paragraph tag is correctly written into footnotes.xml.
     */
    xmlDocPtr pXmlFootnotes = parseExport("word/footnotes.xml");
    if (!pXmlFootnotes)
        return;
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]","id","2");
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r/w:rPr/w:rStyle","val","Footnotereference");
}

DECLARE_OOXMLEXPORT_TEST(testSpacingLineRule,"table_lineRule.docx")
{
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
     assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "lineRule", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testTableLineSpacing, "table_atleast.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:spacing", "line", "320");
}

DECLARE_OOXMLEXPORT_TEST(testFileOpenInputOutputError,"floatingtbl_with_formula.docx")
{
     // Docx containing Floating table with formula was giving "General input/output error" while opening in LibreOffice
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
     if (!pXmlDoc)
         return;
      assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pStyle", "val", "Normal");
}

DECLARE_OOXMLEXPORT_TEST(testExtraSectionBreak, "1_page.docx")
{
    // There was a problem for some documents during export.Invalid sectPr getting added
    // because of faulty calculation of PageDesc value
    // This was the reason for increasing number of pages after RT
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}


DECLARE_OOXMLEXPORT_TEST(testFdo71785, "fdo71785.docx")
{
    // crashtest
}

DECLARE_OOXMLEXPORT_TEST(testFDO78384,"fdo78384.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:rFonts","ascii","Wingdings");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79008, "fdo79008.docx")
{
    /* File getting crash while saving in LO.
     * Checking if document.xml file is getting created after fix
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testFdo69649, "fdo69649.docx")
{
    // The DOCX containing the Table of Contents was not exported with correct page nos
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[21]/w:hyperlink/w:r[2]/w:t");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match("15"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc884615, "bnc884615.docx")
{
    // The problem was that the shape in the header wasn't in the background.
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(getShape(1), "Opaque")));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
