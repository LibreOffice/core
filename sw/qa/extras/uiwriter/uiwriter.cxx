/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>

#include "UndoManager.hxx"

static const char* DATA_DIRECTORY = "/sw/qa/extras/uiwriter/data/";

class SwUiWriterTest : public SwModelTestBase
{

public:
    void testReplaceForward();
    //Regression test of fdo#70143
    //EDITING: undo search&replace corrupt text when searching backward
    void testReplaceBackward();
    void testFdo69893();
    void testFdo75110();
    void testFdo75898();

    void testTdf113877_mergeDocs(const char* aDestDoc, const char* aInsertDoc);
    void testTdf113877_blank_bold_on();
    void testTdf113877_blank_bold_off();

    void testTdf113877();
    void testTdf113877NoMerge();
    void testTdf113877_default_style();
    void testTdf113877_Standard_style();

    CPPUNIT_TEST_SUITE(SwUiWriterTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST(testFdo69893);
    CPPUNIT_TEST(testFdo75110);
    CPPUNIT_TEST(testFdo75898);
    CPPUNIT_TEST(testTdf113877_blank_bold_on);
    CPPUNIT_TEST(testTdf113877_blank_bold_off);
    CPPUNIT_TEST(testTdf113877);
    CPPUNIT_TEST(testTdf113877NoMerge);
    CPPUNIT_TEST(testTdf113877_default_style);
    CPPUNIT_TEST(testTdf113877_Standard_style);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = 0);
};

SwDoc* SwUiWriterTest::createDoc(const char* pName)
{
    if (!pName)
        pName = "empty.odt";
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    return pTxtDoc->GetDocShell()->GetDoc();
}

//Replacement tests

static void lcl_selectCharacters(SwPaM& rPaM, sal_Int32 first, sal_Int32 end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), end);
}

static const OUString ORIGINAL_REPLACE_CONTENT("toto titi tutu");
static const OUString EXPECTED_REPLACE_CONTENT("toto toto tutu");

void SwUiWriterTest::testReplaceForward()
{
    SwDoc* pDoc = createDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, ORIGINAL_REPLACE_CONTENT);

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();
    lcl_selectCharacters(aPaM, 5, 9);
    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTxtNode->GetTxt());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTxtNode->GetTxt());
}

void SwUiWriterTest::testFdo75110()
{
    SwDoc* pDoc = createDoc("fdo75110.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll();
    // The problem was that SwEditShell::DeleteSel() what this Delete() invokes took the wrong selection...
    pWrtShell->Delete();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    // ... so this Undo() call resulted in a crash.
    rUndoManager.Undo();
}

void SwUiWriterTest::testFdo75898()
{
    SwDoc* pDoc = createDoc("fdo75898.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    pWrtShell->InsertRow(1, true);
    pWrtShell->InsertRow(1, true);

    // Now check if the table has 3 lines.
    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    SwTableNode* pTableNode = pShellCrsr->Start()->nNode.GetNode().FindTableNode();
    // This was 1, when doing the same using the UI, Writer even crashed.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pTableNode->GetTable().GetTabLines().size());
}

void SwUiWriterTest::testReplaceBackward()
{
    SwDoc* pDoc = createDoc();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, OUString("toto titi tutu"));
    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_EQUAL(EXPECTED_REPLACE_CONTENT, pTxtNode->GetTxt());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(ORIGINAL_REPLACE_CONTENT, pTxtNode->GetTxt());
}

void SwUiWriterTest::testFdo69893()
{
    SwDoc* pDoc = createDoc("fdo69893.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SelAll();

    SwShellCrsr* pShellCrsr = pWrtShell->getShellCrsr(false);
    SwTxtNode& rEnd = dynamic_cast<SwTxtNode&>(pShellCrsr->End()->nNode.GetNode());
    // Selection did not include the para after table, this was "B1".
    CPPUNIT_ASSERT_EQUAL(OUString("Para after table."), rEnd.GetTxt());
}

void SwUiWriterTest::testTdf113877_mergeDocs(const char* aDestDoc, const char* aInsertDoc)
{
    load(DATA_DIRECTORY, aDestDoc);

    // set a page cursor into the end of the document
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    // insert the same document at current cursor position
    {
        const OUString insertFileid = m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(aInsertDoc);
        uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
        lcl_dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);
    }
}

// During insert of the document with list inside into the main document inside the list
// we should merge both lists into one, when they have the same list properties
void SwUiWriterTest::testTdf113877()
{
    testTdf113877_mergeDocs("tdf113877_insert_numbered_list.odt", "tdf113877_insert_numbered_list.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId4 = getProperty<OUString>(getParagraph(4), "ListId");
    const OUString listId5 = getProperty<OUString>(getParagraph(5), "ListId");
    const OUString listId6 = getProperty<OUString>(getParagraph(6), "ListId");
    const OUString listId7 = getProperty<OUString>(getParagraph(7), "ListId");

    // the initial list with 4 list items
    CPPUNIT_ASSERT_EQUAL(listId1, listId4);

    // the last of the first list, and the first of the inserted list
    CPPUNIT_ASSERT_EQUAL(listId4, listId5);
    CPPUNIT_ASSERT_EQUAL(listId5, listId6);
    CPPUNIT_ASSERT_EQUAL(listId6, listId7);
}

// The same test as testTdf113877() but merging of two list should not be performed.
void SwUiWriterTest::testTdf113877NoMerge()
{
    testTdf113877_mergeDocs("tdf113877_insert_numbered_list.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId4 = getProperty<OUString>(getParagraph(4), "ListId");
    const OUString listId5 = getProperty<OUString>(getParagraph(5), "ListId");
    const OUString listId6 = getProperty<OUString>(getParagraph(6), "ListId");
    const OUString listId7 = getProperty<OUString>(getParagraph(7), "ListId");

    // the initial list with 4 list items
    CPPUNIT_ASSERT_EQUAL(listId1, listId4);

    // the last of the first list, and the first of the inserted list
    CPPUNIT_ASSERT(listId4 != listId5);
    CPPUNIT_ASSERT_EQUAL(listId5, listId6);
    CPPUNIT_ASSERT(listId6 != listId7);
}

// Related test to testTdf113877(): Inserting into empty document a new document with list.
// Insert position has NO its own paragraph style ("Standard" will be used).
//
// Resulting document should be the same for following tests:
// - testTdf113877_default_style()
// - testTdf113877_Standard_style()
//
void SwUiWriterTest::testTdf113877_default_style()
{
    testTdf113877_mergeDocs("tdf113877_blank.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

// Related test to testTdf113877(): Inserting into empty document a new document with list.
// Insert position has its own paragraph style derived from "Standard", but this style is the same as "Standard".
//
// Resulting document should be the same for following tests:
// - testTdf113877_default_style()
// - testTdf113877_Standard_style()
//
void SwUiWriterTest::testTdf113877_Standard_style()
{
    testTdf113877_mergeDocs("tdf113877_blank_ownStandard.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest::testTdf113877_blank_bold_on()
{
    testTdf113877_mergeDocs("tdf113877_blank_bold_on.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest::testTdf113877_blank_bold_off()
{
    testTdf113877_mergeDocs("tdf113877_blank_bold_off.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
