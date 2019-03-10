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
#include <com/sun/star/style/LineSpacing.hpp>
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
#include <flyfrms.hxx>
#include <UndoManager.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <swtypes.hxx>
#include <fmtornt.hxx>
#include <xmloff/odffields.hxx>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data2/";
char const FLOATING_TABLE_DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data/floating_table/";
} // namespace

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest2 : public SwModelTestBase
{
public:
    void testRedlineMoveInsertInDelete();
    void testRedlineInHiddenSection();
    void testTdf47471_paraStyleBackground();
    void testTdf101534();
    void testTdf54819();
    void testTdf64242_optimizeTable();
    void testTdf108687_tabstop();
    void testTdf119571();
    void testTdf119019();
    void testTdf119824();
    void testTdf105413();
    void testUnfloatButtonSmallTable();
    void testUnfloatButton();
    void testUnfloatButtonReadOnlyMode();
    void testUnfloating();
    void testTdf122893();
    void testTdf122901();
    void testTdf122942();
    void testTdf52391();
    void testTdf101873();
    void testTableWidth();
    void testTextFormFieldInsertion();
    void testCheckboxFormFieldInsertion();
    void testDropDownFormFieldInsertion();
    void testMixedFormFieldInsertion();

    CPPUNIT_TEST_SUITE(SwUiWriterTest2);
    CPPUNIT_TEST(testRedlineMoveInsertInDelete);
    CPPUNIT_TEST(testRedlineInHiddenSection);
    CPPUNIT_TEST(testTdf47471_paraStyleBackground);
    CPPUNIT_TEST(testTdf101534);
    CPPUNIT_TEST(testTdf54819);
    CPPUNIT_TEST(testTdf64242_optimizeTable);
    CPPUNIT_TEST(testTdf108687_tabstop);
    CPPUNIT_TEST(testTdf119571);
    CPPUNIT_TEST(testTdf119019);
    CPPUNIT_TEST(testTdf119824);
    CPPUNIT_TEST(testTdf105413);
    CPPUNIT_TEST(testUnfloatButtonSmallTable);
    CPPUNIT_TEST(testUnfloatButton);
    CPPUNIT_TEST(testUnfloatButtonReadOnlyMode);
    CPPUNIT_TEST(testUnfloating);
    CPPUNIT_TEST(testTdf122893);
    CPPUNIT_TEST(testTdf122901);
    CPPUNIT_TEST(testTdf122942);
    CPPUNIT_TEST(testTdf52391);
    CPPUNIT_TEST(testTdf101873);
    CPPUNIT_TEST(testTableWidth);
    CPPUNIT_TEST(testTextFormFieldInsertion);
    CPPUNIT_TEST(testCheckboxFormFieldInsertion);
    CPPUNIT_TEST(testDropDownFormFieldInsertion);
    CPPUNIT_TEST(testMixedFormFieldInsertion);
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

void SwUiWriterTest2::testTdf47471_paraStyleBackground()
{
    SwDoc* pDoc = createDoc("tdf47471_paraStyleBackground.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14729933), getProperty<sal_Int32>(getParagraph(2), "FillColor"));

    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    lcl_dispatchCommand(mxComponent, ".uno:ResetAttributes", {});

    // the background color should revert to the color for 00Background style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14605542), getProperty<sal_Int32>(getParagraph(2), "FillColor"));
    // the paragraph style should not be reset
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // Save it and load it back.
    reload("writer8", "tdf47471_paraStyleBackgroundRT.odt");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(14605542), getProperty<sal_Int32>(getParagraph(2), "FillColor"));
    // on round-trip, the paragraph style name was lost
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
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

void SwUiWriterTest2::testTdf64242_optimizeTable()
{
    SwDoc* pDoc = createDoc("tdf64242_optimizeTable.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);

    double origWidth = getProperty<double>(xTextTable, "Width");
    sal_Int32 nToleranceW = origWidth * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width", double(17013), origWidth, nToleranceW);

    pWrtShell->SelTable(); //select the whole table

    lcl_dispatchCommand(mxComponent, ".uno:SetOptimalColumnWidth", {});
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width: optimize", origWidth,
                                         getProperty<double>(xTextTable, "Width"), nToleranceW);

    lcl_dispatchCommand(mxComponent, ".uno:SetMinimalColumnWidth", {});
    CPPUNIT_ASSERT_MESSAGE("Table Width: minimized",
                           (origWidth - nToleranceW) > getProperty<double>(xTextTable, "Width"));

    double origRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    sal_Int32 nToleranceH = origRowHeight * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Row Height", double(3441), origRowHeight, nToleranceH);

    lcl_dispatchCommand(mxComponent, ".uno:SetOptimalRowHeight", {});
    double optimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    CPPUNIT_ASSERT_MESSAGE("Row Height: optimized",
                           (origRowHeight - nToleranceH) > optimalRowHeight);

    lcl_dispatchCommand(mxComponent, ".uno:SetMinimalRowHeight", {});
    double minimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    CPPUNIT_ASSERT_MESSAGE("Row Height: minimized",
                           (optimalRowHeight - nToleranceH) > minimalRowHeight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row set to auto-height", double(0), minimalRowHeight);
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

void SwUiWriterTest2::testUnfloatButtonSmallTable()
{
    // The floating table in the test document is too small, so we don't provide an unfloat button
    load(FLOATING_TABLE_DATA_DIRECTORY, "small_floating_table.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    const SwSortedObjs* pAnchored
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchored);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnchored->size());
    SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

    SwFlyFrame* pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
    CPPUNIT_ASSERT(pFlyFrame);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));

    SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
    CPPUNIT_ASSERT(pObj);
    pWrtShell->SelectObj(Point(), 0, pObj);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));
}

void SwUiWriterTest2::testUnfloatButton()
{
    // Different use cases where unfloat button should be visible
    const std::vector<OUString> aTestFiles = {
        "unfloatable_floating_table.odt", // Typical use case of multipage floating table
        "unfloatable_floating_table.docx", // Need to test the DOCX import whether we detect the floating table correctly
        "unfloatable_floating_table.doc", // Also the DOC import
        "unfloatable_small_floating_table.docx" // Atypical use case, when the table is small, but because of it's position is it broken to two pages
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString("Failure in the test file: ") + sTestFileName;

        load(FLOATING_TABLE_DATA_DIRECTORY, sTestFileName.getStr());
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pTextDoc);
        SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        const SwSortedObjs* pAnchored;
        if (sTestFileName == "unfloatable_small_floating_table.docx")
            pAnchored = pWrtShell->GetLayout()
                            ->GetLower()
                            ->GetLower()
                            ->GetLower()
                            ->GetNext()
                            ->GetDrawObjs();
        else
            pAnchored = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pAnchored);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), static_cast<size_t>(1),
                                     pAnchored->size());
        SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

        // The unfloat button is not visible until it gets selected
        SwFlyFrame* pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               !pFlyFrame->IsShowUnfloatButton(pWrtShell));

        SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pObj);
        pWrtShell->SelectObj(Point(), 0, pObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame->IsShowUnfloatButton(pWrtShell));
    }
}

void SwUiWriterTest2::testUnfloatButtonReadOnlyMode()
{
    // In read only mode we don't show the unfloat button even if we have a multipage floating table
    load(FLOATING_TABLE_DATA_DIRECTORY, "unfloatable_floating_table.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->SetReadonlyOption(true);

    const SwSortedObjs* pAnchored
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchored);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnchored->size());
    SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

    SwFlyFrame* pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
    CPPUNIT_ASSERT(pFlyFrame);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));

    SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
    CPPUNIT_ASSERT(pObj);
    pWrtShell->SelectObj(Point(), 0, pObj);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));
}

void SwUiWriterTest2::testUnfloating()
{
    // Test unfloating with tables imported from different file formats
    const std::vector<OUString> aTestFiles = {
        "unfloatable_floating_table.odt",
        "unfloatable_floating_table.docx",
        "unfloatable_floating_table.doc",
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OString("Failure in the test file: ") + sTestFileName;

        // Test what happens when pushing the unfloat button
        load(FLOATING_TABLE_DATA_DIRECTORY, "unfloatable_floating_table.docx");
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pTextDoc);
        SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        SwFlyFrame* pFlyFrame;

        // Before unfloating we have only one page with a fly frame
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), SwFrameType::Page,
                                         pWrtShell->GetLayout()->GetLower()->GetType());
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                                   !pWrtShell->GetLayout()->GetLower()->GetNext());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                sFailureMessage.getStr(), SwFrameType::Txt,
                pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetType());
            const SwSortedObjs* pAnchored
                = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pAnchored);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), static_cast<size_t>(1),
                                         pAnchored->size());
            SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];
            pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame);
        }

        // Select the floating table
        SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pObj);
        pWrtShell->SelectObj(Point(), 0, pObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame->IsShowUnfloatButton(pWrtShell));

        // Push the unfloat button
        pFlyFrame->ActiveUnfloatButton(pWrtShell);
        Scheduler::ProcessEventsToIdle();

        // After unfloating we have two pages with one table frame on each page
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               pWrtShell->GetLayout()->GetLower()->GetNext());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), SwFrameType::Page,
                                     pWrtShell->GetLayout()->GetLower()->GetNext()->GetType());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               !pWrtShell->GetLayout()->GetLower()->GetNext()->GetNext());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sFailureMessage.getStr(), SwFrameType::Tab,
            pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetType());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sFailureMessage.getStr(), SwFrameType::Tab,
            pWrtShell->GetLayout()->GetLower()->GetNext()->GetLower()->GetLower()->GetType());
    }
}

void SwUiWriterTest2::testTdf122893()
{
    load(DATA_DIRECTORY, "tdf105413.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // all paragraphs are left-aligned with preset single line spacing
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(i), "ParaAdjust"));
        lcl_dispatchCommand(mxComponent, ".uno:SpacePara1", {});
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

    // Set center-aligned paragraph with preset double line spacing in the 3th paragraph.
    // Because of the tracked deleted region between them,
    // this sets also the same formatting in the first paragraph automatically
    // to keep the changed paragraph formatting at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    lcl_dispatchCommand(mxComponent, ".uno:CenterPara", {});
    lcl_dispatchCommand(mxComponent, ".uno:SpacePara2", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // center-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200),
                         getProperty<style::LineSpacing>(getParagraph(3), "ParaLineSpacing")
                             .Height); // double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // left-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<style::LineSpacing>(getParagraph(2), "ParaLineSpacing")
                             .Height); // single line spacing
    // first paragraph is also center-aligned with double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(getParagraph(1), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(200), getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing").Height);
}

void SwUiWriterTest2::testTdf122901()
{
    load(DATA_DIRECTORY, "tdf105413.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // all paragraphs with zero borders
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), "ParaTopMargin"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), "ParaBottomMargin"));
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

    // Increase paragraph borders in the 3th paragraph, similar to the default icon of the UI
    // "Increase Paragraph Spacing". Because of the tracked deleted region between them,
    // this sets also the same formatting in the first paragraph automatically
    // to keep the changed paragraph formatting at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    lcl_dispatchCommand(mxComponent, ".uno:ParaspaceIncrease", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(101), getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));

    // first paragraph is also center-aligned with double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
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
    // 'Expected less than: 0; Actual: 1030', i.e. the shape was below the
    // paragraph mark, not above it.
    const SwFormatVertOrient& rVert = rFormats[1]->GetVertOrient();
    CPPUNIT_ASSERT_LESS(static_cast<SwTwips>(0), rVert.GetPos());

    reload("writer8", "tdf122942.odt");
    pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pDoc = pWrtShell->GetDoc();
    const SwFrameFormats& rFormats2 = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats2.size());

    SdrObject* pObject = rFormats2[1]->FindSdrObject();
    CPPUNIT_ASSERT(pObject);

    const tools::Rectangle& rOutRect = pObject->GetLastBoundRect();
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected greater than: 5000; Actual: 2817', i.e. the shape moved up
    // after a reload(), while it's expected to not change its position (5773).
    CPPUNIT_ASSERT_GREATER(static_cast<SwTwips>(5000), rOutRect.Top());
}

void SwUiWriterTest2::testTdf52391()
{
    load(DATA_DIRECTORY, "tdf52391.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    lcl_dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1);
    // this was "Portion1", because the tracked background color of Portion1 was
    // accepted for "Reject All". Now rejection clears formatting of the text
    // in format-only changes, concatenating the text portions in the first paragraph.
    CPPUNIT_ASSERT_EQUAL(OUString("Portion1Portion2"), xRun->getString());
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

void SwUiWriterTest2::testTextFormFieldInsertion()
{
    load(DATA_DIRECTORY, "frame_size_export.docx");
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a text form field
    lcl_dispatchCommand(mxComponent, ".uno:TextFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMTEXT), pFieldmark->GetFieldname());

    // The text form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT(xPara->getString().isEmpty());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());
}

void SwUiWriterTest2::testCheckboxFormFieldInsertion()
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a checkbox form field
    lcl_dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
    // The checkbox is not checked by default
    ::sw::mark::ICheckboxFieldmark* pCheckBox
        = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(pFieldmark);
    CPPUNIT_ASSERT(pCheckBox);
    CPPUNIT_ASSERT(!pCheckBox->IsChecked());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
}

void SwUiWriterTest2::testDropDownFormFieldInsertion()
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a drop-down form field
    lcl_dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
    // Check drop down field's parameters. By default these params are not set
    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    CPPUNIT_ASSERT(bool(pListEntries == pParameters->end()));
    auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
    CPPUNIT_ASSERT(bool(pResult == pParameters->end()));

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
}

void SwUiWriterTest2::testMixedFormFieldInsertion()
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert fields
    lcl_dispatchCommand(mxComponent, ".uno:TextFormField", {});
    lcl_dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    lcl_dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest2);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
