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

DECLARE_OOXMLEXPORT_TEST(testTdf153042_largeTab, "tdf153042_largeTab.docx")
{
    // This is not the greatest test because it is slightly weird, and has a different layout
    // in MS Word 2010/2003 than it does in Word 2019. This tests for the 2019 layout.
    // Additionally (in Word 2019), going to paragraph properties and hitting OK changes the layout.
    // It changes back by going to outline numbering properties and hitting OK.

    // export does not keep the tabstop when exporting non-numbering. (Probably a good thing...)
    if (isExported())
        return;

    const auto& pLayout = parseLayoutDump();
    // Ensure a large tabstop is used in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion", "width", "1701");
}

DECLARE_OOXMLEXPORT_TEST(testTdf153042_noTab, "tdf153042_noTab.docx")
{
    // This is not the greatest test because it is slightly weird.
    // It is the same as the "largeTab" file, except the paragraph properties were viewed
    // and OK'ed, and now it looks like how Word 2010 and 2003 were laying it out.
    // Amazingly, LO is handling both documents correctly at the moment, so let's unit test that...

    // export does not keep the tabstop when exporting non-numbering. (Probably a good thing...)
    if (isExported())
        return;

    const auto& pLayout = parseLayoutDump();
    // Ensure a miniscule tab is used in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion", "width", "10");
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

DECLARE_OOXMLEXPORT_TEST(testTdf146984_anchorInShape, "tdf146984_anchorInShape.docx")
{
    // This was only one page b/c the page break was missing.
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    const auto& pLayout = parseLayoutDump();
    // There are shapes on both pages - these should be non-zero numbers
    assertXPath(pLayout, "//page[1]//anchored", 3);
    assertXPath(pLayout, "//page[2]//anchored", 2);
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

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak6, "tdf153613_anchoredAfterPgBreak6.docx")
{
    // An anchored TO character image, followed by more characters moves to the following page
    // The difference from test 2 is that it is not the first character run
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());

    const auto& pLayout = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(OUString("y"), getXPathContent(pLayout, "//page[2]/body/txt[1]"));
    assertXPath(pLayout, "//page[1]//anchored", 1); // DID YOU FIX ME? This should be page[2]
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

DECLARE_OOXMLEXPORT_TEST(testTdf153613_textboxAfterPgBreak3, "tdf153613_textboxAfterPgBreak3.docx")
{
    // All anchored TO-character shapes stay on the first page, before the page break.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//page[2]//anchored", 0);
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
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

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
    CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, "ParaBackColor"));

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
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

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
    CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied to paragraph 11, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(12, u""), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_numberingAfterBreak14, "tdf153964_numberingAfterBreak14.docx")
{
    //Numbering should only apply once in a split paragraph.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, "How numbering affected by a column break?"), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(3, "How is numbering affected by a page break?"), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(4, "x"), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("3."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(5, "column break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(6, "y"), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("3."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(7, "page break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_firstIndentAfterBreak14, "tdf153964_firstIndentAfterBreak14.docx")
{
    //First line indents should only apply once in a split paragraph.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, "How is first line indent affected by a column break?"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, "ParaFirstLineIndent"));
    xPara.set(getParagraph(3, "How is first line indent affected by a page break?"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, "ParaFirstLineIndent"));
    xPara.set(getParagraph(4, "x"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, "ParaFirstLineIndent"));
    xPara.set(getParagraph(5, "column break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaFirstLineIndent"));
    xPara.set(getParagraph(6, "y"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, "ParaFirstLineIndent"));
    xPara.set(getParagraph(7, "page break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaFirstLineIndent"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf76022_textboxWrap)
{
    // Granted, this is an ODT with a bit of an anomoly - tables ignore fly wrapping.
    createSwDoc("tdf76022_textboxWrap.odt");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you make wrapping sane/interoperable?", 1, getPages());

    // When saving to DOCX, the table should obey the fly wrapping
    reload("Office Open XML Text", "");

    // The fly takes up the whole page, so the table needs to shift down to the next page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
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

CPPUNIT_TEST_FIXTURE(Test, testExportingUnknownStyleInRedline)
{
    // This must not fail assertions
    loadAndReload("UnknownStyleInRedline.docx");
    // Check that the original unknown style name "UnknownStyle" is roundtripped
    // (maybe this is wrong, because Word does not do this).
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:pPr/w:pPrChange/w:pPr/w:pStyle[@w:val='UnknownStyle']");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148026)
{
    loadAndReload("tdf148026.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '//w:hyperlink' number of nodes is incorrect
    // i.e. a HYPERLINK field was exported instead of the hyperlink XML element.
    assertXPath(pXmlDoc, "//w:hyperlink", "tgtFrame", "_self");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153664)
{
    loadAndReload("Table-of-Figures.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");
    CPPUNIT_ASSERT(pXmlStyles);
    // Without the fix this was styleId='FigureIndex1' and name was "Figure Index 1"
    // This led to style settings being reset when ToF was updated in Word
    // TOF's paragraph style should be exported as "Table of Figures" as that's the default Word style name
    assertXPath(pXmlStyles, "/w:styles/w:style[12]", "styleId", "TableofFigures");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableofFigures']/w:name", "val", "Table of Figures");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
