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

#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>

#include <docsh.hxx>
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
#include <frameformats.hxx>

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

DECLARE_WW8EXPORT_TEST(testTdf150197_anlv2ListFormat, "tdf150197_anlv2ListFormat.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix me? I should be 2.1", OUString("4.1"),
                                 getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_WW8EXPORT_TEST(testTdf117994_CRnumformatting, "tdf117994_CRnumformatting.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "expand"));
    //Without this fix in place, it would become 200 (and non-bold).
    CPPUNIT_ASSERT_EQUAL(OUString("160"), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "font-height"));
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
    reload(mpFilter, "dont-break-wrapped-tables.doc");

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
        SwFrameFormats* pFlys = pDoc->GetSpzFrameFormats();
        SwFrameFormat* pFly = (*pFlys)[0];
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
    SwFrameFormats* pFlys = pDoc->GetSpzFrameFormats();
    SwFrameFormat* pFly = (*pFlys)[0];
    // Without the accompanying fix in place, this test would have failed, i.e. TFNoAllowOverlap was
    // not written.
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetWrapInfluenceOnObjPos().GetAllowOverlap());
}

DECLARE_WW8EXPORT_TEST(testTdf104704_mangledFooter, "tdf104704_mangledFooter.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
