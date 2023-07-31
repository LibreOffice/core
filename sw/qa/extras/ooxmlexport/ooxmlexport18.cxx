/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <queue>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/util/XRefreshable.hpp>


#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}
};

CPPUNIT_TEST_FIXTURE(Test, testTdf150197_predefinedNumbering)
{
    createSwDoc();

    // The exact numbering style doesn't matter - just any non-bullet pre-defined numbering style.
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Numbering 123")) },
        { "FamilyName", uno::Any(OUString("NumberingStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(1), "ListLabelString"));

    reload("Office Open XML Text", "");
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(1), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testInlineSdtHeader)
{
    // Without the accompanying fix in place, this test would have failed with an assertion failure,
    // we produced not-well-formed XML on save.
    loadAndSave("inline-sdt-header.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testCellSdtRedline)
{
    // Without the accompanying fix in place, this test would have failed with an assertion failure,
    // we produced not-well-formed XML on save.
    loadAndSave("cell-sdt-redline.docx");
}

DECLARE_OOXMLEXPORT_TEST(testTdf148956_directEndFormatting, "tdf148956_directEndFormatting.docx")
{
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:GotoEndOfPara", {});
    if (!isExported())
    {
        CPPUNIT_ASSERT_MESSAGE(
            "Has direct formatting",
            pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetpSwpHints());
    }
    else
    {
        CPPUNIT_ASSERT_MESSAGE(
            "Direct formatting cleared",
            !pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetpSwpHints());
    }

    pWrtShell->SttPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:ResetAttributes", {});

    dispatchCommand(mxComponent, ".uno:GotoEndOfPara", {});

    CPPUNIT_ASSERT_MESSAGE(
        "Direct formatting cleared",
        !pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode()->GetpSwpHints());
}

DECLARE_OOXMLEXPORT_TEST(testTdf147646, "tdf147646_mergedCellNumbering.docx")
{
    parseLayoutDump();
    //Without the fix in place, it would have failed with
    //- Expected: 2.
    //- Actual  : 4.
    CPPUNIT_ASSERT_EQUAL(OUString("2."),parseDump("/root/page/body/tab/row[4]/cell/txt/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']","expand"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153526_commentInNumbering, "tdf153526_commentInNumbering.docx")
{
    // an exception was prematurely ending finishParagraph, losing numbering and CRs
    // so before the patch, this was 6.
    CPPUNIT_ASSERT_EQUAL(13, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153592_columnBreaks)
{
    loadAndSave("tdf153592_columnBreaks.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // The two column breaks were lost on import. (I wouldn't complain if they were at 3,5)
    assertXPath(pXmlDoc, "//w:br", 2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf104394_lostTextbox, "tdf104394_lostTextbox.docx")
{
    // This was only one page b/c the textbox was missing.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf154129_framePr1, "tdf154129_framePr1.docx")
{
    for (size_t i = 1; i < 4; ++i)
    {
        uno::Reference<drawing::XShape> xTextFrame = getShape(i);
        // The anchor is defined in the style, and only the first style was checked, not the parents
        auto nAnchor = getProperty<sal_Int16>(xTextFrame, "HoriOrientRelation");
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nAnchor);
        nAnchor = getProperty<sal_Int16>(xTextFrame, "VertOrientRelation");
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nAnchor);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf154703_framePr, "tdf154703_framePr.docx")
{
    // the frame conversion had been failing, so it imported as plain text only.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTdf154703_framePr2, "tdf154703_framePr2.rtf")
{
    // framePr frames are always imported as fully transparent
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(getShape(1), "FillTransparence"));

    if (!isExported())
    {
        // Fill the frame with a red background. It should be transferred on export to the paragraph
        uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
        xFrame->setPropertyValue("FillColor", uno::Any(COL_RED));
        xFrame->setPropertyValue("FillTransparence", uno::Any(static_cast<sal_Int32>(0)));

        return;
    }

    // exported: framed paragraphs without a background should now have a red background
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:body/w:p[1]/w:pPr/w:shd", "fill", "800000");
    assertXPath(pXmlDoc, "//w:body/w:p[2]/w:pPr/w:shd", "fill", "548DD4"); // was blue already, no change
    assertXPath(pXmlDoc, "//w:body/w:p[3]/w:pPr/w:shd", "fill", "800000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak, "tdf153613_anchoredAfterPgBreak.docx")
{
    const auto& pLayout = parseLayoutDump();
    // An anchored TO character image anchors before the page break.
    assertXPath(pLayout, "//page[1]//anchored", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak2, "tdf153613_anchoredAfterPgBreak2.docx")
{
    const auto& pLayout = parseLayoutDump();
    // An anchored TO character image, followed by more characters moves to the following page
    assertXPath(pLayout, "//page[2]//anchored", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak3, "tdf153613_anchoredAfterPgBreak3.docx")
{
    const auto& pLayout = parseLayoutDump();
    // An anchored TO character image, with setting splitPgBreakAndParaMark moves to the following page
    assertXPath(pLayout, "//page[2]//anchored", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_inlineAfterPgBreak, "tdf153613_inlineAfterPgBreak.docx")
{
    const auto& pLayout = parseLayoutDump();
    // An inline AS character image moves to the following page when after the page break.
    assertXPath(pLayout, "//page[2]//anchored", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_inlineAfterPgBreak2, "tdf153613_inlineAfterPgBreak2.docx")
{
    // An inline AS character image moves to the following page when after the page break.
    // The difference from the previous test is that it is not the first character run
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());

    const auto& pLayout = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(OUString("x"), getXPathContent(pLayout, "//page[1]/body/txt[2]"));
    assertXPath(pLayout, "//page[2]//anchored", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_textboxAfterPgBreak, "tdf153613_textboxAfterPgBreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

    const auto& pLayout = parseLayoutDump();
    assertXPathContent(pLayout, "//page[2]/body/txt", "There should be no prior carriage return.");
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_textboxAfterPgBreak2, "tdf153613_textboxAfterPgBreak2.docx")
{
    // same as previous example, except that it is the first paragraph in the section.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    const auto& pLayout = parseLayoutDump();
    assertXPathContent(pLayout, "//page[2]/body/txt", "There should be no prior carriage return.");
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_sdtAfterPgBreak, "tdf153613_sdtAfterPgBreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_topMarginAfterBreak14, "tdf153964_topMarginAfterBreak14.docx")
{
    //The top margin should only apply once in a split paragraph.
    //In this compat14 (Windows 2010) version, it applies after the break if no prior visible run.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, "a w:br at the start of the document. Does it use 60 point top margin?"), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    xPara.set(getParagraph(3, u"60 pt spacing before"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 3, so it shouldn't apply here
    xPara.set(getParagraph(4, u"column break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    xPara.set(getParagraph(5, u"60 pt followed by page break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 5, so it shouldn't apply here
    xPara.set(getParagraph(6, u"page break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied yet, so with compat14 it should apply here.
    xPara.set(getParagraph(7, u"column break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xe2efd9), getProperty<Color>(xPara, "ParaBackColor"));

    // In an odd twist, the w:br was actually at the end of the previous w:p, so in that case
    // we ignore the top margin definition this time.
    xPara.set(getParagraph(9, u"page break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    // The top margin was not applied before the column break, so with compat14 it should apply here
    xPara.set(getParagraph(10, u""), uno::UNO_QUERY); // after column break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, "ParaBackColor"));

    // In an odd twist, the w:br was actually at the end of the previous w:p, so in that case
    // we ignore the top margin definition this time.
    xPara.set(getParagraph(12, u""), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_topMarginAfterBreak15, "tdf153964_topMarginAfterBreak15.docx")
{
    //The top margin should only apply once (at most) in a split paragraph.
    //In this compat15 (Windows 2013) version, it never applies after the break.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, "a w:br at the start of the document. Does it use 60 point top margin?"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    xPara.set(getParagraph(3, u"60 pt spacing before"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 3, so it shouldn't apply here
    xPara.set(getParagraph(4, u"column break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    xPara.set(getParagraph(5, u"60 pt followed by page break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 5, so it shouldn't apply here
    xPara.set(getParagraph(6, u"page break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied to paragraph 6, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(7, u"column break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xe2efd9), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin not was applied to paragraph 8, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(9, u"page break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    // The top margin was not applied to paragraph 9, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(10, u""), uno::UNO_QUERY); // after column break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied to paragraph 11, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(12, u""), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149551_mongolianVert)
{
    // Given a docx document with a shape with vert="mongolianVert".
    createSwDoc("tdf149551_mongolianVert.docx");

    // The shape is imported as custom shape with attached frame.
    // Without fix the shape itself had WritingMode = 0 = LR_TB,
    // the frame in it had WritingMode = 2 = TB_RL.
    // It should be WritingMode = 3 = TB_LR in both cases.
    const sal_Int16 eExpected(text::WritingMode2::TB_LR);
    CPPUNIT_ASSERT_EQUAL(eExpected, getProperty<sal_Int16>(getShape(1), "WritingMode"));
    uno::Reference<beans::XPropertySet> xShapeProps(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrameProps(xShapeProps->getPropertyValue("TextBoxContent"),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(eExpected, getProperty<sal_Int16>(xFrameProps, "WritingMode"));

    // Such shape must have vert="mongolianVert" again after saving.
    // Without fix the orientation was vert="vert".
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//wps:bodyPr", "vert", "mongolianVert");
}

DECLARE_OOXMLEXPORT_TEST(testTdf151912, "tdf151912.docx")
{
    // For now just ensure roundtrip is successful

    //tdf#151548 - ensure block SDT preserves id (instead of random re-assignment)
    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:sdt//w:sdtPr/w:id", "val", "1802566103");
}

DECLARE_OOXMLEXPORT_TEST(testTdf147724, "tdf147724.docx")
{
    const auto& pLayout = parseLayoutDump();

    // Ensure we load field value from external XML correctly (it was "HERUNTERLADEN")
    assertXPathContent(pLayout, "/root/page[1]/body/txt[1]", "Placeholder -> *ABC*");

    // This SDT has no storage id, it is not an error, but content can be taken from any suitable XML
    // There 2 variants possible, both are acceptable
    OUString sFieldResult = getXPathContent(pLayout, "/root/page[1]/body/txt[2]");
    CPPUNIT_ASSERT(sFieldResult == "Placeholder -> *HERUNTERLADEN*" || sFieldResult == "Placeholder -> *ABC*");
}

DECLARE_OOXMLEXPORT_TEST(testTdf130782, "chart.docx")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xAccess(xTEOSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xAccess->getCount());
    uno::Reference<container::XNamed> xObj(xAccess->getByIndex(0), uno::UNO_QUERY);

    // these properties were not imported
    CPPUNIT_ASSERT_EQUAL(OUString("Diagramm 1"), xObj->getName());
    CPPUNIT_ASSERT_EQUAL(OUString("uninspired default chart"), getProperty<OUString>(xObj, "Title"));
    CPPUNIT_ASSERT_EQUAL(OUString("the description is here"), getProperty<OUString>(xObj, "Description"));
}

CPPUNIT_TEST_FIXTURE(Test, testNumberPortionFormatFromODT)
{
    // Given a document with a single paragraph, direct formatting asks 24pt font size for the
    // numbering and the text portion:
    createSwDoc("number-portion-format.odt");

    // When saving to DOCX:
    save("Office Open XML Text");

    // Then make sure that the paragraph marker's char format has that custom font size:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:pPr/w:rPr/w:sz' number of nodes is incorrect
    // i.e. <w:sz> was missing under <w:pPr>'s <w:rPr>.
    assertXPath(pXmlDoc, "//w:pPr/w:rPr/w:sz", "val", "48");
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleCharPosition)
{
    // Given a loaded document where the Normal paragraph style has <w:position w:val="-1">:
    createSwDoc("para-style-char-position.docx");

    // When saving it back to DOCX:
    save("Office Open XML Text");

    // Then make sure that is not turned into a normal subscript text:
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '/w:styles/w:style[@w:styleId='Normal']/w:rPr/w:position' number of nodes is incorrect
    // i.e. we wrote <w:vertAlign w:val="subscript"> instead of <w:position>.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Normal']/w:rPr/w:position", "val", "-1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150966_regularInset)
{
    // Given a docx document with a rectangular shape with height cy="900000" (EMU), tIns="180000"
    // and bIns="360000", resulting in 360000EMU text area height.
    createSwDoc("tdf150966_regularInset.docx");

    // The shape is imported as custom shape with attached frame.
    // The insets are currently imported as margin top="4.99mm" and bottom="10mm".
    // That should result in tIns="179640" and bIns="360000" on export.

    // Without fix the insets were tIns="359280" and bIns="539640". The text area had 1080Emu height
    // and Word displays no text at all.
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPathAttrs(pXmlDoc, "//wps:bodyPr", { { "tIns", "179640" }, { "bIns", "360000" } });
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152636_lostPageBreak)
{
    loadAndReload("tdf152636_lostPageBreak.odt");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152636_lostPageBreak2)
{
    loadAndReload("tdf152636_lostPageBreak2.docx");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtDuplicatedId)
{
    // Given a document with 2 inline <w:sdt>, with each a <w:id>:
    createSwDoc("sdt-duplicated-id.docx");

    // When exporting that back to DOCX:
    save("Office Open XML Text");

    // Then make sure we write 2 <w:sdt> and no duplicates:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 4
    // i.e. grab-bags introduced 2 unwanted duplicates.
    assertXPath(pXmlDoc, "//w:sdt", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testImageCropping)
{
    loadAndReload("crop-roundtrip.docx");

    // the image has no cropping after roundtrip, because it has been physically cropped
    // NB: this test should be fixed when the core feature to show image cropped when it
    // has the "GraphicCrop" is set is implemented
    auto aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Bottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152200)
{
    // Given a document with a fly anchored after a FORMTEXT in the end of the paragraph:
    createSwDoc("tdf152200-field+textbox.docx");

    // When exporting that back to DOCX:
    save("Office Open XML Text");

    // Then make sure that fldChar with type 'end' goes prior to the at-char anchored fly.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    const int nRunsBeforeFldCharEnd = countXPathNodes(pXmlDoc, "//w:fldChar[@w:fldCharType='end']/preceding::w:r");
    CPPUNIT_ASSERT(nRunsBeforeFldCharEnd);
    const int nRunsBeforeAlternateContent = countXPathNodes(pXmlDoc, "//mc:AlternateContent/preceding::w:r");
    CPPUNIT_ASSERT(nRunsBeforeAlternateContent);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 6
    // - Actual  : 5
    CPPUNIT_ASSERT_GREATER(nRunsBeforeFldCharEnd, nRunsBeforeAlternateContent);
    // Make sure we only have one paragraph in body, and only three field characters overall,
    // located directly in runs of this paragraph
    assertXPath(pXmlDoc, "/w:document/w:body/w:p");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:fldChar", 3);
    assertXPath(pXmlDoc, "//w:fldChar", 3); // no field characters elsewhere
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126477)
{
    loadAndReload("embedded_chart.odt");

    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xAccess(xTEOSupplier->getEmbeddedObjects());
    uno::Sequence<OUString> aSeq(xAccess->getElementNames());

    // Check number of embedded objects.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeq.getLength());

    uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier(xAccess->getByName(aSeq[0]),
                                                                  uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xObj(xEOSupplier->getEmbeddedObject());
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier2(xObj, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xAccess2(xTEOSupplier2->getEmbeddedObjects());
    uno::Sequence<OUString> aSeq2(xAccess2->getElementNames());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the chart lost in the embedded document.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeq2.getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152425)
{
    loadAndReload("tdf152425.docx");

    // Check that "List Number" and "List 5" styles don't get merged
    const OUString Para3Style = getProperty<OUString>(getParagraph(3), "ParaStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("Numbering 1"), Para3Style);
    const OUString Para4Style = getProperty<OUString>(getParagraph(4), "ParaStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("List 5 (WW)"), Para4Style);
    // Also check that "List 5" and "List Bullet 5" styles don't get merged
    const OUString Para5Style = getProperty<OUString>(getParagraph(5), "ParaStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("List 5"), Para5Style);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153104)
{
    loadAndReload("tdf153104.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    OUString numId = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:numPr/w:numId", "val");

    xmlDocUniquePtr pXmlNum = parseExport("word/numbering.xml");
    OString numPath = "/w:numbering/w:num[@w:numId='"
                      + OUStringToOString(numId, RTL_TEXTENCODING_ASCII_US) + "']/";

    // Check that first level's w:lvlOverride/w:startOverride is written correctly:
    // the list defines starting value of 10, which must be kept upon second level
    // numbering reset.
    // Without the fix, this would fail with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/w:numbering/w:num[@w:numId='3']/w:lvlOverride[@w:ilvl='0']/w:startOverride' number of nodes is incorrect
    assertXPath(pXmlNum, numPath + "w:lvlOverride[@w:ilvl='0']/w:startOverride", "val", "10");
    assertXPath(pXmlNum, numPath + "w:lvlOverride[@w:ilvl='1']/w:startOverride", "val", "1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153128)
{
    loadAndReload("tdf153128.docx");
    calcLayout();
    sal_Int32 nFirstLineHeight
        = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion", "height")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), nFirstLineHeight);

    // The text height is 1 pt, i.e. 20 twip; without the fix, it would fail with
    // - Expected less than: 30
    // - Actual  : 414
    CPPUNIT_ASSERT_LESS(sal_Int32(30), nFirstLineHeight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155945)
{
    createSwDoc("tdf155945.docx");

    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());
    // Without a fix in place, this would fail with
    // - Expected: 0
    // - Actual  : 423
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133560)
{
    createSwDoc("lastEmptyLineWithDirectFormatting.docx");

    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());
    // Without a fix in place, this would fail with
    // - Expected: 12
    // - Actual  : 48
    CPPUNIT_ASSERT_EQUAL(12.0f, getProperty<float>(getParagraph(4), "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150408_isLvl_RoundTrip)
{
    loadAndSave("listWithLgl.docx");

    // Second level's numbering should use Arabic numbers for first level reference
    auto xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("CH I"), getProperty<OUString>(xPara, "ListLabelString"));
    xPara = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString("Sect 1.01"), getProperty<OUString>(xPara, "ListLabelString"));
    xPara = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString("CH II"), getProperty<OUString>(xPara, "ListLabelString"));
    xPara = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("Sect 2.01"), getProperty<OUString>(xPara, "ListLabelString"));

    xmlDocUniquePtr pXml = parseExport("word/numbering.xml");
    assertXPath(pXml, "/w:numbering/w:abstractNum"); // Only one list
    // The second list level must keep the isLgl element
    assertXPath(pXml, "/w:numbering/w:abstractNum/w:lvl[2]/w:isLgl");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156548)
{
    // Given a document using two bookmarks with similar names longer than 40 characters
    loadAndReload("longBookmarkName.fodt");

    // After the export, the names must be no longer than 40 characters; they must be unique;
    // and the hyperlinks must use the same names, to still point to the correct targets:

    {
        // 1st  paragraph - hyperlink to 4th paragraph
        auto sURL = getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL");
        CPPUNIT_ASSERT_EQUAL(OUString("#A_bookmark_name_longer_than_forty_charac"), sURL);
        // 4th paragraph - a bookmark
        auto xBookmark = getProperty<uno::Reference<container::XNamed>>(getRun(getParagraph(4), 1),
                                                                        "Bookmark");
        CPPUNIT_ASSERT_EQUAL(OUString("A_bookmark_name_longer_than_forty_charac"),
                             xBookmark->getName());
    }

    {
        // 2nd  paragraph - hyperlink to 5th paragraph
        auto sURL = getProperty<OUString>(getRun(getParagraph(2), 1), "HyperLinkURL");
        CPPUNIT_ASSERT_EQUAL(OUString("#A_bookmark_name_longer_than_forty_chara1"), sURL);
        // 5th paragraph - a bookmark
        auto xBookmark = getProperty<uno::Reference<container::XNamed>>(getRun(getParagraph(5), 1),
                                                                        "Bookmark");
        CPPUNIT_ASSERT_EQUAL(OUString("A_bookmark_name_longer_than_forty_chara1"),
                             xBookmark->getName());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
