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
#include <com/sun/star/text/XTextTable.hpp>

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

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf77964)
{
    loadAndReload("tdf77964.doc");
    // both images were loading as AT_PARA instead of AS_CHAR. Image2 visually had text wrapping.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShapeByName(u"Image2"), "AnchorType"));
}

DECLARE_WW8EXPORT_TEST(testTdf160049_anchorMargin, "tdf160049_anchorMargin.doc")
{
    // given a document with a LEFT "column/text" anchored image

    // The image takes into account the margin, so it looks like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
}

DECLARE_WW8EXPORT_TEST(testTdf150197_anlv2ListFormat, "tdf150197_anlv2ListFormat.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix me? I should be 2.1", OUString("4.1"),
                                 getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_WW8EXPORT_TEST(testTdf117994_CRnumformatting, "tdf117994_CRnumformatting.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr));
    //Without this fix in place, it would become 200 (and non-bold).
    CPPUNIT_ASSERT_EQUAL(OUString("160"), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']/SwFont"_ostr, "height"_ostr));
}

DECLARE_WW8EXPORT_TEST(testTdf151548_formFieldMacros, "tdf151548_formFieldMacros.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
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
    getParagraph(1, "trueResult");
}

DECLARE_WW8EXPORT_TEST(testTdf90408, "tdf90408.doc")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("checkbox is 16pt", 16.f, getProperty<float>(xRun, "CharHeight"));
    xRun.set(getRun(getParagraph(1), 2, "unchecked"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("text is 12pt", 12.f, getProperty<float>(xRun, "CharHeight"));
}

DECLARE_WW8EXPORT_TEST(testTdf90408B, "tdf90408B.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xRun(getRun(xPara, 1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("checkbox is 28pt", 28.f, getProperty<float>(xRun, "CharHeight"));
    xRun.set(getRun(xPara, 2, u" Κατάψυξη,  "_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("text is 10pt", 10.f, getProperty<float>(xRun, "CharHeight"));
}

DECLARE_WW8EXPORT_TEST(testTdf155465_paraAdjustDistribute, "tdf155465_paraAdjustDistribute.doc")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was left, not block.
    const style::ParagraphAdjust eBlock = style::ParagraphAdjust_BLOCK;
    auto nAdjust = getProperty<sal_Int16>(getParagraph(1), "ParaAdjust");
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(1), "ParaLastLineAdjust");
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(2), "ParaAdjust");
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(2), "ParaLastLineAdjust");
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
    saveAndReload("MS Word 97");

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
        pWrtShell->Insert2("before table");
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
    saveAndReload("MS Word 97");

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
    CPPUNIT_ASSERT_EQUAL(OUString("First line"), aTextNodes[0]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[0]));
    // Here exists an inline pagebreak (a pagebreak without a paragraph before it)
    // This text node is not indented because it is not the first line of the paragraph
    CPPUNIT_ASSERT_EQUAL(OUString("Should not be indented"), aTextNodes[1]->GetText());
    CPPUNIT_ASSERT(!IsFirstLine(aTextNodes[1]));
    // Here is the actual second paragraph
    CPPUNIT_ASSERT_EQUAL(OUString("Should be indented"), aTextNodes[2]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[2]));
}

CPPUNIT_TEST_FIXTURE(Test, testLegalNumbering)
{
    auto verify = [this]() {
        // Second level's numbering should use Arabic numbers for first level reference
        auto xPara = getParagraph(1);
        CPPUNIT_ASSERT_EQUAL(OUString("CH I"), getProperty<OUString>(xPara, "ListLabelString"));
        xPara = getParagraph(2);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Sect 1.01
        // - Actual  : Sect I.01
        // i.e. fLegal was ignored on import/export.
        CPPUNIT_ASSERT_EQUAL(OUString("Sect 1.01"), getProperty<OUString>(xPara, "ListLabelString"));
        xPara = getParagraph(3);
        CPPUNIT_ASSERT_EQUAL(OUString("CH II"), getProperty<OUString>(xPara, "ListLabelString"));
        xPara = getParagraph(4);
        CPPUNIT_ASSERT_EQUAL(OUString("Sect 2.01"), getProperty<OUString>(xPara, "ListLabelString"));
    };

    createSwDoc("listWithLgl.doc");
    verify();
    saveAndReload(mpFilter);
    verify();
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
    CPPUNIT_ASSERT_EQUAL(OUString("First line"), aTextNodes[0]->GetText());
    CPPUNIT_ASSERT(IsFirstLine(aTextNodes[0]));
    // Here exists a pagebreak after a paragraph
    // This text node is indented because it is the first line of a paragraph
    CPPUNIT_ASSERT_EQUAL(OUString("Should be indented"), aTextNodes[1]->GetText());
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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);

    CPPUNIT_ASSERT_EQUAL(u"Empty group"_ustr, pObject->GetName());
    CPPUNIT_ASSERT(pObject->IsGroupObject());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pObject->GetSubList()->GetObjCount());

    // it must not assert/crash on save
    saveAndReload(mpFilter);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
