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


class SwDocModelTest : public SwModelTestBase
{
public:
    void testReplaceForward();
    void testReplaceBackward();

    CPPUNIT_TEST_SUITE(SwDocModelTest);
    CPPUNIT_TEST(testReplaceForward);
    CPPUNIT_TEST(testReplaceBackward);
    CPPUNIT_TEST_SUITE_END();

private:
};

static void lcl_selectCharacters(SwPaM& rPaM, int first, int end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetCntntNode(), end);
}

void SwDocModelTest::testReplaceForward()
{
    load("/sw/qa/extras/model/data/", "test.odt");

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();

    sw::UndoManager& aUndoManager = pDoc->GetUndoManager();
    aUndoManager.DoUndo(true);

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, OUString("toto titi tutu"));

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    lcl_selectCharacters(aPaM, 5, 9);
    //select titi

    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_MESSAGE("Replacment of titi by tutu",
        OUString("toto toto tutu") == pTxtNode->GetTxt());

    aUndoManager.Undo();

    CPPUNIT_ASSERT_MESSAGE("Replacment of titi by tutu",
        OUString("toto titi tutu") == pTxtNode->GetTxt());
}

void SwDocModelTest::testReplaceBackward()
{
    load("/sw/qa/extras/model/data/", "test.odt");

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();

    sw::UndoManager& aUndoManager = pDoc->GetUndoManager();
    aUndoManager.DoUndo(true);

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->InsertString(aPaM, OUString("toto titi tutu"));

    SwTxtNode* pTxtNode = aPaM.GetNode()->GetTxtNode();

    lcl_selectCharacters(aPaM, 9, 5);

    pDoc->ReplaceRange(aPaM, OUString("toto"), false);

    CPPUNIT_ASSERT_MESSAGE("Replacment of titi by tutu",
        OUString("toto toto tutu") == pTxtNode->GetTxt());

    aUndoManager.Undo();

    CPPUNIT_ASSERT_MESSAGE("Replacment of titi by tutu",
        OUString("toto titi tutu") == pTxtNode->GetTxt());
}



CPPUNIT_TEST_SUITE_REGISTRATION(SwDocModelTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
