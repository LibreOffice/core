/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>

#include "UndoManager.hxx"

static const char* DATA_DIRECTORY = "/sw/qa/extras/uiwriter/data/";

class SwUiWriterTest : public SwModelTestBase
{
    SwDoc* createEmptyDoc();

public:
    void testReplaceForward();
    void testReplaceBackward();

    CPPUNIT_TEST_SUITE(SwUiWriterTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST_SUITE_END();

private:
};

SwDoc* SwUiWriterTest::createEmptyDoc()
{
    load(DATA_DIRECTORY, "empty.odt");

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    return pTxtDoc->GetDocShell()->GetDoc();
}

static void lcl_selectCharacters(SwPaM& rPaM, int first, int end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), end);
}

void SwUiWriterTest::testReplaceForward()
{
    SwDoc* pDoc = createEmptyDoc();

    sw::UndoManager& aUndoManager = pDoc->GetUndoManager();
    aUndoManager.DoUndo(true);

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, OUString("toto titi tutu"));

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    lcl_selectCharacters(aPaM, 5, 9);

    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_MESSAGE("Replacement of titi by tutu",
        OUString("toto toto tutu") == pTxtNode->GetTxt());

    aUndoManager.Undo();

    CPPUNIT_ASSERT_MESSAGE("Unodoing replacement of titi by tutu",
        pTxtNode->GetTxt() == "toto titi tutu");
}

void SwUiWriterTest::testReplaceBackward()
{
    SwDoc* pDoc = createEmptyDoc();

    sw::UndoManager& aUndoManager = pDoc->GetUndoManager();
    aUndoManager.DoUndo(true);

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, OUString("toto titi tutu"));

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    //select titi
    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_MESSAGE("Replacement of titi by tutu",
        OUString("toto toto tutu") == pTxtNode->GetTxt());

    aUndoManager.Undo();

    CPPUNIT_ASSERT_MESSAGE("Undoing replacement of titi by tutu",
        pTxtNode->GetTxt() == "toto titi tutu");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
