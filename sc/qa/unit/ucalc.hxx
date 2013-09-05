/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_UCALC_HXX
#define SC_UCALC_HXX

#include "helper/qahelper.hxx"
#include "document.hxx"

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

    static void printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption);
    static void clearRange(ScDocument* pDoc, const ScRange& rRange);
    static void copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc);
    static void pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc);
    static ScUndoPaste* createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pUndoDoc);

    template<size_t _Size>
    static ScRange insertRangeData(ScDocument* pDoc, const ScAddress& rPos, const char* aData[][_Size], size_t nRowCount)
    {
        ScRange aRange(rPos);
        aRange.aEnd.SetCol(rPos.Col()+_Size-1);
        aRange.aEnd.SetRow(rPos.Row()+nRowCount-1);

        clearRange(pDoc, aRange);

        for (size_t i = 0; i < _Size; ++i)
        {
            for (size_t j = 0; j < nRowCount; ++j)
            {
                if (!aData[j][i])
                    continue;

                SCCOL nCol = i + rPos.Col();
                SCROW nRow = j + rPos.Row();
                pDoc->SetString(nCol, nRow, rPos.Tab(), OUString(aData[j][i], strlen(aData[j][i]), RTL_TEXTENCODING_UTF8));
            }
        }

        printRange(pDoc, aRange, "Range data content");
        return aRange;
    }

    Test();
    ~Test();

    ScDocShell& getDocShell();

    virtual void setUp();
    virtual void tearDown();

    /**
     * Basic performance regression test. Pick some actions that *should* take
     * only a fraction of a second to complete, and make sure they stay that
     * way. We set the threshold to 1 second for each action which should be
     * large enough to accommodate slower machines or machines with high load.
     */
    void testPerf();
    void testCollator();
    void testRangeList();
    void testInput();

    void testFormulaHashAndTag();
    void testFormulaRefData();
    void testFormulaCompiler();
    void testFormulaRefUpdate();
    void testFormulaRefUpdateRange();
    void testFormulaRefUpdateSheets();
    void testFormulaRefUpdateMove();
    void testFormulaRefUpdateNamedExpression();
    void testFuncCOLUMN();
    void testFuncROW();
    void testFuncSUM();
    void testFuncPRODUCT();
    void testFuncN();
    void testFuncCOUNTIF();
    void testFuncNUMBERVALUE();
    void testFuncVLOOKUP();
    void testFuncMATCH();
    void testFuncCELL();
    void testFuncDATEDIF();
    void testFuncINDIRECT();
    void testFuncIFERROR();
    void testFuncGETPIVOTDATA();

    void testCopyToDocument();
    /**
     * Make sure the SHEETS function gets properly updated during sheet
     * insertion and removal.
     */
    void testSheetsFunc();
    void testVolatileFunc();

    void testHorizontalIterator();

    /**
     * Basic test for formula dependency tracking.
     */
    void testFormulaDepTracking();

    /**
     * Another test for formula dependency tracking, inspired by fdo#56278.
     */
    void testFormulaDepTracking2();

    /**
     * More direct test for cell broadcaster management, used to track formula
     * dependencies.
     */
    void testCellBroadcaster();

    void testFuncParam();
    void testNamedRange();
    void testCSV();
    void testMatrix();
    void testEnterMixedMatrix();

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
     * refrences.
     */
    void testPivotTableFieldReference();

    /**
     * Test pivot table functionality performed via ScDBDocFunc.
     */
    void testPivotTableDocFunc();

    void testSheetCopy();
    void testSheetMove();
    void testExternalRef();
    void testExternalRefFunctions();
    void testDataArea();
    void testAutofilter();
    void testCopyPaste();
    void testCopyPasteRelativeFormula();
    void testMergedCells();
    void testUpdateReference();
    void testSearchCells();
    void testSharedFormulas();
    void testSharedFormulasRefUpdate();
    void testSharedFormulasCopyPaste();
    void testFormulaPosition();

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

    void testPostIts();

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
    void testCopyPasteFormulas();
    void testCopyPasteFormulasExternalDoc();

    void testFindAreaPosVertical();
    void testFindAreaPosColRight();
    void testSort();
    void testSortWithFormulaRefs();
    void testShiftCells();
    void testDeleteRow();
    void testDeleteCol();
    void testAnchoredRotatedShape();
    void testCellTextWidth();
    void testEditTextIterator();

    void testCondFormatINSDEL();
    void testCondFormatInsertRow();
    void testCondFormatInsertCol();

    CPPUNIT_TEST_SUITE(Test);
#if CALC_TEST_PERF
    CPPUNIT_TEST(testPerf);
#endif
    CPPUNIT_TEST(testCollator);
    CPPUNIT_TEST(testRangeList);
    CPPUNIT_TEST(testInput);
    CPPUNIT_TEST(testFormulaHashAndTag);
    CPPUNIT_TEST(testFormulaRefData);
    CPPUNIT_TEST(testFormulaCompiler);
    CPPUNIT_TEST(testFormulaRefUpdate);
    CPPUNIT_TEST(testFormulaRefUpdateRange);
    CPPUNIT_TEST(testFormulaRefUpdateSheets);
    CPPUNIT_TEST(testFormulaRefUpdateMove);
    CPPUNIT_TEST(testFormulaRefUpdateNamedExpression);
    CPPUNIT_TEST(testFuncCOLUMN);
    CPPUNIT_TEST(testFuncROW);
    CPPUNIT_TEST(testFuncSUM);
    CPPUNIT_TEST(testFuncPRODUCT);
    CPPUNIT_TEST(testFuncN);
    CPPUNIT_TEST(testFuncCOUNTIF);
    CPPUNIT_TEST(testFuncNUMBERVALUE);
    CPPUNIT_TEST(testFuncVLOOKUP);
    CPPUNIT_TEST(testFuncMATCH);
    CPPUNIT_TEST(testFuncCELL);
    CPPUNIT_TEST(testFuncDATEDIF);
    CPPUNIT_TEST(testFuncINDIRECT);
    CPPUNIT_TEST(testFuncIFERROR);
    CPPUNIT_TEST(testFuncGETPIVOTDATA);
    CPPUNIT_TEST(testCopyToDocument);
    CPPUNIT_TEST(testSheetsFunc);
    CPPUNIT_TEST(testVolatileFunc);
    CPPUNIT_TEST(testHorizontalIterator);
    CPPUNIT_TEST(testFormulaDepTracking);
    CPPUNIT_TEST(testFormulaDepTracking2);
    CPPUNIT_TEST(testCellBroadcaster);
    CPPUNIT_TEST(testFuncParam);
    CPPUNIT_TEST(testNamedRange);
    CPPUNIT_TEST(testCSV);
    CPPUNIT_TEST(testMatrix);
    CPPUNIT_TEST(testEnterMixedMatrix);
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
    CPPUNIT_TEST(testSheetCopy);
    CPPUNIT_TEST(testSheetMove);
    CPPUNIT_TEST(testExternalRef);
    CPPUNIT_TEST(testExternalRefFunctions);
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testGraphicsInGroup);
    CPPUNIT_TEST(testGraphicsOnSheetMove);
    CPPUNIT_TEST(testPostIts);
    CPPUNIT_TEST(testStreamValid);
    CPPUNIT_TEST(testFunctionLists);
    CPPUNIT_TEST(testToggleRefFlag);
    CPPUNIT_TEST(testAutofilter);
    CPPUNIT_TEST(testCopyPaste);
    CPPUNIT_TEST(testCopyPasteRelativeFormula);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testUpdateReference);
    CPPUNIT_TEST(testSearchCells);
    CPPUNIT_TEST(testSharedFormulas);
    CPPUNIT_TEST(testSharedFormulasRefUpdate);
    CPPUNIT_TEST(testSharedFormulasCopyPaste);
    CPPUNIT_TEST(testFormulaPosition);
    CPPUNIT_TEST(testJumpToPrecedentsDependents);
    CPPUNIT_TEST(testSetBackgroundColor);
    CPPUNIT_TEST(testRenameTable);
    CPPUNIT_TEST(testAutoFill);
    CPPUNIT_TEST(testCopyPasteFormulas);
    CPPUNIT_TEST(testCopyPasteFormulasExternalDoc);
    CPPUNIT_TEST(testFindAreaPosVertical);
    CPPUNIT_TEST(testFindAreaPosColRight);
    CPPUNIT_TEST(testSort);
    CPPUNIT_TEST(testSortWithFormulaRefs);
    CPPUNIT_TEST(testShiftCells);
    CPPUNIT_TEST(testDeleteRow);
    CPPUNIT_TEST(testDeleteCol);
    CPPUNIT_TEST(testAnchoredRotatedShape);
    CPPUNIT_TEST(testCellTextWidth);
    CPPUNIT_TEST(testEditTextIterator);
    CPPUNIT_TEST(testCondFormatINSDEL);
    CPPUNIT_TEST_SUITE_END();

private:
    TestImpl* m_pImpl;
    ScDocument *m_pDoc;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
