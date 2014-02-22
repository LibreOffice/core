/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <swmodeltestbase.hxx>

#if !defined(WNT)

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
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
#include <com/sun/star/style/CaseMap.hpp>
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
#include <com/sun/star/style/BreakType.hpp>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>

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
     * Assert that rXPath exists, and has exactly nNumberOfChildNodes child nodes.
     * Useful for checking that we do have a no child nodes to a specific node (nNumberOfChildNodes == 0).
     */
    void assertXPathChildren(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfChildNodes);

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

        
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }
};

xmlDocPtr Test::parseExport(const OUString& rStreamName)
{
    if (!m_bExported)
        return 0;

    
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), m_aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rStreamName), uno::UNO_QUERY);
    boost::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    pStream->Seek(STREAM_SEEK_TO_END);
    sal_Size nSize = pStream->Tell();
    pStream->Seek(0);
    OStringBuffer aDocument(nSize);
    char ch;
    for (sal_Size i = 0; i < nSize; ++i)
    {
        pStream->ReadChar( ch );
        aDocument.append(ch);
    }

    
    return xmlParseMemory((const char*)aDocument.getStr(), aDocument.getLength());
}

xmlNodeSetPtr Test::getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("mc"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wps"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wpg"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wp"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wp14"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pic"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("rels"), BAD_CAST("http:
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w14"), BAD_CAST("http:
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

void Test::assertXPathChildren(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfChildNodes)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
        1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString("XPath '" + rXPath + "' number of child-nodes is incorrect").getStr(),
        nNumberOfChildNodes, (int)xmlChildElementCount(pXmlNode));
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

#if 1
#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)




#else
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


    uno::Reference< container::XNameAccess > paragraphStyles = getStyles( "ParagraphStyles" );
    uno::Reference< beans::XPropertySet > properties( paragraphStyles->getByName( "Standard" ), uno::UNO_QUERY );
    uno::Sequence< style::TabStop > stops = getProperty< uno::Sequence< style::TabStop > >(
        paragraphStyles->getByName( "Standard" ), "ParaTabStops" );

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
    
    CPPUNIT_ASSERT_EQUAL(OUString("http:
}




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


    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (stack {n # k} right ) {x} ^ {k} {a} ^ {n-k}}",
        getFormula( getRun( getParagraph( 2 ), 1 )));
    CHECK_FORMULA( "{left (1+x right )} ^ {n} =1+ {nx} over {1!} + {n left (n-1 right ) {x} ^ {2}} over {2!} +\xe2\x80\xa6",
        getFormula( getRun( getParagraph( 3 ), 1 )));


    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {n\xcf\x80x} over {L} + {b} rsub {n} sin {n\xcf\x80x} over {L} right )}",
        getFormula( getRun( getParagraph( 4 ), 1 )));
    CHECK_FORMULA( "{a} ^ {2} + {b} ^ {2} = {c} ^ {2}", getFormula( getRun( getParagraph( 5 ), 1 )));
    CHECK_FORMULA( "x = {- b \xc2\xb1 sqrt {{b} ^ {2} -4 ac}} over {2 a}",
        getFormula( getRun( getParagraph( 6 ), 1 )));
    CHECK_FORMULA(
        "{e} ^ {x} =1+ {x} over {1!} + {{x} ^ {2}} over {2!} + {{x} ^ {3}} over {3!} +\xe2\x80\xa6,    -\xe2\x88\x9e<x<\xe2\x88\x9e",
        getFormula( getRun( getParagraph( 7 ), 1 )));
    CHECK_FORMULA(


        "sin \xce\xb1 \xc2\xb1 sin \xce\xb2 =2 sin {1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right ) cos {1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )",
        getFormula( getRun( getParagraph( 8 ), 1 )));
    CHECK_FORMULA(


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
        
        uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(), uno::UNO_QUERY);
        xCtrl->select(uno::makeAny(xTable1));
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl, uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect X coord computed from docx",
            xCoordsFromOffice[i], pos.X, 1);

        
        
        
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
    CPPUNIT_ASSERT_EQUAL(OUString("http:
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
    
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testN789482, "n789482.docx")
{
    
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    getRun(xParagraph, 1, "Before. ");

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(getRun(xParagraph, 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(sal_True, getProperty<sal_Bool>(getRun(xParagraph, 2), "IsStart"));

    getRun(xParagraph, 3, "www.test.com");
    CPPUNIT_ASSERT_EQUAL(OUString("http:

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(getRun(xParagraph, 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(sal_False, getProperty<sal_Bool>(getRun(xParagraph, 4), "IsStart"));

    getRun(xParagraph, 5, " After.");
}

/*
 * doesn't work on openSUSE12.2 at least
DECLARE_OOXMLEXPORT_TEST(test1Table1Page, "1-table-1-page.docx")
{
    
    
    
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}
*/

DECLARE_OOXMLEXPORT_TEST(testTextFrames, "textframes.odt")
{
    
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
    
    
    
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(20.f, getProperty<float>(getRun(xPara, 1), "CharHeight"));



}

DECLARE_OOXMLEXPORT_TEST(testMathLiteral, "math-literal.docx")
{
    CHECK_FORMULA( "iiint from {V} to <?> {\"div\" \"F\"}  dV= llint from {S} to <?> {\"F\" \xe2\x88\x99 \"n \" dS}",
        getFormula( getRun( getParagraph( 1 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testFdo48557, "fdo48557.odt")
{
    
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

DECLARE_OOXMLEXPORT_TEST(testFdo64826, "fdo64826.docx")
{
    
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(mxComponent, "RecordChanges")));
}

DECLARE_OOXMLEXPORT_TEST(testPageBackground, "page-background.docx")
{
    
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testPageGraphicBackground, "page-graphic-background.odt")
{
    
    
    
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65265, "fdo65265.docx")
{
    
    uno::Reference<text::XTextRange> xParagraph1 = getParagraph(1);
    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);

    CPPUNIT_ASSERT_EQUAL(OUString("Format"), getProperty<OUString>(getRun(xParagraph1, 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"), getProperty<OUString>(getRun(xParagraph2, 2), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65655, "fdo65655.docx")
{
    
    
    
    
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
    
    CPPUNIT_ASSERT_EQUAL(OUString("SAMPLE"), xShape->getString());

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xShape, "CustomShapeGeometry");
    bool bFound = false;
    for (int i = 0; i < aProps.getLength(); ++i)
        if (aProps[i].Name == "TextPath")
            bFound = true;
    
    CPPUNIT_ASSERT_EQUAL(true, bFound);

    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45 * 100), getProperty<sal_Int32>(xShape, "RotateAngle"));

    
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xShape, "VertOrientRelation"));

    
    
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, "FillTransparence"));
    
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_NONE, getProperty<drawing::LineStyle>(xShape, "LineStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo43093, "fdo43093.docx")
{
    
    uno::Reference<uno::XInterface> xParaRtlLeft(getParagraph( 1, "RTL Left"));
    sal_Int32 nRtlLeft = getProperty< sal_Int32 >( xParaRtlLeft, "ParaAdjust" );
    
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
    
    
    uno::Reference< text::XTextRange > xParagraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > xText = getRun( xParagraph, 2, "This is a simple sentence.");

    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xText, "CharUnderlineHasColor")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), getProperty<sal_Int32>(xText, "CharUnderlineColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65400, "fdo65400.docx")
{
    
    
    
    uno::Reference< text::XTextRange > paragraph1 = getParagraph( 1 );
    uno::Reference< text::XTextRange > shaded = getRun( paragraph1, 2, "normal" );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0x0026 ), getProperty< sal_Int32 >( shaded, "CharShadingValue" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0xd8d8d8 ), getProperty< sal_Int32 >( shaded, "CharBackColor" ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66543, "fdo66543.docx")
{
    
    

    uno::Reference< text::XTextRange > paragraph1 = getParagraph( 1 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1 ), getProperty< sal_Int32 >( paragraph1, "ParaLineNumberStartValue" ));
}

DECLARE_OOXMLEXPORT_TEST(testN822175, "n822175.odt")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, getProperty<text::WrapTextMode>(xFrame, "Surround"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66688, "fdo66688.docx")
{
    
    
    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 80 ), getProperty< sal_Int32 >( xFrame, "BackColorTransparency" ) );
}

DECLARE_OOXMLEXPORT_TEST(testFdo66773, "fdo66773.docx")
{
    
    
    
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
     
     
     
     //
     
     

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders", "offsetFrom", "page");

    
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders/w:left", "space", "24");

    
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgBorders/w:right", "space", "24");
}

DECLARE_OOXMLEXPORT_TEST(testFdo66145, "fdo66145.docx")
{
    
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(xPropertySet, "FirstIsShared")));
}

DECLARE_OOXMLEXPORT_TEST(testGrabBag, "grabbag.docx")
{
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:mirrorIndents");
}

DECLARE_OOXMLEXPORT_TEST(testFdo66781, "fdo66781.docx")
{
    
    
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; 

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "BulletChar")
        {
            CPPUNIT_ASSERT_EQUAL(OUString("\x0", 1, RTL_TEXTENCODING_UTF8), rProp.Value.get<OUString>());
            return;
        }
    }

    
    CPPUNIT_FAIL("Did not find bullet with level 0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo60990, "fdo60990.odt")
{
    
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00CFE7F5), getProperty<sal_Int32>(xShape, "BackColor"));
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00), getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65718, "fdo65718.docx")
{
    
    
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(0) ), getProperty<sal_Int32>(xPropertySet, "TopMargin") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(0) ), getProperty<sal_Int32>(xPropertySet, "BottomMargin") );

    
    
    
    
    
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(114300) + 1 ), getProperty<sal_Int32>(xPropertySet, "LeftMargin") );
    CPPUNIT_ASSERT_EQUAL(sal_Int32( EMU_TO_MM100(114300) + 1), getProperty<sal_Int32>(xPropertySet, "RightMargin") );
}

DECLARE_OOXMLEXPORT_TEST(testFdo64350, "fdo64350.docx")
{
    
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
    
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getParagraph(2), "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int16(TWIP_TO_MM100(24/8*20)), aShadow.ShadowWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTableFloating, "table-floating.docx")
{
    
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11248), getProperty<sal_Int32>(xTables->getByIndex(1), "Width"));

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xFrame, "RightMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTableFloatingMargins, "table-floating-margins.docx")
{
    
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-499), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xFrame, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xFrame, "BottomMargin"));

    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "after", "0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_0, "fdo44689_start_page_0.docx")
{
    
    uno::Reference<beans::XPropertySet> xPara(getParagraph(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xPara, "PageNumberOffset"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo44689_start_page_7, "fdo44689_start_page_7.docx")
{
    
    uno::Reference<beans::XPropertySet> xPara(getParagraph(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), getProperty<sal_Int16>(xPara, "PageNumberOffset"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo67737, "fdo67737.docx")
{
    
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
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:hyperlink", "anchor", "_Toc363553908");
}

DECLARE_OOXMLEXPORT_TEST(testBnc837302, "bnc837302.docx")
{
    
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);

    
    getRun(xParagraph, 3, "AAA");
    
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"), getProperty<OUString>(getRun(xParagraph, 2), "RedlineType"));

    
    xParagraph = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(xParagraph, 1), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo68418, "fdo68418.docx")
{
    
    
    
    
    
    
    
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "FooterText");
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );

    
    CPPUNIT_ASSERT_EQUAL(OUString("aaaa"), xFooterParagraph->getString());        
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
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testCharacterBorder, "charborder.odt")
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
    
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    
    assertXPath(pXmlStyles, "/w:styles/w:style[1]", "styleId", "Normal");
    
    assertXPath(pXmlStyles, "/w:styles/w:style[4]", "styleId", "Heading3");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:next", 0);

    
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:next", "val", "Normal");
    
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > properties(paragraphStyles->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), getProperty<OUString>(properties, "FollowStyle"));

    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(properties, "OutlineLevel"));

    properties = uno::Reference< beans::XPropertySet >(paragraphStyles->getByName("Heading 11"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getProperty<OUString>(properties, "FollowStyle"));

    
    assertXPath(pXmlStyles, "/w:styles/w:style[2]", "styleId", "Heading1");
    
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:pPr/w:ind", 0);

    
    
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/*", 2);
    
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "ascii", "Times New Roman");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:lang", "bidi", "ar-SA");
    
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/*", 0);

    
    uno::Sequence<beans::PropertyValue> aGrabBag = getProperty< uno::Sequence<beans::PropertyValue> >(mxComponent, "InteropGrabBag");
    uno::Sequence<beans::PropertyValue> aLatentStyles;
    for (sal_Int32 i = 0; i < aGrabBag.getLength(); ++i)
        if (aGrabBag[i].Name == "latentStyles")
            aGrabBag[i].Value >>= aLatentStyles;
    CPPUNIT_ASSERT(aLatentStyles.getLength()); 

    
    OUString aCount;
    uno::Sequence<beans::PropertyValue> aLatentStyleExceptions;
    for (sal_Int32 i = 0; i < aLatentStyles.getLength(); ++i)
    {
        if (aLatentStyles[i].Name == "count")
            aCount = aLatentStyles[i].Value.get<OUString>();
        else if (aLatentStyles[i].Name == "lsdExceptions")
            aLatentStyles[i].Value >>= aLatentStyleExceptions;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("371"), aCount); 

    
    uno::Sequence<beans::PropertyValue> aLatentStyleException;
    aLatentStyleExceptions[0].Value >>= aLatentStyleException;
    OUString aName;
    for (sal_Int32 i = 0; i < aLatentStyleException.getLength(); ++i)
        if (aLatentStyleException[i].Name == "name")
            aName = aLatentStyleException[i].Value.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"), aName); 

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']/w:name", "val", "No List");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:tblPr/w:tblCellMar/w:left", "w", "108");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:semiHidden", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:unhideWhenUsed", 1);

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListParagraph']/w:uiPriority", "val", "34");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:qFormat", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:rsid", "val", "00780346");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']", "default", "1");

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:link", "val", "Heading1Char");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:locked", 1);

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']", "customStyle", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']/w:autoRedefine", 1);

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']", "default", "1");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']", "default", "1");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar1, "calendar1.docx")
{
    
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:basedOn", "val", "TableNormal");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rsid", "val", "00903003");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblPr/w:tblStyleColBandSize", "val", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tcPr/w:shd", "val", "clear");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:pPr/w:spacing", "lineRule", "auto");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rPr/w:lang", "eastAsia", "ja-JP");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:wordWrap", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:rFonts", "hAnsiTheme", "minorHAnsi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tblPr", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:vAlign", "val", "bottom");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='lastRow']/w:tcPr/w:tcBorders/w:tr2bl", "val", "nil");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcBorders/w:top", "themeColor", "text1");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar2, "calendar2.docx")
{
    
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharCaseMap"));
    
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));
    
    xCell.set(xTable->getCellByName("B3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(14.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));

    
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:pPr/w:jc", "val", "center");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "val", "en-US");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "bidi", "ar-SA");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:caps", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:smallCaps", "val", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color", "themeColor", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:spacing", "val", "20");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblPr/w:tblBorders/w:insideV", "themeTint", "99");
}

DECLARE_OOXMLEXPORT_TEST(testQuicktables, "quicktables.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:rFonts", "cstheme", "majorBidi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:color", "themeTint", "80");
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color", "themeShade").equalsIgnoreAsciiCase("BF"));

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:pPr/w:snapToGrid", "val", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:rPr/w:bCs", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFill", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFillShade", "80");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind", "rightChars", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind", "right", "144");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcMar/w:bottom", "w", "86");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='LightList']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:spacing", "before", "0");

    
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumList2-Accent1']/w:tblStylePr[@w:type='band1Vert']/w:tcPr/w:shd", "themeFillTint").equalsIgnoreAsciiCase("3F"));

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumShading2-Accent5']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:tcBorders/w:top", "color", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testFdo71302, "fdo71302.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Strong']", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); 

    sal_Bool bTheme = sal_False;
    for(int i = 0; i < aGrabBag.getLength(); ++i)
    {
      if (aGrabBag[i].Name == "OOXTheme")
      {
        bTheme = sal_True;
        uno::Reference<xml::dom::XDocument> aThemeDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aThemeDom); 
        CPPUNIT_ASSERT(aThemeDom.get()); 
      }
    }
    CPPUNIT_ASSERT(bTheme); 

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); 

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xGroup->getCount()); 

    uno::Reference<beans::XPropertySet> xGroupPropertySet(getShape(1), uno::UNO_QUERY);
    xGroupPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); 

    sal_Bool bData = sal_False, bLayout = sal_False, bQStyle = sal_False, bColor = sal_False, bDrawing = sal_False;
    for(int i = 0; i < aGrabBag.getLength(); ++i)
    {
      if (aGrabBag[i].Name == "OOXData")
      {
        bData = sal_True;
        uno::Reference<xml::dom::XDocument> aDataDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aDataDom); 
        CPPUNIT_ASSERT(aDataDom.get()); 
      }
      else if (aGrabBag[i].Name == "OOXLayout")
      {
        bLayout = sal_True;
        uno::Reference<xml::dom::XDocument> aLayoutDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aLayoutDom); 
        CPPUNIT_ASSERT(aLayoutDom.get()); 
      }
      else if (aGrabBag[i].Name == "OOXStyle")
      {
        bQStyle = sal_True;
        uno::Reference<xml::dom::XDocument> aStyleDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aStyleDom); 
        CPPUNIT_ASSERT(aStyleDom.get()); 
      }
      else if (aGrabBag[i].Name == "OOXColor")
      {
        bColor = sal_True;
        uno::Reference<xml::dom::XDocument> aColorDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aColorDom); 
        CPPUNIT_ASSERT(aColorDom.get()); 
      }
      else if (aGrabBag[i].Name == "OOXDrawing")
      {
        bDrawing = sal_True;
        uno::Reference<xml::dom::XDocument> aDrawingDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= aDrawingDom); 
        CPPUNIT_ASSERT(aDrawingDom.get()); 
      }
    }
    CPPUNIT_ASSERT(bData && bLayout && bQStyle && bColor && bDrawing); 

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(0), uno::UNO_QUERY);
    OUString nValue;
    xPropertySet->getPropertyValue("Name") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(OUString("RenderedShapes"), nValue); 
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
    
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox", "style").match("mso-layout-flow-alt:bottom-to-top"));
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr", "vert", "vert270");
}

DECLARE_OOXMLEXPORT_TEST(testCharHighlight, "char_highlight.docx")
{
    const uno::Reference< text::XTextRange > xPara = getParagraph(1);
    
    const sal_Int32 nBackColor(0x4F81BD);
    for( int nRun = 1; nRun <= 16; ++nRun )
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,nRun), uno::UNO_QUERY);
        sal_Int32 nHighlightColor = 0;
        switch( nRun )
        {
            case 1: nHighlightColor = 0x000000; break; 
            case 2: nHighlightColor = 0x0000ff; break; 
            case 3: nHighlightColor = 0x00ffff; break; 
            case 4: nHighlightColor = 0x00ff00; break; 
            case 5: nHighlightColor = 0xff00ff; break; 
            case 6: nHighlightColor = 0xff0000; break; 
            case 7: nHighlightColor = 0xffff00; break; 
            case 8: nHighlightColor = 0xffffff; break; 
            case 9: nHighlightColor = 0x000080;  break;
            case 10: nHighlightColor = 0x008080; break; 
            case 11: nHighlightColor = 0x008000; break; 
            case 12: nHighlightColor = 0x800080; break; 
            case 13: nHighlightColor = 0x800000; break; 
            case 14: nHighlightColor = 0x808000; break; 
            case 15: nHighlightColor = 0x808080; break; 
            case 16: nHighlightColor = 0xC0C0C0; break; 
        }
        CPPUNIT_ASSERT_EQUAL(nHighlightColor, getProperty<sal_Int32>(xRun,"CharHighlight"));
        CPPUNIT_ASSERT_EQUAL(nBackColor, getProperty<sal_Int32>(xRun,"CharBackColor"));
    }

    
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,17), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0C0C0), getProperty<sal_Int32>(xRun,"CharHighlight"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharBackColor"));
    }

    
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,18), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_TRANSPARENT), getProperty<sal_Int32>(xRun,"CharHighlight"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000ff), getProperty<sal_Int32>(xRun,"CharBackColor"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFontNameIsEmpty, "font-name-is-empty.docx")
{
    
    

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
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:sectPr/w:cols","sep","false");
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlGrabBag, "customxml.docx")
{
   
   
   

   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); 
   sal_Bool CustomXml = sal_False;
   for(int i = 0; i < aGrabBag.getLength(); ++i)
   {
       if (aGrabBag[i].Name == "OOXCustomXml" || aGrabBag[i].Name == "OOXCustomXmlProps")
       {
           CustomXml = sal_True;
           uno::Reference<xml::dom::XDocument> aCustomXmlDom;
           uno::Sequence<uno::Reference<xml::dom::XDocument> > aCustomXmlDomList;
           CPPUNIT_ASSERT(aGrabBag[i].Value >>= aCustomXmlDomList); 
           sal_Int32 length = aCustomXmlDomList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(1), length);
           aCustomXmlDom = aCustomXmlDomList[0];
           CPPUNIT_ASSERT(aCustomXmlDom.get()); 
       }
   }
   CPPUNIT_ASSERT(CustomXml); 
}

DECLARE_OOXMLEXPORT_TEST(testActiveXGrabBag, "activex.docx")
{
   
   

   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); 
   bool bActiveX = false;
   for(int i = 0; i < aGrabBag.getLength(); ++i)
   {
       if (aGrabBag[i].Name == "OOXActiveX")
       {
           bActiveX = true;
           uno::Reference<xml::dom::XDocument> aActiveXDom;
           uno::Sequence<uno::Reference<xml::dom::XDocument> > aActiveXDomList;
           CPPUNIT_ASSERT(aGrabBag[i].Value >>= aActiveXDomList); 
           sal_Int32 length = aActiveXDomList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(5), length);
           aActiveXDom = aActiveXDomList[0];
           CPPUNIT_ASSERT(aActiveXDom.get()); 
       }
   }
   CPPUNIT_ASSERT(bActiveX); 
}

DECLARE_OOXMLEXPORT_TEST(testActiveXBinGrabBag, "activexbin.docx")
{
   
   

   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue(OUString("InteropGrabBag")) >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); 
   bool bActiveX = false;
   for(int i = 0; i < aGrabBag.getLength(); ++i)
   {
       if (aGrabBag[i].Name == "OOXActiveXBin")
       {
           bActiveX = true;
           uno::Reference<io::XInputStream> aActiveXBin;
           uno::Sequence<uno::Reference<io::XInputStream> > aActiveXBinList;
           CPPUNIT_ASSERT(aGrabBag[i].Value >>= aActiveXBinList); 
           sal_Int32 length = aActiveXBinList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(5), length);
           aActiveXBin = aActiveXBinList[0];
           CPPUNIT_ASSERT(aActiveXBin.get()); 
       }
   }
   CPPUNIT_ASSERT(bActiveX); 
}

DECLARE_OOXMLEXPORT_TEST(testFdo69644, "fdo69644.docx")
{
    
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 5);
}

DECLARE_OOXMLEXPORT_TEST(testCp1000015, "cp1000015.odt")
{
    
    getParagraph(1, "Hello.");
    getParagraph(2, "http:
}

DECLARE_OOXMLEXPORT_TEST(testFdo70812, "fdo70812.docx")
{
    
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
    
    
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "stroked").match("f"));
}

DECLARE_OOXMLEXPORT_TEST(testImageData, "image_data.docx")
{
    

    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:imagedata", "detectmouseclick").match("t"));
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
    
    
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2290 ), aGraphicCropStruct.Bottom );
}

DECLARE_OOXMLEXPORT_TEST(testFdo70838, "fdo70838.docx")
{
    
    

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    
    OUString aStyles[4];
    aStyles[0] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:rect", "style");
    
    aStyles[1] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Fallback/w:pict/v:rect", "style");
    
    aStyles[2] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Fallback/w:pict/v:rect", "style");
    
    aStyles[3] = getXPath( pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[4]/mc:Fallback/w:pict/v:rect", "style");
    

    
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
     
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());

    
    
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
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    CPPUNIT_ASSERT(pXmlDoc) ;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink",1);
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxGradientAngle, "fdo65295.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xIndexAccess->getCount());

    
    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame1, "FillStyle"));
    awt::Gradient aGradient1 = getProperty<awt::Gradient>(xFrame1, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(135 * 10), aGradient1.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame2, "FillStyle"));
    awt::Gradient aGradient2 = getProperty<awt::Gradient>(xFrame2, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(180 * 10), aGradient2.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame3(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame3, "FillStyle"));
    awt::Gradient aGradient3 = getProperty<awt::Gradient>(xFrame3, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 90 * 10), aGradient3.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame4(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame4, "FillStyle"));
    awt::Gradient aGradient4 = getProperty<awt::Gradient>(xFrame4, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(225 * 10), aGradient4.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame5(xIndexAccess->getByIndex(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame5, "FillStyle"));
    awt::Gradient aGradient5 = getProperty<awt::Gradient>(xFrame5, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270 * 10), aGradient5.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame6(xIndexAccess->getByIndex(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame6, "FillStyle"));
    awt::Gradient aGradient6 = getProperty<awt::Gradient>(xFrame6, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(315 * 10), aGradient6.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame7(xIndexAccess->getByIndex(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame7, "FillStyle"));
    awt::Gradient aGradient7 = getProperty<awt::Gradient>(xFrame7, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(  0 * 10), aGradient7.Angle);

    
    uno::Reference<beans::XPropertySet> xFrame8(xIndexAccess->getByIndex(7), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame8, "FillStyle"));
    awt::Gradient aGradient8 = getProperty<awt::Gradient>(xFrame8, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 45 * 10), aGradient8.Angle);
}

DECLARE_OOXMLEXPORT_TEST(testCellGridSpan, "cell-grid-span.docx")
{
    
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:tcPr/w:gridSpan",0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:gridSpan",0);
}
DECLARE_OOXMLEXPORT_TEST(testFdo71646, "fdo71646.docx")
{
    
    uno::Reference<uno::XInterface> xParaLTRLeft(getParagraph( 1, "LTR LEFT"));
    sal_Int32 nLTRLeft = getProperty< sal_Int32 >( xParaLTRLeft, "ParaAdjust" );
    
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLTRLeft, "WritingMode" );

    
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
    
#if !defined(MACOSX)
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    ::com::sun::star::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    
    
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1265 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 4256 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1109 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1448 ), aGraphicCropStruct.Bottom );
#endif
}

DECLARE_OOXMLEXPORT_TEST(testPNGImageCrop, "test_PNG_ImageCrop.docx")
{
    
#if !defined(MACOSX)
    /* The problem was image cropping information was not getting saved
     * after roundtrip.
     * Check for presenece of cropping parameters in exported file.
     */
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    ::com::sun::star::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    
    
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

DECLARE_OOXMLEXPORT_TEST(testOoxmlTriangle, "ooxml-triangle.docx")
{
    
    
    getShape(1);
}

DECLARE_OOXMLEXPORT_TEST(testMce, "mce.docx")
{
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x9bbb59), getProperty<sal_Int32>(getShape(1), "FillColor"));
}

DECLARE_OOXMLEXPORT_TEST(testThemePreservation, "theme-preservation.docx")
{
    
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "asciiTheme", "minorHAnsi");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "cstheme", "minorBidi");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:rPr/w:rFonts", "eastAsiaTheme", "minorEastAsia");

    
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color", "themeColor", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color", "themeTint", "99");

    
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts", "hAnsiTheme", "majorHAnsi");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts", "asciiTheme", "majorHAnsi");

    
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color", "themeColor", "accent3");
    OUString sThemeShade = getXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color", "themeShade");
    CPPUNIT_ASSERT_EQUAL(sThemeShade.toInt32(16), sal_Int32(0xbf));
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[7]/w:r[1]/w:rPr/w:color", "themeColor", "accent1");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[7]/w:r[1]/w:rPr/w:color", "themeTint", "99");

    
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang", "val", "en-US");
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang", "eastAsia", "zh-CN");
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang", "bidi", "he-IL");

    
    sal_Unicode fontName[2]; 
    fontName[0] = 0x5b8b;
    fontName[1] = 0x4f53;
    CPPUNIT_ASSERT_EQUAL(OUString(fontName, 2), getProperty<OUString>(getParagraph(1), "CharFontNameAsian"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"),
                         getProperty<OUString>(getParagraph(2), "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(OUString("Trebuchet MS"),
                         getProperty<OUString>(getParagraph(3, "Default style theme font"), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Black"),
                         getProperty<OUString>(getRun(getParagraph(4, "Direct format font"), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Trebuchet MS"),
                         getProperty<OUString>(getParagraph(5, "Major theme font"), "CharFontName"));

    
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeFill", "text2");
}

DECLARE_OOXMLEXPORT_TEST(testcantSplit, "2_table_doc.docx")
{
    
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:cantSplit",0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:cantSplit","val","true");
}

DECLARE_OOXMLEXPORT_TEST(testExtraSectionBreak, "1_page.docx")
{
    
    
    
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testcolumnbreak, "columnbreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(5, "This is first line after col brk."), "BreakType"));
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[1]/w:br", "type", "column");
}

DECLARE_OOXMLEXPORT_TEST(testGlossary, "testGlossary.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/glossary/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:glossaryDocument", "Ignorable", "w14 wp14");
}

DECLARE_OOXMLEXPORT_TEST(testFdo71785, "fdo71785.docx")
{
    
}

DECLARE_OOXMLEXPORT_TEST(testCrashWhileSave, "testCrashWhileSave.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/footer1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:ftr/w:tbl/w:tr/w:tc[1]/w:p[1]/w:pPr/w:pStyle", "val").match("Normal"));
}

DECLARE_OOXMLEXPORT_TEST(testFileOpenInputOutputError,"floatingtbl_with_formula.docx")
{
     
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
     if (!pXmlDoc)
         return;
      assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pStyle", "val", "Normal");
}

DECLARE_OOXMLEXPORT_TEST(testRelorientation, "relorientation.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_RIGHT, getProperty<sal_Int16>(xShape, "HoriOrientRelation"));

    uno::Reference<drawing::XShapes> xGroup(xShape, uno::UNO_QUERY);
    
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"), xShapeDescriptor->getShapeType());

    
    if (m_bExported)
    {
        uno::Reference<drawing::XShape> xYear(xGroup->getByIndex(1), uno::UNO_QUERY);
        
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), xYear->getSize().Width);
    }
}

DECLARE_OOXMLEXPORT_TEST(testBezier, "bezier.odt")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testFdo73215, "fdo73215.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:prstGeom",
                "prst", "rect");
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp[9]/wps:spPr/a:prstGeom/a:avLst/a:gd",
                "name", "adj1");
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTextbox, "groupshape-textbox.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    
    
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xShape->getString());
    
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapePicture, "groupshape-picture.docx")
{
    
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GraphicObjectShape"), xShapeDescriptor->getShapeType());
}

DECLARE_OOXMLEXPORT_TEST(testAutofit, "autofit.docx")
{
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(getShape(1), "FrameIsAutomaticHeight")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(getShape(2), "FrameIsAutomaticHeight")));
}

DECLARE_OOXMLEXPORT_TEST(testDmlShapeTitle, "dml-shape-title.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), getProperty<OUString>(getShape(1), "Title"));
    CPPUNIT_ASSERT_EQUAL(OUString("Description"), getProperty<OUString>(getShape(1), "Description"));
}

DECLARE_OOXMLEXPORT_TEST(testFormControl, "form-control.docx")
{
    if (!m_bExported)
        return;
    
    getParagraph(1, "Foo [Date] bar.");
}

DECLARE_OOXMLEXPORT_TEST(testBehinddoc, "behinddoc.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor", "behindDoc", "1");
}

DECLARE_OOXMLEXPORT_TEST(testDmlZorder, "dml-zorder.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor", "relativeHeight", "2");
}

DECLARE_OOXMLEXPORT_TEST(testDmlShapeRelsize, "dml-shape-relsize.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp14:sizeRelH", "relativeFrom", "margin");
}

DECLARE_OOXMLEXPORT_TEST(testDmlPictureInTextframe, "dml-picture-in-textframe.docx")
{
    if (!m_bExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), m_aTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/media/image1.gif")));
    
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/image2.gif")));
}

DECLARE_OOXMLEXPORT_TEST(testDmlGroupshapeRelsize, "dml-groupshape-relsize.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp14:sizeRelH", "relativeFrom", "margin");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedParagraphMark, "testTrackChangesDeletedParagraphMark.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedParagraphMark, "testTrackChangesInsertedParagraphMark.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:ins");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedTableRow, "testTrackChangesDeletedTableRow.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:del");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedTableRow, "testTrackChangesInsertedTableRow.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:ins");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedTableCell, "testTrackChangesDeletedTableCell.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellDel");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedTableCell, "testTrackChangesInsertedTableCell.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellIns");
}

DECLARE_OOXMLEXPORT_TEST(testFdo69649, "fdo69649.docx")
{
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[21]/w:hyperlink/w:r[2]/w:t");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match("15"));
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxPictureFill, "textbox_picturefill.docx")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    CPPUNIT_ASSERT(!(getProperty<OUString>(xFrame,"BackGraphicURL")).isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testFDO73034, "FDO73034.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:u", "val").match("single"));
}

DECLARE_OOXMLEXPORT_TEST(testSmartArtAnchoredInline, "fdo73227.docx")
{
    /* Given file conatins 3 DrawingML objects as 1Picture,1SmartArt and 1Shape.
     * Check for SmartArt.
    *  SmartArt shoould get written as "Floating Object" i.e. inside <wp:anchor> tag.
    *  Also check for value of attribute "id" of <wp:docPr> . It should be unique for
    *  all 3 DrawingML objects in a document.
    */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[1]/wp:anchor/wp:docPr","id","1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[1]/wp:anchor/wp:docPr","name","Diagram1");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr","id","2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr","name","10-Point Star 3");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr","id","3");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing[2]/wp:anchor/wp:docPr","name","Picture");
}

DECLARE_OOXMLEXPORT_TEST(testFDO71834, "fdo71834.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[4]/w:tr[2]/w:tc[1]/w:tcPr[1]/w:tcW[1]","type", "dxa");
}

DECLARE_OOXMLEXPORT_TEST(testFieldFlagO,"TOC_field_f.docx")
{
   
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    
    
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" TOC \\z \\f \\o \"1-3\" \\u \\h"));
}

DECLARE_OOXMLEXPORT_TEST(testTOCFlag_f, "toc_doc.docx")
{
    
    
    
    
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    
    
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.endsWith("\\h"));
}

DECLARE_OOXMLEXPORT_TEST(testPreserveZfield,"preserve_Z_field_TOC.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" TOC \\z \\f \\o \"1-3\" \\h"));
}

 DECLARE_OOXMLEXPORT_TEST(testPreserveWfieldTOC, "PreserveWfieldTOC.docx")
{
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
   CPPUNIT_ASSERT(contents.match(" TOC \\z \\w \\f \\o \"1-3\" \\h"));
}

DECLARE_OOXMLEXPORT_TEST(testFieldFlagB,"TOC_field_b.docx")
{
   
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    
    
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" TOC \\b \"bookmark111\" \\o \"1-9\" \\h"));
}

DECLARE_OOXMLEXPORT_TEST(testPreserveXfieldTOC, "PreserveXfieldTOC.docx")
{
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
   CPPUNIT_ASSERT(contents.match(" TOC \\x \\f \\o \"1-3\" \\h"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65833, "fdo65833.docx")
{
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:group", "editas", "canvas");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73247, "fdo73247.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm",
        "rot", "1969698");
}

DECLARE_OOXMLEXPORT_TEST(testFdo70942, "fdo70942.docx")
{
     xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "ellipse");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesParagraphProperties, "testTrackChangesParagraphProperties.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange", 0);
}

DECLARE_OOXMLEXPORT_TEST(testMsoSpt180, "mso-spt180.docx")
{
    if (!m_bExported)
        return;

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xGroup->getByIndex(0), "CustomShapeGeometry");
    OUString aType;
    for (int i = 0; i < aProps.getLength(); ++i)
        if (aProps[i].Name == "Type")
            aType = aProps[i].Value.get<OUString>();
    
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-borderCallout1"), aType);
}

DECLARE_OOXMLEXPORT_TEST(testFdo73550, "fdo73550.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:rFonts");
}

DECLARE_OOXMLEXPORT_TEST(testDmlTextshape, "dml-textshape.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xShape, "FillStyle"));
    
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, getProperty<drawing::LineStyle>(xShape, "LineStyle"));

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp[2]/wps:bodyPr", "wrap", "square");

    xShape.set(xGroup->getByIndex(3), uno::UNO_QUERY);
    OUString aType = comphelper::SequenceAsHashMap(getProperty<beans::PropertyValues>(xShape, "CustomShapeGeometry"))["Type"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-bentConnector3"), aType);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4018), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1256), xShape->getPosition().Y);

    xShape.set(xGroup->getByIndex(5), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1016), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testDrawinglayerPicPos, "drawinglayer-pic-pos.docx")
{
    
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    OString aXPath("/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:xfrm/a:off");
    
    assertXPath(pXmlDocument, aXPath, "x", "0");
    
    assertXPath(pXmlDocument, aXPath, "y", "0");
}

DECLARE_OOXMLEXPORT_TEST(testPageRelSize, "pagerelsize.docx")
{
    
    uno::Reference<drawing::XShape> xTextFrame = getTextFrameByName("Frame1");
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeWidthRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeHeightRelation"));

    
    xTextFrame = getTextFrameByName("Text Box 2");
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeHeightRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeWidthRelation"));
}

DECLARE_OOXMLEXPORT_TEST(testRelSizeRound, "rel-size-round.docx")
{
    
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), getProperty<sal_Int16>(getShape(1), "RelativeHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testShapeThemePreservation, "shape-theme-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef",
            "idx", "2");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:lnRef/a:schemeClr/a:shade",
            "val", "50000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:effectRef",
            "idx", "0");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:effectRef/a:schemeClr",
            "val", "accent1");

    
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill",
            0);
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill",
            0);

    
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr",
            "val", "accent6");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:schemeClr",
            "val", "accent3");

    
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr/a:lumMod",
            "val", "40000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:schemeClr/a:lumOff",
            "val", "60000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:schemeClr/a:lumMod",
            "val", "50000");

    
    OUString sFillColor = getXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:solidFill/a:srgbClr",
            "val");
    CPPUNIT_ASSERT_EQUAL(sFillColor.toInt32(16), sal_Int32(0x00b050));
    sal_Int32 nLineColor = getXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill/a:srgbClr",
            "val").toInt32(16);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), nLineColor);

    
    sal_Int32 nLineWidth = getXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln",
            "w").toInt32();
    CPPUNIT_ASSERT(abs(63500 - nLineWidth) < 1000); 
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:miter",
            1);
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:custDash",
            1);

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);
    uno::Reference<drawing::XShape> xShape3 = getShape(3);

    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), getProperty<sal_Int32>(xShape1, "FillColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xfcd5b5), getProperty<sal_Int32>(xShape2, "FillColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00b050), getProperty<sal_Int32>(xShape3, "FillColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x3a5f8b), getProperty<sal_Int32>(xShape1, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f6228), getProperty<sal_Int32>(xShape2, "LineColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(xShape3, "LineColor"));

    
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, getProperty<drawing::LineStyle>(xShape1, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, getProperty<drawing::LineStyle>(xShape2, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH,  getProperty<drawing::LineStyle>(xShape3, "LineStyle"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_ROUND, getProperty<drawing::LineJoint>(xShape1, "LineJoint"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_ROUND, getProperty<drawing::LineJoint>(xShape2, "LineJoint"));
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_MITER, getProperty<drawing::LineJoint>(xShape3, "LineJoint"));
}

DECLARE_OOXMLEXPORT_TEST(testTOCFlag_u,"testTOCFlag_u.docx")
{
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    
    
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" TOC \\z \\o \"1-9\" \\u \\h"));
}

DECLARE_OOXMLEXPORT_TEST(testTestTitlePage, "testTitlePage.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), parseDump("/root/page[2]/footer/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testTableRowDataDisplayedTwice,"table-row-data-displayed-twice.docx")
{
    
    
    
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testFDO73546, "FDO73546.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:hdr/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor", "distL","0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo69616, "fdo69616.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Fallback/w:pict/v:group", "coordorigin").match("696,725"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo73556,"fdo73556.docx")
{
    /*
    *  The file contains a table with 3 columns
    *  the girdcols are as follows: {1210, 1331, 1210}
    *  whereas the individual cells have {1210, 400, 1210}
    *  The table column separators were taken from the Grid, while
    *  the table width was calculated as 2820 from cells instead
    *  of 3751 from the Grid.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 3);
    sal_Int32 tableWidth = 0;
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[1]", "w").toInt32();
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]", "w").toInt32();
    tableWidth += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[3]", "w").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3751), tableWidth);
}

DECLARE_OOXMLEXPORT_TEST(testAlignForShape,"Shape.docx")
{
    
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:positionH/wp:align","1");
}

DECLARE_OOXMLEXPORT_TEST(testLineStyle_DashType, "LineStyle_DashType.docx")
{
    /* DOCX contatining Shape with LineStyle as Dash Type should get preserved inside
     * an XMl tag <a:prstDash> with value "dash".
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:prstDash", "val", "dash");
}

DECLARE_OOXMLEXPORT_TEST(testDMLSolidfillAlpha, "dml-solidfill-alpha.docx")
{
    
    
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(70), getProperty<sal_Int16>(xShape, "FillTransparence"));

    
    xShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, "FillTransparence"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLCustomGeometry, "dml-customgeometry-cubicbezier.docx")
{

    
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
    uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            rProp.Value >>= aPairs;
        else if (rProp.Name == "Segments")
            rProp.Value >>= aSegments;
    }

    
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[0].Count);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::MOVETO), aSegments[0].Command );

    
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), aSegments[1].Count);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CURVETO), aSegments[1].Command );

    
    sal_Int32 nLength = 16;
    CPPUNIT_ASSERT_EQUAL(nLength, aPairs.getLength());
    std::pair<sal_Int32,sal_Int32> aCoordinates[] =
    {
        std::pair<sal_Int32,sal_Int32>(607, 0),
        std::pair<sal_Int32,sal_Int32>(450, 44),
        std::pair<sal_Int32,sal_Int32>(300, 57),
        std::pair<sal_Int32,sal_Int32>(176, 57),
        std::pair<sal_Int32,sal_Int32>(109, 57),
        std::pair<sal_Int32,sal_Int32>(49, 53),
        std::pair<sal_Int32,sal_Int32>(0, 48),
        std::pair<sal_Int32,sal_Int32>(66, 58),
        std::pair<sal_Int32,sal_Int32>(152, 66),
        std::pair<sal_Int32,sal_Int32>(251, 66),
        std::pair<sal_Int32,sal_Int32>(358, 66),
        std::pair<sal_Int32,sal_Int32>(480, 56),
        std::pair<sal_Int32,sal_Int32>(607, 27),
        std::pair<sal_Int32,sal_Int32>(607, 0),
        std::pair<sal_Int32,sal_Int32>(607, 0),
        std::pair<sal_Int32,sal_Int32>(607, 0)
    };

    for( int i = 0; i < nLength; ++i )
    {
        CPPUNIT_ASSERT_EQUAL(aCoordinates[i].first, aPairs[i].First.Value.get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(aCoordinates[i].second, aPairs[i].Second.Value.get<sal_Int32>());
    }
}

DECLARE_OOXMLEXPORT_TEST(testDmlRectangleRelsize, "dml-rectangle-relsize.docx")
{
    
    
    CPPUNIT_ASSERT(getShape(1)->getSize().Height > 21000);

    
    CPPUNIT_ASSERT(getShape(2)->getSize().Height > 300);
}

DECLARE_OOXMLEXPORT_TEST(testGradientFillPreservation, "gradient-fill-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr",
            "val", "ffff00");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr",
            "val", "ffff33");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[3]/a:srgbClr",
            "val", "ff0000");

    
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='0']/a:schemeClr",
            "val", "accent5");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='100000']/a:schemeClr",
            "val", "accent1");

    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:srgbClr/a:alpha",
            "val", "20000");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:schemeClr/a:tint",
            "val", "44500");
    assertXPath(pXmlDocument,
            "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill/a:gsLst/a:gs[@pos='50000']/a:schemeClr/a:satMod",
            "val", "160000");
}

/* FIXME this still crashes
DECLARE_OOXMLEXPORT_TEST(testSegFaultWhileSave, "test_segfault_while_save.docx")
{
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]", "w").match("6138"));
}*/

DECLARE_OOXMLEXPORT_TEST(testDMLTextFrameVertAdjust, "dml-textframe-vertadjust.docx")
{
    
    

    
    uno::Reference<beans::XPropertySet> xFrame(getTextFrameByName("Rectangle 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    
    xFrame.set(getTextFrameByName("Rectangle 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    
    xFrame.set(getTextFrameByName("Rectangle 3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLShapeFillBitmapCrop, "dml-shape-fillbitmapcrop.docx")
{
    
    

    
    text::GraphicCrop aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? 454 : 455 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? 367 : 368 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? -454 : -455 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(m_bExported ? -367 : -368 ), aGraphicCropStruct.Bottom );

    
    aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), aGraphicCropStruct.Bottom );

}

DECLARE_OOXMLEXPORT_TEST(fdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblW","type","auto");
}

DECLARE_OOXMLEXPORT_TEST(testLineStyle_DashType_VML, "LineStyle_DashType_VML.docx")
{
    /* DOCX contatining "Shape with text inside" having Line Style as "Dash Type" should get
     * preserved inside an XML tag <v:stroke> with attribute dashstyle having value "dash".
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:stroke", "dashstyle", "dash");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73541,"fdo73541.docx")
{
    
    
    xmlDocPtr pXmlDoc = parseExport("word/settings.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:settings/w:mirrorMargins");
}

DECLARE_OOXMLEXPORT_TEST(testFDO74106, "FDO74106.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt", "val","hebrew1");
}

DECLARE_OOXMLEXPORT_TEST(testFDO74215, "FDO74215.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:numbering/w:numPicBullet[2]/w:pict/v:shape", "style", "width:6.4pt;height:6.4pt");
}

DECLARE_OOXMLEXPORT_TEST(testFdo74110,"fdo74110.docx")
{
    /*
    The File contains word art which is being exported as shape and the mapping is defaulted to
    shape type rect since the actual shape type(s) is/are commented out for some reason.
    The actual shape type(s) has/have adjustment value(s) where as rect does not have adjustment value.
    Hence the following test case.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]",
                "prst", "rect");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]/a:avLst[1]/a:gd[1]",0);
}

DECLARE_OOXMLEXPORT_TEST(testColumnBreak_ColumnCountIsZero,"fdo74153.docx")
{
    /* fdo73545: Column Break with Column_count = 0 was not getting preserved.
     * The <w:br w:type="column" /> was missing after roundtrip
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:br","type","column");
}

DECLARE_OOXMLEXPORT_TEST(testDMLShapeFillPattern, "dml-shape-fillpattern.docx")
{
    

    
    drawing::Hatch aHatch = getProperty<drawing::Hatch>(getShape(1), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x99FF66), aHatch.Color);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(2), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(3), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(900), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(4), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(900), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(5), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(6), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(7), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1350), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(8), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1350), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_SINGLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(9), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(10), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(11), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);

    
    aHatch = getProperty<drawing::Hatch>(getShape(12), "FillHatch");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(450), aHatch.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aHatch.Distance);
    CPPUNIT_ASSERT_EQUAL(drawing::HatchStyle_DOUBLE, aHatch.Style);
}

DECLARE_OOXMLEXPORT_TEST(testIndentation, "test_indentation.docx")
{
    
    
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:ind", "end", "");
}

DECLARE_OOXMLEXPORT_TEST(testCaption1, "EquationAsScientificNumbering.docx")
{
    
    

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p/w:r[3]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" SEQ scientific \\* ROMAN"));
}



DECLARE_OOXMLEXPORT_TEST(testCaption2, "EquationWithAboveAndBelowCaption.docx")
{
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[5]/w:r[3]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" SEQ Equation \\* ARABIC"));
}


DECLARE_OOXMLEXPORT_TEST(testCaption3, "FigureAsLabelPicture.docx")
{
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[2]/w:r[3]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" SEQ picture \\* ARABIC"));
}

DECLARE_OOXMLEXPORT_TEST(testCaption4, "TableWithAboveCaptions.docx")
{
    
    
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[3]/w:instrText");
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString contents = OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
    CPPUNIT_ASSERT(contents.match(" SEQ Table \\* ARABIC"));
}

DECLARE_OOXMLEXPORT_TEST(testOuterShdw,"testOuterShdw.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:effectLst[1]/a:outerShdw[1]", "dist", "57811035");
}

DECLARE_OOXMLEXPORT_TEST(testExtentValue, "fdo74605.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent","cx","0");
}

DECLARE_OOXMLEXPORT_TEST( testChildNodesOfCubicBezierTo, "FDO74774.docx")
{
    /* Number of children required by cubicBexTo is 3 of type "pt".
       While exporting, sometimes the child nodes are less than 3.
       The test case ensures that there are 3 child nodes of type "pt"
       for cubicBexTo
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath( pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[3]/wps:spPr[1]/a:custGeom[1]/a:pathLst[1]/a:path[1]/a:cubicBezTo[2]/a:pt[3]");
}

DECLARE_OOXMLEXPORT_TEST(testChartInFooter, "chart-in-footer.docx")
{
    
    
    
    xmlDocPtr pXmlDoc = parseExport("word/_rels/footer1.xml.rels");
    if(!pXmlDoc)
        return;

    
    
    assertXPath(pXmlDoc,"/rels:Relationships/rels:Relationship","Id","rId1");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        
        uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount()); 
    }
}

DECLARE_OOXMLEXPORT_TEST(testNestedTextFrames, "nested-text-frames.odt")
{
    

    
    
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(3), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablePosition, "floating-table-position.docx")
{
    
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5964), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8133), getProperty<sal_Int32>(xFrame, "VertOrientPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeChildPosition, "dml-groupshape-childposition.docx")
{
    
    

    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xChildGroup(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? -2119 : -2121), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? 11338 : 11335), xChildGroup->getPosition().Y);

    xGroup.set(xChildGroup, uno::UNO_QUERY);
    xChildGroup.set(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? -1856 : -1858), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? 11338 : 11335), xChildGroup->getPosition().Y);

    xChildGroup.set(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? -2119 : -2121), xChildGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(m_bExported ? 14028 : 14025), xChildGroup->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testDMLGradientFillTheme, "dml-gradientfill-theme.docx")
{
    
    
    
    
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    
    assertXPath(pXmlDoc,
            "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:gradFill",
            0);

    
    assertXPath(pXmlDoc,
            "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef",
            "idx", "2");
    assertXPath(pXmlDoc,
            "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:style/a:fillRef/a:schemeClr",
            "val", "accent1");
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeParaSpacing, "dml-groupshape-paraspacing.docx")
{
    
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();

    
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    style::LineSpacing aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(150), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    
    xRun.set(getRun(getParagraphOfText(2, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    
    xRun.set(getRun(getParagraphOfText(3, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::MINIMUM), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(847), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    
    xRun.set(getRun(getParagraphOfText(4, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(175), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    
    xRun.set(getRun(getParagraphOfText(5, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(741), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    
    xRun.set(getRun(getParagraphOfText(6, xText),1));
    aLineSpacing = getProperty<style::LineSpacing>(xRun, "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(423), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));

    
    xRun.set(getRun(getParagraphOfText(7, xText),1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun, "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testW14TextEffects, "TextEffects.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow", "rad").match("63500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow", "rad").match("228600"));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
