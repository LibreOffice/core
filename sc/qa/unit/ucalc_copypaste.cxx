/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <test/bootstrapfixture.hxx>
#include "helper/debughelper.hxx"
#include "helper/qahelper.hxx"

#include <attrib.hxx>
#include <bcaslot.hxx>
#include <clipparam.hxx>
#include <dbdata.hxx>
#include <docfunc.hxx>
#include <docpool.hxx>
#include <editeng/borderline.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editobj.hxx>
#include <editutil.hxx>
#include <formulacell.hxx>
#include <formulaopt.hxx>
#include <iostream>
#include <patattr.hxx>
#include <postit.hxx>
#include <queryentry.hxx>
#include <queryparam.hxx>
#include <refundo.hxx>
#include <scdll.hxx>
#include <scitems.hxx>
#include <scmod.hxx>
#include <scopetools.hxx>

#include <sfx2/docfile.hxx>

class TestCopyPaste : public test::BootstrapFixture
{
public:
    TestCopyPaste();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCopyPaste();
    void testCopyPasteAsLink();
    void testCopyPasteTranspose();
    void testCopyPasteSpecialAsLinkTranspose();
    void testCopyPasteSpecialAsLinkFilteredTranspose();
    void testCopyPasteSpecialMultiRangeColAsLinkTranspose();
    void testCopyPasteSpecialMultiRangeColAsLinkFilteredTranspose();
    void testCopyPasteSpecialMultiRangeRowAsLinkTranspose();
    void testCopyPasteSpecialMultiRangeRowAsLinkFilteredTranspose();
    void testCopyPasteSpecialAllAsLinkTranspose();
    void testCopyPasteSpecialAllAsLinkFilteredTranspose();
    void testCopyPasteSpecial();
    void testCopyPasteSpecialFiltered();
    void testCopyPasteSpecialIncludeFiltered();
    void testCopyPasteSpecialFilteredIncludeFiltered();
    void testCopyPasteSpecialTranspose();
    void testCopyPasteSpecialTransposeIncludeFiltered();
    void testCopyPasteSpecialFilteredTranspose();
    void testCopyPasteSpecialMergedCellsTranspose();
    void testCopyPasteSpecialMergedCellsFilteredTranspose();
    void testCopyPasteSpecialMultiRangeCol();
    void testCopyPasteSpecialMultiRangeColFiltered();
    void testCopyPasteSpecialMultiRangeColIncludeFiltered();
    void testCopyPasteSpecialMultiRangeColFilteredIncludeFiltered();
    void testCopyPasteSpecialMultiRangeColTranspose();
    void testCopyPasteSpecialMultiRangeColFilteredTranspose();
    void testCopyPasteSpecialMultiRangeColFilteredIncludeFilteredTranspose();
    void testCopyPasteSpecialMultiRangeRow();
    void testCopyPasteSpecialMultiRangeRowFiltered();
    void testCopyPasteSpecialMultiRangeRowIncludeFiltered();
    void testCopyPasteSpecialMultiRangeRowFilteredIncludeFiltered();
    void testCopyPasteSpecialMultiRangeRowTranspose();
    void testCopyPasteSpecialMultiRangeRowFilteredTranspose();
    void testCopyPasteSpecialMultiRangeRowFilteredIncludeFilteredTranspose();
    void testCopyPasteSpecialSkipEmpty();
    void testCopyPasteSpecialSkipEmptyFiltered();
    void testCopyPasteSpecialSkipEmptyIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyFilteredIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyTranspose();
    void testCopyPasteSpecialSkipEmptyTransposeIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyFilteredTranspose();
    void testCopyPasteSpecialSkipEmptyMultiRangeCol();
    void testCopyPasteSpecialSkipEmptyMultiRangeColFiltered();
    void testCopyPasteSpecialSkipEmptyMultiRangeColIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyMultiRangeColTranspose();
    void testCopyPasteSpecialSkipEmptyMultiRangeColFilteredTranspose();
    void testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFilteredTranspose();
    void testCopyPasteSpecialSkipEmptyMultiRangeRow();
    void testCopyPasteSpecialSkipEmptyMultiRangeRowFiltered();
    void testCopyPasteSpecialSkipEmptyMultiRangeRowIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFiltered();
    void testCopyPasteSpecialSkipEmptyMultiRangeRowTranspose();
    void testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredTranspose();
    void testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFilteredTranspose();
    void testCopyPasteMultiRange();
    void testCopyPasteSkipEmpty();
    void testCopyPasteSkipEmpty2();
    void testCutPasteRefUndo();
    void testCutPasteGroupRefUndo();
    void testMoveRefBetweenSheets();
    void testUndoCut();
    void testMoveBlock();
    void testCopyPasteRelativeFormula();
    void testCopyPasteRepeatOneFormula();
    void testCopyPasteMixedReferenceFormula();
    void testCopyPasteFormulas();
    void testCopyPasteFormulasExternalDoc();
    void testCopyPasteReferencesExternalDoc(); // tdf#106456

    CPPUNIT_TEST_SUITE(TestCopyPaste);

    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testCopyPasteAsLink);
    CPPUNIT_TEST(testCopyPasteTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialAsLinkTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialAllAsLinkTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColAsLinkTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColAsLinkFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowAsLinkTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowAsLinkFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialAsLinkFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialAllAsLinkFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMergedCellsTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMergedCellsFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecial);
    CPPUNIT_TEST(testCopyPasteSpecialFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialFilteredIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialTransposeIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeCol);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColFilteredIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeColFilteredIncludeFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRow);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowFilteredIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialMultiRangeRowFilteredIncludeFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmpty);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyFilteredIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyTransposeIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeCol);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeColFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeColIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeColTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeColFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRow);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRowFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRowIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFiltered);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRowTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFilteredTranspose);
    CPPUNIT_TEST(testCopyPasteMultiRange);
    CPPUNIT_TEST(testCopyPasteSkipEmpty);
    CPPUNIT_TEST(testCopyPasteSkipEmpty2);
    CPPUNIT_TEST(testCutPasteRefUndo);
    CPPUNIT_TEST(testCutPasteGroupRefUndo);
    CPPUNIT_TEST(testMoveRefBetweenSheets);
    CPPUNIT_TEST(testUndoCut);
    CPPUNIT_TEST(testMoveBlock);
    CPPUNIT_TEST(testCopyPasteRelativeFormula);
    CPPUNIT_TEST(testCopyPasteRepeatOneFormula);
    CPPUNIT_TEST(testCopyPasteMixedReferenceFormula);

    CPPUNIT_TEST(testCopyPasteFormulas);
    CPPUNIT_TEST(testCopyPasteFormulasExternalDoc);
    CPPUNIT_TEST(testCopyPasteReferencesExternalDoc);

    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef m_xDocShell;
    ScDocument* m_pDoc;

    void executeCopyPasteSpecial(bool bApplyFilter, bool bIncludedFiltered, bool bAsLink,
                                 bool bTranspose, bool bMultiRangeSelection, bool bSkipEmpty,
                                 ScClipParam::Direction eDirection, bool bCalcAll,
                                 InsertDeleteFlags aFlags);
    void checkCopyPasteSpecial(bool bSkipEmpty);
    void checkCopyPasteSpecialFiltered(bool bSkipEmpty);
    void checkCopyPasteSpecialTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialFilteredTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeCol(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeColFiltered(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeColTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeColFilteredTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRow(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRowFiltered(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRowTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRowFilteredTranspose(bool bSkipEmpty);
};

TestCopyPaste::TestCopyPaste() {}

void TestCopyPaste::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();

    m_xDocShell
        = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS
                         | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    m_xDocShell->SetIsInUcalc();
    m_xDocShell->DoInitUnitTest();

    m_pDoc = &m_xDocShell->GetDocument();
}

void TestCopyPaste::tearDown()
{
    m_xDocShell->DoClose();
    m_xDocShell.clear();

    test::BootstrapFixture::tearDown();
}

static ScMF lcl_getMergeFlagOfCell(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const SfxPoolItem& rPoolItem = rDoc.GetPattern(nCol, nRow, nTab)->GetItem(ATTR_MERGE_FLAG);
    const ScMergeFlagAttr& rMergeFlag = static_cast<const ScMergeFlagAttr&>(rPoolItem);
    return rMergeFlag.GetValue();
}

static ScAddress lcl_getMergeSizeOfCell(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const SfxPoolItem& rPoolItem = rDoc.GetPattern(nCol, nRow, nTab)->GetItem(ATTR_MERGE);
    const ScMergeAttr& rMerge = static_cast<const ScMergeAttr&>(rPoolItem);
    return ScAddress(rMerge.GetColMerge(), rMerge.GetRowMerge(), nTab);
}

void TestCopyPaste::testCopyPaste()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    //test copy&paste + ScUndoPaste
    //copy local and global range names in formulas
    //string cells and value cells
    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetValue(3, 0, 0, 0);
    m_pDoc->SetValue(3, 1, 0, 1);
    m_pDoc->SetValue(3, 2, 0, 2);
    m_pDoc->SetValue(3, 3, 0, 3);
    m_pDoc->SetString(2, 0, 0, "test");
    ScAddress aAdr(0, 0, 0);

    //create some range names, local and global
    ScRangeData* pLocal1 = new ScRangeData(*m_pDoc, "local1", aAdr);
    ScRangeData* pLocal2 = new ScRangeData(*m_pDoc, "local2", aAdr);
    ScRangeData* pLocal3 = new ScRangeData(*m_pDoc, "local3", "$Sheet1.$A$1");
    ScRangeData* pLocal4 = new ScRangeData(*m_pDoc, "local4", "Sheet1.$A$1");
    ScRangeData* pLocal5
        = new ScRangeData(*m_pDoc, "local5", "$A$1"); // implicit relative sheet reference
    ScRangeData* pGlobal = new ScRangeData(*m_pDoc, "global", aAdr);
    const OUString aGlobal2Symbol("$Sheet1.$A$1:$A$23");
    ScRangeData* pGlobal2 = new ScRangeData(*m_pDoc, "global2", aGlobal2Symbol);
    std::unique_ptr<ScRangeName> pGlobalRangeName(new ScRangeName());
    pGlobalRangeName->insert(pGlobal);
    pGlobalRangeName->insert(pGlobal2);
    std::unique_ptr<ScRangeName> pLocalRangeName1(new ScRangeName());
    pLocalRangeName1->insert(pLocal1);
    pLocalRangeName1->insert(pLocal2);
    pLocalRangeName1->insert(pLocal3);
    pLocalRangeName1->insert(pLocal4);
    pLocalRangeName1->insert(pLocal5);
    m_pDoc->SetRangeName(std::move(pGlobalRangeName));
    m_pDoc->SetRangeName(0, std::move(pLocalRangeName1));

    // Add formula to B1.
    OUString aFormulaString("=local1+global+SUM($C$1:$D$4)+local3+local4+local5");
    m_pDoc->SetString(1, 0, 0, aFormulaString);

    double fValue = m_pDoc->GetValue(ScAddress(1, 0, 0));
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 11", fValue, 11);

    // add notes to A1:C1
    ScAddress aAdrA1(0, 0, 0); // empty cell content
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, "Hello world in A1");
    ScAddress aAdrB1(1, 0, 0); // formula cell content
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAdrB1);
    pNoteB1->SetText(aAdrB1, "Hello world in B1");
    ScAddress aAdrC1(2, 0, 0); // string cell content
    ScPostIt* pNoteC1 = m_pDoc->GetOrCreateNote(aAdrC1);
    pNoteC1->SetText(aAdrC1, "Hello world in C1");

    //copy Sheet1.A1:C1 to Sheet2.A2:C2
    ScRange aRange(0, 0, 0, 2, 0, 0);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    aRange = ScRange(0, 1, 1, 2, 1, 1); //target: Sheet2.A2:C2
    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(*m_pDoc, 1, 1, true, true);
    std::unique_ptr<ScUndoPaste> pUndo(createUndoPaste(*m_xDocShell, aRange, std::move(pUndoDoc)));
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    m_pDoc->CopyFromClip(aRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

    //check values after copying
    OUString aString;
    m_pDoc->GetFormula(1, 1, 1, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula string was not copied correctly", aString,
                                 aFormulaString);
    // Only the global range points to Sheet1.A1, all copied sheet-local ranges
    // to Sheet2.A1 that is empty, hence the result is 1, not 2.
    fValue = m_pDoc->GetValue(ScAddress(1, 1, 1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("copied formula should return 1", 1.0, fValue);
    fValue = m_pDoc->GetValue(ScAddress(0, 1, 1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("copied value should be 1", 1.0, fValue);

    ScRange aSheet2A1(0, 0, 1, 0, 0, 1);

    //check local range name after copying
    pLocal1 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("local range name 1 should be copied", pLocal1);
    ScRange aRangeLocal1;
    bool bIsValidRef1 = pLocal1->IsValidReference(aRangeLocal1);
    CPPUNIT_ASSERT_MESSAGE("local range name 1 should be valid", bIsValidRef1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 1 should now point to Sheet2.A1", aSheet2A1,
                                 aRangeLocal1);

    pLocal2 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("local2 should not be copied", !pLocal2);

    pLocal3 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL3"));
    CPPUNIT_ASSERT_MESSAGE("local range name 3 should be copied", pLocal3);
    ScRange aRangeLocal3;
    bool bIsValidRef3 = pLocal3->IsValidReference(aRangeLocal3);
    CPPUNIT_ASSERT_MESSAGE("local range name 3 should be valid", bIsValidRef3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 3 should now point to Sheet2.A1", aSheet2A1,
                                 aRangeLocal3);

    pLocal4 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL4"));
    CPPUNIT_ASSERT_MESSAGE("local range name 4 should be copied", pLocal4);
    ScRange aRangeLocal4;
    bool bIsValidRef4 = pLocal4->IsValidReference(aRangeLocal4);
    CPPUNIT_ASSERT_MESSAGE("local range name 4 should be valid", bIsValidRef4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 4 should now point to Sheet2.A1", aSheet2A1,
                                 aRangeLocal4);

    pLocal5 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL5"));
    CPPUNIT_ASSERT_MESSAGE("local range name 5 should be copied", pLocal5);
    ScRange aRangeLocal5;
    bool bIsValidRef5 = pLocal5->IsValidReference(aRangeLocal5);
    CPPUNIT_ASSERT_MESSAGE("local range name 5 should be valid", bIsValidRef5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 5 should now point to Sheet2.A1", aSheet2A1,
                                 aRangeLocal5);

    // check notes after copying
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.A2",
                           m_pDoc->HasNote(ScAddress(0, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.B2",
                           m_pDoc->HasNote(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.C2",
                           m_pDoc->HasNote(ScAddress(2, 1, 1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Note content on Sheet1.A1 not copied to Sheet2.A2, empty cell content",
        m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(),
        m_pDoc->GetNote(ScAddress(0, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Note content on Sheet1.B1 not copied to Sheet2.B2, formula cell content",
        m_pDoc->GetNote(ScAddress(1, 0, 0))->GetText(),
        m_pDoc->GetNote(ScAddress(1, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Note content on Sheet1.C1 not copied to Sheet2.C2, string cell content",
        m_pDoc->GetNote(ScAddress(2, 0, 0))->GetText(),
        m_pDoc->GetNote(ScAddress(2, 1, 1))->GetText());

    //check undo and redo
    pUndo->Undo();
    fValue = m_pDoc->GetValue(ScAddress(1, 1, 1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("after undo formula should return nothing", fValue, 0);
    aString = m_pDoc->GetString(2, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("after undo, string should be removed", aString.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("after undo, note on A2 should be removed",
                           !m_pDoc->HasNote(ScAddress(0, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("after undo, note on B2 should be removed",
                           !m_pDoc->HasNote(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("after undo, note on C2 should be removed",
                           !m_pDoc->HasNote(ScAddress(2, 1, 1)));

    pUndo->Redo();
    fValue = m_pDoc->GetValue(ScAddress(1, 1, 1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 1 after redo", 1.0, fValue);
    aString = m_pDoc->GetString(2, 1, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell Sheet2.C2 should contain: test", OUString("test"), aString);
    m_pDoc->GetFormula(1, 1, 1, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula should be correct again", aFormulaString, aString);

    CPPUNIT_ASSERT_MESSAGE("After Redo, there should be a note on Sheet2.A2",
                           m_pDoc->HasNote(ScAddress(0, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("After Redo, there should be a note on Sheet2.B2",
                           m_pDoc->HasNote(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("After Redo, there should be a note on Sheet2.C2",
                           m_pDoc->HasNote(ScAddress(2, 1, 1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("After Redo, note again on Sheet2.A2, empty cell content",
                                 m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(),
                                 m_pDoc->GetNote(ScAddress(0, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("After Redo, note again on Sheet2.B2, formula cell content",
                                 m_pDoc->GetNote(ScAddress(1, 0, 0))->GetText(),
                                 m_pDoc->GetNote(ScAddress(1, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("After Redo, note again on Sheet2.C2, string cell content",
                                 m_pDoc->GetNote(ScAddress(2, 0, 0))->GetText(),
                                 m_pDoc->GetNote(ScAddress(2, 1, 1))->GetText());

    // Copy Sheet1.A11:A13 to Sheet1.A7:A9, both within global2 range.
    aRange = ScRange(0, 10, 0, 0, 12, 0);
    ScDocument aClipDoc2(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc2);

    aRange = ScRange(0, 6, 0, 0, 8, 0);
    aMark.SetMarkArea(aRange);
    m_pDoc->CopyFromClip(aRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc2);

    // The global2 range must not have changed.
    pGlobal2 = m_pDoc->GetRangeName()->findByUpperName("GLOBAL2");
    CPPUNIT_ASSERT_MESSAGE("GLOBAL2 name not found", pGlobal2);
    OUString aSymbol;
    pGlobal2->GetSymbol(aSymbol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("GLOBAL2 named range changed", aGlobal2Symbol, aSymbol);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteAsLink()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetValue(ScAddress(0, 0, 0), 1); // A1
    m_pDoc->SetValue(ScAddress(0, 1, 0), 2); // A2
    m_pDoc->SetValue(ScAddress(0, 2, 0), 3); // A3

    ScRange aRange(0, 0, 0, 0, 2, 0); // Copy A1:A3 to clip.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    aRange = ScRange(1, 1, 1, 1, 3, 1); // Paste to B2:B4 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    // Paste range as link.
    m_pDoc->CopyFromClip(aRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, &aClipDoc, true,
                         true);

    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, 1));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 2, 1));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 3, 1));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(3.0, pFC->GetValue());

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteTranspose()
{
    m_pDoc->InsertTab(0, "Sheet1");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetString(1, 0, 0, "=A1+1");
    m_pDoc->SetString(2, 0, 0, "test");

    // add notes to A1:C1
    ScAddress aAdrA1(0, 0, 0); // numerical cell content
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, "Hello world in A1");
    ScAddress aAdrB1(1, 0, 0); // formula cell content
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAdrB1);
    pNoteB1->SetText(aAdrB1, "Hello world in B1");
    ScAddress aAdrC1(2, 0, 0); // string cell content
    ScPostIt* pNoteC1 = m_pDoc->GetOrCreateNote(aAdrC1);
    pNoteC1->SetText(aAdrC1, "Hello world in C1");

    // transpose clipboard, paste and check on Sheet2
    m_pDoc->InsertTab(1, "Sheet2");

    ScRange aSrcRange(0, 0, 0, 2, 0, 0);
    ScDocument aNewClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aNewClipDoc);

    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aNewClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, false);

    ScRange aDestRange(3, 1, 1, 3, 3, 1); //target: Sheet2.D2:D4
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get());
    pTransClip.reset();

    //check cell content after transposed copy/paste
    OUString aString = m_pDoc->GetString(3, 3, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell Sheet2.D4 should contain: test", OUString("test"), aString);
    double fValue = m_pDoc->GetValue(ScAddress(3, 1, 1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell should return 1", 1, fValue);
    fValue = m_pDoc->GetValue(ScAddress(3, 2, 1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula should return 2", 2, fValue);
    m_pDoc->GetFormula(3, 2, 1, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula should point on Sheet2.D2", OUString("=D2+1"),
                                 aString);

    // check notes after transposed copy/paste
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.D2",
                           m_pDoc->HasNote(ScAddress(3, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.D3",
                           m_pDoc->HasNote(ScAddress(3, 2, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.D4",
                           m_pDoc->HasNote(ScAddress(3, 3, 1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on Sheet2.D2",
                                 m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on Sheet2.D3",
                                 m_pDoc->GetNote(ScAddress(1, 0, 0))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 2, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on Sheet2.D4",
                                 m_pDoc->GetNote(ScAddress(2, 0, 0))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 3, 1))->GetText());

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteSpecialMergedCellsTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 2, srcSheet, 3); // A3
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    m_pDoc->DoMerge(srcSheet, 0, 1, 1, 1, false); // Merge A2 and B2
    m_pDoc->DoMerge(srcSheet, 0, 2, 1, 2, false); // Merge A3 and B3

    // Test precondition
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 0, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 1, 0, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(2, 1, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 1, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 1, 1, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::Hor, lcl_getMergeFlagOfCell(*m_pDoc, 1, 1, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(2, 1, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 2, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 1, 2, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::Hor, lcl_getMergeFlagOfCell(*m_pDoc, 1, 2, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 3, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 1, 3, srcSheet));

    ScRange aSrcRange(0, 0, srcSheet, 1, 3, srcSheet); // Copy A1:B4 to clip.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 2, destSheet); // Paste to B2:E3 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(), true,
                         false);
    pTransClip.reset();

    // Check transpose of merged cells
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 1, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 1, 2, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 2, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 2, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 2, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 3, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(3, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 4, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 4, 2, destSheet));

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::testCopyPasteSpecialMergedCellsFilteredTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 2, srcSheet, 3); // A3
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    m_pDoc->DoMerge(srcSheet, 0, 1, 1, 1, false); // Merge A2 and B2
    m_pDoc->DoMerge(srcSheet, 0, 2, 1, 2, false); // Merge A3 and B3

    // Filter row 1
    ScDBData* pDBData = new ScDBData("TRANSPOSE_TEST_DATA", srcSheet, 0, 0, 0, 3);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                          aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_NOT_EQUAL;
    rEntry.GetQueryItem().mfVal = 2; // value of row A2 -> filtering row 1
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(srcSheet, aParam, true);

    // Test precondition
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 0, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 1, 0, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(2, 1, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 1, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 1, 1, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::Hor, lcl_getMergeFlagOfCell(*m_pDoc, 1, 1, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(2, 1, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 2, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 1, 2, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::Hor, lcl_getMergeFlagOfCell(*m_pDoc, 1, 2, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, srcSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 0, 3, srcSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 1, 3, srcSheet));

    ScRange aSrcRange(0, 0, srcSheet, 1, 3, srcSheet); // Copy A1:B4 to clip.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, true, false);

    ScRange aDestRange(1, 1, destSheet, 3, 2, destSheet); // Paste to B2:D3 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(), true,
                         false);
    pTransClip.reset();

    // Check transpose of merged cells
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 1, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 1, 2, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 2, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 2, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(2, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScAddress(0, 0, destSheet),
                         lcl_getMergeSizeOfCell(*m_pDoc, 3, 1, destSheet));
    CPPUNIT_ASSERT_EQUAL(ScMF::NONE, lcl_getMergeFlagOfCell(*m_pDoc, 3, 2, destSheet));

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// InsertDeleteFlags::CONTENTS
void TestCopyPaste::testCopyPasteSpecialAsLinkTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    ScRange aSrcRange(0, 0, srcSheet, 0, 3, srcSheet); // Copy A1:A4 to clip.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::CONTENTS, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 1, destSheet); // Paste to B2:E2 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, pTransClip.get(),
                         true, false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(2, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C2", OUString("=$Sheet1.$A$2"), aString);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell D2.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(4, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell E2", OUString("=$Sheet1.$A$4"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// InsertDeleteFlags::CONTENTS
void TestCopyPaste::testCopyPasteSpecialAsLinkFilteredTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    // Filter row 1
    ScDBData* pDBData = new ScDBData("TRANSPOSE_TEST_DATA", srcSheet, 0, 0, 0, 3);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                          aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_NOT_EQUAL;
    rEntry.GetQueryItem().mfVal = 2; // value of row A2 -> filtering row 1
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(srcSheet, aParam, true);

    // Check precondition for test: row 1 is hidden/filtered
    SCROW nRow1, nRow2;
    SCROW nFilteredRow1, nFilteredRow2;
    bool bHidden = m_pDoc->RowHidden(SCROW(1), srcSheet, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be hidden", SCROW(1), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be hidden", SCROW(1), nRow2);
    bool bFiltered = m_pDoc->RowFiltered(SCROW(1), srcSheet, &nFilteredRow1, &nFilteredRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be filtered", bFiltered);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be filtered", SCROW(1), nFilteredRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be filtered", SCROW(1), nFilteredRow2);

    // Copy A1:A4 to clip.
    ScRange aSrcRange(0, 0, srcSheet, 0, 3, srcSheet);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::CONTENTS, true, false);

    ScRange aDestRange(1, 1, destSheet, 3, 1, destSheet); // Paste to B2:D2 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, pTransClip.get(),
                         true, false, false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell C2.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(3, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D2", OUString("=$Sheet1.$A$4"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// tdf#141683
// InsertDeleteFlags::CONTENTS
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowAsLinkTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(1, 0, srcSheet, 2); // B1
    m_pDoc->SetValue(3, 0, srcSheet, 4); // D1

    m_pDoc->SetValue(0, 2, srcSheet, 11); // A3
    m_pDoc->SetValue(1, 2, srcSheet, 12); // B3
    m_pDoc->SetValue(3, 2, srcSheet, 14); // D3

    ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
    aSrcMark.SelectOneTable(0);
    ScClipParam aClipParam;
    aClipParam.meDirection = ScClipParam::Row;
    aClipParam.maRanges.push_back(ScRange(0, 0, srcSheet, 3, 0, srcSheet)); // A1:D1
    aClipParam.maRanges.push_back(ScRange(0, 2, srcSheet, 3, 2, srcSheet)); // A3:D3

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aSrcMark, false, false);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::CONTENTS, true, false);

    ScRange aDestRange(1, 1, destSheet, 2, 4, destSheet); // Paste to B2:C5 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::CONTENTS,
                                   pTransClip.get(), true, false /* false fixes tdf#141683 */,
                                   false, false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B3.", pFC);
    m_pDoc->GetFormula(1, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B3", OUString("=$Sheet1.$B$1"), aString);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 3, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell B4.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(1, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B5", OUString("=$Sheet1.$D$1"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell C2.", pFC);
    m_pDoc->GetFormula(2, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C2", OUString("=$Sheet1.$A$3"), aString);
    CPPUNIT_ASSERT_EQUAL(11.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell C3.", pFC);
    m_pDoc->GetFormula(2, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C3", OUString("=$Sheet1.$B$3"), aString);
    CPPUNIT_ASSERT_EQUAL(12.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 3, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell C4.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(2, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C5", OUString("=$Sheet1.$D$3"), aString);
    CPPUNIT_ASSERT_EQUAL(14.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// tdf#141683
// InsertDeleteFlags::CONTENTS
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowAsLinkFilteredTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(1, 0, srcSheet, 2); // B1
    m_pDoc->SetValue(3, 0, srcSheet, 4); // D1

    m_pDoc->SetValue(0, 1, srcSheet, -1); // A2, filtered and selected
    m_pDoc->SetValue(1, 1, srcSheet, -2); // B2, filtered and selected
    m_pDoc->SetValue(3, 1, srcSheet, -4); // D2, filtered and selected

    m_pDoc->SetValue(0, 2, srcSheet, 11); // A3
    m_pDoc->SetValue(1, 2, srcSheet, 12); // B3
    m_pDoc->SetValue(3, 2, srcSheet, 14); // D3

    m_pDoc->SetValue(0, 3, srcSheet, -11); // A4, filtered and not selected
    m_pDoc->SetValue(1, 3, srcSheet, -12); // B4, filtered and not selected
    m_pDoc->SetValue(3, 3, srcSheet, -14); // D4, filtered and not selected

    m_pDoc->SetValue(0, 5, srcSheet, 111); // A6
    m_pDoc->SetValue(1, 5, srcSheet, 112); // B6
    m_pDoc->SetValue(3, 5, srcSheet, 114); // D6

    // Filter row 1
    ScDBData* pDBData = new ScDBData("TRANSPOSE_TEST_DATA", srcSheet, 0, 0, 3, 3);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                          aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_GREATER_EQUAL;
    rEntry.GetQueryItem().mfVal = 0; // filtering negative values -> filtering row 1 and 2
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(srcSheet, aParam, true);

    ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
    aSrcMark.SelectOneTable(0);
    ScClipParam aClipParam;
    aClipParam.meDirection = ScClipParam::Row;
    aClipParam.maRanges.push_back(ScRange(0, 0, srcSheet, 3, 2, srcSheet)); // A1:C3
    aClipParam.maRanges.push_back(ScRange(0, 5, srcSheet, 3, 5, srcSheet)); // A6:C6

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aSrcMark, false, false);

    printRange(m_pDoc, aClipParam.getWholeRange(), "Src range");
    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::CONTENTS, true, false);

    printRange(&aClipDoc, ScRange(0, 0, 0, 4, 5, 0), "Base doc (&aNewClipDoc)");
    printRange(pTransClip.get(), ScRange(0, 0, 0, 3, 3, 0),
               "Transposed filtered clipdoc (pTransClip.get())");
    ScRange aDestRange(1, 1, destSheet, 3, 4, destSheet); // Paste to B2:D5 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::CONTENTS,
                                   pTransClip.get(), true, false /* false fixes tdf#141683 */,
                                   false, false);
    pTransClip.reset();
    printRange(m_pDoc, aDestRange, "Transposed dest sheet");

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B3.", pFC);
    m_pDoc->GetFormula(1, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B3", OUString("=$Sheet1.$B$1"), aString);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 3, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell B4.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(1, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B5", OUString("=$Sheet1.$D$1"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell C2.", pFC);
    m_pDoc->GetFormula(2, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C2", OUString("=$Sheet1.$A$3"), aString);
    CPPUNIT_ASSERT_EQUAL(11.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell C3.", pFC);
    m_pDoc->GetFormula(2, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C3", OUString("=$Sheet1.$B$3"), aString);
    CPPUNIT_ASSERT_EQUAL(12.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 3, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell C4.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(2, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C5", OUString("=$Sheet1.$D$3"), aString);
    CPPUNIT_ASSERT_EQUAL(14.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell D2.", pFC);
    m_pDoc->GetFormula(3, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D2", OUString("=$Sheet1.$A$6"), aString);
    CPPUNIT_ASSERT_EQUAL(111.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell D3.", pFC);
    m_pDoc->GetFormula(3, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D3", OUString("=$Sheet1.$B$6"), aString);
    CPPUNIT_ASSERT_EQUAL(112.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 3, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell D4.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(3, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D5", OUString("=$Sheet1.$D$6"), aString);
    CPPUNIT_ASSERT_EQUAL(114.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// tdf#141683
// InsertDeleteFlags::CONTENTS
void TestCopyPaste::testCopyPasteSpecialMultiRangeColAsLinkTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    m_pDoc->SetValue(2, 0, srcSheet, 11); // C1
    m_pDoc->SetValue(2, 1, srcSheet, 12); // C2
    m_pDoc->SetValue(2, 3, srcSheet, 14); // C4

    ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
    aSrcMark.SelectOneTable(0);
    ScClipParam aClipParam;
    aClipParam.meDirection = ScClipParam::Column;
    aClipParam.maRanges.push_back(ScRange(0, 0, srcSheet, 0, 3, srcSheet)); // A1:A4
    aClipParam.maRanges.push_back(ScRange(2, 0, srcSheet, 2, 3, srcSheet)); // C1:C4

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aSrcMark, false, false);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::CONTENTS, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 2, destSheet); // Paste to B2:E3 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::CONTENTS,
                                   pTransClip.get(), true, false /* false fixes tdf#141683 */,
                                   false, false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell C2.", pFC);
    m_pDoc->GetFormula(2, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C2", OUString("=$Sheet1.$A$2"), aString);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell D2.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(4, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell E2", OUString("=$Sheet1.$A$4"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B3.", pFC);
    m_pDoc->GetFormula(1, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B3", OUString("=$Sheet1.$C$1"), aString);
    CPPUNIT_ASSERT_EQUAL(11.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell C3.", pFC);
    m_pDoc->GetFormula(2, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C3", OUString("=$Sheet1.$C$2"), aString);
    CPPUNIT_ASSERT_EQUAL(12.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell D3.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(4, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell E3", OUString("=$Sheet1.$C$4"), aString);
    CPPUNIT_ASSERT_EQUAL(14.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// tdf#141683
// InsertDeleteFlags::CONTENTS
void TestCopyPaste::testCopyPasteSpecialMultiRangeColAsLinkFilteredTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    m_pDoc->SetValue(2, 0, srcSheet, 11); // C1
    m_pDoc->SetValue(2, 1, srcSheet, 12); // C2
    m_pDoc->SetValue(2, 3, srcSheet, 14); // C4

    // Filter row 1
    ScDBData* pDBData = new ScDBData("TRANSPOSE_TEST_DATA", srcSheet, 0, 0, 0, 3);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                          aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_NOT_EQUAL;
    rEntry.GetQueryItem().mfVal = 2; // value of row A2 -> filtering row 1
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(srcSheet, aParam, true);

    ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
    aSrcMark.SelectOneTable(0);
    ScClipParam aClipParam;
    aClipParam.meDirection = ScClipParam::Column;
    aClipParam.maRanges.push_back(ScRange(0, 0, srcSheet, 0, 3, srcSheet)); // A1:A4
    aClipParam.maRanges.push_back(ScRange(2, 0, srcSheet, 2, 3, srcSheet)); // C1:C4

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aSrcMark, false, false);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::CONTENTS, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 2, destSheet); // Paste to B2:E3 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::CONTENTS,
                                   pTransClip.get(), true, false /* false fixes tdf#141683 */,
                                   false, false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell C2.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(3, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D2", OUString("=$Sheet1.$A$4"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B3.", pFC);
    m_pDoc->GetFormula(1, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B3", OUString("=$Sheet1.$C$1"), aString);
    CPPUNIT_ASSERT_EQUAL(11.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be no formula cell C3.", !pFC);

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 2, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(3, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D3", OUString("=$Sheet1.$C$4"), aString);
    CPPUNIT_ASSERT_EQUAL(14.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// InsertDeleteFlags::ALL
void TestCopyPaste::testCopyPasteSpecialAllAsLinkTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    ScRange aSrcRange(0, 0, srcSheet, 0, 3, srcSheet); // Copy A1:A4 to clip.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 1, destSheet); // Paste to B2:E2 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(), true,
                         false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(2, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C2", OUString("=$Sheet1.$A$2"), aString);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(3, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D2", OUString("=$Sheet1.$A$3"), aString);
    CPPUNIT_ASSERT_EQUAL(0.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(4, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell E2", OUString("=$Sheet1.$A$4"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// InsertDeleteFlags::ALL
void TestCopyPaste::testCopyPasteSpecialAllAsLinkFilteredTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(srcSheet, "Sheet1");
    m_pDoc->InsertTab(destSheet, "Sheet2");

    m_pDoc->SetValue(0, 0, srcSheet, 1); // A1
    m_pDoc->SetValue(0, 1, srcSheet, 2); // A2
    m_pDoc->SetValue(0, 3, srcSheet, 4); // A4

    // Filter row 1
    ScDBData* pDBData = new ScDBData("TRANSPOSE_TEST_DATA", srcSheet, 0, 0, 0, 3);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                          aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_NOT_EQUAL;
    rEntry.GetQueryItem().mfVal = 2; // value of row A2 -> filtering row 1
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(srcSheet, aParam, true);

    // Check precondition for test: row 1 is hidden/filtered
    SCROW nRow1, nRow2;
    SCROW nFilteredRow1, nFilteredRow2;
    bool bHidden = m_pDoc->RowHidden(SCROW(1), srcSheet, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be hidden", SCROW(1), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be hidden", SCROW(1), nRow2);
    bool bFiltered = m_pDoc->RowFiltered(SCROW(1), srcSheet, &nFilteredRow1, &nFilteredRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be filtered", bFiltered);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be filtered", SCROW(1), nFilteredRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be filtered", SCROW(1), nFilteredRow2);

    // Copy A1:A4 to clip.
    ScRange aSrcRange(0, 0, srcSheet, 0, 3, srcSheet);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aClipDoc);

    // transpose
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, true, false);

    ScRange aDestRange(1, 1, destSheet, 3, 1, destSheet); // Paste to B2:D2 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(), true,
                         false, false);
    pTransClip.reset();

    OUString aString;
    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell B2.", pFC);
    m_pDoc->GetFormula(1, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell B2", OUString("=$Sheet1.$A$1"), aString);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(2, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(2, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell C2", OUString("=$Sheet1.$A$3"), aString);
    CPPUNIT_ASSERT_EQUAL(0.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(3, 1, destSheet));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    m_pDoc->GetFormula(3, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula cell D2", OUString("=$Sheet1.$A$4"), aString);
    CPPUNIT_ASSERT_EQUAL(4.0, pFC->GetValue());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

// This method is used to create the different copy/paste special test cases.
// Principle: Creation of test cases is parameterized, whereas checking uses a minimum of logic
void TestCopyPaste::executeCopyPasteSpecial(bool bApplyFilter, bool bIncludedFiltered, bool bAsLink,
                                            bool bTranspose, bool bMultiRangeSelection,
                                            bool bSkipEmpty,
                                            ScClipParam::Direction eDirection = ScClipParam::Column,
                                            bool bCalcAll = false,
                                            InsertDeleteFlags aFlags = InsertDeleteFlags::CONTENTS
                                                                       | InsertDeleteFlags::ATTRIB)
{
    const SCTAB srcSheet = 0;
    m_pDoc->InsertTab(srcSheet, "SrcSheet");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());
    ScFieldEditEngine& rEditEngine = m_pDoc->GetEditEngine();

    /*
         | A   |    B     | C    |  D  |     E      |        F            |

    1r   | 1 B*| =A1+10  *| a    | R1 *| =A1+A3+60  | =SUMIF(A1:A4;"<4")  |
    2r   | 2 B*| =A2+20 b | b   *| R2 *|            |                    *| <- filtered row
    3r   | 3 B*| =D3+30 b*| c   *|  5 *|          B*|                     |
    4    | 4   | =A2+40 b*| d   *| R4 *| =A1+A3+70 *|    =B$1+$A$3+80    *|
   (5r   | 6   |    q     | r bB*| s bB|     t      |          u          |) optional, for row range
   (6    | -1  |    -2    |  -3  | -4  |     -5     |          -6         |) optional, for row range
   (7r   | -11 |    -12   | -13  | -14 |     -15    |          -16        |) optional, for row range
   (8    | -21 |    -22   | -23  | -24 |     -25    |          -26        |) optional, for row range

          \______________/      \________________________________________/
             col range 1                     col range 2

    * means note attached
    B means background
    b means border
    r means row selected for row range in multi range selection

    The following test scenarios can be created:

    * Filtered row
    * Transpose
    * All cell types: numbers, strings, formulas, rich text, empty cell
    * Notes at different position
    * Formula references to rows before and after filtered row
    * Double reference (e.g. A1:A3)
    * Relative and absolute references
        * absolute references are not changed by transposing
    * Formatting patterns (e.g. cell backgrounds and borders)
    * Multi range selection with direction column and row

    */
    SCCOL nSrcCols = 6;
    SCROW nSrcRows = 4;
    // Add additional row for MultiRange test cases
    if (bMultiRangeSelection)
    {
        nSrcRows = eDirection == ScClipParam::Row ? nSrcRows + 2 : nSrcRows;
        nSrcCols = eDirection == ScClipParam::Column ? nSrcCols + 1 : nSrcCols;
    }

    // col A
    m_pDoc->SetValue(0, 0, srcSheet, 1);
    m_pDoc->SetValue(0, 1, srcSheet, 2);
    m_pDoc->SetValue(0, 2, srcSheet, 3);
    m_pDoc->SetValue(0, 3, srcSheet, 4);
    // col B
    m_pDoc->SetString(1, 0, srcSheet, "=A1+10");
    m_pDoc->SetString(1, 1, srcSheet, "=A2+20");
    m_pDoc->SetString(1, 2, srcSheet, "=D3+30");
    m_pDoc->SetString(1, 3, srcSheet, "=A2+40");
    // col C
    m_pDoc->SetString(2, 0, srcSheet, "a");
    m_pDoc->SetString(2, 1, srcSheet, "b");
    m_pDoc->SetString(2, 2, srcSheet, "c");
    m_pDoc->SetString(2, 3, srcSheet, "d");
    // col D
    rEditEngine.SetTextCurrentDefaults("R1");
    m_pDoc->SetEditText(ScAddress(3, 0, srcSheet), rEditEngine.CreateTextObject());
    rEditEngine.SetTextCurrentDefaults("R2");
    m_pDoc->SetEditText(ScAddress(3, 1, srcSheet), rEditEngine.CreateTextObject());
    m_pDoc->SetValue(3, 2, srcSheet, 5);
    rEditEngine.SetTextCurrentDefaults("R4");
    m_pDoc->SetEditText(ScAddress(3, 3, srcSheet), rEditEngine.CreateTextObject());
    // col E
    m_pDoc->SetValue(4, 0, srcSheet, 9);
    m_pDoc->SetString(4, 0, srcSheet, "=A1+A3+60");
    m_pDoc->SetEmptyCell(ScAddress(4, 1, srcSheet));
    m_pDoc->SetEmptyCell(ScAddress(4, 2, srcSheet));
    m_pDoc->SetString(4, 3, srcSheet, "=A1+A3+70");
    // col F
    m_pDoc->SetValue(5, 0, srcSheet, 9);
    m_pDoc->SetString(5, 0, srcSheet, "=SUMIF(A1:A4;\"<4\")");
    m_pDoc->SetEmptyCell(ScAddress(5, 1, srcSheet));
    m_pDoc->SetEmptyCell(ScAddress(5, 2, srcSheet));
    m_pDoc->SetString(5, 3, srcSheet, "=B$1+$A$3+80");

    const SfxPoolItem* pItem = nullptr;

    // row 4, additional row for MultiRange test case, otherwise not selected
    m_pDoc->SetValue(0, 4, srcSheet, 6);
    m_pDoc->SetString(1, 4, srcSheet, "q");
    m_pDoc->SetString(2, 4, srcSheet, "r");
    m_pDoc->SetString(3, 4, srcSheet, "s");
    m_pDoc->SetString(4, 4, srcSheet, "t");
    m_pDoc->SetString(5, 4, srcSheet, "u");

    // row 5, not selected
    m_pDoc->SetValue(0, 5, srcSheet, -1);
    m_pDoc->SetValue(1, 5, srcSheet, -2);
    m_pDoc->SetValue(2, 5, srcSheet, -3);
    m_pDoc->SetValue(3, 5, srcSheet, -4);
    m_pDoc->SetValue(4, 5, srcSheet, -5);
    m_pDoc->SetValue(5, 5, srcSheet, -6);

    // row 6, additional row for MultiRange test case, otherwise not selected
    m_pDoc->SetValue(0, 6, srcSheet, -11);
    m_pDoc->SetValue(1, 6, srcSheet, -12);
    m_pDoc->SetValue(2, 6, srcSheet, -13);
    m_pDoc->SetValue(3, 6, srcSheet, -14);
    m_pDoc->SetValue(4, 6, srcSheet, -15);
    m_pDoc->SetValue(5, 6, srcSheet, -16);

    // row 7, additional row for MultiRange test case, otherwise not selected
    m_pDoc->SetValue(0, 7, srcSheet, -21);
    m_pDoc->SetValue(1, 7, srcSheet, -22);
    m_pDoc->SetValue(2, 7, srcSheet, -23);
    m_pDoc->SetValue(3, 7, srcSheet, -24);
    m_pDoc->SetValue(4, 7, srcSheet, -25);
    m_pDoc->SetValue(5, 7, srcSheet, -26);

    // Col G, not selected
    m_pDoc->SetValue(6, 0, srcSheet, 111);
    m_pDoc->SetValue(6, 1, srcSheet, 112);
    m_pDoc->SetValue(6, 2, srcSheet, 113);
    m_pDoc->SetValue(6, 3, srcSheet, 114);
    m_pDoc->SetValue(6, 4, srcSheet, 115);
    m_pDoc->SetValue(6, 5, srcSheet, 116);

    // Col H, additional col for MultiRange test case, otherwise not selected
    m_pDoc->SetValue(7, 0, srcSheet, 121);
    m_pDoc->SetValue(7, 1, srcSheet, 122);
    m_pDoc->SetValue(7, 2, srcSheet, 123);
    m_pDoc->SetValue(7, 3, srcSheet, 124);
    m_pDoc->SetValue(7, 4, srcSheet, 125);
    m_pDoc->SetValue(7, 5, srcSheet, 126);

    // Col J, not selected
    m_pDoc->SetValue(8, 0, srcSheet, 131);
    m_pDoc->SetValue(8, 1, srcSheet, 132);
    m_pDoc->SetValue(8, 2, srcSheet, 133);
    m_pDoc->SetValue(8, 3, srcSheet, 134);
    m_pDoc->SetValue(8, 4, srcSheet, 135);
    m_pDoc->SetValue(8, 5, srcSheet, 136);

    // add patterns
    ScPatternAttr aCellBlueColor(m_pDoc->GetPool());
    aCellBlueColor.GetItemSet().Put(SvxBrushItem(COL_BLUE, ATTR_BACKGROUND));
    m_pDoc->ApplyPatternAreaTab(0, 0, 0, 2, srcSheet, aCellBlueColor);

    // Check pattern precondition
    m_pDoc->GetPattern(ScAddress(0, 0, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("SrcSheet.A1 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(0, 1, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("SrcSheet.A2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(0, 3, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("SrcSheet.A4 has no pattern", !pItem);

    // row 2 on empty cell
    ScPatternAttr aCellGreenColor(m_pDoc->GetPool());
    aCellGreenColor.GetItemSet().Put(SvxBrushItem(COL_GREEN, ATTR_BACKGROUND));
    m_pDoc->ApplyPatternAreaTab(4, 2, 4, 2, srcSheet, aCellGreenColor);

    // row 4 for multi range row selection
    ScPatternAttr aCellRedColor(m_pDoc->GetPool());
    aCellRedColor.GetItemSet().Put(SvxBrushItem(COL_RED, ATTR_BACKGROUND));
    m_pDoc->ApplyPatternAreaTab(2, 4, 3, 4, srcSheet, aCellRedColor);

    // add borders
    ::editeng::SvxBorderLine aLine(nullptr, 50, SvxBorderLineStyle::SOLID);
    SvxBoxItem aBorderItem(ATTR_BORDER);
    aBorderItem.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBorderItem.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    m_pDoc->ApplyAttr(1, 1, srcSheet, aBorderItem);
    m_pDoc->ApplyAttr(1, 2, srcSheet, aBorderItem);
    m_pDoc->ApplyAttr(1, 3, srcSheet, aBorderItem);
    // Check border precondition
    pItem = m_pDoc->GetAttr(ScAddress(1, 0, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("SrcSheet.B1 has a border", pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    pItem = m_pDoc->GetAttr(ScAddress(1, 1, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("SrcSheet.B2 has a border", pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    // Check border precondition 2
    m_pDoc->GetPattern(ScAddress(1, 1, srcSheet))->GetItemSet().HasItem(ATTR_BORDER, &pItem);
    CPPUNIT_ASSERT_MESSAGE("SrcSheet.B2 has a border", pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    // row 4 for multi range row selection
    ::editeng::SvxBorderLine aDoubleLine(nullptr, 50, SvxBorderLineStyle::DOUBLE);
    SvxBoxItem aDoubleBorderItem(ATTR_BORDER);
    aDoubleBorderItem.SetLine(&aDoubleLine, SvxBoxItemLine::TOP);
    aDoubleBorderItem.SetLine(&aDoubleLine, SvxBoxItemLine::BOTTOM);
    m_pDoc->ApplyAttr(2, 4, srcSheet, aDoubleBorderItem);
    m_pDoc->ApplyAttr(3, 4, srcSheet, aDoubleBorderItem);

    // add notes to A1:F4

    // add notes row 0
    ScAddress aAdrA1(0, 0, srcSheet);
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, "Hello world in A1");
    ScAddress aAdrB1(1, 0, srcSheet);
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAdrB1);
    pNoteB1->SetText(aAdrB1, "Hello world in B1");
    // No note on C1
    ScAddress aAdrD1(3, 0, srcSheet);
    ScPostIt* pNoteD1 = m_pDoc->GetOrCreateNote(aAdrD1);
    pNoteD1->SetText(aAdrD1, "Hello world in D1");
    // No note on E1
    // No note on F1

    // add notes row 1
    ScAddress aAdrA2(0, 1, srcSheet);
    ScPostIt* pNoteA2 = m_pDoc->GetOrCreateNote(aAdrA2);
    pNoteA2->SetText(aAdrA2, "Hello world in A2");
    // No note on B2
    ScAddress aAdrC2(2, 1, srcSheet);
    ScPostIt* pNoteC2 = m_pDoc->GetOrCreateNote(aAdrC2);
    pNoteC2->SetText(aAdrC2, "Hello world in C2");
    ScAddress aAdrD2(3, 1, srcSheet);
    ScPostIt* pNoteD2 = m_pDoc->GetOrCreateNote(aAdrD2);
    pNoteD2->SetText(aAdrD2, "Hello world in D2");
    ScAddress aAdrE2(4, 2, srcSheet);
    ScPostIt* pNoteE2 = m_pDoc->GetOrCreateNote(aAdrE2);
    pNoteE2->SetText(aAdrE2, "Hello world in E2");
    ScAddress aAdrF2(5, 1, srcSheet);
    ScPostIt* pNoteF2 = m_pDoc->GetOrCreateNote(aAdrF2);
    pNoteF2->SetText(aAdrF2, "Hello world in F2");

    // add notes row 2
    ScAddress aAdrA3(0, 2, srcSheet);
    ScPostIt* pNoteA3 = m_pDoc->GetOrCreateNote(aAdrA3);
    pNoteA3->SetText(aAdrA3, "Hello world in A3");
    ScAddress aAdrB3(1, 2, srcSheet);
    ScPostIt* pNoteB3 = m_pDoc->GetOrCreateNote(aAdrB3);
    pNoteB3->SetText(aAdrB3, "Hello world in B3");
    ScAddress aAdrC3(2, 2, srcSheet);
    ScPostIt* pNoteC3 = m_pDoc->GetOrCreateNote(aAdrC3);
    pNoteC3->SetText(aAdrC3, "Hello world in C3");
    ScAddress aAdrD3(3, 2, srcSheet);
    ScPostIt* pNoteD3 = m_pDoc->GetOrCreateNote(aAdrD3);
    pNoteD3->SetText(aAdrD3, "Hello world in D3");
    // No note on E3
    // No note on F3

    // add notes row 3
    // No note on A4
    ScAddress aAdrB4(1, 3, srcSheet);
    ScPostIt* pNoteB4 = m_pDoc->GetOrCreateNote(aAdrB4);
    pNoteB4->SetText(aAdrB4, "Hello world in B4");
    ScAddress aAdrC4(2, 3, srcSheet);
    ScPostIt* pNoteC4 = m_pDoc->GetOrCreateNote(aAdrC4);
    pNoteC4->SetText(aAdrC4, "Hello world in C4");
    ScAddress aAdrD4(3, 3, srcSheet);
    ScPostIt* pNoteD4 = m_pDoc->GetOrCreateNote(aAdrD4);
    pNoteD4->SetText(aAdrD4, "Hello world in D4");
    ScAddress aAdrE4(4, 3, srcSheet);
    ScPostIt* pNoteE4 = m_pDoc->GetOrCreateNote(aAdrE4);
    pNoteE4->SetText(aAdrE4, "Hello world in E4");
    ScAddress aAdrF4(5, 3, srcSheet);
    ScPostIt* pNoteF4 = m_pDoc->GetOrCreateNote(aAdrF4);
    pNoteF4->SetText(aAdrF4, "Hello world in F4");

    // row 4 for multi range row selection
    ScAddress aAdrC5(2, 4, srcSheet);
    ScPostIt* pNoteC5 = m_pDoc->GetOrCreateNote(aAdrC5);
    pNoteC5->SetText(aAdrC5, "Hello world in C5");

    // Filter out row 1
    if (bApplyFilter)
    {
        ScDBData* pDBData
            = new ScDBData("TRANSPOSE_TEST_DATA", srcSheet, 0, 0, nSrcCols - 1, nSrcRows - 1);
        m_pDoc->SetAnonymousDBData(srcSheet, std::unique_ptr<ScDBData>(pDBData));

        pDBData->SetAutoFilter(true);
        ScRange aRange;
        pDBData->GetArea(aRange);
        m_pDoc->ApplyFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                              aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);

        //create the query param
        ScQueryParam aParam;
        pDBData->GetQueryParam(aParam);
        ScQueryEntry& rEntry = aParam.GetEntry(0);
        rEntry.bDoQuery = true;
        rEntry.nField = 0;
        rEntry.eOp = SC_NOT_EQUAL;
        rEntry.GetQueryItem().mfVal = 2; // value of row A2 -> filtering row 1
        // add queryParam to database range.
        pDBData->SetQueryParam(aParam);

        // perform the query.
        m_pDoc->Query(srcSheet, aParam, true);

        // Check precondition for test: row 1 is hidden/filtered
        SCROW nRow1, nRow2;
        SCROW nFilteredRow1, nFilteredRow2;
        bool bHidden = m_pDoc->RowHidden(SCROW(1), srcSheet, &nRow1, &nRow2);
        CPPUNIT_ASSERT_MESSAGE("row 1 should be hidden", bHidden);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be hidden", SCROW(1), nRow1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be hidden", SCROW(1), nRow2);
        bool bFiltered = m_pDoc->RowFiltered(SCROW(1), srcSheet, &nFilteredRow1, &nFilteredRow2);
        CPPUNIT_ASSERT_MESSAGE("row 1 should be filtered", bFiltered);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be filtered", SCROW(1), nFilteredRow1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be filtered", SCROW(1), nFilteredRow2);
    }

    // create destination sheet
    const SCTAB destSheet = 1;
    m_pDoc->InsertTab(destSheet, "DestSheet");
    // set cells to 1000 to check empty cell behaviour and to detect destination range problems
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            m_pDoc->SetValue(i, j, destSheet, 1000);

    // transpose clipboard, paste on DestSheet
    ScDocument aNewClipDoc(SCDOCMODE_CLIP);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());
    if (!bMultiRangeSelection)
    {
        ScRange aSrcRange(0, 0, srcSheet, nSrcCols - 1, nSrcRows - 1, srcSheet);
        copyToClip(m_pDoc, aSrcRange, &aNewClipDoc);

        // ScDocument::TransposeClip() and ScDocument::CopyFromClip() calls
        // analog to ScViewFunc::PasteFromClip()
        if (bTranspose)
        {
            ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
            aNewClipDoc.TransposeClip(pTransClip.get(), aFlags, bAsLink, bIncludedFiltered);
            ScRange aDestRange(3, 1, destSheet, 3 + nSrcRows - 1, 1 + nSrcCols - 1,
                               destSheet); //target: D2:F6
            aDestMark.SetMarkArea(aDestRange);
            m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, nullptr, pTransClip.get(), true,
                                 bAsLink, bIncludedFiltered, bSkipEmpty);
            pTransClip.reset();
        }
        else
        {
            ScRange aDestRange(3, 1, destSheet, 3 + nSrcCols - 1, 1 + nSrcRows - 1,
                               destSheet); //target: D2:I5
            aDestMark.SetMarkArea(aDestRange);
            m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, nullptr, &aNewClipDoc, true,
                                 bAsLink, bIncludedFiltered, bSkipEmpty);
        }
    }
    else
    {
        ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
        aSrcMark.SelectOneTable(0);
        ScClipParam aClipParam;
        aClipParam.meDirection = eDirection;
        if (eDirection == ScClipParam::Column)
        {
            aClipParam.maRanges.push_back(ScRange(0, 0, srcSheet, 1, 3, srcSheet)); // A1:B4
            aClipParam.maRanges.push_back(ScRange(3, 0, srcSheet, 5, 3, srcSheet)); // D1:F4
            aClipParam.maRanges.push_back(ScRange(7, 0, srcSheet, 7, 3, srcSheet)); // H1:H4
        }
        else if (eDirection == ScClipParam::Row)
        {
            aClipParam.maRanges.push_back(ScRange(0, 0, srcSheet, 5, 2, srcSheet)); // A1:F3
            aClipParam.maRanges.push_back(ScRange(0, 4, srcSheet, 5, 4, srcSheet)); // A5:F5
            aClipParam.maRanges.push_back(ScRange(0, 6, srcSheet, 5, 6, srcSheet)); // A7:F7
        }
        CPPUNIT_ASSERT(aClipParam.isMultiRange());
        m_pDoc->CopyToClip(aClipParam, &aNewClipDoc, &aSrcMark, false, false);

        // ScDocument::TransposeClip() and ScDocument::CopyMultiRangeFromClip() calls
        // analog to ScViewFunc::PasteFromClipToMultiRanges()
        if (bTranspose)
        {
            printRange(m_pDoc, aClipParam.getWholeRange(), "Src range");
            ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
            aNewClipDoc.TransposeClip(pTransClip.get(), aFlags, bAsLink, bIncludedFiltered);
            ScRange aDestRange(3, 1, destSheet, 3 + nSrcRows - 1, 1 + nSrcCols - 1 - 1,
                               destSheet); //target col: D2:G6, target row: D2:H6
            aDestMark.SetMarkArea(aDestRange);
            printRange(&aNewClipDoc, ScRange(0, 0, 0, nSrcCols, nSrcRows, 0),
                       "Base doc (&aNewClipDoc)");
            printRange(pTransClip.get(), ScRange(0, 0, 0, nSrcCols, nSrcRows, 0),
                       "Transposed filtered clipdoc (pTransClip.get())");
            m_pDoc->CopyMultiRangeFromClip(ScAddress(3, 1, destSheet), aDestMark, aFlags,
                                           pTransClip.get(), true, bAsLink && !bTranspose,
                                           bIncludedFiltered, bSkipEmpty);
            pTransClip.reset();
            printRange(m_pDoc, aDestRange, "Transposed dest sheet");
        }
        else
        {
            ScRange aDestRange(3, 1, destSheet, 3 + nSrcCols - 1 - 1, 1 + nSrcRows - 1,
                               destSheet); //target col: D2:I5, target row: D2:I6
            aDestMark.SetMarkArea(aDestRange);
            m_pDoc->CopyMultiRangeFromClip(ScAddress(3, 1, destSheet), aDestMark, aFlags,
                                           &aNewClipDoc, true, bAsLink && !bTranspose,
                                           bIncludedFiltered, bSkipEmpty);
        }
    }
    if (bCalcAll)
        m_pDoc->CalcAll();
}

void TestCopyPaste::testCopyPasteSpecial()
{
    executeCopyPasteSpecial(false, false, false, false, false, false);
    checkCopyPasteSpecial(false);
}

void TestCopyPaste::testCopyPasteSpecialFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, false, false);
    checkCopyPasteSpecialFiltered(false);
}

void TestCopyPaste::testCopyPasteSpecialIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, false, false);
    checkCopyPasteSpecial(false);
}

void TestCopyPaste::testCopyPasteSpecialFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, false, false);
    checkCopyPasteSpecial(false);
}

// similar to TestCopyPaste::testCopyPasteTranspose(), but this test is more complex
void TestCopyPaste::testCopyPasteSpecialTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, false, false);
    checkCopyPasteSpecialTranspose(false);
}

// tdf#107348
void TestCopyPaste::testCopyPasteSpecialFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, false, false);
    checkCopyPasteSpecialFilteredTranspose(false);
}

// tdf#107348
void TestCopyPaste::testCopyPasteSpecialTransposeIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, false, false);
    checkCopyPasteSpecialTranspose(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeCol()
{
    executeCopyPasteSpecial(false, false, false, false, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeColIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFiltered(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeColTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFilteredTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeRow()
{
    executeCopyPasteSpecial(false, false, false, false, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeRowIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRowFiltered(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeRowTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, false, ScClipParam::Row, true);
    checkCopyPasteSpecialMultiRangeRowTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, true, false, ScClipParam::Row, true);
    checkCopyPasteSpecialMultiRangeRowFilteredTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, false, ScClipParam::Row, true);
    checkCopyPasteSpecialMultiRangeRowTranspose(false);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmpty()
{
    executeCopyPasteSpecial(false, false, false, false, false, true);
    checkCopyPasteSpecial(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, false, true);
    checkCopyPasteSpecialFiltered(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, false, true);
    checkCopyPasteSpecial(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, false, true);
    checkCopyPasteSpecial(true);
}

// similar to TestCopyPaste::testCopyPasteTranspose(), but this test is more complex
void TestCopyPaste::testCopyPasteSpecialSkipEmptyTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, false, true);
    checkCopyPasteSpecialTranspose(true);
}

// tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, false, true);
    checkCopyPasteSpecialFilteredTranspose(true);
}

// tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyTransposeIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, false, true);
    checkCopyPasteSpecialTranspose(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeCol()
{
    executeCopyPasteSpecial(false, false, false, false, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFiltered(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, false, false, true, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFilteredTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, true, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRow()
{
    executeCopyPasteSpecial(false, false, false, false, true, true, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, true, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, true, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRowFiltered(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, true, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, true, ScClipParam::Row, true);
    checkCopyPasteSpecialMultiRangeRowTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, true, true, ScClipParam::Row, true);
    checkCopyPasteSpecialMultiRangeRowFilteredTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, true, ScClipParam::Row, true);
    checkCopyPasteSpecialMultiRangeRowTranspose(true);
}

// Base check, nothing filtered, nothing transposed
void TestCopyPaste::checkCopyPasteSpecial(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
         |  D  |    E     | F  |  G  |     H      |        I            |

    2    | 1 B*| =D2+10  *| a  | R1 *| =D2+D4+60  | =SUMIF(D2:D5;"<4")  |
    3    | 2 B*| =D3+20 b | b *| R2 *|            |                    *|
    4    | 3 B*| =G4+30 b*| c *|  5 *|          B*|                     |
    5    | 4   | =D3+40 b*| d *| R4 *| =D2+D4+70 *|    =E$1+$A$3+80    *|

    * means note attached
    B means background
    b means border
    */

    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // col 2
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    // col 3, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(2, m_pDoc->GetValue(3, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(3, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(3, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 5, destSheet));
    // col 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 1, destSheet));
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D3+20"), aString);
    ASSERT_DOUBLES_EQUAL(22, m_pDoc->GetValue(4, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(35, m_pDoc->GetValue(4, 3, destSheet));
    m_pDoc->GetFormula(4, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=G4+30"), aString);
    ASSERT_DOUBLES_EQUAL(42, m_pDoc->GetValue(4, 4, destSheet));
    m_pDoc->GetFormula(4, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D3+40"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    // col 5, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    aString = m_pDoc->GetString(5, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(5, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aString);
    aString = m_pDoc->GetString(5, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aString);
    aString = m_pDoc->GetString(5, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    aString = m_pDoc->GetString(5, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 6, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 0, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 1, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 2, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R2"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(6, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 4, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 5, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 5, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // col 7, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    aString = m_pDoc->GetString(7, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(7, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+D4+60"), aString);
    ASSERT_DOUBLES_EQUAL(64, m_pDoc->GetValue(7, 1, destSheet));
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(7, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(7, 3, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    }
    fValue = m_pDoc->GetValue(7, 4, destSheet);
    m_pDoc->GetFormula(7, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+D4+70"), aString);
    ASSERT_DOUBLES_EQUAL(74, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 8, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 0, destSheet));
    aString = m_pDoc->GetString(8, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(8, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D2:D5;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(6, m_pDoc->GetValue(8, 1, destSheet));
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(8, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(8, 3, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 2, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 3, destSheet));
    }
    fValue = m_pDoc->GetValue(8, 4, destSheet);
    m_pDoc->GetFormula(8, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=E$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL(2080, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 5, destSheet));
    aString = m_pDoc->GetString(8, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 9, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 5, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(3, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(7, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(4, 1, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 5, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(8, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(8, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 5, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(7, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(8, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(8, 4, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialFiltered(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
         |  D  |    E     | F  |  G  |     H      |        I            |

    2    | 1 B*| =D2+10  *| a  | R1 *| =D2+D4+60  | =SUMIF(D2:D5;"<4")  |
    3    | 3 B*| =G3+30 b*| c *|  5 *|          B*|                     |
    4    | 4   | =D2+40 b*| d *| R4 *| =D1+D3+70 *|    =E$1+$A$3+80    *|
    5    | 1 B*| =D5+10  *| a  | R1 *| =D5+D7+60  | =SUMIF(D5:D8;"<4")  |   <- repeated row

    * means note attached
    B means background
    b means border
    */

    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;

    // col 2
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    // col 3, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(3, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(3, 3, destSheet));
    fValue = m_pDoc->GetValue(3, 4, destSheet); // repeated row 1
    ASSERT_DOUBLES_EQUAL(1, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 5, destSheet));
    // col 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    m_pDoc->GetFormula(4, 0, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 1, destSheet));
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=G3+30"), aString);
    ASSERT_DOUBLES_EQUAL(35, fValue);
    fValue = m_pDoc->GetValue(4, 3, destSheet);
    m_pDoc->GetFormula(4, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+40"), aString);
    ASSERT_DOUBLES_EQUAL(41, fValue);
    fValue = m_pDoc->GetValue(4, 4, destSheet); // repeated row 1
    ASSERT_DOUBLES_EQUAL(11, fValue);
    m_pDoc->GetFormula(4, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D5+10"), aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    // col 5, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    aString = m_pDoc->GetString(5, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aString);
    aString = m_pDoc->GetString(5, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), aString);
    aString = m_pDoc->GetString(5, 4, destSheet); // repeated row 1
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    m_pDoc->GetFormula(4, 5, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    // col 6, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 0, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 1, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(6, 2, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 4, destSheet)); // repeated row 1
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 5, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 5, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // col 7, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    aString = m_pDoc->GetString(7, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(7, 1, destSheet, aString);
    fValue = m_pDoc->GetValue(7, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+D4+60"), aString);
    ASSERT_DOUBLES_EQUAL(65, fValue); // formula is not adjusted due to filter row
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(7, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    fValue = m_pDoc->GetValue(7, 3, destSheet);
    m_pDoc->GetFormula(7, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D1+D3+70"), aString);
    ASSERT_DOUBLES_EQUAL(1073, fValue);
    m_pDoc->GetFormula(7, 4, destSheet, aString); // repeated row 1
    fValue = m_pDoc->GetValue(7, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("=D5+D7+60"), aString);
    ASSERT_DOUBLES_EQUAL(1061, fValue); // formula is not adjusted due to filter row
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 8, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 0, destSheet));
    aString = m_pDoc->GetString(8, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(8, 1, destSheet, aString);
    fValue = m_pDoc->GetValue(8, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D2:D5;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(5, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(8, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 2, destSheet));
    fValue = m_pDoc->GetValue(8, 3, destSheet);
    m_pDoc->GetFormula(8, 3, destSheet, aString);
    ASSERT_DOUBLES_EQUAL(2080, fValue);
    CPPUNIT_ASSERT_EQUAL(OUString("=E$1+$A$3+80"), aString);
    m_pDoc->GetFormula(8, 4, destSheet, aString); // repeated row 1
    fValue = m_pDoc->GetValue(8, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D5:D8;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(1, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 5, destSheet));
    aString = m_pDoc->GetString(8, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 9, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 5, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(3, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(7, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(4, 1, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(8, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 5, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(7, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(8, 3, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialTranspose(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
             |         D          |    E     |    F     |       G       |

        2    |         1        B*|    2   B*|    3   B*|       4       |
        3    |      =D2+10       *| =E2+20 b | =F5+30 b*|    =E2+40   b*|
        4    |         a          |    b    *|    c    *|       d      *|
        5    |        R1         *|   R2    *|    5    *|      R4      *|
        6    |     =D2+F2+60      |          |        B*|  =D2+F2+70   *|
        7    | =SUMIF(D2:G2;"<4") |         *|          | =C$1+$A$1+80 *|

        * means note attached
        B means background
        b means border
    */

    //check cell content after transposed copy/paste of filtered data
    // Note: column F is a repetition of srcSheet.Column A
    // Col C and G are checked to be empty
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // row 0
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    // row 1, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    fValue = m_pDoc->GetValue(3, 1, destSheet); // D2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell D2", 1, fValue);
    fValue = m_pDoc->GetValue(4, 1, destSheet); // E2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell E2", 2, fValue);
    fValue = m_pDoc->GetValue(5, 1, destSheet); // F2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell F2", 3, fValue);
    fValue = m_pDoc->GetValue(6, 1, destSheet); // G2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell G2", 4, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 1, destSheet));
    // row 2, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    aString = m_pDoc->GetString(2, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 2, destSheet, aString); // D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D3 should point on D2", OUString("=D2+10"),
                                 aString);
    fValue = m_pDoc->GetValue(3, 2, destSheet); // D3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D3", 11, fValue);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula E3 should point on E2", OUString("=E2+20"),
                                 aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet); // E3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula E3", 22, fValue);
    m_pDoc->GetFormula(5, 2, destSheet, aString); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula F3 should point on F2", OUString("=F5+30"),
                                 aString);
    fValue = m_pDoc->GetValue(5, 2, destSheet); // F3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula F3", 35, fValue);
    m_pDoc->GetFormula(6, 2, destSheet, aString); // G3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G3 should point on E2", OUString("=E2+40"),
                                 aString);
    fValue = m_pDoc->GetValue(6, 2, destSheet); // G3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula G3", 42, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    aString = m_pDoc->GetString(7, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 3, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    aString = m_pDoc->GetString(2, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(3, 3, destSheet); // D4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell D4 should contain: a", OUString("a"), aString);
    aString = m_pDoc->GetString(4, 3, destSheet); // E4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E4 should contain: b", OUString("b"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet); // F4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F4 should contain: c", OUString("c"), aString);
    aString = m_pDoc->GetString(6, 3, destSheet); // G4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G4 should contain: d", OUString("d"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    aString = m_pDoc->GetString(7, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 4, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(2, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in C5.", pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in D5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong in D5 ", OUString("R1"),
                                 pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(4, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in E5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong E5.", OUString("R2"), pEditObj->GetText(0));
    fValue = m_pDoc->GetValue(5, 4, destSheet); // F5
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell F5", 5, fValue);
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in G5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong G5.", OUString("R4"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(7, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in H5.", pEditObj == nullptr);
    // row 5, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 5, destSheet, aString); // D6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D6", OUString("=D2+F2+60"), aString);
    fValue = m_pDoc->GetValue(3, 5, destSheet); // D6
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D6", 64, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 5, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(5, 5, destSheet); // F6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    }
    fValue = m_pDoc->GetValue(6, 5, destSheet); // G6
    m_pDoc->GetFormula(6, 5, destSheet, aString); // G6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G6", OUString("=D2+F2+70"), aString);
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula G6", 74, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 6, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 6, destSheet));
    aString = m_pDoc->GetString(2, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 6, destSheet, aString); // D7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D7", OUString("=SUMIF(D2:G2;\"<4\")"),
                                 aString);
    fValue = m_pDoc->GetValue(3, 6, destSheet); // D7
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D7", 6, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 6, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(5, 6, destSheet); // F6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 6, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 6, destSheet));
    }
    fValue = m_pDoc->GetValue(6, 6, destSheet); // G7
    m_pDoc->GetFormula(6, 6, destSheet, aString); // G7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G7", OUString("=C$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula G7", 2080, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 6, destSheet));
    aString = m_pDoc->GetString(7, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 7
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("D2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("D2 has blue background", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(4, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("E2 has blue background", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("F2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("F2 has a pattern", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(6, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("G2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(7, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("H2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(5, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(3, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("D3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("D3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("D3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("D3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("D3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());

    m_pDoc->GetPattern(ScAddress(4, 2, destSheet))->GetItemSet().HasItem(ATTR_BORDER, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());

    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());

    CPPUNIT_ASSERT_MESSAGE("E3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("E3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("E3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("F3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("F3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("F3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("F3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("F3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("G3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("G3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("G3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("G3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(7, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("H3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("H3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("H3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("H3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("H3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C1",
                           !m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D1",
                           !m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E1",
                           !m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F1",
                           !m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G1",
                           !m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H1",
                           !m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C2",
                           !m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D2",
                           m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E2",
                           m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F2",
                           m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G2",
                           !m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H2",
                           !m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C3",
                           !m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D3",
                           m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E3",
                           !m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F3",
                           m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G3",
                           m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H3",
                           !m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C4",
                           !m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D4",
                           !m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E4",
                           m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F4",
                           m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G4",
                           m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H4",
                           !m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C5",
                           !m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D5",
                           m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E5",
                           m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F5",
                           m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G5",
                           m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H5",
                           !m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C6",
                           !m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D6",
                           !m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E6",
                           !m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G6",
                           m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H6",
                           !m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C7",
                           !m_pDoc->HasNote(ScAddress(2, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D7",
                           !m_pDoc->HasNote(ScAddress(3, 6, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(4, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F7",
                           !m_pDoc->HasNote(ScAddress(5, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note  a G7",
                           m_pDoc->HasNote(ScAddress(6, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H7",
                           !m_pDoc->HasNote(ScAddress(7, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C8",
                           !m_pDoc->HasNote(ScAddress(2, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D8",
                           !m_pDoc->HasNote(ScAddress(3, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E8",
                           !m_pDoc->HasNote(ScAddress(4, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F8",
                           !m_pDoc->HasNote(ScAddress(5, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G8",
                           !m_pDoc->HasNote(ScAddress(6, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H8",
                           !m_pDoc->HasNote(ScAddress(7, 7, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2",
                                 m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2",
                                 m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F2",
                                 m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    // G2 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3",
                                 m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    // E3 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F3",
                                 m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    // D4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4",
                                 m_pDoc->GetNote(ScAddress(2, 1, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F4",
                                 m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5",
                                 m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5",
                                 m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F5",
                                 m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on G6",
                                 m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(6, 5, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E7",
                                     m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText(),
                                     m_pDoc->GetNote(ScAddress(4, 6, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on G7",
                                 m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(6, 6, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialFilteredTranspose(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
                                  ┌--- filtered src row 2          ┌--- repeated row
                                  v                                v

             |     D              |    E     |      F        |     G              |

        2    |     1            B*|   3    B*|      4        |     1            B*|
        3    |  =D2+10           *| =E5+30 b*| =D2+40      b*|  =G2+10           *|
        4    |     a              |    c    *|      d       *|     a              |
        5    |    R1             *|    5    *|     R4       *|    R1             *|
        6    | =D2+F2+60          |        B*| =C2+E2+70    *| =G2+I2+60          |
        7    | =SUMIF(D2:G2;"<4") |          | =B$1+$A$1+80 *| =SUMIF(G2:J2;"<4") |

        * means note attached
    */

    //check cell content after transposed copy/paste of filtered data
    // Note: column F is a repetition of srcSheet.Column A
    // Col C and G are checked to be empty
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // row 0
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    // row 1, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    aString = m_pDoc->GetString(2, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    fValue = m_pDoc->GetValue(3, 1, destSheet); // D2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell D2", 1, fValue);
    fValue = m_pDoc->GetValue(4, 1, destSheet); // E2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell E2", 3, fValue);
    fValue = m_pDoc->GetValue(5, 1, destSheet); // F2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell F2", 4, fValue);
    fValue = m_pDoc->GetValue(6, 1, destSheet); // G2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell G2 (repetition of D2)", 1, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 1, destSheet));
    aString = m_pDoc->GetString(7, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 2, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    aString = m_pDoc->GetString(2, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    fValue = m_pDoc->GetValue(3, 2, destSheet); // D3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D3", 11, fValue);
    m_pDoc->GetFormula(3, 2, destSheet, aString); // D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D3 should point on D2", OUString("=D2+10"),
                                 aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula E3 should point on E2", OUString("=E5+30"),
                                 aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet); // E3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula E3", 35, fValue);
    m_pDoc->GetFormula(5, 2, destSheet, aString); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula F3 should point on F2", OUString("=D2+40"),
                                 aString);
    fValue = m_pDoc->GetValue(5, 2, destSheet); // F3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula F3", 41, fValue);
    fValue = m_pDoc->GetValue(6, 2, destSheet); // G3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula F3", 11, fValue);
    m_pDoc->GetFormula(6, 2, destSheet, aString); // G3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G3 should point on G2 (repetition of D3)",
                                 OUString("=G2+10"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    aString = m_pDoc->GetString(7, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 3, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    aString = m_pDoc->GetString(2, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(3, 3, destSheet); // D4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell D4 should contain: a", OUString("a"), aString);
    aString = m_pDoc->GetString(4, 3, destSheet); // E4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E4 should contain: c", OUString("c"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet); // F4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F4 should contain: d", OUString("d"), aString);
    aString = m_pDoc->GetString(6, 3, destSheet); // G4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G4 should contain: a (repetition of D4)", OUString("a"),
                                 aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    aString = m_pDoc->GetString(7, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 4, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(2, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in C5.", pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in D5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong in D5 ", OUString("R1"),
                                 pEditObj->GetText(0));
    fValue = m_pDoc->GetValue(4, 4, destSheet); // E5
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell E5", 5, fValue);
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in F5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong F5.", OUString("R4"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in G5. (repetition of D5)", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong G5. (repetition of D5)", OUString("R1"),
                                 pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(7, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in H5.", pEditObj == nullptr);
    // row 5, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 5, destSheet, aString); // D6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D6", OUString("=D2+F2+60"), aString);
    fValue = m_pDoc->GetValue(
        ScAddress(3, 5, destSheet)); // D6, formulas over filtered rows are not adjusted
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D6", 65,
                                 fValue); // formulas over filtered rows are not adjusted
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 5, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    m_pDoc->GetFormula(5, 5, destSheet, aString); // F6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula F6", OUString("=C2+E2+70"), aString);
    fValue = m_pDoc->GetValue(
        ScAddress(5, 5, destSheet)); // F6,  formulas over filtered rows are not adjusted
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula F6", 1073, fValue);
    m_pDoc->GetFormula(6, 5, destSheet, aString); // G6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G6 (repetition of D6)", OUString("=G2+I2+60"),
                                 aString);
    fValue = m_pDoc->GetValue(6, 5, destSheet); // G6
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula G6 (repetition of D6)", 1061, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 6, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 6, destSheet));
    aString = m_pDoc->GetString(2, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 6, destSheet, aString); // D7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D7", OUString("=SUMIF(D2:G2;\"<4\")"),
                                 aString);
    fValue = m_pDoc->GetValue(3, 6, destSheet); // D7
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D7", 5, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 6, destSheet); // E7
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 6, destSheet));
    m_pDoc->GetFormula(5, 6, destSheet, aString); // F7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula F7", OUString("=B$1+$A$3+80"), aString);
    fValue = m_pDoc->GetValue(5, 6, destSheet); // F7
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula F6", 2080, fValue);
    m_pDoc->GetFormula(6, 6, destSheet, aString); // G7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G7 (repetition of D7)",
                                 OUString("=SUMIF(G2:J2;\"<4\")"), aString);
    fValue = m_pDoc->GetValue(6, 5, destSheet); // G7
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula G7 (repetition of D7)", 1061, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 6, destSheet));
    aString = m_pDoc->GetString(7, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);

    // row
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));

    // check patterns

    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("D2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("D2 has blue background", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(4, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("F2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(6, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("G2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("G2 has a pattern", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(7, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("H2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(4, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(3, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("D3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("D3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("D3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("D3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("D3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("E3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("E3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("E3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("F3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("F3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("F3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("F3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("F3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("G3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("G3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("G3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("G3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("G3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(7, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("H3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("H3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("H3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("H3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("H3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C1",
                           !m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D1",
                           !m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E1",
                           !m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F1",
                           !m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G1",
                           !m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H1",
                           !m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C2",
                           !m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D2",
                           m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E2",
                           m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F2",
                           !m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G2",
                           m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H2",
                           !m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C3",
                           !m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D3",
                           m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E3",
                           m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F3",
                           m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G3",
                           m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H3",
                           !m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C4",
                           !m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D4",
                           !m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E4",
                           m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F4",
                           m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G4",
                           !m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H4",
                           !m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C5",
                           !m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D5",
                           m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E5",
                           m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F5",
                           m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G5",
                           m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H5",
                           !m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C6",
                           !m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D6",
                           !m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F6",
                           m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G6",
                           !m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H6",
                           !m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C7",
                           !m_pDoc->HasNote(ScAddress(2, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D7",
                           !m_pDoc->HasNote(ScAddress(3, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E7",
                           !m_pDoc->HasNote(ScAddress(4, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note  a F7",
                           m_pDoc->HasNote(ScAddress(5, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G7",
                           !m_pDoc->HasNote(ScAddress(6, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H7",
                           !m_pDoc->HasNote(ScAddress(7, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C8",
                           !m_pDoc->HasNote(ScAddress(2, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D8",
                           !m_pDoc->HasNote(ScAddress(3, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E8",
                           !m_pDoc->HasNote(ScAddress(4, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F8",
                           !m_pDoc->HasNote(ScAddress(5, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G8",
                           !m_pDoc->HasNote(ScAddress(6, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H8",
                           !m_pDoc->HasNote(ScAddress(7, 7, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2",
                                 m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2",
                                 m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    // F2 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on G2",
                                 m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3",
                                 m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E3",
                                 m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F3",
                                 m_pDoc->GetNote(ScAddress(1, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on G3",
                                 m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    // D4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4",
                                 m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F4",
                                 m_pDoc->GetNote(ScAddress(2, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    // G4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5",
                                 m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5",
                                 m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F5",
                                 m_pDoc->GetNote(ScAddress(3, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on G5",
                                 m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(6, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(4, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F6",
                                 m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F7",
                                 m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 6, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeCol(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
                          ┌--- not selected src col C
                          v

         |  D  |    E     |  F  |     G      |        H            |  I  |

    2    | 1 B*| =D2+10  *| R1 *| =C2+C4+60  | =SUMIF(C2:C5;"<4")  | 121 |
    3    | 2 B*| =D3+20 b | R2 *|            |                    *| 122 | <- filtered row
    4    | 3 B*| =G4+30 b*|  5 *|          B*|                     | 123 |
    5    | 4   | =D3+40 b*| R4 *| =C2+C4+70 *|    =D$1+$A$1+80    *| 124 |

    * means note attached
    B means background
    b means border
    */

    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // col 2
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    // col 3, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(2, m_pDoc->GetValue(3, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(3, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(3, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 5, destSheet));
    // col 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    aString = m_pDoc->GetString(4, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 1, destSheet));
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D3+20"), aString);
    ASSERT_DOUBLES_EQUAL(22, m_pDoc->GetValue(4, 2, destSheet));
    fValue = m_pDoc->GetValue(4, 3, destSheet);
    m_pDoc->GetFormula(4, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=G4+30"),
                         aString); // references over selection gaps are not adjusted
    ASSERT_DOUBLES_EQUAL(bSkipEmpty ? 1030 : 30, fValue); // It was 35
    fValue = m_pDoc->GetValue(4, 4, destSheet);
    m_pDoc->GetFormula(4, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D3+40"), aString);
    ASSERT_DOUBLES_EQUAL(42, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    aString = m_pDoc->GetString(4, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 5, strings are not selected
    // col 5, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    aString = m_pDoc->GetString(4, 5, destSheet);
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 0, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 1, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 2, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R2"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(5, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 4, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 5, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // col 6, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    aString = m_pDoc->GetString(6, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(6, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=C2+C4+60"), aString);
    ASSERT_DOUBLES_EQUAL(2060, m_pDoc->GetValue(6, 1, destSheet)); // It was 64
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(6, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(6, 3, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 2, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 3, destSheet));
    }
    fValue = m_pDoc->GetValue(6, 4, destSheet);
    m_pDoc->GetFormula(6, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=C2+C4+70"), aString);
    ASSERT_DOUBLES_EQUAL(2070, fValue); // It was 74
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 5, destSheet));
    aString = m_pDoc->GetString(6, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 7, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    aString = m_pDoc->GetString(7, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(7, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(
        OUString("=SUMIF(C2:C5;\"<4\")"),
        aString); // CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D2:D5;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(7, 1, destSheet)); // It was 6
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(7, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(7, 3, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    }
    fValue = m_pDoc->GetValue(7, 4, destSheet);
    m_pDoc->GetFormula(7, 4, destSheet, aString);
    ASSERT_DOUBLES_EQUAL(2080, fValue);
    CPPUNIT_ASSERT_EQUAL(OUString("=D$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 8, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(121, m_pDoc->GetValue(8, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(122, m_pDoc->GetValue(8, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(123, m_pDoc->GetValue(8, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(124, m_pDoc->GetValue(8, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 5, destSheet));
    // col 9, col repetition is not supported for multi range copy/paste
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 5, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(3, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(6, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(4, 1, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 5, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 5, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(7, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(7, 4, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeColFiltered(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
                          ┌--- not selected src col C
                          v

         |  D  |    E     |  F  |     G      |        H            |  I  |

    2    | 1 B*| =D2+10  *| R1 *| =C2+C4+60  | =SUMIF(C2:C5;"<4")  | 121 |
    3    | 3 B*| =G4+30 b*|  5 *|          B*|                     | 123 |
    4    | 4   | =D2+40 b*| R4 *| =C1+C3+70 *|    =D$1+$A$1+80    *| 124 |

    * means note attached
    B means background
    b means border
    */

    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // col 2
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    // col 3, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(3, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(3, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 4, destSheet));
    // col 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    m_pDoc->GetFormula(4, 0, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 1, destSheet));
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=G3+30"),
                         aString); // references over selection gaps are not adjusted
    ASSERT_DOUBLES_EQUAL(bSkipEmpty ? 1030 : 30, fValue); // It was 35
    fValue = m_pDoc->GetValue(4, 3, destSheet);
    m_pDoc->GetFormula(4, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+40"), aString);
    ASSERT_DOUBLES_EQUAL(41, fValue); // was originally 42, not adjusted by filtering
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 4, destSheet));
    m_pDoc->GetFormula(4, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    // col 5, strings are not selected
    // col 5, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 0, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 1, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(5, 2, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 4, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // col 6, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    m_pDoc->GetFormula(6, 0, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    m_pDoc->GetFormula(6, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=C2+C4+60"), aString);
    ASSERT_DOUBLES_EQUAL(2060, m_pDoc->GetValue(6, 1, destSheet)); // It was 64
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(6, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 2, destSheet));
    fValue = m_pDoc->GetValue(6, 3, destSheet);
    m_pDoc->GetFormula(6, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=C1+C3+70"), aString);
    ASSERT_DOUBLES_EQUAL(2070, fValue); // It was 74
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 4, destSheet));
    m_pDoc->GetFormula(6, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    // col 7, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    m_pDoc->GetFormula(7, 0, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    m_pDoc->GetFormula(7, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(C2:C5;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(7, 1, destSheet)); // It was 6
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(7, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    fValue = m_pDoc->GetValue(7, 3, destSheet);
    m_pDoc->GetFormula(7, 3, destSheet, aString);
    ASSERT_DOUBLES_EQUAL(2080, fValue);
    CPPUNIT_ASSERT_EQUAL(OUString("=D$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 4, destSheet));
    m_pDoc->GetFormula(7, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    // col 8, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(121, m_pDoc->GetValue(8, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(123, m_pDoc->GetValue(8, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(124, m_pDoc->GetValue(8, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 4, destSheet));
    // col 9, col repetition is not supported for multi range copy/paste
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 5, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(3, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(6, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(4, 1, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 4, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeColTranspose(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
             |         D          |    E     |    F     |       G       |

        2    |         1        B*|    2   B*|    3   B*|       4       |
        3    |      =D2+10       *| =E2+20 b | =F5+30 b*|    =E2+40   b*|
        4    |        R1         *|   R2    *|    5    *|      R4      *|
        5    |     =D1+F1+60      |          |        B*|  =D1+F1+70   *|
        6    | =SUMIF(D1:G1;"<4") |         *|          | =C$1+$A$1+80 *|
        7    |       121          |   122    |   123    |     124       |

        * means note attached
        B means background
        b means border
    */

    // check cell content after transposed copy/paste of filtered data
    // Col C and G are checked to be empty
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // row 0
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    // row 1, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(2, m_pDoc->GetValue(4, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(5, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(6, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 1, destSheet));
    // row 2, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    aString = m_pDoc->GetString(2, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(3, 2, destSheet));
    m_pDoc->GetFormula(3, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=E2+20"), aString);
    ASSERT_DOUBLES_EQUAL(22, m_pDoc->GetValue(4, 2, destSheet));
    fValue = m_pDoc->GetValue(5, 2, destSheet);
    m_pDoc->GetFormula(5, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=F5+30"), aString);
    ASSERT_DOUBLES_EQUAL(bSkipEmpty ? 1030 : 30, fValue); // It was 35
    fValue = m_pDoc->GetValue(6, 2, destSheet);
    m_pDoc->GetFormula(6, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=E2+40"), aString);
    ASSERT_DOUBLES_EQUAL(42, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    aString = m_pDoc->GetString(7, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 3, strings was not selected in multi range selection
    // row 3, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(2, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(4, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R2"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(5, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(7, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // row 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    aString = m_pDoc->GetString(2, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D1+F1+60"), aString);
    ASSERT_DOUBLES_EQUAL(2060, m_pDoc->GetValue(3, 4, destSheet)); // It was 64
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 4, destSheet); // E5
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E5", EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(5, 4, destSheet); // F5
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E5", EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 4, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 4, destSheet));
    }
    fValue = m_pDoc->GetValue(6, 4, destSheet);
    m_pDoc->GetFormula(6, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D1+F1+70"), aString);
    ASSERT_DOUBLES_EQUAL(2070, fValue); // It was 74
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 4, destSheet));
    aString = m_pDoc->GetString(7, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 5, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 5, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D1:G1;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(3, 5, destSheet)); // It was 6
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 5, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(5, 5, destSheet); // F6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    }

    fValue = m_pDoc->GetValue(6, 5, destSheet);
    m_pDoc->GetFormula(6, 5, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=C$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL(2080, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 6, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(121, m_pDoc->GetValue(3, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(122, m_pDoc->GetValue(4, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(123, m_pDoc->GetValue(5, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(124, m_pDoc->GetValue(6, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 6, destSheet));
    // row 7, not selected
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(4, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(6, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(7, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(5, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    // std::cout << "bSkipEmpty: " << bSkipEmpty << ", pItem == nullptr: " << (pItem == nullptr) << std::endl;
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(3, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    m_pDoc->GetPattern(ScAddress(4, 2, destSheet))->GetItemSet().HasItem(ATTR_BORDER, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(7, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 6, destSheet)));

    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(4, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 5, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeColFilteredTranspose(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
                                  ┌--- filtered src row 2
                                  v

             |         D          |    E     |       F       |

        2    |         1        B*|    3   B*|       4       |
        3    |      =D2+10       *| =E5+30 b*|    =D2+40   b*|
                                                                <- not copied col C
        4    |        R1         *|    5    *|      R4      *|
        5    |     =D1+F1+60      |        B*|  =C1+E1+70   *|
        6    | =SUMIF(D1:G1;"<4") |          | =B$1+$A$1+80 *|
        7    |       121          |   123    |      124      |

        * means note attached
        B means background
        b means border
    */

    // check cell content after transposed copy/paste of filtered data
    // Col C and G are checked to be empty
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // row 0
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    // row 1, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(4, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(5, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 1, destSheet));
    // row 2, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(3, 2, destSheet));
    m_pDoc->GetFormula(3, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=E5+30"), aString);
    ASSERT_DOUBLES_EQUAL(bSkipEmpty ? 1030 : 30, fValue); // It was 35
    fValue = m_pDoc->GetValue(5, 2, destSheet);
    m_pDoc->GetFormula(5, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+40"), aString);
    ASSERT_DOUBLES_EQUAL(41, fValue); // was originally 42, not adjusted by filtering
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 2, destSheet));
    aString = m_pDoc->GetString(6, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 3, strings was not selected in multi range selection
    // row 3, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(2, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(4, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(5, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 3, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 3, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // row 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    aString = m_pDoc->GetString(2, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D1+F1+60"),
                         aString); // formulas over filtered rows are not adjusted
    ASSERT_DOUBLES_EQUAL(2060, m_pDoc->GetValue(3, 4, destSheet)); // It was 64
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 4, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 4, destSheet));
    fValue = m_pDoc->GetValue(5, 4, destSheet);
    m_pDoc->GetFormula(5, 4, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=C1+E1+70"),
                         aString); // formulas over filtered rows are not adjusted
    ASSERT_DOUBLES_EQUAL(2070, fValue); // It was 74
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 4, destSheet));
    aString = m_pDoc->GetString(6, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 5, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 5, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D1:G1;\"<4\")"),
                         aString); // formulas over filtered rows are not adjusted
    ASSERT_DOUBLES_EQUAL(0, m_pDoc->GetValue(3, 5, destSheet)); // It was 6
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 5, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    fValue = m_pDoc->GetValue(5, 5, destSheet);
    m_pDoc->GetFormula(5, 5, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=B$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL(2080, fValue);
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 5, destSheet));
    aString = m_pDoc->GetString(6, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 6, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(121, m_pDoc->GetValue(3, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(123, m_pDoc->GetValue(4, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(124, m_pDoc->GetValue(5, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 6, destSheet));
    // row 7, not copied
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));
    // row 8, not copied
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(4, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(6, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(4, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(3, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    m_pDoc->GetPattern(ScAddress(4, 2, destSheet))->GetItemSet().HasItem(ATTR_BORDER, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());

    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 6, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 6, destSheet)));

    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeRow(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
         |  D  |    E     | F    |  G   |     H      |        I            |

    2    | 1 B*| =D2+10  *| a    | R1  *| =D2+D4+60  | =SUMIF(D2:D5;"<4")  |
    3    | 2 B*| =D3+20 b | b   *| R2  *|            |                    *| <- filtered row
    4    | 3 B*| =G4+30 b*| c   *|  5  *|          B*|                     |
                                                                             <- not selected row
    5    | 6   |    q     | r bB*|  s bB|     t      |          u          |
    6    | -11 |   -12    | -13  |  -14 |    -15     |         -16         |

    * means note attached
    B means background
    b means border
    */

    OUString aString;
    const EditTextObject* pEditObj;
    // col 2
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    // col 3, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(2, m_pDoc->GetValue(3, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(3, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(6, m_pDoc->GetValue(3, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(-11, m_pDoc->GetValue(3, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 6, destSheet));
    // col 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    aString = m_pDoc->GetString(4, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 1, destSheet));
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D3+20"), aString);
    ASSERT_DOUBLES_EQUAL(22, m_pDoc->GetValue(4, 2, destSheet));
    m_pDoc->GetFormula(4, 3, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=G4+30"), aString);
    ASSERT_DOUBLES_EQUAL(35, m_pDoc->GetValue(4, 3, destSheet));
    aString = m_pDoc->GetString(4, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("q"), aString);
    ASSERT_DOUBLES_EQUAL(-12, m_pDoc->GetValue(4, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 6, destSheet));
    aString = m_pDoc->GetString(4, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 5, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    aString = m_pDoc->GetString(5, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(5, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aString);
    aString = m_pDoc->GetString(5, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aString);
    aString = m_pDoc->GetString(5, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("r"), aString);
    ASSERT_DOUBLES_EQUAL(-13, m_pDoc->GetValue(5, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 6, destSheet));
    aString = m_pDoc->GetString(5, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 6, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 0, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 1, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 2, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R2"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(6, 3, destSheet));
    aString = m_pDoc->GetString(6, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("s"), aString);
    ASSERT_DOUBLES_EQUAL(-14, m_pDoc->GetValue(6, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 6, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 6, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // col 7, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    aString = m_pDoc->GetString(7, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(7, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+D4+60"), aString);
    ASSERT_DOUBLES_EQUAL(64, m_pDoc->GetValue(7, 1, destSheet));
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(7, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(7, 3, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    }
    aString = m_pDoc->GetString(7, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("t"), aString);
    ASSERT_DOUBLES_EQUAL(-15, m_pDoc->GetValue(7, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 6, destSheet));
    aString = m_pDoc->GetString(7, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 8, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 0, destSheet));
    aString = m_pDoc->GetString(7, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(8, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D2:D5;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(6, m_pDoc->GetValue(8, 1, destSheet));
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(8, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(8, 3, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 2, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 3, destSheet));
    }
    aString = m_pDoc->GetString(8, 4, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("u"), aString);
    ASSERT_DOUBLES_EQUAL(-16, m_pDoc->GetValue(8, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 6, destSheet));
    aString = m_pDoc->GetString(8, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 9
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 5, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(3, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(7, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    m_pDoc->GetPattern(ScAddress(4, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(5, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(6, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(7, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(4, 1, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 5, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    pItem = m_pDoc->GetAttr(ScAddress(3, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(7, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(8, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 5, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 5, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(8, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 4, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeRowFiltered(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
         |  D  |    E     | F  |  G  |     H      |        I            |

    2    | 1 B*| =D2+10  *| a  | R1 *| =D2+D4+60  | =SUMIF(D2:D5;"<4")  |
    3    | 3 B*| =G3+30 b*| c *|  5 *|          B*|                     |
                                                                          <- not selected
    4    | 6   |    q     | r  |  s  |     t      |          u          |
    5    | -11 |   -12    |-13 | -14 |    -15     |         -16         |

    * means note attached
    B means background
    b means border
    */

    OUString aString;
    const EditTextObject* pEditObj;
    // col 2
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    // col 3, numbers
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(3, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(3, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(6, m_pDoc->GetValue(3, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(-11, m_pDoc->GetValue(3, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 5, destSheet));
    // col 4, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    aString = m_pDoc->GetString(4, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(4, 1, destSheet));
    m_pDoc->GetFormula(4, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+10"), aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=G3+30"), aString);
    ASSERT_DOUBLES_EQUAL(35, m_pDoc->GetValue(4, 2, destSheet));
    aString = m_pDoc->GetString(4, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("q"), aString);
    ASSERT_DOUBLES_EQUAL(-12, m_pDoc->GetValue(4, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    aString = m_pDoc->GetString(4, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 5, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    aString = m_pDoc->GetString(5, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(5, 1, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aString);
    aString = m_pDoc->GetString(5, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("r"), aString);
    ASSERT_DOUBLES_EQUAL(-13, m_pDoc->GetValue(5, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    aString = m_pDoc->GetString(5, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 6, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 0, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 1, destSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(6, 2, destSheet));
    aString = m_pDoc->GetString(6, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("s"), aString);
    ASSERT_DOUBLES_EQUAL(-14, m_pDoc->GetValue(6, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 5, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(6, 5, destSheet));
    CPPUNIT_ASSERT(pEditObj == nullptr);
    // col 7, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    aString = m_pDoc->GetString(7, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(7, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D2+D4+60"), aString);
    ASSERT_DOUBLES_EQUAL(67, m_pDoc->GetValue(7, 1, destSheet));
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(7, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    aString = m_pDoc->GetString(7, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("t"), aString);
    ASSERT_DOUBLES_EQUAL(-15, m_pDoc->GetValue(7, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 8, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 0, destSheet));
    aString = m_pDoc->GetString(8, 0, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(8, 1, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(D2:D5;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(6, m_pDoc->GetValue(8, 1, destSheet));
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(8, 2, destSheet);
        CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 2, destSheet));
    aString = m_pDoc->GetString(8, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("u"), aString);
    ASSERT_DOUBLES_EQUAL(-16, m_pDoc->GetValue(8, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 5, destSheet));
    aString = m_pDoc->GetString(8, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // col 9
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 1, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(9, 5, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(3, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(3, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(7, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    m_pDoc->GetPattern(ScAddress(4, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(5, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(6, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(7, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(4, 1, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    pItem = m_pDoc->GetAttr(ScAddress(3, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(4, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(7, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 0, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 1, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 2, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 3, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(8, 4, destSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(9, 4, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(7, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 4, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeRowTranspose(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
             |         D          |    E     |    F     | G  |  H  |

        2    |         1        B*|    2   B*|    3   B*| 6  | -11 |
        3    |      =D2+10       *| =E2+20 b | =F5+30 b*| q  | -12 |
        4    |         a          |    b    *|    c    *| r  | -13 |
        5    |        R1         *|   R2    *|    5    *| s  | -14 |
        6    |     =D2+F2+60      |          |        B*| t  | -15 |
        7    | =SUMIF(D2:G2;"<4") |         *|          | u  | -16 |

        * means note attached
        B means background
        b means border
    */

    //check cell content after transposed copy/paste of filtered data
    // Note: column F is a repetition of srcSheet.Column A
    // Col C and G are checked to be empty
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // row 0
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    // row 1, numbers
    fValue = m_pDoc->GetValue(2, 1, destSheet); // C2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell C2", 1000, fValue);
    fValue = m_pDoc->GetValue(3, 1, destSheet); // D2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell D2", 1, fValue);
    fValue = m_pDoc->GetValue(4, 1, destSheet); // E2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell E2", 2, fValue);
    fValue = m_pDoc->GetValue(5, 1, destSheet); // F2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell F2", 3, fValue);
    fValue = m_pDoc->GetValue(6, 1, destSheet); // G2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell G2", 6, fValue);
    ASSERT_DOUBLES_EQUAL(-11, m_pDoc->GetValue(7, 1, destSheet));
    fValue = m_pDoc->GetValue(8, 1, destSheet); // I2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell I2", 1000, fValue);
    // row 2, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    aString = m_pDoc->GetString(2, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    fValue = m_pDoc->GetValue(3, 2, destSheet); // D3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D3", 11, fValue);
    m_pDoc->GetFormula(3, 2, destSheet, aString); // D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D3 should point on D2", OUString("=D2+10"),
                                 aString);
    m_pDoc->GetFormula(4, 2, destSheet, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula E3 should point on E2", OUString("=E2+20"),
                                 aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet); // E3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula E3", 22, fValue);
    fValue = m_pDoc->GetValue(5, 2, destSheet); // F3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula F3", 35, fValue);
    m_pDoc->GetFormula(5, 2, destSheet, aString); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula F3 should point on F2", OUString("=F5+30"),
                                 aString);
    aString = m_pDoc->GetString(6, 2, destSheet); // G3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G4 should contain: q", OUString("q"), aString);
    ASSERT_DOUBLES_EQUAL(-12, m_pDoc->GetValue(7, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 2, destSheet));
    aString = m_pDoc->GetString(8, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 3, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    aString = m_pDoc->GetString(2, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(3, 3, destSheet); // D4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell D4 should contain: a", OUString("a"), aString);
    aString = m_pDoc->GetString(4, 3, destSheet); // E4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E4 should contain: b", OUString("b"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet); // F4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F4 should contain: c", OUString("c"), aString);
    aString = m_pDoc->GetString(6, 3, destSheet); // G4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G4 should contain: r", OUString("r"), aString);
    ASSERT_DOUBLES_EQUAL(-13, m_pDoc->GetValue(7, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 3, destSheet));
    aString = m_pDoc->GetString(8, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 4, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(2, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in C5.", pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in D5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong in D5 ", OUString("R1"),
                                 pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(4, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in E5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong E5.", OUString("R2"), pEditObj->GetText(0));
    fValue = m_pDoc->GetValue(5, 4, destSheet); // F5
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell F5", 5, fValue);
    aString = m_pDoc->GetString(6, 4, destSheet); // G5
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G5 should contain: s", OUString("s"), aString);
    ASSERT_DOUBLES_EQUAL(-14, m_pDoc->GetValue(7, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(8, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in I5.", pEditObj == nullptr);
    // row 5, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 5, destSheet, aString); // D6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D6", OUString("=D2+F2+60"), aString);
    fValue = m_pDoc->GetValue(3, 5, destSheet); // D6
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D6", 64, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 5, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(5, 5, destSheet); // F6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 5, destSheet));
    }
    aString = m_pDoc->GetString(6, 5, destSheet); // G6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G6 should contain: t", OUString("t"), aString);
    ASSERT_DOUBLES_EQUAL(-15, m_pDoc->GetValue(7, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 5, destSheet));
    aString = m_pDoc->GetString(8, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 6, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 6, destSheet));
    aString = m_pDoc->GetString(2, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 6, destSheet, aString); // D7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D7", OUString("=SUMIF(D2:G2;\"<4\")"),
                                 aString);
    fValue = m_pDoc->GetValue(3, 6, destSheet); // D7
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D7", 6, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 6, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
        aString = m_pDoc->GetString(5, 6, destSheet); // F6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
    {
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 6, destSheet));
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 6, destSheet));
    }
    aString = m_pDoc->GetString(6, 6, destSheet); // G4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell G7 should contain: u", OUString("u"), aString);
    ASSERT_DOUBLES_EQUAL(-16, m_pDoc->GetValue(7, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(8, 6, destSheet));
    aString = m_pDoc->GetString(8, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 7
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("D2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("D2 has blue background", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(4, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("E2 has blue background", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("F2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("F2 has a pattern", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(6, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("G2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(7, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("H2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(5, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(3, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("D3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("D3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("D3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("D3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("D3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());

    m_pDoc->GetPattern(ScAddress(4, 2, destSheet))->GetItemSet().HasItem(ATTR_BORDER, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());

    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("E3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("E3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("E3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("F3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("F3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("F3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("F3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("F3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("G3 has top border", !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("G3 has bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("G3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("G3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(7, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("H3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("H3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("H3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("H3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("H3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C1",
                           !m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D1",
                           !m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E1",
                           !m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F1",
                           !m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G1",
                           !m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H1",
                           !m_pDoc->HasNote(ScAddress(7, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C2",
                           !m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D2",
                           m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E2",
                           m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F2",
                           m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G2",
                           !m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H2",
                           !m_pDoc->HasNote(ScAddress(7, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C3",
                           !m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D3",
                           m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E3",
                           !m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F3",
                           m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G3",
                           !m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H3",
                           !m_pDoc->HasNote(ScAddress(7, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C4",
                           !m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D4",
                           !m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E4",
                           m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F4",
                           m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G4",
                           m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H4",
                           !m_pDoc->HasNote(ScAddress(7, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C5",
                           !m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D5",
                           m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E5",
                           m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F5",
                           m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G5",
                           !m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H5",
                           !m_pDoc->HasNote(ScAddress(7, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C6",
                           !m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D6",
                           !m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E6",
                           !m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on G6",
                           !m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H6",
                           !m_pDoc->HasNote(ScAddress(7, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C7",
                           !m_pDoc->HasNote(ScAddress(2, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D7",
                           !m_pDoc->HasNote(ScAddress(3, 6, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(4, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F7",
                           !m_pDoc->HasNote(ScAddress(5, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note  a G7",
                           !m_pDoc->HasNote(ScAddress(6, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H7",
                           !m_pDoc->HasNote(ScAddress(7, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C8",
                           !m_pDoc->HasNote(ScAddress(2, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D8",
                           !m_pDoc->HasNote(ScAddress(3, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E8",
                           !m_pDoc->HasNote(ScAddress(4, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F8",
                           !m_pDoc->HasNote(ScAddress(5, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G8",
                           !m_pDoc->HasNote(ScAddress(6, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on H8",
                           !m_pDoc->HasNote(ScAddress(7, 7, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2",
                                 m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2",
                                 m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F2",
                                 m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    // G2 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3",
                                 m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    // E3 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F3",
                                 m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    // D4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4",
                                 m_pDoc->GetNote(ScAddress(2, 1, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F4",
                                 m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5",
                                 m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5",
                                 m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F5",
                                 m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E7",
                                     m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText(),
                                     m_pDoc->GetNote(ScAddress(4, 6, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 4, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialMultiRangeRowFilteredTranspose(bool bSkipEmpty)
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;

    /*
             |         D          |    E     | F  |  G  |

        2    |         1        B*|    3   B*| 6  | -11 |
        3    |      =D2+10       *| =F5+30 b*| q  | -12 |
        4    |         a          |    c    *| r  | -13 |
        5    |        R1         *|    5    *| s  | -14 |
        6    |     =D2+F2+60      |        B*| t  | -15 |
        7    | =SUMIF(D2:G2;"<4") |          | u  | -16 |

        * means note attached
        B means background
        b means border
    */

    //check cell content after transposed copy/paste of filtered data
    // Note: column F is a repetition of srcSheet.Column A
    // Col C and G are checked to be empty
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // row 0
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 0, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 0, destSheet));
    // row 1, numbers
    fValue = m_pDoc->GetValue(2, 1, destSheet); // C2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell C2", 1000, fValue);
    fValue = m_pDoc->GetValue(3, 1, destSheet); // D2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell D2", 1, fValue);
    fValue = m_pDoc->GetValue(4, 1, destSheet); // E2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell E2", 3, fValue);
    fValue = m_pDoc->GetValue(5, 1, destSheet); // F2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell F2", 6, fValue);
    ASSERT_DOUBLES_EQUAL(-11, m_pDoc->GetValue(6, 1, destSheet));
    fValue = m_pDoc->GetValue(7, 1, destSheet); // H2
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell H2", 1000, fValue);
    // row 2, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 2, destSheet));
    aString = m_pDoc->GetString(2, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    fValue = m_pDoc->GetValue(3, 2, destSheet); // D3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D3", 11, fValue);
    m_pDoc->GetFormula(3, 2, destSheet, aString); // D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D3 should point on D2", OUString("=D2+10"),
                                 aString);
    fValue = m_pDoc->GetValue(4, 2, destSheet); // E3
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula E3", 35, fValue);
    m_pDoc->GetFormula(4, 2, destSheet, aString); // E3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula E3 should point on F2", OUString("=E5+30"),
                                 aString);
    aString = m_pDoc->GetString(5, 2, destSheet); // F3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F4 should contain: q", OUString("q"), aString);
    ASSERT_DOUBLES_EQUAL(-12, m_pDoc->GetValue(6, 2, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 2, destSheet));
    aString = m_pDoc->GetString(7, 2, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 3, strings
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 3, destSheet));
    aString = m_pDoc->GetString(2, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    aString = m_pDoc->GetString(3, 3, destSheet); // D4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell D4 should contain: a", OUString("a"), aString);
    aString = m_pDoc->GetString(4, 3, destSheet); // E4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E4 should contain: c", OUString("c"), aString);
    aString = m_pDoc->GetString(5, 3, destSheet); // F4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F4 should contain: r", OUString("r"), aString);
    ASSERT_DOUBLES_EQUAL(-13, m_pDoc->GetValue(6, 3, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 3, destSheet));
    aString = m_pDoc->GetString(7, 3, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 4, rich text
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(2, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in C5.", pEditObj == nullptr);
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in D5.", pEditObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Edit cell value wrong in D5 ", OUString("R1"),
                                 pEditObj->GetText(0));
    fValue = m_pDoc->GetValue(4, 4, destSheet); // E5
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell E5", 5, fValue);
    aString = m_pDoc->GetString(5, 4, destSheet); // F5
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F5 should contain: s", OUString("s"), aString);
    ASSERT_DOUBLES_EQUAL(-14, m_pDoc->GetValue(6, 4, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 4, destSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(7, 4, destSheet));
    CPPUNIT_ASSERT_MESSAGE("There should be no edit cell in H5.", pEditObj == nullptr);
    // row 5, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 5, destSheet));
    aString = m_pDoc->GetString(2, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 5, destSheet, aString); // D6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D6", OUString("=D2+F2+60"), aString);
    fValue = m_pDoc->GetValue(3, 5, destSheet); // D6
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D6", 67, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 5, destSheet); // E6
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 5, destSheet));
    aString = m_pDoc->GetString(5, 5, destSheet); // F6
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F6 should contain: t", OUString("t"), aString);
    ASSERT_DOUBLES_EQUAL(-15, m_pDoc->GetValue(6, 5, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 5, destSheet));
    aString = m_pDoc->GetString(7, 5, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 6, formulas
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 6, destSheet));
    aString = m_pDoc->GetString(2, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    m_pDoc->GetFormula(3, 6, destSheet, aString); // D7
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula D7", OUString("=SUMIF(D2:G2;\"<4\")"),
                                 aString);
    fValue = m_pDoc->GetValue(3, 6, destSheet); // D7
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula D7", -7, fValue);
    if (!bSkipEmpty)
    {
        aString = m_pDoc->GetString(4, 6, destSheet); // E7
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell E6", EMPTY_OUSTRING, aString);
    }
    else
        ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 6, destSheet));
    aString = m_pDoc->GetString(5, 6, destSheet); // F4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell F7 should contain: u", OUString("u"), aString);
    ASSERT_DOUBLES_EQUAL(-16, m_pDoc->GetValue(6, 6, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 6, destSheet));
    aString = m_pDoc->GetString(7, 6, destSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("1000"), aString);
    // row 7
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(2, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(3, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(4, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(5, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(6, 7, destSheet));
    ASSERT_DOUBLES_EQUAL(1000, m_pDoc->GetValue(7, 7, destSheet));

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(3, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("D2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("D2 has blue background", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(4, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E2 has a pattern", pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("E2 has a pattern", COL_BLUE,
                                 static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("F2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(6, 1, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_MESSAGE("G2 has no pattern", !pItem);
    m_pDoc->GetPattern(ScAddress(4, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(bSkipEmpty, pItem == nullptr);
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    m_pDoc->GetPattern(ScAddress(5, 2, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(5, 3, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 4, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(5, 5, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(5, 6, destSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(3, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("D3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("D3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("D3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("D3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("D3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    m_pDoc->GetPattern(ScAddress(4, 2, destSheet))->GetItemSet().HasItem(ATTR_BORDER, &pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    pItem = m_pDoc->GetAttr(ScAddress(4, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("E3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("E3 has top border", static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("E3 has bottom border",
                           static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("E3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("E3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("F3 has top border", !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("F3 has bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("F3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("F3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(6, 2, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT_MESSAGE("G3 has a border", pItem);
    CPPUNIT_ASSERT_MESSAGE("G3 has no top border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT_MESSAGE("G3 has no bottom border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT_MESSAGE("G3 has no left border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT_MESSAGE("G3 has no right border",
                           !static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 3, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(5, 4, destSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C1",
                           !m_pDoc->HasNote(ScAddress(2, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D1",
                           !m_pDoc->HasNote(ScAddress(3, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E1",
                           !m_pDoc->HasNote(ScAddress(4, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F1",
                           !m_pDoc->HasNote(ScAddress(5, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G1",
                           !m_pDoc->HasNote(ScAddress(6, 0, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C2",
                           !m_pDoc->HasNote(ScAddress(2, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D2",
                           m_pDoc->HasNote(ScAddress(3, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E2",
                           m_pDoc->HasNote(ScAddress(4, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F2",
                           !m_pDoc->HasNote(ScAddress(5, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G2",
                           !m_pDoc->HasNote(ScAddress(6, 1, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C3",
                           !m_pDoc->HasNote(ScAddress(2, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D3",
                           m_pDoc->HasNote(ScAddress(3, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E3",
                           m_pDoc->HasNote(ScAddress(4, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F3",
                           !m_pDoc->HasNote(ScAddress(5, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G3",
                           !m_pDoc->HasNote(ScAddress(6, 2, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C4",
                           !m_pDoc->HasNote(ScAddress(2, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D4",
                           !m_pDoc->HasNote(ScAddress(3, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E4",
                           m_pDoc->HasNote(ScAddress(4, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F4",
                           m_pDoc->HasNote(ScAddress(5, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G4",
                           !m_pDoc->HasNote(ScAddress(6, 3, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C5",
                           !m_pDoc->HasNote(ScAddress(2, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on D5",
                           m_pDoc->HasNote(ScAddress(3, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on E5",
                           m_pDoc->HasNote(ScAddress(4, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F5",
                           !m_pDoc->HasNote(ScAddress(5, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G5",
                           !m_pDoc->HasNote(ScAddress(6, 4, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C6",
                           !m_pDoc->HasNote(ScAddress(2, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D6",
                           !m_pDoc->HasNote(ScAddress(3, 5, destSheet)));
    CPPUNIT_ASSERT_EQUAL(!bSkipEmpty, m_pDoc->HasNote(ScAddress(4, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be  a note on F6",
                           !m_pDoc->HasNote(ScAddress(5, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G6",
                           !m_pDoc->HasNote(ScAddress(6, 5, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C7",
                           !m_pDoc->HasNote(ScAddress(2, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D7",
                           !m_pDoc->HasNote(ScAddress(3, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E7",
                           !m_pDoc->HasNote(ScAddress(4, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note  a F7",
                           !m_pDoc->HasNote(ScAddress(5, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G7",
                           !m_pDoc->HasNote(ScAddress(6, 6, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on C8",
                           !m_pDoc->HasNote(ScAddress(2, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on D8",
                           !m_pDoc->HasNote(ScAddress(3, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on E8",
                           !m_pDoc->HasNote(ScAddress(4, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on F8",
                           !m_pDoc->HasNote(ScAddress(5, 7, destSheet)));
    CPPUNIT_ASSERT_MESSAGE("There should be no note on G8",
                           !m_pDoc->HasNote(ScAddress(6, 7, destSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2",
                                 m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2",
                                 m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3",
                                 m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E3",
                                 m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4",
                                 m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5",
                                 m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5",
                                 m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText(),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText(),
                             m_pDoc->GetNote(ScAddress(4, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetNote(ScAddress(2, 4, srcSheet))->GetText(),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::testCopyPasteMultiRange()
{
    m_pDoc->InsertTab(0, "Test");

    // Fill A2:B6 with numbers.
    for (SCROW nRow = 1; nRow <= 5; ++nRow)
    {
        for (SCCOL nCol = 0; nCol <= 1; ++nCol)
        {
            ScAddress aPos(nCol, nRow, 0);
            m_pDoc->SetValue(aPos, nRow + nCol);
        }
    }

    // Fill D9:E11 with numbers.
    for (SCROW nRow = 8; nRow <= 10; ++nRow)
    {
        for (SCCOL nCol = 3; nCol <= 4; ++nCol)
        {
            ScAddress aPos(nCol, nRow, 0);
            m_pDoc->SetValue(aPos, 10.0);
        }
    }

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Copy A2:B2, A4:B4, and A6:B6 to clipboard.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScClipParam aClipParam;
    aClipParam.maRanges.push_back(ScRange(0, 1, 0, 1, 1, 0)); // A2:B2
    aClipParam.maRanges.push_back(ScRange(0, 3, 0, 1, 3, 0)); // A4:B4
    aClipParam.maRanges.push_back(ScRange(0, 5, 0, 1, 5, 0)); // A6:B6
    aClipParam.meDirection = ScClipParam::Row;
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste to D9:E11, and make sure it won't crash (rhbz#1080196).
    m_pDoc->CopyMultiRangeFromClip(ScAddress(3, 8, 0), aMark, InsertDeleteFlags::CONTENTS,
                                   &aClipDoc);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(3, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(3, 9, 0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(4, 9, 0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(3, 10, 0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(4, 10, 0)));

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteSkipEmpty()
{
    struct Check
    {
        const char* mpStr;
        Color maColor;
        bool mbHasNote;
    };

    struct TestRange
    {
        ScDocument* mpDoc;

        explicit TestRange(ScDocument* pDoc)
            : mpDoc(pDoc)
        {
        }

        bool checkRange(const ScAddress& rPos, const Check* p, const Check* pEnd)
        {
            ScAddress aPos(rPos);
            OUString aPosStr = aPos.Format(ScRefFlags::VALID);
            for (; p != pEnd; ++p, aPos.IncRow())
            {
                if (!mpDoc->GetString(aPos).equalsAscii(p->mpStr))
                {
                    cerr << aPosStr << ": incorrect string value: expected='" << p->mpStr
                         << "' actual='" << mpDoc->GetString(aPos) << endl;
                    return false;
                }

                const SvxBrushItem* pBrush = mpDoc->GetAttr(aPos, ATTR_BACKGROUND);
                if (!pBrush)
                {
                    cerr << aPosStr << ": failed to get brush item from the cell." << endl;
                    return false;
                }

                if (pBrush->GetColor() != p->maColor)
                {
                    Color aExpected = p->maColor;
                    Color aActual = pBrush->GetColor();
                    cerr << aPosStr << ": incorrect cell background color: expected=("
                         << static_cast<int>(aExpected.GetRed()) << ","
                         << static_cast<int>(aExpected.GetGreen()) << ","
                         << static_cast<int>(aExpected.GetBlue()) << "), actual=("
                         << static_cast<int>(aActual.GetRed()) << ","
                         << static_cast<int>(aActual.GetGreen()) << ","
                         << static_cast<int>(aActual.GetBlue()) << ")" << endl;

                    return false;
                }

                bool bHasNote = mpDoc->HasNote(aPos);
                if (bHasNote != p->mbHasNote)
                {
                    cerr << aPosStr << ": ";
                    if (p->mbHasNote)
                        cerr << "this cell should have a cell note, but doesn't." << endl;
                    else
                        cerr << "this cell should NOT have a cell note, but one is found." << endl;

                    return false;
                }
            }

            return true;
        }

    } aTest(m_pDoc);

    m_pDoc->InsertTab(0, "Test");
    m_pDoc->InitDrawLayer(m_xDocShell.get()); // for cell note objects.

    ScRange aSrcRange(0, 0, 0, 0, 4, 0);
    ScRange aDestRange(1, 0, 0, 1, 4, 0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);

    // Put some texts in B1:B5.
    m_pDoc->SetString(ScAddress(1, 0, 0), "A");
    m_pDoc->SetString(ScAddress(1, 1, 0), "B");
    m_pDoc->SetString(ScAddress(1, 2, 0), "C");
    m_pDoc->SetString(ScAddress(1, 3, 0), "D");
    m_pDoc->SetString(ScAddress(1, 4, 0), "E");

    // Set the background color of B1:B5 to blue.
    ScPatternAttr aCellBackColor(m_pDoc->GetPool());
    aCellBackColor.GetItemSet().Put(SvxBrushItem(COL_BLUE, ATTR_BACKGROUND));
    m_pDoc->ApplyPatternAreaTab(1, 0, 1, 4, 0, aCellBackColor);

    // Insert notes to B1:B5.
    m_pDoc->GetOrCreateNote(ScAddress(1, 0, 0));
    m_pDoc->GetOrCreateNote(ScAddress(1, 1, 0));
    m_pDoc->GetOrCreateNote(ScAddress(1, 2, 0));
    m_pDoc->GetOrCreateNote(ScAddress(1, 3, 0));
    m_pDoc->GetOrCreateNote(ScAddress(1, 4, 0));

    // Prepare a clipboard content interleaved with empty cells.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    ScClipParam aParam(aSrcRange, false);
    aClipDoc.SetClipParam(aParam);
    aClipDoc.SetString(ScAddress(0, 0, 0), "Clip1");
    aClipDoc.SetString(ScAddress(0, 2, 0), "Clip2");
    aClipDoc.SetString(ScAddress(0, 4, 0), "Clip3");

    // Set the background color of A1:A5 to yellow.
    aCellBackColor.GetItemSet().Put(SvxBrushItem(COL_YELLOW, ATTR_BACKGROUND));
    aClipDoc.ApplyPatternAreaTab(0, 0, 0, 4, 0, aCellBackColor);

    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aClipDoc.GetCellType(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, aClipDoc.GetCellType(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aClipDoc.GetCellType(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, aClipDoc.GetCellType(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aClipDoc.GetCellType(ScAddress(0, 4, 0)));

    // Check the initial condition.
    {
        static const Check aChecks[] = {
            { "A", COL_BLUE, true }, { "B", COL_BLUE, true }, { "C", COL_BLUE, true },
            { "D", COL_BLUE, true }, { "E", COL_BLUE, true },
        };

        bool bRes
            = aTest.checkRange(ScAddress(1, 0, 0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Initial check failed.", bRes);
    }

    // Create undo document.
    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(*m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aDestRange, InsertDeleteFlags::ALL, false, *pUndoDoc, &aMark);

    // Paste clipboard content onto A1:A5 but skip empty cells.
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, pUndoDoc.get(), &aClipDoc, true,
                         false, false, true /*bSkipEmpty*/);

    // Create redo document.
    ScDocumentUniquePtr pRedoDoc(new ScDocument(SCDOCMODE_UNDO));
    pRedoDoc->InitUndo(*m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aDestRange, InsertDeleteFlags::ALL, false, *pRedoDoc, &aMark);

    // Create an undo object for this.
    std::unique_ptr<ScRefUndoData> pRefUndoData(new ScRefUndoData(m_pDoc));
    ScUndoPaste aUndo(m_xDocShell.get(), aDestRange, aMark, std::move(pUndoDoc),
                      std::move(pRedoDoc), InsertDeleteFlags::ALL, std::move(pRefUndoData));

    // Check the content after the paste.
    {
        static const Check aChecks[] = {
            { "Clip1", COL_YELLOW, false }, { "B", COL_BLUE, true },
            { "Clip2", COL_YELLOW, false }, { "D", COL_BLUE, true },
            { "Clip3", COL_YELLOW, false },
        };

        bool bRes
            = aTest.checkRange(ScAddress(1, 0, 0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Check after paste failed.", bRes);
    }

    // Undo, and check the content.
    aUndo.Undo();
    {
        static const Check aChecks[] = {
            { "A", COL_BLUE, true }, { "B", COL_BLUE, true }, { "C", COL_BLUE, true },
            { "D", COL_BLUE, true }, { "E", COL_BLUE, true },
        };

        bool bRes
            = aTest.checkRange(ScAddress(1, 0, 0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Check after undo failed.", bRes);
    }

    // Redo, and check the content again.
    aUndo.Redo();
    {
        static const Check aChecks[] = {
            { "Clip1", COL_YELLOW, false }, { "B", COL_BLUE, true },
            { "Clip2", COL_YELLOW, false }, { "D", COL_BLUE, true },
            { "Clip3", COL_YELLOW, false },
        };

        bool bRes
            = aTest.checkRange(ScAddress(1, 0, 0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Check after redo failed.", bRes);
    }

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteSkipEmpty2()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0, 0, 0), "A");
    m_pDoc->SetString(ScAddress(2, 0, 0), "C");

    // Copy A1:C1 to clipboard.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, static_cast<SCTAB>(0));
    copyToClip(m_pDoc, ScRange(0, 0, 0, 2, 0, 0), &aClipDoc);

    // Paste to A3 with the skip empty option set.  This used to freeze. (fdo#77735)
    ScRange aDestRange(0, 2, 0, 2, 2, 0);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc, false,
                         false, true, true);

    CPPUNIT_ASSERT_EQUAL(OUString("A"), m_pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("B3 should be empty.", CELLTYPE_NONE,
                                 m_pDoc->GetCellType(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("C"), m_pDoc->GetString(ScAddress(2, 2, 0)));

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCutPasteRefUndo()
{
    // Testing scenario: A2 references B2, and B2 gets cut and pasted onto C2,
    // which updates A2's formula to reference C2. Then the paste action gets
    // undone, which should also undo A2's formula to reference back to B2.

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Test");

    // A2 references B2.
    m_pDoc->SetString(ScAddress(0, 1, 0), "=B2");

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Set up clip document for cutting of B2.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    ScClipParam aParam(ScAddress(1, 1, 0), true);
    aClipDoc.SetClipParam(aParam);
    aClipDoc.SetValue(ScAddress(1, 1, 0), 12.0);

    // Set up undo document for reference update.
    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(*m_pDoc, 0, 0);

    // Do the pasting of 12 into C2.  This should update A2 to reference C2.
    m_pDoc->CopyFromClip(ScAddress(2, 1, 0), aMark, InsertDeleteFlags::CONTENTS, pUndoDoc.get(),
                         &aClipDoc);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(0, 1, 0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 1, 0), "C2", "A2 should be referencing C2.");

    // At this point, the ref undo document should contain a formula cell at A2 that references B2.
    ASSERT_FORMULA_EQUAL(*pUndoDoc, ScAddress(0, 1, 0), "B2",
                         "A2 in the undo document should be referencing B2.");

    ScUndoPaste aUndo(m_xDocShell.get(), ScRange(2, 1, 0), aMark, std::move(pUndoDoc), nullptr,
                      InsertDeleteFlags::CONTENTS, nullptr, false, nullptr);
    aUndo.Undo();

    // Now A2 should be referencing B2 once again.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 1, 0), "B2",
                         "A2 should be referencing B2 after undo.");

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCutPasteGroupRefUndo()
{
    // Test that Cut&Paste part of a grouped formula adjusts references
    // correctly and Undo works.

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Test");

    // Formula data in A1:A9
    std::vector<std::vector<const char*>> aData
        = { { "1" },      { "=A1+A1" }, { "=A2+A1" }, { "=A3+A2" }, { "=A4+A3" },
            { "=A5+A4" }, { "=A6+A5" }, { "=A7+A6" }, { "=A8+A7" } };

    ScAddress aPos(0, 0, 0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to insert data", aPos, aDataRange.aStart);

    // Check initial data.
    const char* aDataCheck[][2] = { { "1", "" },        { "2", "=A1+A1" },  { "3", "=A2+A1" },
                                    { "5", "=A3+A2" },  { "8", "=A4+A3" },  { "13", "=A5+A4" },
                                    { "21", "=A6+A5" }, { "34", "=A7+A6" }, { "55", "=A8+A7" } };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aDataCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0, i, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initial data failure",
                                     OUString::createFromAscii(aDataCheck[i][0]), aString);
        m_pDoc->GetFormula(0, i, 0, aString);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initial formula failure",
                                     OUString::createFromAscii(aDataCheck[i][1]), aString);
    }

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);

    // Set up clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    // Cut A4:A6 to clipboard with Undo.
    std::unique_ptr<ScUndoCut> pUndoCut(
        cutToClip(*m_xDocShell, ScRange(0, 3, 0, 0, 5, 0), &aClipDoc, true));

    // Check data after Cut.
    const char* aCutCheck[] = { "1", "2", "3", "", "", "", "0", "0", "0" };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aCutCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0, i, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cut data failure", OUString::createFromAscii(aCutCheck[i]),
                                     aString);
    }

    // Paste to B5:B7 with Undo.
    ScRange aPasteRange(1, 4, 0, 1, 6, 0);
    aMark.SetMarkArea(aPasteRange);
    ScDocument* pPasteUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pPasteUndoDoc->InitUndoSelected(*m_pDoc, aMark);
    std::unique_ptr<ScUndoPaste> pUndoPaste(
        createUndoPaste(*m_xDocShell, aPasteRange, ScDocumentUniquePtr(pPasteUndoDoc)));
    m_pDoc->CopyFromClip(aPasteRange, aMark, InsertDeleteFlags::ALL, pPasteUndoDoc, &aClipDoc);

    // Check data after Paste.
    const char* aPasteCheck[][4] = { { "1", "", "", "" },
                                     { "2", "", "=A1+A1", "" },
                                     { "3", "", "=A2+A1", "" },
                                     { "", "", "", "" },
                                     { "", "5", "", "=A3+A2" },
                                     { "", "8", "", "=B5+A3" },
                                     { "21", "13", "=B7+B6", "=B6+B5" },
                                     { "34", "", "=A7+B7", "" },
                                     { "55", "", "=A8+A7", "" } };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aPasteCheck); ++i)
    {
        for (size_t j = 0; j < 2; ++j)
        {
            OUString aString = m_pDoc->GetString(j, i, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Paste data failure",
                                         OUString::createFromAscii(aPasteCheck[i][j]), aString);
            m_pDoc->GetFormula(j, i, 0, aString);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Paste formula failure",
                                         OUString::createFromAscii(aPasteCheck[i][2 + j]), aString);
        }
    }

    // Undo Paste and check, must be same as after Cut.
    pUndoPaste->Undo();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aCutCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0, i, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo Paste data failure",
                                     OUString::createFromAscii(aCutCheck[i]), aString);
    }

    // Undo Cut and check, must be initial data.
    pUndoCut->Undo();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aDataCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0, i, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo Cut data failure",
                                     OUString::createFromAscii(aDataCheck[i][0]), aString);
        m_pDoc->GetFormula(0, i, 0, aString);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo Cut formula failure",
                                     OUString::createFromAscii(aDataCheck[i][1]), aString);
    }

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testMoveRefBetweenSheets()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Test1");
    m_pDoc->InsertTab(1, "Test2");

    m_pDoc->SetValue(ScAddress(0, 0, 0), 12.0);
    m_pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    m_pDoc->SetValue(ScAddress(2, 0, 0), 8.0);
    m_pDoc->SetString(ScAddress(0, 1, 0), "=A1");
    m_pDoc->SetString(ScAddress(0, 2, 0), "=SUM(A1:C1)");

    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(ScAddress(0, 2, 0)));

    // These formulas should not display the sheet name.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 1, 0), "A1", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 2, 0), "SUM(A1:C1)", "Wrong formula!");

    // Move Test1.A2:A3 to Test2.A2:A3.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    bool bMoved
        = rFunc.MoveBlock(ScRange(0, 1, 0, 0, 2, 0), ScAddress(0, 1, 1), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("This cell should be empty after the move.", CELLTYPE_NONE,
                                 m_pDoc->GetCellType(ScAddress(0, 1, 0)));
    ASSERT_DOUBLES_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0, 1, 1)));
    ASSERT_DOUBLES_EQUAL(30.0, m_pDoc->GetValue(ScAddress(0, 2, 1)));

    // The reference in the pasted formula should display sheet name after the move.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 1, 1), "Test1.A1", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 2, 1), "SUM(Test1.A1:C1)", "Wrong formula!");

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testUndoCut()
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // Insert values into A1:A3.
    m_pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    m_pDoc->SetValue(ScAddress(0, 1, 0), 10.0);
    m_pDoc->SetValue(ScAddress(0, 2, 0), 100.0);

    // SUM in A4.
    m_pDoc->SetString(ScAddress(0, 3, 0), "=SUM(A1:A3)");
    CPPUNIT_ASSERT_EQUAL(111.0, m_pDoc->GetValue(0, 3, 0));

    // Select A1:A3.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    ScRange aRange(0, 0, 0, 0, 2, 0);
    aMark.SetMarkArea(aRange);
    aMark.MarkToMulti();

    // Set up an undo object for cutting A1:A3.
    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(*m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aRange, InsertDeleteFlags::ALL, false, *pUndoDoc);
    ASSERT_DOUBLES_EQUAL(1.0, pUndoDoc->GetValue(ScAddress(0, 0, 0)));
    ASSERT_DOUBLES_EQUAL(10.0, pUndoDoc->GetValue(ScAddress(0, 1, 0)));
    ASSERT_DOUBLES_EQUAL(100.0, pUndoDoc->GetValue(ScAddress(0, 2, 0)));
    ScUndoCut aUndo(m_xDocShell.get(), aRange, aRange.aEnd, aMark, std::move(pUndoDoc));

    // "Cut" the selection.
    m_pDoc->DeleteSelection(InsertDeleteFlags::ALL, aMark);
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(0, 3, 0)); // The SUM should be zero after the "cut".

    // Undo it, and check the result.
    aUndo.Undo();
    ASSERT_DOUBLES_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0, 0, 0)));
    ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(ScAddress(0, 1, 0)));
    ASSERT_DOUBLES_EQUAL(100.0, m_pDoc->GetValue(ScAddress(0, 2, 0)));
    ASSERT_DOUBLES_EQUAL(
        111.0, m_pDoc->GetValue(0, 3, 0)); // The SUM value should be back to the original.

    // Redo it and check.
    aUndo.Redo();
    ASSERT_DOUBLES_EQUAL(0.0, m_pDoc->GetValue(0, 3, 0));

    // Undo again.
    aUndo.Undo();
    ASSERT_DOUBLES_EQUAL(111.0, m_pDoc->GetValue(0, 3, 0));

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testMoveBlock()
{
    m_pDoc->InsertTab(0, "SheetNotes");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetString(1, 0, 0, "=A1+1");
    m_pDoc->SetString(2, 0, 0, "test");

    // add notes to A1:C1
    ScAddress aAddrA1(0, 0, 0);
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAddrA1);
    pNoteA1->SetText(aAddrA1, "Hello world in A1");
    ScAddress aAddrB1(1, 0, 0);
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAddrB1);
    pNoteB1->SetText(aAddrB1, "Hello world in B1");
    ScAddress aAddrC1(2, 0, 0);
    ScPostIt* pNoteC1 = m_pDoc->GetOrCreateNote(aAddrC1);
    pNoteC1->SetText(aAddrC1, "Hello world in C1");
    ScAddress aAddrD1(3, 0, 0);

    // previous tests on cell note content are ok. this one fails !!! :(
    //CPPUNIT_ASSERT_MESSAGE("Note content in B1 before move block", m_pDoc->GetNote(aAddrB1)->GetText() == aHelloB1);

    // move notes to B1:D1
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    bool bMoveDone = rDocFunc.MoveBlock(ScRange(0, 0, 0, 2, 0, 0), ScAddress(1, 0, 0),
                                        true /*bCut*/, false, false, false);

    CPPUNIT_ASSERT_MESSAGE("Cells not moved", bMoveDone);

    //check cell content
    OUString aString = m_pDoc->GetString(3, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell D1 should contain: test", OUString("test"), aString);
    m_pDoc->GetFormula(2, 0, 0, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell C1 should contain an updated formula", OUString("=B1+1"),
                                 aString);
    double fValue = m_pDoc->GetValue(aAddrB1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("Cell B1 should contain 1", fValue, 1);

    // cell notes has been moved 1 cell right (event when overlapping)
    CPPUNIT_ASSERT_MESSAGE("There should be NO note on A1", !m_pDoc->HasNote(aAddrA1));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on B1", m_pDoc->HasNote(aAddrB1));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on C1", m_pDoc->HasNote(aAddrC1));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on D1", m_pDoc->HasNote(aAddrD1));
    /* still failing, wrong content ???
    OUString sNoteText;
    sNoteText =  m_pDoc->GetNote(aAddrB1)->GetText();
    CPPUNIT_ASSERT_MESSAGE("Note content in B1", sNoteText == aHelloA1);
    sNoteText =  m_pDoc->GetNote(aAddrC1)->GetText();
    CPPUNIT_ASSERT_MESSAGE("Note content in C1", sNoteText == aHelloB1);
    sNoteText =  m_pDoc->GetNote(aAddrD1)->GetText();
    CPPUNIT_ASSERT_MESSAGE("Note content in D1", sNoteText == aHelloC1);
    */

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteRelativeFormula()
{
    m_pDoc->InsertTab(0, "Formula");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);

    // Insert values to A2 and A4.
    m_pDoc->SetValue(ScAddress(0, 1, 0), 1);
    m_pDoc->SetValue(ScAddress(0, 3, 0), 2);

    // Insert formula to B4.
    m_pDoc->SetString(ScAddress(1, 3, 0), "=A4");
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1, 3, 0)));

    // Select and copy B3:B4 to the clipboard.
    ScRange aRange(1, 2, 0, 1, 3, 0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste it to B1:B2.
    InsertDeleteFlags nFlags = InsertDeleteFlags::ALL;
    ScRange aDestRange(1, 0, 0, 1, 1, 0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

    // B2 references A2, so the value should be 1.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1, 1, 0)));

    // Clear content and start over.
    clearSheet(m_pDoc, 0);
    clearSheet(&aClipDoc, 0);

    // Insert a single formula cell in A1.
    m_pDoc->SetString(ScAddress(0, 0, 0), "=ROW()");
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared()); // single formula cell is never shared.

    // Copy A1 to clipboard.
    aClipParam = ScClipParam(ScAddress(0, 0, 0), false);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    pFC = aClipDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared());

    // Paste to A3.
    aDestRange = ScRange(0, 2, 0, 0, 2, 0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

    pFC = m_pDoc->GetFormulaCell(ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared());

    // Delete A3 and make sure it doesn't crash (see fdo#76132).
    clearRange(m_pDoc, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(0, 2, 0)));

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteRepeatOneFormula()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);

    m_pDoc->InsertTab(0, "Test");

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aMark(m_pDoc->GetSheetLimits());

    // Insert values in A1:B10.
    for (SCROW i = 0; i < 10; ++i)
    {
        m_pDoc->SetValue(ScAddress(0, i, 0), i + 1.0); // column A
        m_pDoc->SetValue(ScAddress(1, i, 0), (i + 1.0) * 10.0); // column B
    }

    // Insert a formula in C1.
    ScAddress aPos(2, 0, 0); // C1
    m_pDoc->SetString(aPos, "=SUM(A1:B1)");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(aPos));

    // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // At this point, there should be only one normal area listener listening
    // on A1:B1.
    ScRange aWholeSheet(0, 0, 0, MAXCOL, MAXROW, 0);
    ScBroadcastAreaSlotMachine* pBASM = m_pDoc->GetBASM();
    CPPUNIT_ASSERT(pBASM);
    std::vector<sc::AreaListener> aListeners
        = pBASM->GetAllListeners(aWholeSheet, sc::AreaOverlapType::Inside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    const sc::AreaListener* pListener = aListeners.data();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 0, 0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener shouldn't be a group listener.",
                           !pListener->mbGroupListening);
#endif

    // Copy C1 to clipboard.
    ScClipParam aClipParam(aPos, false);
    aMark.SetMarkArea(aPos);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste it to C2:C10.
    ScRange aDestRange(2, 1, 0, 2, 9, 0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, &aClipDoc);

    // Make sure C1:C10 are grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

    // Check the formula results.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fExpected = (i + 1.0) * 11.0;
        CPPUNIT_ASSERT_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(2, i, 0)));
    }

        // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // At this point, there should only be one area listener and it should be
    // a group listener listening on A1:B10.
    aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaOverlapType::Inside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    pListener = aListeners.data();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 9, 0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener should be a group listener.",
                           pListener->mbGroupListening);
#endif

    // Insert a new row at row 1.
    ScRange aRowOne(0, 0, 0, MAXCOL, 0, 0);
    aMark.SetMarkArea(aRowOne);
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    rFunc.InsertCells(aRowOne, &aMark, INS_INSROWS_BEFORE, true, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 should be empty.", CELLTYPE_NONE,
                                 m_pDoc->GetCellType(ScAddress(2, 0, 0)));

    // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // Make there we only have one group area listener listening on A2:B11.
    aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaOverlapType::Inside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    pListener = aListeners.data();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 1, 10, 0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener should be a group listener.",
                           pListener->mbGroupListening);
#endif

    // Check the formula results.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fExpected = (i + 1.0) * 11.0;
        CPPUNIT_ASSERT_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(2, i + 1, 0)));
    }

    // Delete row at row 1 to shift the cells up.
    rFunc.DeleteCells(aRowOne, &aMark, DelCellCmd::Rows, true);

    // Check the formula results again.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fExpected = (i + 1.0) * 11.0;
        CPPUNIT_ASSERT_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(2, i, 0)));
    }

        // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // Check the group area listener again to make sure it's listening on A1:B10 once again.
    aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaOverlapType::Inside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    pListener = aListeners.data();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 9, 0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener should be a group listener.",
                           pListener->mbGroupListening);
#endif

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteMixedReferenceFormula()
{
    sc::AutoCalcSwitch aAC(*m_pDoc, true); // turn on auto calc.
    m_pDoc->InsertTab(0, "Test");

    // Insert value to C3
    m_pDoc->SetValue(2, 2, 0, 1.0);

    // Insert formula to A1 with mixed relative/absolute addressing.
    m_pDoc->SetString(0, 0, 0, "=SUM(B:$C)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 0, 0), "SUM(B:$C)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 0, 0));

    // Copy formula in A1 to clipboard.
    ScRange aRange(ScAddress(0, 0, 0));
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    // Paste formula to B1.
    aRange = ScAddress(1, 0, 0);
    pasteFromClip(m_pDoc, aRange, &aClipDoc);
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1, 0, 0), "SUM(C:$C)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(1, 0, 0));

    // Paste formula to C1. All three results now must be circular reference.
    aRange = ScAddress(2, 0, 0);
    pasteFromClip(m_pDoc, aRange, &aClipDoc);
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2, 0, 0), "SUM($C:D)",
                         "Wrong formula."); // reference put in order
    CPPUNIT_ASSERT_EQUAL(OUString("Err:522"), m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Err:522"), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Err:522"), m_pDoc->GetString(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

void TestCopyPaste::testCopyPasteFormulas()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(0, 0, 0, "=COLUMN($A$1)");
    m_pDoc->SetString(0, 1, 0, "=$A$1+B2");
    m_pDoc->SetString(0, 2, 0, "=$Sheet2.A1");
    m_pDoc->SetString(0, 3, 0, "=$Sheet2.$A$1");
    m_pDoc->SetString(0, 4, 0, "=$Sheet2.A$1");

    // to prevent ScEditableTester in ScDocFunc::MoveBlock
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(0, 0, 0), 1.0);
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(0, 1, 0), 1.0);
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    bool bMoveDone = rDocFunc.MoveBlock(ScRange(0, 0, 0, 0, 4, 0), ScAddress(10, 10, 0), false,
                                        false, false, true);

    // check that moving was successful, mainly for editable tester
    CPPUNIT_ASSERT(bMoveDone);
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(10, 10, 0), 1.0);
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(10, 11, 0), 1.0);
    OUString aFormula;
    m_pDoc->GetFormula(10, 10, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=COLUMN($A$1)"), aFormula);
    m_pDoc->GetFormula(10, 11, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=$A$1+L12"), aFormula);
    m_pDoc->GetFormula(10, 12, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=$Sheet2.K11"), aFormula);
    m_pDoc->GetFormula(10, 13, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=$Sheet2.$A$1"), aFormula);
    m_pDoc->GetFormula(10, 14, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=$Sheet2.K$1"), aFormula);
}

void TestCopyPaste::testCopyPasteFormulasExternalDoc()
{
    SfxMedium* pMedium = new SfxMedium("file:///source.fake", StreamMode::STD_READWRITE);
    m_xDocShell->DoInitNew(pMedium);

    ScDocShellRef xExtDocSh = new ScDocShell;
    xExtDocSh->SetIsInUcalc();
    OUString const aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, "ExtSheet1");
    rExtDoc.InsertTab(1, "ExtSheet2");

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(0, 0, 0, "=COLUMN($A$1)");
    m_pDoc->SetString(0, 1, 0, "=$A$1+B2");
    m_pDoc->SetString(0, 2, 0, "=$Sheet2.A1");
    m_pDoc->SetString(0, 3, 0, "=$Sheet2.$A$1");
    m_pDoc->SetString(0, 4, 0, "=$Sheet2.A$1");
    m_pDoc->SetString(0, 5, 0, "=$Sheet1.$A$1");

    ScRange aRange(0, 0, 0, 0, 5, 0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    aRange = ScRange(1, 1, 1, 1, 6, 1);
    ScMarkData aMarkData2(m_pDoc->GetSheetLimits());
    aMarkData2.SetMarkArea(aRange);
    rExtDoc.CopyFromClip(aRange, aMarkData2, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

    OUString aFormula;
    rExtDoc.GetFormula(1, 1, 1, aFormula);
    //adjust absolute refs pointing to the copy area
    CPPUNIT_ASSERT_EQUAL(OUString("=COLUMN($B$2)"), aFormula);
    rExtDoc.GetFormula(1, 2, 1, aFormula);
    //adjust absolute refs and keep relative refs
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$2+C3"), aFormula);
    rExtDoc.GetFormula(1, 3, 1, aFormula);
    // make absolute sheet refs external refs
    CPPUNIT_ASSERT_EQUAL(OUString("='file:///source.fake'#$Sheet2.B2"), aFormula);
    rExtDoc.GetFormula(1, 4, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("='file:///source.fake'#$Sheet2.$A$1"), aFormula);
    rExtDoc.GetFormula(1, 5, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("='file:///source.fake'#$Sheet2.B$1"), aFormula);
    rExtDoc.GetFormula(1, 6, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=$ExtSheet2.$B$2"), aFormula);

    xExtDocSh->DoClose();
}

void TestCopyPaste::testCopyPasteReferencesExternalDoc()
{
    SfxMedium* pMedium = new SfxMedium("file:///source.fake", StreamMode::STD_READWRITE);
    m_xDocShell->DoInitNew(pMedium);

    ScDocShellRef xExtDocSh = new ScDocShell;
    xExtDocSh->SetIsInUcalc();
    OUString aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, "ExtSheet1");

    m_pDoc->InsertTab(0, "Sheet1");

    m_pDoc->SetString(0, 5, 0, "=SUM($Sheet1.A1:A5)");

    ScRange aRange(0, 2, 0, 0, 5, 0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    aRange = ScRange(0, 0, 0, 0, 3, 0);
    ScMarkData aMarkData2(m_pDoc->GetSheetLimits());
    aMarkData2.SetMarkArea(aRange);
    rExtDoc.CopyFromClip(aRange, aMarkData2, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

    OUString aFormula;
    rExtDoc.GetFormula(0, 3, 0, aFormula);
    //adjust absolute refs pointing to the copy area
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM('file:///source.fake'#$Sheet1.A#REF!:A3)"), aFormula);

    xExtDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCopyPaste);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
