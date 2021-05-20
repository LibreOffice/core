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

class ScUndoPaste;
class ScUndoCut;

class Test : public test::BootstrapFixture
{
public:
    sal_uInt32 getUserDefinedFormat(OUString& rCode);
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
    void testUserDefinedFormats();
    void testAdvancedFilter();
    void testTdf98642();
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
    CPPUNIT_TEST(testUserDefinedFormats);
    CPPUNIT_TEST(testAdvancedFilter);
    CPPUNIT_TEST(testTdf98642);
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
    CPPUNIT_TEST(testImportStream);
    CPPUNIT_TEST(testDeleteContents);
    CPPUNIT_TEST(testTransliterateText);
    CPPUNIT_TEST(testFormulaToValue);
    CPPUNIT_TEST(testFormulaToValue2);
    CPPUNIT_TEST(testColumnFindEditCells);
    CPPUNIT_TEST(testSetStringAndNote);
    CPPUNIT_TEST(testCopyPasteMatrixFormula);
    CPPUNIT_TEST(testUndoDataAnchor);
    CPPUNIT_TEST(testSetFormula);
    CPPUNIT_TEST(testMultipleDataCellsInRange);
    CPPUNIT_TEST(testEmptyCalcDocDefaults);
    CPPUNIT_TEST(testPrecisionAsShown);
    CPPUNIT_TEST(testProtectedSheetEditByRow);
    CPPUNIT_TEST(testProtectedSheetEditByColumn);
    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef m_xDocShell;
    ScDocument* m_pDoc;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
