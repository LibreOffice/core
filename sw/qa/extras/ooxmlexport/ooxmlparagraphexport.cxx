/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <comphelper/propertysequence.hxx>
#include <svx/svdpage.hxx>
#include <config_features.h>
#include <string>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
    /// Copy&paste helper.
    bool paste(const OUString& rFilename, const uno::Reference<text::XTextRange>& xTextRange)
    {
        uno::Reference<document::XFilter> xFilter(
            m_xSFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"),
            uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(
            m_directories.getURLFromSrc("/sw/qa/extras/ooxmlexport/data/") + rFilename,
            StreamMode::READ);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
            { "InputStream", uno::makeAny(xStream) },
            { "InputMode", uno::makeAny(true) },
            { "TextInsertModeRange", uno::makeAny(xTextRange) },
        }));
        return xFilter->filter(aDescriptor);
    }
};

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedParagraphMark,
                         "testTrackChangesDeletedParagraphMark.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedParagraphMark,
                         "testTrackChangesInsertedParagraphMark.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:ins");
}

DECLARE_OOXMLEXPORT_TEST(testFDO73034, "FDO73034.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(
        getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:u", "val").match("single"));
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesParagraphProperties,
                         "testTrackChangesParagraphProperties.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange", 0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo73550, "fdo73550.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    // This was wrap="none".
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:rFonts");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73541, "fdo73541.docx")
{
    // fdo#73541: The mirrored margins were not imported and mapped correctly in Page Layout
    // Hence <w:mirrorMargins /> tag was not exported back in settings.xml
    xmlDocPtr pXmlDoc = parseExport("word/settings.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:settings/w:mirrorMargins");
}

DECLARE_OOXMLEXPORT_TEST(testFdo106029, "fdo106029.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/settings.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:doNotExpandShiftReturn");
}

DECLARE_OOXMLEXPORT_TEST(testColumnBreak_ColumnCountIsZero, "fdo74153.docx")
{
    /* fdo73545: Column Break with Column_count = 0 was not getting preserved.
    * The <w:br w:type="column" /> was missing after roundtrip
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (pXmlDoc)
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:br", "type", "column");

    //tdf76349 match Word's behavior of treating breaks in single columns as page breaks.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testIndentation, "test_indentation.docx")
{
    // fdo#74141 :There was a problem that in style.xml and document.xml in <w:ind> tag "right" & "left" margin
    // attributes gets added(w:right=0 & w:left=0) if these attributes are not set in original document.
    // This test is to verify <w:ind> does not contain w:right attribute.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:ind", "end", "");
}

DECLARE_OOXMLEXPORT_TEST(testAbi11739, "abi11739.docx")
{
    // Validation test: order of elements were wrong.
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;
    // Order was: uiPriority, link, basedOn.
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "basedOn")
                   < getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "link"));
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "link")
                   < getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "uiPriority"));
    // Order was: qFormat, unhideWhenUsed.
    CPPUNIT_ASSERT(getXPathPosition(pXmlDoc, "/w:styles/w:style[11]", "unhideWhenUsed")
                   < getXPathPosition(pXmlDoc, "/w:styles/w:style[11]", "qFormat"));
}

DECLARE_OOXMLEXPORT_TEST(testPageBreak, "fdo74566.docx")
{
    /*  Break to next page was written into wrong paragraph as <w:pageBreakBefore />.
    *  LO was not preserving Page Break as <w:br w:type="page" />.
    *  Now after fix , LO writes Page Break as the new paragraph and also
    *  preserves the xml tag <w:br>.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);
    uno::Reference<text::XTextRange> xParagraph4 = getParagraph(4);

    getRun(xParagraph2, 1, "First Page Second Line");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:br", "type", "page");
    getRun(xParagraph4, 1, "Second Page First line after Page Break");
}

DECLARE_OOXMLEXPORT_TEST(test_OpeningBrace, "2120112713_OpenBrace.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Checking for OpeningBrace tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:d[1]/m:dPr[1]/m:begChr[1]", "val",
                "");
}

DECLARE_OOXMLEXPORT_TEST(test_ClosingBrace, "2120112713.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Checking for ClosingBrace tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/m:oMath[1]/m:d[2]/m:dPr[1]/m:endChr[1]", "val",
                "");
}

DECLARE_OOXMLEXPORT_TEST(testTdf92521, "tdf92521.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        // There should be a section break that's in the middle of the document: right after the table.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTscp, "tscp.docx")
{
    uno::Reference<uno::XComponentContext> xComponentContext(
        comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, "urn:bails");
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Sequence<uno::Reference<rdf::XURI>> aGraphNames
        = xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    // This failed, no graphs had the urn:bails type.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aGraphNames.getLength());
    uno::Reference<rdf::XURI> xGraphName = aGraphNames[0];
    uno::Reference<rdf::XNamedGraph> xGraph
        = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);

    // No RDF statement on the first paragraph.
    uno::Reference<rdf::XResource> xParagraph(getParagraph(1), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(
        xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));

    // 3 RDF statements on the second paragraph.
    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    std::map<OUString, OUString> aExpectedStatements
        = { { "urn:bails:ExportControl:BusinessAuthorization:Identifier", "urn:example:tscp:1" },
            { "urn:bails:ExportControl:BusinessAuthorizationCategory:Identifier",
              "urn:example:tscp:1:confidential" },
            { "urn:bails:ExportControl:Authorization:StartValidity", "2015-11-27" } };
    std::map<OUString, OUString> aActualStatements;
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(),
                                        uno::Reference<rdf::XURI>());
    while (xStatements->hasMoreElements())
    {
        rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
        aActualStatements[aStatement.Predicate->getNamespace()
                          + aStatement.Predicate->getLocalName()]
            = aStatement.Object->getStringValue();
    }
    CPPUNIT_ASSERT(bool(aExpectedStatements == aActualStatements));

    // No RDF statement on the third paragraph.
    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(),
                                        uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));
}

DECLARE_OOXMLEXPORT_TEST(testFDO76587, "fdo76587.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "line", "240");
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "lineRule", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77890, "fdo77890.docx")
{
    /*
    Ensure that the page break is preserved i.e it should not be converted to a section break, in case
    if the different first page is set for the pages in the document.
    For additional comments pls refer https://www.libreoffice.org/bugzilla/show_bug.cgi?id=77890#c2
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:br", "type", "page");
}

DECLARE_OOXMLEXPORT_TEST(testFDO76597, "fdo76597.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "before", "96");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "after", "120");
}

DECLARE_OOXMLEXPORT_TEST(testFDO78887, "fdo78887.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:t[1]", "Lyrics: ");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:br[2]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo78882, "fdo78882.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Ensure that Section Break is getting written inside second paragraph
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:pPr[1]/w:sectPr[1]", 1);

    // Ensure that no dummy paragraph gets created inside second paragraph for Section Break
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:p[1]/w:pPr[1]/w:sectPr[1]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testfdo79668, "fdo79668.docx")
{
    // fdo#79668: Document was Crashing on DebugUtil build while Saving
    // because of repeated attribute value in same element.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // w:pPr's  w:shd attributes were getting added to w:pPrChange/w:pPr's w:shd hence checking
    // w:fill for both shd elements
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:shd", "fill", "FFFFFF");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pPrChange/w:pPr/w:shd", "fill",
                "FFFFFF");
}

DECLARE_OOXMLEXPORT_TEST(testFDO79915, "fdo79915.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[10]/w:t",
                       "How much buoyancy does the water provide?");
}

DECLARE_OOXMLEXPORT_TEST(testTextVerticalAdjustment, "tdf36117_verticalAdjustment.docx")
{
    //Preserve the page vertical alignment setting for .docx
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwPageDesc& Desc = pDoc->GetPageDesc(0);
    drawing::TextVerticalAdjust nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER, nVA);

    Desc = pDoc->GetPageDesc(1);
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, nVA);

    Desc = pDoc->GetPageDesc(2);
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, nVA);

    Desc = pTextDoc->GetDocShell()->GetDoc()->GetPageDesc(3);
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BLOCK, nVA);
}

DECLARE_OOXMLEXPORT_TEST(testTDF93675, "no-numlevel-but-indented.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//w:ind", "start", "1418");
}

DECLARE_OOXMLEXPORT_TEST(testN750935, "n750935.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    // Some page break types were ignores, resulting in less pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), xCursor->getPage());

    /*
    * The problem was that the header and footer was not shared.
    *
    * xray ThisComponent.StyleFamilies.PageStyles.Default.FooterIsShared
    */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    bool bValue = false;
    xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);
    xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testFdo74745, "fdo74745.docx")
{
    uno::Reference<text::XTextRange> paragraph = getParagraph(3);
    uno::Reference<text::XTextRange> text(paragraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("09/02/14"), text->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFdo81486, "fdo81486.docx")
{
    uno::Reference<text::XTextRange> paragraph = getParagraph(1);
    uno::Reference<text::XTextRange> text(paragraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("CustomTitle"), text->getString());
}

DECLARE_OOXMLEXPORT_TEST(testN750255, "n750255.docx")
{
    /*
    Column break without columns on the page is a page break, so check those paragraphs
    are on page 2 and page 3
    */
    CPPUNIT_ASSERT_EQUAL(OUString("one"), parseDump("/root/page[2]/body/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("two"), parseDump("/root/page[3]/body/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testN652364, "n652364.docx")
{
    /*
    Related to 750255 above, column break with columns on the page however should be a column break.
    enum = ThisComponent.Text.createEnumeration
    enum.nextElement
    para1 = enum.nextElement
    xray para1.String
    xray para1.PageStyleName
    enum.nextElement
    para2 = enum.nextElement
    xray para2.String
    xray para2.PageStyleName
    */
    // get the 2nd and 4th paragraph
    uno::Reference<uno::XInterface> paragraph1(getParagraph(2, "text1"));
    uno::Reference<uno::XInterface> paragraph2(getParagraph(4, "text2"));
    OUString pageStyle1 = getProperty<OUString>(paragraph1, "PageStyleName");
    OUString pageStyle2 = getProperty<OUString>(paragraph2, "PageStyleName");
    // "Standard" is the style for the first page (2nd is "Converted1").
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), pageStyle1);
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), pageStyle2);
}

DECLARE_OOXMLEXPORT_TEST(testN764005, "n764005.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    // The picture in the header wasn't absolutely positioned and wasn't in the background.
    text::TextContentAnchorType eValue;
    xPropertySet->getPropertyValue("AnchorType") >>= eValue;
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
    bool bValue = true;
    xPropertySet->getPropertyValue("Opaque") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testN766481, "n766481.docx")
{
    /*
    * The problem was that we had an additional paragraph before the pagebreak.
    *
    * oParas = ThisComponent.Text.createEnumeration
    * oPara = oParas.nextElement
    * oPara = oParas.nextElement
    * xray oParas.hasMoreElements ' should be false
    */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    for (int i = 0; i < 2; ++i)
        xParaEnum->nextElement();
    CPPUNIT_ASSERT_EQUAL(sal_False, xParaEnum->hasMoreElements());
}

DECLARE_OOXMLEXPORT_TEST(testN766487, "n766487.docx")
{
    /*
    * The problem was that 1) the font size of the first para was too large 2) numbering had no first-line-indent.
    *
    * oParas = ThisComponent.Text.createEnumeration
    * oPara = oParas.nextElement
    * oRuns = oPara.createEnumeration
    * oRun = oRuns.nextElement
    * xray oRun.CharHeight ' 11, was larger
    * oPara = oParas.nextElement
    * xray oPara.ParaFirstLineIndent ' -635, was 0
    */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());

    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    float fValue = 0;
    xPropertySet->getPropertyValue("CharHeight") >>= fValue;
    CPPUNIT_ASSERT_EQUAL(11.f, fValue);

    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-360)), nValue);
}

DECLARE_OOXMLEXPORT_TEST(testN693238, "n693238.docx")
{
    /*
    * The problem was that a continuous section break at the end of the doc caused the margins to be ignored.
    *
    * xray ThisComponent.StyleFamilies.PageStyles.Default.LeftMargin ' was 2000, should be 635
    */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nValue);
}

DECLARE_OOXMLEXPORT_TEST(testN775906, "n775906.docx")
{
    /*
    * The problem was that right margin (via direct formatting) erased the left/first margin (inherited from numbering style).
    *
    * oParas = ThisComponent.Text.createEnumeration
    * oPara = oParas.nextElement
    * xray oPara.ParaFirstLineIndent ' was 0
    * xray oPara.ParaLeftMargin ' was 0
    */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635),
                         getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1905),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testN777337, "n777337.docx")
{
    /*
    * The problem was that the top and bottom margin on the first page was only 0.1cm instead of 1.7cm.
    *
    * oFirst = ThisComponent.StyleFamilies.PageStyles.getByName("First Page")
    * xray oFirst.TopMargin
    * xray oFirst.BottomMargin
    */
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "BottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testN778836, "n778836.docx")
{
    /*
    * The problem was that the paragraph inherited margins from the numbering
    * and parent paragraph styles and the result was incorrect.
    */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270),
                         getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3810),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635),
                         getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testN778828, "n778828.docx")
{
    /*
    * The problem was that a page break after a continuous section break caused
    * double page break on title page.
    */
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testFdo55187, "fdo55187.docx")
{
    // 0x010d was imported as a newline.
    getParagraph(1, OUString(u"lup\u010Dka"));
}

DECLARE_OOXMLEXPORT_TEST(testN782345, "n782345.docx")
{
    /*
    * The problem was that the page break was inserted before the 3rd para, instead of before the 2nd para.
    */
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testN793998, "n793998.docx")
{
    sal_Int32 nTextPortion = parseDump("/root/page/body/txt/Text[1]", "nWidth")
                                 .toInt32(); // Width of the first (text) portion
    sal_Int32 nTabPortion = parseDump("/root/page/body/txt/Text[2]", "nWidth")
                                .toInt32(); // Width of the second (tab) portion
    sal_Int32 nParagraph = parseDump("/root/page/body/txt/infos/bounds", "width")
                               .toInt32(); // Width of the paragraph
    sal_Int32 const nRightMargin = 3000;
    // The problem was that the tab portion didn't ignore the right margin, so text + tab width wasn't larger than body (paragraph - right margin) width.
    CPPUNIT_ASSERT(nTextPortion + nTabPortion > nParagraph - nRightMargin);
}

DECLARE_OOXMLEXPORT_TEST(testFdo60922, "fdo60922.docx")
{
    // This was 0, not 100, due to wrong import of w:position w:val="0"
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(100), getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharEscapementHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testIndentation_2, "indentation.docx")
{
    uno::Reference<uno::XInterface> xParaLTRTitle(getParagraph(1, "Title aligned"));
    uno::Reference<uno::XInterface> xParaLTRNormal(getParagraph(2, ""));

    // this will test the text direction for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB,
                         getProperty<sal_Int16>(xParaLTRTitle, "WritingMode"));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB,
                         getProperty<sal_Int16>(xParaLTRNormal, "WritingMode"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65632, "fdo65632.docx")
{
    // The problem was that the footnote text had fake redline: only the body
    // text has redline in fact.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    //uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(
        OUString("Text"),
        getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), "TextPortionType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo43093, "fdo43093b.docx")
{
    // The problem was that the direction and alignment are not correct for RTL paragraphs.
    uno::Reference<uno::XInterface> xParaRtlRight(getParagraph(1, "Right and RTL in M$"));
    sal_Int32 nRtlRight = getProperty<sal_Int32>(xParaRtlRight, "ParaAdjust");
    sal_Int16 nRRDir = getProperty<sal_Int32>(xParaRtlRight, "WritingMode");

    uno::Reference<uno::XInterface> xParaRtlLeft(getParagraph(2, "Left and RTL in M$"));
    sal_Int32 nRtlLeft = getProperty<sal_Int32>(xParaRtlLeft, "ParaAdjust");
    sal_Int16 nRLDir = getProperty<sal_Int32>(xParaRtlLeft, "WritingMode");

    uno::Reference<uno::XInterface> xParaLtrRight(getParagraph(3, "Right and LTR in M$"));
    sal_Int32 nLtrRight = getProperty<sal_Int32>(xParaLtrRight, "ParaAdjust");
    sal_Int16 nLRDir = getProperty<sal_Int32>(xParaLtrRight, "WritingMode");

    uno::Reference<uno::XInterface> xParaLtrLeft(getParagraph(4, "Left and LTR in M$"));
    sal_Int32 nLtrLeft = getProperty<sal_Int32>(xParaLtrLeft, "ParaAdjust");
    sal_Int16 nLLDir = getProperty<sal_Int32>(xParaLtrLeft, "WritingMode");

    // this will test the both the text direction and alignment for each paragraph
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT), nRtlRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRRDir);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT), nRtlLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRLDir);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT), nLtrRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT), nLtrLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLLDir);
}

DECLARE_OOXMLEXPORT_TEST(testTdf92045, "tdf92045.docx")
{
    // This was true, <w:effect w:val="none"/> resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95031, "tdf95031.docx")
{
    // This was 494, in-numbering paragraph's automating spacing was handled as visible spacing, while it should not.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106690, "tdf106690.docx")
{
    // This was 0, numbering rules with automatic spacing meant 0
    // before/autospacing for all text nodes, even for ones at the start/end of
    // a numbered text node block.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106690Cell, "tdf106690-cell.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, bottom margin of the second paragraph in the A1 table cell
    // had a reduced auto-space, just because of a next paragraph in the A2
    // cell.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(494),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106970, "tdf106970.docx")
{
    // The second paragraph (first numbered one) had 0 bottom margin:
    // autospacing was even collapsed between different numbering styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(getParagraph(4), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testKern, "kern.docx")
{
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), "CharAutoKerning"));
    // This failed: kerning was also enabled for the second paragraph.
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(2), 1), "CharAutoKerning"));

    uno::Reference<beans::XPropertySet> xStyle(
        getStyles("ParagraphStyles")->getByName("Default Style"), uno::UNO_QUERY);
    //tdf107801: kerning normally isn't enabled by default for .docx
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be false", false,
                                 getProperty<bool>(xStyle, "CharAutoKerning"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf98700_keepWithNext, "tdf98700_keepWithNext.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading style keeps with next", true,
                                 getProperty<bool>(getParagraph(1), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default style doesn't keep with next", false,
                                 getProperty<bool>(getParagraph(2), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading 1 style inherits keeps with next", true,
                                 getProperty<bool>(getParagraph(3), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading 2 style disabled keep with next", false,
                                 getProperty<bool>(getParagraph(4), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text Body style toggled off keep with next", false,
                                 getProperty<bool>(getParagraph(5), "ParaKeepTogether"));

    //tdf#95114 - follow style is Text Body - ODT test
    uno::Reference<beans::XPropertySet> properties(
        getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Text body"), getProperty<OUString>(properties, "FollowStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakB, "tdf103975_notPageBreakB.docx")
{
    // turn on View Formatting Marks to see these documents.
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(3), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(4), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakC, "tdf103975_notPageBreakC.docx")
{
    // turn on View Formatting Marks to see these documents.
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(3), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(4), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakD, "tdf103975_notPageBreakD.docx")
{
    // The problem was that the column break was moving outside of the columns, making a page break.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakE, "tdf103975_notPageBreakE.docx")
{
    // The problem was that the column break was getting lost.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104150, "tdf104150.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // This was 0xff0000, i.e. red: background shape wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106001, "tdf106001.docx")
{
    // This was 0 (1 on UI), while Word treats outliers as 100 (outlier = not in [1..600])
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(100),
                         getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharScaleWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106001_2, "tdf106001-2.odt")
{
    // In test ODT CharScaleWidth = 900, this was not changed upon OOXML export to stay in [1..600], now it's clamped to 600
    // Note: we disregard what's set in pPr / rPr and only care about r / rPr
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:w", "val", "600");
}

DECLARE_OOXMLEXPORT_TEST(testTdf107033, "tdf107033.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // This was 0: footnote separator was disabled even in case the document
    // had no footnotes.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25),
                         getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108682, "tdf108682.docx")
{
    auto aLineSpacing = getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing");
    // This was style::LineSpacingMode::PROP.
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::FIX, aLineSpacing.Mode);
    // 260 twips in mm100, this was a negative value.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(459), aLineSpacing.Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf111964, "tdf111964.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Unicode spaces that are not XML whitespace must not be trimmed
    const sal_Unicode sWSReference[]{ 0x2002, 0x2002, 0x2002, 0x2002, 0x2002, 0 };
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", sWSReference);
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560, "fdo72560.docx")
{
    // The problem was libreoffice confuse when there RTL default style for paragraph
    uno::Reference<uno::XInterface> xParaLeftRTL(getParagraph(1, "RTL LEFT"));
    uno::Reference<uno::XInterface> xParaRightLTR(getParagraph(2, "LTR RIGHT"));

    // this will test the text direction and alignment for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB,
                         getProperty<sal_Int16>(xParaLeftRTL, "WritingMode"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT),
                         getProperty<sal_Int32>(xParaLeftRTL, "ParaAdjust"));

    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB,
                         getProperty<sal_Int16>(xParaRightLTR, "WritingMode"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT),
                         getProperty<sal_Int32>(xParaRightLTR, "ParaAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testRPrChangeClosed, "rprchange_closed.docx")
{
    // Redline defined by rPrChanged wasn't removed.
    // First paragraph has an rPrChange element, make sure it doesn't appear in the second paragraph.
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo79535, "fdo79535.docx")
{
    // fdo#79535 : LO was crashing while opening document
    // Checking there is a single page after loading a doc successfully in LO.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testCaption, "caption.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Caption"),
                                               uno::UNO_QUERY);
    // This was awt::FontSlant_ITALIC: Writer default was used instead of what is in the document.
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
}

DECLARE_OOXMLEXPORT_TEST(testIndents, "indents.docx")
{
    //expected left margin and first line indent values
    static const sal_Int32 indents[] = { 0, 0, -2000, 0, -2000, 1000, -1000, -1000, 2000, -1000 };
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(),
                                                                  uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    size_t paraIndex = 0;
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            uno::Reference<beans::XPropertySet> const xPropertySet(xServiceInfo,
                                                                   uno::UNO_QUERY_THROW);
            sal_Int32 nIndent = 0;
            sal_Int32 nFirstLine = 0;
            xPropertySet->getPropertyValue("ParaLeftMargin") >>= nIndent;
            xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nFirstLine;
            CPPUNIT_ASSERT_EQUAL(indents[paraIndex * 2], nIndent);
            CPPUNIT_ASSERT_EQUAL(indents[paraIndex * 2 + 1], nFirstLine);
            ++paraIndex;
        }
    } while (xParaEnum->hasMoreElements());
}

DECLARE_OOXMLEXPORT_TEST(testTdf92454, "tdf92454.docx")
{
    // The first paragraph had a large indentation / left margin as inheritance
    // in Word and Writer works differently, and no direct value was set to be
    // explicit.
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    // This was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraph->getPropertyState("ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95377, "tdf95377.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xParagraph, "ParaRightMargin"));

    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-501),
                         getProperty<sal_Int32>(xParagraph, "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraph->getPropertyState("ParaFirstLineIndent"));

    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-250),
                         getProperty<sal_Int32>(xParagraph, "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraph->getPropertyState("ParaFirstLineIndent"));

    //default style has numbering enabled.  Styles inherit numbering unless specifically disabled
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//body/txt/Special", 3); //first three paragraphs have numbering
    assertXPath(pXmlDoc, "//body/txt[1]/Special", "rText", "a.");
    assertXPath(pXmlDoc, "//body/txt[2]/Special", "rText", "b.");
    assertXPath(pXmlDoc, "//body/txt[3]/Special", "rText", "c.");
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/Special",
                0); //last paragraph style disables numbering
}

DECLARE_OOXMLEXPORT_TEST(testTdf95376, "tdf95376.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(2), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE: indentation-from-numbering
    // did not have priority over indentation-from-paragraph-style, due to a
    // filter workaround that's not correct here.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xParagraph->getPropertyState("ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92124, "tdf92124.docx")
{
    // Get the second paragraph's numbering style's 1st level's suffix.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(0) >>= aLevel; // 1st level
    OUString aSuffix
        = std::find_if(aLevel.begin(), aLevel.end(),
                       [](const beans::PropertyValue& rValue) { return rValue.Name == "Suffix"; })
              ->Value.get<OUString>();
    // Make sure it's empty as the source document contains <w:suff w:val="nothing"/>.
    CPPUNIT_ASSERT(aSuffix.isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf93919, "tdf93919.docx")
{
    // This was 0, left margin was not inherited from the list style.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf94374, "hello.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    // This failed: it wasn't possible to insert a DOCX document into an existing Writer one.
    CPPUNIT_ASSERT(paste("tdf94374.docx", xEnd));
}

DECLARE_OOXMLEXPORT_TEST(testTdf83300, "tdf83300.docx")
{
    // This was 'Contents Heading', which (in the original document) implied 'keep with next' on unexpected paragraphs.
    CPPUNIT_ASSERT_EQUAL(OUString("TOC Heading"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf97417, "section_break_numbering.docx")
{
    uno::Reference<beans::XPropertySet> xProps(getParagraph(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("1st page: first paragraph erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());
    // paragraph with numbering and section break was removed by writerfilter
    // but its numbering was copied to all following paragraphs
    CPPUNIT_ASSERT_MESSAGE(
        "2nd page: first paragraph missing numbering",
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules")
            .is());
    xProps = uno::Reference<beans::XPropertySet>(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("2nd page: second paragraph erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf95213, "tdf95213.docx")
{
    // Get the second paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aName = std::find_if(aLevel.begin(), aLevel.end(),
                                  [](const beans::PropertyValue& rValue) {
                                      return rValue.Name == "CharStyleName";
                                  })
                         ->Value.get<OUString>();

    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aName),
                                               uno::UNO_QUERY);
    // This was awt::FontWeight::BOLD.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xStyle, "CharWeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf107359, "tdf107359-char-pitch.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);

    bool bGridSnapToChars;
    xPropertySet->getPropertyValue("GridSnapToChars") >>= bGridSnapToChars;
    CPPUNIT_ASSERT_EQUAL(false, bGridSnapToChars);

    sal_Int32 nRubyHeight;
    xPropertySet->getPropertyValue("GridRubyHeight") >>= nRubyHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRubyHeight);

    sal_Int32 nBaseHeight;
    xPropertySet->getPropertyValue("GridBaseHeight") >>= nBaseHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(18 * 20)), nBaseHeight);

    sal_Int32 nBaseWidth;
    xPropertySet->getPropertyValue("GridBaseWidth") >>= nBaseWidth;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(24 * 20)), nBaseWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf92524_autoColor, "tdf92524_autoColor.doc")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO,
                         Color(getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf46938_clearTabStop, "tdf46938_clearTabStop.docx")
{
    // Number of tabstops should be zero, overriding the one in the style
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf63561_clearTabs, "tdf63561_clearTabs.docx")
{
    // MSO2013 gives 5,7, and 4 respectively
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(1),
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops").getLength());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3),
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(3), "ParaTabStops").getLength());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(4),
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(4), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf82065_Ind_start_strict, "tdf82065_Ind_start_strict.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    bool bFoundIndentAt = false;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "IndentAt")
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("IndentAt", double(6001),
                                                 rProp.Value.get<double>(), 10);
            bFoundIndentAt = true;
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("IndentAt defined", true, bFoundIndentAt);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112694, "tdf112694.docx")
{
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    // Header was on when header for file was for explicit first pages only
    // (marked via <w:titlePg>).
    CPPUNIT_ASSERT(!getProperty<bool>(aPageStyle, "HeaderIsOn"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf115861, "tdf115861.docx")
{
    // Second item in the paragraph enumeration was a table, 2nd paragraph was
    // lost.
    CPPUNIT_ASSERT_EQUAL(OUString("(k)"), getParagraph(2)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.docx")
{
    OUString s = OUString::fromUtf8(u8"«Name»");
    uno::Reference<beans::XPropertySet> xTextField
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;

    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(u8"«Name»"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(
        xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("Name") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.Name"), sValue);
}

DECLARE_OOXMLEXPORT_TEST(testParagraphSplitOnSectionBorder, "parasplit-on-section-border.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Test document has only two paragraphs. After splitting, it should contain
    // three of them.
    assertXPath(pXmlDoc, "//w:sectPr", 2);
    assertXPath(pXmlDoc, "//w:p", 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf44832_testSectionWithDifferentHeader,
                         "tdf44832_section_new_header.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:headerReference", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf113547, "tdf113547.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aProps(xLevels->getByIndex(0)); // 1st level
        // This was 0, first-line left margin of the numbering was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-635), aProps["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf49073, "tdf49073.docx")
{
    // test case for Asisan phontic guide ( ruby text.)
    sal_Unicode aRuby[3] = { 0x304D, 0x3082, 0x3093 };
    OUString sRuby = OUString(aRuby, SAL_N_ELEMENTS(aRuby));
    CPPUNIT_ASSERT_EQUAL(sRuby, getProperty<OUString>(getParagraph(1)->getStart(), "RubyText"));
    OUString sStyle = getProperty<OUString>(getParagraph(1)->getStart(), "RubyCharStyleName");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("CharacterStyles")->getByName(sStyle), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(5.f, getProperty<float>(xPropertySet, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_CENTER),
                         getProperty<sal_Int16>(getParagraph(2)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_BLOCK),
                         getProperty<sal_Int16>(getParagraph(3)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_INDENT_BLOCK),
                         getProperty<sal_Int16>(getParagraph(4)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_LEFT),
                         getProperty<sal_Int16>(getParagraph(5)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_RIGHT),
                         getProperty<sal_Int16>(getParagraph(6)->getStart(), "RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyPosition::INTER_CHARACTER),
                         getProperty<sal_Int16>(getParagraph(7)->getStart(), "RubyPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf114703, "tdf114703.docx")
{
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 0, level override "default" replaced the non-default value from
    // the abstract level.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(-1000),
        comphelper::SequenceAsHashMap(xRules->getByIndex(0))["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf107035, "tdf107035.docx")
{
    // Select the second run containing the page number field
    auto xPgNumRun = getRun(getParagraph(1), 2, "1");

    // Check that the page number field colour is set to "automatic".
    sal_Int32 nPgNumColour = getProperty<sal_Int32>(xPgNumRun, "CharColor");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), nPgNumColour);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112118_DOCX, "tdf112118.docx")
{
    // The resulting left margin width (2081) differs from its DOC counterpart from ww8export2.cxx,
    // because DOCX import does two conversions between mm/100 and twips on the route, losing one
    // twip on the road and arriving with a value that is 2 mm/100 less. I don't see an obvious way
    // to avoid that.
    struct
    {
        const char* styleName;
        struct
        {
            const char* sideName;
            sal_Int32 nMargin;
            sal_Int32 nBorderDistance;
            sal_Int32 nBorderWidth;
        } sideParams[4];
    } styleParams[] = { // Margin (MS-style), border distance, border width
                        { "Standard",
                          {
                              { "Top", 496, 847, 159 }, //  851 twip, 24 pt (from text), 4.5 pt
                              { "Left", 2081, 706, 212 }, // 1701 twip, 20 pt (from text), 6.0 pt
                              { "Bottom", 1401, 564, 35 }, // 1134 twip, 16 pt (from text), 1.0 pt
                              { "Right", 3471, 423, 106 } // 2268 twip, 12 pt (from text), 3.0 pt
                          } },
                        { "Converted1",
                          {
                              { "Top", 847, 496, 159 }, //  851 twip, 24 pt (from edge), 4.5 pt
                              { "Left", 706, 2081, 212 }, // 1701 twip, 20 pt (from edge), 6.0 pt
                              { "Bottom", 564, 1401, 35 }, // 1134 twip, 16 pt (from edge), 1.0 pt
                              { "Right", 423, 3471, 106 } // 2268 twip, 12 pt (from edge), 3.0 pt
                          } }
    };
    auto xStyles = getStyles("PageStyles");

    for (const auto& style : styleParams)
    {
        const OUString sName = OUString::createFromAscii(style.styleName);
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(sName), uno::UNO_QUERY_THROW);
        for (const auto& side : style.sideParams)
        {
            const OUString sSide = OUString::createFromAscii(side.sideName);
            const OString sStage = OString(style.styleName) + " " + side.sideName;

            sal_Int32 nMargin = getProperty<sal_Int32>(xStyle, sSide + "Margin");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " margin width").getStr(), side.nMargin,
                                         nMargin);

            sal_Int32 nBorderDistance = getProperty<sal_Int32>(xStyle, sSide + "BorderDistance");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border distance").getStr(),
                                         side.nBorderDistance, nBorderDistance);

            table::BorderLine aBorder = getProperty<table::BorderLine>(xStyle, sSide + "Border");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                OString(sStage + " border width").getStr(), side.nBorderWidth,
                sal_Int32(aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance));

            // tdf#116472: check that AUTO border color is imported as black
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border color").getStr(),
                                         sal_Int32(COL_BLACK), aBorder.Color);
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf117504_numberingIndent, "tdf117504_numberingIndent.docx")
{
    OUString sName = getProperty<OUString>(getParagraph(1), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Paragraph has numbering style", !sName.isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf82175_noStyleInheritance, "tdf82175_noStyleInheritance.docx")
{
    // The document's "Default" paragraph style is 1 inch fixed line spacing, and that is what should not be inherited.
    style::LineSpacing aSpacing
        = getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing");
    // MSWord uses 115% line spacing, but LO follows the documentation and sets single spacing.
    CPPUNIT_ASSERT_MESSAGE("Text Body style 115% line spacing", sal_Int16(120) > aSpacing.Height);
    CPPUNIT_ASSERT_MESSAGE("THANKS for *FIXING* line spacing", sal_Int16(115) != aSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aSpacing.Mode);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
