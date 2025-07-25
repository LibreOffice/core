/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>

#include <comphelper/processfactory.hxx>
#include <o3tl/cppunittraitshelper.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testFDO76248)
{
    loadAndSave("FDO76248.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // In two cases the a:graphicData elements had no children, which is invalid.
    assertXPath(pXmlDoc, "//a:graphicData[not(*)]", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTscp, "tscp.docx")
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, u"urn:bails"_ustr);
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);

    assertXPath ( pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText","val",u"%1" );
}

CPPUNIT_TEST_FIXTURE(Test, testDecimalNumberingNoLeveltext)
{
    loadAndSave("decimal-numbering-no-leveltext.docx");
    // This was "%1", not empty: we turned a kind-of-none numbering into a decimal one.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText","val",u"");
}

CPPUNIT_TEST_FIXTURE(Test, testNoDuplicateAttributeExport)
{
    loadAndSave("duplicate-east-asia.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // File asserting while saving in LO.
    parseExport(u"word/document.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79008)
{
    loadAndReload("fdo79008.docx");
    /* File crashing while saving in LO.
     * Check if document.xml file is created after fix
     */
    parseExport(u"word/document.xml"_ustr);

    // tdf#134951: there is only one comment
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/comments.xml"_ustr);
    assertXPath(pXmlSettings, "/w:comments/w:comment", 1);

    // Read-only is set, but it is not enforced, so it should be off...
    CPPUNIT_ASSERT(!getSwDocShell()->IsSecurityOptOpenReadOnly());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf120852_readOnlyProtection)
{
    createSwDoc("tdf120852_readOnlyProtection.docx");
    // Read-only is set, so Enforcement must enable it.
    CPPUNIT_ASSERT(getSwDocShell()->IsSecurityOptOpenReadOnly());

    saveAndReload(mpFilter);

    CPPUNIT_ASSERT(getSwDocShell()->IsSecurityOptOpenReadOnly());

    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", u"1");
    assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", u"readOnly");

}

CPPUNIT_TEST_FIXTURE(Test, testTdf120852_readOnlyUnProtected)
{
    auto verify = [this](bool bIsExport = false) {
        // Readonly is not enforced, just a suggestion,
        // so when a section is protected, the document should enable forms protection.
        CPPUNIT_ASSERT(!getSwDocShell()->IsSecurityOptOpenReadOnly());

        uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY_THROW);
        const sal_Int32 nLastSection = xSections->getCount() - 1;
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(nLastSection), uno::UNO_QUERY_THROW);
        if ( !bIsExport )
        {
            CPPUNIT_ASSERT_MESSAGE("Section is not protected", !getProperty<bool>(xSect, u"IsProtected"_ustr));
            // Enable section protection. The round-trip should have forms protection enabled.
            xSect->setPropertyValue(u"IsProtected"_ustr, uno::Any(true));
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("Section is protected", getProperty<bool>(xSect, u"IsProtected"_ustr));
            xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
            assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", u"forms");
            assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", u"true");
        }
    };

    createSwDoc("tdf120852_readOnlyUnProtected.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorPropertySdt)
{
    loadAndSave("author-property.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "xpath", u"/ns1:coreProperties[1]/ns0:creator[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "storeItemID",u"{6C3C8BC8-F283-45AE-878A-BAB7291924A1}");
    // FIXME: the next property doesn't match, though it's correct in theory. A bug in assertXPath?
    // assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "prefixMappings",
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // there is only one table in the test file
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[1]", "w", u"1601");
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[2]", "w", u"7844");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76587 )
{
    loadAndSave("fdo76587.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "line", u"240");
    assertXPath(pXmlDoc, "/w:styles/w:style[8]/w:pPr/w:spacing", "lineRule", u"auto");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77890 )
{
    loadAndSave("fdo77890.docx");
    /*
    Ensure that the page break is preserved i.e it should not be converted to a section break, in case
    if the different first page is set for the pages in the document.
    For additional comments please refer to https://www.libreoffice.org/bugzilla/show_bug.cgi?id=77890#c2
    */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:br", "type", u"page");
}

CPPUNIT_TEST_FIXTURE(Test, testNumberedList)
{
    loadAndReload("NumberedList.docx");
    //fdo74150:In document.xml, for pStyle = "NumberedList1", iLvl and numId was not preserved
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:pStyle", "val", u"NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:ilvl","val", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:numId","val", u"0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:pStyle","val", u"NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:ilvl","val", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:numId","val", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131819)
{
    loadAndSave("NumberedList.docx");
    // keep width of fixed size cells in the nested table
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // These were 4030 and 4249.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblGrid/w:gridCol[1]", "w", u"3841");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblGrid/w:gridCol[2]", "w", u"4050");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131959)
{
    loadAndSave("NumberedList.docx");
    // import tblInd from table style
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // These were 0.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblInd", "w", u"360");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblInd", "w", u"360");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131203)
{
    loadAndSave("tdf131203.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // loading thrown divide_by_zero()
    assertXPath(pXmlDoc, "//w:tbl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO76597)
{
    loadAndSave("fdo76597.docx");
    // check XML
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "before", u"96");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "after", u"120");
}

CPPUNIT_TEST_FIXTURE(Test, testContentTypeTIF)
{
    loadAndSave("fdo77476.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc, "/ContentType:Types/ContentType:Override[@ContentType='image/tiff']", "PartName", u"/word/media/image1.tif");
}

DECLARE_OOXMLEXPORT_TEST(testFDO77117, "fdo77117.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This checks textbox textrun size of font which is in group shape.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xShape, u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTable)
{
    loadAndSave("fdo77887.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "horzAnchor", u"margin");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "leftFromText", u"141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "rightFromText", u"141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpXSpec", u"center");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpY", u"2266");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "vertAnchor", u"page");

    //make sure not to write empty attributes which requires enumeration
    assertXPathNoAttribute(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpYSpec");
}

CPPUNIT_TEST_FIXTURE(Test, testOldComplexMergeRight)
{
    loadAndSave("tdf90681-1.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:vMerge", "val", u"restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:vMerge", "val", u"continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[2]/w:tcPr/w:vMerge", "val", u"continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[2]/w:tcPr/w:vMerge", "val", u"continue");
}

CPPUNIT_TEST_FIXTURE(Test, testOldComplexMergeleft)
{
    loadAndSave("tdf90681-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:vMerge", "val", u"restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:vMerge", "val", u"continue");
}

CPPUNIT_TEST_FIXTURE(Test, testOldComplexMergeTableInTable)
{
    loadAndSave("ooo96040-2.odt");
    parseExport(u"word/document.xml"_ustr);

    // Check tdf#161202 - this document has all kinds of tables inside hidden sections.
    // The page count must be 13, but for unclear reason, it is 12 in some tests on Linux
    // (maybe the layout hasn't finished?).
    // Without the fix, it was 52.
    CPPUNIT_ASSERT_LESSEQUAL(13, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkContainingPlaceholderField)
{
    loadAndReload("hyperlink-field.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    parseExport(u"word/document.xml"_ustr);

    // tdf#148380 output unknown field's contents("") instead of field name("Sender" aka e-mail)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getParagraph(1)->getString().indexOf("Sender"));
}

CPPUNIT_TEST_FIXTURE(Test, testTablePreferredWidth)
{
    loadAndSave("tablePreferredWidth.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Problem :If the table preferred width is in percent, then after RT it changes to 0 & width type changes
    // to 'auto' instead of 'pct'.
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "w", u"3000");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "type",u"pct");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO75431)
{
    loadAndSave("fdo75431.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "//w:tbl", 2);
    assertXPath(pXmlDoc, "//w:p/w:pPr/w:sectPr/w:type", "val", u"nextPage");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77725)
{
    loadAndSave("fdo77725.docx");
    xmlDocUniquePtr pXmlFootnotes = parseExport(u"word/footnotes.xml"_ustr);

    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[1]", 0);
    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[2]", 0);
    assertXPath(pXmlFootnotes, "//w:footnotes[1]/w:footnote[3]/w:p[3]/w:r[1]/w:br[3]", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testFieldRotation)
{
    loadAndReload("field-rotated.fodt");
    uno::Reference<text::XTextRange> const xRun(getRun(uno::Reference<text::XTextRange>(getParagraphOrTable(1), uno::UNO_QUERY), 1));
    uno::Reference<text::XTextField> const xField(getProperty<uno::Reference<text::XTextField>>(xRun, u"TextField"_ustr));
    CPPUNIT_ASSERT(xField.is());
    CPPUNIT_ASSERT_EQUAL(u"DocInformation:Title"_ustr, xField->getPresentation(true));
    CPPUNIT_ASSERT_EQUAL(u"Rotationeering"_ustr, xField->getPresentation(false));
    // problem was that the rotation wasn't applied to all runs of the field
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), getProperty<sal_Int16>(xRun, u"CharRotation"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFootnoteSeparator)
{
    loadAndSave("footnotesep.fodt");
    // footnote separator definitions - taken from default page style
    xmlDocUniquePtr pXmlFootnotes = parseExport(u"word/footnotes.xml"_ustr);
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]", "id", u"0");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]", "type", u"separator");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]/w:p[1]/w:r[1]/w:separator", 0);
    // use paragraph font size to simulate height
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[1]/w:p[1]/w:pPr[1]/w:rPr[1]/w:sz", "val", u"12");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]", "id", u"1");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]", "type", u"continuationSeparator");
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]/w:p[1]/w:r[1]/w:continuationSeparator", 0);
    assertXPath(pXmlFootnotes, "/w:footnotes[1]/w:footnote[2]/w:p[1]/w:pPr[1]/w:rPr[1]/w:sz", "val", u"12");

    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings[1]/w:footnotePr[1]/w:footnote[1]", "id", u"0");
    assertXPath(pXmlSettings, "/w:settings[1]/w:footnotePr[1]/w:footnote[2]", "id", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121441)
{
    loadAndSave("tdf121441.docx");
    xmlDocUniquePtr pXmlFootnotes = parseExport(u"word/footnotes.xml"_ustr);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r[1]/w:rPr/w:rStyle", 1);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r[2]/w:rPr/w:rStyle", 0);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r[2]/w:rPr/w:rFonts", 0);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "eastAsia", u"Symbol");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "cs", u"Symbol");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "ascii", u"Symbol");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:rFonts", "hAnsi", u"Symbol");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77812)
{
    loadAndSave("fdo77812.docx");
    /* Additional sectPr was getting inserted and hence Column properties
     * were getting added into this additional sectPr instead of Default setPr.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Check no additional section break is inserted.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:sectPr", 0);

    // Check w:cols comes under Default sectPr
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols", "num", u"2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[1]", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[2]", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testContentTypeOLE)
{
    loadAndSave("fdo77759.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.spreadsheetml.sheet']",
                "PartName",
                u"/word/embeddings/oleObject1.xlsx");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.xlsx']",
        "Type",
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        u"Excel.Sheet.12");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78420)
{
    loadAndSave("fdo78420.docx");
    xmlDocUniquePtr pXmlHeader = parseExport(u"word/header2.xml"_ustr);

    xmlDocUniquePtr pXmlHeaderRels = parseExport(u"word/_rels/header2.xml.rels"_ustr);

    assertXPath(pXmlHeaderRels,"/rels:Relationships/rels:Relationship[1]","Id",u"rId1");
}


CPPUNIT_TEST_FIXTURE(Test, testPageBreakInFirstPara)
{
    loadAndReload("fdo77727.docx");
    /* Break to next page was not exported if it is in first paragraph of the section.
     * Now after fix , LO writes Next Page Break and also preserves <w:br> tag.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br","type",u"page");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78284)
{
    loadAndSave("fdo78284.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDoc,"/ContentType:Types/ContentType:Override[@PartName='/word/media/OOXDiagramDataRels1_0.png']",
                        "ContentType",
                        u"image/png");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78384)
{
    loadAndReload("fdo78384.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:rFonts","ascii",u"Wingdings");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78469)
{
    loadAndSave("fdo78469.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/header1.xml"_ustr);
    // make sure dataBinding & text tags not present in sdtcontent
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:dataBinding[1]",0);
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:text[1]",0);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78887)
{
    loadAndSave("fdo78887.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:t[1]", u"Lyrics: ");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:br[2]", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO78887b)
{
    loadAndSave("missing_newline.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[2]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/w:t[1]", u"Tab and line break");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[5]/w:br[1]", 1);
    assertXPathContent(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[6]/w:t[1]", u"New line");
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78651)
{
    loadAndSave("fdo78651.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // ensure that there are only two tables
    assertXPath(pXmlDoc, "//w:tbl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78882)
{
    loadAndSave("fdo78882.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

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

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);

    // Ensure that after fix LO is preserving AutoSpacing property in styles.xml
    assertXPath ( pXmlDoc, "/w:styles[1]/w:style[@w:styleId='Title']/w:pPr[1]/w:spacing[1]", "beforeAutospacing", u"1" );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79540)
{
    loadAndSave("fdo79540.docx");
    /* Issue was, <w:drawing> was getting written inside <w:drawing>.
     * So postpone the writing of Inner Drawing tag.
     * MS Office does not allow nesting of drawing tags.
     */

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Ensure that two separate w:drawing tags are written and they are not nested.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO79062)
{
    loadAndSave("fdo79062.docx");
    xmlDocUniquePtr pXmlFootNotes = parseExport(u"word/footnotes.xml"_ustr);
    assertXPath(pXmlFootNotes, "/w:footnotes", "Ignorable", u"w14 wp14 w15");

    xmlDocUniquePtr pXmlEndNotes = parseExport(u"word/endnotes.xml"_ustr);
    assertXPath(pXmlEndNotes, "/w:endnotes", "Ignorable", u"w14 wp14 w15");

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
    CPPUNIT_ASSERT_EQUAL(u" Microsoft Office."_ustr, xFootnoteText->getString());

    // Ensure that paragraph markers are not lost.
    xFootnoteText.set(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in second footnote", 2, getParagraphs(xFootnoteText) );


    // tdf#136706: Two useless page styles were created for each of the four footnotes.
    CPPUNIT_ASSERT( !getStyles(u"PageStyles"_ustr)->hasByName(u"Converted8"_ustr) );
    CPPUNIT_ASSERT( !getStyles(u"PageStyles"_ustr)->hasByName(u"Converted1"_ustr) );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79668)
{
    loadAndReload("fdo79668.docx");
    // fdo#79668: Document was Crashing on DebugUtil build while Saving
    // because of repeated attribute value in same element.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:pPr's  w:shd attributes were getting added to w:pPrChange/w:pPr's w:shd hence checking
    // w:fill for both shd elements
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:shd", "fill", u"FFFFFF" );
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pPrChange/w:pPr/w:shd", "fill", u"FFFFFF" );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78907)
{
    loadAndReload("fdo78907.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", u"page" );

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"word/footer1.xml"_ustr);
    assertXPath ( pXmlDoc1, "/w:ftr[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl", 0 );
}

CPPUNIT_TEST_FIXTURE(Test, tdf118702)
{
    loadAndReload("tdf118702.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:type", "val", u"nextPage" );
    assertXPath ( pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:pgSz", "orient", u"landscape" );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79822)
{
    loadAndSave("fdo79822.docx");
    /* File getting crash while saving in LO.
     * The Docx contain smartart and the file was created in ms word 2007
     */
    parseExport(u"word/document.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO79915)
{
    loadAndSave("fdo79915.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[9]/w:t", u"How much buoyancy does the water provide?");
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79817)
{
    loadAndSave("fdo79817.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "storeItemID", u"{9222E47B-A68B-4AEB-9855-21C912B9D3D2}");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "xpath", u"/ns0:properties[1]/documentManagement[1]/ns2:Responsible_x0020_Officer_x0020_Title[1]");
}


CPPUNIT_TEST_FIXTURE(Test, testfdo79968_sldx)
{
    loadAndSave("fdo79968.docx");
    // This UT for DOCX embedded with powerpoint slide
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.presentationml.slide']",
                "PartName",
                u"/word/embeddings/oleObject1.sldx");

    // check the rels too
    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
        "/rels:Relationships/rels:Relationship[@Target='embeddings/oleObject1.sldx']",
        "Type",
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    // check the content too
    xmlDocUniquePtr pXmlDocContent = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocContent,
        "/w:document/w:body/w:p[1]/w:r/w:object/o:OLEObject",
        "ProgID",
        u"PowerPoint.Slide.12");
}

CPPUNIT_TEST_FIXTURE(Test, testLostFooterStyle)
{
    // Given a document with many numberings and a Footer para style:
    loadAndSave("lost-footer-style.docx");

    // When saving that to DOCX and checking the Footer style:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed, the Footer style was
    // lost on export due to the many ListLabel char styles.
    OUString aType = getXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Footer']", "type");

    // Then make sure we have a Footer para style:
    CPPUNIT_ASSERT_EQUAL(u"paragraph"_ustr, aType);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
