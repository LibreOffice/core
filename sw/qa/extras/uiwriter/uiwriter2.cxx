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
#include <comphelper/propertysequence.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <vcl/scheduler.hxx>
#include <dcontact.hxx>
#include <drawdoc.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <redline.hxx>
#include <UndoManager.hxx>
#include <itabenum.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>

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
    void testTdf54819();
    void testTdf109376();
    void testTdf108687_tabstop();
    void testTdf119571();
    void testTdf119019();
    void testTdf119824();
    void testTdf105413();
    void testTdf101873();
    void testTableWidth();
    void testTdf122942();
    void testDocxAttributeTableExport();

    CPPUNIT_TEST_SUITE(SwUiWriterTest2);
    CPPUNIT_TEST(testRedlineMoveInsertInDelete);
    CPPUNIT_TEST(testRedlineInHiddenSection);
    CPPUNIT_TEST(testTdf101534);
    CPPUNIT_TEST(testTdf54819);
    CPPUNIT_TEST(testTdf109376);
    CPPUNIT_TEST(testTdf108687_tabstop);
    CPPUNIT_TEST(testTdf119571);
    CPPUNIT_TEST(testTdf119019);
    CPPUNIT_TEST(testTdf119824);
    CPPUNIT_TEST(testTdf105413);
    CPPUNIT_TEST(testTdf101873);
    CPPUNIT_TEST(testTableWidth);
    CPPUNIT_TEST(testTdf122942);
    CPPUNIT_TEST(testDocxAttributeTableExport);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = nullptr);
};

static void lcl_dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                                const OUString& rCommand,
                                const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY)->getCurrentController();
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

void SwUiWriterTest2::testTdf54819()
{
    load(DATA_DIRECTORY, "tdf54819.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    //turn on red-lining and hide changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove first paragraph with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // remaining paragraph keeps its original style
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
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

void SwUiWriterTest2::testTdf108687_tabstop()
{
    SwDoc* pDoc = createDoc("tdf108687_tabstop.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sal_Int32 nStartIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), nStartIndex);

    // Now pressing 'tab' should jump to the radio buttons.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    //sal_Int32 nEndIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(11), nEndIndex);
}

void SwUiWriterTest2::testTdf119571()
{
    load(DATA_DIRECTORY, "tdf54819.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    //turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // join paragraphs by removing the end of the first one with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // second paragraph changes its style in "Show changes" mode
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

void SwUiWriterTest2::testTdf119019()
{
    // check handling of overlapping redlines
    load(DATA_DIRECTORY, "tdf119019.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus."),
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(2), 1)->getString());
    // second paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 1), "RedlineType"));

    // delete last word of the second paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // check tracked text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("tellus."), getRun(getParagraph(2), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(2), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), "RedlineType"));

    // make sure that the tracked paragraph formatting is removed
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
}

void SwUiWriterTest2::testTdf119824()
{
    // check handling of overlapping redlines with Redo
    SwDoc* pDoc = createDoc("tdf119019.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "
                                  "et netus et malesuada fames ac turpis egestas. "
                                  "Proin pharetra nonummy pede. Mauris et orci."),
                         getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    // third paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // and a tracked text deletion at the beginning of the paragraph
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 2), "RedlineType"));

    // delete last word of the third paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 5, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // check tracking of the new text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("orci."), getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), "RedlineType"));

    // make sure that the tracked paragraph formatting is removed (tracked deletion is in the second run)
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // tdf#119824 check redo
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Redo();

    // check again the first tracked text deletion (we lost this before the redo fix)
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // check redo of the new tracked text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("orci."), getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), "RedlineType"));
}

void SwUiWriterTest2::testTdf105413()
{
    load(DATA_DIRECTORY, "tdf105413.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // all paragraphs have got Standard paragraph style
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                             getProperty<OUString>(getParagraph(i), "ParaStyleName"));
    }

    // turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Set Heading 1 paragraph style in the 3th paragraph.
    // Because of the tracked deleted region between them,
    // this sets also the same style in the first paragraph automatically
    // to keep the changed paragraph style at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 1")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    lcl_dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    // first paragraph gets the same heading style
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

void SwUiWriterTest2::testTdf101873()
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    SwDocShell* pDocShell = pDoc->GetDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Insert some content.
    pWrtShell->Insert("something");

    // Search for something which does not exist, twice.
    uno::Sequence<beans::PropertyValue> aFirst(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("fig")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);

    uno::Sequence<beans::PropertyValue> aSecond(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("something")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSecond);

    // Without the accompanying fix in place, this test would have failed with "Expected: something;
    // Actual:", i.e. searching for "something" failed, even if it was inserted above.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("something"), pShellCursor->GetText());
}

void SwUiWriterTest2::testTableWidth()
{
    load(DATA_DIRECTORY, "frame_size_export.docx");

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // after exporting: table width was overwritten in the doc model
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<sal_Int16>(xTables->getByIndex(0), "RelativeWidth"));
}

void SwUiWriterTest2::testTdf122942()
{
    load(DATA_DIRECTORY, "tdf122942.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    // Do the moral equivalent of mouse button down, move and up.
    // Start creating a custom shape that overlaps with the rounded rectangle
    // already present in the document.
    Point aStartPos(8000, 3000);
    pWrtShell->BeginCreate(static_cast<sal_uInt16>(OBJ_CUSTOMSHAPE), aStartPos);

    // Set its size.
    Point aMovePos(10000, 5000);
    pWrtShell->MoveCreate(aMovePos);

    // Finish creation.
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Make sure that the shape is inserted.
    SwDoc* pDoc = pWrtShell->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());

    // Without the accompanying fix in place, this test would have failed with
    // 'Expected less than: 0; Actual  : 1030', i.e. the shape was below the
    // paragraph mark, not above it.
    const SwFormatVertOrient& rVert = rFormats[1]->GetVertOrient();
    CPPUNIT_ASSERT_LESS(static_cast<SwTwips>(0), rVert.GetPos());
}

void SwUiWriterTest2::testDocxAttributeTableExport()
{
    createDoc("floating-table-position.docx");

    // get the table frame, set new values and dismiss the references
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                          uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

        // change the properties
        // 8133 -> 8000
        xShape->setPropertyValue("VertOrientPosition", uno::makeAny(static_cast<sal_Int32>(8000)));
        // 5964 -> 5000
        xShape->setPropertyValue("HoriOrientPosition", uno::makeAny(static_cast<sal_Int32>(5000)));
        // 0 (frame) -> 8 (page print area)
        xShape->setPropertyValue("VertOrientRelation", uno::makeAny(static_cast<sal_Int16>(8)));
        // 8 (page print area) -> 0 (frame)
        xShape->setPropertyValue("HoriOrientRelation", uno::makeAny(static_cast<sal_Int16>(0)));
    }
    // save it to docx
    reload("Office Open XML Text", "floating-table-position.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                      uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // test the new values
    sal_Int32 nValue = getProperty<sal_Int32>(xShape, "VertOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(7999) <= nValue && nValue <= sal_Int32(8001));
    nValue = getProperty<sal_Int32>(xShape, "HoriOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(4999) <= nValue && nValue <= sal_Int32(5001));

    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest2);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
