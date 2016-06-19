/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_UCALC_HXX
#define INCLUDED_SC_QA_UNIT_UCALC_HXX

#include "helper/qahelper.hxx"
#include "document.hxx"
#include <stringutil.hxx>
#include <memory>

struct TestImpl;
class ScUndoPaste;

/**
 * Temporarily set formula grammar.
 */
class FormulaGrammarSwitch
{
    ScDocument* mpDoc;
    formula::FormulaGrammar::Grammar meOldGrammar;
public:
    FormulaGrammarSwitch(ScDocument* pDoc, formula::FormulaGrammar::Grammar eGrammar);
    ~FormulaGrammarSwitch();
};

class Test : public test::BootstrapFixture
{
public:
    struct RangeNameDef
    {
        const char* mpName;
        const char* mpExpr;
        sal_uInt16 mnIndex;
    };

    static ScDocShell* findLoadedDocShellByName(const OUString& rName);
    static bool insertRangeNames(ScDocument* pDoc, ScRangeName* pNames, const RangeNameDef* p, const RangeNameDef* pEnd);
    static void printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption);
    static void clearRange(ScDocument* pDoc, const ScRange& rRange);
    static void clearSheet(ScDocument* pDoc, SCTAB nTab);
    static void copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc);
    static void pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc);
    static ScUndoPaste* createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pUndoDoc);

    /**
     * Enable or disable expand reference options which controls how
     * references in formula are expanded when inserting rows or columns.
     */
    static void setExpandRefs(bool bExpand);

    static void setCalcAsShown(ScDocument* pDoc, bool bCalcAsShown);


    template<size_t Size>
    static ScRange insertRangeData(
        ScDocument* pDoc, const ScAddress& rPos, const char* aData[][Size], size_t nRowCount )
    {
        ScRange aRange(rPos);
        aRange.aEnd.SetCol(rPos.Col()+Size-1);
        aRange.aEnd.SetRow(rPos.Row()+nRowCount-1);

        clearRange(pDoc, aRange);

        for (size_t i = 0; i < Size; ++i)
        {
            for (size_t j = 0; j < nRowCount; ++j)
            {
                if (!aData[j][i])
                    continue;

                SCCOL nCol = i + rPos.Col();
                SCROW nRow = j + rPos.Row();
                OUString aStr(aData[j][i], strlen(aData[j][i]), RTL_TEXTENCODING_UTF8);

                ScSetStringParam aParam; // Leave default.
                aParam.meStartListening = sc::NoListening;
                pDoc->SetString(nCol, nRow, rPos.Tab(), aStr, &aParam);
            }
        }

        pDoc->StartAllListeners(aRange);

        printRange(pDoc, aRange, "Range data content");
        return aRange;
    }

    Test();
    virtual ~Test();

    ScDocShell& getDocShell();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCollator();
    void testSharedStringPool();
    void testSharedStringPoolUndoDoc();
    void testRangeList();
    void testMarkData();
    void testInput();
    void testDocStatistics();

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

    void testFormulaCreateStringFromTokens();
    void testFormulaParseReference();
    void testFetchVectorRefArray();
    void testFormulaHashAndTag();
    void testFormulaTokenEquality();
    void testFormulaRefData();
    void testFormulaCompiler();
    void testFormulaCompilerJumpReordering();
    void testFormulaRefUpdate();
    void testFormulaRefUpdateRange();
    void testFormulaRefUpdateSheets();
    void testFormulaRefUpdateSheetsDelete();
    void testFormulaRefUpdateInsertRows();
    void testFormulaRefUpdateInsertColumns();
    void testFormulaRefUpdateMove();
    void testFormulaRefUpdateMoveUndo();
    void testFormulaRefUpdateMoveToSheet();
    void testFormulaRefUpdateDeleteContent();
    void testFormulaRefUpdateDeleteAndShiftLeft();
    void testFormulaRefUpdateDeleteAndShiftUp();
    void testFormulaRefUpdateName();
    void testFormulaRefUpdateNameMove();
    void testFormulaRefUpdateNameExpandRef();
    void testFormulaRefUpdateNameDeleteRow();
    void testFormulaRefUpdateNameCopySheet();
    void testFormulaRefUpdateNameCopySheetCheckTab( SCTAB Tab, bool bCheckNames );
    void testFormulaRefUpdateNameDelete();
    void testFormulaRefUpdateValidity();
    void testMultipleOperations();
    void testFuncCOLUMN();
    void testFuncCOUNT();
    void testFuncCOUNTBLANK();
    void testFuncROW();
    void testFuncSUM();
    void testFuncPRODUCT();
    void testFuncSUMPRODUCT();
    void testFuncSUMXMY2();
    void testFuncMIN();
    void testFuncN();
    void testFuncCOUNTIF();
    void testFuncNUMBERVALUE();
    void testFuncLEN();
    void testFuncLOOKUP();
    void testFuncVLOOKUP();
    void testFuncMATCH();
    void testFuncCELL();
    void testFuncDATEDIF();
    void testFuncINDIRECT();
    void testFuncINDIRECT2();
    void testFuncIF();
    void testFuncCHOOSE();
    void testFuncIFERROR();
    void testFuncSHEET();
    void testFuncNOW();
    void testFuncGETPIVOTDATA();
    void testFuncGETPIVOTDATALeafAccess();
    void testMatrixOp();
    void testFuncRangeOp();
    void testFuncFORMULA();
    void testFuncTableRef();
    void testFuncFTEST();
    void testFuncFTESTBug();
    void testFuncCHITEST();
    void testFuncTTEST();
    void testFuncSUMX2PY2();
    void testFuncSUMX2MY2();
    void testFuncGCD();
    void testFuncLCM();
    void testFuncSUMSQ();
    void testFuncMDETERM();
    void testMatConcat();
    void testMatConcatReplication();
    void testRefR1C1WholeCol();
    void testRefR1C1WholeRow();

    void testExternalRef();
    void testExternalRefFunctions();
    void testExternalRangeName();
    void testExternalRefUnresolved();

    void testCopyToDocument();

    void testHorizontalIterator();
    void testValueIterator();
    void testHorizontalAttrIterator();

    /**
     * Basic test for formula dependency tracking.
     */
    void testFormulaDepTracking();

    /**
     * Another test for formula dependency tracking, inspired by fdo#56278.
     */
    void testFormulaDepTracking2();

    void testFormulaDepTracking3();

    void testFormulaDepTrackingDeleteRow();

    void testFormulaDepTrackingDeleteCol();

    void testFormulaMatrixResultUpdate();

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

    /**
     * Basic test for pivot tables.
     */
    void testPivotTable();

    /**
     * Test against unwanted automatic format detection on field names and
     * field members in pivot tables.
     */
    void testPivotTableLabels();

    /**
     * Make sure that we set cells displaying date values numeric cells,
     * rather than text cells.  Grouping by date or number functionality
     * depends on this.
     */
    void testPivotTableDateLabels();

    /**
     * Test for pivot table's filtering functionality by page fields.
     */
    void testPivotTableFilters();

    /**
     * Test for pivot table's named source range.
     */
    void testPivotTableNamedSource();

    /**
     * Test for pivot table cache.  Each dimension in the pivot cache stores
     * only unique values that are sorted in ascending order.
     */
    void testPivotTableCache();

    /**
     * Test for pivot table containing data fields that reference the same
     * source field but different functions.
     */
    void testPivotTableDuplicateDataFields();

    void testPivotTableNormalGrouping();
    void testPivotTableNumberGrouping();
    void testPivotTableDateGrouping();
    void testPivotTableEmptyRows();
    void testPivotTableTextNumber();

    /**
     * Test for checking that pivot table treats strings in a case insensitive
     * manner.
     */
    void testPivotTableCaseInsensitiveStrings();

    /**
     * Test for pivot table's handling of double-precision numbers that are
     * very close together.
     */
    void testPivotTableNumStability();

    /**
     * Test for pivot table that include field with various non-default field
     * references.
     */
    void testPivotTableFieldReference();

    /**
     * Test pivot table functionality performed via ScDBDocFunc.
     */
    void testPivotTableDocFunc();

    /**
     * Test pivot table per-field repeat item labels functionality
     */
    void testPivotTableRepeatItemLabels();

    void testCellCopy();
    void testSheetCopy();
    void testSheetMove();
    void testDataArea();
    void testAutofilter();
    void testCopyPaste();
    void testCopyPasteAsLink();
    void testCopyPasteTranspose();
    void testCopyPasteMultiRange();
    void testCopyPasteSkipEmpty();
    void testCopyPasteSkipEmpty2();
    void testCopyPasteSkipEmptyConditionalFormatting();
    void testCutPasteRefUndo();
    void testMoveRefBetweenSheets();
    void testUndoCut();
    void testMoveBlock();
    void testCopyPasteRelativeFormula();
    void testCopyPasteRepeatOneFormula();
    void testCopyPasteMixedReferenceFormula();
    void testMergedCells();
    void testUpdateReference();
    void testSearchCells();
    void testSharedFormulas();
    void testSharedFormulasRefUpdate();
    void testSharedFormulasRefUpdateMove();
    void testSharedFormulasRefUpdateMove2();
    void testSharedFormulasRefUpdateRange();
    void testSharedFormulasRefUpdateRangeDeleteRow();
    void testSharedFormulasRefUpdateExternal();
    void testSharedFormulasInsertRow();
    void testSharedFormulasDeleteRows();
    void testSharedFormulasDeleteColumns();
    void testSharedFormulasRefUpdateMoveSheets();
    void testSharedFormulasRefUpdateCopySheets();
    void testSharedFormulasRefUpdateDeleteSheets();
    void testSharedFormulasCopyPaste();
    void testSharedFormulaInsertColumn();
    void testSharedFormulaMoveBlock();
    void testSharedFormulaUpdateOnNamedRangeChange();
    void testSharedFormulaUpdateOnDBChange();
    void testSharedFormulaAbsCellListener();
    void testSharedFormulaUnshareAreaListeners();
    void testSharedFormulaListenerDeleteArea();
    void testSharedFormulaUpdateOnReplacement();
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

    void testAutoFill();
    void testAutoFillSimple();
    void testCopyPasteFormulas();
    void testCopyPasteFormulasExternalDoc();

    void testFindAreaPosVertical();
    void testFindAreaPosColRight();
    void testShiftCells();

    void testSort();
    void testSortHorizontal();
    void testSortHorizontalWholeColumn();
    void testSortSingleRow();
    void testSortWithFormulaRefs();
    void testSortWithStrings();
    void testSortInFormulaGroup();
    void testSortWithCellFormats();
    void testSortRefUpdate();
    void testSortRefUpdate2();
    void testSortRefUpdate3();
    void testSortRefUpdate4();
    void testSortRefUpdate4_Impl();
    void testSortRefUpdate5();
    void testSortRefUpdate6();
    void testSortBroadcaster();
    void testSortBroadcastBroadcaster();
    void testSortOutOfPlaceResult();
    void testSortPartialFormulaGroup();

    void testNoteBasic();
    void testNoteDeleteRow();
    void testNoteDeleteCol();
    void testNoteLifeCycle();
    void testNoteCopyPaste();
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

    // Check that the Listeners are correctly updated when we
    // call a operation
    void testCondFormatUpdateMoveTab();
    void testCondFormatUpdateDeleteTab();
    void testCondFormatUpdateInsertTab();
    void testCondFormatUpdateReference();

    void testCondFormatEndsWithStr();
    void testCondFormatEndsWithVal();

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

    void testTdf97369();
    void testTdf97587();

    void testEmptyCalcDocDefaults();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCollator);
    CPPUNIT_TEST(testSharedStringPool);
    CPPUNIT_TEST(testSharedStringPoolUndoDoc);
    CPPUNIT_TEST(testRangeList);
    CPPUNIT_TEST(testMarkData);
    CPPUNIT_TEST(testInput);
    CPPUNIT_TEST(testDocStatistics);
    CPPUNIT_TEST(testDataEntries);
    CPPUNIT_TEST(testSelectionFunction);
    CPPUNIT_TEST(testFormulaCreateStringFromTokens);
    CPPUNIT_TEST(testFormulaParseReference);
    CPPUNIT_TEST(testFetchVectorRefArray);
    // CPPUNIT_TEST(testFormulaHashAndTag);
    CPPUNIT_TEST(testFormulaTokenEquality);
    CPPUNIT_TEST(testFormulaRefData);
    CPPUNIT_TEST(testFormulaCompiler);
    CPPUNIT_TEST(testFormulaCompilerJumpReordering);
    CPPUNIT_TEST(testFormulaRefUpdate);
    CPPUNIT_TEST(testFormulaRefUpdateRange);
    CPPUNIT_TEST(testFormulaRefUpdateSheets);
    CPPUNIT_TEST(testFormulaRefUpdateSheetsDelete);
    CPPUNIT_TEST(testFormulaRefUpdateInsertRows);
    CPPUNIT_TEST(testFormulaRefUpdateInsertColumns);
    CPPUNIT_TEST(testFormulaRefUpdateMove);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndo);
    CPPUNIT_TEST(testFormulaRefUpdateMoveToSheet);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteContent);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteAndShiftLeft);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteAndShiftUp);
    CPPUNIT_TEST(testFormulaRefUpdateName);
    CPPUNIT_TEST(testFormulaRefUpdateNameMove);
    CPPUNIT_TEST(testFormulaRefUpdateNameExpandRef);
    CPPUNIT_TEST(testFormulaRefUpdateNameDeleteRow);
    CPPUNIT_TEST(testFormulaRefUpdateNameCopySheet);
    CPPUNIT_TEST(testFormulaRefUpdateNameDelete);
    CPPUNIT_TEST(testFormulaRefUpdateValidity);
    CPPUNIT_TEST(testMultipleOperations);
    CPPUNIT_TEST(testFuncCOLUMN);
    CPPUNIT_TEST(testFuncCOUNT);
    CPPUNIT_TEST(testFuncCOUNTBLANK);
    CPPUNIT_TEST(testFuncROW);
    CPPUNIT_TEST(testFuncSUM);
    CPPUNIT_TEST(testFuncPRODUCT);
    CPPUNIT_TEST(testFuncSUMPRODUCT);
    CPPUNIT_TEST(testFuncSUMXMY2);
    CPPUNIT_TEST(testFuncMIN);
    CPPUNIT_TEST(testFuncN);
    CPPUNIT_TEST(testFuncCOUNTIF);
    CPPUNIT_TEST(testFuncNUMBERVALUE);
    CPPUNIT_TEST(testFuncLEN);
    CPPUNIT_TEST(testFuncLOOKUP);
    CPPUNIT_TEST(testFuncVLOOKUP);
    CPPUNIT_TEST(testFuncMATCH);
    CPPUNIT_TEST(testFuncCELL);
    CPPUNIT_TEST(testFuncDATEDIF);
    CPPUNIT_TEST(testFuncINDIRECT);
    CPPUNIT_TEST(testFuncINDIRECT2);
    CPPUNIT_TEST(testFuncIF);
    CPPUNIT_TEST(testFuncCHOOSE);
    CPPUNIT_TEST(testFuncIFERROR);
    CPPUNIT_TEST(testFuncGETPIVOTDATA);
    CPPUNIT_TEST(testFuncGETPIVOTDATALeafAccess);
    CPPUNIT_TEST(testRefR1C1WholeCol);
    CPPUNIT_TEST(testRefR1C1WholeRow);
    CPPUNIT_TEST(testMatrixOp);
    CPPUNIT_TEST(testFuncRangeOp);
    CPPUNIT_TEST(testFuncFORMULA);
    CPPUNIT_TEST(testFuncTableRef);
    CPPUNIT_TEST(testFuncFTEST);
    CPPUNIT_TEST(testFuncFTESTBug);
    CPPUNIT_TEST(testFuncCHITEST);
    CPPUNIT_TEST(testFuncTTEST);
    CPPUNIT_TEST(testFuncSUMX2PY2);
    CPPUNIT_TEST(testFuncSUMX2MY2);
    CPPUNIT_TEST(testFuncGCD);
    CPPUNIT_TEST(testFuncLCM);
    CPPUNIT_TEST(testFuncSUMSQ);
    CPPUNIT_TEST(testFuncMDETERM);
    CPPUNIT_TEST(testMatConcat);
    CPPUNIT_TEST(testMatConcatReplication);
    CPPUNIT_TEST(testExternalRef);
    CPPUNIT_TEST(testExternalRangeName);
    CPPUNIT_TEST(testExternalRefFunctions);
    // currently crashes windows
    // CPPUNIT_TEST(testExternalRefUnresolved);
    CPPUNIT_TEST(testCopyToDocument);
    CPPUNIT_TEST(testFuncSHEET);
    CPPUNIT_TEST(testFuncNOW);
    CPPUNIT_TEST(testHorizontalIterator);
    CPPUNIT_TEST(testValueIterator);
    CPPUNIT_TEST(testHorizontalAttrIterator);
    CPPUNIT_TEST(testFormulaDepTracking);
    CPPUNIT_TEST(testFormulaDepTracking2);
    CPPUNIT_TEST(testFormulaDepTracking3);
    CPPUNIT_TEST(testFormulaDepTrackingDeleteRow);
    CPPUNIT_TEST(testFormulaDepTrackingDeleteCol);
    CPPUNIT_TEST(testFormulaMatrixResultUpdate);
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
    CPPUNIT_TEST(testPivotTable);
    CPPUNIT_TEST(testPivotTableLabels);
    CPPUNIT_TEST(testPivotTableDateLabels);
    CPPUNIT_TEST(testPivotTableFilters);
    CPPUNIT_TEST(testPivotTableNamedSource);
    CPPUNIT_TEST(testPivotTableCache);
    CPPUNIT_TEST(testPivotTableDuplicateDataFields);
    CPPUNIT_TEST(testPivotTableNormalGrouping);
    CPPUNIT_TEST(testPivotTableNumberGrouping);
    CPPUNIT_TEST(testPivotTableDateGrouping);
    CPPUNIT_TEST(testPivotTableEmptyRows);
    CPPUNIT_TEST(testPivotTableTextNumber);
    CPPUNIT_TEST(testPivotTableCaseInsensitiveStrings);
    CPPUNIT_TEST(testPivotTableNumStability);
    CPPUNIT_TEST(testPivotTableFieldReference);
    CPPUNIT_TEST(testPivotTableDocFunc);
    CPPUNIT_TEST(testPivotTableRepeatItemLabels);
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
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testCopyPasteAsLink);
    CPPUNIT_TEST(testCopyPasteTranspose);
    CPPUNIT_TEST(testCopyPasteMultiRange);
    CPPUNIT_TEST(testCopyPasteSkipEmpty);
    CPPUNIT_TEST(testCopyPasteSkipEmpty2);
    //CPPUNIT_TEST(testCopyPasteSkipEmptyConditionalFormatting);
    CPPUNIT_TEST(testCutPasteRefUndo);
    CPPUNIT_TEST(testMoveRefBetweenSheets);
    CPPUNIT_TEST(testUndoCut);
    CPPUNIT_TEST(testMoveBlock);
    CPPUNIT_TEST(testCopyPasteRelativeFormula);
    CPPUNIT_TEST(testCopyPasteRepeatOneFormula);
    CPPUNIT_TEST(testCopyPasteMixedReferenceFormula);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testUpdateReference);
    CPPUNIT_TEST(testSearchCells);
    CPPUNIT_TEST(testSharedFormulas);
    CPPUNIT_TEST(testSharedFormulasRefUpdate);
    CPPUNIT_TEST(testSharedFormulasRefUpdateMove);
    CPPUNIT_TEST(testSharedFormulasRefUpdateMove2);
    CPPUNIT_TEST(testSharedFormulasRefUpdateRange);
    CPPUNIT_TEST(testSharedFormulasRefUpdateRangeDeleteRow);
    CPPUNIT_TEST(testSharedFormulasRefUpdateExternal);
    CPPUNIT_TEST(testSharedFormulasInsertRow);
    CPPUNIT_TEST(testSharedFormulasDeleteRows);
    CPPUNIT_TEST(testSharedFormulasDeleteColumns);
    CPPUNIT_TEST(testSharedFormulasRefUpdateMoveSheets);
    CPPUNIT_TEST(testSharedFormulasRefUpdateCopySheets);
    CPPUNIT_TEST(testSharedFormulasRefUpdateDeleteSheets);
    CPPUNIT_TEST(testSharedFormulasCopyPaste);
    CPPUNIT_TEST(testSharedFormulaInsertColumn);
    CPPUNIT_TEST(testSharedFormulaUpdateOnNamedRangeChange);
    CPPUNIT_TEST(testSharedFormulaUpdateOnDBChange);
    CPPUNIT_TEST(testSharedFormulaAbsCellListener);
    CPPUNIT_TEST(testSharedFormulaUnshareAreaListeners);
    CPPUNIT_TEST(testSharedFormulaListenerDeleteArea);
    CPPUNIT_TEST(testSharedFormulaUpdateOnReplacement);
    CPPUNIT_TEST(testFormulaPosition);
    CPPUNIT_TEST(testFormulaWizardSubformula);
    CPPUNIT_TEST(testMixData);
    CPPUNIT_TEST(testJumpToPrecedentsDependents);
    CPPUNIT_TEST(testSetBackgroundColor);
    CPPUNIT_TEST(testRenameTable);
    CPPUNIT_TEST(testAutoFill);
    CPPUNIT_TEST(testAutoFillSimple);
    CPPUNIT_TEST(testCopyPasteFormulas);
    CPPUNIT_TEST(testCopyPasteFormulasExternalDoc);
    CPPUNIT_TEST(testFindAreaPosVertical);
    CPPUNIT_TEST(testFindAreaPosColRight);
    CPPUNIT_TEST(testSort);
    CPPUNIT_TEST(testSortHorizontal);
    CPPUNIT_TEST(testSortHorizontalWholeColumn);
    CPPUNIT_TEST(testSortSingleRow);
    CPPUNIT_TEST(testSortWithFormulaRefs);
    CPPUNIT_TEST(testSortWithStrings);
    CPPUNIT_TEST(testSortInFormulaGroup);
    CPPUNIT_TEST(testSortWithCellFormats);
    CPPUNIT_TEST(testSortRefUpdate);
    CPPUNIT_TEST(testSortRefUpdate2);
    CPPUNIT_TEST(testSortRefUpdate3);
    CPPUNIT_TEST(testSortRefUpdate4);
    CPPUNIT_TEST(testSortRefUpdate5);
    CPPUNIT_TEST(testSortRefUpdate6);
    CPPUNIT_TEST(testSortBroadcaster);
    CPPUNIT_TEST(testSortBroadcastBroadcaster);
    CPPUNIT_TEST(testSortOutOfPlaceResult);
    CPPUNIT_TEST(testSortPartialFormulaGroup);
    CPPUNIT_TEST(testShiftCells);
    CPPUNIT_TEST(testNoteBasic);
    CPPUNIT_TEST(testNoteDeleteRow);
    CPPUNIT_TEST(testNoteDeleteCol);
    CPPUNIT_TEST(testNoteLifeCycle);
    CPPUNIT_TEST(testNoteCopyPaste);
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
    CPPUNIT_TEST(testCondCopyPasteSheetBetweenDoc);
    CPPUNIT_TEST(testCondCopyPasteSheet);
    CPPUNIT_TEST(testCondFormatEndsWithStr);
    CPPUNIT_TEST(testCondFormatEndsWithVal);
    CPPUNIT_TEST(testIconSet);
    CPPUNIT_TEST(testDataBarLengthAutomaticAxis);
    CPPUNIT_TEST(testDataBarLengthMiddleAxis);
    CPPUNIT_TEST(testFormulaListenerSingleCellToSingleCell);
    CPPUNIT_TEST(testFormulaListenerSingleCellToMultipleCells);
    CPPUNIT_TEST(testFormulaListenerMultipleCellsToSingleCell);
    CPPUNIT_TEST(testFormulaListenerMultipleCellsToMultipleCells);
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
    CPPUNIT_TEST(testTdf97369);
    CPPUNIT_TEST(testTdf97587);
    CPPUNIT_TEST(testEmptyCalcDocDefaults);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<TestImpl> m_pImpl;
    ScDocument *m_pDoc;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
