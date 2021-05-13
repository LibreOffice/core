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
    struct RangeNameDef
    {
        const char* mpName;
        const char* mpExpr;
        sal_uInt16 mnIndex;
    };

    static ScDocShell* findLoadedDocShellByName(std::u16string_view rName);
    static bool insertRangeNames(ScDocument* pDoc, ScRangeName* pNames, const RangeNameDef* p,
                                 const RangeNameDef* pEnd);
    static ScUndoCut* cutToClip(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pClipDoc,
                                bool bCreateUndo);
    static void copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc);
    static void pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange,
                              ScDocument* pClipDoc);
    static void pasteOneCellFromClip(ScDocument* pDestDoc, const ScRange& rDestRange,
                                     ScDocument* pClipDoc,
                                     InsertDeleteFlags eFlags = InsertDeleteFlags::ALL);
    static ScUndoPaste* createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange,
                                        ScDocumentUniquePtr pUndoDoc);

    /**
     * Enable or disable expand reference options which controls how
     * references in formula are expanded when inserting rows or columns.
     */
    static void setExpandRefs(bool bExpand);

    static void setCalcAsShown(ScDocument* pDoc, bool bCalcAsShown);

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

    void testFormulaCreateStringFromTokens();
    void testFormulaParseReference();
    void testFetchVectorRefArray();
    void testGroupConverter3D();
    void testFormulaHashAndTag();
    void testFormulaTokenEquality();
    void testFormulaRefData();
    void testFormulaCompiler();
    void testFormulaCompilerJumpReordering();
    void testFormulaCompilerImplicitIntersection2Param();
    void testFormulaCompilerImplicitIntersection1ParamNoChange();
    void testFormulaCompilerImplicitIntersection1ParamWithChange();
    void testFormulaCompilerImplicitIntersection1NoGroup();
    void testFormulaCompilerImplicitIntersectionOperators();
    void testFormulaAnnotateTrimOnDoubleRefs();
    void testFormulaRefUpdate();
    void testFormulaRefUpdateRange();
    void testFormulaRefUpdateSheets();
    void testFormulaRefUpdateSheetsDelete();
    void testFormulaRefUpdateInsertRows();
    void testFormulaRefUpdateInsertColumns();
    void testFormulaRefUpdateMove();
    void testFormulaRefUpdateMoveUndo();
    void testFormulaRefUpdateMoveUndo2();
    void testFormulaRefUpdateMoveUndo3NonShared();
    void testFormulaRefUpdateMoveUndo3Shared();
    void testFormulaRefUpdateMoveUndoDependents();
    void testFormulaRefUpdateMoveUndo4();
    void testFormulaRefUpdateMoveToSheet();
    void testFormulaRefUpdateDeleteContent();
    void testFormulaRefUpdateDeleteAndShiftLeft();
    void testFormulaRefUpdateDeleteAndShiftLeft2();
    void testFormulaRefUpdateDeleteAndShiftUp();
    void testFormulaRefUpdateName();
    void testFormulaRefUpdateNameMove();
    void testFormulaRefUpdateNameExpandRef();
    void testFormulaRefUpdateNameExpandRef2();
    void testFormulaRefUpdateNameDeleteRow();
    void testFormulaRefUpdateNameCopySheet();
    void testFormulaRefUpdateNameCopySheetCheckTab(SCTAB Tab, bool bCheckNames);
    void testFormulaRefUpdateSheetLocalMove();
    void testFormulaRefUpdateNameDelete();
    void testFormulaRefUpdateValidity();
    void testTokenArrayRefUpdateMove();
    void testSingleCellCopyColumnLabel();
    void testIntersectionOpExcel();
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
    void testFuncLOOKUParrayWithError();
    void testTdf141146();
    void testFuncVLOOKUP();
    void testFuncMATCH();
    void testFuncCELL();
    void testFuncDATEDIF();
    void testFuncINDIRECT();
    void testFuncINDIRECT2();
    void testFunc_MATCH_INDIRECT();
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
    void testFuncSUMIFS();
    void testFuncRefListArraySUBTOTAL();
    void testFuncJumpMatrixArrayIF();
    void testFuncJumpMatrixArrayOFFSET();
    void testMatConcat();
    void testMatConcatReplication();
    void testRefR1C1WholeCol();
    void testRefR1C1WholeRow();
    void testIterations();

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

    /**
     * Test DPCollection public methods
     */
    void testPivotTableDPCollection();

    /**
    * Test pivot table median function
    */
    void testPivotTableMedianFunc();

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
    void testSharedFormulaDeleteTopCell();
    void testSharedFormulaCutCopyMoveIntoRef();
    void testSharedFormulaCutCopyMoveWithRef();
    void testSharedFormulaCutCopyMoveWithinRun();
    void testSharedFormulaInsertShift();
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

    void testTdf97369();
    void testTdf97587();
    void testTdf93415();
    void testTdf100818();

    void testEmptyCalcDocDefaults();

    void testPrecisionAsShown();
    void testProtectedSheetEditByRow();
    void testProtectedSheetEditByColumn();
    void testFuncRowsHidden();
    void testInsertColCellStoreEventSwap();
    void testFormulaAfterDeleteRows();

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
    CPPUNIT_TEST(testFormulaCreateStringFromTokens);
    CPPUNIT_TEST(testFormulaParseReference);
    CPPUNIT_TEST(testFetchVectorRefArray);
    CPPUNIT_TEST(testGroupConverter3D);
    // CPPUNIT_TEST(testFormulaHashAndTag);
    CPPUNIT_TEST(testFormulaTokenEquality);
    CPPUNIT_TEST(testFormulaRefData);
    CPPUNIT_TEST(testFormulaCompiler);
    CPPUNIT_TEST(testFormulaCompilerJumpReordering);
    CPPUNIT_TEST(testFormulaCompilerImplicitIntersection2Param);
    CPPUNIT_TEST(testFormulaCompilerImplicitIntersection1ParamNoChange);
    CPPUNIT_TEST(testFormulaCompilerImplicitIntersection1ParamWithChange);
    CPPUNIT_TEST(testFormulaCompilerImplicitIntersection1NoGroup);
    CPPUNIT_TEST(testFormulaCompilerImplicitIntersectionOperators);
    CPPUNIT_TEST(testFormulaAnnotateTrimOnDoubleRefs);
    CPPUNIT_TEST(testFormulaRefUpdate);
    CPPUNIT_TEST(testFormulaRefUpdateRange);
    CPPUNIT_TEST(testFormulaRefUpdateSheets);
    CPPUNIT_TEST(testFormulaRefUpdateSheetsDelete);
    CPPUNIT_TEST(testFormulaRefUpdateInsertRows);
    CPPUNIT_TEST(testFormulaRefUpdateInsertColumns);
    CPPUNIT_TEST(testFormulaRefUpdateMove);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndo);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndo2);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndo3NonShared);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndo3Shared);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndoDependents);
    CPPUNIT_TEST(testFormulaRefUpdateMoveUndo4);
    CPPUNIT_TEST(testFormulaRefUpdateMoveToSheet);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteContent);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteAndShiftLeft);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteAndShiftLeft2);
    CPPUNIT_TEST(testFormulaRefUpdateDeleteAndShiftUp);
    CPPUNIT_TEST(testFormulaRefUpdateName);
    CPPUNIT_TEST(testFormulaRefUpdateNameMove);
    CPPUNIT_TEST(testFormulaRefUpdateNameExpandRef);
    CPPUNIT_TEST(testFormulaRefUpdateNameExpandRef2);
    CPPUNIT_TEST(testFormulaRefUpdateNameDeleteRow);
    CPPUNIT_TEST(testFormulaRefUpdateNameCopySheet);
    CPPUNIT_TEST(testFormulaRefUpdateSheetLocalMove);
    CPPUNIT_TEST(testFormulaRefUpdateNameDelete);
    CPPUNIT_TEST(testFormulaRefUpdateValidity);
    CPPUNIT_TEST(testTokenArrayRefUpdateMove);
    CPPUNIT_TEST(testIntersectionOpExcel);
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
    CPPUNIT_TEST(testFuncLOOKUParrayWithError);
    CPPUNIT_TEST(testTdf141146);
    CPPUNIT_TEST(testFuncVLOOKUP);
    CPPUNIT_TEST(testFuncMATCH);
    CPPUNIT_TEST(testFuncCELL);
    CPPUNIT_TEST(testFuncDATEDIF);
    CPPUNIT_TEST(testFuncINDIRECT);
    CPPUNIT_TEST(testFuncINDIRECT2);
    CPPUNIT_TEST(testFunc_MATCH_INDIRECT);
    CPPUNIT_TEST(testFuncIF);
    CPPUNIT_TEST(testFuncCHOOSE);
    CPPUNIT_TEST(testFuncIFERROR);
    CPPUNIT_TEST(testFuncGETPIVOTDATA);
    CPPUNIT_TEST(testFuncGETPIVOTDATALeafAccess);
    CPPUNIT_TEST(testRefR1C1WholeCol);
    CPPUNIT_TEST(testRefR1C1WholeRow);
    CPPUNIT_TEST(testIterations);
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
    CPPUNIT_TEST(testFuncSUMIFS);
    CPPUNIT_TEST(testFuncRefListArraySUBTOTAL);
    CPPUNIT_TEST(testFuncJumpMatrixArrayIF);
    CPPUNIT_TEST(testFuncJumpMatrixArrayOFFSET);
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
    CPPUNIT_TEST(testPivotTableDPCollection);
    CPPUNIT_TEST(testPivotTableMedianFunc);
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
    CPPUNIT_TEST(testSharedFormulaDeleteTopCell);
    CPPUNIT_TEST(testSharedFormulaCutCopyMoveIntoRef);
    CPPUNIT_TEST(testSharedFormulaCutCopyMoveWithRef);
    CPPUNIT_TEST(testSharedFormulaCutCopyMoveWithinRun);
    CPPUNIT_TEST(testSharedFormulaInsertShift);
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
    CPPUNIT_TEST(testTdf97369);
    CPPUNIT_TEST(testTdf97587);
    CPPUNIT_TEST(testTdf93415);
    CPPUNIT_TEST(testTdf100818);
    CPPUNIT_TEST(testEmptyCalcDocDefaults);
    CPPUNIT_TEST(testPrecisionAsShown);
    CPPUNIT_TEST(testProtectedSheetEditByRow);
    CPPUNIT_TEST(testProtectedSheetEditByColumn);
    CPPUNIT_TEST(testFuncRowsHidden);
    CPPUNIT_TEST(testInsertColCellStoreEventSwap);
    CPPUNIT_TEST(testFormulaAfterDeleteRows);
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
