/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <IDocumentSettingAccess.hxx>

#include <editsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx") || OString(filename) == "ooo39250-1-min.rtf";
    }
};

// TODO: the re-import doesn't work just yet, but that isn't a regression...
DECLARE_SW_EXPORT_TEST(testFlyInFly, "ooo39250-1-min.rtf", nullptr, Test)
{
    // check that anchor of text frame is in other text frame
    uno::Reference<text::XTextContent> const xAnchored(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAnchored.is());
    CPPUNIT_ASSERT_EQUAL(OUString(""), uno::Reference<container::XNamed>(xAnchored, uno::UNO_QUERY_THROW)->getName());
    uno::Reference<text::XText> const xAnchorText(xAnchored->getAnchor()->getText());
    uno::Reference<text::XTextFrame> const xAnchorFrame(xAnchorText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAnchorFrame.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Frame2"), uno::Reference<container::XNamed>(xAnchorFrame, uno::UNO_QUERY_THROW)->getName());
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF, "tdf121374_sectionHF.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL( OUString("footer"), xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 6, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF2, "tdf121374_sectionHF2.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
}

DECLARE_OOXMLEXPORT_TEST(testTdf126723, "tdf126723.docx")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), "ParaLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testendingSectionProps, "endingSectionProps.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT_EQUAL( OUString("General header"), xHeaderText->getString());

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of paragraphs", 2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section is RightToLeft", text::WritingMode2::RL_TB, getProperty<sal_Int16>(xSect, "WritingMode"));
    //regression: tdf124637
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(2540), getProperty<sal_Int32>(xSect, "SectionLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTbrlTextbox, "tbrl-textbox.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // Without the accompanying fix in place, this test would have failed with 'Expected: -90;
    // Actual: 0', i.e. tbRl writing direction was imported as lrTb.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-90),
                         aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf124637_sectionMargin, "tdf124637_sectionMargin.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // sections 0 and 1 must be related to footnotes...
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(2), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(0), getProperty<sal_Int32>(xSect, "SectionLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak, "tdf123636_newlinePageBreak.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //special case: split first empty paragraph in a section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 2, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak2, "tdf123636_newlinePageBreak2.docx")
{
    //WITHOUT SplitPgBreakAndParaMark: a following anchored shape should force a page break
    //CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 2, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2, ""), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak3, "tdf123636_newlinePageBreak3.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //proof case: split any non-empty paragraphs, not just the first paragraph of a section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 5, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocPtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "Last line on page 1");
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak4, "tdf123636_newlinePageBreak4.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //special case: an empty paragraph doesn't split (except if first paragraph).
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 3, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocPtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[2]/body/txt[1]/Text", 0);
}

DECLARE_OOXMLEXPORT_TEST(tdf123912_protectedForm, "tdf123912_protectedForm.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compatibility: Protect form", true,
                                 pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ) );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    if ( xSect.is() )
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", false, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_OOXMLEXPORT_TEST(testDateControl, "empty-date-control.odt")
{
    // Check that we did not lost the date control
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDraws->getCount());
    uno::Reference<drawing::XControlShape> xControl(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControl->getControl().is());

    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // We need to export date format and a dummy character (" ") for empty date control
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:dateFormat", "val", "dd/MM/yyyy");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t", u" ");
}

DECLARE_OOXMLEXPORT_TEST(testTdf121867, "tdf121867.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwEditShell* pEditShell = pTextDoc->GetDocShell()->GetEditShell();
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 3; Actual  : 0', i.e. page width zoom was lost on export.
    CPPUNIT_ASSERT_EQUAL(SvxZoomType::PAGEWIDTH, pEditShell->GetViewOptions()->GetZoomType());
}

DECLARE_OOXMLEXPORT_TEST(testParaAdjustDistribute, "para-adjust-distribute.docx")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was
    // left, not block.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(1), "ParaLastLineAdjust")));

    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(2), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(2), "ParaLastLineAdjust")));
}

DECLARE_OOXMLEXPORT_TEST(testInputListExport, "tdf122186_input_list.odt")
{
    // We need to make sure we don't export the text itself next to the input list field
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r", 5);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116371, "tdf116371.odt")
{
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, "RotateAngle"), 10);
    auto frameRect = getProperty<awt::Rectangle>(xShape, "FrameRect");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24070), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24188), frameRect.Width);
}

DECLARE_OOXMLEXPORT_TEST(testFrameSizeExport, "floating-tables-anchor.docx")
{
    // Make sure the table width is 4000
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tblPr/w:tblW", "w", "4000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf119201, "tdf119201.docx")
{
    // Visibility of shapes wasn't imported/exported, for now base printable property on that, too
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_MESSAGE("First shape should be visible.", getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("First shape should be printable.", getProperty<bool>(xShape, "Printable"));
    xShape = getShape(2);
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be visible.", !getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be printable.", !getProperty<bool>(xShape, "Printable"));
    xShape = getShape(3);
    CPPUNIT_ASSERT_MESSAGE("Third shape should be visible.", getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Third shape should be printable.", getProperty<bool>(xShape, "Printable"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf124594, "tdf124594.docx")
{
    xmlDocPtr pDump = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed, as the portion text was
    // only "Er horte leise Schritte hinter", which means the 1st line of the 2nd paragraph was
    // split into two by a Special portion, i.e. the top margin of the shape was too large.
    assertXPath(pDump, "/root/page/body/txt[2]/Text[1]", "Portion",
                "Er horte leise Schritte hinter sich. Das bedeutete nichts Gutes. Wer wurde ihm ");
}

DECLARE_OOXMLEXPORT_TEST(testTextInput, "textinput.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Ensure we have a formfield
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText", " FILLIN \"\"");
    // and it's content is not gone
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:t", "SomeText");
}

DECLARE_OOXMLIMPORT_TEST(testTdf123460, "tdf123460.docx")
{
    // check paragraph mark deletion at terminating moveFrom
    CPPUNIT_ASSERT_EQUAL(true,getParagraph( 2 )->getString().startsWith("Nunc"));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 1 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(2), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, getRun( getParagraph( 2 ), 2 )->getString().endsWith("tellus."));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 3 )->getString());
    bool bCaught = false;
    try
    {
        getRun( getParagraph( 2 ), 4 );
    }
    catch (container::NoSuchElementException&)
    {
        bCaught = true;
    }
    CPPUNIT_ASSERT_EQUAL(true, bCaught);
}

//tdf#113483: fix handling of non-ascii characters in bookmark names and instrText xml tags
DECLARE_OOXMLEXPORT_TEST(testTdf113483, "tdf113483_crossreflink_nonascii_bookmarkname.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // check whether test file keeps non-ascii values or not
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart[1]", "name", OUString::fromUtf8("Els\u0151"));
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[2]/w:instrText[1]", OUString::fromUtf8(" REF Els\u0151 \\h "));
}

//tdf#125298: fix charlimit restrictions in bookmarknames and field references if they contain non-ascii characters
DECLARE_OOXMLEXPORT_TEST(testTdf125298, "tdf125298_crossreflink_nonascii_charlimit.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // check whether test file keeps non-ascii values or not
    OUString bookmarkName1 = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart[1]", "name");
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00e1rv\u00edzt\u0171r\u0151_t\u00fck\u00f6rf\u00far\u00f3g\u00e9p"), bookmarkName1);

    OUString bookmarkName2 = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:bookmarkStart[1]", "name");
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00e91\u00e12\u01713\u01514\u00fa5\u00f66\u00fc7\u00f38\u00ed9"), bookmarkName2);
    OUString fieldName1 = getXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[2]/w:instrText[1]");
    OUString expectedFieldName1(" REF ");
    expectedFieldName1 += bookmarkName1;
    expectedFieldName1 += " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName1, fieldName1);
    OUString fieldName2 = getXPathContent(pXmlDoc, "/w:document/w:body/w:p[7]/w:r[2]/w:instrText[1]");
    OUString expectedFieldName2(" REF ");
    expectedFieldName2 += bookmarkName2;
    expectedFieldName2 += " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName2, fieldName2);
}

DECLARE_OOXMLIMPORT_TEST(testTdf121784, "tdf121784.docx")
{
    // check tracked insertion of footnotes
    CPPUNIT_ASSERT_EQUAL( OUString( "Text1" ), getParagraph( 1 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 1 ), 2 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "1" ), getRun( getParagraph( 1 ), 3 )->getString());

    // check tracked insertion of endnotes
    CPPUNIT_ASSERT_EQUAL( OUString( "texti" ), getParagraph( 2 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 2 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(2), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "i" ), getRun( getParagraph( 2 ), 3 )->getString());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf125657, "tdf125657.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    auto checkAttrIsInt = [&](const OString& sAttrName) {
        OUString sAttr = getXPath(pXmlDoc,
                                  "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:inline/a:graphic/"
                                  "a:graphicData/pic:pic/pic:blipFill/a:srcRect",
                                  sAttrName);
        OString sAssertMsg("Attribute " + sAttrName + " value " + sAttr.toUtf8()
                           + " is not a valid integer");
        CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(), !sAttr.isEmpty());
        // Only decimal characters allowed, optionally prepended with '-'; no '.'
        CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(),
                               sAttr[0] == '-' || (sAttr[0] >= '0' && sAttr[0] <= '9'));
        for (sal_Int32 i = 1; i < sAttr.getLength(); ++i)
            CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(), sAttr[i] >= '0' && sAttr[i] <= '9');
    };
    // check that we export all coordinates of srcRect as integers
    checkAttrIsInt("l");
    checkAttrIsInt("t");
    checkAttrIsInt("r");
    checkAttrIsInt("b");
}

DECLARE_OOXMLEXPORT_TEST(testTdf125324, "tdf125324.docx")
{
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/tab/infos/bounds", "top", "4193");
}

DECLARE_OOXMLEXPORT_TEST(testTdf78657, "tdf78657_picture_hyperlink.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    xmlDocPtr pXmlRels = parseExport("word/_rels/document.xml.rels");
    if (!pXmlDoc || !pXmlRels)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/wp:docPr/a:hlinkClick", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:nvPicPr/pic:cNvPr/a:hlinkClick", 1);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Target='http://www.google.com']", "TargetMode", "External");
}

DECLARE_OOXMLEXPORT_TEST(testTdf125518, "tdf125518.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // First diagram is anchored
    OUString anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:drawing/wp:anchor/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Object1"), anchorName);

    // Second diagram has anchor
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:drawing/wp:anchor/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Objekt1"), anchorName);

    // Third diagram has no anchor
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:r[2]/w:drawing/wp:inline/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Object2"), anchorName);

    // 4th diagram has anchor too
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:r[3]/w:drawing/wp:anchor/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Object3"), anchorName);
}

DECLARE_OOXMLEXPORT_TEST(tdf127085, "tdf127085.docx")
{
    // Fill transparency was lost during export
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, "FillTransparence"));
}

DECLARE_OOXMLEXPORT_TEST(tdf118169, "tdf118169.docx")
{
    // Unicode characters were converted to question marks.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString(u"őőőőőőőőőőőűűűű"), getProperty<OUString>(xPropertySet, "Label"));
}

DECLARE_OOXMLEXPORT_TEST(tdf119809, "tdf119809.docx")
{
    // Combobox without an item list lost during import
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<OUString> >(xPropertySet, "StringItemList").getLength());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf121663, "tdf121663.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    // auto distance of line numbering is 0.5 cm
    assertXPath(pXmlDoc, "//w:lnNumType", "distance", "283");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
