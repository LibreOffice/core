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
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <redline.hxx>
#include <ndtxt.hxx>
#include <UndoManager.hxx>
#include <itabenum.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data2/";
}

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest2 : public SwModelTestBase
{
public:
    void testRedlineMoveInsertInDelete();
    void testRedlineInHiddenSection();
    void testTdf101534();
    void testTdf131684();
    void testTdf132236();
    void testTdf109376_redline();
    void testTdf109376();

    CPPUNIT_TEST_SUITE(SwUiWriterTest2);
    CPPUNIT_TEST(testRedlineMoveInsertInDelete);
    CPPUNIT_TEST(testRedlineInHiddenSection);
    CPPUNIT_TEST(testTdf101534);
    CPPUNIT_TEST(testTdf131684);
    CPPUNIT_TEST(testTdf132236);
    CPPUNIT_TEST(testTdf109376_redline);
    CPPUNIT_TEST(testTdf109376);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = nullptr);
};

static void lcl_dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                                const OUString& rCommand,
                                const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
}

SwDoc* SwUiWriterTest2::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

void SwUiWriterTest2::testTdf131684()
{
    load(DATA_DIRECTORY, "tdf131684.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});

    lcl_dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    lcl_dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // without the fix, it crashes
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // check that the text frame has the correct upper
    xmlDocPtr pXmlDoc = parseLayoutDump();
    OUString const sectionId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]", "id");
    OUString const sectionLower = getXPath(pXmlDoc, "/root/page[1]/body/section[7]", "lower");
    OUString const textId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]", "id");
    OUString const textUpper = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]", "upper");
    CPPUNIT_ASSERT_EQUAL(textId, sectionLower);
    CPPUNIT_ASSERT_EQUAL(sectionId, textUpper);
}

void SwUiWriterTest2::testTdf101534()
{
    // Copy the first paragraph of the document.
    load(DATA_DIRECTORY, "tdf101534.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();

    // Go to the second paragraph, assert that we have margins as direct
    // formatting.
    pWrtShell->Down(/*bSelect=*/false);
    SfxItemSet aSet(pWrtShell->GetAttrPool(), svl::Items<RES_LR_SPACE, RES_LR_SPACE>{});
    pWrtShell->GetCurAttr(aSet);
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));

    // Make sure that direct formatting is preserved during paste.
    pWrtShell->EndPara(/*bSelect=*/false);
    TransferableDataHelper aHelper(pTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    aSet.ClearItem();
    pWrtShell->GetCurAttr(aSet);
    // This failed, direct formatting was lost.
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));
}

void SwUiWriterTest2::testRedlineMoveInsertInDelete()
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* const pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert(" foo");
    pWrtShell->SttEndDoc(true);
    pWrtShell->InsertFootnote("");
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    // insert redline
    pWrtShell->Insert("bar");
    // first delete redline, logically containing the insert redline
    // (note: Word apparently allows similar things...)
    pWrtShell->SttEndDoc(true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete(); // the footnote
    // second delete redline, following the first one
    pWrtShell->EndOfSection(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    pWrtShell->Delete(); // "foo"

    // hiding used to copy the 2nd delete redline "foo", but not delete it
    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide
    CPPUNIT_ASSERT_EQUAL(
        OUString(" "),
        pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    pWrtShell->SetRedlineFlags(mode); // show again
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"\u0001 foo"),
        pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwUiWriterTest2::testRedlineInHiddenSection()
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* const pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SplitNode();
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->Insert("baz");

    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);

    // delete paragraph "bar"
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->StartOfSection();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndOfSection(true);

    SwSectionData section(CONTENT_SECTION, pWrtShell->GetUniqueSectionName());
    section.SetHidden(true);
    SwSection const* pSection = pWrtShell->InsertSection(section, nullptr);

    SwSectionNode const* pNode = pSection->GetFormat()->GetSectionNode();

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 2]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 3]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 4]->IsEndNode());

    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 2]->IsEndNode());

    pWrtShell->SetRedlineFlags(mode); // show again

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    // there was a frame created here
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 2]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 3]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 4]->IsEndNode());
}

void SwUiWriterTest2::testTdf132236()
{
    load(DATA_DIRECTORY, "tdf132236.odt");

    SwXTextDocument* const pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // select everything and delete
    SwWrtShell* const pWrtShell(pTextDoc->GetDocShell()->GetWrtShell());
    pWrtShell->Down(true);
    pWrtShell->Down(true);
    pWrtShell->Down(true);
    pWrtShell->Delete();
    SwDoc* const pDoc(pWrtShell->GetDoc());
    sw::UndoManager& rUndoManager(pDoc->GetUndoManager());
    rUndoManager.Undo();

    // check that the text frames exist inside their sections
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
}

void SwUiWriterTest2::testTdf109376_redline()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection(false);

    // add AT_PARA fly at 1st to be deleted node
    SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    flySet.Put(anchor);
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    pWrtShell->SttEndDoc(false);
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->StartOfSection(false);
    SwPaM pam(*pWrtShell->GetCursor()->GetPoint());
    pam.SetMark();
    pam.GetPoint()->nNode = *rTable.GetTableNode();
    pam.GetPoint()->nContent.Assign(nullptr, 0);
    pam.Exchange(); // same selection direction as in doc compare...

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);
    rIDRA.AppendRedline(new SwRangeRedline(nsRedlineType_t::REDLINE_DELETE, pam), true);
    // this used to assert/crash with m_pAnchoredFlys mismatch because the
    // fly was not deleted but its anchor was moved to the SwTableNode
    rIDRA.AcceptAllRedline(true);

    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

void SwUiWriterTest2::testTdf109376()
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection(false);

    // add AT_PARA fly at 1st to be deleted node
    SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    flySet.Put(anchor);
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    pWrtShell->SttEndDoc(false);
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->StartOfSection(false);
    SwPaM pam(*pWrtShell->GetCursor()->GetPoint());
    pam.SetMark();
    pam.GetPoint()->nNode = *rTable.GetTableNode();
    pam.GetPoint()->nContent.Assign(nullptr, 0);
    pam.Exchange(); // same selection direction as in doc compare...

    // this used to assert/crash with m_pAnchoredFlys mismatch because the
    // fly was not deleted but its anchor was moved to the SwTableNode
    pDoc->getIDocumentContentOperations().DeleteRange(pam);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest2);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
