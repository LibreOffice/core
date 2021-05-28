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
    void testTdf68976();
    void testTdf71058();

    void testCutPasteSpecial();
    void testCutPasteSpecialTranspose();
    void testCutPasteSpecialSkipEmpty();
    void testCutPasteSpecialSkipEmptyTranspose();
    void testTdf142201Row();
    void testTdf142201ColRel();
    void testTdf142201ColAbs();
    void testTdf142065();
    void testCutTransposedFormulas();
    void testCutTransposedFormulasSquare();
    void testReferencedCutRangesRow();
    void testReferencedCutTransposedRangesRowTab0To0();
    void testReferencedCutTransposedRangesRowTab0To1();
    void testReferencedCutTransposedRangesRowTab1To3();
    void testReferencedCutTransposedRangesRowTab3To1();
    void testReferencedCutRangesCol();
    void testReferencedCutTransposedRangesColTab0To0();
    void testReferencedCutTransposedRangesColTab0To1();
    void testReferencedCutTransposedRangesColTab1To3();
    void testReferencedCutTransposedRangesColTab3To1();

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

    CPPUNIT_TEST(testTdf68976);
    CPPUNIT_TEST(testTdf71058);

    CPPUNIT_TEST(testCutPasteSpecial);
    CPPUNIT_TEST(testCutPasteSpecialTranspose);
    CPPUNIT_TEST(testCutPasteSpecialSkipEmpty);
    CPPUNIT_TEST(testCutPasteSpecialSkipEmptyTranspose);
    CPPUNIT_TEST(testTdf142201Row);
    CPPUNIT_TEST(testTdf142201ColRel);
    CPPUNIT_TEST(testTdf142201ColAbs);
    CPPUNIT_TEST(testTdf142065);
    CPPUNIT_TEST(testCutTransposedFormulas);
    CPPUNIT_TEST(testCutTransposedFormulasSquare);
    CPPUNIT_TEST(testReferencedCutRangesRow);
    CPPUNIT_TEST(testReferencedCutTransposedRangesRowTab0To0);
    CPPUNIT_TEST(testReferencedCutTransposedRangesRowTab0To1);
    CPPUNIT_TEST(testReferencedCutTransposedRangesRowTab1To3);
    CPPUNIT_TEST(testReferencedCutTransposedRangesRowTab3To1);
    CPPUNIT_TEST(testReferencedCutRangesCol);
    CPPUNIT_TEST(testReferencedCutTransposedRangesColTab0To0);
    CPPUNIT_TEST(testReferencedCutTransposedRangesColTab0To1);
    CPPUNIT_TEST(testReferencedCutTransposedRangesColTab1To3);
    CPPUNIT_TEST(testReferencedCutTransposedRangesColTab3To1);

    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef m_xDocShell;
    ScDocument* m_pDoc;

    enum CalcMode
    {
        NoCalc,
        AutoCalc,
        RecalcAtEnd,
        HardRecalcAtEnd
    };

    void executeCopyPasteSpecial(bool bApplyFilter, bool bIncludedFiltered, bool bAsLink,
                                 bool bTranspose, bool bMultiRangeSelection, bool bSkipEmpty,
                                 bool bCut = false,
                                 ScClipParam::Direction eDirection = ScClipParam::Column,
                                 CalcMode eCalcMode = CalcMode::AutoCalc,
                                 InsertDeleteFlags aFlags
                                 = InsertDeleteFlags::CONTENTS | InsertDeleteFlags::ATTRIB);
    void executeCopyPasteSpecial(const SCTAB srcSheet, const SCTAB destSheet, bool bApplyFilter,
                                 bool bIncludedFiltered, bool bAsLink, bool bTranspose,
                                 bool bMultiRangeSelection, bool bSkipEmpty,
                                 std::unique_ptr<ScUndoCut>& pUndoCut,
                                 std::unique_ptr<ScUndoPaste>& pUndoPaste, bool bCut = false,
                                 ScClipParam::Direction eDirection = ScClipParam::Column,
                                 CalcMode eCalcMode = CalcMode::AutoCalc,
                                 InsertDeleteFlags aFlags
                                 = InsertDeleteFlags::CONTENTS | InsertDeleteFlags::ATTRIB);
    void checkCopyPasteSpecialInitial(const SCTAB srcSheet);
    void checkCopyPasteSpecial(bool bSkipEmpty, bool bCut = false);
    void checkCopyPasteSpecialFiltered(bool bSkipEmpty);
    void checkCopyPasteSpecialTranspose(bool bSkipEmpty, bool bCut = false);
    void checkCopyPasteSpecialFilteredTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeCol(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeColFiltered(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeColTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeColFilteredTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRow(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRowFiltered(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRowTranspose(bool bSkipEmpty);
    void checkCopyPasteSpecialMultiRangeRowFilteredTranspose(bool bSkipEmpty);
    void checkReferencedCutTransposedRangesRowUndo(const SCTAB nSrcTab, const SCTAB nDestTab);
    void executeReferencedCutRangesRow(const bool bTransposed, const SCTAB nSrcTab,
                                       const SCTAB nDestTab, const bool bUndo,
                                       std::unique_ptr<ScUndoCut>& pUndoCut,
                                       std::unique_ptr<ScUndoPaste>& pUndoPaste);
    void checkReferencedCutRangesRowIntitial(const SCTAB nSrcTab, const OUString& rDesc);
    void checkReferencedCutRangesRow(const SCTAB nSrcTab, const SCTAB nDestTab);
    void checkReferencedCutTransposedRangesRow(const SCTAB nSrcTab, const SCTAB nDestTab);
    void executeReferencedCutRangesCol(const bool bTransposed, const SCTAB nSrcTab,
                                       const SCTAB nDestTab, const bool bUndo,
                                       std::unique_ptr<ScUndoCut>& pUndoCut,
                                       std::unique_ptr<ScUndoPaste>& pUndoPaste);
    void checkReferencedCutRangesColIntitial(const SCTAB nSrcTab, const SCTAB nDestTab,
                                             const OUString& rDesc);
    void checkReferencedCutRangesCol(const SCTAB nSrcTab, const SCTAB nDestTab);
    void checkReferencedCutTransposedRangesColUndo(const SCTAB nSrcTab, const SCTAB nDestTab);
    void checkReferencedCutTransposedRangesCol(const SCTAB nSrcTab, const SCTAB nDestTab);
    void prepareUndoBeforePaste(bool bCut, ScDocumentUniquePtr& pPasteUndoDoc,
                                std::unique_ptr<ScDocument>& pPasteRefUndoDoc,
                                const ScMarkData& rDestMark, const ScRange& rDestRange,
                                std::unique_ptr<ScRefUndoData>& pUndoData);
    void prepareUndoAfterPaste(ScDocumentUniquePtr& pPasteUndoDoc,
                               std::unique_ptr<ScDocument>& pPasteRefUndoDoc,
                               const ScMarkData& rDestMark, const ScRange& rDestRange,
                               std::unique_ptr<ScRefUndoData>& pUndoData,
                               std::unique_ptr<ScUndoPaste>& pUndoPaste, bool bTranspose = false,
                               bool bAsLink = false, bool bSkipEmpty = false,
                               ScPasteFunc nFunction = ScPasteFunc::NONE,
                               InsCellCmd eMoveMode = InsCellCmd::INS_NONE);

    OUString getFormula(SCCOL nCol, SCROW nRow, SCTAB nTab);
    OUString getRangeByName(const OUString& aRangeName);
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

static void lcl_printValuesAndFormulasInRange(ScDocument* pDoc, const ScRange& rRange,
                                              const OString& rCaption)
{
    printRange(pDoc, rRange, rCaption, false);
    printRange(pDoc, rRange, rCaption, true);
}

OUString TestCopyPaste::getFormula(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    return ::getFormula(m_pDoc, nCol, nRow, nTab);
}

OUString TestCopyPaste::getRangeByName(const OUString& aRangeName)
{
    return ::getRangeByName(m_pDoc, aRangeName);
}

// Cannot be moved to qahelper since ScDocument::CopyToDocument() is not SC_DLLPUBLIC
/** Executes the same steps for undo as ScViewFunc::PasteFromClip(). */
void TestCopyPaste::prepareUndoBeforePaste(bool bCut, ScDocumentUniquePtr& pPasteUndoDoc,
                                           std::unique_ptr<ScDocument>& pPasteRefUndoDoc,
                                           const ScMarkData& rDestMark, const ScRange& rDestRange,
                                           std::unique_ptr<ScRefUndoData>& pUndoData)
{
    InsertDeleteFlags nUndoFlags = InsertDeleteFlags::CONTENTS;
    SCTAB nTabCount = m_pDoc->GetTableCount();

    pPasteUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
    pPasteUndoDoc->InitUndoSelected(*m_pDoc, rDestMark, false, false);
    // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
    m_pDoc->CopyToDocument(rDestRange.aStart.Col(), rDestRange.aStart.Row(), 0,
                           rDestRange.aEnd.Col(), rDestRange.aEnd.Row(), nTabCount - 1, nUndoFlags,
                           false, *pPasteUndoDoc);

    if (bCut)
    {
        // save changed references
        pPasteRefUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pPasteRefUndoDoc->InitUndo(*m_pDoc, 0, nTabCount - 1);

        pUndoData.reset(new ScRefUndoData(m_pDoc));
    }
}

// Cannot be moved to qahelper since ScDocument::CopyToDocument() is not SC_DLLPUBLIC
/** Executes the same steps for undo as ScViewFunc::PasteFromClip(). */
void TestCopyPaste::prepareUndoAfterPaste(ScDocumentUniquePtr& pPasteUndoDoc,
                                          std::unique_ptr<ScDocument>& pPasteRefUndoDoc,
                                          const ScMarkData& rDestMark, const ScRange& rDestRange,
                                          std::unique_ptr<ScRefUndoData>& pUndoData,
                                          std::unique_ptr<ScUndoPaste>& pUndoPaste, bool bTranspose,
                                          bool bAsLink, bool bSkipEmpty, ScPasteFunc nFunction,
                                          InsCellCmd eMoveMode)
{
    InsertDeleteFlags nUndoFlags = InsertDeleteFlags::CONTENTS;
    SCTAB nTabCount = m_pDoc->GetTableCount();

    ScDocumentUniquePtr pPasteRedoDoc;
    // copy redo data after appearance of the first undo
    // don't create Redo-Doc without RefUndoDoc

    if (pPasteRefUndoDoc)
    {
        pPasteRedoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pPasteRedoDoc->InitUndo(*m_pDoc, rDestRange.aStart.Tab(), rDestRange.aEnd.Tab(), false,
                                false);

        // move adapted refs to Redo-Doc

        pPasteRedoDoc->AddUndoTab(0, nTabCount - 1);
        m_pDoc->CopyUpdated(pPasteRefUndoDoc.get(), pPasteRedoDoc.get());

        pPasteUndoDoc->AddUndoTab(0, nTabCount - 1);
        pPasteRefUndoDoc->DeleteArea(rDestRange.aStart.Col(), rDestRange.aStart.Row(),
                                     rDestRange.aEnd.Col(), rDestRange.aEnd.Row(), rDestMark,
                                     InsertDeleteFlags::ALL);
        pPasteRefUndoDoc->CopyToDocument(0, 0, 0, pPasteUndoDoc->MaxCol(), pPasteUndoDoc->MaxRow(),
                                         nTabCount - 1, InsertDeleteFlags::FORMULA, false,
                                         *pPasteUndoDoc);
        pPasteRefUndoDoc.reset();
    }

    ScUndoPasteOptions aOptions; // store options for repeat
    aOptions.nFunction = nFunction;
    aOptions.bSkipEmpty = bSkipEmpty;
    aOptions.bTranspose = bTranspose;
    aOptions.bAsLink = bAsLink;
    aOptions.eMoveMode = eMoveMode;

    pUndoPaste.reset(new ScUndoPaste(&*m_xDocShell, rDestRange, rDestMark, std::move(pPasteUndoDoc),
                                     std::move(pPasteRedoDoc), nUndoFlags, std::move(pUndoData),
                                     false,
                                     &aOptions)); // false = Redo data not yet copied
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
// InsertDeleteFlags::VALUE
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
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::VALUE, true, false);

    ScRange aDestRange(1, 1, destSheet, 2, 4, destSheet); // Paste to B2:C5 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(
        ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::VALUE | InsertDeleteFlags::FORMULA,
        pTransClip.get(), true, false /* false fixes tdf#141683 */, false, false);
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
// InsertDeleteFlags::VALUE
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
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::VALUE, true, false);

    printRange(&aClipDoc, ScRange(0, 0, 0, 4, 5, 0), "Base doc (&aClipDoc)");
    printRange(pTransClip.get(), ScRange(0, 0, 0, 3, 3, 0),
               "Transposed filtered clipdoc (pTransClip.get())");
    ScRange aDestRange(1, 1, destSheet, 3, 4, destSheet); // Paste to B2:D5 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(
        ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::VALUE | InsertDeleteFlags::FORMULA,
        pTransClip.get(), true, false /* false fixes tdf#141683 */, false, false);
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
// InsertDeleteFlags::VALUE
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
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::VALUE, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 2, destSheet); // Paste to B2:E3 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(
        ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::VALUE | InsertDeleteFlags::FORMULA,
        pTransClip.get(), true, false /* false fixes tdf#141683 */, false, false);
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
// InsertDeleteFlags::VALUE
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
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::VALUE, true, false);

    ScRange aDestRange(1, 1, destSheet, 4, 2, destSheet); // Paste to B2:E3 on Sheet2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyMultiRangeFromClip(
        ScAddress(1, 1, destSheet), aMark, InsertDeleteFlags::VALUE | InsertDeleteFlags::FORMULA,
        pTransClip.get(), true, false /* false fixes tdf#141683 */, false, false);
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

// Compatibility method since normal copy/paste tests do not test undo
void TestCopyPaste::executeCopyPasteSpecial(bool bApplyFilter, bool bIncludedFiltered, bool bAsLink,
                                            bool bTranspose, bool bMultiRangeSelection,
                                            bool bSkipEmpty, bool bCut,
                                            ScClipParam::Direction eDirection, CalcMode eCalcMode,
                                            InsertDeleteFlags aFlags)
{
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;
    executeCopyPasteSpecial(0, 1, bApplyFilter, bIncludedFiltered, bAsLink, bTranspose,
                            bMultiRangeSelection, bSkipEmpty, pUndoCut, pUndoPaste, bCut,
                            eDirection, eCalcMode, aFlags);
}

// This method is used to create the different copy/paste special test cases.
// Principle: Creation of test cases is parameterized, whereas checking uses a minimum of logic
void TestCopyPaste::executeCopyPasteSpecial(const SCTAB srcSheet, const SCTAB destSheet,
                                            bool bApplyFilter, bool bIncludedFiltered, bool bAsLink,
                                            bool bTranspose, bool bMultiRangeSelection,
                                            bool bSkipEmpty, std::unique_ptr<ScUndoCut>& pUndoCut,
                                            std::unique_ptr<ScUndoPaste>& pUndoPaste, bool bCut,
                                            ScClipParam::Direction eDirection, CalcMode eCalcMode,
                                            InsertDeleteFlags aFlags)
{
    // turn on/off auto calc
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, eCalcMode == AutoCalc);

    for (int i = 0; i < srcSheet; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));

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

    refs to cells (used for cut/paste tests)
    15   | =B3 | =$B$3    | =$B3 | =B$3| =SUM(B3:B3) | =SUM($B3:$B3) | =SUM($B3:$B3) | =SUM(B$3:B$3) | =SUM($A$1:$A$4) | =SUM($A$1:$A$8) |
    16   | =Range_B3 | =Range_aBa3    | =Range_aB3 | =Range_Ba3| =SUM(Range_B3_B3) | =SUM(Range_aBa3_aBa3) | =SUM(Range_aB3_aB3) | =SUM(Range_Ba3_Ba3) | =SUM(Range_aAa1_aAa4) | =SUM(Range_aAa1_aAa8) |

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

    // row 14, refs to copied/cut range
    m_pDoc->SetString(0, 14, srcSheet, "=B3");
    m_pDoc->SetString(1, 14, srcSheet, "=$B$3");
    m_pDoc->SetString(2, 14, srcSheet, "=$B3");
    m_pDoc->SetString(3, 14, srcSheet, "=B$3");
    m_pDoc->SetString(4, 14, srcSheet, "=SUM(B3:B3)");
    m_pDoc->SetString(5, 14, srcSheet, "=SUM($B$3:$B$3)");
    m_pDoc->SetString(6, 14, srcSheet, "=SUM($B3:$B3)");
    m_pDoc->SetString(7, 14, srcSheet, "=SUM(B$3:B$3)");
    m_pDoc->SetString(8, 14, srcSheet, "=SUM($A$1:$A$4)");
    m_pDoc->SetString(9, 14, srcSheet, "=SUM($A$1:$A$8)");

    // Cell position is used for ranges relative to current position
    ScAddress cellB4(1, 3, srcSheet);
    ScAddress cellA1(0, 0, srcSheet);
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B3", cellB4, "$SrcSheet.B3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aBa3", cellA1, "$SrcSheet.$B$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aB3", cellB4, "$SrcSheet.$B3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_Ba3", cellB4, "$SrcSheet.B$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B3_B3", cellB4, "$SrcSheet.B3:B3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aBa3_aBa3", cellA1, "$SrcSheet.$B$3:$B$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aB3_aB3", cellB4, "$SrcSheet.$B3:$B3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aB3_aB3", cellB4, "$SrcSheet.$B3:$B3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_Ba3_Ba3", cellB4, "$SrcSheet.B$3:B$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aAa1_aAa4", cellA1, "$SrcSheet.$A$1:$A$4"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_aAa1_aAa8", cellA1, "$SrcSheet.$A$1:$A$8"));

    // row 15, refs to copied/cut range using range
    m_pDoc->SetString(0, 15, srcSheet, "=Range_B3");
    m_pDoc->SetString(1, 15, srcSheet, "=Range_aBa3");
    m_pDoc->SetString(2, 15, srcSheet, "=Range_aB3");
    m_pDoc->SetString(3, 15, srcSheet, "=Range_Ba3");
    m_pDoc->SetString(4, 15, srcSheet, "=SUM(Range_B3_B3)");
    m_pDoc->SetString(5, 15, srcSheet, "=SUM(Range_aBa3_aBa3)");
    m_pDoc->SetString(6, 15, srcSheet, "=SUM(Range_aB3_aB3)");
    m_pDoc->SetString(7, 15, srcSheet, "=SUM(Range_Ba3_Ba3)");
    m_pDoc->SetString(8, 15, srcSheet, "=SUM(Range_aAa1_aAa4)");
    m_pDoc->SetString(9, 15, srcSheet, "=SUM(Range_aAa1_aAa8)");

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
    pNoteA1->SetText(aAdrA1, "Note A1");
    ScAddress aAdrB1(1, 0, srcSheet);
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAdrB1);
    pNoteB1->SetText(aAdrB1, "Note B1");
    // No note on C1
    ScAddress aAdrD1(3, 0, srcSheet);
    ScPostIt* pNoteD1 = m_pDoc->GetOrCreateNote(aAdrD1);
    pNoteD1->SetText(aAdrD1, "Note D1");
    // No note on E1
    // No note on F1

    // add notes row 1
    ScAddress aAdrA2(0, 1, srcSheet);
    ScPostIt* pNoteA2 = m_pDoc->GetOrCreateNote(aAdrA2);
    pNoteA2->SetText(aAdrA2, "Note A2");
    // No note on B2
    ScAddress aAdrC2(2, 1, srcSheet);
    ScPostIt* pNoteC2 = m_pDoc->GetOrCreateNote(aAdrC2);
    pNoteC2->SetText(aAdrC2, "Note C2");
    ScAddress aAdrD2(3, 1, srcSheet);
    ScPostIt* pNoteD2 = m_pDoc->GetOrCreateNote(aAdrD2);
    pNoteD2->SetText(aAdrD2, "Note D2");
    ScAddress aAdrE2(4, 2, srcSheet);
    ScPostIt* pNoteE2 = m_pDoc->GetOrCreateNote(aAdrE2);
    pNoteE2->SetText(aAdrE2, "Note E2");
    ScAddress aAdrF2(5, 1, srcSheet);
    ScPostIt* pNoteF2 = m_pDoc->GetOrCreateNote(aAdrF2);
    pNoteF2->SetText(aAdrF2, "Note F2");

    // add notes row 2
    ScAddress aAdrA3(0, 2, srcSheet);
    ScPostIt* pNoteA3 = m_pDoc->GetOrCreateNote(aAdrA3);
    pNoteA3->SetText(aAdrA3, "Note A3");
    ScAddress aAdrB3(1, 2, srcSheet);
    ScPostIt* pNoteB3 = m_pDoc->GetOrCreateNote(aAdrB3);
    pNoteB3->SetText(aAdrB3, "Note B3");
    ScAddress aAdrC3(2, 2, srcSheet);
    ScPostIt* pNoteC3 = m_pDoc->GetOrCreateNote(aAdrC3);
    pNoteC3->SetText(aAdrC3, "Note C3");
    ScAddress aAdrD3(3, 2, srcSheet);
    ScPostIt* pNoteD3 = m_pDoc->GetOrCreateNote(aAdrD3);
    pNoteD3->SetText(aAdrD3, "Note D3");
    // No note on E3
    // No note on F3

    // add notes row 3
    // No note on A4
    ScAddress aAdrB4(1, 3, srcSheet);
    ScPostIt* pNoteB4 = m_pDoc->GetOrCreateNote(aAdrB4);
    pNoteB4->SetText(aAdrB4, "Note B4");
    ScAddress aAdrC4(2, 3, srcSheet);
    ScPostIt* pNoteC4 = m_pDoc->GetOrCreateNote(aAdrC4);
    pNoteC4->SetText(aAdrC4, "Note C4");
    ScAddress aAdrD4(3, 3, srcSheet);
    ScPostIt* pNoteD4 = m_pDoc->GetOrCreateNote(aAdrD4);
    pNoteD4->SetText(aAdrD4, "Note D4");
    ScAddress aAdrE4(4, 3, srcSheet);
    ScPostIt* pNoteE4 = m_pDoc->GetOrCreateNote(aAdrE4);
    pNoteE4->SetText(aAdrE4, "Note E4");
    ScAddress aAdrF4(5, 3, srcSheet);
    ScPostIt* pNoteF4 = m_pDoc->GetOrCreateNote(aAdrF4);
    pNoteF4->SetText(aAdrF4, "Note F4");

    // row 4 for multi range row selection
    ScAddress aAdrC5(2, 4, srcSheet);
    ScPostIt* pNoteC5 = m_pDoc->GetOrCreateNote(aAdrC5);
    pNoteC5->SetText(aAdrC5, "Note C5");

    // Recalc if needed
    if (bMultiRangeSelection && bTranspose && eDirection == ScClipParam::Row
        && eCalcMode == RecalcAtEnd)
        m_pDoc->CalcFormulaTree();
    else if (bMultiRangeSelection && bTranspose && eDirection == ScClipParam::Row
             && eCalcMode == HardRecalcAtEnd)
        m_pDoc->CalcAll();

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
    // const SCTAB destSheet = 1;
    for (int i = srcSheet + 1; i < destSheet; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));

    if (srcSheet != destSheet)
        m_pDoc->InsertTab(destSheet, "DestSheet");

    m_pDoc->SetString(3, 101, srcSheet, "=DestSheet.D1");
    m_pDoc->SetString(3, 102, srcSheet, "=DestSheet.D2");
    m_pDoc->SetString(3, 103, srcSheet, "=DestSheet.D3");
    m_pDoc->SetString(3, 104, srcSheet, "=DestSheet.D4");
    m_pDoc->SetString(3, 105, srcSheet, "=DestSheet.D5");
    m_pDoc->SetString(3, 106, srcSheet, "=DestSheet.D6");
    m_pDoc->SetString(3, 107, srcSheet, "=DestSheet.D7");
    m_pDoc->SetString(4, 101, srcSheet, "=DestSheet.E1");
    m_pDoc->SetString(4, 102, srcSheet, "=DestSheet.E2");
    m_pDoc->SetString(4, 103, srcSheet, "=DestSheet.E3");
    m_pDoc->SetString(4, 104, srcSheet, "=DestSheet.E4");
    m_pDoc->SetString(4, 105, srcSheet, "=DestSheet.E5");
    m_pDoc->SetString(4, 106, srcSheet, "=DestSheet.E6");
    m_pDoc->SetString(4, 107, srcSheet, "=DestSheet.E7");
    m_pDoc->SetString(5, 101, srcSheet, "=DestSheet.F1");
    m_pDoc->SetString(5, 102, srcSheet, "=DestSheet.F2");
    m_pDoc->SetString(5, 103, srcSheet, "=DestSheet.F3");
    m_pDoc->SetString(5, 104, srcSheet, "=DestSheet.F4");
    m_pDoc->SetString(5, 105, srcSheet, "=DestSheet.F5");
    m_pDoc->SetString(5, 106, srcSheet, "=DestSheet.F6");
    m_pDoc->SetString(5, 107, srcSheet, "=DestSheet.F7");
    m_pDoc->SetString(6, 101, srcSheet, "=DestSheet.G1");
    m_pDoc->SetString(6, 102, srcSheet, "=DestSheet.G2");
    m_pDoc->SetString(6, 103, srcSheet, "=DestSheet.G3");
    m_pDoc->SetString(6, 104, srcSheet, "=DestSheet.G4");
    m_pDoc->SetString(6, 105, srcSheet, "=DestSheet.G5");
    m_pDoc->SetString(6, 106, srcSheet, "=DestSheet.G6");
    m_pDoc->SetString(6, 107, srcSheet, "=DestSheet.G7");
    m_pDoc->SetString(7, 101, srcSheet, "=DestSheet.H1");
    m_pDoc->SetString(7, 102, srcSheet, "=DestSheet.H2");
    m_pDoc->SetString(7, 103, srcSheet, "=DestSheet.H3");
    m_pDoc->SetString(7, 104, srcSheet, "=DestSheet.H4");
    m_pDoc->SetString(7, 105, srcSheet, "=DestSheet.H5");
    m_pDoc->SetString(7, 106, srcSheet, "=DestSheet.H6");
    m_pDoc->SetString(7, 107, srcSheet, "=DestSheet.H7");
    m_pDoc->SetString(8, 101, srcSheet, "=DestSheet.I1");
    m_pDoc->SetString(8, 102, srcSheet, "=DestSheet.I2");
    m_pDoc->SetString(8, 103, srcSheet, "=DestSheet.I3");
    m_pDoc->SetString(8, 104, srcSheet, "=DestSheet.I4");
    m_pDoc->SetString(8, 105, srcSheet, "=DestSheet.I5");
    m_pDoc->SetString(8, 106, srcSheet, "=DestSheet.I6");
    m_pDoc->SetString(8, 107, srcSheet, "=DestSheet.I7");
    m_pDoc->SetString(9, 101, srcSheet, "=DestSheet.J1");
    m_pDoc->SetString(9, 102, srcSheet, "=DestSheet.J2");
    m_pDoc->SetString(9, 103, srcSheet, "=DestSheet.J3");
    m_pDoc->SetString(9, 104, srcSheet, "=DestSheet.J4");
    m_pDoc->SetString(9, 105, srcSheet, "=DestSheet.J5");
    m_pDoc->SetString(9, 106, srcSheet, "=DestSheet.J6");
    m_pDoc->SetString(9, 107, srcSheet, "=DestSheet.J7");

    // Check precondition
    checkCopyPasteSpecialInitial(srcSheet);

    // set cells to 1000 to check empty cell behaviour and to detect destination range problems
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            m_pDoc->SetValue(i, j, destSheet, 1000);

    // transpose clipboard, paste on DestSheet
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());
    ScRange aDestRange;

    ScDocumentUniquePtr pPasteUndoDoc;
    std::unique_ptr<ScDocument> pPasteRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;

    if (!bMultiRangeSelection)
    {
        ScRange aSrcRange(0, 0, srcSheet, nSrcCols - 1, nSrcRows - 1, srcSheet);
        printRange(m_pDoc, aSrcRange, "Src range");
        if (!bCut)
            copyToClip(m_pDoc, aSrcRange, &aClipDoc);
        else
        {
            pUndoCut.reset(cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, true));
        }

        printRange(&aClipDoc, ScRange(0, 0, srcSheet, nSrcCols, nSrcRows, srcSheet),
                   "Base doc (&aClipDoc)");

        // ScDocument::TransposeClip() and ScDocument::CopyFromClip() calls
        // analog to ScViewFunc::PasteFromClip()
        if (bTranspose)
        {
            ScDocument* pOrigClipDoc = &aClipDoc;
            ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
            aClipDoc.TransposeClip(pTransClip.get(), aFlags, bAsLink, bIncludedFiltered);
            aDestRange = ScRange(3, 1, destSheet, 3 + nSrcRows - 1, 1 + nSrcCols - 1,
                                 destSheet); //target: D2:F6
            aDestMark.SetMarkArea(aDestRange);
            printRange(pTransClip.get(), ScRange(0, 0, srcSheet, nSrcCols, nSrcRows, srcSheet),
                       "Transposed clipdoc (pTransClip.get())");

            if (bCut)
                prepareUndoBeforePaste(bCut, pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange,
                                       pUndoData);

            m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, pPasteRefUndoDoc.get(),
                                 pTransClip.get(), true, bAsLink, bIncludedFiltered, bSkipEmpty);
            printRange(m_pDoc, aDestRange, "Transposed dest sheet");
            if (bCut)
            {
                m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark,
                                        pPasteRefUndoDoc.get());
                printRange(m_pDoc, aDestRange, "Transposed dest sheet after UpdateTranspose()");
            }
            pTransClip.reset();
        }
        else
        {
            aDestRange = ScRange(3, 1, destSheet, 3 + nSrcCols - 1, 1 + nSrcRows - 1,
                                 destSheet); //target: D2:I5
            aDestMark.SetMarkArea(aDestRange);
            if (bCut)
                prepareUndoBeforePaste(bCut, pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange,
                                       pUndoData);

            m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, pPasteRefUndoDoc.get(), &aClipDoc,
                                 true, bAsLink, bIncludedFiltered, bSkipEmpty);
        }

        if (bCut)
            prepareUndoAfterPaste(pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange, pUndoData,
                                  pUndoPaste, bTranspose, bAsLink, bSkipEmpty);
    }
    else // multi range selection
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
        m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aSrcMark, false, false);

        // ScDocument::TransposeClip() and ScDocument::CopyMultiRangeFromClip() calls
        // analog to ScViewFunc::PasteFromClipToMultiRanges()
        if (bTranspose)
        {
            printRange(m_pDoc, aClipParam.getWholeRange(), "Src range");
            ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
            aClipDoc.TransposeClip(pTransClip.get(), aFlags, bAsLink, bIncludedFiltered);
            aDestRange = ScRange(3, 1, destSheet, 3 + nSrcRows - 1, 1 + nSrcCols - 1 - 1,
                                 destSheet); //target col: D2:G6, target row: D2:H6
            aDestMark.SetMarkArea(aDestRange);
            printRange(&aClipDoc, ScRange(0, 0, srcSheet, nSrcCols, nSrcRows, srcSheet),
                       "Base doc (&aClipDoc)");
            printRange(pTransClip.get(), ScRange(0, 0, srcSheet, nSrcCols, nSrcRows, srcSheet),
                       "Transposed clipdoc (pTransClip.get())");
            m_pDoc->CopyMultiRangeFromClip(ScAddress(3, 1, destSheet), aDestMark, aFlags,
                                           pTransClip.get(), true, bAsLink && !bTranspose,
                                           bIncludedFiltered, bSkipEmpty);
            pTransClip.reset();
            printRange(m_pDoc, aDestRange, "Transposed dest sheet");
        }
        else
        {
            aDestRange = ScRange(3, 1, destSheet, 3 + nSrcCols - 1 - 1, 1 + nSrcRows - 1,
                                 destSheet); //target col: D2:I5, target row: D2:I6
            aDestMark.SetMarkArea(aDestRange);
            m_pDoc->CopyMultiRangeFromClip(ScAddress(3, 1, destSheet), aDestMark, aFlags, &aClipDoc,
                                           true, bAsLink && !bTranspose, bIncludedFiltered,
                                           bSkipEmpty);
        }
    }

    if (eCalcMode == RecalcAtEnd)
        m_pDoc->CalcFormulaTree();
    else if (eCalcMode == HardRecalcAtEnd)
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
    executeCopyPasteSpecial(false, false, false, false, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeColIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFiltered(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeColTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFilteredTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeColFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, false, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeRow()
{
    executeCopyPasteSpecial(false, false, false, false, true, false, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeRowIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, false, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, false, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRowFiltered(false);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, false, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(false);
}

void TestCopyPaste::testCopyPasteSpecialMultiRangeRowTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, false, false, ScClipParam::Row,
                            HardRecalcAtEnd);
    checkCopyPasteSpecialMultiRangeRowTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, true, false, false, ScClipParam::Row,
                            HardRecalcAtEnd);
    checkCopyPasteSpecialMultiRangeRowFilteredTranspose(false);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialMultiRangeRowFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, false, false, ScClipParam::Row,
                            HardRecalcAtEnd);
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
    executeCopyPasteSpecial(false, false, false, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFiltered(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeCol(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, false, false, true, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColFilteredTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeColFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, true, false, ScClipParam::Column);
    checkCopyPasteSpecialMultiRangeColTranspose(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRow()
{
    executeCopyPasteSpecial(false, false, false, false, true, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(false, true, false, false, true, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFiltered()
{
    executeCopyPasteSpecial(true, false, false, false, true, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRowFiltered(true);
}

// tdf#45958
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFiltered()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, false, true, true, false, ScClipParam::Row);
    checkCopyPasteSpecialMultiRangeRow(true);
}

void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowTranspose()
{
    executeCopyPasteSpecial(false, false, false, true, true, true, false, ScClipParam::Row,
                            HardRecalcAtEnd);
    checkCopyPasteSpecialMultiRangeRowTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredTranspose()
{
    executeCopyPasteSpecial(true, false, false, true, true, true, false, ScClipParam::Row,
                            HardRecalcAtEnd);
    checkCopyPasteSpecialMultiRangeRowFilteredTranspose(true);
}

// tdf#45958, tdf#107348
void TestCopyPaste::testCopyPasteSpecialSkipEmptyMultiRangeRowFilteredIncludeFilteredTranspose()
{
    // For bIncludeFiltered=true, the non-filtered outcome is expected
    executeCopyPasteSpecial(true, true, false, true, true, true, false, ScClipParam::Row,
                            HardRecalcAtEnd);
    checkCopyPasteSpecialMultiRangeRowTranspose(true);
}

void TestCopyPaste::testCutPasteSpecial()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;

    executeCopyPasteSpecial(srcSheet, destSheet, false, true, false, false, false, false, pUndoCut,
                            pUndoPaste, true);
    checkCopyPasteSpecial(false, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkCopyPasteSpecial(false, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

void TestCopyPaste::testCutPasteSpecialTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;

    executeCopyPasteSpecial(srcSheet, destSheet, false, true, false, true, false, false, pUndoCut,
                            pUndoPaste, true);
    checkCopyPasteSpecialTranspose(false, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkCopyPasteSpecialTranspose(false, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

void TestCopyPaste::testCutPasteSpecialSkipEmpty()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;

    executeCopyPasteSpecial(srcSheet, destSheet, false, true, false, false, false, true, pUndoCut,
                            pUndoPaste, true);
    checkCopyPasteSpecial(true, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkCopyPasteSpecial(true, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

void TestCopyPaste::testCutPasteSpecialSkipEmptyTranspose()
{
    const SCTAB srcSheet = 0;
    const SCTAB destSheet = 1;
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;

    executeCopyPasteSpecial(srcSheet, destSheet, false, true, false, true, false, true, pUndoCut,
                            pUndoPaste, true);
    checkCopyPasteSpecialTranspose(true, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkCopyPasteSpecialTranspose(true, true);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkCopyPasteSpecialInitial(srcSheet);

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

// check initial source
void TestCopyPaste::checkCopyPasteSpecialInitial(const SCTAB srcSheet)
{
    OUString aString;
    double fValue;
    const EditTextObject* pEditObj;
    // col 0
    ASSERT_DOUBLES_EQUAL(1, m_pDoc->GetValue(0, 0, srcSheet));
    ASSERT_DOUBLES_EQUAL(2, m_pDoc->GetValue(0, 1, srcSheet));
    ASSERT_DOUBLES_EQUAL(3, m_pDoc->GetValue(0, 2, srcSheet));
    ASSERT_DOUBLES_EQUAL(4, m_pDoc->GetValue(0, 3, srcSheet));
    // col 1, formulas
    ASSERT_DOUBLES_EQUAL(11, m_pDoc->GetValue(1, 0, srcSheet));
    m_pDoc->GetFormula(1, 0, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=A1+10"), aString);
    m_pDoc->GetFormula(1, 1, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=A2+20"), aString);
    ASSERT_DOUBLES_EQUAL(22, m_pDoc->GetValue(1, 1, srcSheet));
    ASSERT_DOUBLES_EQUAL(35, m_pDoc->GetValue(1, 2, srcSheet));
    m_pDoc->GetFormula(1, 2, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=D3+30"), aString);
    ASSERT_DOUBLES_EQUAL(42, m_pDoc->GetValue(1, 3, srcSheet));
    m_pDoc->GetFormula(1, 3, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=A2+40"), aString);
    // col 2, strings
    aString = m_pDoc->GetString(2, 0, srcSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aString);
    aString = m_pDoc->GetString(2, 1, srcSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), aString);
    aString = m_pDoc->GetString(2, 2, srcSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aString);
    aString = m_pDoc->GetString(2, 3, srcSheet);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), aString);
    // col 3, rich text
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 0, srcSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R1"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 1, srcSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R2"), pEditObj->GetText(0));
    ASSERT_DOUBLES_EQUAL(5, m_pDoc->GetValue(3, 2, srcSheet));
    pEditObj = m_pDoc->GetEditText(ScAddress(3, 3, srcSheet));
    CPPUNIT_ASSERT(pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("R4"), pEditObj->GetText(0));
    // col 4, formulas
    m_pDoc->GetFormula(4, 0, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=A1+A3+60"), aString);
    ASSERT_DOUBLES_EQUAL(64, m_pDoc->GetValue(4, 0, srcSheet));
    aString = m_pDoc->GetString(4, 1, srcSheet);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    aString = m_pDoc->GetString(4, 2, srcSheet);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    fValue = m_pDoc->GetValue(4, 3, srcSheet);
    m_pDoc->GetFormula(4, 3, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=A1+A3+70"), aString);
    ASSERT_DOUBLES_EQUAL(74, fValue);
    // col 5, formulas
    m_pDoc->GetFormula(5, 0, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIF(A1:A4;\"<4\")"), aString);
    ASSERT_DOUBLES_EQUAL(6, m_pDoc->GetValue(5, 0, srcSheet));
    aString = m_pDoc->GetString(5, 1, srcSheet);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    aString = m_pDoc->GetString(5, 2, srcSheet);
    CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, aString);
    fValue = m_pDoc->GetValue(5, 3, srcSheet);
    m_pDoc->GetFormula(5, 3, srcSheet, aString);
    CPPUNIT_ASSERT_EQUAL(OUString("=B$1+$A$3+80"), aString);
    ASSERT_DOUBLES_EQUAL(94, fValue);

    // check patterns
    const SfxPoolItem* pItem = nullptr;
    m_pDoc->GetPattern(ScAddress(0, 0, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(0, 1, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(0, 2, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<const SvxBrushItem*>(pItem)->GetColor());
    m_pDoc->GetPattern(ScAddress(0, 3, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(0, 4, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(!pItem);
    m_pDoc->GetPattern(ScAddress(4, 2, srcSheet))->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT_EQUAL(COL_GREEN, static_cast<const SvxBrushItem*>(pItem)->GetColor());

    // check border, left and right borders were transformed to top and bottom borders
    pItem = m_pDoc->GetAttr(ScAddress(1, 0, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(1, 1, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(1, 2, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(1, 3, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(static_cast<const SvxBoxItem*>(pItem)->GetRight());
    pItem = m_pDoc->GetAttr(ScAddress(1, 4, srcSheet), ATTR_BORDER);
    CPPUNIT_ASSERT(pItem);
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetTop());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetBottom());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetLeft());
    CPPUNIT_ASSERT(!static_cast<const SvxBoxItem*>(pItem)->GetRight());

    // check notes after transposed copy/paste
    // check presence of notes
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(0, 0, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(1, 0, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(2, 0, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 0, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 0, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 0, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 0, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(0, 1, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(1, 1, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(2, 1, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 1, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 1, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 1, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 1, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(0, 2, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(1, 2, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(2, 2, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 2, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 2, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 2, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 2, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(0, 3, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(1, 3, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(2, 3, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(3, 3, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(4, 3, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(5, 3, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 3, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(0, 4, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(1, 4, srcSheet)));
    CPPUNIT_ASSERT(m_pDoc->HasNote(ScAddress(2, 4, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(3, 4, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(4, 4, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(5, 4, srcSheet)));
    CPPUNIT_ASSERT(!m_pDoc->HasNote(ScAddress(6, 4, srcSheet)));

    // check values of notes
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(0, 0, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A2"),
                         m_pDoc->GetNote(ScAddress(0, 1, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(0, 2, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(1, 0, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(1, 2, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C2"),
                         m_pDoc->GetNote(ScAddress(2, 1, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C3"),
                         m_pDoc->GetNote(ScAddress(2, 2, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(3, 0, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D2"),
                         m_pDoc->GetNote(ScAddress(3, 1, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(3, 2, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                         m_pDoc->GetNote(ScAddress(4, 2, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(4, 3, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F2"),
                         m_pDoc->GetNote(ScAddress(5, 1, srcSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(5, 3, srcSheet))->GetText());

    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));
}

// Base check, nothing filtered, nothing transposed
void TestCopyPaste::checkCopyPasteSpecial(bool bSkipEmpty, bool bCut)
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
    if (!bCut)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=E$1+$A$3+80"), aString);
        ASSERT_DOUBLES_EQUAL(2080, fValue);
    }
    else
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=E$2+$D$4+80"), aString);
        ASSERT_DOUBLES_EQUAL(94, fValue);
    }
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
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A2"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C2"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C3"),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D2"),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(7, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note F2"),
                             m_pDoc->GetNote(ScAddress(8, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(8, 4, destSheet))->GetText());

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

    // row 14 on src sheet, refs to copied/cut range
    if (!bCut)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

        m_pDoc->DeleteTab(destSheet);
        m_pDoc->DeleteTab(srcSheet);
    }
    else
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.$E$4"), getFormula(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.$E4"), getFormula(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E$4"), getFormula(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.E4:E4)"), getFormula(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.$E$4:$E$4)"), getFormula(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.$E4:$E4)"), getFormula(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.E$4:E$4)"), getFormula(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.$D$2:$D$5)"), getFormula(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-27.0, m_pDoc->GetValue(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-27.0, m_pDoc->GetValue(9, 15, srcSheet));
    }
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
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C3"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(7, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(8, 3, destSheet))->GetText());

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::checkCopyPasteSpecialTranspose(bool bSkipEmpty, bool bCut)
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
    if (!bCut)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula G7", OUString("=C$1+$A$3+80"), aString);
        ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula G7", 2080, fValue);
    }
    else
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=D$3+$F$2+80"), aString);
        ASSERT_DOUBLES_EQUAL(94, fValue);
    }
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2", OUString("Note A1"),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2", OUString("Note A2"),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F2", OUString("Note A3"),
                                 m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    // G2 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3", OUString("Note B1"),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    // E3 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F3", OUString("Note B3"),
                                 m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    // D4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4", OUString("Note C2"),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F4", OUString("Note C3"),
                                 m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5", OUString("Note D1"),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5", OUString("Note D2"),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F5", OUString("Note D3"),
                                 m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on G6", OUString("Note E4"),
                                 m_pDoc->GetNote(ScAddress(6, 5, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E7", OUString("Note F2"),
                                     m_pDoc->GetNote(ScAddress(4, 6, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on G7", OUString("Note F4"),
                                 m_pDoc->GetNote(ScAddress(6, 6, destSheet))->GetText());

    // row 14 on src sheet, refs to copied/cut range
    if (!bCut)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

        // Existing references to the destination range must not change
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

        m_pDoc->DeleteTab(destSheet);
        m_pDoc->DeleteTab(srcSheet);
    }
    else
    {
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.$F$3"), getFormula(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.$F3"), getFormula(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F$3"), getFormula(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.F3:F3)"), getFormula(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.$F$3:$F$3)"), getFormula(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.$F3:$F3)"), getFormula(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.F$3:F$3)"), getFormula(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(DestSheet.$D$2:$G$2)"), getFormula(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-27.0, m_pDoc->GetValue(9, 14, srcSheet));

        CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(3, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
        CPPUNIT_ASSERT_EQUAL(-27.0, m_pDoc->GetValue(9, 15, srcSheet));

        // Existing references to the destination range must not change
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
        CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));
    }
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2", OUString("Note A1"),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2", OUString("Note A3"),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    // F2 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on G2", OUString("Note A1"),
                                 m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3", OUString("Note B1"),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E3", OUString("Note B3"),
                                 m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F3", OUString("Note B4"),
                                 m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on G3", OUString("Note B1"),
                                 m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    // D4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4", OUString("Note C3"),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F4", OUString("Note C4"),
                                 m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    // G4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5", OUString("Note D1"),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5", OUString("Note D3"),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F5", OUString("Note D4"),
                                 m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on G5", OUString("Note D1"),
                                 m_pDoc->GetNote(ScAddress(6, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(4, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F6", OUString("Note E4"),
                                 m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F7", OUString("Note F4"),
                                 m_pDoc->GetNote(ScAddress(5, 6, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A2"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D2"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(6, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note F2"),
                             m_pDoc->GetNote(ScAddress(7, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(7, 4, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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

    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A2"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D2"),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(6, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note F2"),
                             m_pDoc->GetNote(ScAddress(4, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(6, 5, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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

    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note E4"),
                         m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note F4"),
                         m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A2"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(3, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C2"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C3"),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D2"),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(7, 3, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note F2"),
                             m_pDoc->GetNote(ScAddress(8, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C5"),
                         m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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
    CPPUNIT_ASSERT_EQUAL(OUString("Note A1"),
                         m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note A3"),
                         m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B1"),
                         m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note B3"),
                         m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C3"),
                         m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D1"),
                         m_pDoc->GetNote(ScAddress(6, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note D3"),
                         m_pDoc->GetNote(ScAddress(6, 2, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(7, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C5"),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2", OUString("Note A1"),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2", OUString("Note A2"),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F2", OUString("Note A3"),
                                 m_pDoc->GetNote(ScAddress(5, 1, destSheet))->GetText());
    // G2 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3", OUString("Note B1"),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    // E3 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F3", OUString("Note B3"),
                                 m_pDoc->GetNote(ScAddress(5, 2, destSheet))->GetText());
    // D4 has no note
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4", OUString("Note C2"),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on F4", OUString("Note C3"),
                                 m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5", OUString("Note D1"),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5", OUString("Note D2"),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on F5", OUString("Note D3"),
                                 m_pDoc->GetNote(ScAddress(5, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(5, 5, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E7", OUString("Note F2"),
                                     m_pDoc->GetNote(ScAddress(4, 6, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C5"),
                         m_pDoc->GetNote(ScAddress(6, 3, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D2", OUString("Note A1"),
                                 m_pDoc->GetNote(ScAddress(3, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E2", OUString("Note A3"),
                                 m_pDoc->GetNote(ScAddress(4, 1, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on D3", OUString("Note B1"),
                                 m_pDoc->GetNote(ScAddress(3, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E3", OUString("Note B3"),
                                 m_pDoc->GetNote(ScAddress(4, 2, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong content of cell note on E4", OUString("Note C3"),
                                 m_pDoc->GetNote(ScAddress(4, 3, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on D5", OUString("Note D1"),
                                 m_pDoc->GetNote(ScAddress(3, 4, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on E5", OUString("Note D3"),
                                 m_pDoc->GetNote(ScAddress(4, 4, destSheet))->GetText());
    if (!bSkipEmpty)
        CPPUNIT_ASSERT_EQUAL(OUString("Note E2"),
                             m_pDoc->GetNote(ScAddress(4, 5, destSheet))->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Note C5"),
                         m_pDoc->GetNote(ScAddress(5, 3, destSheet))->GetText());

    // check row 14 on src sheet, refs to copied/cut range
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B3"), getFormula(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=B$3"), getFormula(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:B3)"), getFormula(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$B$3)"), getFormula(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B3:$B3)"), getFormula(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B$3:B$3)"), getFormula(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$4)"), getFormula(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($A$1:$A$8)"), getFormula(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(3, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(7, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 14, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 14, srcSheet));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aBa3"), getFormula(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_aB3"), getFormula(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_Ba3"), getFormula(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_B3)"), getFormula(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aBa3_aBa3)"), getFormula(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aB3_aB3)"), getFormula(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_Ba3_Ba3)"), getFormula(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa4)"), getFormula(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_aAa1_aAa8)"), getFormula(9, 15, srcSheet));

    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(0, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(1, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(2, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(3, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(4, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(5, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(35.0, m_pDoc->GetValue(6, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(123.0, m_pDoc->GetValue(7, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(8, 15, srcSheet));
    CPPUNIT_ASSERT_EQUAL(-17.0, m_pDoc->GetValue(9, 15, srcSheet));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D1"), getFormula(3, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D2"), getFormula(3, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D3"), getFormula(3, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D4"), getFormula(3, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D5"), getFormula(3, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D6"), getFormula(3, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.D7"), getFormula(3, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E1"), getFormula(4, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E2"), getFormula(4, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E3"), getFormula(4, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E4"), getFormula(4, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E5"), getFormula(4, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E6"), getFormula(4, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.E7"), getFormula(4, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F1"), getFormula(5, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F2"), getFormula(5, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F3"), getFormula(5, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F4"), getFormula(5, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F5"), getFormula(5, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F6"), getFormula(5, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.F7"), getFormula(5, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G1"), getFormula(6, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G2"), getFormula(6, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G3"), getFormula(6, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G4"), getFormula(6, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G5"), getFormula(6, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G6"), getFormula(6, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.G7"), getFormula(6, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H1"), getFormula(7, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H2"), getFormula(7, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H3"), getFormula(7, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H4"), getFormula(7, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H5"), getFormula(7, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H6"), getFormula(7, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.H7"), getFormula(7, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I1"), getFormula(8, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I2"), getFormula(8, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I3"), getFormula(8, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I4"), getFormula(8, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I5"), getFormula(8, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I6"), getFormula(8, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.I7"), getFormula(8, 107, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J1"), getFormula(9, 101, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J2"), getFormula(9, 102, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J3"), getFormula(9, 103, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J4"), getFormula(9, 104, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J5"), getFormula(9, 105, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J6"), getFormula(9, 106, srcSheet));
    CPPUNIT_ASSERT_EQUAL(OUString("=DestSheet.J7"), getFormula(9, 107, srcSheet));

    m_pDoc->DeleteTab(destSheet);
    m_pDoc->DeleteTab(srcSheet);
}

void TestCopyPaste::testTdf142201Row()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetValue(0, 1, nTab, 2.0); // A2
    m_pDoc->SetValue(1, 0, nTab, 11.0); // B1
    m_pDoc->SetValue(1, 1, nTab, 12.0); // B2

    m_pDoc->SetString(0, 3, nTab, "=A1"); // A4
    m_pDoc->SetString(0, 4, nTab, "=A2"); // A5
    m_pDoc->SetString(1, 3, nTab, "=B1"); // B4
    m_pDoc->SetString(1, 4, nTab, "=B2"); // B5

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));

    ScRange aReferencesRange(0, 3, nTab, 1, 4, nTab);
    printRange(m_pDoc, ScRange(0, 0, nTab, 1, 1, nTab), "Source");
    printRange(m_pDoc, aReferencesRange, "References");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);

    // Cut A1:A2 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(0, 0, nTab, 0, 1, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);

    // To B7:C7
    ScRange aDestRange(1, 6, nTab, 2, 6, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    printRange(m_pDoc, aReferencesRange, "References after cut");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    printRange(m_pDoc, aReferencesRange, "References after cut transposed");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);

    // Check results
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: 11
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=B7"), getFormula(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=C7"), getFormula(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B2"), getFormula(1, 4, nTab));
}

void TestCopyPaste::testTdf142201ColRel()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetValue(0, 1, nTab, 2.0); // A2
    m_pDoc->SetValue(1, 0, nTab, 11.0); // B1
    m_pDoc->SetValue(1, 1, nTab, 12.0); // B2

    m_pDoc->SetString(0, 3, nTab, "=A1"); // A4
    m_pDoc->SetString(0, 4, nTab, "=A2"); // A5
    m_pDoc->SetString(1, 3, nTab, "=B1"); // B4
    m_pDoc->SetString(1, 4, nTab, "=B2"); // B5

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));

    ScRange aReferencesRange(0, 3, nTab, 1, 4, nTab);
    printRange(m_pDoc, ScRange(0, 0, nTab, 1, 1, nTab), "Source");
    printRange(m_pDoc, aReferencesRange, "References");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);

    // Cut values A1:B1 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(0, 0, nTab, 1, 0, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);

    // To B7:B8
    ScRange aDestRange(1, 6, nTab, 1, 7, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    printRange(m_pDoc, aReferencesRange, "References after paste");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    printRange(m_pDoc, aReferencesRange, "References after paste transposed");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);

    // Check results
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: 2
    // - Actual  : 11
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=B7"), getFormula(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B8"), getFormula(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B2"), getFormula(1, 4, nTab));
}

void TestCopyPaste::testTdf142201ColAbs()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetValue(0, 1, nTab, 2.0); // A2
    m_pDoc->SetValue(1, 0, nTab, 11.0); // B1
    m_pDoc->SetValue(1, 1, nTab, 12.0); // B2

    m_pDoc->SetString(0, 3, nTab, "=$A$1"); // A4
    m_pDoc->SetString(0, 4, nTab, "=$A$2"); // A5
    m_pDoc->SetString(1, 3, nTab, "=$B$1"); // B4
    m_pDoc->SetString(1, 4, nTab, "=$B$2"); // B5

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));

    ScRange aReferencesRange(0, 3, nTab, 1, 4, nTab);
    printRange(m_pDoc, ScRange(0, 0, nTab, 1, 1, nTab), "Source");
    printRange(m_pDoc, aReferencesRange, "References");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);

    // Cut values A1:B1 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(0, 0, nTab, 1, 0, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);

    // To B7:B8
    ScRange aDestRange(1, 6, nTab, 1, 7, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    printRange(m_pDoc, aReferencesRange, "References after paste");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    printRange(m_pDoc, aReferencesRange, "References after paste transposed");
    printFormula(m_pDoc, 0, 3, nTab);
    printFormula(m_pDoc, 0, 4, nTab);
    printFormula(m_pDoc, 1, 3, nTab);
    printFormula(m_pDoc, 1, 4, nTab);

    // Check results
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: 2
    // - Actual  : 11
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=$B$7"), getFormula(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=$A$2"), getFormula(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$8"), getFormula(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$2"), getFormula(1, 4, nTab));
}

void TestCopyPaste::checkReferencedCutRangesRowIntitial(const SCTAB nSrcTab, const OUString& rDesc)
{
    printRange(m_pDoc, ScRange(1, 1, nSrcTab, 3, 2, nSrcTab), rDesc.toUtf8() + ": Source");
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(1, 1, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(2, 1, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(3, 1, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(3, 2, nSrcTab));

    // Guards
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(0, 0, nSrcTab)); // A1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(1, 0, nSrcTab)); // B1
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(2, 0, nSrcTab)); // C1
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(3, 0, nSrcTab)); // D1
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(4, 0, nSrcTab)); // E1
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(0, 1, nSrcTab)); // A2
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(4, 1, nSrcTab)); // E2
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(0, 2, nSrcTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(4, 2, nSrcTab)); // E3
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(0, 3, nSrcTab)); // A4
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(1, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(2, 3, nSrcTab)); // C4
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(3, 3, nSrcTab)); // D4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(4, 3, nSrcTab)); // E4
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(24, 0, nSrcTab)); // Y1
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(24, 1, nSrcTab)); // Y2
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(24, 2, nSrcTab)); // Y3
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(21, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(22, 3, nSrcTab)); // W4
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(24, 3, nSrcTab)); // Y4
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(OUString("=D1"), getFormula(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(OUString("=E1"), getFormula(24, 0, nSrcTab)); // Y1
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(OUString("=E2"), getFormula(24, 1, nSrcTab)); // Y2
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(OUString("=E3"), getFormula(24, 2, nSrcTab)); // Y3
    CPPUNIT_ASSERT_EQUAL(OUString("=A4"), getFormula(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(OUString("=B4"), getFormula(21, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(OUString("=C4"), getFormula(22, 3, nSrcTab)); // W4
    CPPUNIT_ASSERT_EQUAL(OUString("=D4"), getFormula(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(OUString("=E4"), getFormula(24, 3, nSrcTab)); // Y4

    for (int i = 10; i < 20; ++i)
        for (int j = 0; j < 10; ++j)
        {
            CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(j, i, nSrcTab));
            CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, getFormula(j, i, nSrcTab));
        }

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 20, nSrcTab, 2, 21, nSrcTab),
                                      rDesc.toUtf8() + ": Relative references");
    CPPUNIT_ASSERT_EQUAL(OUString("=B2"), getFormula(0, 20, nSrcTab)); // A21
    CPPUNIT_ASSERT_EQUAL(OUString("=C2"), getFormula(1, 20, nSrcTab)); // B21
    CPPUNIT_ASSERT_EQUAL(OUString("=D2"), getFormula(2, 20, nSrcTab)); // C21
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 21, nSrcTab)); // A22
    CPPUNIT_ASSERT_EQUAL(OUString("=C3"), getFormula(1, 21, nSrcTab)); // B22
    CPPUNIT_ASSERT_EQUAL(OUString("=D3"), getFormula(2, 21, nSrcTab)); // C22
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 21, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 30, nSrcTab, 2, 31, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute references");
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$2"), getFormula(0, 30, nSrcTab)); // A31
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$2"), getFormula(1, 30, nSrcTab)); // B31
    CPPUNIT_ASSERT_EQUAL(OUString("=$D$2"), getFormula(2, 30, nSrcTab)); // C31
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(0, 31, nSrcTab)); // A32
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$3"), getFormula(1, 31, nSrcTab)); // B32
    CPPUNIT_ASSERT_EQUAL(OUString("=$D$3"), getFormula(2, 31, nSrcTab)); // C32
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 31, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2"), getRangeByName("Range_B2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2"), getRangeByName("Range_C2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$D$2"), getRangeByName("Range_D2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3"), getRangeByName("Range_B3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$3"), getRangeByName("Range_C3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$D$3"), getRangeByName("Range_D3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$D$2"), getRangeByName("Range_B2_D2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3:$D$3"), getRangeByName("Range_B3_D3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$D$3"), getRangeByName("Range_B2_D3"));
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), getRangeByName("RelRange_Cm20_R0"));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 40, nSrcTab, 2, 41, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute ranges");
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B2"), getFormula(0, 40, nSrcTab)); // A41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C2"), getFormula(1, 40, nSrcTab)); // B41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_D2"), getFormula(2, 40, nSrcTab)); // C41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 41, nSrcTab)); // A42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C3"), getFormula(1, 41, nSrcTab)); // B42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_D3"), getFormula(2, 41, nSrcTab)); // C42
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 41, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 50, nSrcTab, 2, 51, nSrcTab),
                                      rDesc.toUtf8() + ": Relative ranges");
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 50, nSrcTab)); // A51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 50, nSrcTab)); // B51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(2, 50, nSrcTab)); // C51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 51, nSrcTab)); // A52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 51, nSrcTab)); // B52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(2, 51, nSrcTab)); // C52
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 51, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 60, nSrcTab, 2, 61, nSrcTab),
                                      rDesc.toUtf8() + ": Relative sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B2:D2)"), getFormula(0, 60, nSrcTab)); // A61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B2:D2)"), getFormula(1, 60, nSrcTab)); // B61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B2:D2)"), getFormula(2, 60, nSrcTab)); // C61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(0, 61, nSrcTab)); // A62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(1, 61, nSrcTab)); // B62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(2, 61, nSrcTab)); // C62
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 61, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 70, nSrcTab, 2, 71, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$2)"), getFormula(0, 70, nSrcTab)); // A71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$2)"), getFormula(1, 70, nSrcTab)); // B71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$2)"), getFormula(2, 70, nSrcTab)); // C71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(0, 71, nSrcTab)); // A72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(1, 71, nSrcTab)); // B72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(2, 71, nSrcTab)); // C72
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 71, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 80, nSrcTab, 2, 81, nSrcTab),
                                      rDesc.toUtf8() + ": Relative range sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(0, 80, nSrcTab)); // A81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(1, 80, nSrcTab)); // B81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(2, 80, nSrcTab)); // C81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(0, 81, nSrcTab)); // A82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(1, 81, nSrcTab)); // B82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(2, 81, nSrcTab)); // C82
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 81, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 90, nSrcTab, 2, 91, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(0, 90, nSrcTab)); // A91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(1, 90, nSrcTab)); // B91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(2, 90, nSrcTab)); // C91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(0, 91, nSrcTab)); // A92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(1, 91, nSrcTab)); // B92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(2, 91, nSrcTab)); // C92
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(2, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(2, 91, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 100, nSrcTab, 2, 101, nSrcTab),
                                      rDesc.toUtf8() + ": Relative range sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(0, 100, nSrcTab)); // A101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(1, 100, nSrcTab)); // B101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(2, 100, nSrcTab)); // C101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(0, 101, nSrcTab)); // A102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(1, 101, nSrcTab)); // B102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(2, 101, nSrcTab)); // C102
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(2, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(2, 101, nSrcTab));
}

void TestCopyPaste::executeReferencedCutRangesRow(const bool bTransposed, const SCTAB nSrcTab,
                                                  const SCTAB nDestTab, const bool bUndo,
                                                  std::unique_ptr<ScUndoCut>& pUndoCut,
                                                  std::unique_ptr<ScUndoPaste>& pUndoPaste)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    for (int i = 0; i < nSrcTab; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));
    m_pDoc->InsertTab(nSrcTab, "Test");

    m_pDoc->SetValue(1, 1, nSrcTab, 01.0); // B2  \.
    m_pDoc->SetValue(2, 1, nSrcTab, 11.0); // C2   | cut
    m_pDoc->SetValue(3, 1, nSrcTab, 21.0); // D2  /
    m_pDoc->SetValue(1, 2, nSrcTab, 02.0); // B3
    m_pDoc->SetValue(2, 2, nSrcTab, 12.0); // C3
    m_pDoc->SetValue(3, 2, nSrcTab, 22.0); // D3
    printRange(m_pDoc, ScRange(1, 1, nSrcTab, 3, 2, nSrcTab), "Source");

    // Guard values
    m_pDoc->SetValue(0, 0, nSrcTab, 1000.0); // A1
    m_pDoc->SetValue(1, 0, nSrcTab, 1001.0); // B1
    m_pDoc->SetValue(2, 0, nSrcTab, 1002.0); // C1
    m_pDoc->SetValue(3, 0, nSrcTab, 1003.0); // D1
    m_pDoc->SetValue(4, 0, nSrcTab, 1004.0); // E1
    m_pDoc->SetValue(0, 1, nSrcTab, 1010.0); // A2
    m_pDoc->SetValue(4, 1, nSrcTab, 1014.0); // E2
    m_pDoc->SetValue(0, 2, nSrcTab, 1020.0); // A3
    m_pDoc->SetValue(4, 2, nSrcTab, 1024.0); // E3
    m_pDoc->SetValue(0, 3, nSrcTab, 1030.0); // A4
    m_pDoc->SetValue(1, 3, nSrcTab, 1031.0); // B4
    m_pDoc->SetValue(2, 3, nSrcTab, 1032.0); // C4
    m_pDoc->SetValue(3, 3, nSrcTab, 1033.0); // D4
    m_pDoc->SetValue(4, 3, nSrcTab, 1034.0); // E4

    m_pDoc->SetString(20, 0, nSrcTab, "=A1"); // U1
    m_pDoc->SetString(21, 0, nSrcTab, "=B1"); // V1
    m_pDoc->SetString(22, 0, nSrcTab, "=C1"); // W1
    m_pDoc->SetString(23, 0, nSrcTab, "=D1"); // X1
    m_pDoc->SetString(24, 0, nSrcTab, "=E1"); // Y1
    m_pDoc->SetString(20, 1, nSrcTab, "=A2"); // U2
    m_pDoc->SetString(24, 1, nSrcTab, "=E2"); // Y2
    m_pDoc->SetString(20, 2, nSrcTab, "=A3"); // U3
    m_pDoc->SetString(24, 2, nSrcTab, "=E3"); // Y3
    m_pDoc->SetString(20, 3, nSrcTab, "=A4"); // U4
    m_pDoc->SetString(21, 3, nSrcTab, "=B4"); // B4
    m_pDoc->SetString(22, 3, nSrcTab, "=C4"); // W4
    m_pDoc->SetString(23, 3, nSrcTab, "=D4"); // X4
    m_pDoc->SetString(24, 3, nSrcTab, "=E4"); // Y4

    // Cell position is used for ranges relative to current position
    ScAddress cellA1(0, 0, nSrcTab);
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B2", cellA1, "$Test.$B$2"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_C2", cellA1, "$Test.$C$2"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_D2", cellA1, "$Test.$D$2"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B3", cellA1, "$Test.$B$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_C3", cellA1, "$Test.$C$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_D3", cellA1, "$Test.$D$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B2_D2", cellA1, "$Test.$B$2:$D$2"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B3_D3", cellA1, "$Test.$B$3:$D$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B2_D3", cellA1, "$Test.$B$2:$D$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("RelRange_Cm20_R0", ScAddress(1, 21, nSrcTab), "B2"));

    m_pDoc->SetString(0, 20, nSrcTab, "=B2"); // A21
    m_pDoc->SetString(1, 20, nSrcTab, "=C2"); // B21
    m_pDoc->SetString(2, 20, nSrcTab, "=D2"); // C21
    m_pDoc->SetString(0, 21, nSrcTab, "=B3"); // A22
    m_pDoc->SetString(1, 21, nSrcTab, "=C3"); // B22
    m_pDoc->SetString(2, 21, nSrcTab, "=D3"); // C22

    m_pDoc->SetString(0, 30, nSrcTab, "=$B$2"); // A31
    m_pDoc->SetString(1, 30, nSrcTab, "=$C$2"); // B31
    m_pDoc->SetString(2, 30, nSrcTab, "=$D$2"); // C31
    m_pDoc->SetString(0, 31, nSrcTab, "=$B$3"); // A32
    m_pDoc->SetString(1, 31, nSrcTab, "=$C$3"); // B32
    m_pDoc->SetString(2, 31, nSrcTab, "=$D$3"); // C32

    m_pDoc->SetString(0, 40, nSrcTab, "=Range_B2"); // A41
    m_pDoc->SetString(1, 40, nSrcTab, "=Range_C2"); // B41
    m_pDoc->SetString(2, 40, nSrcTab, "=Range_D2"); // C41
    m_pDoc->SetString(0, 41, nSrcTab, "=Range_B3"); // A42
    m_pDoc->SetString(1, 41, nSrcTab, "=Range_C3"); // B42
    m_pDoc->SetString(2, 41, nSrcTab, "=Range_D3"); // C42

    m_pDoc->SetString(0, 50, nSrcTab, "=RelRange_Cm20_R0"); // A51
    m_pDoc->SetString(1, 50, nSrcTab, "=RelRange_Cm20_R0"); // B51
    m_pDoc->SetString(2, 50, nSrcTab, "=RelRange_Cm20_R0"); // C51
    m_pDoc->SetString(0, 51, nSrcTab, "=RelRange_Cm20_R0"); // A52
    m_pDoc->SetString(1, 51, nSrcTab, "=RelRange_Cm20_R0"); // B52
    m_pDoc->SetString(2, 51, nSrcTab, "=RelRange_Cm20_R0"); // C52

    m_pDoc->SetString(0, 60, nSrcTab, "=SUM(B2:D2)"); // A61
    m_pDoc->SetString(1, 60, nSrcTab, "=SUM(B2:D2)"); // B61
    m_pDoc->SetString(2, 60, nSrcTab, "=SUM(B2:D2)"); // C61
    m_pDoc->SetString(0, 61, nSrcTab, "=SUM(B3:D3)"); // A62
    m_pDoc->SetString(1, 61, nSrcTab, "=SUM(B3:D3)"); // B62
    m_pDoc->SetString(2, 61, nSrcTab, "=SUM(B3:D3)"); // C62

    m_pDoc->SetString(0, 70, nSrcTab, "=SUM($B$2:$D$2)"); // A71
    m_pDoc->SetString(1, 70, nSrcTab, "=SUM($B$2:$D$2)"); // B71
    m_pDoc->SetString(2, 70, nSrcTab, "=SUM($B$2:$D$2)"); // C71
    m_pDoc->SetString(0, 71, nSrcTab, "=SUM($B$3:$D$3)"); // A72
    m_pDoc->SetString(1, 71, nSrcTab, "=SUM($B$3:$D$3)"); // B72
    m_pDoc->SetString(2, 71, nSrcTab, "=SUM($B$3:$D$3)"); // C72

    m_pDoc->SetString(0, 80, nSrcTab, "=SUM(Range_B2_D2)"); // A81
    m_pDoc->SetString(1, 80, nSrcTab, "=SUM(Range_B2_D2)"); // B81
    m_pDoc->SetString(2, 80, nSrcTab, "=SUM(Range_B2_D2)"); // C81
    m_pDoc->SetString(0, 81, nSrcTab, "=SUM(Range_B3_D3)"); // A82
    m_pDoc->SetString(1, 81, nSrcTab, "=SUM(Range_B3_D3)"); // B82
    m_pDoc->SetString(2, 81, nSrcTab, "=SUM(Range_B3_D3)"); // C82

    m_pDoc->SetString(0, 90, nSrcTab, "=SUM($B$2:$D$3)"); // A91
    m_pDoc->SetString(1, 90, nSrcTab, "=SUM($B$2:$D$3)"); // B91
    m_pDoc->SetString(2, 90, nSrcTab, "=SUM($B$2:$D$3)"); // C91
    m_pDoc->SetString(0, 91, nSrcTab, "=SUM($B$2:$D$3)"); // A92
    m_pDoc->SetString(1, 91, nSrcTab, "=SUM($B$2:$D$3)"); // B92
    m_pDoc->SetString(2, 91, nSrcTab, "=SUM($B$2:$D$3)"); // C92

    m_pDoc->SetString(0, 100, nSrcTab, "=SUM(Range_B2_D3)"); // A101
    m_pDoc->SetString(1, 100, nSrcTab, "=SUM(Range_B2_D3)"); // B101
    m_pDoc->SetString(2, 100, nSrcTab, "=SUM(Range_B2_D3)"); // C101
    m_pDoc->SetString(0, 101, nSrcTab, "=SUM(Range_B2_D3)"); // A102
    m_pDoc->SetString(1, 101, nSrcTab, "=SUM(Range_B2_D3)"); // B102
    m_pDoc->SetString(2, 101, nSrcTab, "=SUM(Range_B2_D3)"); // C102

    // Check precondition
    checkReferencedCutRangesRowIntitial(nSrcTab, "Initial");

    // Cut values B2:D2 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(1, 1, nSrcTab, 3, 1, nSrcTab);
    ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
    aSrcMark.SetMarkArea(aSrcRange);

    pUndoCut.reset(cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, bUndo));

    for (int i = nSrcTab + 1; i < nDestTab; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));

    if (nSrcTab < nDestTab)
        m_pDoc->InsertTab(nDestTab, "Dest");
    else if (nSrcTab > nDestTab)
        m_pDoc->RenameTab(nDestTab, "Dest");

    int nTabCount = m_pDoc->GetTableCount();
    for (int i = nTabCount; i < nTabCount + 2; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));
    nTabCount = m_pDoc->GetTableCount();

    InsertDeleteFlags aFlags(InsertDeleteFlags::ALL);

    ScDocumentUniquePtr pPasteUndoDoc;
    std::unique_ptr<ScDocument> pPasteRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;

    ScRange aDestRange;
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    if (bTransposed)
    {
        // To C12:C14
        aDestRange = ScRange(2, 11, nDestTab, 2, 13, nDestTab);
        aDestMark.SetMarkArea(aDestRange);

        if (bUndo)
            prepareUndoBeforePaste(true, pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange,
                                   pUndoData);

        // Transpose
        ScDocument* pOrigClipDoc = &aClipDoc;
        ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
        aClipDoc.TransposeClip(pTransClip.get(), aFlags, false, true);
        // Paste
        m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, pPasteRefUndoDoc.get(),
                             pTransClip.get(), true, false, true, false);
        lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 20, nSrcTab, 2, 21, nSrcTab),
                                          "Relative references after copy");

        m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, pPasteRefUndoDoc.get());
        lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 20, nSrcTab, 2, 21, nSrcTab),
                                          "Relative references after UpdateTranspose");
        pTransClip.reset();
    }
    else
    {
        // To C12:E12
        aDestRange = ScRange(2, 11, nDestTab, 4, 11, nDestTab);

        aDestMark.SetMarkArea(aDestRange);

        if (bUndo)
            prepareUndoBeforePaste(true, pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange,
                                   pUndoData);

        m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, pPasteRefUndoDoc.get(), &aClipDoc, true,
                             false, false, false);
    }

    if (bUndo)
        prepareUndoAfterPaste(pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange, pUndoData,
                              pUndoPaste, bTransposed);
}

void TestCopyPaste::checkReferencedCutRangesRow(const SCTAB nSrcTab, const SCTAB nDestTab)
{
    // Cut B2:D2 and pasted to C12:E12

    OUString aFBase("=");
    if (nSrcTab != nDestTab)
        aFBase += "Dest.";

    // Precondition
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(2, 11, nDestTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(3, 11, nDestTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(4, 11, nDestTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(3, 2, nSrcTab));

    // Guards
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(0, 0, nSrcTab)); // A1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(1, 0, nSrcTab)); // B1
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(2, 0, nSrcTab)); // C1
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(3, 0, nSrcTab)); // D1
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(4, 0, nSrcTab)); // E1
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(0, 1, nSrcTab)); // A2
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(4, 1, nSrcTab)); // E2
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(0, 2, nSrcTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(4, 2, nSrcTab)); // E3
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(0, 3, nSrcTab)); // A4
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(1, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(2, 3, nSrcTab)); // C4
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(3, 3, nSrcTab)); // D4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(4, 3, nSrcTab)); // E4
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(24, 0, nSrcTab)); // Y1
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(24, 1, nSrcTab)); // Y2
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(24, 2, nSrcTab)); // Y3
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(21, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(22, 3, nSrcTab)); // W4
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(24, 3, nSrcTab)); // Y4
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(OUString("=D1"), getFormula(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(OUString("=E1"), getFormula(24, 0, nSrcTab)); // Y1
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(OUString("=E2"), getFormula(24, 1, nSrcTab)); // Y2
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(OUString("=E3"), getFormula(24, 2, nSrcTab)); // Y3
    CPPUNIT_ASSERT_EQUAL(OUString("=A4"), getFormula(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(OUString("=B4"), getFormula(21, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(OUString("=C4"), getFormula(22, 3, nSrcTab)); // W4
    CPPUNIT_ASSERT_EQUAL(OUString("=D4"), getFormula(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(OUString("=E4"), getFormula(24, 3, nSrcTab)); // Y4

    // Note: Values (mostly) remain the same

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 20, nSrcTab)); // A21
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D12"), getFormula(1, 20, nSrcTab)); // B21
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E12"), getFormula(2, 20, nSrcTab)); // C21
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 21, nSrcTab)); // A22
    CPPUNIT_ASSERT_EQUAL(OUString("=C3"), getFormula(1, 21, nSrcTab)); // B22
    CPPUNIT_ASSERT_EQUAL(OUString("=D3"), getFormula(2, 21, nSrcTab)); // C22
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 21, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$12"), getFormula(0, 30, nSrcTab)); // A31
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$D$12"), getFormula(1, 30, nSrcTab)); // B31
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$E$12"), getFormula(2, 30, nSrcTab)); // C31
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(0, 31, nSrcTab)); // A32
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$3"), getFormula(1, 31, nSrcTab)); // B32
    CPPUNIT_ASSERT_EQUAL(OUString("=$D$3"), getFormula(2, 31, nSrcTab)); // C32
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 31, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12") : OUString("$Test.$C$12"),
                         getRangeByName("Range_B2"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$D$12") : OUString("$Test.$D$12"),
                         getRangeByName("Range_C2"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$E$12") : OUString("$Test.$E$12"),
                         getRangeByName("Range_D2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3"), getRangeByName("Range_B3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$3"), getRangeByName("Range_C3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$D$3"), getRangeByName("Range_D3")); // no change
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12:$E$12")
                                             : OUString("$Test.$C$12:$E$12"),
                         getRangeByName("Range_B2_D2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3:$D$3"), getRangeByName("Range_B3_D3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$D$3"), getRangeByName("Range_B2_D3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), getRangeByName("RelRange_Cm20_R0")); // no change

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B2"), getFormula(0, 40, nSrcTab)); // A41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C2"), getFormula(1, 40, nSrcTab)); // B41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_D2"), getFormula(2, 40, nSrcTab)); // C41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 41, nSrcTab)); // A42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C3"), getFormula(1, 41, nSrcTab)); // B42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_D3"), getFormula(2, 41, nSrcTab)); // C42
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 41, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 50, nSrcTab)); // A51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 50, nSrcTab)); // B51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(2, 50, nSrcTab)); // C51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 51, nSrcTab)); // A52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 51, nSrcTab)); // B52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(2, 51, nSrcTab)); // C52
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 51, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:E12)")
                                             : OUString("=SUM(C12:E12)"),
                         getFormula(0, 60, nSrcTab)); // A61
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:E12)")
                                             : OUString("=SUM(C12:E12)"),
                         getFormula(1, 60, nSrcTab)); // B61
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:E12)")
                                             : OUString("=SUM(C12:E12)"),
                         getFormula(2, 60, nSrcTab)); // C61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(0, 61, nSrcTab)); // A62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(1, 61, nSrcTab)); // B62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(2, 61, nSrcTab)); // C62
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 61, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$E$12)")
                                             : OUString("=SUM($C$12:$E$12)"),
                         getFormula(0, 70, nSrcTab)); // A71
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$E$12)")
                                             : OUString("=SUM($C$12:$E$12)"),
                         getFormula(1, 70, nSrcTab)); // B71
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$E$12)")
                                             : OUString("=SUM($C$12:$E$12)"),
                         getFormula(2, 70, nSrcTab)); // C71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(0, 71, nSrcTab)); // A72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(1, 71, nSrcTab)); // B72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(2, 71, nSrcTab)); // C72
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 71, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(0, 80, nSrcTab)); // A81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(1, 80, nSrcTab)); // B81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(2, 80, nSrcTab)); // C81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(0, 81, nSrcTab)); // A82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(1, 81, nSrcTab)); // B82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(2, 81, nSrcTab)); // C82
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 81, nSrcTab));

    // no change in formula after cut
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(0, 90, nSrcTab)); // A91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(1, 90, nSrcTab)); // B91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(2, 90, nSrcTab)); // C91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(0, 91, nSrcTab)); // A92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(1, 91, nSrcTab)); // B92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(2, 91, nSrcTab)); // C92
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 90, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 90, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 90, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 91, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 91, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 91, nSrcTab)); // only 2nd row

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(0, 100, nSrcTab)); // A101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(1, 100, nSrcTab)); // B101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(2, 100, nSrcTab)); // C101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(0, 101, nSrcTab)); // A102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(1, 101, nSrcTab)); // B102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(2, 101, nSrcTab)); // C102
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 100, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 100, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 100, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 101, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 101, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 101, nSrcTab)); // only 2nd row
}

void TestCopyPaste::checkReferencedCutTransposedRangesRow(const SCTAB nSrcTab, const SCTAB nDestTab)
{
    // Cut B2:D2 and pasted transposed to C12:C14

    OUString aFBase("=");
    if (nSrcTab != nDestTab)
        aFBase += "Dest.";

    // Precondition
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(2, 11, nDestTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(2, 12, nDestTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 13, nDestTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(3, 2, nSrcTab));

    // Guards
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(0, 0, nSrcTab)); // A1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(1, 0, nSrcTab)); // B1
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(2, 0, nSrcTab)); // C1
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(3, 0, nSrcTab)); // D1
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(4, 0, nSrcTab)); // E1
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(0, 1, nSrcTab)); // A2
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(4, 1, nSrcTab)); // E2
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(0, 2, nSrcTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(4, 2, nSrcTab)); // E3
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(0, 3, nSrcTab)); // A4
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(1, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(2, 3, nSrcTab)); // C4
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(3, 3, nSrcTab)); // D4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(4, 3, nSrcTab)); // E4
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(24, 0, nSrcTab)); // Y1
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(24, 1, nSrcTab)); // Y2
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(24, 2, nSrcTab)); // Y3
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(21, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(22, 3, nSrcTab)); // W4
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(24, 3, nSrcTab)); // Y4
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(OUString("=D1"), getFormula(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(OUString("=E1"), getFormula(24, 0, nSrcTab)); // Y1
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(OUString("=E2"), getFormula(24, 1, nSrcTab)); // Y2
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(OUString("=E3"), getFormula(24, 2, nSrcTab)); // Y3
    CPPUNIT_ASSERT_EQUAL(OUString("=A4"), getFormula(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(OUString("=B4"), getFormula(21, 3, nSrcTab)); // B4
    CPPUNIT_ASSERT_EQUAL(OUString("=C4"), getFormula(22, 3, nSrcTab)); // W4
    CPPUNIT_ASSERT_EQUAL(OUString("=D4"), getFormula(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(OUString("=E4"), getFormula(24, 3, nSrcTab)); // Y4

    // Note: Values (mostly) remain the same

    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 21, nSrcTab));

    // ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 20, nSrcTab), "C12", "Wrong reference");
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C13"), getFormula(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C14"), getFormula(2, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=C3"), getFormula(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=D3"), getFormula(2, 21, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 31, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$12"), getFormula(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$13"), getFormula(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$14"), getFormula(2, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$3"), getFormula(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=$D$3"), getFormula(2, 31, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12") : OUString("$Test.$C$12"),
                         getRangeByName("Range_B2"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$13") : OUString("$Test.$C$13"),
                         getRangeByName("Range_C2"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$14") : OUString("$Test.$C$14"),
                         getRangeByName("Range_D2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3"), getRangeByName("Range_B3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$3"), getRangeByName("Range_C3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$D$3"), getRangeByName("Range_D3")); // no change
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12:$C$14")
                                             : OUString("$Test.$C$12:$C$14"),
                         getRangeByName("Range_B2_D2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3:$D$3"), getRangeByName("Range_B3_D3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$D$3"), getRangeByName("Range_B2_D3")); // no change
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), getRangeByName("RelRange_Cm20_R0")); // no change

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B2"), getFormula(0, 40, nSrcTab)); // A41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C2"), getFormula(1, 40, nSrcTab)); // B41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_D2"), getFormula(2, 40, nSrcTab)); // C41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 41, nSrcTab)); // A42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C3"), getFormula(1, 41, nSrcTab)); // B42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_D3"), getFormula(2, 41, nSrcTab)); // C42
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 41, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 50, nSrcTab)); // A51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 50, nSrcTab)); // B51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(2, 50, nSrcTab)); // C51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 51, nSrcTab)); // A52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 51, nSrcTab)); // B52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(2, 51, nSrcTab)); // C52
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(0, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 51, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:C14)")
                                             : OUString("=SUM(C12:C14)"),
                         getFormula(0, 60, nSrcTab)); // A61
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:C14)")
                                             : OUString("=SUM(C12:C14)"),
                         getFormula(1, 60, nSrcTab)); // B61
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:C14)")
                                             : OUString("=SUM(C12:C14)"),
                         getFormula(2, 60, nSrcTab)); // C61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(0, 61, nSrcTab)); // A62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(1, 61, nSrcTab)); // B62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B3:D3)"), getFormula(2, 61, nSrcTab)); // C62
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 61, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$C$14)")
                                             : OUString("=SUM($C$12:$C$14)"),
                         getFormula(0, 70, nSrcTab)); // A71
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$C$14)")
                                             : OUString("=SUM($C$12:$C$14)"),
                         getFormula(1, 70, nSrcTab)); // B71
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$C$14)")
                                             : OUString("=SUM($C$12:$C$14)"),
                         getFormula(2, 70, nSrcTab)); // C71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(0, 71, nSrcTab)); // A72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(1, 71, nSrcTab)); // B72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$3:$D$3)"), getFormula(2, 71, nSrcTab)); // C72
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 71, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(0, 80, nSrcTab)); // A81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(1, 80, nSrcTab)); // B81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D2)"), getFormula(2, 80, nSrcTab)); // C81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(0, 81, nSrcTab)); // A82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(1, 81, nSrcTab)); // B82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B3_D3)"), getFormula(2, 81, nSrcTab)); // C82
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(1, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(2, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 81, nSrcTab));

    // no change in formula after cut
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(0, 90, nSrcTab)); // A91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(1, 90, nSrcTab)); // B91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(2, 90, nSrcTab)); // C91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(0, 91, nSrcTab)); // A92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(1, 91, nSrcTab)); // B92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$D$3)"), getFormula(2, 91, nSrcTab)); // C92
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 90, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 90, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 90, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 91, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 91, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 91, nSrcTab)); // only 2nd row

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(0, 100, nSrcTab)); // A101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(1, 100, nSrcTab)); // B101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(2, 100, nSrcTab)); // C101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(0, 101, nSrcTab)); // A102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(1, 101, nSrcTab)); // B102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_D3)"), getFormula(2, 101, nSrcTab)); // C102
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 100, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 100, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 100, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 101, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 101, nSrcTab)); // only 2nd row
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(2, 101, nSrcTab)); // only 2nd row
}

void TestCopyPaste::testReferencedCutRangesRow()
{
    const SCTAB nSrcTab = 0;
    const SCTAB nDestTab = 2;
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;
    executeReferencedCutRangesRow(false, nSrcTab, nDestTab, true, pUndoCut, pUndoPaste);
    checkReferencedCutRangesRow(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesRowIntitial(nSrcTab, "After undo");

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkReferencedCutRangesRow(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesRowIntitial(nSrcTab, "After undo");

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesRowTab0To0()
{
    checkReferencedCutTransposedRangesRowUndo(0, 0);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesRowTab0To1()
{
    checkReferencedCutTransposedRangesRowUndo(0, 1);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesRowTab1To3()
{
    checkReferencedCutTransposedRangesRowUndo(1, 3);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesRowTab3To1()
{
    checkReferencedCutTransposedRangesRowUndo(3, 1);
}

// tdf#142201
void TestCopyPaste::checkReferencedCutTransposedRangesRowUndo(const SCTAB nSrcTab,
                                                              const SCTAB nDestTab)
{
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;
    executeReferencedCutRangesRow(true, nSrcTab, nDestTab, true, pUndoCut, pUndoPaste);
    checkReferencedCutTransposedRangesRow(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesRowIntitial(nSrcTab, "After undo");

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkReferencedCutTransposedRangesRow(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesRowIntitial(nSrcTab, "After undo");

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

void TestCopyPaste::checkReferencedCutRangesColIntitial(const SCTAB nSrcTab, const SCTAB nDestTab,
                                                        const OUString& rDesc)
{
    printRange(m_pDoc, ScRange(1, 1, nSrcTab, 2, 3, nSrcTab), rDesc.toUtf8() + ": Source");
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(1, 1, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(1, 3, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(2, 1, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 2, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 3, nSrcTab));

    // Guards
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(0, 0, nSrcTab)); // A1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(0, 1, nSrcTab)); // A2
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(0, 2, nSrcTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(0, 3, nSrcTab)); // A4
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(0, 4, nSrcTab)); // A5
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(1, 0, nSrcTab)); // B1
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(1, 4, nSrcTab)); // B5
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(2, 0, nSrcTab)); // C1
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(2, 4, nSrcTab)); // C5
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(3, 0, nSrcTab)); // D1
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(3, 1, nSrcTab)); // D2
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(3, 2, nSrcTab)); // D3
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(3, 3, nSrcTab)); // D4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(3, 4, nSrcTab)); // D5
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(20, 4, nSrcTab)); // U5
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(21, 4, nSrcTab)); // V5
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(22, 4, nSrcTab)); // W5
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(23, 1, nSrcTab)); // X2
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(23, 2, nSrcTab)); // X3
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(23, 4, nSrcTab)); // X5
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(OUString("=A4"), getFormula(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(OUString("=A5"), getFormula(20, 4, nSrcTab)); // U5
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(OUString("=B5"), getFormula(21, 4, nSrcTab)); // V5
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(OUString("=C5"), getFormula(22, 4, nSrcTab)); // W5
    CPPUNIT_ASSERT_EQUAL(OUString("=D1"), getFormula(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(OUString("=D2"), getFormula(23, 1, nSrcTab)); // X2
    CPPUNIT_ASSERT_EQUAL(OUString("=D3"), getFormula(23, 2, nSrcTab)); // X3
    CPPUNIT_ASSERT_EQUAL(OUString("=D4"), getFormula(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(OUString("=D5"), getFormula(23, 4, nSrcTab)); // X5

    for (int i = 10; i < 20; ++i)
        for (int j = 0; j < 10; ++j)
        {
            CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(j, i, nSrcTab));
            CPPUNIT_ASSERT_EQUAL(EMPTY_OUSTRING, getFormula(j, i, nSrcTab));
        }

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 20, nSrcTab, 1, 22, nSrcTab),
                                      rDesc.toUtf8() + ": Relative references");
    CPPUNIT_ASSERT_EQUAL(OUString("=B2"), getFormula(0, 20, nSrcTab)); // A21
    CPPUNIT_ASSERT_EQUAL(OUString("=B3"), getFormula(0, 21, nSrcTab)); // A22
    CPPUNIT_ASSERT_EQUAL(OUString("=B4"), getFormula(0, 22, nSrcTab)); // A23
    CPPUNIT_ASSERT_EQUAL(OUString("=C2"), getFormula(1, 20, nSrcTab)); // B21
    CPPUNIT_ASSERT_EQUAL(OUString("=C3"), getFormula(1, 21, nSrcTab)); // B22
    CPPUNIT_ASSERT_EQUAL(OUString("=C4"), getFormula(1, 22, nSrcTab)); // B23
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 22, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 22, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 30, nSrcTab, 1, 32, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute references");
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$2"), getFormula(0, 30, nSrcTab)); // A31
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$3"), getFormula(0, 31, nSrcTab)); // A32
    CPPUNIT_ASSERT_EQUAL(OUString("=$B$4"), getFormula(0, 32, nSrcTab)); // A33
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$2"), getFormula(1, 30, nSrcTab)); // B31
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$3"), getFormula(1, 31, nSrcTab)); // B32
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$4"), getFormula(1, 32, nSrcTab)); // B33
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 32, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 32, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2"), getRangeByName("Range_B2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$3"), getRangeByName("Range_B3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$4"), getRangeByName("Range_B4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2"), getRangeByName("Range_C2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$3"), getRangeByName("Range_C3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$4"), getRangeByName("Range_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$B$4"), getRangeByName("Range_B2_B4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2:$C$4"), getRangeByName("Range_C2_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$C$4"), getRangeByName("Range_B2_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), getRangeByName("RelRange_Cm20_R0"));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 40, nSrcTab, 1, 42, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute ranges");
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B2"), getFormula(0, 40, nSrcTab)); // A41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 41, nSrcTab)); // A42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B4"), getFormula(0, 42, nSrcTab)); // A43
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C2"), getFormula(1, 40, nSrcTab)); // B41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C3"), getFormula(1, 41, nSrcTab)); // B42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C4"), getFormula(1, 42, nSrcTab)); // B43
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 42, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 42, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 50, nSrcTab, 1, 52, nSrcTab),
                                      rDesc.toUtf8() + ": Relative ranges");
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 50, nSrcTab)); // A51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 51, nSrcTab)); // A52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 52, nSrcTab)); // A53
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 50, nSrcTab)); // B51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 51, nSrcTab)); // B52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 52, nSrcTab)); // B53
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 52, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 52, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 60, nSrcTab, 1, 62, nSrcTab),
                                      rDesc.toUtf8() + ": Relative sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B2:B4)"), getFormula(0, 60, nSrcTab)); // A61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B2:B4)"), getFormula(0, 61, nSrcTab)); // A62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B2:B4)"), getFormula(0, 62, nSrcTab)); // A63
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 60, nSrcTab)); // B61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 61, nSrcTab)); // B62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 62, nSrcTab)); // B63
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 62, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 62, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 70, nSrcTab, 1, 72, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$B$4)"), getFormula(0, 70, nSrcTab)); // A71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$B$4)"), getFormula(0, 71, nSrcTab)); // A72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$B$4)"), getFormula(0, 72, nSrcTab)); // A73
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 70, nSrcTab)); // B71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 71, nSrcTab)); // B72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 72, nSrcTab)); // B73
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 72, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 72, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 80, nSrcTab, 1, 82, nSrcTab),
                                      rDesc.toUtf8() + ": Relative range sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 80, nSrcTab)); // A81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 81, nSrcTab)); // A82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 82, nSrcTab)); // A83
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 80, nSrcTab)); // B81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 81, nSrcTab)); // B82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 82, nSrcTab)); // B83
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 82, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 82, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 90, nSrcTab, 1, 92, nSrcTab),
                                      rDesc.toUtf8() + ": Absolute sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 90, nSrcTab)); // A91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 91, nSrcTab)); // A92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 92, nSrcTab)); // A93
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 90, nSrcTab)); // B91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 91, nSrcTab)); // B92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 92, nSrcTab)); // B93
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 92, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 92, nSrcTab));

    lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 100, nSrcTab, 1, 102, nSrcTab),
                                      rDesc.toUtf8() + ": Relative range sum");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 100, nSrcTab)); // A101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 101, nSrcTab)); // A102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 102, nSrcTab)); // A103
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 100, nSrcTab)); // B101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 101, nSrcTab)); // B102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 102, nSrcTab)); // B103
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(0, 102, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(69.0, m_pDoc->GetValue(1, 102, nSrcTab));

    // References to the dest range
    OUString aFBase("=");
    if (nSrcTab != nDestTab)
        aFBase += "Dest.";

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C13"), getFormula(0, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C14"), getFormula(0, 114, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D12"), getFormula(1, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D13"), getFormula(1, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D14"), getFormula(1, 114, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E12"), getFormula(2, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E13"), getFormula(2, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E14"), getFormula(2, 114, nSrcTab));
}

void TestCopyPaste::executeReferencedCutRangesCol(const bool bTransposed, const SCTAB nSrcTab,
                                                  const SCTAB nDestTab, const bool bUndo,
                                                  std::unique_ptr<ScUndoCut>& pUndoCut,
                                                  std::unique_ptr<ScUndoPaste>& pUndoPaste)
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    for (int i = 0; i < nSrcTab; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));
    m_pDoc->InsertTab(nSrcTab, "Test");

    m_pDoc->SetValue(1, 1, nSrcTab, 01.0); // B2  \.
    m_pDoc->SetValue(1, 2, nSrcTab, 11.0); // B3   | cut
    m_pDoc->SetValue(1, 3, nSrcTab, 21.0); // B4  /
    m_pDoc->SetValue(2, 1, nSrcTab, 02.0); // C2
    m_pDoc->SetValue(2, 2, nSrcTab, 12.0); // C3
    m_pDoc->SetValue(2, 3, nSrcTab, 22.0); // C4
    printRange(m_pDoc, ScRange(1, 1, nSrcTab, 2, 3, nSrcTab), "Source");

    // Guard values
    m_pDoc->SetValue(0, 0, nSrcTab, 1000.0); // A1
    m_pDoc->SetValue(0, 1, nSrcTab, 1001.0); // A2
    m_pDoc->SetValue(0, 2, nSrcTab, 1002.0); // A3
    m_pDoc->SetValue(0, 3, nSrcTab, 1003.0); // A4
    m_pDoc->SetValue(0, 4, nSrcTab, 1004.0); // A5
    m_pDoc->SetValue(1, 0, nSrcTab, 1010.0); // B1
    m_pDoc->SetValue(1, 4, nSrcTab, 1014.0); // B5
    m_pDoc->SetValue(2, 0, nSrcTab, 1020.0); // C1
    m_pDoc->SetValue(2, 4, nSrcTab, 1024.0); // C5
    m_pDoc->SetValue(3, 0, nSrcTab, 1030.0); // D1
    m_pDoc->SetValue(3, 1, nSrcTab, 1031.0); // D2
    m_pDoc->SetValue(3, 2, nSrcTab, 1032.0); // D3
    m_pDoc->SetValue(3, 3, nSrcTab, 1033.0); // D4
    m_pDoc->SetValue(3, 4, nSrcTab, 1034.0); // D5

    m_pDoc->SetString(20, 0, nSrcTab, "=A1"); // U1
    m_pDoc->SetString(20, 1, nSrcTab, "=A2"); // U2
    m_pDoc->SetString(20, 2, nSrcTab, "=A3"); // U3
    m_pDoc->SetString(20, 3, nSrcTab, "=A4"); // U4
    m_pDoc->SetString(20, 4, nSrcTab, "=A5"); // U5
    m_pDoc->SetString(21, 0, nSrcTab, "=B1"); // V1
    m_pDoc->SetString(21, 4, nSrcTab, "=B5"); // V5
    m_pDoc->SetString(22, 0, nSrcTab, "=C1"); // W1
    m_pDoc->SetString(22, 4, nSrcTab, "=C5"); // W5
    m_pDoc->SetString(23, 0, nSrcTab, "=D1"); // X1
    m_pDoc->SetString(23, 1, nSrcTab, "=D2"); // X2
    m_pDoc->SetString(23, 2, nSrcTab, "=D3"); // X3
    m_pDoc->SetString(23, 3, nSrcTab, "=D4"); // X4
    m_pDoc->SetString(23, 4, nSrcTab, "=D5"); // X5

    // Cell position is used for ranges relative to current position
    ScAddress cellA1(0, 0, nSrcTab);
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B2", cellA1, "$Test.$B$2"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B3", cellA1, "$Test.$B$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B4", cellA1, "$Test.$B$4"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_C2", cellA1, "$Test.$C$2"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_C3", cellA1, "$Test.$C$3"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_C4", cellA1, "$Test.$C$4"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B2_B4", cellA1, "$Test.$B$2:$B$4"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_C2_C4", cellA1, "$Test.$C$2:$C$4"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("Range_B2_C4", cellA1, "$Test.$B$2:$C$4"));
    CPPUNIT_ASSERT(m_pDoc->InsertNewRangeName("RelRange_Cm20_R0", ScAddress(1, 21, nSrcTab), "B2"));

    m_pDoc->SetString(0, 20, nSrcTab, "=B2"); // A21
    m_pDoc->SetString(0, 21, nSrcTab, "=B3"); // A22
    m_pDoc->SetString(0, 22, nSrcTab, "=B4"); // A23
    m_pDoc->SetString(1, 20, nSrcTab, "=C2"); // B21
    m_pDoc->SetString(1, 21, nSrcTab, "=C3"); // B22
    m_pDoc->SetString(1, 22, nSrcTab, "=C4"); // B23

    m_pDoc->SetString(0, 30, nSrcTab, "=$B$2"); // A31
    m_pDoc->SetString(0, 31, nSrcTab, "=$B$3"); // A32
    m_pDoc->SetString(0, 32, nSrcTab, "=$B$4"); // A33
    m_pDoc->SetString(1, 30, nSrcTab, "=$C$2"); // B31
    m_pDoc->SetString(1, 31, nSrcTab, "=$C$3"); // B32
    m_pDoc->SetString(1, 32, nSrcTab, "=$C$4"); // B33

    m_pDoc->SetString(0, 40, nSrcTab, "=Range_B2"); // A41
    m_pDoc->SetString(0, 41, nSrcTab, "=Range_B3"); // A42
    m_pDoc->SetString(0, 42, nSrcTab, "=Range_B4"); // A43
    m_pDoc->SetString(1, 40, nSrcTab, "=Range_C2"); // B41
    m_pDoc->SetString(1, 41, nSrcTab, "=Range_C3"); // B42
    m_pDoc->SetString(1, 42, nSrcTab, "=Range_C4"); // B43

    m_pDoc->SetString(0, 50, nSrcTab, "=RelRange_Cm20_R0"); // A51
    m_pDoc->SetString(0, 51, nSrcTab, "=RelRange_Cm20_R0"); // A52
    m_pDoc->SetString(0, 52, nSrcTab, "=RelRange_Cm20_R0"); // A53
    m_pDoc->SetString(1, 50, nSrcTab, "=RelRange_Cm20_R0"); // B51
    m_pDoc->SetString(1, 51, nSrcTab, "=RelRange_Cm20_R0"); // B52
    m_pDoc->SetString(1, 52, nSrcTab, "=RelRange_Cm20_R0"); // B53

    m_pDoc->SetString(0, 60, nSrcTab, "=SUM(B2:B4"); // A61
    m_pDoc->SetString(0, 61, nSrcTab, "=SUM(B2:B4"); // A62
    m_pDoc->SetString(0, 62, nSrcTab, "=SUM(B2:B4"); // A63
    m_pDoc->SetString(1, 60, nSrcTab, "=SUM(C2:C4"); // B61
    m_pDoc->SetString(1, 61, nSrcTab, "=SUM(C2:C4"); // B62
    m_pDoc->SetString(1, 62, nSrcTab, "=SUM(C2:C4"); // B63

    m_pDoc->SetString(0, 70, nSrcTab, "=SUM($B$2:$B$4"); // A71
    m_pDoc->SetString(0, 71, nSrcTab, "=SUM($B$2:$B$4"); // A72
    m_pDoc->SetString(0, 72, nSrcTab, "=SUM($B$2:$B$4"); // A73
    m_pDoc->SetString(1, 70, nSrcTab, "=SUM($C$2:$C$4"); // B71
    m_pDoc->SetString(1, 71, nSrcTab, "=SUM($C$2:$C$4"); // B72
    m_pDoc->SetString(1, 72, nSrcTab, "=SUM($C$2:$C$4"); // B73

    m_pDoc->SetString(0, 80, nSrcTab, "=SUM(Range_B2_B4)"); // A81
    m_pDoc->SetString(0, 81, nSrcTab, "=SUM(Range_B2_B4)"); // A82
    m_pDoc->SetString(0, 82, nSrcTab, "=SUM(Range_B2_B4)"); // A83
    m_pDoc->SetString(1, 80, nSrcTab, "=SUM(Range_C2_C4)"); // B81
    m_pDoc->SetString(1, 81, nSrcTab, "=SUM(Range_C2_C4)"); // B82
    m_pDoc->SetString(1, 82, nSrcTab, "=SUM(Range_C2_C4)"); // B83

    m_pDoc->SetString(0, 90, nSrcTab, "=SUM($B$2:$C$4"); // A91
    m_pDoc->SetString(0, 91, nSrcTab, "=SUM($B$2:$C$4"); // A92
    m_pDoc->SetString(0, 92, nSrcTab, "=SUM($B$2:$C$4"); // A93
    m_pDoc->SetString(1, 90, nSrcTab, "=SUM($B$2:$C$4"); // B91
    m_pDoc->SetString(1, 91, nSrcTab, "=SUM($B$2:$C$4"); // B92
    m_pDoc->SetString(1, 92, nSrcTab, "=SUM($B$2:$C$4"); // B93

    m_pDoc->SetString(0, 100, nSrcTab, "=SUM(Range_B2_C4"); // A101
    m_pDoc->SetString(0, 101, nSrcTab, "=SUM(Range_B2_C4"); // A102
    m_pDoc->SetString(0, 102, nSrcTab, "=SUM(Range_B2_C4"); // A103
    m_pDoc->SetString(1, 100, nSrcTab, "=SUM(Range_B2_C4"); // B101
    m_pDoc->SetString(1, 101, nSrcTab, "=SUM(Range_B2_C4"); // B102
    m_pDoc->SetString(1, 102, nSrcTab, "=SUM(Range_B2_C4"); // B103

    for (int i = nSrcTab + 1; i < nDestTab; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));

    if (nSrcTab < nDestTab)
        m_pDoc->InsertTab(nDestTab, "Dest");
    else if (nSrcTab > nDestTab)
        m_pDoc->RenameTab(nDestTab, "Dest");

    int nTabCount = m_pDoc->GetTableCount();
    for (int i = nTabCount; i < nTabCount + 2; ++i)
        m_pDoc->InsertTab(i, "Empty Tab " + OUString::number(i));
    nTabCount = m_pDoc->GetTableCount();

    // References to the dest range
    OUString aFBase("=");
    if (nSrcTab != nDestTab)
        aFBase += "Dest.";

    m_pDoc->SetString(0, 112, nSrcTab, OUString(aFBase + "C12"));
    m_pDoc->SetString(0, 113, nSrcTab, OUString(aFBase + "C13"));
    m_pDoc->SetString(0, 114, nSrcTab, OUString(aFBase + "C14"));
    m_pDoc->SetString(1, 112, nSrcTab, OUString(aFBase + "D12"));
    m_pDoc->SetString(1, 113, nSrcTab, OUString(aFBase + "D13"));
    m_pDoc->SetString(1, 114, nSrcTab, OUString(aFBase + "D14"));
    m_pDoc->SetString(2, 112, nSrcTab, OUString(aFBase + "E12"));
    m_pDoc->SetString(2, 113, nSrcTab, OUString(aFBase + "E13"));
    m_pDoc->SetString(2, 114, nSrcTab, OUString(aFBase + "E14"));

    // Check precondition
    checkReferencedCutRangesColIntitial(nSrcTab, nDestTab, "Initial");

    // Cut values B2:B4 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(1, 1, nSrcTab, 1, 3, nSrcTab);
    ScMarkData aSrcMark(m_pDoc->GetSheetLimits());
    aSrcMark.SetMarkArea(aSrcRange);

    pUndoCut.reset(cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, bUndo));

    InsertDeleteFlags aFlags(InsertDeleteFlags::ALL);

    ScDocumentUniquePtr pPasteUndoDoc;
    std::unique_ptr<ScDocument> pPasteRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;

    ScRange aDestRange;
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    if (bTransposed)
    {
        // To C12:E12
        aDestRange = ScRange(2, 11, nDestTab, 4, 11, nDestTab);
        aDestMark.SetMarkArea(aDestRange);

        if (bUndo)
            prepareUndoBeforePaste(true, pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange,
                                   pUndoData);

        // Transpose
        ScDocument* pOrigClipDoc = &aClipDoc;
        ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
        aClipDoc.TransposeClip(pTransClip.get(), aFlags, false, true);
        // Paste
        m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, pPasteRefUndoDoc.get(),
                             pTransClip.get(), true, false, true, false);
        lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 20, nSrcTab, 2, 21, nSrcTab),
                                          "Relative references after copy");

        m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, pPasteRefUndoDoc.get());
        lcl_printValuesAndFormulasInRange(m_pDoc, ScRange(0, 20, nSrcTab, 2, 21, nSrcTab),
                                          "Relative references after UpdateTranspose");
        pTransClip.reset();
    }
    else
    {
        // To C12:C14
        aDestRange = ScRange(2, 11, nDestTab, 2, 13, nDestTab);
        // aDestMark = ScMarkData(m_pDoc->GetSheetLimits());

        aDestMark.SetMarkArea(aDestRange);

        if (bUndo)
            prepareUndoBeforePaste(true, pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange,
                                   pUndoData);

        m_pDoc->CopyFromClip(aDestRange, aDestMark, aFlags, pPasteRefUndoDoc.get(), &aClipDoc, true,
                             false, false, false);
    }

    if (bUndo)
        prepareUndoAfterPaste(pPasteUndoDoc, pPasteRefUndoDoc, aDestMark, aDestRange, pUndoData,
                              pUndoPaste, bTransposed);
}

void TestCopyPaste::checkReferencedCutRangesCol(const SCTAB nSrcTab, const SCTAB nDestTab)
{
    // Cut B2:B4 and pasted to C12:C14

    OUString aFBase("=");
    if (nSrcTab != nDestTab)
        aFBase += "Dest.";

    // Precondition
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(2, 11, nDestTab)); // C12
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(2, 12, nDestTab)); // C13
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 13, nDestTab)); // C14
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(2, 1, nSrcTab)); // C2
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 2, nSrcTab)); // C3
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 3, nSrcTab)); // C4

    // Guards
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(0, 0, nSrcTab)); // A1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(0, 1, nSrcTab)); // A2
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(0, 2, nSrcTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(0, 3, nSrcTab)); // A4
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(0, 4, nSrcTab)); // A5
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(1, 0, nSrcTab)); // B1
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(1, 4, nSrcTab)); // B5
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(2, 0, nSrcTab)); // C1
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(2, 4, nSrcTab)); // C5
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(3, 0, nSrcTab)); // D1
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(3, 1, nSrcTab)); // D2
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(3, 2, nSrcTab)); // D3
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(3, 3, nSrcTab)); // D4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(3, 4, nSrcTab)); // D5
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(20, 4, nSrcTab)); // U5
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(21, 4, nSrcTab)); // V5
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(22, 4, nSrcTab)); // W5
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(23, 1, nSrcTab)); // X2
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(23, 2, nSrcTab)); // X3
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(23, 4, nSrcTab)); // X5
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(OUString("=A4"), getFormula(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(OUString("=A5"), getFormula(20, 4, nSrcTab)); // U5
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(OUString("=B5"), getFormula(21, 4, nSrcTab)); // V5
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(OUString("=C5"), getFormula(22, 4, nSrcTab)); // W5
    CPPUNIT_ASSERT_EQUAL(OUString("=D1"), getFormula(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(OUString("=D2"), getFormula(23, 1, nSrcTab)); // X2
    CPPUNIT_ASSERT_EQUAL(OUString("=D3"), getFormula(23, 2, nSrcTab)); // X3
    CPPUNIT_ASSERT_EQUAL(OUString("=D4"), getFormula(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(OUString("=D5"), getFormula(23, 4, nSrcTab)); // X5

    // Note: Values (mostly) remain the same

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 20, nSrcTab)); // A21
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C13"), getFormula(0, 21, nSrcTab)); // A22
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C14"), getFormula(0, 22, nSrcTab)); // A23
    CPPUNIT_ASSERT_EQUAL(OUString("=C2"), getFormula(1, 20, nSrcTab)); // B21
    CPPUNIT_ASSERT_EQUAL(OUString("=C3"), getFormula(1, 21, nSrcTab)); // B22
    CPPUNIT_ASSERT_EQUAL(OUString("=C4"), getFormula(1, 22, nSrcTab)); // B23
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 22, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 22, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$12"), getFormula(0, 30, nSrcTab)); // A31
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$13"), getFormula(0, 31, nSrcTab)); // A32
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$14"), getFormula(0, 32, nSrcTab)); // A33
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$2"), getFormula(1, 30, nSrcTab)); // B31
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$3"), getFormula(1, 31, nSrcTab)); // B32
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$4"), getFormula(1, 32, nSrcTab)); // B33
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 32, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 32, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12") : OUString("$Test.$C$12"),
                         getRangeByName("Range_B2"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$13") : OUString("$Test.$C$13"),
                         getRangeByName("Range_B3"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$14") : OUString("$Test.$C$14"),
                         getRangeByName("Range_B4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2"), getRangeByName("Range_C2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$3"), getRangeByName("Range_C3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$4"), getRangeByName("Range_C4"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12:$C$14")
                                             : OUString("$Test.$C$12:$C$14"),
                         getRangeByName("Range_B2_B4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2:$C$4"), getRangeByName("Range_C2_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$C$4"), getRangeByName("Range_B2_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), getRangeByName("RelRange_Cm20_R0"));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B2"), getFormula(0, 40, nSrcTab)); // A41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 41, nSrcTab)); // A42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B4"), getFormula(0, 42, nSrcTab)); // A43
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C2"), getFormula(1, 40, nSrcTab)); // B41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C3"), getFormula(1, 41, nSrcTab)); // B42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C4"), getFormula(1, 42, nSrcTab)); // B43
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 42, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 42, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 50, nSrcTab)); // A51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 51, nSrcTab)); // A52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 52, nSrcTab)); // A53
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 50, nSrcTab)); // B51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 51, nSrcTab)); // B52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 52, nSrcTab)); // B53
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 52, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 52, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:C14)")
                                             : OUString("=SUM(C12:C14)"),
                         getFormula(0, 60, nSrcTab)); // A61
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:C14)")
                                             : OUString("=SUM(C12:C14)"),
                         getFormula(0, 61, nSrcTab)); // A62
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:C14)")
                                             : OUString("=SUM(C12:C14)"),
                         getFormula(0, 62, nSrcTab)); // A63
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 60, nSrcTab)); // B61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 61, nSrcTab)); // B62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 62, nSrcTab)); // B63
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 62, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 62, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$C$14)")
                                             : OUString("=SUM($C$12:$C$14)"),
                         getFormula(0, 70, nSrcTab)); // A71
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$C$14)")
                                             : OUString("=SUM($C$12:$C$14)"),
                         getFormula(0, 71, nSrcTab)); // A72
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$C$14)")
                                             : OUString("=SUM($C$12:$C$14)"),
                         getFormula(0, 72, nSrcTab)); // A73
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 70, nSrcTab)); // B71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 71, nSrcTab)); // B72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 72, nSrcTab)); // B73
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 72, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 72, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 80, nSrcTab)); // A81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 81, nSrcTab)); // A82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 82, nSrcTab)); // A83
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 80, nSrcTab)); // B81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 81, nSrcTab)); // B82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 82, nSrcTab)); // B83
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 82, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 82, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 90, nSrcTab)); // A91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 91, nSrcTab)); // A92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 92, nSrcTab)); // A93
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 90, nSrcTab)); // B91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 91, nSrcTab)); // B92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 92, nSrcTab)); // B93
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 92, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 92, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 100, nSrcTab)); // A101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 101, nSrcTab)); // A102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 102, nSrcTab)); // A103
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 100, nSrcTab)); // B101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 101, nSrcTab)); // B102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 102, nSrcTab)); // B103
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 102, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 102, nSrcTab));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C13"), getFormula(0, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C14"), getFormula(0, 114, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D12"), getFormula(1, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D13"), getFormula(1, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D14"), getFormula(1, 114, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E12"), getFormula(2, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E13"), getFormula(2, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E14"), getFormula(2, 114, nSrcTab));
}

void TestCopyPaste::checkReferencedCutTransposedRangesCol(const SCTAB nSrcTab, const SCTAB nDestTab)
{
    // Cut B2:D2 and pasted transposed to C12:E12

    OUString aFBase("=");
    if (nSrcTab != nDestTab)
        aFBase += "Dest.";

    // Precondition
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(2, 11, nDestTab)); // C12
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(3, 11, nDestTab)); // D12
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(4, 11, nDestTab)); // E12
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(2, 1, nSrcTab)); // C2
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 2, nSrcTab)); // C3
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 3, nSrcTab)); // C4

    // Guards
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(0, 0, nSrcTab)); // A1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(0, 1, nSrcTab)); // A2
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(0, 2, nSrcTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(0, 3, nSrcTab)); // A4
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(0, 4, nSrcTab)); // A5
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(1, 0, nSrcTab)); // B1
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(1, 4, nSrcTab)); // B5
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(2, 0, nSrcTab)); // C1
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(2, 4, nSrcTab)); // C5
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(3, 0, nSrcTab)); // D1
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(3, 1, nSrcTab)); // D2
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(3, 2, nSrcTab)); // D3
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(3, 3, nSrcTab)); // D4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(3, 4, nSrcTab)); // D5
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(1001.0, m_pDoc->GetValue(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(1002.0, m_pDoc->GetValue(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(1003.0, m_pDoc->GetValue(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(1004.0, m_pDoc->GetValue(20, 4, nSrcTab)); // U5
    CPPUNIT_ASSERT_EQUAL(1010.0, m_pDoc->GetValue(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(1014.0, m_pDoc->GetValue(21, 4, nSrcTab)); // V5
    CPPUNIT_ASSERT_EQUAL(1020.0, m_pDoc->GetValue(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(1024.0, m_pDoc->GetValue(22, 4, nSrcTab)); // W5
    CPPUNIT_ASSERT_EQUAL(1030.0, m_pDoc->GetValue(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(1031.0, m_pDoc->GetValue(23, 1, nSrcTab)); // X2
    CPPUNIT_ASSERT_EQUAL(1032.0, m_pDoc->GetValue(23, 2, nSrcTab)); // X3
    CPPUNIT_ASSERT_EQUAL(1033.0, m_pDoc->GetValue(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(1034.0, m_pDoc->GetValue(23, 4, nSrcTab)); // X5
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(20, 0, nSrcTab)); // U1
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(20, 1, nSrcTab)); // U2
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(20, 2, nSrcTab)); // U3
    CPPUNIT_ASSERT_EQUAL(OUString("=A4"), getFormula(20, 3, nSrcTab)); // U4
    CPPUNIT_ASSERT_EQUAL(OUString("=A5"), getFormula(20, 4, nSrcTab)); // U5
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(21, 0, nSrcTab)); // V1
    CPPUNIT_ASSERT_EQUAL(OUString("=B5"), getFormula(21, 4, nSrcTab)); // V5
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(22, 0, nSrcTab)); // W1
    CPPUNIT_ASSERT_EQUAL(OUString("=C5"), getFormula(22, 4, nSrcTab)); // W5
    CPPUNIT_ASSERT_EQUAL(OUString("=D1"), getFormula(23, 0, nSrcTab)); // X1
    CPPUNIT_ASSERT_EQUAL(OUString("=D2"), getFormula(23, 1, nSrcTab)); // X2
    CPPUNIT_ASSERT_EQUAL(OUString("=D3"), getFormula(23, 2, nSrcTab)); // X3
    CPPUNIT_ASSERT_EQUAL(OUString("=D4"), getFormula(23, 3, nSrcTab)); // X4
    CPPUNIT_ASSERT_EQUAL(OUString("=D5"), getFormula(23, 4, nSrcTab)); // X5

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 20, nSrcTab)); // A21
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D12"), getFormula(0, 21, nSrcTab)); // A22
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E12"), getFormula(0, 22, nSrcTab)); // A23
    CPPUNIT_ASSERT_EQUAL(OUString("=C2"), getFormula(1, 20, nSrcTab)); // B21
    CPPUNIT_ASSERT_EQUAL(OUString("=C3"), getFormula(1, 21, nSrcTab)); // B22
    CPPUNIT_ASSERT_EQUAL(OUString("=C4"), getFormula(1, 22, nSrcTab)); // B23
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 22, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 20, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 21, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 22, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$C$12"), getFormula(0, 30, nSrcTab)); // A31
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$D$12"), getFormula(0, 31, nSrcTab)); // A32
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "$E$12"), getFormula(0, 32, nSrcTab)); // A33
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$2"), getFormula(1, 30, nSrcTab)); // B31
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$3"), getFormula(1, 31, nSrcTab)); // B32
    CPPUNIT_ASSERT_EQUAL(OUString("=$C$4"), getFormula(1, 32, nSrcTab)); // B33
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 32, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 30, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 31, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 32, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12") : OUString("$Test.$C$12"),
                         getRangeByName("Range_B2"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$D$12") : OUString("$Test.$D$12"),
                         getRangeByName("Range_B3"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$E$12") : OUString("$Test.$E$12"),
                         getRangeByName("Range_B4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2"), getRangeByName("Range_C2"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$3"), getRangeByName("Range_C3"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$4"), getRangeByName("Range_C4"));
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("$Dest.$C$12:$E$12")
                                             : OUString("$Test.$C$12:$E$12"),
                         getRangeByName("Range_B2_B4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$C$2:$C$4"), getRangeByName("Range_C2_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$B$2:$C$4"), getRangeByName("Range_B2_C4"));
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), getRangeByName("RelRange_Cm20_R0"));

    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B2"), getFormula(0, 40, nSrcTab)); // A41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B3"), getFormula(0, 41, nSrcTab)); // A42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_B4"), getFormula(0, 42, nSrcTab)); // A43
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C2"), getFormula(1, 40, nSrcTab)); // B41
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C3"), getFormula(1, 41, nSrcTab)); // B42
    CPPUNIT_ASSERT_EQUAL(OUString("=Range_C4"), getFormula(1, 42, nSrcTab)); // B43
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 42, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 40, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 41, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 42, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 50, nSrcTab)); // A51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 51, nSrcTab)); // A52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(0, 52, nSrcTab)); // A53
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 50, nSrcTab)); // B51
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 51, nSrcTab)); // B52
    CPPUNIT_ASSERT_EQUAL(OUString("=RelRange_Cm20_R0"), getFormula(1, 52, nSrcTab)); // B53
    CPPUNIT_ASSERT_EQUAL(01.0, m_pDoc->GetValue(0, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(0, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(0, 52, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(02.0, m_pDoc->GetValue(1, 50, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 51, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(1, 52, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:E12)")
                                             : OUString("=SUM(C12:E12)"),
                         getFormula(0, 60, nSrcTab)); // A61
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:E12)")
                                             : OUString("=SUM(C12:E12)"),
                         getFormula(0, 61, nSrcTab)); // A62
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.C12:E12)")
                                             : OUString("=SUM(C12:E12)"),
                         getFormula(0, 62, nSrcTab)); // A63
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 60, nSrcTab)); // B61
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 61, nSrcTab)); // B62
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C2:C4)"), getFormula(1, 62, nSrcTab)); // B63
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 62, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 60, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 61, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 62, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$E$12)")
                                             : OUString("=SUM($C$12:$E$12)"),
                         getFormula(0, 70, nSrcTab)); // A71
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$E$12)")
                                             : OUString("=SUM($C$12:$E$12)"),
                         getFormula(0, 71, nSrcTab)); // A72
    CPPUNIT_ASSERT_EQUAL(nSrcTab != nDestTab ? OUString("=SUM(Dest.$C$12:$E$12)")
                                             : OUString("=SUM($C$12:$E$12)"),
                         getFormula(0, 72, nSrcTab)); // A73
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 70, nSrcTab)); // B71
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 71, nSrcTab)); // B72
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($C$2:$C$4)"), getFormula(1, 72, nSrcTab)); // B73
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 72, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 70, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 71, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 72, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 80, nSrcTab)); // A81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 81, nSrcTab)); // A82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_B4)"), getFormula(0, 82, nSrcTab)); // A83
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 80, nSrcTab)); // B81
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 81, nSrcTab)); // B82
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_C2_C4)"), getFormula(1, 82, nSrcTab)); // B83
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(33.0, m_pDoc->GetValue(0, 82, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 80, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 81, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 82, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 90, nSrcTab)); // A91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 91, nSrcTab)); // A92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(0, 92, nSrcTab)); // A93
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 90, nSrcTab)); // B91
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 91, nSrcTab)); // B92
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($B$2:$C$4)"), getFormula(1, 92, nSrcTab)); // B93
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 92, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 90, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 91, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 92, nSrcTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 100, nSrcTab)); // A101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 101, nSrcTab)); // A102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(0, 102, nSrcTab)); // A103
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 100, nSrcTab)); // B101
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 101, nSrcTab)); // B102
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(Range_B2_C4)"), getFormula(1, 102, nSrcTab)); // B103
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(0, 102, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 100, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 101, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(36.0, m_pDoc->GetValue(1, 102, nSrcTab));

    // Existing references to the destination range must not change
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C12"), getFormula(0, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C13"), getFormula(0, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "C14"), getFormula(0, 114, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D12"), getFormula(1, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D13"), getFormula(1, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "D14"), getFormula(1, 114, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E12"), getFormula(2, 112, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E13"), getFormula(2, 113, nSrcTab));
    CPPUNIT_ASSERT_EQUAL(OUString(aFBase + "E14"), getFormula(2, 114, nSrcTab));
}

void TestCopyPaste::testReferencedCutRangesCol()
{
    const SCTAB nSrcTab = 0;
    const SCTAB nDestTab = 2;
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;
    executeReferencedCutRangesCol(false, nSrcTab, nDestTab, true, pUndoCut, pUndoPaste);
    checkReferencedCutRangesCol(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesColIntitial(nSrcTab, nDestTab, "After undo");

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkReferencedCutRangesCol(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesColIntitial(nSrcTab, nDestTab, "After undo");

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesColTab0To0()
{
    checkReferencedCutTransposedRangesColUndo(0, 0);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesColTab0To1()
{
    checkReferencedCutTransposedRangesColUndo(0, 1);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesColTab1To3()
{
    checkReferencedCutTransposedRangesColUndo(1, 3);
}

// tdf#142201
void TestCopyPaste::testReferencedCutTransposedRangesColTab3To1()
{
    checkReferencedCutTransposedRangesColUndo(3, 1);
}

// tdf#142201
void TestCopyPaste::checkReferencedCutTransposedRangesColUndo(const SCTAB nSrcTab,
                                                              const SCTAB nDestTab)
{
    std::unique_ptr<ScUndoCut> pUndoCut;
    std::unique_ptr<ScUndoPaste> pUndoPaste;
    executeReferencedCutRangesCol(true, nSrcTab, nDestTab, true, pUndoCut, pUndoPaste);
    checkReferencedCutTransposedRangesCol(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesColIntitial(nSrcTab, nDestTab, "After undo");

    pUndoCut->Redo();
    pUndoPaste->Redo();
    checkReferencedCutTransposedRangesCol(nSrcTab, nDestTab);

    pUndoPaste->Undo();
    pUndoCut->Undo();
    checkReferencedCutRangesColIntitial(nSrcTab, nDestTab, "After undo");

    pUndoPaste.reset();
    pUndoCut.reset();

    for (int i = m_pDoc->GetTableCount(); i > 0; --i)
        m_pDoc->DeleteTab(i - 1);
}

void TestCopyPaste::testCutTransposedFormulas()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetValue(1, 0, nTab, 2.0); // B1

    m_pDoc->SetString(1, 1, nTab, "=A1"); // B2
    m_pDoc->SetString(2, 1, nTab, "=B1"); // C2
    m_pDoc->SetString(3, 1, nTab, "=SUM(A1:B1)"); // D2
    m_pDoc->SetString(4, 1, nTab, "=$B$1"); // E2
    m_pDoc->SetString(5, 1, nTab, "=$B1"); // F2
    m_pDoc->SetString(6, 1, nTab, "=B$1"); // G2

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(3, 1, nTab));

    // Cut formulas B2:G2 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(1, 1, nTab, 6, 1, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);

    // To C3:C8
    ScRange aDestRange(2, 2, nTab, 2, 7, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(2, 2, nTab));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(2, 2, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(2, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 3, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:B1)"), getFormula(2, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(2, 4, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=$B$1"), getFormula(2, 5, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 5, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=$B1"), getFormula(2, 6, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 6, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=B$1"), getFormula(2, 7, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 7, nTab));
}

void TestCopyPaste::testCutTransposedFormulasSquare()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetValue(0, 1, nTab, 2.0); // A2
    m_pDoc->SetValue(1, 0, nTab, 11.0); // B1
    m_pDoc->SetValue(1, 1, nTab, 12.0); // B2
    m_pDoc->SetValue(2, 0, nTab, 21.0); // C1
    m_pDoc->SetValue(2, 1, nTab, 22.0); // C2

    m_pDoc->SetString(0, 3, nTab, "=A1"); // A4
    m_pDoc->SetString(0, 4, nTab, "=A2"); // A5
    m_pDoc->SetString(1, 3, nTab, "=B1"); // B4
    m_pDoc->SetString(1, 4, nTab, "=B2"); // B5
    m_pDoc->SetString(2, 3, nTab, "=C1"); // C4
    m_pDoc->SetString(2, 4, nTab, "=C2"); // C5

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(1, 4, nTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(2, 3, nTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 4, nTab));

    printRange(m_pDoc, ScRange(0, 0, nTab, 2, 1, nTab), "Values");
    printRange(m_pDoc, ScRange(0, 3, nTab, 2, 4, nTab), "Formulas");
    printFormula(m_pDoc, 0, 4, nTab);

    // Cut formulas A4:B5 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(0, 3, nTab, 2, 4, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);

    // To B7:C9
    ScRange aDestRange(1, 6, nTab, 2, 8, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    printRange(m_pDoc, aDestRange, "Formulas after cut transposed");
    printFormula(m_pDoc, 2, 6, nTab);

    // Check results
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(1, 6, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 6, nTab));
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(1, 7, nTab));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(2, 7, nTab));
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(1, 8, nTab));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(2, 8, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), getFormula(1, 6, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=A2"), getFormula(2, 6, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B1"), getFormula(1, 7, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=B2"), getFormula(2, 7, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=C1"), getFormula(1, 8, nTab));
    CPPUNIT_ASSERT_EQUAL(OUString("=C2"), getFormula(2, 8, nTab));
}

void TestCopyPaste::testTdf142065()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetString(1, 0, nTab, "=A1"); // B1
    m_pDoc->SetString(2, 0, nTab, "=SUM(A1:B1)"); // C1
    m_pDoc->SetString(3, 0, nTab, "=$A$1"); // D1
    m_pDoc->SetString(4, 0, nTab, "=$A1"); // E1
    m_pDoc->SetString(5, 0, nTab, "=A$1"); // F1

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2, 0, nTab));

    // Cut A1:F1 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(0, 0, nTab, 5, 0, nTab);
    printRange(m_pDoc, aSrcRange, "Src sheet");
    printFormula(m_pDoc, 1, 0, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);
    printRange(&aClipDoc, aSrcRange, "clip doc (&aClipDoc)");
    printFormula(&aClipDoc, 1, 0, nTab);

    // To A3:A9
    ScRange aDestRange(0, 2, nTab, 0, 7, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    printRange(pTransClip.get(), ScRange(0, 0, nTab, 0, 1, nTab),
               "transposed clip doc (pTransClip.get())");
    printFormula(pTransClip.get(), 0, 1, nTab);
    printFormula(pTransClip.get(), 1, 0, nTab);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    printRange(m_pDoc, aDestRange, "dest doc");
    printFormula(m_pDoc, 0, 3, nTab);
    printRange(pOrigClipDoc, aSrcRange, "orig clip doc (pOrigClipDoc)");
    printFormula(pOrigClipDoc, 1, 0, nTab);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();
    printRange(m_pDoc, aDestRange, "dest doc after UpdateTranspose()");
    printFormula(m_pDoc, 0, 3, nTab);

    // Check results
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 2, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: =A3
    // - Actual  : =#REF!#REF!
    CPPUNIT_ASSERT_EQUAL(OUString("=A3"), getFormula(0, 3, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: 1
    // - Actual  : #REF!
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 3, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: =SUM(A3:A4)
    // - Actual  : =SUM(#REF!#REF!:#REF!#REF!)
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A3:A4)"), getFormula(0, 4, nTab));
    // Without the fix in place, this would have failed with
    // - Expected: 2
    // - Actual  : #REF!
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 4, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=$A$3"), getFormula(0, 5, nTab));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 5, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=$A3"), getFormula(0, 6, nTab));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 6, nTab));

    CPPUNIT_ASSERT_EQUAL(OUString("=A$3"), getFormula(0, 7, nTab));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 7, nTab));
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
    m_xDocShell->DoLoad(pMedium);

    ScDocShellRef xExtDocSh = new ScDocShell;
    xExtDocSh->SetIsInUcalc();
    OUString const aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoLoad(pMed);
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
    m_xDocShell->DoLoad(pMedium);

    ScDocShellRef xExtDocSh = new ScDocShell;
    xExtDocSh->SetIsInUcalc();
    OUString aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoLoad(pMed);
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

void TestCopyPaste::testTdf68976()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetValue(0, 0, nTab, 1.0); // A1
    m_pDoc->SetString(0, 1, nTab, "=$A$1"); // A2
    m_pDoc->SetValue(0, 2, nTab, 1000.0); // A3

    // Cut A3 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(0, 2, nTab, 0, 2, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false); // A3

    ScRange aDestRange(1, 3, nTab, 1, 3, nTab); // B4
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    // Check results
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 0, nTab)); // A1
    // Without the fix in place, this would have failed with
    // - Expected: =$A$1
    // - Actual  : =$B$4
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0, 1, nTab), "$A$1", "Wrong formula");
    // Without the fix in place, this would have failed with
    // - Expected: 1
    // - Actual  : 1000
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 1, nTab)); // A2
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(0, 2, nTab)); // A3
    CPPUNIT_ASSERT_EQUAL(1000.0, m_pDoc->GetValue(1, 3, nTab)); // B4
}

void TestCopyPaste::testTdf71058()
{
    const SCTAB nTab = 0;
    m_pDoc->InsertTab(nTab, "Test");

    m_pDoc->SetString(2, 2, nTab, "=C4"); // C3
    m_pDoc->SetString(3, 2, nTab, "=D4"); // D3
    m_pDoc->SetValue(2, 3, nTab, 1.0); // C4
    m_pDoc->SetValue(3, 3, nTab, 2.0); // D4

    // Cut C4:C5 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(2, 3, nTab, 3, 3, nTab);
    cutToClip(*m_xDocShell, aSrcRange, &aClipDoc, false);

    // To E6:E7
    ScRange aDestRange(4, 5, nTab, 4, 6, nTab);
    ScMarkData aDestMark(m_pDoc->GetSheetLimits());

    // Transpose
    ScDocument* pOrigClipDoc = &aClipDoc;
    ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false, true);
    aDestMark.SetMarkArea(aDestRange);
    // Paste
    m_pDoc->CopyFromClip(aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get(),
                         true, false, true, false);
    m_pDoc->UpdateTranspose(aDestRange.aStart, pOrigClipDoc, aDestMark, nullptr);
    pTransClip.reset();

    // Check precondition
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(4, 5, nTab));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(4, 6, nTab));

    // Check results
    // Without the fix in place, this would have failed with
    // - Expected: =E6
    // - Actual  : =C4
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2, 2, nTab), "E6", "Wrong formula");
    // Without the fix in place, this would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(2, 2, nTab));

    // Without the fix in place, this would have failed with
    // - Expected: =E7
    // - Actual  : =D4
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(3, 2, nTab), "E7", "Wrong formula");
    // Without the fix in place, this would have failed with
    // - Expected: 2
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(3, 2, nTab));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCopyPaste);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
