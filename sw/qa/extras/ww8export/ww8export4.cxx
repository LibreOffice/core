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
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <svx/svdpage.hxx>

#include <docsh.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <editeng/lrspitem.hxx>
#include <wrtsh.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <fmtftntx.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ww8export/data/"_ustr, u"MS Word 97"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf77964)
{
    loadAndReload("tdf77964.doc");
    // both images were loading as AT_PARA instead of AS_CHAR. Image2 visually had text wrapping.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShapeByName(u"Image2"), u"AnchorType"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf72511_editengLRSpace, "tdf72511_editengLRSpace.doc")
{
    // given a default paragraph style with a left indent of 2 inches,
    // the comment should ignore the indent, but the textbox must not.
    uno::Reference<beans::XPropertySet> xRun(
        getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 3), u"TextField"_ustr));
    uno::Reference<text::XText> xComment(getProperty<uno::Reference<text::XText>>(xRun, u"TextRange"_ustr));
    uno::Reference<beans::XPropertySet> xParagraph(getParagraphOfText(1, xComment), uno::UNO_QUERY);
    // The comment was indented by 4001 (2 inches) instead of nothing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));

    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xGroupShape->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.TextShape"_ustr, xShape2->getShapeType());
    uno::Reference<text::XTextRange> xTextbox(xShape2, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xTBPara(xTextbox, uno::UNO_QUERY);
    // Textbox paragraphs had no indent instead of 5080 (2 inches - the same as normal paragraphs).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5080), getProperty<sal_Int32>(xTBPara, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("sanity check: normal paragraph's indent", sal_Int32(5080),
                                 getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf160049_anchorMargin, "tdf160049_anchorMargin.doc")
{
    // given a document with a LEFT "column/text" anchored image

    // The image takes into account the margin, so it looks like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf150197_anlv2ListFormat, "tdf150197_anlv2ListFormat.doc")
{
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix me? I should be 2.1", u"4.1"_ustr,
                                 getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf117994_CRnumformatting, "tdf117994_CRnumformatting.doc")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "expand", u"1.");
    //Without this fix in place, it would become 200 (and non-bold).
    assertXPath(pXmlDoc, "//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']/SwFont", "height", u"160");
}

DECLARE_WW8EXPORT_TEST(testTdf151548_formFieldMacros, "tdf151548_formFieldMacros.doc")
{
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    for(auto aIter = pMarkAccess->getFieldmarksBegin(); aIter != pMarkAccess->getFieldmarksEnd(); ++aIter)
    {
        const OUString sName = (*aIter)->GetName();
        CPPUNIT_ASSERT(sName == "Check1" || sName == "Check2" || sName == "Text1" || sName == "Dropdown1");
    }
}

DECLARE_WW8EXPORT_TEST(testTdf141649_conditionalText, "tdf141649_conditionalText.doc")
{
    // In MS Word, the IF field is editable and requires manual update, so the most correct
    // result is "manual refresh with F9" inside a text field,
    // but for our purposes, a single instance of "trueResult" is appropriate.
    getParagraph(1, u"trueResult"_ustr);
}

DECLARE_WW8EXPORT_TEST(testTdf91632_layoutInCellD, "tdf91632_layoutInCellD.doc")
{
    // given a table with two layoutInCell images, and cell A1 has 1/2 inch border padding (margin)
    // - A1 contains an image, vertically aligned to the outside of the page (aka cell)
    // - B1 contains an image, vertically aligned from top of the page (aka cell)

    // In Microsoft's layoutInCell implementation, vertical "page" is identical to "margin",
    // and everything (including center/bottom) actually is oriented to the top of the margin.

    xmlDocUniquePtr pDump = parseLayoutDump();
    // Cell A1
    sal_Int32 nShapeTop
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/anchored/fly/SwAnchoredObject/bounds",
                   "top")
              .toInt32();
    sal_Int32 nShapeBottom
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/anchored/fly/SwAnchoredObject/bounds",
                   "bottom")
              .toInt32();
    // use paragraph 1 to indicate where the cell spacing/padding ends, and the text starts.
    sal_Int32 nPara1Top
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/infos/bounds", "top").toInt32();
    // use paragraph 5 to prove the image is not at the bottom.
    assertXPathContent(pDump, "//tab/row[1]/cell[1]/txt[5]", u"Below logo");
    sal_Int32 nPara5Top
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[5]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(nShapeTop, nPara1Top);
    CPPUNIT_ASSERT(nPara5Top > nShapeBottom); // ShapeBottom is higher than Para5Top

    // In the file it is specified as "page" (PAGE_FRAME), but implemented as if it were "margin"
    // so on import we intentionally changed it to match the closest setting to the implementation.
    const auto xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"IsFollowingTextFlow"_ustr));

    // Cell B1
    nShapeTop
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/anchored/fly/SwAnchoredObject/bounds",
                   "top")
              .toInt32();
    nShapeBottom
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/anchored/fly/SwAnchoredObject/bounds",
                   "bottom")
              .toInt32();
    // use paragraph 1 to indicate where the cell spacing/padding ends, and the text starts.
    nPara1Top
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    // use paragraph 5 to prove the image is not at the bottom.
    assertXPathContent(pDump, "//tab/row[1]/cell[2]/txt[5]", u"Below image");
    nPara5Top
        = getXPath(pDump, "//tab[1]/row/cell[2]/txt[5]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(nShapeTop, nPara1Top);
    CPPUNIT_ASSERT(nPara5Top > nShapeBottom); // ShapeBottom is higher than Para5Top

    const auto xShape2 = getShape(2);
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape2, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape2, u"IsFollowingTextFlow"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf162541, "tdf162541_notLayoutInCell_paraLeft.doc")
{
    // Note: this file looks very strange in MS Word. The image splits the table into two...

    // given cell B2 with a para-left para-fromTop image that is NOT layoutInCell
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nShapeLeft
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt/anchored/fly/SwAnchoredObject/bounds",
                   "left")
              .toInt32();
    sal_Int32 nParaLeft
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt/infos/bounds", "left").toInt32();
    sal_Int32 nTableLeft
        = getXPath(pDump, "//tab/infos/bounds", "left").toInt32();
    // The image uses the table-paragraph to orient to the left (bizarre MSO layout anomaly)
    CPPUNIT_ASSERT(nShapeLeft < nParaLeft); // shape is located in column A, not column B
    CPPUNIT_ASSERT_EQUAL(nTableLeft, nShapeLeft);
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf162542, "tdf162542_notLayoutInCell_charLeft_wrapThrough.doc")
{
    // given cell B2 with a char-oriented-left wrapThrough image that is NOT layoutInCell
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nShapeLeft
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt[6]/anchored/fly/SwAnchoredObject/bounds",
                   "left")
              .toInt32();
    sal_Int32 nPara6Left
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt[6]/infos/bounds", "left").toInt32();
    CPPUNIT_ASSERT(nShapeLeft > nPara6Left); // nShapeLeft starts after the word "anchor"

    // tdf#162551: The top is oriented to the top of the page - but MSO treats it as layoutInCell
    sal_Int32 nShapeTop
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt[6]/anchored/fly/SwAnchoredObject/bounds",
                   "top")
              .toInt32();
    sal_Int32 nPara1Top
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    // layoutInCell uses the cell margin as the top-most point, not the cell edge
    CPPUNIT_ASSERT_EQUAL(nPara1Top, nShapeTop); // nShapeTop starts at the cell margin"

    // since in fact layoutInCell is supposed to be applied, we mark (and export) as layoutInCell
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr)); // tdf#162551
}

CPPUNIT_TEST_FIXTURE(Test, testEndnotesAtSectEndDOC)
{
    // Given a document, endnotes at collected at section end:
    createSwDoc();
    {
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->SplitNode();
        pWrtShell->Up(/*bSelect=*/false);
        pWrtShell->Insert(u"x"_ustr);
        pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
        SwSectionData aSection(SectionType::Content, pWrtShell->GetUniqueSectionName());
        pWrtShell->StartAction();
        SfxItemSetFixed<RES_FTN_AT_TXTEND, RES_FRAMEDIR> aSet(pWrtShell->GetAttrPool());
        aSet.Put(SwFormatEndAtTextEnd(FTNEND_ATTXTEND));
        pWrtShell->InsertSection(aSection, &aSet);
        pWrtShell->EndAction();
        pWrtShell->InsertFootnote(OUString(), /*bEndNote=*/true);
    }

    // When saving to DOC:
    saveAndReload(u"MS Word 97"_ustr);

    // Then make sure the endnote position is section end:
    SwDoc* pDoc = getSwDoc();
    SwSectionFormats& rSections = pDoc->GetSections();
    SwSectionFormat* pFormat = rSections[0];
    // Without the accompanying fix in place, this test would have failed, endnotes were at doc end.
    CPPUNIT_ASSERT(pFormat->GetEndAtTextEnd().IsAtEnd());
}

DECLARE_WW8EXPORT_TEST(testTdf90408, "tdf90408.doc")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("checkbox is 16pt", 16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
    xRun.set(getRun(getParagraph(1), 2, u"unchecked"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("text is 12pt", 12.f, getProperty<float>(xRun, u"CharHeight"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf90408B, "tdf90408B.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xRun(getRun(xPara, 1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("checkbox is 28pt", 28.f, getProperty<float>(xRun, u"CharHeight"_ustr));
    xRun.set(getRun(xPara, 2, u" Κατάψυξη,  "_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("text is 10pt", 10.f, getProperty<float>(xRun, u"CharHeight"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf155465_paraAdjustDistribute, "tdf155465_paraAdjustDistribute.doc")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was left, not block.
    const style::ParagraphAdjust eBlock = style::ParagraphAdjust_BLOCK;
    auto nAdjust = getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr);
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(1), u"ParaLastLineAdjust"_ustr);
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(2), u"ParaAdjust"_ustr);
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(2), u"ParaLastLineAdjust"_ustr);
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(nAdjust));
}

CPPUNIT_TEST_FIXTURE(Test, testDontBreakWrappedTables)
{
    // Given a document with the DO_NOT_BREAK_WRAPPED_TABLES compat mode enabled:
    createSwDoc();
    {
        SwDoc* pDoc = getSwDoc();
        IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
        rIDSA.set(DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES, true);
    }

    // When saving to doc:
    saveAndReload(u"MS Word 97"_ustr);

    // Then make sure the compat flag is serialized:
    SwDoc* pDoc = getSwDoc();
    IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
    bool bDontBreakWrappedTables = rIDSA.get(DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES);
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bDontBreakWrappedTables);
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableOverlapNeverDOCExport)
{
    // Given a document with a floating table, overlap is not allowed:
    {
        createSwDoc();
        SwDoc* pDoc = getSwDoc();
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert2(u"before table"_ustr);
        // Insert a table:
        SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
        pWrtShell->InsertTable(aTableOptions, /*nRows=*/1, /*nCols=*/1);
        pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
        // Select table:
        pWrtShell->SelAll();
        // Wrap the table in a text frame:
        SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
        pWrtShell->StartAllAction();
        aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
        pWrtShell->EndAllAction();
        // Allow the text frame to split:
        pWrtShell->StartAllAction();
        sw::FrameFormats<sw::SpzFrameFormat*>* pFlys = pDoc->GetSpzFrameFormats();
        sw::SpzFrameFormat* pFly = (*pFlys)[0];
        SwAttrSet aSet(pFly->GetAttrSet());
        aSet.Put(SwFormatFlySplit(true));
        // Don't allow overlap:
        SwFormatWrapInfluenceOnObjPos aInfluence;
        aInfluence.SetAllowOverlap(false);
        aSet.Put(aInfluence);
        pDoc->SetAttr(aSet, *pFly);
        pWrtShell->EndAllAction();
    }

    // When saving to DOC:
    saveAndReload(u"MS Word 97"_ustr);

    // Then make sure that the overlap=never markup is written:
    SwDoc* pDoc = getSwDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>* pFlys = pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = (*pFlys)[0];
    // Without the accompanying fix in place, this test would have failed, i.e. TFNoAllowOverlap was
    // not written.
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetWrapInfluenceOnObjPos().GetAllowOverlap());
}

bool IsFirstLine(const SwTextNode* pTextNode)
{
    const SfxPoolItem* pItem = pTextNode->GetNoCondAttr(RES_MARGIN_FIRSTLINE, false);
    return !!pItem;
}

DECLARE_WW8EXPORT_TEST(testInlinePageBreakFirstLine, "inlinePageBreakFirstLine.doc")
{
    SwDoc* pDoc = getSwDoc();
    const SwNodes& rNodes = pDoc->GetNodes();

    std::vector<SwTextNode*> aTextNodes;

    for (SwNodeOffset nNode(0); nNode < rNodes.Count(); ++nNode)
    {
        SwNode* pNode = pDoc->GetNodes()[nNode];
        SwTextNode* pTextNode = pNode->GetTextNode();
        if (!pTextNode)
            continue;
        aTextNodes.push_back(pTextNode);
    }

    CPPUNIT_ASSERT_EQUAL(size_t(3), aTextNodes.size());
    CPPUNIT_ASSERT_EQUAL(u"First line"_ustr, aTextNodes[0]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[0]));
    // Here exists an inline pagebreak (a pagebreak without a paragraph before it)
    // This text node is not indented because it is not the first line of the paragraph
    CPPUNIT_ASSERT_EQUAL(u"Should not be indented"_ustr, aTextNodes[1]->GetText());
    CPPUNIT_ASSERT(!IsFirstLine(aTextNodes[1]));
    // Here is the actual second paragraph
    CPPUNIT_ASSERT_EQUAL(u"Should be indented"_ustr, aTextNodes[2]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[2]));
}

CPPUNIT_TEST_FIXTURE(Test, testLegalNumbering)
{
    auto verify = [this]() {
        // Second level's numbering should use Arabic numbers for first level reference
        auto xPara = getParagraph(1);
        CPPUNIT_ASSERT_EQUAL(u"CH I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        xPara = getParagraph(2);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Sect 1.01
        // - Actual  : Sect I.01
        // i.e. fLegal was ignored on import/export.
        CPPUNIT_ASSERT_EQUAL(u"Sect 1.01"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        xPara = getParagraph(3);
        CPPUNIT_ASSERT_EQUAL(u"CH II"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        xPara = getParagraph(4);
        CPPUNIT_ASSERT_EQUAL(u"Sect 2.01"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    };

    createSwDoc("listWithLgl.doc");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDOCExportDoNotMirrorRtlDrawObjs)
{
    // Given a document with a shape, anchored in an RTL paragraph, loaded from DOCX:
    createSwDoc("draw-obj-rtl-no-mirror-vml.docx");

    // When saving that to DOC:
    saveAndReload(mpFilter);

    // Then make sure the shape is on the right margin:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPageRight = getXPath(pXmlDoc, "//page/infos/bounds", "right").toInt32();
    sal_Int32 nBodyRight = getXPath(pXmlDoc, "//body/infos/bounds", "right").toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "left").toInt32();
    CPPUNIT_ASSERT_GREATER(nBodyRight, nShapeLeft);
    sal_Int32 nShapeRight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "right").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 12523
    // - Actual  : 12536
    // i.e. the shape was outside of the page right margin area, due to an unwanted mapping.
    CPPUNIT_ASSERT_LESS(nPageRight, nShapeRight);
}

DECLARE_WW8EXPORT_TEST(testNonInlinePageBreakFirstLine, "nonInlinePageBreakFirstLine.doc")
{
    SwDoc* pDoc = getSwDoc();
    const SwNodes& rNodes = pDoc->GetNodes();

    std::vector<SwTextNode*> aTextNodes;

    for (SwNodeOffset nNode(0); nNode < rNodes.Count(); ++nNode)
    {
        SwNode* pNode = pDoc->GetNodes()[nNode];
        SwTextNode* pTextNode = pNode->GetTextNode();
        if (!pTextNode)
            continue;
        aTextNodes.push_back(pTextNode);
    }

    CPPUNIT_ASSERT_EQUAL(size_t(2), aTextNodes.size());
    CPPUNIT_ASSERT_EQUAL(u"First line"_ustr, aTextNodes[0]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[0]));
    // Here exists a pagebreak after a paragraph
    // This text node is indented because it is the first line of a paragraph
    CPPUNIT_ASSERT_EQUAL(u"Should be indented"_ustr, aTextNodes[1]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[1]));
}

DECLARE_WW8EXPORT_TEST(testTdf104704_mangledFooter, "tdf104704_mangledFooter.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testEmptyGroup)
{
    // Given a document with an empty group
    createSwDoc("empty_group.docx");

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    SwDoc* pDoc = getSwDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);

    CPPUNIT_ASSERT_EQUAL(u"Empty group"_ustr, pObject->GetName());
    CPPUNIT_ASSERT(pObject->IsGroupObject());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pObject->GetSubList()->GetObjCount());

    // it must not assert/crash on save
    saveAndReload(mpFilter);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135709)
{
    createSwDoc("tdf135709.odt");
    saveAndReload("MS Word 97");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xTextFramesSupplier->getTextFrames()->getByName("Frame1") , uno::UNO_QUERY);

    xPropertySet->setPropertyValue("AnchorType",
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));

    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrap should be PARALLEL", text::WrapTextMode_PARALLEL, eValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135710)
{
    // Uses same test doc as testTdf135709
    createSwDoc("tdf135709.odt");
    saveAndReload("MS Word 97");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nFlyLeft = getXPath(pXmlDoc, "(//anchored)[1]/fly/infos/bounds", "left").toInt32();

    // Set the anchor of the image to AT PARAGRAPH, without the fix in place this
    // results in the picture moving to the first column
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xTextFramesSupplier->getTextFrames()->getByName("Frame1") , uno::UNO_QUERY);
    xPropertySet->setPropertyValue("AnchorType",
                                       uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    pXmlDoc = parseLayoutDump();

    sal_Int32 nFlyLeftAfter = getXPath(pXmlDoc, "(//anchored)[1]/fly/infos/bounds", "left").toInt32();

    // Without the fix in place this fails with
    // Expected: 4771
    // Actual:   1418
    // i.e. the picture has moved from the second column to the first column
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<double>(nFlyLeft), static_cast<double>(nFlyLeftAfter), 2.0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf56738)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

        // make sure we get to the correct field to test
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Reference<text::XTextField> xField;
        xField.set(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xField->getPresentation(false));

        CPPUNIT_ASSERT(xFields->hasMoreElements());
        xField.set(xFields->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xField->getPresentation(false));

        CPPUNIT_ASSERT(xFields->hasMoreElements());
        xField.set(xFields->nextElement(), uno::UNO_QUERY_THROW);
        OUString sExpComment1(u"Como eu vou saber se é a USG é anterior a 20s????"_ustr);
        CPPUNIT_ASSERT_EQUAL(sExpComment1, getProperty<OUString>(xField, u"Content"_ustr));

        CPPUNIT_ASSERT(xFields->hasMoreElements());
        xField.set(xFields->nextElement(), uno::UNO_QUERY_THROW);

        OUString sExpComment(u"Não sei se é relevante esta pergunta. O que eu queria saber é se o médico ate\
nde muito parto na água. Tb posso fazer porcentagem de atendimento..."_ustr);

        // Without the fix in place this fails with
        // Expected: Não sei se é relevante esta pergunta. O que eu queria saber é
        //           se o médico atende muito parto na água. Tb posso fazer porcentagem de atendimento...
        // Actual:   N縊 sei se �relevante esta pergunta. O que eu queria saber �
        //           se o m馘ico atende muito parto na 疊ua. Tb posso fazer porcentagem de atendimento...
        // i.e. the display characters of the second comment were getting re-interpreted from Latin-1 to Shift-Js
        CPPUNIT_ASSERT_EQUAL(sExpComment, getProperty<OUString>(xField, u"Content"_ustr));
    };

    // make sure everything survives roundtrip
    createSwDoc("tdf56738.doc");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf120629)
{
    createSwDoc("tdf120629.odt");

    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(56), numFormat);

    saveAndReload("MS Word 97");
    sal_Int16 numFormat_after = getNumberingTypeOfParagraph(1);
    // Without the fix in place this fails with
    // Expected: 56
    // Actual:   4
    // i.e. numbering type gets changed to ARABIC, should stay the same
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(56), numFormat_after);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf54862)
{
    createSwDoc("tdf54862.doc");
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();

        sal_Int32 nCellA2Height = getXPath(pXmlDoc, "//tab/row[1]/cell[2]/infos/bounds", "height").toInt32();
        sal_Int32 nCellB4Height = getXPath(pXmlDoc, "//tab/row[4]/cell[2]/infos/bounds", "height").toInt32();

        // Without the fix in place this is this fails with:
        // Expected: 1269, 9021
        // Actual:   562, 623
        // i.e. Cells A2 and B4 are not vertically merged, making them the wrong height
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1269), nCellA2Height);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9021), nCellB4Height);
    };

    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf83844)
{
    createSwDoc("tdf83844.fodt");

    auto fnVerify = [this]
    {
        auto pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                    u"A A A A ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                    u"B B B B B B B B ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]", "portion",
                    u"C C C C C C C C ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[4]", "portion",
                    u"D D D D");
    };

    fnVerify();
    saveAndReload(mpFilter);
    fnVerify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
