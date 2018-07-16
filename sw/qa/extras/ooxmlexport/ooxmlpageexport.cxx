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
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <ftninfo.hxx>
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
};

DECLARE_OOXMLEXPORT_TEST(testFdo64350, "fdo64350.docx")
{
    // The problem was that page border shadows were not exported
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(
        getStyles("PageStyles")->getByName("Standard"), "ShadowFormat");
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
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    uno::Reference<text::XTextRange> xHeaderParagraph = getParagraphOfText(1, xHeaderText);
    table::BorderLine2 aHeaderBottomBorder
        = getProperty<table::BorderLine2>(xHeaderParagraph, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x622423), aHeaderBottomBorder.Color);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(106), aHeaderBottomBorder.InnerLineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.LineDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7), aHeaderBottomBorder.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aHeaderBottomBorder.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(26), aHeaderBottomBorder.OuterLineWidth);

    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference<text::XTextRange> xFooterParagraph = getParagraphOfText(1, xFooterText);
    table::BorderLine2 aFooterTopBorder
        = getProperty<table::BorderLine2>(xFooterParagraph, "TopBorder");
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
    table::ShadowFormat aShadow
        = getProperty<table::ShadowFormat>(getParagraph(2), "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    // w:sz="48" is in eights of a point, 1 pt is 20 twips.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(24 / 8 * 20)), aShadow.ShadowWidth);
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

DECLARE_OOXMLEXPORT_TEST(testMultiPageToc, "multi-page-toc.docx")
{
    // Import of this document triggered an STL assertion.

    // Document has a ToC from its second paragraph.
    uno::Reference<container::XNamed> xTextSection
        = getProperty<uno::Reference<container::XNamed>>(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT_EQUAL(OUString("Table of Contents1"), xTextSection->getName());
    // There should be a field in the header as well.
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    CPPUNIT_ASSERT_EQUAL(
        OUString("TextFieldStart"),
        getProperty<OUString>(getRun(getParagraphOfText(1, xHeaderText), 1), "TextPortionType"));
}

DECLARE_OOXMLEXPORT_TEST(testPageBreakBefore, "page-break-before.docx")
{
    // This was style::BreakType_PAGE_BEFORE, i.e. page break wasn't ignored, as it should have been.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65655, "fdo65655.docx")
{
    // The problem was that the DOCX had a non-blank odd footer and a blank even footer
    // The 'Different Odd & Even Pages' was turned on
    // However - LO assumed that because the 'even' footer is blank - it should ignore the 'Different Odd & Even Pages' flag
    // So it did not import it and did not export it
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    bool bValue = false;
    xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bValue);
    xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bValue);
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
    CPPUNIT_ASSERT_EQUAL(OUString("This is the header for the REST OF THE FILE."),
                         parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the header for the REST OF THE FILE."),
                         parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo68418, "fdo68418.docx")
{
    // The problem was that in 'MSWordExportBase::SectionProperties' function in 'wrt8sty.cxx'
    // it checked if it 'IsPlausableSingleWordSection'.
    // The 'IsPlausableSingleWordSection' compared different aspects of 2 'SwFrameFormat' objects.
    // One of the checks was 'do both formats have the same distance from the top and bottom ?'
    // This check is correct if both have headers or both don't have headers.
    // However - if one has a header, and the other one has an empty header (no header) - it is not correct to compare
    // between them (same goes for 'footer').
    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference<text::XTextRange> xFooterParagraph = getParagraphOfText(1, xFooterText);

    // First page footer is empty, second page footer is 'aaaa'
    CPPUNIT_ASSERT_EQUAL(OUString("aaaa"),
                         xFooterParagraph->getString()); // I get an error that it expects ''
}

DECLARE_OOXMLEXPORT_TEST(testA4AndBorders, "a4andborders.docx")
{
    /*
    * The problem was that in case of a document with borders, the pgSz attribute
    * was exported as a child of pgBorders, thus being ignored on reload.
    * We assert dimension against A4 size in mm (to avoid minor rounding errors)
    */
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Width (mm)", sal_Int32(210),
                                 getProperty<sal_Int32>(xPageStyle, "Width") / 100);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Height (mm)", sal_Int32(297),
                                 getProperty<sal_Int32>(xPageStyle, "Height") / 100);
}

DECLARE_OOXMLEXPORT_TEST(testFdo68787, "fdo68787.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // This was 25, the 'lack of w:separator' <-> '0 line width' mapping was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92470_footnoteRestart, "tdf92470_footnoteRestart.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote doesn't restart every Page", FTNNUM_PAGE,
                                 pDoc->GetFootnoteInfo().eNum);

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25),
                         getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlGrabBag, "customxml.docx")
{
    // The problem was that item[n].xml and itemProps[n].xml and .rels files for item[n].xml
    // files were missing from docx file after saving file.
    // This test case tests whether customxml files grabbagged properly in correct object.

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
    bool CustomXml = false;
    for (int i = 0; i < aGrabBag.getLength(); ++i)
    {
        if (aGrabBag[i].Name == "OOXCustomXml" || aGrabBag[i].Name == "OOXCustomXmlProps")
        {
            CustomXml = true;
            uno::Reference<xml::dom::XDocument> aCustomXmlDom;
            uno::Sequence<uno::Reference<xml::dom::XDocument>> aCustomXmlDomList;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aCustomXmlDomList); // PropertyValue of proper type
            sal_Int32 length = aCustomXmlDomList.getLength();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), length);
            aCustomXmlDom = aCustomXmlDomList[0];
            CPPUNIT_ASSERT(aCustomXmlDom.get()); // Reference not empty
        }
    }
    CPPUNIT_ASSERT(CustomXml); // Grab Bag has all the expected elements
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlRelationships, "customxml.docx")
{
    xmlDocPtr pXmlDoc = parseExport("customXml/_rels/item1.xml.rels");
    if (!pXmlDoc)
        return;

    // Check there is a relation to itemProps1.xml.
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship", 1);
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "itemProps1.xml");
}

DECLARE_OOXMLEXPORT_TEST(testFootnoteParagraphTag, "testFootnote.docx")
{
    /* In footnotes.xml, the paragraph tag inside <w:footnote w:id="2"> was getting written into document.xml.
    * Check for, paragraph tag is correctly written into footnotes.xml.
    */
    xmlDocPtr pXmlFootnotes = parseExport("word/footnotes.xml");
    if (!pXmlFootnotes)
        return;
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]", "id", "2");
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r/w:footnoteRef", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTestTitlePage, "testTitlePage.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
                         parseDump("/root/page[last()]/footer/txt/text()"));
}

// Checks that all runs of the field have text properties.
// Old behaviour: only first run has text properties of the field
//
// There are several runs are used in fields:
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::StartRunProperties() / DocxAttributeOutput::EndRunProperties().
//         </w:rPr>
//         <w:fldChar w:fldCharType="begin" />
//     </w:r>
//         <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:instrText>TIME \@"HH:mm:ss"</w:instrText>
//     </w:r>
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:fldChar w:fldCharType="separate" />
//     </w:r>
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:t>14:01:13</w:t>
//         </w:r>
//     <w:r>
//         <w:rPr>
//             <!-- properties written with DocxAttributeOutput::DoWriteFieldRunProperties()
//         </w:rPr>
//         <w:fldChar w:fldCharType="end" />
//     </w:r>
// See, tdf#38778
DECLARE_OOXMLEXPORT_TEST(testTdf38778, "tdf38778_properties_in_run_for_field.doc")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    const OUString psz("20");
    const OUString pszCs("20");

    // w:fldCharType="begin"
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:sz", "val", psz);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w:szCs", "val", pszCs);

    // PAGE
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:rPr/w:sz", "val", psz);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:rPr/w:szCs", "val", pszCs);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:instrText", " PAGE ");

    // w:fldCharType="separate"
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]/w:rPr/w:sz", "val", psz);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]/w:rPr/w:szCs", "val", pszCs);

    // field result: 1
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:rPr/w:sz", "val", psz);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:rPr/w:szCs", "val", pszCs);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:t", "1"); // field result

    // w:fldCharType="end"
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[7]/w:rPr/w:sz", "val", psz);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[7]/w:rPr/w:szCs", "val", pszCs);
}

DECLARE_OOXMLEXPORT_TEST(testSdtContent, "SdtContent.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:hdr[1]/w:sdt[1]/w:sdtContent[1]/w:p[1]/w:del[1]");
}

DECLARE_OOXMLEXPORT_TEST(testTdf96750_landscapeFollow, "tdf96750_landscapeFollow.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, "IsLandscape"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf86926_A3, "tdf86926_A3.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(42000), getProperty<sal_Int32>(xStyle, "Height"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(29700), getProperty<sal_Int32>(xStyle, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95367_inheritFollowStyle, "tdf95367_inheritFollowStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("header"), parseDump("/root/page[2]/header/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testInheritFirstHeader, "inheritFirstHeader.docx")
{
    // First page headers always link to last used first header, never to a follow header
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    xCursor->jumpToLastPage();
    OUString sPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("Last Header"), xHeaderText->getString());

    xCursor->jumpToPreviousPage();
    sPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("First Header"), xHeaderText->getString());

    xCursor->jumpToPreviousPage();
    sPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName(sPageStyleName), "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("Follow Header"), xHeaderText->getString());
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

DECLARE_OOXMLEXPORT_TEST(testfdo78469, "fdo78469.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    // make sure dataBinding & text tags not present in sdtcontent
    assertXPath(pXmlDoc,
                "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:dataBinding[1]",
                0);
    assertXPath(pXmlDoc, "/w:hdr[1]/w:tbl[1]/w:tr[1]/w:tc[2]/w:p[1]/w:sdt[2]/w:sdtPr[1]/w:text[1]",
                0);
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
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc(xFootnoteSupp->getFootnotes(),
                                                            uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote(xFootnoteIdxAcc->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xFootnoteText->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<text::XTextRange> xTextRange;
    xParaEnum->nextElement();
    xParaEnum->nextElement() >>= xTextRange;
    OUString sFootnotePara = xTextRange->getString();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph starts with W(87), not tab(9)", u'W', sFootnotePara[0]);
}

DECLARE_OOXMLEXPORT_TEST(testfdo78907, "fdo78907.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page");

    xmlDocPtr pXmlDoc1 = parseExport("word/footer1.xml");
    if (!pXmlDoc1)
        return;
    assertXPath(pXmlDoc1, "/w:ftr[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl", 0);
}

DECLARE_OOXMLEXPORT_TEST(tdf118702, "tdf118702.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:type", "val", "nextPage");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:sectPr/w:pgSz", "orient", "landscape");
}

DECLARE_OOXMLEXPORT_TEST(testFdo77129, "fdo77129.docx")
{
    // The problem was that text after TOC field was missing if footer reference  comes in field.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Data was lost from this paragraph.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[1]/w:t", "Abstract");
}

DECLARE_OOXMLEXPORT_TEST(test2colHeader, "2col-header.docx")
{
    // Header was lost on export when the document had multiple columns.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
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
    assertXPath(pXmlDoc, "/w:ftr/w:sdt/w:sdtContent/w:p[1]/w:sdt/w:sdtContent/w:r[1]/w:t",
                "1/2/2013");
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

DECLARE_OOXMLEXPORT_TEST(testVMLData, "TestVMLData.docx")
{
    // The problem was exporter was exporting vml data for shape in w:rPr element.
    // vml data should not come under w:rPr element.
    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc,
                            "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape",
                            "stroked")
                       .match("f"));
}

DECLARE_OOXMLEXPORT_TEST(testImageData, "image_data.docx")
{
    // The problem was exporter was exporting v:imagedata data for shape in w:pict as v:fill w element.

    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(
        getXPath(pXmlDoc,
                 "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape/v:imagedata",
                 "detectmouseclick")
            .match("t"));
}

DECLARE_OOXMLEXPORT_TEST(testFDO73546, "FDO73546.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:hdr/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor",
                "distL", "0");
}

DECLARE_OOXMLEXPORT_TEST(testFdo76979, "fdo76979.docx")
{
    // The problem was that black was exported as "auto" fill color, resulting in well-formed, but invalid XML.
    xmlDocPtr pXmlDoc = parseExport("word/header2.xml");
    if (!pXmlDoc)
        return;
    // This was "auto", not "FFFFFF".
    assertXPath(pXmlDoc, "//wps:spPr/a:solidFill/a:srgbClr", "val", "FFFFFF");
}

DECLARE_OOXMLEXPORT_TEST(testFdo78957, "fdo78957.docx")
{
    xmlDocPtr pXmlHeader = parseExport("word/header2.xml");

    if (!pXmlHeader)
        return;

    const sal_Int64 IntMax = SAL_MAX_INT32;
    sal_Int64 cx = 0, cy = 0;
    cx = getXPath(pXmlHeader,
                  "/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                  "wp:anchor[1]/wp:extent[1]",
                  "cx")
             .toInt64();
    cy = getXPath(pXmlHeader,
                  "/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/"
                  "wp:anchor[1]/wp:extent[1]",
                  "cy")
             .toInt64();
    //  Here we check the values of extent width & height
    CPPUNIT_ASSERT(cx <= IntMax);
    CPPUNIT_ASSERT(cy >= 0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo49940, "fdo49940.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xPara->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aValue);
}

DECLARE_OOXMLEXPORT_TEST(testFdo79738, "fdo79738.docx")
{
    uno::Reference<style::XStyleFamiliesSupplier> xStylesSupplier(mxComponent,
                                                                  uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xStyleFamilies = xStylesSupplier->getStyleFamilies();
    uno::Reference<container::XNameContainer> xStyles;
    xStyleFamilies->getByName("ParagraphStyles") >>= xStyles;
    uno::Reference<beans::XPropertySet> xPropertySetHeader(xStyles->getByName("Header"),
                                                           uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false,
                         xPropertySetHeader->getPropertyValue("ParaLineNumberCount").get<bool>());
    uno::Reference<beans::XPropertySet> xPropertySetFooter(xStyles->getByName("Footer"),
                                                           uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false,
                         xPropertySetFooter->getPropertyValue("ParaLineNumberCount").get<bool>());
}

DECLARE_OOXMLEXPORT_TEST(testRHBZ1180114, "rhbz1180114.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLEXPORT_TEST(testN780843, "n780843.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    OUString aStyleName = getProperty<OUString>(xPara, "PageStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aStyleName);

    //tdf64372 this document should only have one page break (2 pages, not 3)
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testN780843b, "n780843b.docx")
{
    // Same document as testN780843 except there is more text before the continuous break. Now the opposite footer results should happen.
    uno::Reference<text::XTextRange> xPara = getParagraph(3);
    OUString aStyleName = getProperty<OUString>(xPara, "PageStyleName");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(aStyleName),
                                                   uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText
        = getProperty<uno::Reference<text::XTextRange>>(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL(OUString("hidden footer"), xFooterText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testPageBorderShadow, "page-border-shadow.docx")
{
    // The problem was that in w:pgBorders, child elements had a w:shadow attribute, but that was ignored.
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(
        getStyles("PageStyles")->getByName("Standard"), "ShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    // w:sz="48" is in eights of a point, 1 pt is 20 twips.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(48 / 8 * 20)), aShadow.ShadowWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf104420, "tdf104420_lostParagraph.docx")
{
    // the add/remove dummy paragraph was losing an entire header and paragraph
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf41542_borderlessPadding, "tdf41542_borderlessPadding.odt")
{
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(tdf105490_negativeMargins, "tdf105490_negativeMargins.docx")
{
    // negative margins should change to minimal margins, not default margins.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf104061_tableSectionColumns, "tdf104061_tableSectionColumns.docx")
{
    CPPUNIT_ASSERT_MESSAGE("There should be two or three pages", getPages() <= 3);

    //tdf#95114 - follow style is Text Body - DOCX test
    uno::Reference<beans::XPropertySet> properties(
        getStyles("ParagraphStyles")->getByName("annotation subject"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("annotation text"),
                         getProperty<OUString>(properties, "FollowStyle"));
}

// base class to supply a helper method for testHFLinkToPrev
class testHFBase : public Test
{
protected:
    OUString getHFText(const uno::Reference<style::XStyle>& xPageStyle, const OUString& sPropName)
    {
        auto xTextRange = getProperty<uno::Reference<text::XTextRange>>(xPageStyle, sPropName);
        return xTextRange->getString();
    }
};

DECLARE_SW_EXPORT_TEST(testHFLinkToPrev, "headerfooter-link-to-prev.docx", nullptr, testHFBase)
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    // get LO page style for page 1, corresponding to docx section 1 first page
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check page 1 header & footer text
    CPPUNIT_ASSERT_EQUAL(OUString("First page header for all sections"),
                         getHFText(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer for section 1 only"),
                         getHFText(xPageStyle, "FooterText"));

    // get LO page style for page 2, corresponding to docx section 1
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header for section 1 only"),
                         getHFText(xPageStyle, "HeaderTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer for all sections"),
                         getHFText(xPageStyle, "FooterTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header for all sections"),
                         getHFText(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer for section 1 only"),
                         getHFText(xPageStyle, "FooterText"));

    // get LO page style for page 4, corresponding to docx section 2 first page
    xCursor->jumpToPage(4);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(OUString("First page header for all sections"),
                         getHFText(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer for sections 2 and 3 only"),
                         getHFText(xPageStyle, "FooterText"));

    // get LO page style for page 5, corresponding to docx section 2
    xCursor->jumpToPage(5);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header for sections 2 and 3 only"),
                         getHFText(xPageStyle, "HeaderTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer for all sections"),
                         getHFText(xPageStyle, "FooterTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header for all sections"),
                         getHFText(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer for sections 2 and 3 only"),
                         getHFText(xPageStyle, "FooterText"));

    // get LO page style for page 7, corresponding to docx section 3 first page
    xCursor->jumpToPage(7);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(OUString("First page header for all sections"),
                         getHFText(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer for sections 2 and 3 only"),
                         getHFText(xPageStyle, "FooterText"));

    // get LO page style for page 8, corresponding to docx section 3
    xCursor->jumpToPage(8);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header for sections 2 and 3 only"),
                         getHFText(xPageStyle, "HeaderTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer for all sections"),
                         getHFText(xPageStyle, "FooterTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header for all sections"),
                         getHFText(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer for sections 2 and 3 only"),
                         getHFText(xPageStyle, "FooterText"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc519228OddBreaks, "bnc519228_odd-breaksB.docx")
{
    // Check that all the normal styles are not set as right-only, those should be only those used after odd page breaks.
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL),
                         defaultStyle->getPropertyValue("PageStyleLayout"));
    uno::Reference<beans::XPropertySet> firstPage(getStyles("PageStyles")->getByName("First Page"),
                                                  uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL),
                         firstPage->getPropertyValue("PageStyleLayout"));

    OUString page1StyleName
        = getProperty<OUString>(getParagraph(1, "This is the first page."), "PageDescName");
    uno::Reference<beans::XPropertySet> page1Style(
        getStyles("PageStyles")->getByName(page1StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT),
                         page1Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText(1, getProperty<uno::Reference<text::XText>>(page1Style, "HeaderText"),
                       "This is the header for odd pages");

    // Page2 comes from follow of style for page 1 and should be a normal page. Also check the two page style have the same properties,
    // since page style for page1 was created from page style for page 2.
    OUString page2StyleName = getProperty<OUString>(page1Style, "FollowStyle");
    uno::Reference<beans::XPropertySet> page2Style(
        getStyles("PageStyles")->getByName(page2StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL),
                         page2Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText(1, getProperty<uno::Reference<text::XText>>(page2Style, "HeaderTextLeft"),
                       "This is the even header");
    getParagraphOfText(1, getProperty<uno::Reference<text::XText>>(page2Style, "HeaderTextRight"),
                       "This is the header for odd pages");
    CPPUNIT_ASSERT_EQUAL(getProperty<sal_Int32>(page1Style, "TopMargin"),
                         getProperty<sal_Int32>(page2Style, "TopMargin"));

    OUString page5StyleName = getProperty<OUString>(
        getParagraph(4, "Then an odd break after an odd page, should lead us to page #5."),
        "PageDescName");
    uno::Reference<beans::XPropertySet> page5Style(
        getStyles("PageStyles")->getByName(page5StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT),
                         page5Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText(1, getProperty<uno::Reference<text::XText>>(page5Style, "HeaderText"),
                       "This is the header for odd pages");
}

DECLARE_OOXMLEXPORT_TEST(testTdf99227, "tdf99227.docx")
{
    // A drawing anchored as character to a footnote caused write past end of document.xml at export to docx.
    // After that, importing after export failed with
    // SAXParseException: '[word/document.xml line 2]: Extra content at the end of the document', Stream 'word / document.xml',
    // and before commit ebf767eeb2a169ba533e1b2ffccf16f41d95df35, the drawing was silently lost.
    xmlDocPtr pXmlDoc = parseExport("word/footnotes.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "//w:footnote/w:p/w:r/w:drawing", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_footnoteStyle, "tdf82173_footnoteStyle.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was footnote text, which didn't match with newly created footnotes
    CPPUNIT_ASSERT_EQUAL(
        OUString("Footnote"),
        getProperty<OUString>(getParagraphOfText(1, xFootnoteText), "ParaStyleName"));

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("CharacterStyles")->getByName("Footnote Characters"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(xPageStyle, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00), getProperty<sal_Int32>(xPageStyle, "CharColor"));

    xPageStyle.set(getStyles("CharacterStyles")->getByName("Footnote anchor"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(xPageStyle, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00), getProperty<sal_Int32>(xPageStyle, "CharColor"));

    //tdf#118361 - in RTL locales, the footnote separator should still be left aligned.
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote separator LTR", sal_Int16(0),
                                 getProperty<sal_Int16>(aPageStyle, "FootnoteLineAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_endnoteStyle, "tdf82173_endnoteStyle.docx")
{
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    // character properties were previously not assigned to the footnote/endnote in-text anchor.
    CPPUNIT_ASSERT_EQUAL(24.0f, getProperty<float>(xEndnote->getAnchor(), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000),
                         getProperty<sal_Int32>(xEndnote->getAnchor(), "CharColor"));

    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;
    // This was Endnote Symbol
    CPPUNIT_ASSERT_EQUAL(
        OUString("Endnote"),
        getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x993300),
                         getProperty<sal_Int32>(getParagraphOfText(1, xEndnoteText), "CharColor"));

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("CharacterStyles")->getByName("Endnote Characters"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(xPageStyle, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF00FF), getProperty<sal_Int32>(xPageStyle, "CharColor"));

    xPageStyle.set(getStyles("CharacterStyles")->getByName("Endnote anchor"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), getProperty<sal_Int32>(xPageStyle, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF00FF), getProperty<sal_Int32>(xPageStyle, "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf55427_footnote2endnote, "tdf55427_footnote2endnote.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("ParagraphStyles")->getByName("Footnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote style is rose color", sal_Int32(0xFF007F),
                                 getProperty<sal_Int32>(xPageStyle, "CharColor"));
    xPageStyle.set(getStyles("ParagraphStyles")->getByName("Endnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Endnote style is cyan3 color", sal_Int32(0x2BD0D2),
                                 getProperty<sal_Int32>(xPageStyle, "CharColor"));

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    // The footnote numbering type of ARABIC will not transfer over when those footnotes are converted to endnotes.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote numbering type", SVX_NUM_ARABIC,
                                 pDoc->GetFootnoteInfo().aFormat.GetNumberingType());
    // The original document has a real endnote using ROMAN_LOWER numbering, so that setting MUST remain unchanged.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Endnote numbering type", SVX_NUM_ROMAN_LOWER,
                                 pDoc->GetEndNoteInfo().aFormat.GetNumberingType());

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);

    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;

    // ODT footnote-at-document-end's closest DOCX match is an endnote, so the two imports will not exactly match by design.
    if (!mbExported)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("original footnote count", sal_Int32(5),
                                     xFootnotes->getCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("original endnote count", sal_Int32(1), xEndnotes->getCount());

        uno::Reference<text::XFootnote> xFootnote;
        xFootnotes->getByIndex(0) >>= xFootnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("original footnote's number", OUString("1"),
                                     xFootnote->getAnchor()->getString());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("original endnote's number", OUString("i"),
                                     xEndnote->getAnchor()->getString());

        uno::Reference<text::XText> xFootnoteText;
        xFootnotes->getByIndex(0) >>= xFootnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "original footnote style", OUString("Footnote"),
            getProperty<OUString>(getParagraphOfText(1, xFootnoteText), "ParaStyleName"));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "original endnote style", OUString("Endnote"),
            getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName"));
    }
    else
    {
        // These asserted items are major differences in the conversion from footnote to endnote, NOT necessary conditions for a proper functioning document.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("At-Document-End footnotes were converted into endnotes",
                                     sal_Int32(0), xFootnotes->getCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("At-Document-End footnotes became endnotes", sal_Int32(6),
                                     xEndnotes->getCount());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("converted footnote's number", OUString("i"),
                                     xEndnote->getAnchor()->getString());
        xEndnotes->getByIndex(4) >>= xEndnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("original endnote's new number", OUString("v"),
                                     xEndnote->getAnchor()->getString());

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "retained footnote style", OUString("Footnote"),
            getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName"));
        xEndnotes->getByIndex(4) >>= xEndnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "original endnote style", OUString("Endnote"),
            getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf99074, "tdf99074.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> const xController(xModel->getCurrentController(),
                                                                  uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> const xViewSettings(xController->getViewSettings());

    // The behavior changed - Word 2013 and 2016 ignore this setting on
    // import, and instead honor the user's setting.
    // Let's ignore the <w:view w:val="web"/> too.
    CPPUNIT_ASSERT(!getProperty<bool>(xViewSettings, "ShowOnlineLayout"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf107618, "tdf107618.doc")
{
    // This was false, header was lost on export.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf105095, "tdf105095.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content
    // was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("\tfootnote"), xTextRange->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf106062_nonHangingFootnote, "tdf106062_nonHangingFootnote.odt")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content was lost on import.
    CPPUNIT_ASSERT_MESSAGE("Footnote starts with a tab", xTextRange->getString().startsWith("\t"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118361_RTLfootnoteSeparator, "tdf118361_RTLfootnoteSeparator.docx")
{
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote separator RTL", sal_Int16(2),
                                 getProperty<sal_Int16>(aPageStyle, "FootnoteLineAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789, "tdf90789.docx")
{
    uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape->getAnchor() != nullptr);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(),
                                                   uno::UNO_QUERY_THROW);
    xCtrl->select(uno::makeAny(xShape->getAnchor()));

    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl,
                                                                          uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursor> xTextCursor(xTextViewCursorSupplier->getViewCursor(),
                                                      uno::UNO_QUERY_THROW);
    uno::Reference<text::XPageCursor> xPageCursor(xTextCursor.get(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xPageCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789_2, "tdf90789-2.docx")
{
    // Section break before frame and shape was ignored
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testFdo69649, "fdo69649.docx")
{
    // The DOCX containing the Table of Contents was not imported with correct page nos
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes(xIndexSupplier->getDocumentIndexes(),
                                                     uno::UNO_QUERY);
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xTOCIndex->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextRange->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor(), uno::UNO_QUERY);
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aTocString(xTextCursor->getString());

    // heading 15 on the 15th page
    aTocString = aTocString.copy(aTocString.indexOf("Heading 15.1:\t") + strlen("Heading 15.1:\t"));
    CPPUNIT_ASSERT(aTocString.startsWithIgnoreAsciiCase("15"));
}

DECLARE_OOXMLEXPORT_TEST(testI124106, "i124106.docx")
{
    // This was 2.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(test_extra_image, "test_extra_image.docx")
{
    // fdo#74652 Check there is no shape added to the doc during import
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testFootnote, "footnote.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFootnote(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString aFootnote = xFootnote->getString();
    // Ensure there are no additional newlines after "bar".
    CPPUNIT_ASSERT(aFootnote.endsWith("bar"));
}

DECLARE_OOXMLEXPORT_TEST(testChtOutlineNumberingOoxml, "chtoutline.docx")
{
    const sal_Unicode aExpectedPrefix[2] = { 0x7b2c, 0x0020 };
    const sal_Unicode aExpectedSuffix[2] = { 0x0020, 0x7ae0 };
    uno::Reference<text::XChapterNumberingSupplier> xChapterNumberingSupplier(mxComponent,
                                                                              uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xChapterNumberingSupplier->getChapterNumberingRules());
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix, aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix, SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix, SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_OOXMLEXPORT_TEST(testTdf90611, "tdf90611.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was 11.
    CPPUNIT_ASSERT_EQUAL(10.f,
                         getProperty<float>(getParagraphOfText(1, xFootnoteText), "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf90810, "tdf90810short.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XFootnotesSupplier> xFootnoteSupp(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc(xFootnoteSupp->getFootnotes(),
                                                            uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote(xFootnoteIdxAcc->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    rtl::OUString sFootnoteText = xFootnoteText->getString();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(89), sFootnoteText.getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf78902, "tdf78902.docx")
{
    // This hung in layout.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf115719, "tdf115719.docx")
{
    // This was a single page, instead of pushing the textboxes to the second
    // page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf116410, "tdf116410.docx")
{
    // Opposite of the above, was 2 pages, should be 1 page.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf107969, "tdf107969.docx")
{
    // A VML object in a footnote's tracked changes caused write past end of document.xml at export to docx.
    // After that, importing after export failed with
    // SAXParseException: '[word/document.xml line 2]: Extra content at the end of the document', Stream 'word/document.xml'.
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
