/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>

#include <o3tl/cppunittraitshelper.hxx>
#include <comphelper/processfactory.hxx>

#include <ndindex.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        const char* aDenylist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
        };
        std::vector<const char*> vDenylist(aDenylist, aDenylist + SAL_N_ELEMENTS(aDenylist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vDenylist.begin(), vDenylist.end(), filename) == vDenylist.end());
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFDO76248)
{
    loadAndSave("FDO76248.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
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

CPPUNIT_TEST_FIXTURE(Test, testfdo76589 )
{
    loadAndSave("fdo76589.docx");
    /* Numbered list was not preserve after RT.
     * In numbering.xml, when NumberingType is "decimal" and level is zero,
     * w:val of w:lvlText was empty.
     * It should be <w:lvlText w:val="%1" />
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/numbering.xml");

    assertXPath ( pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText","val","%1" );
}

CPPUNIT_TEST_FIXTURE(Test, testDecimalNumberingNoLeveltext)
{
    loadAndSave("decimal-numbering-no-leveltext.docx");
    // This was "%1", not empty: we turned a kind-of-none numbering into a decimal one.
    xmlDocUniquePtr pXmlDoc = parseExport("word/numbering.xml");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText","val","");
}

CPPUNIT_TEST_FIXTURE(Test, testNoDuplicateAttributeExport)
{
    loadAndSave("duplicate-east-asia.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // File asserting while saving in LO.
    parseExport("word/document.xml");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79008)
{
    loadAndSave("fdo79008.docx");
    /* File crashing while saving in LO.
     * Check if document.xml file is created after fix
     */
    parseExport("word/document.xml");

    // tdf#134951: there is only one comment
    xmlDocUniquePtr pXmlSettings = parseExport("word/comments.xml");
    assertXPath(pXmlSettings, "/w:comments/w:comment", 1);

    // Read-only is set, but it is not enforced, so it should be off...
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT(!pTextDoc->GetDocShell()->IsSecurityOptOpenReadOnly());
}

DECLARE_OOXMLEXPORT_TEST(testTdf120852_readOnlyProtection, "tdf120852_readOnlyProtection.docx")
{
    if (xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml"))
    {
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", "1");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", "readOnly");
    }

    // Read-only is set, so Enforcement must enable it.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT(pTextDoc->GetDocShell()->IsSecurityOptOpenReadOnly());
}

DECLARE_OOXMLEXPORT_TEST(testTdf120852_readOnlyUnProtected, "tdf120852_readOnlyUnProtected.docx")
{
    // Readonly is not enforced, just a suggestion,
    // so when a section is protected, the document should enable forms protection.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT(!pTextDoc->GetDocShell()->IsSecurityOptOpenReadOnly());

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY_THROW);
    const sal_Int32 nLastSection = xSections->getCount() - 1;
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(nLastSection), uno::UNO_QUERY_THROW);
    if ( !mbExported )
    {
        CPPUNIT_ASSERT_MESSAGE("Section is not protected", !getProperty<bool>(xSect, "IsProtected"));
        // Enable section protection. The round-trip should have forms protection enabled.
        xSect->setPropertyValue("IsProtected", uno::makeAny(true));
    }
    else
    {
        CPPUNIT_ASSERT_MESSAGE("Section is protected", getProperty<bool>(xSect, "IsProtected"));
        xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", "forms");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", "true");
    }
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorPropertySdt)
{
    loadAndSave("author-property.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "xpath", "/ns1:coreProperties[1]/ns0:creator[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "storeItemID","{6C3C8BC8-F283-45AE-878A-BAB7291924A1}");
    // FIXME: the next property doesn't match, though it's correct in theory. A bug in assertXPath?
    // assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "prefixMappings",
    //            "xmlns:ns0='http://purl.org/dc/elements/1.1/' xmlns:ns1='http://schemas.openxmlformats.org/package/2006/metadata/core-properties'");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76586)
{
    loadAndSave("fdo76586.docx");
    /*
     * In the test file gridCol had only one value for entire table width
     * while there are two cells in a table row.
     * So the table was not imported with the correct cell widths
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // there is only one table in the test file
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[1]", "w", "1601");
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[2]", "w", "7843");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76587 )
{
    loadAndSave("fdo76587.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "line", "240");
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "lineRule", "auto");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77890 )
{
    loadAndSave("fdo77890.docx");
    /*
    Ensure that the page break is preserved i.e it should not be converted to a section break, in case
    if the different first page is set for the pages in the document.
    For additional comments please refer to https://www.libreoffice.org/bugzilla/show_bug.cgi?id=77890#c2
    */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:br", "type", "page");
}

CPPUNIT_TEST_FIXTURE(Test, testNumberedList)
{
    loadAndReload("NumberedList.docx");
    //fdo74150:In document.xml, for pStyle = "NumberedList1", iLvl and numId was not preserved
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:pStyle", "val", "NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:ilvl","val", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:numId","val", "0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:pStyle","val", "NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:ilvl","val", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:numId","val", "0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131819)
{
    loadAndSave("NumberedList.docx");
    // keep width of fixed size cells in the nested table
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // These were 4030 and 4249.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblGrid/w:gridCol[1]", "w", "3841");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblGrid/w:gridCol[2]", "w", "4049");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131959)
{
    loadAndSave("NumberedList.docx");
    // import tblInd from table style
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // These were 0.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblInd", "w", "360");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblInd", "w", "360");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76597)
{
    loadAndSave("fdo76597.docx");
    // check XML
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "before", "96");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "after", "120");
}

CPPUNIT_TEST_FIXTURE(Test, testContentTypeTIF)
{
    loadAndSave("fdo77476.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc, "/ContentType:Types/ContentType:Override[@ContentType='image/tiff']", "PartName", "/word/media/image1.tif");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77117, "fdo77117.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This checks textbox textrun size of font which is in group shape.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTable)
{
    loadAndSave("fdo77887.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "horzAnchor", "margin");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "leftFromText", "141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "rightFromText", "141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpXSpec", "center");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpY", "2266");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "vertAnchor", "page");

    //make sure not to write empty attributes which requires enumeration
    assertXPathNoAttribute(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpYSpec");
}

CPPUNIT_TEST_FIXTURE(Test, testOldComplexMergeRight)
{
    loadAndSave("tdf90681-1.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:vMerge", "val", "restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:vMerge", "val", "continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[2]/w:tcPr/w:vMerge", "val", "continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[2]/w:tcPr/w:vMerge", "val", "continue");
}

CPPUNIT_TEST_FIXTURE(Test, testOldComplexMergeleft)
{
    loadAndSave("tdf90681-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:vMerge", "val", "restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:vMerge", "val", "continue");
}

CPPUNIT_TEST_FIXTURE(Test, testOldComplexMergeTableInTable)
{
    loadAndSave("ooo96040-2.odt");
    parseExport("word/document.xml");
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkContainingPlaceholderField)
{
    loadAndSave("hyperlink-field.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    parseExport("word/document.xml");
}

CPPUNIT_TEST_FIXTURE(Test, testTablePreferredWidth)
{
    loadAndSave("tablePreferredWidth.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Problem :If the table preferred width is in percent, then after RT it changes to 0 & width type changes
    // to 'auto' instead of 'pct'.
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "w", "3000");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "type","pct");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO75431)
{
    loadAndSave("fdo75431.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "//w:tbl", 2);
    assertXPath(pXmlDoc, "//w:p/w:pPr/w:sectPr/w:type", "val", "nextPage");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77725)
{
    loadAndSave("fdo77725.docx");
    xmlDocUniquePtr pXmlFootnotes = parseExport("word/footnotes.xml");

    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[1]", 0);
    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[2]", 0);
    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[3]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testFieldRotation, "field-rotated.fodt")
{
    uno::Reference<text::XTextRange> const xRun(getRun(uno::Reference<text::XTextRange>(getParagraphOrTable(1), uno::UNO_QUERY), 1));
    uno::Reference<text::XTextField> const xField(getProperty<uno::Reference<text::XTextField>>(xRun, "TextField"));
    CPPUNIT_ASSERT(xField.is());
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title"), xField->getPresentation(true));
    CPPUNIT_ASSERT_EQUAL(OUString("Rotationeering"), xField->getPresentation(false));
    // problem was that the rotation wasn't applied to all runs of the field
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), getProperty<sal_Int16>(xRun, "CharRotation"));
}

CPPUNIT_TEST_FIXTURE(Test, testFootnoteSeparator)
{
    loadAndSave("footnotesep.fodt");
    // footnote separator definitions - taken from default page style
    xmlDocUniquePtr pXmlFootnotes = parseExport("word/footnotes.xml");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]", "id", "0");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]", "type", "separator");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]/w:p[1]/w:r[1]/w:separator", 0);
    // use paragraph font size to simulate height
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]/w:p[1]/w:pPr[1]/w:rPr[1]/w:sz", "val", "12");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]", "id", "1");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]", "type", "continuationSeparator");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]/w:p[1]/w:r[1]/w:continuationSeparator", 0);
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]/w:p[1]/w:pPr[1]/w:rPr[1]/w:sz", "val", "12");

    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings[1]/w:footnotePr[1]/w:footnote[1]", "id", "0");
    assertXPath(pXmlSettings, "/w:settings[1]/w:footnotePr[1]/w:footnote[2]", "id", "1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121441)
{
    loadAndSave("tdf121441.docx");
    xmlDocUniquePtr pXmlFootnotes = parseExport("word/footnotes.xml");
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r[1]/w:rPr/w:rStyle", 1);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r[2]/w:rPr/w:rStyle", 0);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r[2]/w:rPr/w:rFonts", 0);

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "eastAsia", "Symbol");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "cs", "Symbol");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "ascii", "Symbol");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "hAnsi", "Symbol");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77812)
{
    loadAndSave("fdo77812.docx");
    /* Additional sectPr was getting inserted and hence Column properties
     * were getting added into this additional sectPr instead of Default setPr.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Check no additional section break is inserted.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:sectPr", 0);

    // Check w:cols comes under Default sectPr
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols", "num", "2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[1]", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[2]", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testContentTypeOLE)
{
    loadAndSave("fdo77759.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.spreadsheetml.sheet']",
                "PartName",
                "/word/embeddings/oleObject1.xlsx");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.Sheet.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78420)
{
    loadAndSave("fdo78420.docx");
    xmlDocUniquePtr pXmlHeader = parseExport("word/header2.xml");

    xmlDocUniquePtr pXmlHeaderRels = parseExport("word/_rels/header2.xml.rels");

    assertXPath(pXmlHeaderRels,"/rels:Relationships/rels:Relationship[1]","Id","rId1");
}


CPPUNIT_TEST_FIXTURE(Test, testPageBreakInFirstPara)
{
    loadAndReload("fdo77727.docx");
    /* Break to next page was not exported if it is in first paragraph of the section.
     * Now after fix , LO writes Next Page Break and also preserves <w:br> tag.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br","type","page");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78284)
{
    loadAndSave("fdo78284.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");
    assertXPath(pXmlDoc,"/ContentType:Types/ContentType:Override[@PartName='/word/media/OOXDiagramDataRels1_0.png']",
                        "ContentType",
                        "image/png");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78384)
{
    loadAndReload("fdo78384.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:rFonts","ascii","Wingdings");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78469)
{
    loadAndSave("fdo78469.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/header1.xml");
    // make sure dataBinding & text tags not present in sdtcontent
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:dataBinding[1]",0);
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:text[1]",0);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78887)
{
    loadAndSave("fdo78887.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:t[1]", "Lyrics: ");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:br[2]", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78887b)
{
    loadAndSave("missing_newline.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[2]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:t[1]", "Tab and line break");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[5]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[6]/w:t[1]", "New line");
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78651)
{
    loadAndSave("fdo78651.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // ensure that there are only two tables
    assertXPath(pXmlDoc, "//w:tbl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78882)
{
    loadAndSave("fdo78882.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Ensure that Section Break is getting written inside second paragraph
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:pPr[1]/w:sectPr[1]",1);

    // Ensure that no dummy paragraph gets created inside second paragraph for Section Break
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[2]/w:p[1]/w:pPr[1]/w:sectPr[1]",0);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo76934)
{
    loadAndSave("fdo76934.docx");
    /* Issue was, AutoSpacing property if present inside styles.xml, LO was not able to
     * preserve it.
     */

    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");

    // Ensure that after fix LO is preserving AutoSpacing property in styles.xml
    assertXPath ( pXmlDoc, "/w:styles[1]/w:style[@w:styleId='Title']/w:pPr[1]/w:spacing[1]", "beforeAutospacing", "1" );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79540)
{
    loadAndSave("fdo79540.docx");
    /* Issue was, <w:drawing> was getting written inside <w:drawing>.
     * So postpone the writing of Inner Drawing tag.
     * MS Office does not allow nesting of drawing tags.
     */

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Ensure that two separate w:drawing tags are written and they are not nested.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO79062)
{
    loadAndSave("fdo79062.docx");
    xmlDocUniquePtr pXmlFootNotes = parseExport("word/footnotes.xml");
    assertXPath(pXmlFootNotes, "/w:footnotes", "Ignorable", "w14 wp14 w15");

    xmlDocUniquePtr pXmlEndNotes = parseExport("word/endnotes.xml");
    assertXPath(pXmlEndNotes, "/w:endnotes", "Ignorable", "w14 wp14 w15");

    //tdf#93121 don't add fake tabs in front of extra footnote paragraphs
    uno::Reference<text::XFootnotesSupplier> xFootnoteSupp(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc = xFootnoteSupp->getFootnotes();
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

DECLARE_OOXMLEXPORT_TEST(testTdf123262_textFootnoteSeparators, "tdf123262_textFootnoteSeparators.docx")
{
    //Everything easily fits on one page
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 1, getPages() );

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XText> xFootnoteText(xFootnotes->getByIndex(0), uno::UNO_QUERY);

    // The text in the separator footnote should not be added to the footnotes
    CPPUNIT_ASSERT_EQUAL(OUString(" Microsoft Office."), xFootnoteText->getString());

    // Ensure that paragraph markers are not lost.
    xFootnoteText.set(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in second footnote", 2, getParagraphs(xFootnoteText) );


    // tdf#136706: Two useless page styles were created for each of the four footnotes.
    CPPUNIT_ASSERT( !getStyles("PageStyles")->hasByName("Converted8") );
    CPPUNIT_ASSERT( !getStyles("PageStyles")->hasByName("Converted1") );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79668)
{
    loadAndReload("fdo79668.docx");
    // fdo#79668: Document was Crashing on DebugUtil build while Saving
    // because of repeated attribute value in same element.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // w:pPr's  w:shd attributes were getting added to w:pPrChange/w:pPr's w:shd hence checking
    // w:fill for both shd elements
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:shd", "fill", "FFFFFF" );
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pPrChange/w:pPr/w:shd", "fill", "FFFFFF" );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78907)
{
    loadAndReload("fdo78907.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page" );

    xmlDocUniquePtr pXmlDoc1 = parseExport("word/footer1.xml");
    assertXPath ( pXmlDoc1, "/w:ftr[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl", 0 );
}

CPPUNIT_TEST_FIXTURE(Test, tdf118702)
{
    loadAndReload("tdf118702.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:type", "val", "nextPage" );
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:pgSz", "orient", "landscape" );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79822)
{
    loadAndSave("fdo79822.docx");
    /* File getting crash while saving in LO.
     * The Docx contain smartart and the file was created in ms word 2007
     */
    parseExport("word/document.xml");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO79915)
{
    loadAndSave("fdo79915.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[9]/w:t", "How much buoyancy does the water provide?");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79817)
{
    loadAndSave("fdo79817.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "storeItemID", "{9222E47B-A68B-4AEB-9855-21C912B9D3D2}");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "xpath", "/ns0:properties[1]/documentManagement[1]/ns2:Responsible_x0020_Officer_x0020_Title[1]");
}


CPPUNIT_TEST_FIXTURE(Test, testfdo79968_sldx)
{
    loadAndSave("fdo79968.docx");
    // This UT for DOCX embedded with powerpoint slide
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.presentationml.slide']",
                "PartName",
                "/word/embeddings/oleObject1.sldx");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldx']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "PowerPoint.Slide.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79969_xlsb)
{
    loadAndSave("fdo79969_xlsb.docx");
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-excel.sheet.binary.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.xlsb");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsb']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.SheetBinaryMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80097)
{
    loadAndSave("fdo80097.docx");
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

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

CPPUNIT_TEST_FIXTURE(Test, testTdf95033)
{
    loadAndSave("tdf95033.docx");
    //tdf#95033 : Table borders defined by row-level tblPrEx are not getting preserved.

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    //Not disabled table cell borders
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[2]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[9]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[9]/w:tc[2]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[9]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[9]/w:tc[2]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'nil']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133455)
{
    loadAndSave("tdf133455.docx");
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    //Not disabled table cell borders
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:top[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[11]/w:tc[1]/w:tcPr/w:tcBorders/w:start[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[11]/w:tc[1]/w:tcPr/w:tcBorders/w:end[@w:val = 'nil']", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[11]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom[@w:val = 'nil']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138612)
{
    loadAndSave("tdf138612.docx");
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    // Row 5 Col 1 - vertically merged cell
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[1]/w:tcPr/w:vMerge", "val", "restart");
    // Row 5 Col 2 - split cell
    // This was w:vMerge="restart"
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[5]/w:tc[2]/w:tcPr/w:vMerge", 0);

    // Row 6 Col 1 - merged with cell in Row 5 Col 1
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[1]/w:tcPr/w:vMerge", "val", "continue");
    // Row 6 Col 2 - split cell
    // This was w:vMerge="continue" (merged with cell in Row 5 Col 2)
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[6]/w:tc[2]/w:tcPr/w:vMerge", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf140597)
{
    loadAndSave("tdf140597.docx");
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    // There were missing tblPrEx table exception borders
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:top");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:tcBorders/w:start");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:tcPr/w:tcBorders/w:top");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:tcPr/w:tcBorders/w:start");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:tcPr/w:tcBorders/w:top");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:tcPr/w:tcBorders/w:start");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128646)
{
    loadAndSave("tdf128646.docx");
    // The problem was that not hidden shapes anchored to empty hidden paragraphs were imported as hidden.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,"/w:document/w:body/w:tbl/w:tr/w:tc/w:p[7]/w:pPr/w:rPr/w:vanish", 1);
    if (!mbExported)
        // originally no <w:vanish> (the same as <w:vanish val="false">)
        assertXPath(pXmlDoc,"/w:document/w:body/w:tbl/w:tr/w:tc/w:p[7]/w:r/w:rPr/w:vanish", 0);
    else
        // This was hidden (<w:vanish/>)
        assertXPath(pXmlDoc,"/w:document/w:body/w:tbl/w:tr/w:tc/w:p[7]/w:r/w:rPr/w:vanish", "val", "false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119800)
{
    loadAndSave("tdf119800.docx");
    // The problem was that not hidden shapes anchored to empty hidden paragraphs were imported as hidden.
    // (tdf#128646 solved the same only for table paragraphs)
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:vanish", 1);
    if (!mbExported)
        // originally no <w:vanish> (the same as <w:vanish val="false">)
        assertXPath(pXmlDoc,"/w:document/w:body/w:p[2]/w:r/w:rPr/w:vanish", 0);
    else
        // This was hidden (<w:vanish/>)
        assertXPath(pXmlDoc,"/w:document/w:body/w:p[2]/w:r/w:rPr/w:vanish", "val", "false");
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77129)
{
    loadAndSave("fdo77129.docx");
    // The problem was that text after TOC field was missing if footer reference  comes in field.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Data was lost from this paragraph.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:t", "Abstract");
}

// Test the same testdoc used for testFdo77129.
DECLARE_OOXMLEXPORT_TEST(testTdf129402, "fdo77129.docx")
{
    // tdf#129402: ToC title must be "Contents", not "Content"; the index field must include
    // pre-rendered element.

    // Currently export drops empty paragraph after ToC, so skip getParagraphs test for now
//    CPPUNIT_ASSERT_EQUAL(5, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("owners."), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Contents"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("How\t2"), getParagraph(3)->getString());
//    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(4)->getString());

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aTocString(xTextCursor->getString());

    // Check that the pre-rendered entry is inside the index
    CPPUNIT_ASSERT_EQUAL(OUString("How\t2"), aTocString);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79969_xlsm)
{
    loadAndSave("fdo79969_xlsm.docx");
    // This UT for DOCX embedded with excel work sheet.
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-excel.sheet.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.xlsm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Excel.SheetMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80522)
{
    loadAndReload("fdo80522.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-word.document.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.docm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.docm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.DocumentMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80523_pptm)
{
    loadAndReload("fdo80523_pptm.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-powerpoint.presentation.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.pptm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.pptm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "PowerPoint.ShowMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80523_sldm)
{
    loadAndReload("fdo80523_sldm.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.ms-powerpoint.slide.macroEnabled.12']",
                "PartName",
                "/word/embeddings/oleObject1.sldm");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldm']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "PowerPoint.SlideMacroEnabled.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80898)
{
    loadAndSave("fdo80898.docx");
    // This UT for DOCX embedded with binary excel work sheet.
    xmlDocUniquePtr pXmlDoc = parseExport("[Content_Types].xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/msword']",
                "PartName",
                "/word/embeddings/oleObject1.doc");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.doc']",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport("word/document.xml");
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        "Word.Document.8");
}

CPPUNIT_TEST_FIXTURE(Test, testOleIconDrawAspect)
{
    loadAndSave("tdf131537.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object/o:OLEObject",
        "DrawAspect", "Icon");
}

CPPUNIT_TEST_FIXTURE(Test, testTableCellWithDirectFormatting)
{
    loadAndSave("fdo80800.docx");
    // Issue was Direct Formatting for non-first Table cells was not getting preserved.

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

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
    uno::Reference<text::XText> xText = xPara->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("Green text, default size (9), 1.5 spaced"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 green font", Color(0x70AD47), Color(ColorTransparency, getProperty<sal_Int32>(getRun(xPara, 1), "CharColor")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 1.5 line spacing", sal_Int16(150), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xText = xPara->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("TableGrid color (blue), TableGrid size (9), double spacing"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph3 blue font", Color(0x00B0F0), Color(ColorTransparency, getProperty<sal_Int32>(getRun(xPara, 1), "CharColor")));
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

CPPUNIT_TEST_FIXTURE(Test, testfdo83048)
{
    loadAndSave("fdo83048.docx");
    // Issue was wrong SDT properties were getting exported for Date SDT
    xmlDocUniquePtr pXmlDoc = parseExport("word/footer2.xml");

    // Make sure Date is inside SDT tag.
    // This will happen only if right SDT properties are exported.
    assertXPathContent(pXmlDoc, "/w:ftr/w:sdt/w:sdtContent/w:p[1]/w:sdt/w:sdtContent/w:r[2]/w:t",
                       "1/2/2013");
}

CPPUNIT_TEST_FIXTURE(Test, testSdt2Run)
{
    loadAndSave("sdt-2-run.docx");
    xmlDocUniquePtr pXmlDoc = parseExport();

    // The problem was that <w:sdt> was closed after "first", not after "second", so the second assert failed.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[1]/w:t", "first");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t", "second");
    // Make sure the third portion is still outside <w:sdt>.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/w:t", "third");
}

CPPUNIT_TEST_FIXTURE(Test, testFD083057)
{
    loadAndSave("fdo83057.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/header2.xml");

    // A fly frame was attached to a para which started with a hint (run) containing an SDT.
    // This SDT was handled while exporting the FLYFRAME and also the text of the run.
    // So, even though the original file had only one sdt in the header, the RT file had two;
    // one for a separate run as expected, and one incorrectly exported in the alternateContent (FLYFRAME)

    // Assert that the file has only one sdt, in a separate run
    assertXPath(pXmlDoc, "//w:sdt", 1);
    assertXPath(pXmlDoc, "//mc:AlternateContent//w:sdt", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testHeaderBorder)
{
    loadAndSave("header-border.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // This was 0, as header margin was lost during import.
    assertXPath(pXmlDoc, "//w:pgMar", "header", "720");
    // This was 33: 33 points -> 660 twips. We counted 900 - 240 (distance
    // of page and body frame) instead of 720 - 240 (distance of page and
    // header frame).
    assertXPath(pXmlDoc, "//w:pgBorders/w:top", "space", "24");
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

CPPUNIT_TEST_FIXTURE(Test, testTdf89774)
{
    loadAndSave("tdf89774.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport("docProps/app.xml");
    // This was 65, as unit was seconds instead of minutes.
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:TotalTime", "1");
}

DECLARE_OOXMLEXPORT_TEST(testSectionProtection, "sectionprot.odt")
{
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr/w:formProt", "val", "true");
        assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:formProt", "val", "false");
    }

    if (xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml"))
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

CPPUNIT_TEST_FIXTURE(Test, testSectionProtection2)
{
    loadAndSave("sectionprot2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml"))
    {
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", "true");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", "forms");
    }

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextSection is protected", true, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_OOXMLEXPORT_TEST(tdf66398_permissions, "tdf66398_permissions.docx")
{
    // check document permission settings for the whole document
    if (xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml"))
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
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // check: we have 2 bookmarks
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("_GoBack"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("permission-for-group:267014232:everyone"));
}

CPPUNIT_TEST_FIXTURE(Test, tdf106843)
{
    loadAndSave("tdf106843.fodt");
    // check Track Changes permission set in Writer/OpenDocument (password: "test", encoded by default encoding of Writer)
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit",               "trackedChanges");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement",        "1");

    // LO intends to export a .docx format that is natively compatible with 2013
    // but this document has an implicitly added setting AddExternalLeading = false
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "name", "compatibilityMode");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "val", "14"); // compatible with 2010
}

CPPUNIT_TEST_FIXTURE(Test, tdf89991_revisionView)
{
    loadAndSave("tdf89991.docx");
    // check revisionView (Show Changes) import and export
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:revisionView", "insDel",     "0");
    assertXPath(pXmlSettings, "/w:settings/w:revisionView", "formatting", "0");

    // There was no compatibilityMode defined.
    // 12: Use word processing features specified in ECMA-376. This is the default.
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "name", "compatibilityMode");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:compatSetting[1]", "val", "12");
}

DECLARE_OOXMLEXPORT_TEST(tdf122201_editUnprotectedText, "tdf122201_editUnprotectedText.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // get the document
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    // get two different nodes
    SwNodeIndex aDocEnd(pDoc->GetNodes().GetEndOfContent());
    SwNodeIndex aDocStart(*aDocEnd.GetNode().StartOfSectionNode(), 3);

    // check protected area
    SwPaM aPaMProtected(aDocStart);
    CPPUNIT_ASSERT(aPaMProtected.HasReadonlySel(false));

    // check unprotected area
    SwPaM aPaMUnprotected(aDocEnd);
    CPPUNIT_ASSERT(!aPaMUnprotected.HasReadonlySel(false));
}

DECLARE_OOXMLEXPORT_TEST(testSectionHeader, "sectionprot.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
    {
        // this test must not be zero
        assertXPath(pXmlDoc, "//w:headerReference", 2);
    }
}

DECLARE_OOXMLEXPORT_TEST(testOO47778_1, "ooo47778-3.odt")
{
    CPPUNIT_ASSERT_EQUAL(5, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[3]", "c");
}

DECLARE_OOXMLEXPORT_TEST(testOO47778_2, "ooo47778-4.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[4]", "c");

    // tdf116436: The problem was that the table background was undefined, not white.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(Color(0xffffff), Color(ColorTransparency, getProperty<sal_Int32>(xCell, "BackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testOO67471, "ooo67471-2.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[2]", "B");
}

DECLARE_OOXMLEXPORT_TEST(testKDE302504, "kde302504-1.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//v:shape", "ID", "KoPathShape");
}

DECLARE_OOXMLEXPORT_TEST(testKDE216114, "kde216114-1.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:pict", 1);
}

DECLARE_OOXMLEXPORT_TEST(testOO72950, "ooo72950-1.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 1);
}

//There are two tables to export in this doc the second of which is inside a
//frame anchored to first cell of the first table. They must not be
//considered the same table
CPPUNIT_TEST_FIXTURE(Test, fdo60957)
{
    loadAndSave("fdo60957-2.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:tbl", 2);
}

//This has more cells than msword supports, we must balance the
//number of cell start and ends
CPPUNIT_TEST_FIXTURE(Test, testOO106020)
{
    loadAndSave("ooo106020-1.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:tbl", 1);
}

DECLARE_OOXMLEXPORT_TEST(testNonBMPChar, "nonbmpchar.docx")
{
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString(u"\U00024b62"), xTextRange1->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testSpacingGroupShapeText)
{
    loadAndSave("tdf131775_SpacingGroupShapeText.docx");
    // tdf#131775: Check if correct spacing.

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:spacing", "val", "40");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100581)
{
    loadAndSave("tdf100581.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:pPr/w:pStyle", "val", "FrameContents");

    // w:sectPr is not exported
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:pPr/w:sectPr", 0);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"
        "/v:textbox/w:txbxContent/w:p[1]/w:pPr/w:pStyle", "val", "FrameContents");

    // w:sectPr is not exported
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"
        "/v:textbox/w:txbxContent/w:p[1]/w:pPr/w:sectPr", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112287)
{
    loadAndSave("tdf112287.docx");
    // tdf#131775: Check if correct spacing.

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:pPr/w:framePr","vAnchor","margin");
}

CPPUNIT_TEST_FIXTURE(Test, testZOrderInHeader)
{
    loadAndSave("tdf120760_ZOrderInHeader.docx");
    // tdf#120760 Check that the Z-Order of the background is smaller than the front shape's.
    xmlDocUniquePtr pXml = parseExport("word/header2.xml");

    // Get the Z-Order of the background image and of the shape in front of it.
    sal_Int32 nBackground = getXPath(pXml, "/w:hdr/w:p[1]/w:r[1]/w:drawing/wp:anchor", "relativeHeight").toInt32();
    sal_Int32 nFrontShape = getXPath(pXml, "/w:hdr/w:p[1]/w:r[1]/mc:AlternateContent[2]"
        "/mc:Choice/w:drawing/wp:anchor", "relativeHeight").toInt32();

    // Assert that background is in the back.
    CPPUNIT_ASSERT(nBackground < nFrontShape);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
