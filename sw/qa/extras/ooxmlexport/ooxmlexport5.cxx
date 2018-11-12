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
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XFootnote.hpp>
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
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>

#include <string>
#include <config_features.h>
#include <unocrsr.hxx>
#include <ndtxt.hxx>

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
};

DECLARE_OOXMLEXPORT_TEST(testFDO76248, "FDO76248.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;
    // In two cases the a:graphicData elements had no children, which is invalid.
    assertXPath(pXmlDoc, "//a:graphicData[not(*)]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTscp, "tscp.docx")
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, "urn:bails");
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent, uno::UNO_QUERY);
    uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    // This failed, no graphs had the urn:bails type.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aGraphNames.getLength());
    uno::Reference<rdf::XURI> xGraphName = aGraphNames[0];
    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);

    // No RDF statement on the first paragraph.
    uno::Reference<rdf::XResource> xParagraph(getParagraph(1), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));

    // 3 RDF statements on the second paragraph.
    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    std::map<OUString, OUString> aExpectedStatements =
    {
        {"urn:bails:ExportControl:BusinessAuthorization:Identifier", "urn:example:tscp:1"},
        {"urn:bails:ExportControl:BusinessAuthorizationCategory:Identifier", "urn:example:tscp:1:confidential"},
        {"urn:bails:ExportControl:Authorization:StartValidity", "2015-11-27"}
    };
    std::map<OUString, OUString> aActualStatements;
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    while (xStatements->hasMoreElements())
    {
        rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
        aActualStatements[aStatement.Predicate->getNamespace() + aStatement.Predicate->getLocalName()] = aStatement.Object->getStringValue();
    }
    CPPUNIT_ASSERT(bool(aExpectedStatements == aActualStatements));

    // No RDF statement on the third paragraph.
    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));
}

DECLARE_OOXMLEXPORT_TEST(testfdo76589 , "fdo76589.docx")
{
    /* Numbered list was not preserve after RT.
     * In numbering.xml, when NumberingType is "decimal" and level is zero,
     * w:val of w:lvlText was empty.
     * It should be <w:lvlText w:val="%1" />
     */
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;

    assertXPath ( pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText","val","%1" );
}

DECLARE_OOXMLEXPORT_TEST(testDecimalNumberingNoLeveltext, "decimal-numbering-no-leveltext.docx")
{
    // This was "%1", not empty: we turned a kind-of-none numbering into a decimal one.
    if (xmlDocPtr pXmlDoc = parseExport("word/numbering.xml"))
        assertXPath (pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText","val", "");
}

DECLARE_OOXMLEXPORT_TEST(testNoDuplicateAttributeExport, "duplicate-east-asia.odt")
{
    // File asserting while saving in LO.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testfdo79008, "fdo79008.docx")
{
    /* File crashing while saving in LO.
     * Check if document.xml file is created after fix
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testAuthorPropertySdt, "author-property.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "xpath", "/ns1:coreProperties[1]/ns0:creator[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "storeItemID","{6C3C8BC8-F283-45AE-878A-BAB7291924A1}");
    // FIXME: the next property doesn't match, though it's correct in theory. A bug in assertXPath?
    // assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "prefixMappings",
    //            "xmlns:ns0='http://purl.org/dc/elements/1.1/' xmlns:ns1='http://schemas.openxmlformats.org/package/2006/metadata/core-properties'");
}

DECLARE_OOXMLEXPORT_TEST(testFDO76586, "fdo76586.docx")
{
    /*
     * In the test file gridCol had only one value for entire table width
     * while there are two cells in a table row.
     * So the table was not imported with the correct cell widths
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    // there is only one table in the test file
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[1]", "w", "1601");
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[2]", "w", "7843");
}

DECLARE_OOXMLEXPORT_TEST(testFDO76587 , "fdo76587.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "line", "240");
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "lineRule", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77890 , "fdo77890.docx")
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

DECLARE_OOXMLEXPORT_TEST(testNumberedList,"NumberedList.docx")
{
    //fdo74150:In document.xml, for pStyle = "NumberedList1", iLvl and numId was not preserved
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:pStyle", "val", "NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:ilvl","val", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:numId","val", "0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:pStyle","val", "NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:ilvl","val", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:numId","val", "0");
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

DECLARE_OOXMLEXPORT_TEST(testContentTypeTIF, "fdo77476.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/ContentType:Types/ContentType:Override[@ContentType='image/tiff']", "PartName", "/word/media/image1.tif");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77117, "fdo77117.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This checks textbox textrun size of font which is in group shape.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTable, "fdo77887.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "horzAnchor", "margin");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "leftFromText", "141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "rightFromText", "141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpXSpec", "center");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpY", "2266");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "vertAnchor", "page");

    //make sure not to write empty attributes which requires enumeration
    assertXPathNoAttribute(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpYSpec");
}

DECLARE_OOXMLEXPORT_TEST(testOldComplexMergeRight, "tdf90681-1.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:vMerge", "val", "restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:vMerge", "val", "continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[2]/w:tcPr/w:vMerge", "val", "continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[2]/w:tcPr/w:vMerge", "val", "continue");
}

DECLARE_OOXMLEXPORT_TEST(testOldComplexMergeleft, "tdf90681-2.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:vMerge", "val", "restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:vMerge", "val", "continue");
}

DECLARE_OOXMLEXPORT_TEST(testOldComplexMergeTableInTable, "ooo96040-2.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;
}

DECLARE_OOXMLEXPORT_TEST(testHyperlinkContainingPlaceholderField, "hyperlink-field.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;
}

DECLARE_OOXMLEXPORT_TEST(testTablePreferredWidth, "tablePreferredWidth.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if(!pXmlDoc)
        return;

    // Problem :If the table preferred width is in percent, then after RT it changes to 0 & width type changes
    // to 'auto' instead of 'pct'.
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "w", "3000");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "type","pct");
}

DECLARE_OOXMLEXPORT_TEST(testFDO75431, "fdo75431.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "//w:tbl", 2);
    assertXPath(pXmlDoc, "//w:p/w:pPr/w:sectPr/w:type", "val", "nextPage");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77725, "fdo77725.docx")
{
    xmlDocPtr pXmlFootnotes = parseExport("word/footnotes.xml");
    if (!pXmlFootnotes)
        return;

    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[1]", 0);
    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[2]", 0);
    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[3]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testFDO77812, "fdo77812.docx")
{
    /* Additional sectPr was getting inserted and hence Column properties
     * were getting added into this additional sectPr instead of Default setPr.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Check no additional section break is inserted.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:sectPr", 0);

    // Check w:cols comes under Default sectPr
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols", "num", "2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[1]", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[2]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testContentTypeOLE, "fdo77759.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.spreadsheetml.sheet']",
                "PartName",
                "/word/embeddings/oleObject1.xlsx");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.Sheet.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo78420, "fdo78420.docx")
{
    xmlDocPtr pXmlHeader = parseExport("word/header2.xml");

    if (!pXmlHeader)
       return;

    xmlDocPtr pXmlHeaderRels = parseExport("word/_rels/header2.xml.rels");
    if(!pXmlHeaderRels)
       return;

    assertXPath(pXmlHeaderRels,"/rels:Relationships/rels:Relationship[1]","Id","rId1");
}


DECLARE_OOXMLEXPORT_TEST(testPageBreakInFirstPara,"fdo77727.docx")
{
    /* Break to next page was not exported if it is in first paragraph of the section.
     * Now after fix , LO writes Next Page Break and also preserves <w:br> tag.
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br","type","page");
}

DECLARE_OOXMLEXPORT_TEST(testFDO78284, "fdo78284.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,"/ContentType:Types/ContentType:Override[@PartName='/word/media/OOXDiagramDataRels1_0.png']",
                        "ContentType",
                        "image/png");
}

DECLARE_OOXMLEXPORT_TEST(testFDO78384,"fdo78384.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:rFonts","ascii","Wingdings");
}

DECLARE_OOXMLEXPORT_TEST(testfdo78469, "fdo78469.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
       return;
    // make sure dataBinding & text tags not present in sdtcontent
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:dataBinding[1]",0);
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:text[1]",0);
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

DECLARE_OOXMLEXPORT_TEST(testFdo78651, "fdo78651.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;
    // ensure that there are only two tables
    assertXPath(pXmlDoc, "//w:tbl", 2);
}

DECLARE_OOXMLEXPORT_TEST(testfdo78882, "fdo78882.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Ensure that Section Break is getting written inside second paragraph
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:pPr[1]/w:sectPr[1]",1);

    // Ensure that no dummy paragraph gets created inside second paragraph for Section Break
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:p[1]/w:pPr[1]/w:sectPr[1]",0);
}

DECLARE_OOXMLEXPORT_TEST(testfdo76934, "fdo76934.docx")
{
    /* Issue was, AutoSpacing property if present inside styles.xml, LO was not able to
     * preserve it.
     */

    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");

    if (!pXmlDoc)
        return;

    // Ensure that after fix LO is preserving AutoSpacing property in styles.xml
    assertXPath ( pXmlDoc, "/w:styles[1]/w:style[@w:styleId='Title']/w:pPr[1]/w:spacing[1]", "beforeAutospacing", "1" );
}

DECLARE_OOXMLEXPORT_TEST(testfdo79540, "fdo79540.docx")
{
    /* Issue was, <w:drawing> was getting written inside <w:drawing>.
     * So postpone the writing of Inner Drawing tag.
     * MS Office does not allow nesting of drawing tags.
     */

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Ensure that two separate w:drawing tags are written and they are not nested.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing", 1);
}

DECLARE_OOXMLEXPORT_TEST(testFDO79062, "fdo79062.docx")
{
    xmlDocPtr pXmlFootNotes = parseExport("word/footnotes.xml");
    if (!pXmlFootNotes)
        return;
    assertXPath(pXmlFootNotes, "/w:footnotes", "Ignorable", "w14 wp14");

    xmlDocPtr pXmlEndNotes = parseExport("word/endnotes.xml");
    if (!pXmlEndNotes)
        return;
    assertXPath(pXmlEndNotes, "/w:endnotes", "Ignorable", "w14 wp14");

    //tdf#93121 don't add fake tabs in front of extra footnote paragraphs
    uno::Reference<text::XFootnotesSupplier> xFootnoteSupp(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc(xFootnoteSupp->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote(xFootnoteIdxAcc->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess>xParaEnumAccess(xFootnoteText->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration>xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<text::XTextRange> xTextRange;
    xParaEnum->nextElement();
    xParaEnum->nextElement() >>= xTextRange;
    OUString sFootnotePara = xTextRange->getString();
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Paragraph starts with W(87), not tab(9)", u'W', sFootnotePara[0] );
}

DECLARE_OOXMLEXPORT_TEST(testfdo79668,"fdo79668.docx")
{
    // fdo#79668: Document was Crashing on DebugUtil build while Saving
    // because of repeated attribute value in same element.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // w:pPr's  w:shd attributes were getting added to w:pPrChange/w:pPr's w:shd hence checking
    // w:fill for both shd elements
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:shd", "fill", "FFFFFF" );
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pPrChange/w:pPr/w:shd", "fill", "FFFFFF" );
}

DECLARE_OOXMLEXPORT_TEST(testfdo78907,"fdo78907.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page" );

    xmlDocPtr pXmlDoc1 = parseExport("word/footer1.xml");
    if (!pXmlDoc1)
        return;
    assertXPath ( pXmlDoc1, "/w:ftr[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl", 0 );
}

DECLARE_OOXMLEXPORT_TEST(tdf118702,"tdf118702.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:type", "val", "nextPage" );
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:pgSz", "orient", "landscape" );
}

DECLARE_OOXMLEXPORT_TEST(testfdo79822, "fdo79822.docx")
{
    /* File getting crash while saving in LO.
     * The Docx contain smartart and the file was created in ms word 2007
     */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
}

DECLARE_OOXMLEXPORT_TEST(testFDO79915, "fdo79915.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[10]/w:t", "How much buoyancy does the water provide?");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79817, "fdo79817.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "storeItemID", "{9222E47B-A68B-4AEB-9855-21C912B9D3D2}");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "xpath", "/ns0:properties[1]/documentManagement[1]/ns2:Responsible_x0020_Officer_x0020_Title[1]");
    }
}


DECLARE_OOXMLEXPORT_TEST(testfdo79968_sldx, "fdo79968.docx")
{
    // This UT for DOCX embedded with powerpoint slide
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.presentationml.slide']",
                "PartName",
                "/word/embeddings/oleObject1.sldx");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldx']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "PowerPoint.Slide.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79969_xlsb, "fdo79969_xlsb.docx")
{
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-excel.sheet.binary.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.xlsb");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsb']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.SheetBinaryMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80097, "fdo80097.docx")
{
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    //Table Cell Borders
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:top[@w:val = 'single']",1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:top[@w:sz = 4]", 1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:top[@w:space = 0]", 1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:top[@w:color = '000000']", 1);

    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'single']",1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:sz = 4]", 1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:space = 0]", 1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:color = '000000']", 1);

    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:insideH",0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:insideV",0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo77129, "fdo77129.docx")
{
    // The problem was that text after TOC field was missing if footer reference  comes in field.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
       return;

    // Data was lost from this paragraph.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:t", "Abstract");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79969_xlsm, "fdo79969_xlsm.docx")
{
    // This UT for DOCX embedded with excel work sheet.
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-excel.sheet.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.xlsm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.SheetMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80522,"fdo80522.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-word.document.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.docm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.DocumentMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80523_pptm,"fdo80523_pptm.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-powerpoint.presentation.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.pptm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.pptm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "PowerPoint.ShowMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80523_sldm,"fdo80523_sldm.docx")
{
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-powerpoint.slide.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.sldm");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "PowerPoint.SlideMacroEnabled.12");
}

DECLARE_OOXMLEXPORT_TEST(testfdo80898, "fdo80898.docx")
{
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml");

    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/msword']",
                "PartName",
                "/word/embeddings/oleObject1.doc");

    // check the rels too
    xmlDocPtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.doc']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // check the content too
    xmlDocPtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.Document.8");
}

DECLARE_OOXMLEXPORT_TEST(testTableCellWithDirectFormatting, "fdo80800.docx")
{
    // Issue was Direct Formatting for non-first Table cells was not getting preserved.

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    // Ensure that for Third Table cell Direct Formatting is preserved.
    // In file, Direct Formatting used for Third Table cell is Line Spacing="1.5 lines"
    // For Line Spacing "1.5 lines" w:line equals 360
    assertXPath(pXmlDoc,"/w:document/w:body/w:tbl/w:tr/w:tc[3]/w:p/w:pPr/w:spacing","line","360");

}

DECLARE_OOXMLEXPORT_TEST(testFdo80800b_tableStyle, "fdo80800b_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell1 1.5lines"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A1 1.5 line spacing", sal_Int16(150), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);

    xCell.set(xTable->getCellByName("B1"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell2 Implicit (Single)"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 single line spacing", sal_Int16(100), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);

    xCell.set(xTable->getCellByName("C1"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell3 Implicit (Single)"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 paragraph1 single line spacing", sal_Int16(100), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 paragraph3 line spacing", sal_Int16(212), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117297_tableStyle, "tdf117297_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xPara->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Green text, default size (9), 1.5 spaced"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 green font", sal_Int32(0x70AD47), getProperty<sal_Int32>(getRun(xPara, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 1.5 line spacing", sal_Int16(150), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xText.set(xPara->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TableGrid color (blue), TableGrid size (9), double spacing"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph3 blue font", sal_Int32(0x00B0F0), getProperty<sal_Int32>(getRun(xPara, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph3 double spacing", sal_Int16(200), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf82175_noStyleInheritance, "tdf82175_noStyleInheritance.docx")
{
    // The document's "Default" paragraph style is 1 inch fixed line spacing, and that is what should not be inherited.
    style::LineSpacing aSpacing = getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing");
    // MSWord uses 115% line spacing, but LO follows the documentation and sets single spacing.
    CPPUNIT_ASSERT_MESSAGE("Text Body style 115% line spacing", sal_Int16(120) > aSpacing.Height);
    CPPUNIT_ASSERT_MESSAGE("THANKS for *FIXING* line spacing", sal_Int16(115) != aSpacing.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aSpacing.Mode);
}

DECLARE_OOXMLEXPORT_TEST(test2colHeader, "2col-header.docx")
{
    // Header was lost on export when the document had multiple columns.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));
}

DECLARE_OOXMLEXPORT_TEST(testfdo83048, "fdo83048.docx")
{
    // Issue was wrong SDT properties were getting exported for Date SDT
    xmlDocPtr pXmlDoc = parseExport("word/footer1.xml");
    if (!pXmlDoc)
       return;

    // Make sure Date is inside SDT tag.
    // This will happen only if right SDT properties are exported.
    assertXPath(pXmlDoc, "/w:ftr/w:sdt/w:sdtContent/w:p[1]/w:sdt/w:sdtContent/w:r[1]/w:t", "1/2/2013");
}

DECLARE_OOXMLEXPORT_TEST(testSdt2Run, "sdt-2-run.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // The problem was that <w:sdt> was closed after "first", not after "second", so the second assert failed.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[1]/w:t", "first");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t", "second");
    // Make sure the third portion is still outside <w:sdt>.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/w:t", "third");
}

DECLARE_OOXMLEXPORT_TEST(testFD083057, "fdo83057.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;

    // A fly frame was attached to a para which started with a hint (run) containing an SDT.
    // This SDT was handled while exporting the FLYFRAME and also the text of the run.
    // So, even though the original file had only one sdt in the header, the RT file had two;
    // one for a separate run as expected, and one incorrectly exported in the alternateContent (FLYFRAME)

    // Assert that the file has only one sdt, in a separate run
    assertXPath(pXmlDoc, "//w:sdt", 1);
    assertXPath(pXmlDoc, "//mc:AlternateContent//w:sdt", 0);
}

DECLARE_OOXMLEXPORT_TEST(testHeaderBorder, "header-border.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This was 0, as header margin was lost during import.
        assertXPath(pXmlDoc, "//w:pgMar", "header", "720");
        // This was 33: 33 points -> 660 twips. We counted 900 - 240 (distance
        // of page and body frame) instead of 720 - 240 (distance of page and
        // header frame).
        assertXPath(pXmlDoc, "//w:pgBorders/w:top", "space", "24");
    }
}

DECLARE_OOXMLEXPORT_TEST(testTablepprShape, "tblppr-shape.docx")
{
    // Unhandled exception due to unexpected w:tblpPr resulted in not importing page size, either.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 2000, page width wasn't large enough.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2540), getProperty<sal_Int32>(xPageStyle, "LeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testImageNoborder, "image-noborder.docx")
{
    // This was 26; we exported border for an image that had no border.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf89774, "tdf89774.fodt")
{
    if (xmlDocPtr pXmlDoc = parseExport("docProps/app.xml"))
        // This was 65, as unit was seconds instead of minutes.
        assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:TotalTime", "1");
}

DECLARE_OOXMLEXPORT_TEST(testSectionProtection, "sectionprot.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr/w:formProt", "val", "true");
        assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:formProt", "val", "false");
    }

    if (xmlDocPtr pXmlSettings = parseExport("word/settings.xml"))
    {
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", "true");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", "forms");
    }

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextSection is protected", true, getProperty<bool>(xSect, "IsProtected"));
    xSect.set(xSections->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", false, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_OOXMLEXPORT_TEST(tdf66398_permissions, "tdf66398_permissions.docx")
{
    // check document permission settings for the whole document
    if (xmlDocPtr pXmlSettings = parseExport("word/settings.xml"))
    {
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit",               "readOnly");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement",        "1");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptProviderType",  "rsaAES");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmClass","hash");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmType", "typeAny");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptAlgorithmSid",  "14");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "cryptSpinCount",     "100000");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "hash",               "A0/Xy6KcXljJlZjP0TwJMPJuW2rc46UwXqn2ctxckc2nCECE5i89M85z2Noh3ZEA5NBQ9RJ5ycxiUH6nzmJaKw==");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "salt",               "B8k6wb1pkjUs4Nv/8QBk/w==");
    }

    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);

    // check: we have 2 bookmarks
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("_GoBack"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("permission-for-group:267014232:everyone"));
}

DECLARE_OOXMLEXPORT_TEST(tdf122201_editUnprotectedText, "tdf122201_editUnprotectedText.odt")
{
    // get the document
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    // get two different nodes
    SwNodeIndex aDocEnd(pDoc->GetNodes().GetEndOfContent());
    SwNodeIndex aDocStart(*aDocEnd.GetNode().StartOfSectionNode(), 3);

    // check protected area
    SwPaM aPaMPortected(aDocStart);
    CPPUNIT_ASSERT(aPaMPortected.HasReadonlySel(false));

    // check unprotected area
    SwPaM aPaMUnprotected(aDocEnd);
    CPPUNIT_ASSERT(!aPaMUnprotected.HasReadonlySel(false));
}

DECLARE_OOXMLEXPORT_TEST(testSectionHeader, "sectionprot.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // this test must not be zero
        assertXPath(pXmlDoc, "//w:headerReference", 2);
    }
}

DECLARE_OOXMLEXPORT_TEST(testOO47778_1, "ooo47778-3.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[3]", "c");
}

DECLARE_OOXMLEXPORT_TEST(testOO47778_2, "ooo47778-4.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[4]", "c");

    // tdf116436: The problem was that the table background was undefined, not white.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testOO67471, "ooo67471-2.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[2]", "B");
}

DECLARE_OOXMLEXPORT_TEST(testKDE302504, "kde302504-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//v:shape", "ID", "KoPathShape");
}

DECLARE_OOXMLEXPORT_TEST(testKDE216114, "kde216114-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:pict", 1);
}

DECLARE_OOXMLEXPORT_TEST(testOO72950, "ooo72950-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 1);
}

//There are two tables to export in this doc the second of which is inside a
//frame anchored to first cell of the first table. They must not be
//considered the same table
DECLARE_OOXMLEXPORT_TEST(fdo60957, "fdo60957-2.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 2);
}

//This has more cells than msword supports, we must balance the
//number of cell start and ends
DECLARE_OOXMLEXPORT_TEST(testOO106020, "ooo106020-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 1);
}

DECLARE_OOXMLEXPORT_TEST(testNonBMPChar, "nonbmpchar.docx")
{
    sal_uInt32  nCh = 0x24b62;
    OUString aExpected( &nCh, 1);
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange1->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
