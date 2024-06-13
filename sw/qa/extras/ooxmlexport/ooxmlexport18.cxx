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

#include <com/sun/star/awt/FontStrikeout.hpp>
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
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testTdf150197_predefinedNumbering)
{
    createSwDoc();

    // The exact numbering style doesn't matter - just any non-bullet pre-defined numbering style.
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Numbering 123"_ustr) },
        { "FamilyName", uno::Any(u"NumberingStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));

    saveAndReload(u"Office Open XML Text"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(getParagraph(1), u"ListLabelString"_ustr));
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
    dispatchCommand(mxComponent, u".uno:GotoEndOfPara"_ustr, {});
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
    dispatchCommand(mxComponent, u".uno:ResetAttributes"_ustr, {});

    dispatchCommand(mxComponent, u".uno:GotoEndOfPara"_ustr, {});

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
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr,parseDump("/root/page/body/tab/row[4]/cell/txt/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr,"expand"_ostr));
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

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Ensure a large tabstop is used in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion"_ostr, "width"_ostr, u"1701"_ustr);
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

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Ensure a miniscule tab is used in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion"_ostr, "width"_ostr, u"10"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf154751_dualStrikethrough, "tdf154751_dualStrikethrough.docx")
{
    auto nStrike = getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharStrikeout"_ustr);
    CPPUNIT_ASSERT_EQUAL(awt::FontStrikeout::SINGLE, nStrike);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf154478)
{
    loadAndSave("tdf154478.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/comments.xml"_ustr);

    OUString aValues[5] = { u"Comment1 seen."_ustr, u"Comment2 seen."_ustr, u"Comment3 NOTseen."_ustr, u"Comment4 NOTseen."_ustr, u"Comment5 NOTseen."_ustr };
    for (size_t i = 1; i < 6; ++i)
    {
        OString sPath = "/w:comments/w:comment[" + OString::number(i) + "]/w:p/w:r/w:t";

        // Without the fix in place, this test would have failed with
        // - In <>, XPath '/w:comments/w:comment[3]/w:p/w:r/w:t' not found
        assertXPathContent(pXmlDoc, sPath, aValues[i - 1]);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153592_columnBreaks)
{
    loadAndSave("tdf153592_columnBreaks.docx");

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // The two column breaks were lost on import. (I wouldn't complain if they were at 3,5)
    assertXPath(pXmlDoc, "//w:br"_ostr, 2);
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
}

DECLARE_OOXMLEXPORT_TEST(testTdf127622_framePr, "tdf127622_framePr.docx")
{
    // All the paragraphs end up with the same frame definition, so put them all in one frame
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTdf105035_framePrB, "tdf105035_framePrB.docx")
{
    // The paragraphs have different frame definitions, so they must be in separate frames,
    // and the frames must not overlap - even though their vertical positions are identical.
    xmlDocUniquePtr pLayout = parseLayoutDump();
    sal_Int32 n1stFlyBottom
        = getXPath(pLayout, "//page[1]//anchored/fly[1]/infos/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 n2ndFlyTop
        = getXPath(pLayout, "//page[1]//anchored/fly[2]/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(n1stFlyBottom, n2ndFlyTop); //Top is greater than bottom

    // Impossible layout TODO: the textboxes are in the wrong order.
    OUString sTextBox1(u"Preparation of Papers for IEEE TRANSACTIONS and JOURNALS (November 2012)"_ustr);
    CPPUNIT_ASSERT_MESSAGE("DID YOU FIX ME? Wow - I didn't think this would be possible!",
        !getXPathContent(pLayout, "//page[1]//anchored/fly[1]/txt"_ostr).startsWith(sTextBox1));
}

DECLARE_OOXMLEXPORT_TEST(testTdf105035_framePrC, "tdf105035_framePrC.docx")
{
    // The paragraphs have different frame definitions, so they must be in separate frames,
    // and the frames DO overlap this time.
    xmlDocUniquePtr pLayout = parseLayoutDump();
    sal_Int32 n1stFlyTop
        = getXPath(pLayout, "//page[1]//anchored/fly[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 n2ndFlyTop
        = getXPath(pLayout, "//page[1]//anchored/fly[2]/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(n1stFlyTop, n2ndFlyTop); //both frames start at the same position
}

DECLARE_OOXMLEXPORT_TEST(testTdf154129_framePr1, "tdf154129_framePr1.docx")
{
    for (size_t i = 1; i < 4; ++i)
    {
        uno::Reference<drawing::XShape> xTextFrame = getShape(i);
        // The anchor is defined in the style, and only the first style was checked, not the parents
        auto nAnchor = getProperty<sal_Int16>(xTextFrame, u"HoriOrientRelation"_ustr);
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nAnchor);
        nAnchor = getProperty<sal_Int16>(xTextFrame, u"VertOrientRelation"_ustr);
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(getShape(1), u"FillTransparence"_ustr));

    // as opposed to testLibreOfficeHang (RTF != INVERT_BORDER_SPACING) do not duplicate left/right
    uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    CPPUNIT_ASSERT_EQUAL(u"framePr"_ustr, getParagraphOfText(1, xText)->getString());
    sal_Int32 nFrame = getProperty<sal_Int32>(getShape(1), u"LeftBorderDistance"_ustr);
    sal_Int32 nPara = getProperty<sal_Int32>(getParagraphOfText(1, xText), u"LeftBorderDistance"_ustr);
    if (!isExported()) // RTF
        CPPUNIT_ASSERT_EQUAL(sal_Int32(529), nFrame + nPara);
    else // DOCX
        CPPUNIT_ASSERT_EQUAL(sal_Int32(529*2), nFrame + nPara);

    if (!isExported())
    {
        // Fill the frame with a red background. It should be transferred on export to the paragraph
        uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
        xFrame->setPropertyValue(u"FillColor"_ustr, uno::Any(COL_RED));
        xFrame->setPropertyValue(u"FillTransparence"_ustr, uno::Any(static_cast<sal_Int32>(0)));

        return;
    }

    // exported: framed paragraphs without a background should now have a red background
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:body/w:p[1]/w:pPr/w:shd"_ostr, "fill"_ostr, u"800000"_ustr);
    assertXPath(pXmlDoc, "//w:body/w:p[2]/w:pPr/w:shd"_ostr, "fill"_ostr, u"548DD4"_ustr); // was blue already, no change
    assertXPath(pXmlDoc, "//w:body/w:p[3]/w:pPr/w:shd"_ostr, "fill"_ostr, u"800000"_ustr);
    assertXPath(pXmlDoc, "//w:body/w:p[3]/w:pPr/w:framePr"_ostr, "yAlign"_ostr, u"center"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//w:body/w:p[3]/w:pPr/w:framePr"_ostr, "y"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf154703_framePrWrapSpacing, "tdf154703_framePrWrapSpacing.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    if (!isExported())
        return;

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // before the fix, this was half of the correct value.
    assertXPath(pXmlDoc, "//w:body/w:p/w:pPr/w:framePr"_ostr, "hSpace"_ostr, u"2552"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf154703_framePrTextDirection, "tdf154703_framePrTextDirection.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::WritingMode2::TB_RL), getProperty<sal_Int16>(getShape(1), u"WritingMode"_ustr));
    if (!isExported())
        return;

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:body/w:p/w:pPr/w:textDirection"_ostr, "val"_ostr, u"tbRl"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak, "tdf153613_anchoredAfterPgBreak.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // An anchored TO character image anchors before the page break.
    assertXPath(pLayout, "//page[1]//anchored"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak2, "tdf153613_anchoredAfterPgBreak2.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // An anchored TO character image, followed by more characters moves to the following page
    assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak3, "tdf153613_anchoredAfterPgBreak3.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // An anchored TO character image, with setting splitPgBreakAndParaMark moves to the following page
    assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_anchoredAfterPgBreak6, "tdf153613_anchoredAfterPgBreak6.docx")
{
    // An anchored TO character image, followed by more characters moves to the following page
    // The difference from test 2 is that it is not the first character run
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());

    xmlDocUniquePtr pLayout = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(u"y"_ustr, getXPathContent(pLayout, "//page[2]/body/txt[1]"_ostr));
    assertXPath(pLayout, "//page[1]//anchored"_ostr, 1); // DID YOU FIX ME? This should be page[2]
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_inlineAfterPgBreak, "tdf153613_inlineAfterPgBreak.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // An inline AS character image moves to the following page when after the page break.
    assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_inlineAfterPgBreak2, "tdf153613_inlineAfterPgBreak2.docx")
{
    // An inline AS character image moves to the following page when after the page break.
    // The difference from the previous test is that it is not the first character run
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());

    xmlDocUniquePtr pLayout = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, getXPathContent(pLayout, "//page[1]/body/txt[2]"_ostr));
    assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_sdtAfterPgBreak, "tdf153613_sdtAfterPgBreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf153613_textboxAfterPgBreak3, "tdf153613_textboxAfterPgBreak3.docx")
{
    // both textboxes on the second (last) page
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pLayout = parseLayoutDump();
    assertXPath(pLayout, "//page[2]/body/txt/anchored/fly"_ostr, 2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_topMarginAfterBreak14, "tdf153964_topMarginAfterBreak14.docx")
{
    //The top margin should only apply once in a split paragraph.
    //In this compat14 (Windows 2010) version, it applies after the break if no prior visible run.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"a w:br at the start of the document. Does it use 60 point top margin?"_ustr), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    xPara.set(getParagraph(3, u"60 pt spacing before"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was applied to paragraph 3, so it shouldn't apply here
    xPara.set(getParagraph(4, u"column break1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    xPara.set(getParagraph(5, u"60 pt followed by page break"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was applied to paragraph 5, so it shouldn't apply here
    xPara.set(getParagraph(6, u"page break1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was not applied yet, so with compat14 it should apply here.
    xPara.set(getParagraph(7, u"column break2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xe2efd9), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // In an odd twist, the w:br was actually at the end of the previous w:p, so in that case
    // we ignore the top margin definition this time.
    xPara.set(getParagraph(9, u"page break2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));

    // The top margin was not applied before the column break, so with compat14 it should apply here
    xPara.set(getParagraph(10, u""_ustr), uno::UNO_QUERY); // after column break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // In an odd twist, the w:br was actually at the end of the previous w:p, so in that case
    // we ignore the top margin definition this time.
    xPara.set(getParagraph(12, u""_ustr), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_topMarginAfterBreak15, "tdf153964_topMarginAfterBreak15.docx")
{
    //The top margin should only apply once (at most) in a split paragraph.
    //In this compat15 (Windows 2013) version, it never applies after the break.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"a w:br at the start of the document. Does it use 60 point top margin?"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));

    xPara.set(getParagraph(3, u"60 pt spacing before"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was applied to paragraph 3, so it shouldn't apply here
    xPara.set(getParagraph(4, u"column break1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    xPara.set(getParagraph(5, u"60 pt followed by page break"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was applied to paragraph 5, so it shouldn't apply here
    xPara.set(getParagraph(6, u"page break1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was not applied to paragraph 6, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(7, u"column break2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xe2efd9), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin not was applied to paragraph 8, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(9, u"page break2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));

    // The top margin was not applied to paragraph 9, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(10, u""_ustr), uno::UNO_QUERY); // after column break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // The top margin was not applied to paragraph 11, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(12, u""_ustr), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_numberingAfterBreak14, "tdf153964_numberingAfterBreak14.docx")
{
    //Numbering should only apply once in a split paragraph.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"How numbering affected by a column break?"_ustr), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(3, u"How is numbering affected by a page break?"_ustr), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(4, u"x"_ustr), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("3."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(5, u"column break"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(6, u"y"_ustr), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(OUString("3."), getProperty<OUString>(xPara, "ListLabelString"));
    xPara.set(getParagraph(7, u"page break"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_firstIndentAfterBreak14, "tdf153964_firstIndentAfterBreak14.docx")
{
    //First line indents should only apply once in a split paragraph.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"How is first line indent affected by a column break?"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));
    xPara.set(getParagraph(3, u"How is first line indent affected by a page break?"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));
    xPara.set(getParagraph(4, u"x"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));
    xPara.set(getParagraph(5, u"column break"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));
    xPara.set(getParagraph(6, u"y"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));
    xPara.set(getParagraph(7, u"page break"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148834_lineNumbering)
{
    loadAndSave("tdf148834_lineNumbering.odt");

    xmlDocUniquePtr pStylesXml = parseExport(u"word/styles.xml"_ustr);
    // user specified: do not include in line numbering
    assertXPath(pStylesXml, "//w:style[@w:styleId='Normal']/w:pPr/w:suppressLineNumbers"_ostr, 1);
    // even though it matches the parent style, these should always avoid showing line numbering
    assertXPath(pStylesXml, "//w:style[@w:styleId='Footer']/w:pPr/w:suppressLineNumbers"_ostr, 1);
    assertXPath(pStylesXml,
                "//w:style[@w:styleId='0NUMBERED']/w:pPr/w:suppressLineNumbers"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157598)
{
    loadAndSave("tdf157598.docx");

    xmlDocUniquePtr pStylesXml = parseExport(u"word/styles.xml"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    assertXPath(pStylesXml, "//w:style[@w:styleId='Normal']/w:rPr/w:rtl"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf76022_textboxWrap)
{
    // Granted, this is an ODT with a bit of an anomaly - tables ignore fly wrapping.
    createSwDoc("tdf76022_textboxWrap.odt");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you make wrapping sane/interoperable?", 1, getPages());

    // When saving to DOCX, the table should obey the fly wrapping
    saveAndReload(u"Office Open XML Text"_ustr);

    // The fly takes up the whole page, so the table needs to shift down to the next page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf134114_allowOverlap, "tdf134114_allowOverlap.docx")
{
    // CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), u"AllowOverlap"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(getShape(2), u"AllowOverlap"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(eExpected, getProperty<sal_Int16>(getShape(1), u"WritingMode"_ustr));
    uno::Reference<beans::XPropertySet> xShapeProps(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrameProps(xShapeProps->getPropertyValue(u"TextBoxContent"_ustr),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(eExpected, getProperty<sal_Int16>(xFrameProps, u"WritingMode"_ustr));

    // Such shape must have vert="mongolianVert" again after saving.
    // Without fix the orientation was vert="vert".
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//wps:bodyPr"_ostr, "vert"_ostr, u"mongolianVert"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf151912, "tdf151912.docx")
{
    // For now just ensure roundtrip is successful

    //tdf#151548 - ensure block SDT preserves id (instead of random re-assignment)
    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:sdt//w:sdtPr/w:id"_ostr, "val"_ostr, u"1802566103"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf147724, "tdf147724.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();

    // Ensure we load field value from external XML correctly (it was "HERUNTERLADEN")
    assertXPathContent(pLayout, "/root/page[1]/body/txt[1]"_ostr, u"Placeholder -> *ABC*"_ustr);

    // This SDT has no storage id, it is not an error, but content can be taken from any suitable XML
    // There 2 variants possible, both are acceptable
    OUString sFieldResult = getXPathContent(pLayout, "/root/page[1]/body/txt[2]"_ostr);
    CPPUNIT_ASSERT(sFieldResult == "Placeholder -> *HERUNTERLADEN*" || sFieldResult == "Placeholder -> *ABC*");
}

DECLARE_OOXMLEXPORT_TEST(testTdf130782, "chart.docx")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xAccess(xTEOSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xAccess->getCount());
    uno::Reference<container::XNamed> xObj(xAccess->getByIndex(0), uno::UNO_QUERY);

    // these properties were not imported
    CPPUNIT_ASSERT_EQUAL(u"Diagramm 1"_ustr, xObj->getName());
    CPPUNIT_ASSERT_EQUAL(u"uninspired default chart"_ustr, getProperty<OUString>(xObj, u"Title"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"the description is here"_ustr, getProperty<OUString>(xObj, u"Description"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testNumberPortionFormatFromODT)
{
    // Given a document with a single paragraph, direct formatting asks 24pt font size for the
    // numbering and the text portion:
    createSwDoc("number-portion-format.odt");

    // When saving to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure that the paragraph marker's char format has that custom font size:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:pPr/w:rPr/w:sz' number of nodes is incorrect
    // i.e. <w:sz> was missing under <w:pPr>'s <w:rPr>.
    assertXPath(pXmlDoc, "//w:pPr/w:rPr/w:sz"_ostr, "val"_ostr, u"48"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleCharPosition)
{
    // Given a loaded document where the Normal paragraph style has <w:position w:val="-1">:
    createSwDoc("para-style-char-position.docx");

    // When saving it back to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure that is not turned into a normal subscript text:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '/w:styles/w:style[@w:styleId='Normal']/w:rPr/w:position' number of nodes is incorrect
    // i.e. we wrote <w:vertAlign w:val="subscript"> instead of <w:position>.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Normal']/w:rPr/w:position"_ostr, "val"_ostr, u"-1"_ustr);
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
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathAttrs(pXmlDoc, "//wps:bodyPr"_ostr, { { "tIns", "179640" }, { "bIns", "360000" } });
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
    save(u"Office Open XML Text"_ustr);

    // Then make sure we write 2 <w:sdt> and no duplicates:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 4
    // i.e. grab-bags introduced 2 unwanted duplicates.
    assertXPath(pXmlDoc, "//w:sdt"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testImageCropping)
{
    loadAndReload("crop-roundtrip.docx");

    // the image has no cropping after roundtrip, because it has been physically cropped
    // NB: this test should be fixed when the core feature to show image cropped when it
    // has the "GraphicCrop" is set is implemented
    auto aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(1), u"GraphicCrop"_ustr);
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
    save(u"Office Open XML Text"_ustr);

    // Then make sure that fldChar with type 'end' goes prior to the at-char anchored fly.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    const int nRunsBeforeFldCharEnd = countXPathNodes(pXmlDoc, "//w:fldChar[@w:fldCharType='end']/preceding::w:r"_ostr);
    CPPUNIT_ASSERT(nRunsBeforeFldCharEnd);
    const int nRunsBeforeAlternateContent = countXPathNodes(pXmlDoc, "//mc:AlternateContent/preceding::w:r"_ostr);
    CPPUNIT_ASSERT(nRunsBeforeAlternateContent);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 6
    // - Actual  : 5
    CPPUNIT_ASSERT_GREATER(nRunsBeforeFldCharEnd, nRunsBeforeAlternateContent);
    // Make sure we only have one paragraph in body, and only three field characters overall,
    // located directly in runs of this paragraph
    assertXPath(pXmlDoc, "/w:document/w:body/w:p"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:fldChar"_ostr, 3);
    assertXPath(pXmlDoc, "//w:fldChar"_ostr, 3); // no field characters elsewhere
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
    const OUString Para3Style = getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Numbering 1"_ustr, Para3Style);
    const OUString Para4Style = getProperty<OUString>(getParagraph(4), u"ParaStyleName"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"List 5 (WW)"_ustr, Para4Style);
    // Also check that "List 5" and "List Bullet 5" styles don't get merged
    const OUString Para5Style = getProperty<OUString>(getParagraph(5), u"ParaStyleName"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"List 5"_ustr, Para5Style);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153104)
{
    loadAndReload("tdf153104.docx");

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OUString numId = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr);

    xmlDocUniquePtr pXmlNum = parseExport(u"word/numbering.xml"_ustr);
    OString numPath = "/w:numbering/w:num[@w:numId='"
                      + OUStringToOString(numId, RTL_TEXTENCODING_ASCII_US) + "']/";

    // Check that first level's w:lvlOverride/w:startOverride is written correctly:
    // the list defines starting value of 10, which must be kept upon second level
    // numbering reset.
    // Without the fix, this would fail with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/w:numbering/w:num[@w:numId='3']/w:lvlOverride[@w:ilvl='0']/w:startOverride' number of nodes is incorrect
    assertXPath(pXmlNum, numPath + "w:lvlOverride[@w:ilvl='0']/w:startOverride", "val"_ostr, u"10"_ustr);
    assertXPath(pXmlNum, numPath + "w:lvlOverride[@w:ilvl='1']/w:startOverride", "val"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153128)
{
    loadAndReload("tdf153128.docx");
    calcLayout();
    sal_Int32 nFirstLineHeight
        = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr, "height"_ostr)
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:pPr/w:pPrChange/w:pPr/w:pStyle[@w:val='UnknownStyle']"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148026)
{
    loadAndReload("tdf148026.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '//w:hyperlink' number of nodes is incorrect
    // i.e. a HYPERLINK field was exported instead of the hyperlink XML element.
    assertXPath(pXmlDoc, "//w:hyperlink"_ostr, "tgtFrame"_ostr, u"_self"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153664)
{
    loadAndReload("Table-of-Figures.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlStyles);
    // Without the fix this was styleId='FigureIndex1' and name was "Figure Index 1"
    // This led to style settings being reset when ToF was updated in Word
    // TOF's paragraph style should be exported as "table of figures" as that's the default Word style name
    assertXPath(pXmlStyles, "/w:styles/w:style[12]"_ostr, "styleId"_ostr, u"TableofFigures"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableofFigures']/w:name"_ostr, "val"_ostr, u"table of figures"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf124472_hyperlink, "tdf124472.docx")
{
    CPPUNIT_ASSERT_EQUAL(u"https://www.libreoffice.org/"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"mailto:info@libreoffice.org"_ustr,
                         getProperty<OUString>(getRun(getParagraph(2), 1), u"HyperLinkURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(u""_ustr,
                         getProperty<OUString>(getRun(getParagraph(3), 1), u"HyperLinkURL"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf135786, "tdf135786.docx")
{
    // Empty first line remain, if the section's initial dummy paragraph is not deleted:
    // - Expected: 2
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

DECLARE_OOXMLEXPORT_TEST(testTdf155736, "tdf155736_PageNumbers_footer.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/footer"_ostr);
    assertXPath(pXmlDoc, "/root/page[2]/footer"_ostr);
    //Without the fix in place, it would have failed with
    //- Expected: Page * of *
    //- Actual  : Page of
    CPPUNIT_ASSERT_EQUAL(u"Page * of *"_ustr, parseDump("/root/page[1]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Page * of *"_ustr, parseDump("/root/page[2]/footer/txt/text()"_ostr));
}

// The following zOrder tests are checking the shapes "stacking height".
// getShape(nZOrder) already gets them in lowest-to-highest order,
// so for any overlapping shapes the highest numbered shape is expected to be the one on top.
// Which shape that actually is can be usually be verified based on the shape name.
DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_duplicate, "tdf159158_zOrder_duplicate_compat15.docx")
{
    // given a yellow star with relativeHeight 2, followed by an overlapping blue star also with 2
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // the zOrder of the stars should remain consistent between round-trips - last duplicate wins
    // and compatibility has nothing to do with this for relativeHeight (other tests are compat12).
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_1and0equalA, "tdf159158_zOrder_1and0equalA.docx")
{
    // given a yellow star with relativeHeight 1, followed by an overlapping blue star with 0
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // 0 is treated the same as 1 - the maximum value, so blue is duplicate - last duplicate wins
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_1and0equalB, "tdf159158_zOrder_1and0equalB.docx")
{
    // given a yellow star with relativeHeight 0, followed by an overlapping blue star with 1
    // since they have the same zOrder value, last one wins, so same result as version A
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // 1 is treated the same as 0 - the maximum value, so blue is duplicate - last duplicate wins
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_1and0max, "tdf159158_zOrder_1and0max.docx")
{
    // given a yellow star with maximum relativeHeight 503316479 (1DFF FFFF)
    // followed by an overlapping blue star with 0.
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // 0 is treated the same as the maximum value, last duplicate wins, so blue is on top
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_maxLessOne, "tdf159158_zOrder_maxLessOne.docx")
{
    // given a yellow star with relativeHeight 503316479 (1DFF FFFF)
    // followed by a partially hidden blue star with lower relativeHeight 503316478 (1DFF FFFE)
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // since yellow is a higher value, it should be on top
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder0,u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder1, u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_max, "tdf159158_zOrder_max.docx")
{
    // given a yellow star with (one higher than maximum) relativeHeight 503316480 (1E00 0000)
    // followed by an overlapping blue star with maximum relativeHeight  503316479 (1DFF FFFF)
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // while yellow is a higher value, last duplicate wins, so lower value blue must be the maximum
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_zIndexMax, "tdf159158_zOrder_zIndexMax.docx")
{
    // given a yellow star with a heaven z-index of MAX_SAL_INT32
    // followed by overlapped blue star with a heaven z-index of MAX_SAL_INT32 - 1
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // there is no artificial maximum for z-index. All values are unique. Yellow is on top
    if (!isExported()) //somehow the name is lost on this export
        CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    if (!isExported()) //somehow the name is lost on this export
        CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_zIndexDuplicate_compat15, "tdf159158_zOrder_zIndexDuplicate_compat15.docx")
{
    // given a yellow star with a heaven z-index of MAX_SAL_INT32 - 1
    // followed by overlapping blue star with the same heaven z-index (last duplicate wins)
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // should be the same as relativeHeight - last duplicate wins so blue is on top.
    if (!isExported()) //somehow the name is lost on this export
        CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    if (!isExported()) //somehow the name is lost on this export
        CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_zIndexWins, "tdf159158_zOrder_zIndexWins.docx")
{
    // given a yellow star with relativeHeight 0 (typically a maximum value, but not today)
    // followed by an overlapping-everything textbox at z-index 0 (the lowest heaven-layer z-index)
    // followed by a partially overlapping blue star with a
    // seems-to-be-a-magic-number relativeHeight 251658240 (0F00 0000)
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder2(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(zOrder2, u"ZOrder"_ustr)); // higher
    // I'm puzzled. Somehow 0 is larger than 0EFF FFFF, but not larger than 0F00 0000
    // and yet the maximum value was established earlier as 1DFF FFFF. Something doesn't line up.
    // Perhaps 0 and 1 don't mean maximum value at all, but something completely different?
    CPPUNIT_ASSERT_MESSAGE("DID YOU FIX ME? I really should be yellow, not blue",
                            "5-Point Star Yellow" != getProperty<OUString>(zOrder0, u"Name"_ustr));
    // CPPUNIT_ASSERT_EQUAL(OUString("5-Point Star Blue"), getProperty<OUString>(zOrder1,"Name"));
    // If zOrder is defined by z-index, it seems that it goes above everything set by relativeHeight
    if (isExported()) // not named on import
        CPPUNIT_ASSERT_EQUAL(u"Frame1"_ustr, getProperty<OUString>(zOrder2,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_behindDocA, "tdf159158_zOrder_behindDocA.docx")
{
    // given a yellow star with lowest relativeHeight 2 but behindDoc
    // followed by an overlapping blue star with negative z-index -1644167168.
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // yellow is at the lowest hell-level possible for relativeHeight, so expected to be under blue
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    if (!isExported()) // the name is lost on export
        CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_behindDocB, "tdf159158_zOrder_behindDocB.docx")
{
    // given a yellow star with a high relativeHeight 503314431 (1DFF F7FF) but behindDoc
    // followed by an overlapping blue star with negative z-index -1644167168.
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // yellow is at the highest hell-level possible for relativeHeight,
    // so you will be forgiven for thinking yellow should be on the top.
    // Any z-index level ends up being above any relativeHeight, so blue should still be on top
    CPPUNIT_ASSERT_EQUAL(u"5-Point Star Yellow"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    if (!isExported()) // the name is lost on export
        CPPUNIT_ASSERT_EQUAL(u"5-Point Star Blue"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_headerBehind, "tdf159158_zOrder_headerBehind.odt")
{
    // given a blue star (not marked as behind text) anchored in the header
    // and an overlapping yellow rectangle anchored in the body text.
    // (note that in ODT format the star is on top, but for DOCX format it must be behind (hidden)
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, u"ZOrder"_ustr)); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, u"ZOrder"_ustr)); // higher
    // I don't know why the star is the lowest order in ODT import (maybe header weirdness),
    // but it certainly needs to be the lowest on docx round-trip (also for header weirdness)
    CPPUNIT_ASSERT_EQUAL(u"StarInHeader"_ustr, getProperty<OUString>(zOrder0, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"RectangleInBody"_ustr, getProperty<OUString>(zOrder1,u"Name"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf159158_zOrder_headerBehind2, "tdf159158_zOrder_headerBehind2.docx")
{
    // given a logo (marked as behind text) anchored in the header
    // and an overlapping blue rectangle anchored in the body text.
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, "ZOrder")); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, "ZOrder")); // higher
    CPPUNIT_ASSERT_EQUAL(OUString("HeaderImage"),
                         getProperty<OUString>(zOrder0, "LinkDisplayName"));
    CPPUNIT_ASSERT_EQUAL(OUString("BodyBlueRectangle"),
                         getProperty<OUString>(zOrder1, "LinkDisplayName"));
    // The logo should not be opaque since it is in the header.
    CPPUNIT_ASSERT(!getProperty<bool>(zOrder0, u"Opaque"_ustr)); // logo should be invisible
    CPPUNIT_ASSERT(!getProperty<bool>(zOrder1, u"Opaque"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf100037_inlineZOrder, "tdf100037_inlineZOrder.docx")
{
    // given a floating textbox followed by an inline image,
    // an inline image should always be behind a heaven-layer floating object.
    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, "ZOrder")); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, "ZOrder")); // higher
    CPPUNIT_ASSERT_EQUAL(OUString("Image 2"), getProperty<OUString>(zOrder0, "LinkDisplayName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame1"), getProperty<OUString>(zOrder1, "LinkDisplayName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf100037_inlineZOrder2, "tdf100037_inlineZOrder2.docx")
{
    // given a yellow floating textbox-with-image overlapped by a blue textbox-with-image,
    // the inline image should take its zOrder from the textbox it is in.
    if (isExported())
        return; // we don't export images inside of draw textboxes I guess

    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder2(getShape(3), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder3(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, "ZOrder")); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(zOrder2, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(zOrder3, "ZOrder")); // higher
    // yellow textbox (Frame1) is the lowest
    CPPUNIT_ASSERT_EQUAL(OUString("Frame1"), getProperty<OUString>(zOrder0, "LinkDisplayName"));
    //CPPUNIT_ASSERT_EQUAL(OUString("Image1"), getProperty<OUString>(zOrder1, "Name"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame2"), getProperty<OUString>(zOrder2, "LinkDisplayName"));
    // CPPUNIT_ASSERT_EQUAL(OUString("Image2"), getProperty<OUString>(zOrder3, "LinkDisplayName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf100037_inlineZOrder3, "tdf100037_inlineZOrder3.docx")
{
    // given a yellow floating textbox-with-image that overlaps a blue textbox-with-image,
    // the inline image should take its zOrder from the textbox it is in.
    if (isExported())
        return; // we don't export images inside of draw textboxes I guess

    uno::Reference<beans::XPropertySet> zOrder0(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder1(getShape(2), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder2(getShape(3), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> zOrder3(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(zOrder0, "ZOrder")); // lower
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(zOrder1, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(zOrder2, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(zOrder3, "ZOrder")); // higher
    // blue textbox (Frame2) is the lowest
    CPPUNIT_ASSERT_EQUAL(OUString("Frame2"), getProperty<OUString>(zOrder0, "LinkDisplayName"));
    // CPPUNIT_ASSERT_EQUAL(OUString("Image2"), getProperty<OUString>(zOrder1, "LinkDisplayName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame1"), getProperty<OUString>(zOrder2, "LinkDisplayName"));
    // CPPUNIT_ASSERT_EQUAL(OUString("Image1"), getProperty<OUString>(zOrder3, "LinkDisplayName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf155903, "tdf155903.odt")
{
    // Without the accompanying fix in place, this test would have crashed,
    // because the exported file was corrupted.
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
