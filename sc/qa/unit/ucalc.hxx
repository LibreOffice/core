/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/bootstrapfixture.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <stringutil.hxx>
#include <memory>
#include <clipparam.hxx>

struct TestImpl;
class ScUndoPaste;
class ScUndoCut;

class Test : public test::BootstrapFixture
{
public:
    void checkPrecisionAsShown(OUString& rCode, double fValue, double fExpectedRoundVal);

    Test();
    virtual ~Test() override;

    ScDocShell& getDocShell();

    /** Get a separate new ScDocShell with ScDocument that suits unit test needs. */
    void getNewDocShell(ScDocShellRef& rDocShellRef);
    /** Close such new ScDocShell. */
    void closeDocShell(ScDocShellRef& rDocShellRef);

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCollator();
    void testSharedStringPool();
    void testSharedStringPoolUndoDoc();
    void testRangeList();
    void testMarkData();
    void testInput();
    void testColumnIterator();
    void testTdf90698();
    void testTdf134490();
    void testTdf135249();
    void testDocStatistics();
    void testRowForHeight();

    /**
     * The 'data entries' data is a list of strings used for suggestions as
     * the user types in new cell value.
     */
    void testDataEntries();

    /**
     * Selection function is responsible for displaying quick calculation
     * results in the status bar.
     */
    void testSelectionFunction();

    void testMarkedCellIteration();

    void testCopyToDocument();

    void testHorizontalIterator();
    void testValueIterator();
    void testHorizontalAttrIterator();

    /**
     * More direct test for cell broadcaster management, used to track formula
     * dependencies.
     */
    void testCellBroadcaster();

    void testFuncParam();
    void testNamedRange();
    void testInsertNameList();
    void testCSV();
    void testMatrix();
    void testMatrixComparisonWithErrors();
    void testMatrixConditionalBooleanResult();
    void testEnterMixedMatrix();
    void testMatrixEditable();

    void testCellCopy();
    void testSheetCopy();
    void testSheetMove();
    void testDataArea();
    void testAutofilter();
    void testAutoFilterTimeValue();
    void testAdvancedFilter();
    void testTdf98642();
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
    void testCopyPasteSkipEmptyConditionalFormatting();
    void testCutPasteRefUndo();
    void testCutPasteGroupRefUndo();
    void testMoveRefBetweenSheets();
    void testUndoCut();
    void testMoveBlock();
    void testCopyPasteRelativeFormula();
    void testCopyPasteRepeatOneFormula();
    void testCopyPasteMixedReferenceFormula();
    void testMergedCells();
    void testUpdateReference();
    void testSearchCells();
    void testFormulaPosition();
    void testFormulaWizardSubformula();

    void testMixData();

    /**
     * Make sure the sheet streams are invalidated properly.
     */
    void testStreamValid();

    /**
     * Test built-in cell functions to make sure their categories and order
     * are correct.
     */
    void testFunctionLists();

    void testGraphicsInGroup();
    void testGraphicsOnSheetMove();

    /**
     * Test toggling relative/absolute flag of cell and cell range references.
     * This corresponds with hitting Shift-F4 while the cursor is on a formula
     * cell.
     */
    void testToggleRefFlag();

    /**
     * Test to make sure correct precedent / dependent cells are obtained when
     * preparing to jump to them.
     */
    void testJumpToPrecedentsDependents();

    void testSetBackgroundColor();
    void testRenameTable();

    void testTdf64001();
    void testAutoFill();
    void testAutoFillSimple();
    void testCopyPasteFormulas();
    void testCopyPasteFormulasExternalDoc();
    void testCopyPasteReferencesExternalDoc(); // tdf#106456

    void testFindAreaPosVertical();
    void testFindAreaPosColRight();
    void testShiftCells();

    void testNoteBasic();
    void testNoteDeleteRow();
    void testNoteDeleteCol();
    void testNoteLifeCycle();
    void testNoteCopyPaste();
    void testNoteContainsNotesInRange();
    void testAreasWithNotes();
    void testAnchoredRotatedShape();
    void testCellTextWidth();
    void testEditTextIterator();

    // conditional format tests
    // mostly in ucalc_condformat.cxx
    void testCondFormatINSDEL();
    void testCondFormatInsertRow();
    void testCondFormatInsertCol();
    void testCondFormatInsertDeleteSheets();
    void testCondCopyPaste();
    void testCondCopyPasteSingleCell(); //e.g. fdo#82503
    void testCondCopyPasteSingleCellToRange(); //e.g. fdo#82503
    void testCondCopyPasteSingleCellIntoSameFormatRange(); // e.g., tdf#95295
    void testCondCopyPasteSingleRowToRange(); //e.g. tdf#106242
    void testCondCopyPasteSingleRowToRange2();
    void testCondCopyPasteSheetBetweenDoc();
    void testCondCopyPasteSheet();
    void testIconSet();
    void testDataBarLengthAutomaticAxis();
    void testDataBarLengthMiddleAxis();

    // Tests for the ScFormulaListener class
    void testFormulaListenerSingleCellToSingleCell();
    void testFormulaListenerMultipleCellsToSingleCell();
    void testFormulaListenerSingleCellToMultipleCells();
    void testFormulaListenerMultipleCellsToMultipleCells();
    void testFormulaListenerUpdateInsertTab();
    void testFormulaListenerUpdateDeleteTab();

    // Check that the Listeners are correctly updated when we
    // call an operation
    void testCondFormatUpdateMoveTab();
    void testCondFormatUpdateDeleteTab();
    void testCondFormatUpdateInsertTab();
    void testCondFormatUpdateReference();
    void testCondFormatUpdateReferenceDelRow();
    void testCondFormatUpdateReferenceInsRow();

    void testCondFormatEndsWithStr();
    void testCondFormatEndsWithVal();

    void testCondFormatUndoList();
    void testMultipleSingleCellCondFormatCopyPaste();
    void testDeduplicateMultipleCondFormats();
    void testCondFormatListenToOwnRange();
    void testCondFormatVolatileFunctionRecalc();

    void testImportStream();
    void testDeleteContents();
    void testTransliterateText();

    void testFormulaToValue();
    void testFormulaToValue2();

    void testColumnFindEditCells();
    void testSetStringAndNote();

    // tdf#80137
    void testCopyPasteMatrixFormula();
    void testUndoDataAnchor();
    void testFormulaErrorPropagation();
    void testSetFormula();
    void testMultipleDataCellsInRange();

    void testEmptyCalcDocDefaults();

    void testPrecisionAsShown();
    void testProtectedSheetEditByRow();
    void testProtectedSheetEditByColumn();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCollator);
    CPPUNIT_TEST(testSharedStringPool);
    CPPUNIT_TEST(testSharedStringPoolUndoDoc);
    CPPUNIT_TEST(testRangeList);
    CPPUNIT_TEST(testMarkData);
    CPPUNIT_TEST(testInput);
    CPPUNIT_TEST(testColumnIterator);
    CPPUNIT_TEST(testTdf90698);
    CPPUNIT_TEST(testTdf134490);
    CPPUNIT_TEST(testTdf135249);
    CPPUNIT_TEST(testDocStatistics);
    CPPUNIT_TEST(testRowForHeight);
    CPPUNIT_TEST(testDataEntries);
    CPPUNIT_TEST(testSelectionFunction);
    CPPUNIT_TEST(testMarkedCellIteration);
    CPPUNIT_TEST(testCopyToDocument);
    CPPUNIT_TEST(testHorizontalIterator);
    CPPUNIT_TEST(testValueIterator);
    CPPUNIT_TEST(testHorizontalAttrIterator);
    CPPUNIT_TEST(testCellBroadcaster);
    CPPUNIT_TEST(testFuncParam);
    CPPUNIT_TEST(testNamedRange);
    CPPUNIT_TEST(testInsertNameList);
    CPPUNIT_TEST(testCSV);
    CPPUNIT_TEST(testMatrix);
    CPPUNIT_TEST(testMatrixComparisonWithErrors);
    CPPUNIT_TEST(testMatrixConditionalBooleanResult);
    CPPUNIT_TEST(testEnterMixedMatrix);
    CPPUNIT_TEST(testMatrixEditable);
    CPPUNIT_TEST(testCellCopy);
    CPPUNIT_TEST(testSheetCopy);
    CPPUNIT_TEST(testSheetMove);
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testGraphicsInGroup);
    CPPUNIT_TEST(testGraphicsOnSheetMove);
    CPPUNIT_TEST(testStreamValid);
    CPPUNIT_TEST(testFunctionLists);
    CPPUNIT_TEST(testToggleRefFlag);
    CPPUNIT_TEST(testAutofilter);
    CPPUNIT_TEST(testAutoFilterTimeValue);
    CPPUNIT_TEST(testAdvancedFilter);
    CPPUNIT_TEST(testTdf98642);
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
    //CPPUNIT_TEST(testCopyPasteSkipEmptyConditionalFormatting);
    CPPUNIT_TEST(testCutPasteRefUndo);
    CPPUNIT_TEST(testCutPasteGroupRefUndo);
    CPPUNIT_TEST(testMoveRefBetweenSheets);
    CPPUNIT_TEST(testUndoCut);
    CPPUNIT_TEST(testMoveBlock);
    CPPUNIT_TEST(testCopyPasteRelativeFormula);
    CPPUNIT_TEST(testCopyPasteRepeatOneFormula);
    CPPUNIT_TEST(testCopyPasteMixedReferenceFormula);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testUpdateReference);
    CPPUNIT_TEST(testSearchCells);
    CPPUNIT_TEST(testFormulaPosition);
    CPPUNIT_TEST(testFormulaWizardSubformula);
    CPPUNIT_TEST(testMixData);
    CPPUNIT_TEST(testJumpToPrecedentsDependents);
    CPPUNIT_TEST(testSetBackgroundColor);
    CPPUNIT_TEST(testRenameTable);
    CPPUNIT_TEST(testTdf64001);
    CPPUNIT_TEST(testAutoFill);
    CPPUNIT_TEST(testAutoFillSimple);
    CPPUNIT_TEST(testCopyPasteFormulas);
    CPPUNIT_TEST(testCopyPasteFormulasExternalDoc);
    CPPUNIT_TEST(testCopyPasteReferencesExternalDoc);
    CPPUNIT_TEST(testFindAreaPosVertical);
    CPPUNIT_TEST(testFindAreaPosColRight);
    CPPUNIT_TEST(testShiftCells);
    CPPUNIT_TEST(testNoteBasic);
    CPPUNIT_TEST(testNoteDeleteRow);
    CPPUNIT_TEST(testNoteDeleteCol);
    CPPUNIT_TEST(testNoteLifeCycle);
    CPPUNIT_TEST(testNoteCopyPaste);
    CPPUNIT_TEST(testNoteContainsNotesInRange);
    CPPUNIT_TEST(testAreasWithNotes);
    CPPUNIT_TEST(testAnchoredRotatedShape);
    CPPUNIT_TEST(testCellTextWidth);
    CPPUNIT_TEST(testEditTextIterator);
    CPPUNIT_TEST(testCondFormatINSDEL);
    CPPUNIT_TEST(testCondFormatInsertRow);
    CPPUNIT_TEST(testCondFormatInsertCol);
    CPPUNIT_TEST(testCondFormatInsertDeleteSheets);
    CPPUNIT_TEST(testCondCopyPaste);
    CPPUNIT_TEST(testCondCopyPasteSingleCell);
    CPPUNIT_TEST(testCondCopyPasteSingleCellToRange);
    CPPUNIT_TEST(testCondCopyPasteSingleCellIntoSameFormatRange);
    CPPUNIT_TEST(testCondCopyPasteSingleRowToRange);
    CPPUNIT_TEST(testCondCopyPasteSingleRowToRange2);
    CPPUNIT_TEST(testCondCopyPasteSheetBetweenDoc);
    CPPUNIT_TEST(testCondCopyPasteSheet);
    CPPUNIT_TEST(testCondFormatEndsWithStr);
    CPPUNIT_TEST(testCondFormatEndsWithVal);
    CPPUNIT_TEST(testCondFormatUpdateReferenceDelRow);
    CPPUNIT_TEST(testCondFormatUpdateReferenceInsRow);
    CPPUNIT_TEST(testCondFormatUndoList);
    CPPUNIT_TEST(testMultipleSingleCellCondFormatCopyPaste);
    CPPUNIT_TEST(testDeduplicateMultipleCondFormats);
    CPPUNIT_TEST(testCondFormatVolatileFunctionRecalc);
    CPPUNIT_TEST(testIconSet);
    CPPUNIT_TEST(testDataBarLengthAutomaticAxis);
    CPPUNIT_TEST(testDataBarLengthMiddleAxis);
    CPPUNIT_TEST(testFormulaListenerSingleCellToSingleCell);
    CPPUNIT_TEST(testFormulaListenerSingleCellToMultipleCells);
    CPPUNIT_TEST(testFormulaListenerMultipleCellsToSingleCell);
    CPPUNIT_TEST(testFormulaListenerMultipleCellsToMultipleCells);
    CPPUNIT_TEST(testFormulaListenerUpdateInsertTab);
    CPPUNIT_TEST(testFormulaListenerUpdateDeleteTab);
    CPPUNIT_TEST(testImportStream);
    CPPUNIT_TEST(testDeleteContents);
    CPPUNIT_TEST(testTransliterateText);
    CPPUNIT_TEST(testFormulaToValue);
    CPPUNIT_TEST(testFormulaToValue2);
    CPPUNIT_TEST(testColumnFindEditCells);
    CPPUNIT_TEST(testSetStringAndNote);
    CPPUNIT_TEST(testCopyPasteMatrixFormula);
    CPPUNIT_TEST(testUndoDataAnchor);
    CPPUNIT_TEST(testFormulaErrorPropagation);
    CPPUNIT_TEST(testSetFormula);
    CPPUNIT_TEST(testMultipleDataCellsInRange);
    CPPUNIT_TEST(testEmptyCalcDocDefaults);
    CPPUNIT_TEST(testPrecisionAsShown);
    CPPUNIT_TEST(testProtectedSheetEditByRow);
    CPPUNIT_TEST(testProtectedSheetEditByColumn);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<TestImpl> m_pImpl;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
