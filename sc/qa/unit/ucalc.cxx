/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/debughelper.hxx"
#include "helper/qahelper.hxx"

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <svl/asiancfg.hxx>
#include <svl/gridprinter.hxx>

#include <scdll.hxx>
#include <simpleformulacalc.hxx>
#include <formulaopt.hxx>
#include <stringutil.hxx>
#include <scmatrix.hxx>
#include <drwlayer.hxx>
#include <scitems.hxx>
#include <reffind.hxx>
#include <markdata.hxx>
#include <clipparam.hxx>
#include <refundo.hxx>
#include <undoblk.hxx>
#include <undotab.hxx>
#include <queryentry.hxx>
#include <postit.hxx>
#include <attrib.hxx>
#include <dbdata.hxx>
#include <reftokenhelper.hxx>
#include <userdat.hxx>
#include <clipcontext.hxx>
#include <refdata.hxx>

#include <docsh.hxx>
#include <docfunc.hxx>
#include <funcdesc.hxx>

#include <calcconfig.hxx>
#include <columniterator.hxx>
#include <types.hxx>
#include <tokenarray.hxx>
#include <scopetools.hxx>
#include <dociter.hxx>
#include <queryparam.hxx>
#include <edittextiterator.hxx>
#include <editutil.hxx>
#include <cellform.hxx>
#include <asciiopt.hxx>
#include <impex.hxx>
#include <docoptio.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <globalnames.hxx>
#include <inputopt.hxx>
#include <columnspanset.hxx>

#include <editable.hxx>
#include <bcaslot.hxx>
#include <sharedformula.hxx>
#include <tabprotection.hxx>
#include <scmod.hxx>

#include <formula/IFunctionDescription.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/lineitem.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <svl/numformat.hxx>
#include <svl/srchitem.hxx>
#include <svl/sharedstringpool.hxx>
#include <tools/UnitConversion.hxx>
#include <unotools/collatorwrapper.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>

#include <iostream>
#include <limits>
#include <memory>
#include <vector>

class ScUndoPaste;
class ScUndoCut;

class Test : public test::BootstrapFixture
{
public:
    Test();

    void checkPrecisionAsShown(OUString& rCode, double fValue, double fExpectedRoundVal);

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
    void testAutofilterOptimizations();
    void testTdf76836();
    void testTdf76441();
    void testTdf142186();
    void testTdf137063();
    void testTdf126342();
    void testAdvancedFilter();
    void testDateFilterContains();
    void testTdf98642();
    void testMergedCells();
    void testUpdateReference();
    void testSearchCells();
    void testFormulaPosition();
    void testFormulaWizardSubformula();
    void testDiagonalBorders();

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
    CPPUNIT_TEST(testAutofilterOptimizations);
    CPPUNIT_TEST(testTdf76836);
    CPPUNIT_TEST(testTdf76441);
    CPPUNIT_TEST(testTdf142186);
    CPPUNIT_TEST(testTdf137063);
    CPPUNIT_TEST(testTdf126342);
    CPPUNIT_TEST(testAdvancedFilter);
    CPPUNIT_TEST(testDateFilterContains);
    CPPUNIT_TEST(testTdf98642);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testUpdateReference);
    CPPUNIT_TEST(testSearchCells);
    CPPUNIT_TEST(testFormulaPosition);
    CPPUNIT_TEST(testFormulaWizardSubformula);
    CPPUNIT_TEST(testDiagonalBorders);
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

Test::Test()
{
}

void Test::getNewDocShell( ScDocShellRef& rDocShellRef )
{
    rDocShellRef = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    rDocShellRef->SetIsInUcalc();
    rDocShellRef->DoInitUnitTest();
}

void Test::closeDocShell( ScDocShellRef& rDocShellRef )
{
    rDocShellRef->DoClose();
    rDocShellRef.clear();
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();

    getNewDocShell(m_xDocShell);
    m_pDoc = &m_xDocShell->GetDocument();
}

void Test::tearDown()
{
    closeDocShell(m_xDocShell);
    BootstrapFixture::tearDown();
}

void Test::testCollator()
{
    sal_Int32 nRes = ScGlobal::GetCollator().compareString("A", "B");
    CPPUNIT_ASSERT_MESSAGE("these strings are supposed to be different!", nRes != 0);
}

void Test::testSharedStringPool()
{
    m_pDoc->InsertTab(0, "foo");

    // Strings that are identical.
    m_pDoc->SetString(ScAddress(0,0,0), "Andy");  // A1
    m_pDoc->SetString(ScAddress(0,1,0), "Andy");  // A2
    m_pDoc->SetString(ScAddress(0,2,0), "Bruce"); // A3
    m_pDoc->SetString(ScAddress(0,3,0), "andy");  // A4
    m_pDoc->SetString(ScAddress(0,4,0), "BRUCE"); // A5

    {
        // These two shared string objects must go out of scope before the purge test.
        svl::SharedString aSS1 = m_pDoc->GetSharedString(ScAddress(0,0,0));
        svl::SharedString aSS2 = m_pDoc->GetSharedString(ScAddress(0,1,0));
        CPPUNIT_ASSERT_MESSAGE("Failed to get a valid shared string.", aSS1.isValid());
        CPPUNIT_ASSERT_MESSAGE("Failed to get a valid shared string.", aSS2.isValid());
        CPPUNIT_ASSERT_EQUAL(aSS1.getData(), aSS2.getData());

        aSS2 = m_pDoc->GetSharedString(ScAddress(0,2,0));
        CPPUNIT_ASSERT_MESSAGE("They must differ", aSS1.getData() != aSS2.getData());

        aSS2 = m_pDoc->GetSharedString(ScAddress(0,3,0));
        CPPUNIT_ASSERT_MESSAGE("They must differ", aSS1.getData() != aSS2.getData());

        aSS2 = m_pDoc->GetSharedString(ScAddress(0,4,0));
        CPPUNIT_ASSERT_MESSAGE("They must differ", aSS1.getData() != aSS2.getData());

        // A3 and A5 should differ but should be equal case-insensitively.
        aSS1 = m_pDoc->GetSharedString(ScAddress(0,2,0));
        aSS2 = m_pDoc->GetSharedString(ScAddress(0,4,0));
        CPPUNIT_ASSERT_MESSAGE("They must differ", aSS1.getData() != aSS2.getData());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("They must be equal when cases are ignored.", aSS1.getDataIgnoreCase(), aSS2.getDataIgnoreCase());

        // A2 and A4 should be equal when ignoring cases.
        aSS1 = m_pDoc->GetSharedString(ScAddress(0,1,0));
        aSS2 = m_pDoc->GetSharedString(ScAddress(0,3,0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("They must be equal when cases are ignored.", aSS1.getDataIgnoreCase(), aSS2.getDataIgnoreCase());
    }

    // Check the string counts after purging. Purging shouldn't remove any strings in this case.
    svl::SharedStringPool& rPool = m_pDoc->GetSharedStringPool();
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A1 and purge again.
    clearRange(m_pDoc, ScAddress(0,0,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A2 and purge again.
    clearRange(m_pDoc, ScAddress(0,1,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A3 and purge again.
    clearRange(m_pDoc, ScAddress(0,2,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A4 and purge again.
    clearRange(m_pDoc, ScAddress(0,3,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPool.getCountIgnoreCase());

    // Clear A5 and the pool should be completely empty.
    clearRange(m_pDoc, ScAddress(0,4,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rPool.getCountIgnoreCase());

    // Now, compare string and edit text cells.
    m_pDoc->SetString(ScAddress(0,0,0), "Andy and Bruce"); // A1
    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetTextCurrentDefaults("Andy and Bruce");

    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = 0;

    {
        // Set 'Andy' bold.
        SfxItemSet aItemSet = rEE.GetEmptyItemSet();
        aSel.nStartPos = 0;
        aSel.nEndPos = 4;
        SvxWeightItem aWeight(WEIGHT_BOLD, EE_CHAR_WEIGHT);
        aItemSet.Put(aWeight);
        rEE.QuickSetAttribs(aItemSet, aSel);
    }

    {
        // Set 'Bruce' italic.
        SfxItemSet aItemSet = rEE.GetEmptyItemSet();
        SvxPostureItem aItalic(ITALIC_NORMAL, EE_CHAR_ITALIC);
        aItemSet.Put(aItalic);
        aSel.nStartPos = 9;
        aSel.nEndPos = 14;
        rEE.QuickSetAttribs(aItemSet, aSel);
    }

    m_pDoc->SetEditText(ScAddress(1,0,0), rEE.CreateTextObject()); // B1

    // These two should be equal.
    svl::SharedString aSS1 = m_pDoc->GetSharedString(ScAddress(0,0,0));
    svl::SharedString aSS2 = m_pDoc->GetSharedString(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("Failed to get a valid string ID.", aSS1.isValid());
    CPPUNIT_ASSERT_MESSAGE("Failed to get a valid string ID.", aSS2.isValid());
    CPPUNIT_ASSERT_EQUAL(aSS1.getData(), aSS2.getData());

    rEE.SetTextCurrentDefaults("ANDY and BRUCE");
    m_pDoc->SetEditText(ScAddress(2,0,0), rEE.CreateTextObject()); // C1
    aSS2 = m_pDoc->GetSharedString(ScAddress(2,0,0));
    CPPUNIT_ASSERT_MESSAGE("Failed to get a valid string ID.", aSS2.isValid());
    CPPUNIT_ASSERT_MESSAGE("These two should be different when cases are considered.", aSS1.getData() != aSS2.getData());

    // But they should be considered equal when cases are ignored.
    aSS1 = m_pDoc->GetSharedString(ScAddress(0,0,0));
    aSS2 = m_pDoc->GetSharedString(ScAddress(2,0,0));
    CPPUNIT_ASSERT_MESSAGE("Failed to get a valid string ID.", aSS1.isValid());
    CPPUNIT_ASSERT_MESSAGE("Failed to get a valid string ID.", aSS2.isValid());
    CPPUNIT_ASSERT_EQUAL(aSS1.getDataIgnoreCase(), aSS2.getDataIgnoreCase());

    m_pDoc->DeleteTab(0);
}

void Test::testSharedStringPoolUndoDoc()
{
    struct
    {
        bool check( const ScDocument& rSrcDoc, ScDocument& rCopyDoc )
        {
            // Copy A1:A4 to the undo document.
            for (SCROW i = 0; i <= 4; ++i)
            {
                ScAddress aPos(0,i,0);
                rCopyDoc.SetString(aPos, rSrcDoc.GetString(aPos));
            }

            // String values in A1:A4 should have identical hash.
            for (SCROW i = 0; i <= 4; ++i)
            {
                ScAddress aPos(0,i,0);
                svl::SharedString aSS1 = rSrcDoc.GetSharedString(aPos);
                svl::SharedString aSS2 = rCopyDoc.GetSharedString(aPos);
                if (aSS1.getDataIgnoreCase() != aSS2.getDataIgnoreCase())
                {
                    cerr << "String hash values are not equal at row " << (i+1)
                        << " for string '" << aSS1.getString() << "'" << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,0,0), "Header");
    m_pDoc->SetString(ScAddress(0,1,0), "A1");
    m_pDoc->SetString(ScAddress(0,2,0), "A2");
    m_pDoc->SetString(ScAddress(0,3,0), "A3");

    ScDocument aUndoDoc(SCDOCMODE_UNDO);
    aUndoDoc.InitUndo(*m_pDoc, 0, 0);

    bool bSuccess = aTest.check(*m_pDoc, aUndoDoc);
    CPPUNIT_ASSERT_MESSAGE("Check failed with undo document.", bSuccess);

    // Test the clip document as well.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, static_cast<SCTAB>(0));

    bSuccess = aTest.check(*m_pDoc, aClipDoc);
    CPPUNIT_ASSERT_MESSAGE("Check failed with clip document.", bSuccess);

    m_pDoc->DeleteTab(0);
}

void Test::testRangeList()
{
    m_pDoc->InsertTab(0, "foo");

    ScRangeList aRL;
    aRL.push_back(ScRange(1,1,0,3,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("List should have one range.", size_t(1), aRL.size());
    const ScRange* p = &aRL[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get the range object.", p);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong range.", ScAddress(1,1,0), p->aStart);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong range.", ScAddress(3,10,0), p->aEnd);

    // TODO: Add more tests here.

    m_pDoc->DeleteTab(0);
}

void Test::testMarkData()
{
    ScMarkData aMarkData(m_pDoc->GetSheetLimits());

    // Empty mark. Nothing is selected.
    std::vector<sc::ColRowSpan> aSpans = aMarkData.GetMarkedRowSpans();
    CPPUNIT_ASSERT_MESSAGE("Span should be empty.", aSpans.empty());
    aSpans = aMarkData.GetMarkedColSpans();
    CPPUNIT_ASSERT_MESSAGE("Span should be empty.", aSpans.empty());

    // Select B3:F7.
    aMarkData.SetMarkArea(ScRange(1,2,0,5,6,0));
    aSpans = aMarkData.GetMarkedRowSpans();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one selected row span.", size_t(1), aSpans.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(2), aSpans[0].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(6), aSpans[0].mnEnd);

    aSpans = aMarkData.GetMarkedColSpans();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one selected column span.", size_t(1), aSpans.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(1), aSpans[0].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(5), aSpans[0].mnEnd);

    // Select A11:B13.
    aMarkData.SetMultiMarkArea(ScRange(0,10,0,1,12,0));
    aSpans = aMarkData.GetMarkedRowSpans();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 2 selected row spans.", size_t(2), aSpans.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(2), aSpans[0].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(6), aSpans[0].mnEnd);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(10), aSpans[1].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(12), aSpans[1].mnEnd);

    aSpans = aMarkData.GetMarkedColSpans();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one selected column span.", size_t(1), aSpans.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(0), aSpans[0].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(5), aSpans[0].mnEnd);

    // Select C8:C10.
    aMarkData.SetMultiMarkArea(ScRange(2,7,0,2,9,0));
    aSpans = aMarkData.GetMarkedRowSpans();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one selected row span.", size_t(1), aSpans.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(2), aSpans[0].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(12), aSpans[0].mnEnd);

    aSpans = aMarkData.GetMarkedColSpans();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one selected column span.", size_t(1), aSpans.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(0), aSpans[0].mnStart);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOLROW>(5), aSpans[0].mnEnd);
}

void Test::testInput()
{

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, "foo"));
    OUString test;

    m_pDoc->SetString(0, 0, 0, "'10.5");
    test = m_pDoc->GetString(0, 0, 0);
    bool bTest = test == "10.5";
    CPPUNIT_ASSERT_MESSAGE("String number should have the first apostrophe stripped.", bTest);
    m_pDoc->SetString(0, 0, 0, "'apple'");
    test = m_pDoc->GetString(0, 0, 0);
    bTest = test == "'apple'";
    CPPUNIT_ASSERT_MESSAGE("Text content should have retained the first apostrophe.", bTest);

    // Customized string handling policy.
    ScSetStringParam aParam;
    aParam.setTextInput();
    m_pDoc->SetString(0, 0, 0, "000123", &aParam);
    test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text content should have been treated as string, not number.", OUString("000123"), test);

    m_pDoc->DeleteTab(0);
}

void Test::testColumnIterator() // tdf#118620
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, "foo"));

    m_pDoc->SetString(0, 0, 0, "'10.5");
    m_pDoc->SetString(0, MAXROW-5, 0, "42.0");
    std::optional<sc::ColumnIterator> it = m_pDoc->GetColumnIterator(0, 0, MAXROW - 10, MAXROW);
    while (it->hasCell())
    {
        it->getCell();
        it->next();
    }

    m_pDoc->DeleteTab(0);
}

void Test::testTdf90698()
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));
    m_pDoc->SetString(ScAddress(0,0,0), "=(1;2)");

    // Without the fix in place, this would have failed with
    // - Expected: =(1;2)
    // - Actual  : =(1~2)
    OUString aFormula = m_pDoc->GetFormula(0,0,0);
    CPPUNIT_ASSERT_EQUAL(OUString("=(1;2)"), aFormula);

    m_pDoc->DeleteTab(0);
}

void Test::testTdf134490()
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));

    m_pDoc->SetString(ScAddress(0,0,0), "--1");
    m_pDoc->SetString(ScAddress(0,1,0), "---1");
    m_pDoc->SetString(ScAddress(0,2,0), "+-1");
    m_pDoc->SetString(ScAddress(0,3,0), "+--1");

    // Without the fix in place, this test would have failed with
    // - Expected: --1
    // - Actual  : -1
    CPPUNIT_ASSERT_EQUAL(OUString("--1"), m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("---1"), m_pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("+-1"), m_pDoc->GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("+--1"), m_pDoc->GetString(ScAddress(0,3,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testTdf135249()
{
    CPPUNIT_ASSERT(m_pDoc->InsertTab (0, "Test"));

    m_pDoc->SetString(ScAddress(0,0,0), "1:60");
    m_pDoc->SetString(ScAddress(0,1,0), "1:123");
    m_pDoc->SetString(ScAddress(0,2,0), "1:1:123");
    m_pDoc->SetString(ScAddress(0,3,0), "0:123");
    m_pDoc->SetString(ScAddress(0,4,0), "0:0:123");
    m_pDoc->SetString(ScAddress(0,5,0), "0:123:59");

    // These are not valid duration inputs
    CPPUNIT_ASSERT_EQUAL(OUString("1:60"), m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1:123"), m_pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1:1:123"), m_pDoc->GetString(ScAddress(0,2,0)));

    // These are valid duration inputs
    // Without the fix in place, this test would have failed with
    // - Expected: 02:03:00 AM
    // - Actual  : 0:123
    CPPUNIT_ASSERT_EQUAL(OUString("02:03:00 AM"), m_pDoc->GetString(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("12:02:03 AM"), m_pDoc->GetString(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("02:03:59 AM"), m_pDoc->GetString(ScAddress(0,5,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testDocStatistics()
{
    SCTAB nStartTabs = m_pDoc->GetTableCount();
    m_pDoc->InsertTab(0, "Sheet1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to increment sheet count.",
                               static_cast<SCTAB>(nStartTabs+1), m_pDoc->GetTableCount());
    m_pDoc->InsertTab(1, "Sheet2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to increment sheet count.",
                               static_cast<SCTAB>(nStartTabs+2), m_pDoc->GetTableCount());

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(0), m_pDoc->GetCellCount());
    m_pDoc->SetValue(ScAddress(0,0,0), 2.0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), m_pDoc->GetCellCount());
    m_pDoc->SetValue(ScAddress(2,2,0), 2.5);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), m_pDoc->GetCellCount());
    m_pDoc->SetString(ScAddress(1,1,1), "Test");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(3), m_pDoc->GetCellCount());

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(0), m_pDoc->GetFormulaGroupCount());
    m_pDoc->SetString(ScAddress(3,0,1), "=A1");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), m_pDoc->GetFormulaGroupCount());
    m_pDoc->SetString(ScAddress(3,1,1), "=A2");
    m_pDoc->SetString(ScAddress(3,2,1), "=A3");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(1), m_pDoc->GetFormulaGroupCount());
    m_pDoc->SetString(ScAddress(3,3,1), "=A5");
    m_pDoc->SetString(ScAddress(3,4,1), "=A6");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(2), m_pDoc->GetFormulaGroupCount());
    m_pDoc->SetString(ScAddress(3,1,1), "=A3");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(4), m_pDoc->GetFormulaGroupCount());

    m_pDoc->DeleteTab(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to decrement sheet count.",
                               static_cast<SCTAB>(nStartTabs+1), m_pDoc->GetTableCount());
    m_pDoc->DeleteTab(0); // This may fail in case there is only one sheet in the document.
}

void Test::testRowForHeight()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->SetRowHeightRange( 0,  9, 0, 100);
    m_pDoc->SetRowHeightRange(10, 19, 0, 200);
    m_pDoc->SetRowHeightRange(20, 29, 0, 300);

    // Hide some rows.
    m_pDoc->SetRowHidden(3,  5, 0, true);
    m_pDoc->SetRowHidden(8, 12, 0, true);

    struct Check
    {
        sal_uLong nHeight;
        SCROW nRow;
    };

    std::vector<Check> aChecks = {
        {   1, 1 },
        {  99, 1 },
        { 120, 2 },
        { 330, 7 },
        { 420, 13 },
        { 780, 15 },
        { 1860, 20 },
        { 4020, 28 },
    };

    for (const Check& rCheck : aChecks)
    {
        SCROW nRow = m_pDoc->GetRowForHeight(0, rCheck.nHeight);
        CPPUNIT_ASSERT_EQUAL(rCheck.nRow, nRow);
    }
}

void Test::testDataEntries()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,5,0), "Andy");
    m_pDoc->SetString(ScAddress(0,6,0), "Bruce");
    m_pDoc->SetString(ScAddress(0,7,0), "Charlie");
    m_pDoc->SetString(ScAddress(0,10,0), "Andy");

    std::vector<ScTypedStrData> aEntries;
    m_pDoc->GetDataEntries(0, 0, 0, aEntries); // Try at the very top.

    // Entries are supposed to be sorted in ascending order, and are all unique.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aEntries.size());
    std::vector<ScTypedStrData>::const_iterator it = aEntries.begin();
    CPPUNIT_ASSERT_EQUAL(OUString("Andy"), it->GetString());
    ++it;
    CPPUNIT_ASSERT_EQUAL(OUString("Bruce"), it->GetString());
    ++it;
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie"), it->GetString());
    ++it;
    CPPUNIT_ASSERT_MESSAGE("The entries should have ended here.", bool(it == aEntries.end()));

    aEntries.clear();
    m_pDoc->GetDataEntries(0, MAXROW, 0, aEntries); // Try at the very bottom.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aEntries.size());

    // Make sure we get the same set of suggestions.
    it = aEntries.begin();
    CPPUNIT_ASSERT_EQUAL(OUString("Andy"), it->GetString());
    ++it;
    CPPUNIT_ASSERT_EQUAL(OUString("Bruce"), it->GetString());
    ++it;
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie"), it->GetString());
    ++it;
    CPPUNIT_ASSERT_MESSAGE("The entries should have ended here.", bool(it == aEntries.end()));

    m_pDoc->DeleteTab(0);
}

void Test::testSelectionFunction()
{
    m_pDoc->InsertTab(0, "Test");

    // Insert values into B2:B4.
    m_pDoc->SetString(ScAddress(1,1,0), "=1"); // formula
    m_pDoc->SetValue(ScAddress(1,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,3,0), 3.0);

    // Insert strings into B5:B8.
    m_pDoc->SetString(ScAddress(1,4,0), "A");
    m_pDoc->SetString(ScAddress(1,5,0), "B");
    m_pDoc->SetString(ScAddress(1,6,0), "=\"C\""); // formula
    m_pDoc->SetString(ScAddress(1,7,0), "D");

    // Insert values into D2:D4.
    m_pDoc->SetValue(ScAddress(3,1,0), 4.0);
    m_pDoc->SetValue(ScAddress(3,2,0), 5.0);
    m_pDoc->SetValue(ScAddress(3,3,0), 6.0);

    // Insert edit text into D5.
    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetTextCurrentDefaults("Rich Text");
    m_pDoc->SetEditText(ScAddress(3,4,0), rEE.CreateTextObject());

    // Insert Another string into D6.
    m_pDoc->SetString(ScAddress(3,5,0), "E");

    // Select B2:B8 & D2:D8 disjoint region.
    ScRangeList aRanges;
    aRanges.push_back(ScRange(1,1,0,1,7,0)); // B2:B8
    aRanges.push_back(ScRange(3,1,0,3,7,0)); // D2:D8
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.MarkFromRangeList(aRanges, true);

    struct Check
    {
        ScSubTotalFunc meFunc;
        double mfExpected;
    };

    {
        static const Check aChecks[] =
        {
            { SUBTOTAL_FUNC_AVE,              3.5 },
            { SUBTOTAL_FUNC_CNT2,            12.0 },
            { SUBTOTAL_FUNC_CNT,              6.0 },
            { SUBTOTAL_FUNC_MAX,              6.0 },
            { SUBTOTAL_FUNC_MIN,              1.0 },
            { SUBTOTAL_FUNC_SUM,             21.0 },
            { SUBTOTAL_FUNC_SELECTION_COUNT, 14.0 }
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            double fRes = 0.0;
            bool bRes = m_pDoc->GetSelectionFunction(aChecks[i].meFunc, ScAddress(), aMark, fRes);
            CPPUNIT_ASSERT_MESSAGE("Failed to fetch selection function result.", bRes);
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mfExpected, fRes);
        }
    }

    // Hide rows 4 and 6 and check the results again.

    m_pDoc->SetRowHidden(3, 3, 0, true);
    m_pDoc->SetRowHidden(5, 5, 0, true);
    CPPUNIT_ASSERT_MESSAGE("This row should be hidden.", m_pDoc->RowHidden(3, 0));
    CPPUNIT_ASSERT_MESSAGE("This row should be hidden.", m_pDoc->RowHidden(5, 0));

    {
        static const Check aChecks[] =
        {
            { SUBTOTAL_FUNC_AVE,              3.0 },
            { SUBTOTAL_FUNC_CNT2,             8.0 },
            { SUBTOTAL_FUNC_CNT,              4.0 },
            { SUBTOTAL_FUNC_MAX,              5.0 },
            { SUBTOTAL_FUNC_MIN,              1.0 },
            { SUBTOTAL_FUNC_SUM,             12.0 },
            { SUBTOTAL_FUNC_SELECTION_COUNT, 10.0 }
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            double fRes = 0.0;
            bool bRes = m_pDoc->GetSelectionFunction(aChecks[i].meFunc, ScAddress(), aMark, fRes);
            CPPUNIT_ASSERT_MESSAGE("Failed to fetch selection function result.", bRes);
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mfExpected, fRes);
        }
    }

    // Make sure that when no selection is present, use the current cursor position.
    ScMarkData aEmpty(m_pDoc->GetSheetLimits());

    {
        // D3 (numeric cell containing 5.)
        ScAddress aPos(3, 2, 0);

        static const Check aChecks[] =
        {
            { SUBTOTAL_FUNC_AVE,             5.0 },
            { SUBTOTAL_FUNC_CNT2,            1.0 },
            { SUBTOTAL_FUNC_CNT,             1.0 },
            { SUBTOTAL_FUNC_MAX,             5.0 },
            { SUBTOTAL_FUNC_MIN,             5.0 },
            { SUBTOTAL_FUNC_SUM,             5.0 },
            { SUBTOTAL_FUNC_SELECTION_COUNT, 1.0 }
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            double fRes = 0.0;
            bool bRes = m_pDoc->GetSelectionFunction(aChecks[i].meFunc, aPos, aEmpty, fRes);
            CPPUNIT_ASSERT_MESSAGE("Failed to fetch selection function result.", bRes);
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mfExpected, fRes);
        }
    }

    {
        // B7 (string formula cell containing ="C".)
        ScAddress aPos(1, 6, 0);

        static const Check aChecks[] =
        {
            { SUBTOTAL_FUNC_CNT2,            1.0 },
            { SUBTOTAL_FUNC_SELECTION_COUNT, 1.0 }
        };

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            double fRes = 0.0;
            bool bRes = m_pDoc->GetSelectionFunction(aChecks[i].meFunc, aPos, aEmpty, fRes);
            CPPUNIT_ASSERT_MESSAGE("Failed to fetch selection function result.", bRes);
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mfExpected, fRes);
        }
    }

    // Calculate function across selected sheets.
    clearSheet(m_pDoc, 0);
    m_pDoc->InsertTab(1, "Test2");
    m_pDoc->InsertTab(2, "Test3");

    // Set values at B2 and C3 on each sheet.
    m_pDoc->SetValue(ScAddress(1,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(2,2,0), 2.0);
    m_pDoc->SetValue(ScAddress(1,1,1), 4.0);
    m_pDoc->SetValue(ScAddress(2,2,1), 8.0);
    m_pDoc->SetValue(ScAddress(1,1,2), 16.0);
    m_pDoc->SetValue(ScAddress(2,2,2), 32.0);

    // Mark B2 and C3 on first sheet.
    aRanges.RemoveAll();
    aRanges.push_back(ScRange(1,1,0)); // B2
    aRanges.push_back(ScRange(2,2,0)); // C3
    aMark.MarkFromRangeList(aRanges, true);
    // Additionally select third sheet.
    aMark.SelectTable(2, true);

    {
        double fRes = 0.0;
        bool bRes = m_pDoc->GetSelectionFunction( SUBTOTAL_FUNC_SUM, ScAddress(), aMark, fRes);
        CPPUNIT_ASSERT_MESSAGE("Failed to fetch selection function result.", bRes);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("1+2+16+32=", 51.0, fRes);
    }

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testMarkedCellIteration()
{
    m_pDoc->InsertTab(0, "Test");

    // Insert cells to A1, A5, B2 and C3.
    m_pDoc->SetString(ScAddress(0,0,0), "California");
    m_pDoc->SetValue(ScAddress(0,4,0), 1.2);
    m_pDoc->SetEditText(ScAddress(1,1,0), "Boston");
    m_pDoc->SetFormula(ScAddress(2,2,0), "=SUM(1,2,3)", m_pDoc->GetGrammar());

    // Select A1:C5.
    ScMarkData aMarkData(m_pDoc->GetSheetLimits());
    aMarkData.SetMarkArea(ScRange(0,0,0,2,4,0));
    aMarkData.MarkToMulti(); // TODO : we shouldn't have to do this.

    struct Check
    {
        SCCOL mnCol;
        SCROW mnRow;
    };

    const std::vector<Check> aChecks = {
        { 0, 0 }, // A1
        { 0, 4 }, // A5
        { 1, 1 }, // B2
        { 2, 2 }, // C3
    };

    SCROW nRow = -1; // Start from the imaginary row before A1.
    SCCOL nCol = 0;

    for (const Check& rCheck : aChecks)
    {
        bool bFound = m_pDoc->GetNextMarkedCell(nCol, nRow, 0, aMarkData);
        if (!bFound)
        {
            std::ostringstream os;
            os << ScAddress(rCheck.mnCol, rCheck.mnRow, 0).GetColRowString() << " was expected, but not found.";
            CPPUNIT_FAIL(os.str());
        }

        CPPUNIT_ASSERT_EQUAL(rCheck.mnRow, nRow);
        CPPUNIT_ASSERT_EQUAL(rCheck.mnCol, nCol);
    }

    // No more marked cells on this sheet.
    bool bFound = m_pDoc->GetNextMarkedCell(nCol, nRow, 0, aMarkData);
    CPPUNIT_ASSERT(!bFound);

    m_pDoc->DeleteTab(0);
}

void Test::testCopyToDocument()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "src"));

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    m_pDoc->SetString(0, 0, 0, "Header");
    m_pDoc->SetString(0, 1, 0, "1");
    m_pDoc->SetString(0, 2, 0, "2");
    m_pDoc->SetString(0, 3, 0, "3");
    m_pDoc->SetString(0, 4, 0, "=4/2");
    m_pDoc->CalcAll();

    //note on A1
    ScAddress aAdrA1 (0, 0, 0); // numerical cell content
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aAdrA1);
    pNote->SetText(aAdrA1, "Hello world in A1");

    // Copy statically to another document.

    ScDocShellRef xDocSh2;
    getNewDocShell(xDocSh2);
    ScDocument* pDestDoc = &xDocSh2->GetDocument();
    pDestDoc->InsertTab(0, "src");
    pDestDoc->InitDrawLayer(xDocSh2.get());     // for note caption objects

    m_pDoc->CopyStaticToDocument(ScRange(0,1,0,0,3,0), 0, *pDestDoc); // Copy A2:A4
    m_pDoc->CopyStaticToDocument(ScAddress(0,0,0), 0,     *pDestDoc); // Copy A1
    m_pDoc->CopyStaticToDocument(ScRange(0,4,0,0,7,0), 0, *pDestDoc); // Copy A5:A8

    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,0,0), pDestDoc->GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,1,0), pDestDoc->GetString(0,1,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,2,0), pDestDoc->GetString(0,2,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,3,0), pDestDoc->GetString(0,3,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,4,0), pDestDoc->GetString(0,4,0));

    // verify note
    CPPUNIT_ASSERT_MESSAGE("There should be a note in A1 destDocument", pDestDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The notes content should be the same on both documents",
            m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(), pDestDoc->GetNote(ScAddress(0, 0, 0))->GetText());

    pDestDoc->DeleteTab(0);
    closeDocShell(xDocSh2);

    m_pDoc->DeleteTab(0);
}

namespace {

struct HoriIterCheck
{
    SCCOL nCol;
    SCROW nRow;
    const char* pVal;
};

bool checkHorizontalIterator(ScDocument& rDoc, const std::vector<std::vector<const char*>>& rData, const HoriIterCheck* pChecks, size_t nCheckCount)
{
    ScAddress aPos(0,0,0);
    insertRangeData(&rDoc, aPos, rData);
    ScHorizontalCellIterator aIter(rDoc, 0, 0, 0, 1, rData.size() - 1);

    SCCOL nCol;
    SCROW nRow;
    size_t i = 0;
    for (ScRefCellValue* pCell = aIter.GetNext(nCol, nRow); pCell; pCell = aIter.GetNext(nCol, nRow), ++i)
    {
        if (i >= nCheckCount)
        {
            cerr << "hit invalid check " << i << " of " << nCheckCount << endl;
            CPPUNIT_FAIL("Iterator claims there is more data than there should be.");
            return false;
        }

        if (pChecks[i].nCol != nCol)
        {
            cerr << "Column mismatch " << pChecks[i].nCol << " vs. " << nCol << endl;
            return false;
        }

        if (pChecks[i].nRow != nRow)
        {
            cerr << "Row mismatch " << pChecks[i].nRow << " vs. " << nRow << endl;
            return false;
        }

        if (OUString::createFromAscii(pChecks[i].pVal) != pCell->getString(&rDoc))
        {
            cerr << "String mismatch " << pChecks[i].pVal << " vs. " <<
                pCell->getString(&rDoc) << endl;
            return false;
        }
    }

    return true;
}

}

void Test::testHorizontalIterator()
{
    m_pDoc->InsertTab(0, "test");

    {
        // Raw data - mixed types
        std::vector<std::vector<const char*>> aData = {
            { "A", "B" },
            { "C", "1" },
            { "D", "2" },
            { "E", "3" }
        };

        static const HoriIterCheck aChecks[] = {
            { 0, 0, "A" },
            { 1, 0, "B" },
            { 0, 1, "C" },
            { 1, 1, "1" },
            { 0, 2, "D" },
            { 1, 2, "2" },
            { 0, 3, "E" },
            { 1, 3, "3" },
        };

        bool bRes = checkHorizontalIterator(
            *m_pDoc, aData, aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test mixed.");
    }

    {
        // Raw data - 'hole' data
        std::vector<std::vector<const char*>> aData = {
            { "A", "B" },
            { "C",  nullptr  },
            { "D", "E" },
        };

        static const HoriIterCheck aChecks[] = {
            { 0, 0, "A" },
            { 1, 0, "B" },
            { 0, 1, "C" },
            { 0, 2, "D" },
            { 1, 2, "E" },
        };

        bool bRes = checkHorizontalIterator(
            *m_pDoc, aData, aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test hole.");
    }

    {
        // Very holy data
        std::vector<std::vector<const char*>> aData = {
            {  nullptr,  "A" },
            {  nullptr,   nullptr  },
            {  nullptr,  "1" },
            { "B",  nullptr  },
            { "C", "2" },
            { "D", "3" },
            { "E",  nullptr  },
            {  nullptr,  "G" },
            {  nullptr,   nullptr  },
        };

        static const HoriIterCheck aChecks[] = {
            { 1, 0, "A" },
            { 1, 2, "1" },
            { 0, 3, "B" },
            { 0, 4, "C" },
            { 1, 4, "2" },
            { 0, 5, "D" },
            { 1, 5, "3" },
            { 0, 6, "E" },
            { 1, 7, "G" },
        };

        bool bRes = checkHorizontalIterator(
            *m_pDoc, aData, aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test holy.");
    }

    {
        // Degenerate case
        std::vector<std::vector<const char*>> aData = {
            {  nullptr,   nullptr },
            {  nullptr,   nullptr },
            {  nullptr,   nullptr },
        };

        bool bRes = checkHorizontalIterator(
            *m_pDoc, aData, nullptr, 0);

        if (!bRes)
            CPPUNIT_FAIL("Failed on test degenerate.");
    }

    {
        // Data at end
        std::vector<std::vector<const char*>> aData = {
            {  nullptr,   nullptr },
            {  nullptr,   nullptr },
            {  nullptr,  "A" },
        };

        static const HoriIterCheck aChecks[] = {
            { 1, 2, "A" },
        };

        bool bRes = checkHorizontalIterator(
            *m_pDoc, aData, aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test at end.");
    }

    {
        // Data in middle
        std::vector<std::vector<const char*>> aData = {
            {  nullptr,   nullptr  },
            {  nullptr,   nullptr  },
            {  nullptr,  "A" },
            {  nullptr,  "1" },
            {  nullptr,   nullptr  },
        };

        static const HoriIterCheck aChecks[] = {
            { 1, 2, "A" },
            { 1, 3, "1" },
        };

        bool bRes = checkHorizontalIterator(
            *m_pDoc, aData, aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test in middle.");
    }

    m_pDoc->DeleteTab(0);
}

void Test::testValueIterator()
{
    m_pDoc->InsertTab(0, "Test");

    // Turn on "precision as shown" option.
    ScDocOptions aOpt = m_pDoc->GetDocOptions();
    aOpt.SetCalcAsShown(true);
    m_pDoc->SetDocOptions(aOpt);

    // Purely horizontal data layout with numeric data.
    for (SCCOL i = 1; i <= 3; ++i)
        m_pDoc->SetValue(ScAddress(i,2,0), i);

    {
        const double aChecks[] = { 1.0, 2.0, 3.0 };
        size_t const nCheckLen = SAL_N_ELEMENTS(aChecks);
        ScValueIterator aIter(*m_pDoc, ScRange(1,2,0,3,2,0));
        bool bHas = false;
        size_t nCheckPos = 0;
        double fVal;
        FormulaError nErr;
        for (bHas = aIter.GetFirst(fVal, nErr); bHas; bHas = aIter.GetNext(fVal, nErr), ++nCheckPos)
        {
            CPPUNIT_ASSERT_MESSAGE("Iteration longer than expected.", nCheckPos < nCheckLen);
            CPPUNIT_ASSERT_EQUAL(aChecks[nCheckPos], fVal);
            CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(nErr));
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testHorizontalAttrIterator()
{
    m_pDoc->InsertTab(0, "Test");

    // Set the background color of B2:C3,D2,E3,C4:D4,B5:D5 to blue
    ScPatternAttr aCellBackColor(m_pDoc->GetPool());
    aCellBackColor.GetItemSet().Put(SvxBrushItem(COL_BLUE, ATTR_BACKGROUND));
    m_pDoc->ApplyPatternAreaTab(1, 1, 2, 2, 0, aCellBackColor);
    m_pDoc->ApplyPatternAreaTab(3, 1, 3, 1, 0, aCellBackColor);
    m_pDoc->ApplyPatternAreaTab(4, 2, 4, 2, 0, aCellBackColor);
    m_pDoc->ApplyPatternAreaTab(2, 3, 3, 3, 0, aCellBackColor);
    m_pDoc->ApplyPatternAreaTab(1, 4, 4, 4, 0, aCellBackColor);

    // some numeric data
    for (SCCOL i = 1; i <= 4; ++i)
        for (SCROW j = 1; j <= 4; ++j)
            m_pDoc->SetValue(ScAddress(i,j,0), i*10+j);

    {
        const int aChecks[][3] = { {1, 3, 1}, {1, 2, 2}, {4, 4, 2}, {2, 3, 3}, {1, 4, 4} };
        const size_t nCheckLen = SAL_N_ELEMENTS(aChecks);

        ScHorizontalAttrIterator aIter(*m_pDoc, 0, 0, 0, 5, 5);
        SCCOL nCol1, nCol2;
        SCROW nRow;
        size_t nCheckPos = 0;
        for (const ScPatternAttr* pAttr = aIter.GetNext(nCol1, nCol2, nRow); pAttr; pAttr = aIter.GetNext(nCol1, nCol2, nRow), ++nCheckPos)
        {
              CPPUNIT_ASSERT_MESSAGE("Iteration longer than expected.", nCheckPos < nCheckLen);
              CPPUNIT_ASSERT_EQUAL(aChecks[nCheckPos][0], static_cast<int>(nCol1));
              CPPUNIT_ASSERT_EQUAL(aChecks[nCheckPos][1], static_cast<int>(nCol2));
              CPPUNIT_ASSERT_EQUAL(aChecks[nCheckPos][2], static_cast<int>(nRow));
        }
    }

    m_pDoc->DeleteTab(0);
}

namespace {

bool broadcasterShifted(const ScDocument& rDoc, const ScAddress& rFrom, const ScAddress& rTo)
{
    const SvtBroadcaster* pBC = rDoc.GetBroadcaster(rFrom);
    if (pBC)
    {
        cerr << "Broadcaster shouldn't be here." << endl;
        return false;
    }

    pBC = rDoc.GetBroadcaster(rTo);
    if (!pBC)
    {
        cerr << "Broadcaster should be here." << endl;
        return false;
    }
    return true;
}

formula::FormulaToken* getSingleRefToken(ScDocument& rDoc, const ScAddress& rPos)
{
    ScFormulaCell* pFC = rDoc.GetFormulaCell(rPos);
    if (!pFC)
    {
        cerr << "Formula cell expected, but not found." << endl;
        return nullptr;
    }

    ScTokenArray* pTokens = pFC->GetCode();
    if (!pTokens)
    {
        cerr << "Token array is not present." << endl;
        return nullptr;
    }

    formula::FormulaToken* pToken = pTokens->FirstToken();
    if (!pToken || pToken->GetType() != formula::svSingleRef)
    {
        cerr << "Not a single reference token." << endl;
        return nullptr;
    }

    return pToken;
}

bool checkRelativeRefToken(ScDocument& rDoc, const ScAddress& rPos, SCCOL nRelCol, SCROW nRelRow)
{
    formula::FormulaToken* pToken = getSingleRefToken(rDoc, rPos);
    if (!pToken)
        return false;

    ScSingleRefData& rRef = *pToken->GetSingleRef();
    if (!rRef.IsColRel() || rRef.Col() != nRelCol)
    {
        cerr << "Unexpected relative column address." << endl;
        return false;
    }

    if (!rRef.IsRowRel() || rRef.Row() != nRelRow)
    {
        cerr << "Unexpected relative row address." << endl;
        return false;
    }

    return true;
}

bool checkDeletedRefToken(ScDocument& rDoc, const ScAddress& rPos)
{
    formula::FormulaToken* pToken = getSingleRefToken(rDoc, rPos);
    if (!pToken)
        return false;

    ScSingleRefData& rRef = *pToken->GetSingleRef();
    if (!rRef.IsDeleted())
    {
        cerr << "Deleted reference is expected, but it's still a valid reference." << endl;
        return false;
    }

    return true;
}

}

void Test::testCellBroadcaster()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.
    m_pDoc->SetString(ScAddress(1,0,0), "=A1"); // B1 depends on A1.
    double val = m_pDoc->GetValue(ScAddress(1,0,0)); // A1 is empty, so the result should be 0.
    CPPUNIT_ASSERT_EQUAL(0.0, val);

    const SvtBroadcaster* pBC = m_pDoc->GetBroadcaster(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("Cell A1 should have a broadcaster.", pBC);

    // Change the value of A1 and make sure that B1 follows.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.23);
    val = m_pDoc->GetValue(ScAddress(1,0,0));
    CPPUNIT_ASSERT_EQUAL(1.23, val);

    // Move column A down 5 cells. Make sure B1 now references A6, not A1.
    m_pDoc->InsertRow(0, 0, 0, 0, 0, 5);
    CPPUNIT_ASSERT_MESSAGE("Relative reference check failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,0,0), -1, 5));

    // Make sure the broadcaster has also moved.
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,0,0), ScAddress(0,5,0)));

    // Set new value to A6 and make sure B1 gets updated.
    m_pDoc->SetValue(ScAddress(0,5,0), 45.6);
    val = m_pDoc->GetValue(ScAddress(1,0,0));
    CPPUNIT_ASSERT_EQUAL(45.6, val);

    // Move column A up 3 cells, and make sure B1 now references A3, not A6.
    m_pDoc->DeleteRow(0, 0, 0, 0, 0, 3);
    CPPUNIT_ASSERT_MESSAGE("Relative reference check failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,0,0), -1, 2));

    // The broadcaster should also have been relocated from A6 to A3.
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,5,0), ScAddress(0,2,0)));

    // Insert cells over A1:A10 and shift cells to right.
    m_pDoc->InsertCol(ScRange(0, 0, 0, 0, 10, 0));
    CPPUNIT_ASSERT_MESSAGE("Relative reference check failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(2,0,0), -1, 2));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,2,0), ScAddress(1,2,0)));

    // Delete formula in C2, which should remove the broadcaster in B3.
    pBC = m_pDoc->GetBroadcaster(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster in B3 should still exist.", pBC);
    clearRange(m_pDoc, ScAddress(2,0,0));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(2,0,0))); // C2 should be empty.
    pBC = m_pDoc->GetBroadcaster(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster in B3 should have been removed.", !pBC);

    // Clear everything and start over.
    clearRange(m_pDoc, ScRange(0,0,0,10,100,0));

    m_pDoc->SetString(ScAddress(1,0,0), "=A1"); // B1 depends on A1.
    pBC = m_pDoc->GetBroadcaster(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist in A1.", pBC);

    // While column A is still empty, move column A down 2 cells. This should
    // move the broadcaster from A1 to A3.
    m_pDoc->InsertRow(0, 0, 0, 0, 0, 2);
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,0,0), ScAddress(0,2,0)));

    // Move it back while column A is still empty.
    m_pDoc->DeleteRow(0, 0, 0, 0, 0, 2);
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,2,0), ScAddress(0,0,0)));

    // Clear everything again
    clearRange(m_pDoc, ScRange(0,0,0,10,100,0));

    // B1:B3 depends on A1:A3
    m_pDoc->SetString(ScAddress(1,0,0), "=A1");
    m_pDoc->SetString(ScAddress(1,1,0), "=A2");
    m_pDoc->SetString(ScAddress(1,2,0), "=A3");
    CPPUNIT_ASSERT_MESSAGE("Relative reference check in B1 failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,0,0), -1, 0));
    CPPUNIT_ASSERT_MESSAGE("Relative reference check in B2 failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,1,0), -1, 0));
    CPPUNIT_ASSERT_MESSAGE("Relative reference check in B3 failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,2,0), -1, 0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist in A1.", m_pDoc->GetBroadcaster(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist in A2.", m_pDoc->GetBroadcaster(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist in A3.", m_pDoc->GetBroadcaster(ScAddress(0,2,0)));

    // Insert Rows at row 2, down 5 rows.
    m_pDoc->InsertRow(0, 0, 0, 0, 1, 5);
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist in A1.", m_pDoc->GetBroadcaster(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_MESSAGE("Relative reference check in B1 failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,0,0), -1, 0));

    // Broadcasters in A2 and A3 should shift down by 5 rows.
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,1,0), ScAddress(0,6,0)));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster relocation failed.",
                           broadcasterShifted(*m_pDoc, ScAddress(0,2,0), ScAddress(0,7,0)));

    // B2 and B3 should reference shifted cells.
    CPPUNIT_ASSERT_MESSAGE("Relative reference check in B2 failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,1,0), -1, 5));
    CPPUNIT_ASSERT_MESSAGE("Relative reference check in B2 failed.",
                           checkRelativeRefToken(*m_pDoc, ScAddress(1,2,0), -1, 5));

    // Delete cells with broadcasters.
    m_pDoc->DeleteRow(0, 0, 0, 0, 4, 6);
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should NOT exist in A7.", !m_pDoc->GetBroadcaster(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should NOT exist in A8.", !m_pDoc->GetBroadcaster(ScAddress(0,7,0)));

    // References in B2 and B3 should be invalid.
    CPPUNIT_ASSERT_MESSAGE("Deleted reference check in B2 failed.",
                           checkDeletedRefToken(*m_pDoc, ScAddress(1,1,0)));
    CPPUNIT_ASSERT_MESSAGE("Deleted reference check in B3 failed.",
                           checkDeletedRefToken(*m_pDoc, ScAddress(1,2,0)));

    // Clear everything again
    clearRange(m_pDoc, ScRange(0,0,0,10,100,0));

    {
        // Switch to R1C1 to make it easier to input relative references in multiple cells.
        FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

        // Have B1:B20 reference A1:A20.
        val = 0.0;
        for (SCROW i = 0; i < 20; ++i)
        {
            m_pDoc->SetValue(ScAddress(0,i,0), val++);
            m_pDoc->SetString(ScAddress(1,i,0), "=RC[-1]");
        }
    }

    // Ensure that the formula cells show correct values, and the referenced
    // cells have broadcasters.
    val = 0.0;
    for (SCROW i = 0; i < 20; ++i, ++val)
    {
        CPPUNIT_ASSERT_EQUAL(val, m_pDoc->GetValue(ScAddress(1,i,0)));
        pBC = m_pDoc->GetBroadcaster(ScAddress(0,i,0));
        CPPUNIT_ASSERT_MESSAGE("Broadcast should exist here.", pBC);
    }

    // Delete formula cells in B2:B19.
    clearRange(m_pDoc, ScRange(1,1,0,1,18,0));
    // Ensure that A2:A19 no longer have broadcasters, but A1 and A20 still do.
    CPPUNIT_ASSERT_MESSAGE("A1 should still have broadcaster.", m_pDoc->GetBroadcaster(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_MESSAGE("A20 should still have broadcaster.", m_pDoc->GetBroadcaster(ScAddress(0,19,0)));
    for (SCROW i = 1; i <= 18; ++i)
    {
        pBC = m_pDoc->GetBroadcaster(ScAddress(0,i,0));
        CPPUNIT_ASSERT_MESSAGE("Broadcaster should have been deleted.", !pBC);
    }

    // Clear everything again
    clearRange(m_pDoc, ScRange(0,0,0,10,100,0));

    m_pDoc->SetValue(ScAddress(0,0,0), 2.0);
    m_pDoc->SetString(ScAddress(1,0,0), "=A1");
    m_pDoc->SetString(ScAddress(2,0,0), "=B1");
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,0,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,0,0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(2,0,0));

    pBC = m_pDoc->GetBroadcaster(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist here.", pBC);
    pBC = m_pDoc->GetBroadcaster(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist here.", pBC);

    // Change the value of A1 and make sure everyone follows suit.
    m_pDoc->SetValue(ScAddress(0,0,0), 3.5);
    CPPUNIT_ASSERT_EQUAL(3.5, m_pDoc->GetValue(0,0,0));
    CPPUNIT_ASSERT_EQUAL(3.5, m_pDoc->GetValue(1,0,0));
    CPPUNIT_ASSERT_EQUAL(3.5, m_pDoc->GetValue(2,0,0));

    // Insert a column at column B.
    m_pDoc->InsertCol(ScRange(1,0,0,1,MAXROW,0));
    pBC = m_pDoc->GetBroadcaster(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist here.", pBC);
    pBC = m_pDoc->GetBroadcaster(ScAddress(2,0,0));
    CPPUNIT_ASSERT_MESSAGE("Broadcaster should exist here.", pBC);

    // Change the value of A1 again.
    m_pDoc->SetValue(ScAddress(0,0,0), 5.5);
    CPPUNIT_ASSERT_EQUAL(5.5, m_pDoc->GetValue(0,0,0));
    CPPUNIT_ASSERT_EQUAL(5.5, m_pDoc->GetValue(2,0,0));
    CPPUNIT_ASSERT_EQUAL(5.5, m_pDoc->GetValue(3,0,0));

    m_pDoc->DeleteTab(0);
}

void Test::testFuncParam()
{

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, "foo"));

    // First, the normal case, with no missing parameters.
    m_pDoc->SetString(0, 0, 0, "=AVERAGE(1;2;3)");
    m_pDoc->CalcFormulaTree(false, false);
    double val = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 2.0, val);

    // Now function with missing parameters.  Missing values should be treated
    // as zeros.
    m_pDoc->SetString(0, 0, 0, "=AVERAGE(1;;;)");
    m_pDoc->CalcFormulaTree(false, false);
    val = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 0.25, val);

    // Conversion of string to numeric argument.
    m_pDoc->SetString(0, 0, 0, "=\"\"+3");    // empty string
    m_pDoc->SetString(0, 1, 0, "=\" \"+3");   // only blank
    m_pDoc->SetString(0, 2, 0, "=\" 4 \"+3"); // number in blanks
    m_pDoc->SetString(0, 3, 0, "=\" x \"+3"); // non-numeric
    m_pDoc->SetString(0, 4, 0, "=\"4.4\"+3"); // locale dependent

    OUString aVal;
    ScCalcConfig aConfig;

    // With "Convert also locale dependent" and "Empty string as zero"=True option.
    aConfig.meStringConversion = ScCalcConfig::StringConversion::LOCALE;
    aConfig.mbEmptyStringAsZero = true;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    val = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 2, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.0, val);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    val = m_pDoc->GetValue(0, 4, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.4, val);

    // With "Convert also locale dependent" and "Empty string as zero"=False option.
    aConfig.meStringConversion = ScCalcConfig::StringConversion::LOCALE;
    aConfig.mbEmptyStringAsZero = false;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    aVal = m_pDoc->GetString( 0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    val = m_pDoc->GetValue(0, 2, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.0, val);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    val = m_pDoc->GetValue(0, 4, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.4, val);

    // With "Convert only unambiguous" and "Empty string as zero"=True option.
    aConfig.meStringConversion = ScCalcConfig::StringConversion::UNAMBIGUOUS;
    aConfig.mbEmptyStringAsZero = true;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    val = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 2, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.0, val);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 4, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);

    // With "Convert only unambiguous" and "Empty string as zero"=False option.
    aConfig.meStringConversion = ScCalcConfig::StringConversion::UNAMBIGUOUS;
    aConfig.mbEmptyStringAsZero = false;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    aVal = m_pDoc->GetString( 0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    m_pDoc->GetValue(0, 2, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.0, val);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 4, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);

    // With "Treat as zero" ("Empty string as zero" is ignored).
    aConfig.meStringConversion = ScCalcConfig::StringConversion::ZERO;
    aConfig.mbEmptyStringAsZero = true;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    val = m_pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 2, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 3, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    val = m_pDoc->GetValue(0, 4, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);

    // With "Generate #VALUE! error" ("Empty string as zero" is ignored).
    aConfig.meStringConversion = ScCalcConfig::StringConversion::ILLEGAL;
    aConfig.mbEmptyStringAsZero = false;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    aVal = m_pDoc->GetString( 0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 2, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    aVal = m_pDoc->GetString( 0, 4, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);

    m_pDoc->DeleteTab(0);
}

void Test::testNamedRange()
{
    static const RangeNameDef aNames[] = {
        { "Divisor",  "$Sheet1.$A$1:$A$1048576", 1 },
        { "MyRange1", "$Sheet1.$A$1:$A$100",     2 },
        { "MyRange2", "$Sheet1.$B$1:$B$100",     3 },
        { "MyRange3", "$Sheet1.$C$1:$C$100",     4 }
    };

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "Sheet1"));

    m_pDoc->SetValue (0, 0, 0, 101);

    std::unique_ptr<ScRangeName> pNames(new ScRangeName);
    bool bSuccess = insertRangeNames(m_pDoc, pNames.get(), aNames, aNames + SAL_N_ELEMENTS(aNames));
    CPPUNIT_ASSERT_MESSAGE("Failed to insert range names.", bSuccess);
    m_pDoc->SetRangeName(std::move(pNames));

    ScRangeName* pNewRanges = m_pDoc->GetRangeName();
    CPPUNIT_ASSERT(pNewRanges);

    // Make sure the index lookup does the right thing.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = pNewRanges->findByIndex(aNames[i].mnIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed.", p);
        OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved.", aName.equalsAscii(aNames[i].mpName));
    }

    // Test usage in formula expression.
    m_pDoc->SetString (1, 0, 0, "=A1/Divisor");
    m_pDoc->CalcAll();

    double result = m_pDoc->GetValue (1, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE ("calculation failed", 1.0, result);

    // Test copy-ability of range names.
    std::unique_ptr<ScRangeName> pCopiedRanges(new ScRangeName(*pNewRanges));
    m_pDoc->SetRangeName(std::move(pCopiedRanges));
    // Make sure the index lookup still works.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = m_pDoc->GetRangeName()->findByIndex(aNames[i].mnIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed with the copied instance.", p);
        OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved with the copied instance.", aName.equalsAscii(aNames[i].mpName));
    }

    // Test using another-sheet-local name, scope Sheet1.
    ScRangeData* pLocal1 = new ScRangeData( *m_pDoc, "local1", ScAddress(0,0,0));
    ScRangeData* pLocal2 = new ScRangeData( *m_pDoc, "local2", "$Sheet1.$A$1");
    ScRangeData* pLocal3 = new ScRangeData( *m_pDoc, "local3", "Sheet1.$A$1");
    ScRangeData* pLocal4 = new ScRangeData( *m_pDoc, "local4", "$A$1"); // implicit relative sheet reference
    std::unique_ptr<ScRangeName> pLocalRangeName1(new ScRangeName);
    pLocalRangeName1->insert(pLocal1);
    pLocalRangeName1->insert(pLocal2);
    pLocalRangeName1->insert(pLocal3);
    pLocalRangeName1->insert(pLocal4);
    m_pDoc->SetRangeName(0, std::move(pLocalRangeName1));

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (1, "Sheet2"));

    // Use other-sheet-local name of Sheet1 on Sheet2.
    ScAddress aPos(1,0,1);
    OUString aFormula("=Sheet1.local1+Sheet1.local2+Sheet1.local3+Sheet1.local4");
    m_pDoc->SetString(aPos, aFormula);
    OUString aString = m_pDoc->GetFormula(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula string should be equal", aFormula, aString);
    double fValue = m_pDoc->GetValue(aPos);
    ASSERT_DOUBLES_EQUAL_MESSAGE("value should be 4 times Sheet1.A1", 404.0, fValue);

    m_pDoc->DeleteTab(1);
    m_pDoc->SetRangeName(0,nullptr); // Delete the names.
    m_pDoc->SetRangeName(nullptr); // Delete the names.
    m_pDoc->DeleteTab(0);
}

void Test::testInsertNameList()
{
    m_pDoc->InsertTab(0, "Test");

    static const RangeNameDef aNames[] = {
        { "MyRange1", "$Test.$A$1:$A$100", 1 },
        { "MyRange2", "$Test.$B$1:$B$100", 2 },
        { "MyRange3", "$Test.$C$1:$C$100", 3 }
    };

    std::unique_ptr<ScRangeName> pNames(new ScRangeName);
    bool bSuccess = insertRangeNames(m_pDoc, pNames.get(), aNames, aNames + SAL_N_ELEMENTS(aNames));
    CPPUNIT_ASSERT_MESSAGE("Failed to insert range names.", bSuccess);
    m_pDoc->SetRangeName(std::move(pNames));

    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    ScAddress aPos(1,1,0);
    rDocFunc.InsertNameList(aPos, true);

    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i, aPos.IncRow())
    {
        OUString aName = m_pDoc->GetString(aPos);
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aNames[i].mpName), aName);
        ScAddress aExprPos = aPos;
        aExprPos.IncCol();
        OUString aExpr = m_pDoc->GetString(aExprPos);
        OUString aExpected = "=" + OUString::createFromAscii(aNames[i].mpExpr);
        CPPUNIT_ASSERT_EQUAL(aExpected, aExpr);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCSV()
{
    const int English = 0, European = 1;
    struct {
        const char *pStr; int eSep; bool bResult; double nValue;
    } aTests[] = {
        { "foo",       English,  false, 0.0 },
        { "1.0",       English,  true,  1.0 },
        { "1,0",       English,  false, 0.0 },
        { "1.0",       European, false, 0.0 },
        { "1.000",     European, true,  1000.0 },
        { "1,000",     European, true,  1.0 },
        { "1.000",     English,  true,  1.0 },
        { "1,000",     English,  true,  1000.0 },
        { " 1.0",      English,  true,  1.0 },
        { " 1.0  ",    English,  true,  1.0 },
        { "1.0 ",      European, false, 0.0 },
        { "1.000",     European, true,  1000.0 },
        { "1137.999",  English,  true,  1137.999 },
        { "1.000.00",  European, false, 0.0 },
        { "+,123",     English,  false, 0.0 },
        { "-,123",     English,  false, 0.0 }
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aTests); i++) {
        OUString aStr(aTests[i].pStr, strlen (aTests[i].pStr), RTL_TEXTENCODING_UTF8);
        double nValue = 0.0;
        bool bResult = ScStringUtil::parseSimpleNumber
                (aStr, aTests[i].eSep == English ? '.' : ',',
                 aTests[i].eSep == English ? ',' : '.',
                 0,
                 nValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE ("CSV numeric detection failure", aTests[i].bResult, bResult);
        CPPUNIT_ASSERT_EQUAL_MESSAGE ("CSV numeric value failure", aTests[i].nValue, nValue);
    }
}

template<typename Evaluator>
static void checkMatrixElements(const ScMatrix& rMat)
{
    SCSIZE nC, nR;
    rMat.GetDimensions(nC, nR);
    Evaluator aEval;
    for (SCSIZE i = 0; i < nC; ++i)
    {
        for (SCSIZE j = 0; j < nR; ++j)
        {
            aEval(i, j, rMat.Get(i, j));
        }
    }
}

namespace {

struct AllZeroMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of numeric type", int(ScMatValType::Value), static_cast<int>(rVal.nType));
        ASSERT_DOUBLES_EQUAL_MESSAGE("element value must be zero", 0.0, rVal.fVal);
    }
};

struct PartiallyFilledZeroMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of numeric type", int(ScMatValType::Value), static_cast<int>(rVal.nType));
        if (1 <= nCol && nCol <= 2 && 2 <= nRow && nRow <= 8)
        {
            ASSERT_DOUBLES_EQUAL_MESSAGE("element value must be 3.0", 3.0, rVal.fVal);
        }
        else
        {
            ASSERT_DOUBLES_EQUAL_MESSAGE("element value must be zero", 0.0, rVal.fVal);
        }
    }
};

struct AllEmptyMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of empty type", int(ScMatValType::Empty), static_cast<int>(rVal.nType));
        ASSERT_DOUBLES_EQUAL_MESSAGE("value of \"empty\" element is expected to be zero", 0.0, rVal.fVal);
    }
};

struct PartiallyFilledEmptyMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        if (nCol == 1 && nRow == 1)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of boolean type", int(ScMatValType::Boolean), static_cast<int>(rVal.nType));
            ASSERT_DOUBLES_EQUAL_MESSAGE("element value is not what is expected", 1.0, rVal.fVal);
        }
        else if (nCol == 4 && nRow == 5)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of value type", int(ScMatValType::Value), static_cast<int>(rVal.nType));
            ASSERT_DOUBLES_EQUAL_MESSAGE("element value is not what is expected", -12.5, rVal.fVal);
        }
        else if (nCol == 8 && nRow == 2)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of value type", int(ScMatValType::String), static_cast<int>(rVal.nType));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element value is not what is expected", OUString("Test"), rVal.aStr.getString());
        }
        else if (nCol == 8 && nRow == 11)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of empty path type", int(ScMatValType::EmptyPath), static_cast<int>(rVal.nType));
            ASSERT_DOUBLES_EQUAL_MESSAGE("value of \"empty\" element is expected to be zero", 0.0, rVal.fVal);
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of empty type", int(ScMatValType::Empty), static_cast<int>(rVal.nType));
            ASSERT_DOUBLES_EQUAL_MESSAGE("value of \"empty\" element is expected to be zero", 0.0, rVal.fVal);
        }
    }
};

}

void Test::testMatrix()
{
    svl::SharedStringPool& rPool = m_pDoc->GetSharedStringPool();
    ScMatrixRef pMat, pMat2;

    // First, test the zero matrix type.
    pMat = new ScMatrix(0, 0, 0.0);
    SCSIZE nC, nR;
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("matrix is not empty", SCSIZE(0), nC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("matrix is not empty", SCSIZE(0), nR);
    pMat->Resize(4, 10, 0.0);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("matrix size is not as expected", SCSIZE(4), nC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("matrix size is not as expected", SCSIZE(10), nR);
    CPPUNIT_ASSERT_MESSAGE("both 'and' and 'or' should evaluate to false",
                           !pMat->And());
    CPPUNIT_ASSERT_MESSAGE("both 'and' and 'or' should evaluate to false",
                           !pMat->Or());

    // Resizing into a larger matrix should fill the void space with zeros.
    checkMatrixElements<AllZeroMatrix>(*pMat);

    pMat->FillDouble(3.0, 1, 2, 2, 8);
    checkMatrixElements<PartiallyFilledZeroMatrix>(*pMat);
    CPPUNIT_ASSERT_MESSAGE("matrix is expected to be numeric", pMat->IsNumeric());
    CPPUNIT_ASSERT_MESSAGE("partially non-zero matrix should evaluate false on 'and' and true on 'or",
                           !pMat->And());
    CPPUNIT_ASSERT_MESSAGE("partially non-zero matrix should evaluate false on 'and' and true on 'or",
                           pMat->Or());
    pMat->FillDouble(5.0, 0, 0, nC-1, nR-1);
    CPPUNIT_ASSERT_MESSAGE("fully non-zero matrix should evaluate true both on 'and' and 'or",
                           pMat->And());
    CPPUNIT_ASSERT_MESSAGE("fully non-zero matrix should evaluate true both on 'and' and 'or",
                           pMat->Or());

    // Test the AND and OR evaluations.
    pMat = new ScMatrix(2, 2, 0.0);

    // Only some of the elements are non-zero.
    pMat->PutBoolean(true, 0, 0);
    pMat->PutDouble(1.0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("incorrect OR result", pMat->Or());
    CPPUNIT_ASSERT_MESSAGE("incorrect AND result", !pMat->And());

    // All of the elements are non-zero.
    pMat->PutBoolean(true, 0, 1);
    pMat->PutDouble(2.3, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect OR result", pMat->Or());
    CPPUNIT_ASSERT_MESSAGE("incorrect AND result", pMat->And());

    // Now test the empty matrix type.
    pMat = new ScMatrix(10, 20);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("matrix size is not as expected", SCSIZE(10), nC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("matrix size is not as expected", SCSIZE(20), nR);
    checkMatrixElements<AllEmptyMatrix>(*pMat);

    pMat->PutBoolean(true, 1, 1);
    pMat->PutDouble(-12.5, 4, 5);
    pMat->PutString(rPool.intern("Test"), 8, 2);
    pMat->PutEmptyPath(8, 11);
    checkMatrixElements<PartiallyFilledEmptyMatrix>(*pMat);

    // Test resizing.
    pMat = new ScMatrix(0, 0);
    pMat->Resize(2, 2, 1.5);
    pMat->PutEmpty(1, 1);

    CPPUNIT_ASSERT_EQUAL(1.5, pMat->GetDouble(0, 0));
    CPPUNIT_ASSERT_EQUAL(1.5, pMat->GetDouble(0, 1));
    CPPUNIT_ASSERT_EQUAL(1.5, pMat->GetDouble(1, 0));
    CPPUNIT_ASSERT_MESSAGE("PutEmpty() call failed.", pMat->IsEmpty(1, 1));

    // Max and min values.
    pMat = new ScMatrix(2, 2, 0.0);
    pMat->PutDouble(-10, 0, 0);
    pMat->PutDouble(-12, 0, 1);
    pMat->PutDouble(-8, 1, 0);
    pMat->PutDouble(-25, 1, 1);
    CPPUNIT_ASSERT_EQUAL(-25.0, pMat->GetMinValue(false));
    CPPUNIT_ASSERT_EQUAL(-8.0, pMat->GetMaxValue(false));
    pMat->PutString(rPool.intern("Test"), 0, 0);
    CPPUNIT_ASSERT_EQUAL(0.0, pMat->GetMaxValue(true)); // text as zero.
    CPPUNIT_ASSERT_EQUAL(-8.0, pMat->GetMaxValue(false)); // ignore text.
    pMat->PutBoolean(true, 0, 0);
    CPPUNIT_ASSERT_EQUAL(1.0, pMat->GetMaxValue(false));
    pMat = new ScMatrix(2, 2, 10.0);
    pMat->PutBoolean(false, 0, 0);
    pMat->PutDouble(12.5, 1, 1);
    CPPUNIT_ASSERT_EQUAL(0.0, pMat->GetMinValue(false));
    CPPUNIT_ASSERT_EQUAL(12.5, pMat->GetMaxValue(false));

    // Convert matrix into a linear double array. String elements become NaN
    // and empty elements become 0.
    pMat = new ScMatrix(3, 3);
    pMat->PutDouble(2.5, 0, 0);
    pMat->PutDouble(1.2, 0, 1);
    pMat->PutString(rPool.intern("A"), 1, 1);
    pMat->PutDouble(2.3, 2, 1);
    pMat->PutDouble(-20, 2, 2);

    static const double fNaN = std::numeric_limits<double>::quiet_NaN();

    std::vector<double> aDoubles;
    pMat->GetDoubleArray(aDoubles);

    {
        const double pChecks[] = { 2.5, 1.2, 0, 0, fNaN, 0, 0, 2.3, -20 };
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(pChecks), aDoubles.size());
        for (size_t i = 0, n = aDoubles.size(); i < n; ++i)
        {
            if (std::isnan(pChecks[i]))
                CPPUNIT_ASSERT_MESSAGE("NaN is expected, but it's not.", std::isnan(aDoubles[i]));
            else
                CPPUNIT_ASSERT_EQUAL(pChecks[i], aDoubles[i]);
        }
    }

    pMat2 = new ScMatrix(3, 3, 10.0);
    pMat2->PutString(rPool.intern("B"), 1, 0);
    pMat2->MergeDoubleArrayMultiply(aDoubles);

    {
        const double pChecks[] = { 25, 12, 0, fNaN, fNaN, 0, 0, 23, -200 };
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(pChecks), aDoubles.size());
        for (size_t i = 0, n = aDoubles.size(); i < n; ++i)
        {
            if (std::isnan(pChecks[i]))
                CPPUNIT_ASSERT_MESSAGE("NaN is expected, but it's not.", std::isnan(aDoubles[i]));
            else
                CPPUNIT_ASSERT_EQUAL(pChecks[i], aDoubles[i]);
        }
    }
}

void Test::testMatrixComparisonWithErrors()
{
    m_pDoc->InsertTab(0, "foo");

    // Insert the source values in A1:A2.
    m_pDoc->SetString(0, 0, 0, "=1/0");
    m_pDoc->SetValue( 0, 1, 0, 1.0);

    // Create a matrix formula in B3:B4 referencing A1:A2 and doing a greater
    // than comparison on it's values. Error value must be propagated.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(1, 2, 1, 3, aMark, "=A1:A2>0");

    CPPUNIT_ASSERT_EQUAL(OUString("#DIV/0!"), m_pDoc->GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL(1.0,                 m_pDoc->GetValue( 0,1,0));
    CPPUNIT_ASSERT_EQUAL(OUString("#DIV/0!"), m_pDoc->GetString(1,2,0));
    CPPUNIT_ASSERT_EQUAL(OUString("TRUE"),    m_pDoc->GetString(1,3,0));

    m_pDoc->DeleteTab(0);
}

void Test::testMatrixConditionalBooleanResult()
{
    m_pDoc->InsertTab(0, "foo");

    // Create matrix formulas in A1:B1,A2:B2,A3:B3,A4:B4 producing mixed
    // boolean and numeric results in an unformatted area.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula( 0,0, 1,0, aMark, "=IF({1;0};TRUE();42)");  // {TRUE,42}
    m_pDoc->InsertMatrixFormula( 0,1, 1,1, aMark, "=IF({0;1};TRUE();42)");  // {42,1} aim for {42,TRUE}
    m_pDoc->InsertMatrixFormula( 0,2, 1,2, aMark, "=IF({1;0};42;FALSE())"); // {42,0} aim for {42,FALSE}
    m_pDoc->InsertMatrixFormula( 0,3, 1,3, aMark, "=IF({0;1};42;FALSE())"); // {FALSE,42}

    CPPUNIT_ASSERT_EQUAL( OUString("TRUE"),  m_pDoc->GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL( OUString("42"),    m_pDoc->GetString(1,0,0));
    CPPUNIT_ASSERT_EQUAL( OUString("42"),    m_pDoc->GetString(0,1,0));
    //CPPUNIT_ASSERT_EQUAL( OUString("TRUE"),  m_pDoc->GetString(1,1,0));   // not yet
    CPPUNIT_ASSERT_EQUAL( OUString("42"),    m_pDoc->GetString(0,2,0));
    //CPPUNIT_ASSERT_EQUAL( OUString("FALSE"), m_pDoc->GetString(1,2,0));   // not yet
    CPPUNIT_ASSERT_EQUAL( OUString("FALSE"), m_pDoc->GetString(0,3,0));
    CPPUNIT_ASSERT_EQUAL( OUString("42"),    m_pDoc->GetString(1,3,0));

    m_pDoc->DeleteTab(0);
}

void Test::testEnterMixedMatrix()
{
    m_pDoc->InsertTab(0, "foo");

    // Insert the source values in A1:B2.
    m_pDoc->SetString(0, 0, 0, "A");
    m_pDoc->SetString(1, 0, 0, "B");
    double val = 1.0;
    m_pDoc->SetValue(0, 1, 0, val);
    val = 2.0;
    m_pDoc->SetValue(1, 1, 0, val);

    // Create a matrix range in A4:B5 referencing A1:B2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 3, 1, 4, aMark, "=A1:B2");

    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,0,0), m_pDoc->GetString(0,3,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(1,0,0), m_pDoc->GetString(1,3,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(0,1,0), m_pDoc->GetValue(0,4,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(1,1,0), m_pDoc->GetValue(1,4,0));

    m_pDoc->DeleteTab(0);
}

void Test::testMatrixEditable()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn auto calc on.

    m_pDoc->InsertTab(0, "Test");

    // Values in A1:B1.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(1,0,0), 2.0);

    // A2 is a normal formula.
    m_pDoc->SetString(ScAddress(0,1,0), "=5");

    // A3:A4 is a matrix.
    ScRange aMatRange(0,2,0,0,3,0);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aMatRange);
    m_pDoc->InsertMatrixFormula(0, 2, 0, 3, aMark, "=TRANSPOSE(A1:B1)");

    // Check their values.
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,3,0)));

    // Make sure A3:A4 is a matrix.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A3 should be matrix origin.",
                               ScMatrixMode::Formula, pFC->GetMatrixFlag());

    pFC = m_pDoc->GetFormulaCell(ScAddress(0,3,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A4 should be matrix reference.",
                               ScMatrixMode::Reference, pFC->GetMatrixFlag());

    // Check to make sure A3:A4 combined is editable.
    ScEditableTester aTester;
    aTester.TestSelection(*m_pDoc, aMark);
    CPPUNIT_ASSERT(aTester.IsEditable());

    m_pDoc->DeleteTab(0);
}

void Test::testCellCopy()
{
    m_pDoc->InsertTab(0, "TestTab");
    ScAddress aSrc(0,0,0);
    ScAddress aDest(0,1,0);
    OUString aStr("please copy me");
    m_pDoc->SetString(aSrc, "please copy me");
    CPPUNIT_ASSERT_EQUAL(aStr, m_pDoc->GetString(aSrc));
    // copy to self - why not ?
    m_pDoc->CopyCellToDocument(aSrc,aDest,*m_pDoc);
    CPPUNIT_ASSERT_EQUAL(aStr, m_pDoc->GetString(aDest));
}

void Test::testSheetCopy()
{
    m_pDoc->InsertTab(0, "TestTab");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.",
                               static_cast<SCTAB>(1), m_pDoc->GetTableCount());

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    // Insert text in A1.
    m_pDoc->SetString(ScAddress(0,0,0), "copy me");

    // Insert edit cells in B1:B3.
    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetTextCurrentDefaults("Edit 1");
    m_pDoc->SetEditText(ScAddress(1,0,0), rEE.CreateTextObject());
    rEE.SetTextCurrentDefaults("Edit 2");
    m_pDoc->SetEditText(ScAddress(1,1,0), rEE.CreateTextObject());
    rEE.SetTextCurrentDefaults("Edit 3");
    m_pDoc->SetEditText(ScAddress(1,2,0), rEE.CreateTextObject());

    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", MAXROW, nRow2);

    // insert a note
    ScAddress aAdrA1 (0,2,0); // empty cell content.
    ScPostIt *pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, "Hello world in A3");

    // Copy and test the result.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.",
                               static_cast<SCTAB>(2), m_pDoc->GetTableCount());

    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("copied sheet should also have all rows visible as the original.", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("copied sheet should also have all rows visible as the original.", MAXROW, nRow2);
    CPPUNIT_ASSERT_MESSAGE("There should be note on A3 in new sheet", m_pDoc->HasNote(ScAddress(0,2,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("copy me"), m_pDoc->GetString(ScAddress(0,0,1)));

    // Check the copied edit cells.
    const EditTextObject* pEditObj = m_pDoc->GetEditText(ScAddress(1,0,1));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in B1.", pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("Edit 1"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(1,1,1));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in B2.", pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("Edit 2"), pEditObj->GetText(0));
    pEditObj = m_pDoc->GetEditText(ScAddress(1,2,1));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell in B3.", pEditObj);
    CPPUNIT_ASSERT_EQUAL(OUString("Edit 3"), pEditObj->GetText(0));

    m_pDoc->DeleteTab(1);

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(4), nRow2);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(5), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(10), nRow2);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", SCROW(11), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", MAXROW, nRow2);

    // Copy the sheet once again.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.",
                               static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(4), nRow2);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(5), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(10), nRow2);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", SCROW(11), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", MAXROW, nRow2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSheetMove()
{
    m_pDoc->InsertTab(0, "TestTab1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.", static_cast<SCTAB>(1), m_pDoc->GetTableCount());
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", MAXROW, nRow2);

    //test if inserting before another sheet works
    m_pDoc->InsertTab(0, "TestTab2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have two sheets", static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", MAXROW, nRow2);

    // Move and test the result.
    m_pDoc->MoveTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("copied sheet should also have all rows visible as the original.", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("copied sheet should also have all rows visible as the original.", MAXROW, nRow2);
    OUString aName;
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "sheets should have changed places", OUString("TestTab1"), aName);

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(4), nRow2);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(5), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(10), nRow2);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", SCROW(11), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", MAXROW, nRow2);

    // Move the sheet once again.
    m_pDoc->MoveTab(1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 0 - 4 should be visible", SCROW(4), nRow2);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(5), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 5 - 10 should be hidden", SCROW(10), nRow2);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", SCROW(11), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 11 - maxrow should be visible", SCROW(MAXROW), nRow2);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "sheets should have changed places", OUString("TestTab2"), aName);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testDataArea()
{
    m_pDoc->InsertTab(0, "Data");

    // Totally empty sheet should be rightfully considered empty in all accounts.
    CPPUNIT_ASSERT_MESSAGE("Sheet is expected to be empty.", m_pDoc->IsPrintEmpty(0, 0, 0, 100, 100));
    CPPUNIT_ASSERT_MESSAGE("Sheet is expected to be empty.", m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // Now, set borders in some cells...
    ::editeng::SvxBorderLine aLine(nullptr, 50, SvxBorderLineStyle::SOLID);
    SvxBoxItem aBorderItem(ATTR_BORDER);
    aBorderItem.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBorderItem.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    for (SCROW i = 0; i < 100; ++i)
        // Set borders from row 1 to 100.
        m_pDoc->ApplyAttr(0, i, 0, aBorderItem);

    // Now the sheet is considered non-empty for printing purposes, but still
    // be empty in all the other cases.
    CPPUNIT_ASSERT_MESSAGE("Empty sheet with borders should be printable.",
                           !m_pDoc->IsPrintEmpty(0, 0, 0, 100, 100));
    CPPUNIT_ASSERT_MESSAGE("But it should still be considered empty in all the other cases.",
                           m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // Adding a real cell content should turn the block non-empty.
    m_pDoc->SetString(0, 0, 0, "Some text");
    CPPUNIT_ASSERT_MESSAGE("Now the block should not be empty with a real cell content.",
                           !m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // TODO: Add more tests for normal data area calculation.

    m_pDoc->DeleteTab(0);
}

void Test::testStreamValid()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");
    m_pDoc->InsertTab(3, "Sheet4");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We should have 4 sheet instances.", static_cast<SCTAB>(4), m_pDoc->GetTableCount());

    OUString a1("A1");
    OUString a2("A2");
    OUString test;

    // Put values into Sheet1.
    m_pDoc->SetString(0, 0, 0, a1);
    m_pDoc->SetString(0, 1, 0, a2);
    test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected value in Sheet1.A1", test, a1);
    test = m_pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected value in Sheet1.A2", test, a2);

    // Put formulas into Sheet2 to Sheet4 to reference values from Sheet1.
    m_pDoc->SetString(0, 0, 1, "=Sheet1.A1");
    m_pDoc->SetString(0, 1, 1, "=Sheet1.A2");
    m_pDoc->SetString(0, 0, 2, "=Sheet1.A1");
    m_pDoc->SetString(0, 0, 3, "=Sheet1.A2");

    test = m_pDoc->GetString(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected value in Sheet2.A1", test, a1);
    test = m_pDoc->GetString(0, 1, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected value in Sheet2.A2", test, a2);
    test = m_pDoc->GetString(0, 0, 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected value in Sheet3.A1", test, a1);
    test = m_pDoc->GetString(0, 0, 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected value in Sheet3.A1", test, a2);

    // Set all sheet streams valid after all the initial cell values are in
    // place. In reality we need to have real XML streams stored in order to
    // claim they are valid, but we are just testing the flag values here.
    m_pDoc->SetStreamValid(0, true);
    m_pDoc->SetStreamValid(1, true);
    m_pDoc->SetStreamValid(2, true);
    m_pDoc->SetStreamValid(3, true);
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(0));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(1));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(2));
    CPPUNIT_ASSERT_MESSAGE("Stream is expected to be valid.", m_pDoc->IsStreamValid(3));

    // Now, insert a new row at row 2 position on Sheet1.  This will move cell
    // A2 downward but cell A1 remains unmoved.
    m_pDoc->InsertRow(0, 0, MAXCOL, 0, 1, 2);
    test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A1 should not have moved.", test, a1);
    test = m_pDoc->GetString(0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the old cell A2 should now be at A4.", test, a2);
    ScRefCellValue aCell;
    aCell.assign(*m_pDoc, ScAddress(0,1,0));
    CPPUNIT_ASSERT_MESSAGE("Cell A2 should be empty.", aCell.isEmpty());
    aCell.assign(*m_pDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_MESSAGE("Cell A3 should be empty.", aCell.isEmpty());

    // After the move, Sheet1, Sheet2, and Sheet4 should have their stream
    // invalidated, whereas Sheet3's stream should still be valid.
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(0));
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(1));
    CPPUNIT_ASSERT_MESSAGE("Stream should have been invalidated.", !m_pDoc->IsStreamValid(3));
    CPPUNIT_ASSERT_MESSAGE("Stream should still be valid.", m_pDoc->IsStreamValid(2));

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFunctionLists()
{
    const char* aDataBase[] = {
        "DAVERAGE",
        "DCOUNT",
        "DCOUNTA",
        "DGET",
        "DMAX",
        "DMIN",
        "DPRODUCT",
        "DSTDEV",
        "DSTDEVP",
        "DSUM",
        "DVAR",
        "DVARP",
        nullptr
    };

    const char* aDateTime[] = {
        "DATE",
        "DATEDIF",
        "DATEVALUE",
        "DAY",
        "DAYS",
        "DAYS360",
        "DAYSINMONTH",
        "DAYSINYEAR",
        "EASTERSUNDAY",
        "HOUR",
        "ISLEAPYEAR",
        "ISOWEEKNUM",
        "MINUTE",
        "MONTH",
        "MONTHS",
        "NETWORKDAYS",
        "NETWORKDAYS.INTL",
        "NOW",
        "SECOND",
        "TIME",
        "TIMEVALUE",
        "TODAY",
        "WEEKDAY",
        "WEEKNUM",
        "WEEKNUM_OOO",
        "WEEKS",
        "WEEKSINYEAR",
        "WORKDAY.INTL",
        "YEAR",
        "YEARS",
        nullptr
    };

    const char* aFinancial[] = {
        "CUMIPMT",
        "CUMPRINC",
        "DB",
        "DDB",
        "EFFECT",
        "FV",
        "IPMT",
        "IRR",
        "ISPMT",
        "MIRR",
        "NOMINAL",
        "NPER",
        "NPV",
        "OPT_BARRIER",
        "OPT_PROB_HIT",
        "OPT_PROB_INMONEY",
        "OPT_TOUCH",
        "PDURATION",
        "PMT",
        "PPMT",
        "PV",
        "RATE",
        "RRI",
        "SLN",
        "SYD",
        "VDB",
        nullptr
    };

    const char* aInformation[] = {
        "CELL",
        "CURRENT",
        "FORMULA",
        "INFO",
        "ISBLANK",
        "ISERR",
        "ISERROR",
        "ISEVEN",
        "ISFORMULA",
        "ISLOGICAL",
        "ISNA",
        "ISNONTEXT",
        "ISNUMBER",
        "ISODD",
        "ISREF",
        "ISTEXT",
        "N",
        "NA",
        "TYPE",
        nullptr
    };

    const char* aLogical[] = {
        "AND",
        "FALSE",
        "IF",
        "IFERROR",
        "IFNA",
        "IFS",
        "NOT",
        "OR",
        "SWITCH",
        "TRUE",
        "XOR",
        nullptr
    };

    const char* aMathematical[] = {
        "ABS",
        "ACOS",
        "ACOSH",
        "ACOT",
        "ACOTH",
        "AGGREGATE",
        "ASIN",
        "ASINH",
        "ATAN",
        "ATAN2",
        "ATANH",
        "BITAND",
        "BITLSHIFT",
        "BITOR",
        "BITRSHIFT",
        "BITXOR",
        "CEILING",
        "CEILING.MATH",
        "CEILING.PRECISE",
        "CEILING.XCL",
        "COLOR",
        "COMBIN",
        "COMBINA",
        "CONVERT_OOO",
        "COS",
        "COSH",
        "COT",
        "COTH",
        "CSC",
        "CSCH",
        "DEGREES",
        "EUROCONVERT",
        "EVEN",
        "EXP",
        "FACT",
        "FLOOR",
        "FLOOR.MATH",
        "FLOOR.PRECISE",
        "FLOOR.XCL",
        "GCD",
        "INT",
        "ISO.CEILING",
        "LCM",
        "LN",
        "LOG",
        "LOG10",
        "MOD",
        "ODD",
        "PI",
        "POWER",
        "PRODUCT",
        "RADIANS",
        "RAND",
        "RAND.NV",
        "RANDBETWEEN.NV",
        "RAWSUBTRACT",
        "ROUND",
        "ROUNDDOWN",
        "ROUNDSIG",
        "ROUNDUP",
        "SEC",
        "SECH",
        "SIGN",
        "SIN",
        "SINH",
        "SQRT",
        "SUBTOTAL",
        "SUM",
        "SUMIF",
        "SUMIFS",
        "SUMSQ",
        "TAN",
        "TANH",
        "TRUNC",
        nullptr
    };

    const char* aArray[] = {
        "FOURIER",
        "FREQUENCY",
        "GROWTH",
        "LINEST",
        "LOGEST",
        "MDETERM",
        "MINVERSE",
        "MMULT",
        "MUNIT",
        "SUMPRODUCT",
        "SUMX2MY2",
        "SUMX2PY2",
        "SUMXMY2",
        "TRANSPOSE",
        "TREND",
        nullptr
    };

    const char* aStatistical[] = {
        "AVEDEV",
        "AVERAGE",
        "AVERAGEA",
        "AVERAGEIF",
        "AVERAGEIFS",
        "B",
        "BETA.DIST",
        "BETA.INV",
        "BETADIST",
        "BETAINV",
        "BINOM.DIST",
        "BINOM.INV",
        "BINOMDIST",
        "CHIDIST",
        "CHIINV",
        "CHISQ.DIST",
        "CHISQ.DIST.RT",
        "CHISQ.INV",
        "CHISQ.INV.RT",
        "CHISQ.TEST",
        "CHISQDIST",
        "CHISQINV",
        "CHITEST",
        "CONFIDENCE",
        "CONFIDENCE.NORM",
        "CONFIDENCE.T",
        "CORREL",
        "COUNT",
        "COUNTA",
        "COUNTBLANK",
        "COUNTIF",
        "COUNTIFS",
        "COVAR",
        "COVARIANCE.P",
        "COVARIANCE.S",
        "CRITBINOM",
        "DEVSQ",
        "ERF.PRECISE",
        "ERFC.PRECISE",
        "EXPON.DIST",
        "EXPONDIST",
        "F.DIST",
        "F.DIST.RT",
        "F.INV",
        "F.INV.RT",
        "F.TEST",
        "FDIST",
        "FINV",
        "FISHER",
        "FISHERINV",
        "FORECAST",
        "FORECAST.ETS.ADD",
        "FORECAST.ETS.MULT",
        "FORECAST.ETS.PI.ADD",
        "FORECAST.ETS.PI.MULT",
        "FORECAST.ETS.SEASONALITY",
        "FORECAST.ETS.STAT.ADD",
        "FORECAST.ETS.STAT.MULT",
        "FORECAST.LINEAR",
        "FTEST",
        "GAMMA",
        "GAMMA.DIST",
        "GAMMA.INV",
        "GAMMADIST",
        "GAMMAINV",
        "GAMMALN",
        "GAMMALN.PRECISE",
        "GAUSS",
        "GEOMEAN",
        "HARMEAN",
        "HYPGEOM.DIST",
        "HYPGEOMDIST",
        "INTERCEPT",
        "KURT",
        "LARGE",
        "LOGINV",
        "LOGNORM.DIST",
        "LOGNORM.INV",
        "LOGNORMDIST",
        "MAX",
        "MAXA",
        "MAXIFS",
        "MEDIAN",
        "MIN",
        "MINA",
        "MINIFS",
        "MODE",
        "MODE.MULT",
        "MODE.SNGL",
        "NEGBINOM.DIST",
        "NEGBINOMDIST",
        "NORM.DIST",
        "NORM.INV",
        "NORM.S.DIST",
        "NORM.S.INV",
        "NORMDIST",
        "NORMINV",
        "NORMSDIST",
        "NORMSINV",
        "PEARSON",
        "PERCENTILE",
        "PERCENTILE.EXC",
        "PERCENTILE.INC",
        "PERCENTRANK",
        "PERCENTRANK.EXC",
        "PERCENTRANK.INC",
        "PERMUT",
        "PERMUTATIONA",
        "PHI",
        "POISSON",
        "POISSON.DIST",
        "PROB",
        "QUARTILE",
        "QUARTILE.EXC",
        "QUARTILE.INC",
        "RANK",
        "RANK.AVG",
        "RANK.EQ",
        "RSQ",
        "SKEW",
        "SKEWP",
        "SLOPE",
        "SMALL",
        "STANDARDIZE",
        "STDEV",
        "STDEV.P",
        "STDEV.S",
        "STDEVA",
        "STDEVP",
        "STDEVPA",
        "STEYX",
        "T.DIST",
        "T.DIST.2T",
        "T.DIST.RT",
        "T.INV",
        "T.INV.2T",
        "T.TEST",
        "TDIST",
        "TINV",
        "TRIMMEAN",
        "TTEST",
        "VAR",
        "VAR.P",
        "VAR.S",
        "VARA",
        "VARP",
        "VARPA",
        "WEIBULL",
        "WEIBULL.DIST",
        "Z.TEST",
        "ZTEST",
        nullptr
    };

    const char* aSpreadsheet[] = {
        "ADDRESS",
        "AREAS",
        "CHOOSE",
        "COLUMN",
        "COLUMNS",
        "DDE",
        "ERROR.TYPE",
        "ERRORTYPE",
        "GETPIVOTDATA",
        "HLOOKUP",
        "HYPERLINK",
        "INDEX",
        "INDIRECT",
        "LOOKUP",
        "MATCH",
        "OFFSET",
        "ROW",
        "ROWS",
        "SHEET",
        "SHEETS",
        "STYLE",
        "VLOOKUP",
        nullptr
    };

    const char* aText[] = {
        "ARABIC",
        "ASC",
        "BAHTTEXT",
        "BASE",
        "CHAR",
        "CLEAN",
        "CODE",
        "CONCAT",
        "CONCATENATE",
        "DECIMAL",
        "DOLLAR",
        "ENCODEURL",
        "EXACT",
        "FILTERXML",
        "FIND",
        "FINDB",
        "FIXED",
        "JIS",
        "LEFT",
        "LEFTB",
        "LEN",
        "LENB",
        "LOWER",
        "MID",
        "MIDB",
        "NUMBERVALUE",
        "PROPER",
        "REGEX",
        "REPLACE",
        "REPLACEB",
        "REPT",
        "RIGHT",
        "RIGHTB",
        "ROMAN",
        "ROT13",
        "SEARCH",
        "SEARCHB",
        "SUBSTITUTE",
        "T",
        "TEXT",
        "TEXTJOIN",
        "TRIM",
        "UNICHAR",
        "UNICODE",
        "UPPER",
        "VALUE",
        "WEBSERVICE",
        nullptr
    };

    struct {
        const char* Category; const char** Functions;
    } aTests[] = {
        { "Database",     aDataBase },
        { "Date&Time",    aDateTime },
        { "Financial",    aFinancial },
        { "Information",  aInformation },
        { "Logical",      aLogical },
        { "Mathematical", aMathematical },
        { "Array",        aArray },
        { "Statistical",  aStatistical },
        { "Spreadsheet",  aSpreadsheet },
        { "Text",         aText },
        { "Add-in",       nullptr },
        { nullptr, nullptr }
    };

    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    sal_uInt32 n = pFuncMgr->getCount();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        const formula::IFunctionCategory* pCat = pFuncMgr->getCategory(i);
        CPPUNIT_ASSERT_MESSAGE("Unexpected category name", pCat->getName().equalsAscii(aTests[i].Category));
        sal_uInt32 nFuncCount = pCat->getCount();
        for (sal_uInt32 j = 0; j < nFuncCount; ++j)
        {
            const formula::IFunctionDescription* pFunc = pCat->getFunction(j);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected function name", OUString::createFromAscii(aTests[i].Functions[j]), pFunc->getFunctionName());
        }
    }
}

void Test::testGraphicsInGroup()
{
    m_pDoc->InsertTab(0, "TestTab");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.",
                               static_cast<SCTAB>(1), m_pDoc->GetTableCount());
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(MAXROW), nRow2);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != nullptr);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != nullptr);

    {
        //Add a square
        tools::Rectangle aOrigRect(2,2,100,100);
        SdrRectObj *pObj = new SdrRectObj(*pDrawLayer, aOrigRect);
        pPage->InsertObject(pObj);
        const tools::Rectangle &rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("must have equal position and size",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetPageAnchored(*pObj);

        //Use a range of rows guaranteed to include all of the square
        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should not change when page anchored",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should not change when page anchored",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("That shouldn't change size or positioning",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hiding should not change the logic rectangle",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);
        CPPUNIT_ASSERT_MESSAGE("Hiding should make invisible", !pObj->IsVisible());

        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should not change when cell anchored",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);
        CPPUNIT_ASSERT_MESSAGE("Show should make visible", pObj->IsVisible());
    }

    {
        // Add a circle.
        tools::Rectangle aOrigRect(10,10,210,210); // 200 x 200
        SdrCircObj* pObj = new SdrCircObj(*pDrawLayer, SdrCircKind::Full, aOrigRect);
        pPage->InsertObject(pObj);
        const tools::Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Position and size of the circle shouldn't change when inserted into the page.",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size changed when cell anchored. Not good.",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        // Insert 2 rows at the top.  This should push the circle object down.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);

        // Make sure the size of the circle is still identical.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size of the circle has changed, but shouldn't!",
                               aOrigRect.GetSize(), rNewRect.GetSize());

        // Delete 2 rows at the top.  This should bring the circle object to its original position.
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to move back to its original position.",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);
    }

    {
        // Add a line.
        basegfx::B2DPolygon aTempPoly;
        Point aStartPos(10,300), aEndPos(110,200); // bottom-left to top-right.
        tools::Rectangle aOrigRect(10,200,110,300); // 100 x 100
        aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
        aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
        SdrPathObj* pObj = new SdrPathObj(*pDrawLayer, OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
        pObj->NbcSetLogicRect(aOrigRect);
        pPage->InsertObject(pObj);
        const tools::Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size differ.",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size changed when cell-anchored. Not good.",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        // Insert 2 rows at the top and delete them immediately.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size of a line object changed after row insertion and removal.",
                               const_cast<const tools::Rectangle &>(aOrigRect), rNewRect);

        sal_Int32 n = pObj->GetPointCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly 2 points in a line object.", static_cast<sal_Int32>(2), n);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line shape has changed.",
                               aStartPos, pObj->GetPoint(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line shape has changed.",
                               aEndPos, pObj->GetPoint(1));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testGraphicsOnSheetMove()
{
    m_pDoc->InsertTab(0, "Tab1");
    m_pDoc->InsertTab(1, "Tab2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be only 2 sheets to begin with", static_cast<SCTAB>(2), m_pDoc->GetTableCount());

    m_pDoc->InitDrawLayer();
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);

    // Insert an object.
    tools::Rectangle aObjRect(2,2,100,100);
    SdrObject* pObj = new SdrRectObj(*pDrawLayer, aObjRect);
    pPage->InsertObject(pObj);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one object on the 1st sheet.", static_cast<size_t>(1), pPage->GetObjCount());

    const ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Object meta-data doesn't exist.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(0), pData->maStart.Tab());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(0), pData->maEnd.Tab());

    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 2nd sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet shouldn't have any object.", static_cast<size_t>(0), pPage->GetObjCount());

    // Insert a new sheet at left-end, and make sure the object has moved to
    // the 2nd page.
    m_pDoc->InsertTab(0, "NewTab");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 3 sheets.", static_cast<SCTAB>(3), m_pDoc->GetTableCount());
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have no object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1st sheet should have no object.", size_t(0), pPage->GetObjCount());
    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("2nd sheet should have one object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet should have one object.", size_t(1), pPage->GetObjCount());
    pPage = pDrawLayer->GetPage(2);
    CPPUNIT_ASSERT_MESSAGE("3rd sheet should have no object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("3rd sheet should have no object.", size_t(0), pPage->GetObjCount());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(1), pData->maStart.Tab());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(1), pData->maEnd.Tab());

    // Now, delete the sheet that just got inserted. The object should be back
    // on the 1st sheet.
    m_pDoc->DeleteTab(0);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have one object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1st sheet should have one object.", size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Size and position of the object shouldn't change.",
                           aObjRect, pObj->GetLogicRect());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(0), pData->maStart.Tab());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(0), pData->maEnd.Tab());

    // Move the 1st sheet to the last position.
    m_pDoc->MoveTab(0, 1);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have no object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1st sheet should have no object.", size_t(0), pPage->GetObjCount());
    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("2nd sheet should have one object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet should have one object.", size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(1), pData->maStart.Tab());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(1), pData->maEnd.Tab());

    // Copy the 2nd sheet, which has one drawing object to the last position.
    m_pDoc->CopyTab(1, 2);
    pPage = pDrawLayer->GetPage(2);
    CPPUNIT_ASSERT_MESSAGE("Copied sheet should have one object.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Copied sheet should have one object.", size_t(1), pPage->GetObjCount());
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object meta-data.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(2), pData->maStart.Tab());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong sheet ID in cell anchor data!", SCTAB(2), pData->maEnd.Tab());

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testToggleRefFlag()
{
    // In this test, there is no need to insert formula string into a cell in
    // the document, as ScRefFinder does not depend on the content of the
    // document except for the sheet names.

    m_pDoc->InsertTab(0, "Test");

    {
        // Calc A1: basic 2D reference

        OUString aFormula("=B100");
        ScAddress aPos(1, 5, 0);
        ScRefFinder aFinder(aFormula, aPos, *m_pDoc, formula::FormulaGrammar::CONV_OOO);

        // Original
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Does not equal the original text.", aFormula, aFinder.GetText());

        // column relative / row relative -> column absolute / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong conversion.", OUString("=$B$100"), aFormula );

        // column absolute / row absolute -> column relative / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong conversion.", OUString("=B$100"), aFormula );

        // column relative / row absolute -> column absolute / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong conversion.", OUString("=$B100"), aFormula );

        // column absolute / row relative -> column relative / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong conversion.", OUString("=B100"), aFormula );
    }

    {
        // Excel R1C1: basic 2D reference

        OUString aFormula("=R2C1");
        ScAddress aPos(3, 5, 0);
        ScRefFinder aFinder(aFormula, aPos, *m_pDoc, formula::FormulaGrammar::CONV_XL_R1C1);

        // Original
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Does not equal the original text.", aFormula, aFinder.GetText());

        // column absolute / row absolute -> column relative / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R2C[-3]"), aFormula);

        // column relative / row absolute - > column absolute / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R[-4]C1"), aFormula);

        // column absolute / row relative -> column relative / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R[-4]C[-3]"), aFormula);

        // column relative / row relative -> column absolute / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R2C1"), aFormula);
    }

    {
        // Excel R1C1: Selection at the end of the formula string and does not
        // overlap the formula string at all (inspired by fdo#39135).
        OUString aFormula("=R1C1");
        ScAddress aPos(1, 1, 0);
        ScRefFinder aFinder(aFormula, aPos, *m_pDoc, formula::FormulaGrammar::CONV_XL_R1C1);

        // Original
        CPPUNIT_ASSERT_EQUAL(aFormula, aFinder.GetText());

        // Make the column relative.
        sal_Int32 n = aFormula.getLength();
        aFinder.ToggleRel(n, n);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R1C[-1]"), aFormula);

        // Make the row relative.
        n = aFormula.getLength();
        aFinder.ToggleRel(n, n);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R[-1]C1"), aFormula);

        // Make both relative.
        n = aFormula.getLength();
        aFinder.ToggleRel(n, n);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R[-1]C[-1]"), aFormula);

        // Back to the original.
        n = aFormula.getLength();
        aFinder.ToggleRel(n, n);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=R1C1"), aFormula);
    }

    {
        // Calc A1:
        OUString aFormula("=A1+4");
        ScAddress aPos(1, 1, 0);
        ScRefFinder aFinder(aFormula, aPos, *m_pDoc, formula::FormulaGrammar::CONV_OOO);

        // Original
        CPPUNIT_ASSERT_EQUAL(aFormula, aFinder.GetText());

        // Set the cursor over the 'A1' part and toggle.
        aFinder.ToggleRel(2, 2);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=$A$1+4"), aFormula);

        aFinder.ToggleRel(2, 2);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=A$1+4"), aFormula);

        aFinder.ToggleRel(2, 2);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=$A1+4"), aFormula);

        aFinder.ToggleRel(2, 2);
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_EQUAL(OUString("=A1+4"), aFormula);
    }

    // TODO: Add more test cases esp. for 3D references, Excel A1 syntax, and
    // partial selection within formula string.

    m_pDoc->DeleteTab(0);
}

void Test::testAutofilter()
{
    m_pDoc->InsertTab( 0, "Test" );

    // cell contents (0 = empty cell)
    const char* aData[][3] = {
        { "C1", "C2", "C3" },
        {  "0",  "1",  "A" },
        {  "1",  "2",    nullptr },
        {  "1",  "2",  "B" },
        {  "0",  "2",  "B" }
    };

    SCCOL nCols = SAL_N_ELEMENTS(aData[0]);
    SCROW nRows = SAL_N_ELEMENTS(aData);

    // Populate cells.
    for (SCROW i = 0; i < nRows; ++i)
        for (SCCOL j = 0; j < nCols; ++j)
            if (aData[i][j])
                m_pDoc->SetString(j, i, 0, OUString::createFromAscii(aData[i][j]));

    ScDBData* pDBData = new ScDBData("NONAME", 0, 0, 0, nCols-1, nRows-1);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(), aRange.aStart.Row(),
                           aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_EQUAL;
    rEntry.GetQueryItem().mfVal = 0;
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(0, aParam, true);

    //control output
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 2 & 3 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 2 & 3 should be hidden", SCROW(2), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 2 & 3 should be hidden", SCROW(3), nRow2);

    // Remove filtering.
    rEntry.Clear();
    m_pDoc->Query(0, aParam, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("All rows should be shown.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All rows should be shown.", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All rows should be shown.", SCROW(MAXROW), nRow2);

    // Filter for non-empty cells by column C.
    rEntry.bDoQuery = true;
    rEntry.nField = 2;
    rEntry.SetQueryByNonEmpty();
    m_pDoc->Query(0, aParam, true);

    // only row 3 should be hidden.  The rest should be visible.
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 1 & 2 should be visible.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 1 & 2 should be visible.", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 1 & 2 should be visible.", SCROW(1), nRow2);
    bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 3 should be hidden.", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 3 should be hidden.", SCROW(2), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 3 should be hidden.", SCROW(2), nRow2);
    bHidden = m_pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 4 and down should be visible.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 4 and down should be visible.", SCROW(3), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 4 and down should be visible.", SCROW(MAXROW), nRow2);

    // Now, filter for empty cells by column C.
    rEntry.SetQueryByEmpty();
    m_pDoc->Query(0, aParam, true);

    // Now, only row 1 and 3, and 6 and down should be visible.
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be visible.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be visible.", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 1 should be visible.", SCROW(0), nRow2);
    bHidden = m_pDoc->RowHidden(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 2 should be hidden.", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 2 should be hidden.", SCROW(1), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 2 should be hidden.", SCROW(1), nRow2);
    bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 3 should be visible.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 3 should be visible.", SCROW(2), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("row 3 should be visible.", SCROW(2), nRow2);
    bHidden = m_pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 4 & 5 should be hidden.", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 4 & 5 should be hidden.", SCROW(3), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 4 & 5 should be hidden.", SCROW(4), nRow2);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 6 and down should be all visible.", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 6 and down should be all visible.", SCROW(5), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 6 and down should be all visible.", SCROW(MAXROW), nRow2);

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFilterTimeValue()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,0,0), "Hours");
    m_pDoc->SetValue(ScAddress(0,1,0), 72.3604166666671);
    m_pDoc->SetValue(ScAddress(0,2,0), 265);

    ScDBData* pDBData = new ScDBData(STR_DB_GLOBAL_NONAME, 0, 0, 0, 0, 2);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    // Apply the "hour:minute:second" format to A2:A3.
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    sal_uInt32 nFormat = pFormatter->GetFormatIndex(NF_TIME_HH_MMSS, LANGUAGE_ENGLISH_US);
    ScPatternAttr aNewAttrs(m_pDoc->GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));

    m_pDoc->ApplyPatternAreaTab(0, 1, 0, 2, 0, aNewAttrs); // apply it to A2:A3.

    printRange(m_pDoc, ScRange(0,0,0,0,2,0), "Data"); // A1:A3

    // Make sure the hour:minute:second format is really applied.
    CPPUNIT_ASSERT_EQUAL(OUString("1736:39:00"), m_pDoc->GetString(ScAddress(0,1,0))); // A2
    CPPUNIT_ASSERT_EQUAL(OUString("6360:00:00"), m_pDoc->GetString(ScAddress(0,2,0))); // A3

    // Filter by the A2 value.  Only A1 and A2 should be visible.
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_EQUAL;
    rEntry.GetQueryItem().maString = m_pDoc->GetSharedStringPool().intern("1736:39:00");
    rEntry.GetQueryItem().meType = ScQueryEntry::ByString;

    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(0, aParam, true);

    // A1:A2 should be visible while A3 should be filtered out.
    CPPUNIT_ASSERT_MESSAGE("A1 should be visible.", !m_pDoc->RowFiltered(0,0));
    CPPUNIT_ASSERT_MESSAGE("A2 should be visible.", !m_pDoc->RowFiltered(1,0));
    CPPUNIT_ASSERT_MESSAGE("A3 should be filtered out.", m_pDoc->RowFiltered(2,0));

    m_pDoc->DeleteTab(0);
}

void Test::testAutofilterOptimizations()
{
    m_pDoc->InsertTab( 0, "Test" );

    constexpr SCCOL nCols = 4;
    constexpr SCROW nRows = 200;
    m_pDoc->SetString(0, 0, 0, "Column1");
    m_pDoc->SetString(1, 0, 0, "Column2");
    m_pDoc->SetString(2, 0, 0, "Column3");
    m_pDoc->SetString(3, 0, 0, "Column4");

    // Fill 1st column with 0-199, 2nd with 1-200, 3rd with "1000"-"1199", 4th with "1001-1200"
    // (the pairs are off by one to each other to check filtering out a value filters out
    // only the relevant column).
    for(SCROW i = 0; i < nRows; ++i)
    {
        m_pDoc->SetValue(0, i + 1, 0, i);
        m_pDoc->SetValue(1, i + 1, 0, i+1);
        m_pDoc->SetString(2, i + 1, 0, "val" + OUString::number(i+1000));
        m_pDoc->SetString(3, i + 1, 0, "val" + OUString::number(i+1000+1));
    }

    ScDBData* pDBData = new ScDBData("NONAME", 0, 0, 0, nCols, nRows);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(), aRange.aStart.Row(),
                           aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry0 = aParam.GetEntry(0);
    rEntry0.bDoQuery = true;
    rEntry0.nField = 0;
    rEntry0.eOp = SC_EQUAL;
    rEntry0.GetQueryItems().resize(nRows);
    ScQueryEntry& rEntry1 = aParam.GetEntry(1);
    rEntry1.bDoQuery = true;
    rEntry1.nField = 1;
    rEntry1.eOp = SC_EQUAL;
    rEntry1.GetQueryItems().resize(nRows);
    ScQueryEntry& rEntry2 = aParam.GetEntry(2);
    rEntry2.bDoQuery = true;
    rEntry2.nField = 2;
    rEntry2.eOp = SC_EQUAL;
    rEntry2.GetQueryItems().resize(nRows);
    ScQueryEntry& rEntry3 = aParam.GetEntry(3);
    rEntry3.bDoQuery = true;
    rEntry3.nField = 3;
    rEntry3.eOp = SC_EQUAL;
    rEntry3.GetQueryItems().resize(nRows);
    // Set up autofilter to select all values except one in each column.
    // This should only filter out 2nd, 3rd, 6th and 7th rows.
    for( int i = 0; i < nRows; ++i )
    {
        if(i!= 1)
            rEntry0.GetQueryItems()[i].mfVal = i;
        if(i!= 2)
            rEntry1.GetQueryItems()[i].mfVal = i + 1;
        if(i!= 5)
        {
            rEntry2.GetQueryItems()[i].maString = m_pDoc->GetSharedStringPool().intern("val" + OUString::number(i+1000));
            rEntry2.GetQueryItems()[i].meType = ScQueryEntry::ByString;
        }
        if(i!= 6)
        {
            rEntry3.GetQueryItems()[i].maString = m_pDoc->GetSharedStringPool().intern("val" + OUString::number(i+1000+1));
            rEntry3.GetQueryItems()[i].meType = ScQueryEntry::ByString;
        }
    }
    // add queryParam to database range.
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(0, aParam, true);

    // check that only rows with filtered out values are hidden, and not rows that share
    // a value in a different column
    SCROW nRow1, nRow2;
    CPPUNIT_ASSERT_MESSAGE("row 2 should be visible", !m_pDoc->RowHidden(1, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 3 should be hidden", m_pDoc->RowHidden(2, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 4 should be hidden", m_pDoc->RowHidden(3, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 5 should be visible", !m_pDoc->RowHidden(4, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 6 should be visible", !m_pDoc->RowHidden(5, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 7 should be hidden", m_pDoc->RowHidden(6, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 8 should be hidden", m_pDoc->RowHidden(7, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_MESSAGE("row 9 should be visible", !m_pDoc->RowHidden(8, 0, &nRow1, &nRow2));

    // Remove filtering.
    rEntry0.Clear();
    rEntry1.Clear();
    rEntry2.Clear();
    m_pDoc->Query(0, aParam, true);
    CPPUNIT_ASSERT_MESSAGE("All rows should be shown.", !m_pDoc->RowHidden(0, 0, &nRow1, &nRow2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All rows should be shown.", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("All rows should be shown.", SCROW(MAXROW), nRow2);

    m_pDoc->DeleteTab(0);
}

void Test::testTdf76441()
{
    m_pDoc->InsertTab(0, "Test");

    // The result will be different depending on whether the format is set before
    // or after inserting the string

    OUString aCode = "MM:SS";
    sal_Int32 nCheckPos;
    SvNumFormatType nType;
    sal_uInt32 nFormat;
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    pFormatter->PutEntry( aCode, nCheckPos, nType, nFormat );

    ScPatternAttr aNewAttrs(m_pDoc->GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    {
        // First insert the string, then the format
        m_pDoc->SetString(ScAddress(0,0,0), "01:20");

        m_pDoc->ApplyPattern(0, 0, 0, aNewAttrs);

        CPPUNIT_ASSERT_EQUAL(OUString("20:00"), m_pDoc->GetString(ScAddress(0,0,0)));
    }

    {
        // First set the format, then insert the string
        m_pDoc->ApplyPattern(0, 1, 0, aNewAttrs);

        m_pDoc->SetString(ScAddress(0,1,0), "01:20");

        // Without the fix in place, this test would have failed with
        // - Expected: 01:20
        // - Actual  : 20:00
        CPPUNIT_ASSERT_EQUAL(OUString("01:20"), m_pDoc->GetString(ScAddress(0,1,0)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testTdf76836()
{
    m_pDoc->InsertTab(0, "Test");

    OUString aCode = "\"192.168.0.\"@";
    sal_Int32 nCheckPos;
    SvNumFormatType nType;
    sal_uInt32 nFormat;
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    pFormatter->PutEntry( aCode, nCheckPos, nType, nFormat );

    ScPatternAttr aNewAttrs(m_pDoc->GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));

    m_pDoc->ApplyPattern(0, 0, 0, aNewAttrs);
    m_pDoc->SetValue(0,0,0, 10.0);

    // Without the fix in place, this test would have failed with
    // - Expected: 10
    // - Actual  : 192.168.0.10
    CPPUNIT_ASSERT_EQUAL(OUString("10"), m_pDoc->GetString(ScAddress(0,0,0)));

    m_pDoc->ApplyPattern(0, 1, 0, aNewAttrs);
    m_pDoc->SetString(ScAddress(0,1,0), "10");
    CPPUNIT_ASSERT_EQUAL(OUString("192.168.0.10"), m_pDoc->GetString(ScAddress(0,1,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testTdf142186()
{
    m_pDoc->InsertTab(0, "Test");

    // The result will be different depending on whether the format is set before
    // or after inserting the string

    OUString aCode = "0\".\"0";
    sal_Int32 nCheckPos;
    SvNumFormatType nType;
    sal_uInt32 nFormat;
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    pFormatter->PutEntry( aCode, nCheckPos, nType, nFormat );

    ScPatternAttr aNewAttrs(m_pDoc->GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    {
        // First insert the string, then the format
        m_pDoc->SetString(ScAddress(0,0,0), "123.45");

        m_pDoc->ApplyPattern(0, 0, 0, aNewAttrs);

        CPPUNIT_ASSERT_EQUAL(OUString("12.3"), m_pDoc->GetString(ScAddress(0,0,0)));
    }

    {
        // First set the format, then insert the string
        m_pDoc->ApplyPattern(0, 1, 0, aNewAttrs);

        m_pDoc->SetString(ScAddress(0,1,0), "123.45");

        // Without the fix in place, this test would have failed with
        // - Expected: 12.3
        // - Actual  : 1234.5
        CPPUNIT_ASSERT_EQUAL(OUString("12.3"), m_pDoc->GetString(ScAddress(0,1,0)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testTdf137063()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(0,0,0, 0.000000006);
    m_pDoc->SetValue(0,1,0, 0.0000000006);

    // Without the fix in place, this test would have failed with
    // - Expected: 0.000000006
    // - Actual  : 6E-09
    CPPUNIT_ASSERT_EQUAL(OUString("0.000000006"), m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("6E-10"), m_pDoc->GetString(ScAddress(0,1,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testTdf126342()
{
    m_pDoc->InsertTab(0, "Test");

    OUString aCode = "YYYY-MM-DD";
    sal_Int32 nCheckPos;
    SvNumFormatType nType;
    sal_uInt32 nFormat;
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    pFormatter->PutEntry( aCode, nCheckPos, nType, nFormat );

    ScPatternAttr aNewAttrs(m_pDoc->GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    m_pDoc->ApplyPattern(0, 0, 0, aNewAttrs);

    m_pDoc->SetString(ScAddress(0,0,0), "11/7/19");

    CPPUNIT_ASSERT_EQUAL(OUString("2019-11-07"), m_pDoc->GetString(ScAddress(0,0,0)));

    // Overwrite the existing date with the exact same input
    m_pDoc->SetString(ScAddress(0,0,0), "11/7/19");

    // Without the fix in place, this test would have failed with
    // - Expected: 2019-11-07
    // - Actual  : 2011-07-19
    CPPUNIT_ASSERT_EQUAL(OUString("2019-11-07"), m_pDoc->GetString(ScAddress(0,0,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testAdvancedFilter()
{
    m_pDoc->InsertTab(0, "Test");

    // cell contents (nullptr = empty cell)
    std::vector<std::vector<const char*>> aData = {
        { "Value", "Tag" }, // A1:B11
        {  "1", "R" },
        {  "2", "R" },
        {  "3", "R" },
        {  "4", "C" },
        {  "5", "C" },
        {  "6", "C" },
        {  "7", "R" },
        {  "8", "R" },
        {  "9", "R" },
        { "10", "C" },
        { nullptr },
        { "Value", "Tag" }, // A13:B14
        { "> 5", "R" },
    };

    // Populate cells.
    for (size_t nRow = 0; nRow < aData.size(); ++nRow)
    {
        const std::vector<const char*>& rRowData = aData[nRow];
        for (size_t nCol = 0; nCol < rRowData.size(); ++nCol)
        {
            const char* pCell = rRowData[nCol];
            if (pCell)
                m_pDoc->SetString(nCol, nRow, 0, OUString::createFromAscii(pCell));
        }
    }

    ScDBData* pDBData = new ScDBData(STR_DB_GLOBAL_NONAME, 0, 0, 0, 1, 10);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    ScRange aDataRange(0,0,0,1,10,0);
    ScRange aFilterRuleRange(0,12,0,1,13,0);

    printRange(m_pDoc, aDataRange, "Data");
    printRange(m_pDoc, aFilterRuleRange, "Filter Rule");

    ScQueryParam aQueryParam;
    aQueryParam.bHasHeader = true;
    aQueryParam.nCol1 = aDataRange.aStart.Col();
    aQueryParam.nRow1 = aDataRange.aStart.Row();
    aQueryParam.nCol2 = aDataRange.aEnd.Col();
    aQueryParam.nRow2 = aDataRange.aEnd.Row();
    aQueryParam.nTab = aDataRange.aStart.Tab();

    bool bGood = m_pDoc->CreateQueryParam(aFilterRuleRange, aQueryParam);
    CPPUNIT_ASSERT_MESSAGE("failed to create query param.", bGood);

    // First entry is for the 'Value' field, and is greater than 5.
    ScQueryEntry aEntry = aQueryParam.GetEntry(0);
    CPPUNIT_ASSERT(aEntry.bDoQuery);
    CPPUNIT_ASSERT_EQUAL(SCCOLROW(0), aEntry.nField);
    CPPUNIT_ASSERT_EQUAL(SC_GREATER, aEntry.eOp);

    ScQueryEntry::QueryItemsType aItems = aEntry.GetQueryItems();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aItems.size());
    CPPUNIT_ASSERT_EQUAL(ScQueryEntry::ByValue, aItems[0].meType);
    CPPUNIT_ASSERT_EQUAL(5.0, aItems[0].mfVal);

    // Second entry is for the 'Tag' field, and is == 'R'.
    aEntry = aQueryParam.GetEntry(1);
    CPPUNIT_ASSERT(aEntry.bDoQuery);
    CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aEntry.nField);
    CPPUNIT_ASSERT_EQUAL(SC_EQUAL, aEntry.eOp);

    aItems = aEntry.GetQueryItems();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aItems.size());
    CPPUNIT_ASSERT_EQUAL(ScQueryEntry::ByString, aItems[0].meType);
    CPPUNIT_ASSERT_EQUAL(OUString("R"), aItems[0].maString.getString());

    // perform the query.
    m_pDoc->Query(0, aQueryParam, true);

    // Only rows 1,8-10 should be visible.
    bool bFiltered = m_pDoc->RowFiltered(0, 0);
    CPPUNIT_ASSERT_MESSAGE("row 1 (header row) should be visible", !bFiltered);

    SCROW nRow1 = -1, nRow2 = -1;
    bFiltered = m_pDoc->RowFiltered(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 2-7 should be filtered out.", bFiltered);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 2-7 should be filtered out.", SCROW(1), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 2-7 should be filtered out.", SCROW(6), nRow2);

    bFiltered = m_pDoc->RowFiltered(7, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 8-10 should be visible.", !bFiltered);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 8-10 should be visible.", SCROW(7), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("rows 8-10 should be visible.", SCROW(9), nRow2);

    m_pDoc->DeleteTab(0);
}

void Test::testDateFilterContains()
{
    m_pDoc->InsertTab(0, "Test");

    constexpr SCCOL nCols = 1;
    constexpr SCROW nRows = 5;
    m_pDoc->SetString(0, 0, 0, "Date");
    m_pDoc->SetString(0, 1, 0, "1/2/2021");
    m_pDoc->SetString(0, 2, 0, "2/1/1999");
    m_pDoc->SetString(0, 3, 0, "2/1/1997");
    m_pDoc->SetString(0, 4, 0, "3/3/2001");
    m_pDoc->SetString(0, 5, 0, "3/3/1996");

    // Set the fields as dates.
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    sal_uInt32 nFormat = pFormatter->GetFormatIndex(NF_DATE_DIN_YYMMDD, LANGUAGE_ENGLISH_US);
    ScPatternAttr aNewAttrs(m_pDoc->GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    m_pDoc->ApplyPatternAreaTab(0, 1, 0, 5, 0, aNewAttrs); // apply it to A1:A6

    ScDBData* pDBData = new ScDBData("NONAME", 0, 0, 0, nCols, nRows);
    m_pDoc->SetAnonymousDBData(0, std::unique_ptr<ScDBData>(pDBData));

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(), aRange.aStart.Row(),
                           aRange.aStart.Tab(), ScMF::Auto);

    //create the query param
    ScQueryParam aParam;
    pDBData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_CONTAINS;
    rEntry.GetQueryItem().maString = m_pDoc->GetSharedStringPool().intern("2");
    pDBData->SetQueryParam(aParam);

    // perform the query.
    m_pDoc->Query(0, aParam, true);

    // Dates in rows 2-4 contain '2', row 5 shows 2001 only as 01, and row 6 doesn't contain it at all.
    CPPUNIT_ASSERT_MESSAGE("row 2 should be visible", !m_pDoc->RowHidden(1, 0));
    CPPUNIT_ASSERT_MESSAGE("row 3 should be visible", !m_pDoc->RowHidden(2, 0));
    CPPUNIT_ASSERT_MESSAGE("row 4 should be visible", !m_pDoc->RowHidden(3, 0));
    CPPUNIT_ASSERT_MESSAGE("row 5 should be hidden", m_pDoc->RowHidden(4, 0));
    CPPUNIT_ASSERT_MESSAGE("row 6 should be hidden", m_pDoc->RowHidden(5, 0));

    m_pDoc->DeleteTab(0);
}

void Test::testTdf98642()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->SetString(0, 0, 0, "test");

    ScRangeData* pName1 = new ScRangeData( *m_pDoc, "name1", "$Sheet1.$A$1");
    ScRangeData* pName2 = new ScRangeData( *m_pDoc, "name2", "$Sheet1.$A$1");

    std::unique_ptr<ScRangeName> pGlobalRangeName(new ScRangeName());
    pGlobalRangeName->insert(pName1);
    pGlobalRangeName->insert(pName2);
    m_pDoc->SetRangeName(std::move(pGlobalRangeName));

    m_pDoc->SetString(1, 0, 0, "=name1");
    m_pDoc->SetString(1, 1, 0, "=name2");

    CPPUNIT_ASSERT_EQUAL(OUString("test"), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test"), m_pDoc->GetString(1, 1, 0));

    OUString aFormula = m_pDoc->GetFormula(1,0,0);
    CPPUNIT_ASSERT_EQUAL(OUString("=name1"), aFormula);
    aFormula = m_pDoc->GetFormula(1,1,0);

    // Without the fix in place, this test would have failed with
    // - Expected: =name2
    // - Actual  : =name1
    CPPUNIT_ASSERT_EQUAL(OUString("=name2"), aFormula);

    m_pDoc->DeleteTab(0);
}

void Test::testMergedCells()
{
    //test merge and unmerge
    //TODO: an undo/redo test for this would be a good idea
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->DoMerge(0, 1, 1, 3, 3, false);
    SCCOL nEndCol = 1;
    SCROW nEndRow = 1;
    m_pDoc->ExtendMerge( 1, 1, nEndCol, nEndRow, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("did not merge cells", SCCOL(3), nEndCol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("did not merge cells", SCROW(3), nEndRow);
    ScRange aRange(0,2,0,MAXCOL,2,0);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    m_xDocShell->GetDocFunc().InsertCells(aRange, &aMark, INS_INSROWS_BEFORE, true, true);
    m_pDoc->ExtendMerge(1, 1, nEndCol, nEndRow, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("did not increase merge area", SCCOL(3), nEndCol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("did not increase merge area", SCROW(4), nEndRow);
    m_pDoc->DeleteTab(0);
}

void Test::testRenameTable()
{
    //test set rename table
    //TODO: set name1 and name2 and do an undo to check if name 1 is set now
    //TODO: also check if new name for table is same as another table

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    //test case 1 , rename table2 to sheet 1, it should return error
    OUString nameToSet = "Sheet1";
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    CPPUNIT_ASSERT_MESSAGE("name same as another table is being set", !rDocFunc.RenameTable(1,nameToSet,false,true) );

    //test case 2 , simple rename to check name
    nameToSet = "test1";
    m_xDocShell->GetDocFunc().RenameTable(0,nameToSet,false,true);
    OUString nameJustSet;
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("table not renamed", nameToSet, nameJustSet);

    //test case 3 , rename again
    OUString anOldName;
    m_pDoc->GetName(0,anOldName);

    nameToSet = "test2";
    rDocFunc.RenameTable(0,nameToSet,false,true);
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("table not renamed", nameToSet, nameJustSet);

    //test case 4 , check if  undo works
    SfxUndoAction* pUndo = new ScUndoRenameTab(m_xDocShell.get(),0,anOldName,nameToSet);
    pUndo->Undo();
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct name is not set after undo", nameJustSet, anOldName);

    pUndo->Redo();
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set after redo", nameJustSet, nameToSet);
    delete pUndo;

    m_pDoc->DeleteTab(0);
    m_pDoc->DeleteTab(1);
}

void Test::testSetBackgroundColor()
{
    //test set background color
    //TODO: set color1 and set color2 and do an undo to check if color1 is set now.

    m_pDoc->InsertTab(0, "Sheet1");
    Color aColor;

     //test yellow
    aColor=COL_YELLOW;
    m_xDocShell->GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set",
                           aColor, m_pDoc->GetTabBgColor(0));

    Color aOldTabBgColor=m_pDoc->GetTabBgColor(0);
    aColor = COL_BLUE;
    m_xDocShell->GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set the second time",
                           aColor, m_pDoc->GetTabBgColor(0));

    //now check for undo
    SfxUndoAction* pUndo = new ScUndoTabColor(m_xDocShell.get(), 0, aOldTabBgColor, aColor);
    pUndo->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set after undo", aOldTabBgColor, m_pDoc->GetTabBgColor(0));
    pUndo->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set after undo", aColor, m_pDoc->GetTabBgColor(0));
    delete pUndo;
    m_pDoc->DeleteTab(0);
}

void Test::testUpdateReference()
{
    //test that formulas are correctly updated during sheet delete
    //TODO: add tests for relative references, updating of named ranges, ...
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->InsertTab(2, "Sheet3");
    m_pDoc->InsertTab(3, "Sheet4");

    m_pDoc->SetValue(0,0,2, 1);
    m_pDoc->SetValue(1,0,2, 2);
    m_pDoc->SetValue(1,1,3, 4);
    m_pDoc->SetString(2,0,2, "=A1+B1");
    m_pDoc->SetString(2,1,2, "=Sheet4.B2+A1");

    double aValue;
    aValue = m_pDoc->GetValue(2,0,2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2,1,2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula does not return correct result", aValue, 5);

    //test deleting both sheets: one is not directly before the sheet, the other one is
    m_pDoc->DeleteTab(0);
    aValue = m_pDoc->GetValue(2,0,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting first sheet formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2,1,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting first sheet formula does not return correct result", aValue, 5);

    m_pDoc->DeleteTab(0);
    aValue = m_pDoc->GetValue(2,0,0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting second sheet formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2,1,0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting second sheet formula does not return correct result", aValue, 5);

    //test adding two sheets
    m_pDoc->InsertTab(0, "Sheet2");
    aValue = m_pDoc->GetValue(2,0,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2,1,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 5);

    m_pDoc->InsertTab(0, "Sheet1");
    aValue = m_pDoc->GetValue(2,0,2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting second sheet formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2,1,2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting second sheet formula does not return correct result", aValue, 5);

    //test new DeleteTabs/InsertTabs methods
    m_pDoc->DeleteTabs(0, 2);
    aValue = m_pDoc->GetValue(2, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting sheets formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting sheets formula does not return correct result", aValue, 5);

    std::vector<OUString> aSheets;
    aSheets.emplace_back("Sheet1");
    aSheets.emplace_back("Sheet2");
    m_pDoc->InsertTabs(0, aSheets, true);
    aValue = m_pDoc->GetValue(2, 0, 2);

    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 3);
    aValue = m_pDoc->GetValue(2, 1, 2);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 5);

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);

    // Test positional update and invalidation of lookup cache for insertion
    // and deletion within entire column reference.
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    m_pDoc->SetString(0,1,0, "s1");
    m_pDoc->SetString(0,0,1, "=MATCH(\"s1\";Sheet1.A:A;0)");
    aValue = m_pDoc->GetValue(0,0,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected MATCH result", 2, aValue);
    m_pDoc->InsertRow(0,0,MAXCOL,0,0,1);    // insert 1 row before row 1 in Sheet1
    aValue = m_pDoc->GetValue(0,0,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected MATCH result", 3, aValue);
    m_pDoc->DeleteRow(0,0,MAXCOL,0,0,1);    // delete row 1 in Sheet1
    aValue = m_pDoc->GetValue(0,0,1);
    ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected MATCH result", 2, aValue);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSearchCells()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,0,0), "A");
    m_pDoc->SetString(ScAddress(0,1,0), "B");
    m_pDoc->SetString(ScAddress(0,2,0), "A");
    // Leave A4 blank.
    m_pDoc->SetString(ScAddress(0,4,0), "A");
    m_pDoc->SetString(ScAddress(0,5,0), "B");
    m_pDoc->SetString(ScAddress(0,6,0), "C");

    SvxSearchItem aItem(SID_SEARCH_ITEM);
    aItem.SetSearchString("A");
    aItem.SetCommand(SvxSearchCmd::FIND_ALL);
    ScMarkData aMarkData(m_pDoc->GetSheetLimits());
    aMarkData.SelectOneTable(0);
    SCCOL nCol = 0;
    SCROW nRow = 0;
    SCTAB nTab = 0;
    ScRangeList aMatchedRanges;
    OUString aUndoStr;
    bool bSuccess = m_pDoc->SearchAndReplace(aItem, nCol, nRow, nTab, aMarkData, aMatchedRanges, aUndoStr);

    CPPUNIT_ASSERT_MESSAGE("Search And Replace should succeed", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly 3 matching cells.", size_t(3), aMatchedRanges.size());
    ScAddress aHit(0,0,0);
    CPPUNIT_ASSERT_MESSAGE("A1 should be inside the matched range.", aMatchedRanges.In(aHit));
    aHit.SetRow(2);
    CPPUNIT_ASSERT_MESSAGE("A3 should be inside the matched range.", aMatchedRanges.In(aHit));
    aHit.SetRow(4);
    CPPUNIT_ASSERT_MESSAGE("A5 should be inside the matched range.", aMatchedRanges.In(aHit));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaPosition()
{
    m_pDoc->InsertTab(0, "Test");

    ScAddress aPos(0,0,0); // A1
    m_pDoc->SetString(aPos, "=ROW()");
    aPos.IncRow(); // A2
    m_pDoc->SetString(aPos, "=ROW()");
    aPos.SetRow(3); // A4;
    m_pDoc->SetString(aPos, "=ROW()");

    {
        SCROW aRows[] = { 0, 1, 3 };
        bool bRes = checkFormulaPositions(*m_pDoc, aPos.Tab(), aPos.Col(), aRows, SAL_N_ELEMENTS(aRows));
        CPPUNIT_ASSERT(bRes);
    }

    m_pDoc->InsertRow(0,0,0,0,1,5); // Insert 5 rows at A2.
    {
        SCROW aRows[] = { 0, 6, 8 };
        bool bRes = checkFormulaPositions(*m_pDoc, aPos.Tab(), aPos.Col(), aRows, SAL_N_ELEMENTS(aRows));
        CPPUNIT_ASSERT(bRes);
    }

    m_pDoc->DeleteTab(0);
}

namespace {

bool hasRange(const ScDocument* pDoc, const std::vector<ScTokenRef>& rRefTokens, const ScRange& rRange, const ScAddress& rPos)
{
    for (const ScTokenRef& p : rRefTokens)
    {
        if (!ScRefTokenHelper::isRef(p) || ScRefTokenHelper::isExternalRef(p))
            continue;

        switch (p->GetType())
        {
            case formula::svSingleRef:
            {
                ScSingleRefData aData = *p->GetSingleRef();
                if (rRange.aStart != rRange.aEnd)
                    break;

                ScAddress aThis = aData.toAbs(*pDoc, rPos);
                if (aThis == rRange.aStart)
                    return true;
            }
            break;
            case formula::svDoubleRef:
            {
                ScComplexRefData aData = *p->GetDoubleRef();
                ScRange aThis = aData.toAbs(*pDoc, rPos);
                if (aThis == rRange)
                    return true;
            }
            break;
            default:
                ;
        }
    }
    return false;
}

}

void Test::testJumpToPrecedentsDependents()
{
    // Precedent is another cell that the cell references, while dependent is
    // another cell that references it.
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(2, 0, 0, "=A1+A2+B3"); // C1
    m_pDoc->SetString(2, 1, 0, "=A1");       // C2
    m_pDoc->CalcAll();

    std::vector<ScTokenRef> aRefTokens;
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();

    {
        // C1's precedent should be A1:A2,B3.
        ScAddress aC1(2, 0, 0);
        ScRangeList aRange(aC1);
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("A1:A2 should be a precedent of C1.",
                               hasRange(m_pDoc, aRefTokens, ScRange(0, 0, 0, 0, 1, 0), aC1));
        CPPUNIT_ASSERT_MESSAGE("B3 should be a precedent of C1.",
                               hasRange(m_pDoc, aRefTokens, ScRange(1, 2, 0), aC1));
    }

    {
        // C2's precedent should be A1 only.
        ScAddress aC2(2, 1, 0);
        ScRangeList aRange(aC2);
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("there should only be one reference token.",
                               static_cast<size_t>(1), aRefTokens.size());
        CPPUNIT_ASSERT_MESSAGE("A1 should be a precedent of C1.",
                               hasRange(m_pDoc, aRefTokens, ScRange(0, 0, 0), aC2));
    }

    {
        // A1's dependent should be C1:C2.
        ScAddress aA1(0, 0, 0);
        ScRangeList aRange(aA1);
        rDocFunc.DetectiveCollectAllSuccs(aRange, aRefTokens);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("C1:C2 should be the only dependent of A1.",
                               std::vector<ScTokenRef>::size_type(1), aRefTokens.size());
        CPPUNIT_ASSERT_MESSAGE("C1:C2 should be the only dependent of A1.",
                               hasRange(m_pDoc, aRefTokens, ScRange(2, 0, 0, 2, 1, 0), aA1));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testTdf64001()
{
    m_pDoc->InsertTab(0, "test");

    ScMarkData aMarkData(m_pDoc->GetSheetLimits());
    aMarkData.SelectTable(0, true);

    m_pDoc->SetString( 0, 0, 0, "TRUE" );
    m_pDoc->Fill( 0, 0, 0, 0, nullptr, aMarkData, 9, FILL_TO_BOTTOM, FILL_AUTO );

    for (SCCOL i = 0; i < 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL( OUString("TRUE"), m_pDoc->GetString( 0, i, 0 ) );
    }

    m_pDoc->SetString( 0, 10, 0, "FALSE" );

    m_pDoc->SetString( 1, 0, 0, "=COUNTIF(A1:A11;TRUE)" );

    // Without the fix in place, this test would have failed with
    // - Expected: 10
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL( 10.0, m_pDoc->GetValue( 1, 0, 0 ) );

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFill()
{
    m_pDoc->InsertTab(0, "test");

    m_pDoc->SetValue(0,0,0,1);

    ScMarkData aMarkData(m_pDoc->GetSheetLimits());
    aMarkData.SelectTable(0, true);

    m_pDoc->Fill( 0, 0, 0, 0, nullptr, aMarkData, 5);
    for (SCROW i = 0; i< 6; ++i)
        ASSERT_DOUBLES_EQUAL(static_cast<double>(i+1.0), m_pDoc->GetValue(0, i, 0));

    // check that hidden rows are not affected by autofill
    // set values for hidden rows
    m_pDoc->SetValue(0,1,0,10);
    m_pDoc->SetValue(0,2,0,10);

    m_pDoc->SetRowHidden(1, 2, 0, true);
    m_pDoc->Fill( 0, 0, 0, 0, nullptr, aMarkData, 8);

    ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,1,0));
    ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,2,0));
    for (SCROW i = 3; i< 8; ++i)
        ASSERT_DOUBLES_EQUAL(static_cast<double>(i-1.0), m_pDoc->GetValue(0, i, 0));

    m_pDoc->Fill( 0, 0, 0, 8, nullptr, aMarkData, 5, FILL_TO_RIGHT );
    for (SCCOL i = 0; i < 5; ++i)
    {
        for(SCROW j = 0; j < 8; ++j)
        {
            if (j > 2)
            {
                ASSERT_DOUBLES_EQUAL(static_cast<double>(j-1+i), m_pDoc->GetValue(i, j, 0));
            }
            else if (j == 0)
            {
                ASSERT_DOUBLES_EQUAL(static_cast<double>(i+1), m_pDoc->GetValue(i, 0, 0));
            }
            else // j == 1 || j == 2
            {
                if(i == 0)
                    ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,j,0));
                else
                    ASSERT_DOUBLES_EQUAL(0.0, m_pDoc->GetValue(i,j,0));
            }
        }
    }

    // test auto fill user data lists
    m_pDoc->SetString( 0, 100, 0, "January" );
    m_pDoc->Fill( 0, 100, 0, 100, nullptr, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    OUString aTestValue = m_pDoc->GetString( 0, 101, 0 );
    CPPUNIT_ASSERT_EQUAL( OUString("February"), aTestValue );
    aTestValue = m_pDoc->GetString( 0, 102, 0 );
    CPPUNIT_ASSERT_EQUAL( OUString("March"), aTestValue );

    // test that two same user data list entries will not result in incremental fill
    m_pDoc->SetString( 0, 101, 0, "January" );
    m_pDoc->Fill( 0, 100, 0, 101, nullptr, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    for ( SCROW i = 102; i <= 103; ++i )
    {
        aTestValue = m_pDoc->GetString( 0, i, 0 );
        CPPUNIT_ASSERT_EQUAL( OUString("January"), aTestValue );
    }

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    // Fill A1:A6 with 1,2,3,4,5,6.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    ScRange aRange(0,0,0,0,5,0);
    aMarkData.SetMarkArea(aRange);
    rFunc.FillSeries(aRange, &aMarkData, FILL_TO_BOTTOM, FILL_AUTO, FILL_DAY, MAXDOUBLE, 1.0, MAXDOUBLE, true);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // Undo should clear the area except for the top cell.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    for (SCROW i = 1; i <= 5; ++i)
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(0,i,0)));

    // Redo should put the serial values back in.
    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,5,0)));

    // test that filling formulas vertically up does the right thing
    for(SCROW nRow = 0; nRow < 10; ++nRow)
        m_pDoc->SetValue(100, 100 + nRow, 0, 1);

    m_pDoc->SetString(100, 110, 0, "=A111");

    m_pDoc->Fill(100, 110, 100, 110, nullptr, aMarkData, 10, FILL_TO_TOP, FILL_AUTO);
    for(SCROW nRow = 110; nRow >= 100; --nRow)
    {
        OUString aExpected = "=A" + OUString::number(nRow +1);
        OUString aFormula = m_pDoc->GetFormula(100, nRow, 0);
        CPPUNIT_ASSERT_EQUAL(aExpected, aFormula);
    }

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    m_pDoc->SetString( 0, 100, 0, "2012-10-31" );
    m_pDoc->SetString( 0, 101, 0, "2012-10-31" );
    m_pDoc->Fill( 0, 100, 0, 101, nullptr, aMarkData, 3, FILL_TO_BOTTOM, FILL_AUTO );

    // tdf#89754, Without the fix in place, this test would have failed with
    // - Expected: 2012-10-31
    // - Actual  : 2012-11-01
    CPPUNIT_ASSERT_EQUAL( OUString("2012-10-31"), m_pDoc->GetString( 0, 102, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("2012-10-31"), m_pDoc->GetString( 0, 103, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("2012-10-31"), m_pDoc->GetString( 0, 104, 0 ) );

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0, 0, 0, 0, MAXROW, 0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    m_pDoc->SetString(0, 100, 0, "2019-10-31");
    m_pDoc->SetString(0, 101, 0, "2019-11-30");
    m_pDoc->SetString(0, 102, 0, "2019-12-31");
    m_pDoc->Fill(0, 100, 0, 102, nullptr, aMarkData, 3, FILL_TO_BOTTOM, FILL_AUTO);

    // tdf#58745, Without the fix in place, this test would have failed with
    // - Expected: 2020-01-31
    // - Actual  : 2019-01-11
    CPPUNIT_ASSERT_EQUAL(OUString("2020-01-31"), m_pDoc->GetString(0, 103, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("2020-02-29"), m_pDoc->GetString(0, 104, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("2020-03-31"), m_pDoc->GetString(0, 105, 0));

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    m_pDoc->SetString( 0, 50, 0, "1.0" );
    m_pDoc->SetString( 0, 51, 0, "1.1" );
    m_pDoc->SetString( 0, 52, 0, "1.2" );
    m_pDoc->SetString( 0, 53, 0, "1.3" );
    m_pDoc->Fill( 0, 50, 0, 53, nullptr, aMarkData, 3, FILL_TO_BOTTOM, FILL_AUTO );

    CPPUNIT_ASSERT_EQUAL( OUString("1.4"), m_pDoc->GetString( 0, 54, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("1.5"), m_pDoc->GetString( 0, 55, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("1.6"), m_pDoc->GetString( 0, 56, 0 ) );

    m_pDoc->SetString( 0, 60, 0, "4.0" );
    m_pDoc->SetString( 0, 61, 0, "4.1" );
    m_pDoc->SetString( 0, 62, 0, "4.2" );
    m_pDoc->SetString( 0, 63, 0, "4.3" );
    m_pDoc->Fill( 0, 60, 0, 63, nullptr, aMarkData, 3, FILL_TO_BOTTOM, FILL_AUTO );

    // tdf#37424: Without the fix in place, this test would have failed with
    // - Expected: 4.4
    // - Actual  : 5
    CPPUNIT_ASSERT_EQUAL( OUString("4.4"), m_pDoc->GetString( 0, 64, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("4.5"), m_pDoc->GetString( 0, 65, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("4.6"), m_pDoc->GetString( 0, 66, 0 ) );

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    m_pDoc->SetString( 0, 70, 0, "001-001-001" );
    m_pDoc->Fill( 0, 70, 0, 70, nullptr, aMarkData, 3, FILL_TO_BOTTOM, FILL_AUTO );

    // tdf#105268: Without the fix in place, this test would have failed with
    // - Expected: 001-001-002
    // - Actual  : 001-001000
    CPPUNIT_ASSERT_EQUAL( OUString("001-001-002"), m_pDoc->GetString( 0, 71, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("001-001-003"), m_pDoc->GetString( 0, 72, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("001-001-004"), m_pDoc->GetString( 0, 73, 0 ) );

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    m_pDoc->SetString( 0, 80, 0, "1%" );
    m_pDoc->Fill( 0, 80, 0, 80, nullptr, aMarkData, 3, FILL_TO_BOTTOM, FILL_AUTO );

    // tdf#89998: Without the fix in place, this test would have failed with
    // - Expected: 2.00%
    // - Actual  : 101.00%
    CPPUNIT_ASSERT_EQUAL( OUString("2.00%"), m_pDoc->GetString( 0, 81, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("3.00%"), m_pDoc->GetString( 0, 82, 0 ) );
    CPPUNIT_ASSERT_EQUAL( OUString("4.00%"), m_pDoc->GetString( 0, 83, 0 ) );

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    m_pDoc->SetString( 0, 0, 0, "1" );
    m_pDoc->SetString( 0, 1, 0, "1.1" );
    m_pDoc->Fill( 0, 0, 0, 1, nullptr, aMarkData, 60, FILL_TO_BOTTOM, FILL_AUTO );

    // tdf#129606: Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 6.00000000000001
    CPPUNIT_ASSERT_EQUAL( OUString("6"), m_pDoc->GetString( 0, 50, 0 ) );

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFillSimple()
{
    m_pDoc->InsertTab(0, "test");

    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetString(0, 1, 0, "=10");

    ScMarkData aMarkData(m_pDoc->GetSheetLimits());
    aMarkData.SelectTable(0, true);

    m_pDoc->Fill( 0, 0, 0, 1, nullptr, aMarkData, 6, FILL_TO_BOTTOM, FILL_AUTO);

    for(SCROW nRow = 0; nRow < 8; ++nRow)
    {
        if (nRow % 2 == 0)
        {
            double nVal = m_pDoc->GetValue(0, nRow, 0);
            CPPUNIT_ASSERT_EQUAL((nRow+2)/2.0, nVal);
        }
        else
        {
            OString aMsg = "wrong value in row: " + OString::number(nRow);
            double nVal = m_pDoc->GetValue(0, nRow, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), 10.0, nVal);
        }
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFindAreaPosVertical()
{
    std::vector<std::vector<const char*>> aData = {
        {   nullptr, "1", "1" },
        { "1",   nullptr, "1" },
        { "1", "1", "1" },
        {   nullptr, "1", "1" },
        { "1", "1", "1" },
        { "1",   nullptr, "1" },
        { "1", "1", "1" },
    };

    m_pDoc->InsertTab(0, "Test1");
    clearRange( m_pDoc, ScRange(0, 0, 0, 1, aData.size(), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    m_pDoc->SetRowHidden(4,4,0,true);
    bool bHidden = m_pDoc->RowHidden(4,0);
    CPPUNIT_ASSERT(bHidden);

    SCCOL nCol = 0;
    SCROW nRow = 0;
    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(MAXROW, nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(0), nCol);

    nCol = 1;
    nRow = 2;

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_DOWN);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCol);

    nCol = 2;
    nRow = 6;
    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_UP);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), nCol);

    m_pDoc->DeleteTab(0);
}

void Test::testFindAreaPosColRight()
{
    std::vector<std::vector<const char*>> aData = {
        { "", "1", "1", "", "1", "1", "1" },
        { "", "", "1", "1", "1", "", "1" },
    };

    m_pDoc->InsertTab(0, "test1");
    clearRange( m_pDoc, ScRange(0, 0, 0, 7, aData.size(), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    m_pDoc->SetColHidden(4,4,0,true);
    bool bHidden = m_pDoc->ColHidden(4,0);
    CPPUNIT_ASSERT(bHidden);

    SCCOL nCol = 0;
    SCROW nRow = 0;
    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(2), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(5), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(6), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), nRow);
    CPPUNIT_ASSERT_EQUAL(MAXCOL, nCol);

    nCol = 2;
    nRow = 1;

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(3), nCol);

    m_pDoc->FindAreaPos(nCol, nRow, 0, SC_MOVE_RIGHT);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), nRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(6), nCol);

    m_pDoc->DeleteTab(0);
}

void Test::testShiftCells()
{
    m_pDoc->InsertTab(0, "foo");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    OUString aTestVal("Some Text");

    // Text into cell E5.
    m_pDoc->SetString(4, 3, 0, aTestVal);

    // put a Note in cell E5
    ScAddress rAddr(4, 3, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, "Hello");

    CPPUNIT_ASSERT_MESSAGE("there should be a note", m_pDoc->HasNote(4, 3, 0));

    // Insert cell at D5. This should shift the string cell to right.
    m_pDoc->InsertCol(3, 0, 3, 0, 3, 1);
    OUString aStr = m_pDoc->GetString(5, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We should have a string cell here.", aTestVal, aStr);
    CPPUNIT_ASSERT_MESSAGE("D5 is supposed to be blank.", m_pDoc->IsBlockEmpty(0, 3, 4, 3, 4));

    CPPUNIT_ASSERT_MESSAGE("there should be NO note", !m_pDoc->HasNote(4, 3, 0));
    CPPUNIT_ASSERT_MESSAGE("there should be a note", m_pDoc->HasNote(5, 3, 0));

    // Delete cell D5, to shift the text cell back into D5.
    m_pDoc->DeleteCol(3, 0, 3, 0, 3, 1);
    aStr = m_pDoc->GetString(4, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We should have a string cell here.", aTestVal, aStr);
    CPPUNIT_ASSERT_MESSAGE("E5 is supposed to be blank.", m_pDoc->IsBlockEmpty(0, 4, 4, 4, 4));

    CPPUNIT_ASSERT_MESSAGE("there should be NO note", !m_pDoc->HasNote(5, 3, 0));
    CPPUNIT_ASSERT_MESSAGE("there should be a note", m_pDoc->HasNote(4, 3, 0));

    m_pDoc->DeleteTab(0);
}

void Test::testNoteBasic()
{
    m_pDoc->InsertTab(0, "PostIts");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    CPPUNIT_ASSERT(!m_pDoc->HasNotes());

    // Check for note's presence in all tables before inserting any notes.
    for (SCTAB i = 0; i <= MAXTAB; ++i)
    {
        bool bHasNotes = m_pDoc->HasTabNotes(i);
        CPPUNIT_ASSERT(!bHasNotes);
    }

    ScAddress aAddr(2, 2, 0); // cell C3
    ScPostIt *pNote = m_pDoc->GetOrCreateNote(aAddr);

    pNote->SetText(aAddr, "Hello world");
    pNote->SetAuthor("Jim Bob");

    ScPostIt *pGetNote = m_pDoc->GetNote(aAddr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("note should be itself", pNote, pGetNote);

    // Insert one row at row 1.
    bool bInsertRow = m_pDoc->InsertRow(0, 0, MAXCOL, 0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("failed to insert row", bInsertRow );

    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", !m_pDoc->GetNote(aAddr));
    aAddr.IncRow(); // cell C4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("note not there", pNote, m_pDoc->GetNote(aAddr));

    // Insert column at column A.
    bool bInsertCol = m_pDoc->InsertCol(0, 0, MAXROW, 0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("failed to insert column", bInsertCol );

    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", !m_pDoc->GetNote(aAddr));
    aAddr.IncCol(); // cell D4
    CPPUNIT_ASSERT_EQUAL_MESSAGE("note not there", pNote, m_pDoc->GetNote(aAddr));

    // Insert a new sheet to shift the current sheet to the right.
    m_pDoc->InsertTab(0, "Table2");
    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", !m_pDoc->GetNote(aAddr));
    aAddr.IncTab(); // Move to the next sheet.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("note not there", pNote, m_pDoc->GetNote(aAddr));

    m_pDoc->DeleteTab(0);
    aAddr.IncTab(-1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("note not there", pNote, m_pDoc->GetNote(aAddr));

    // Insert cell at C4.  This should NOT shift the note position.
    bInsertRow = m_pDoc->InsertRow(2, 0, 2, 0, 3, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert cell at C4.", bInsertRow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note shouldn't have moved but it has.", pNote, m_pDoc->GetNote(aAddr));

    // Delete cell at C4.  Again, this should NOT shift the note position.
    m_pDoc->DeleteRow(2, 0, 2, 0, 3, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note shouldn't have moved but it has.", pNote, m_pDoc->GetNote(aAddr));

    // Now, with the note at D4, delete cell D3. This should shift the note one cell up.
    m_pDoc->DeleteRow(3, 0, 3, 0, 2, 1);
    aAddr.IncRow(-1); // cell D3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note at D4 should have shifted up to D3.", pNote, m_pDoc->GetNote(aAddr));

    // Delete column C. This should shift the note one cell left.
    m_pDoc->DeleteCol(0, 0, MAXROW, 0, 2, 1);
    aAddr.IncCol(-1); // cell C3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note at D3 should have shifted left to C3.", pNote, m_pDoc->GetNote(aAddr));

    // Insert a text where the note is.
    m_pDoc->SetString(aAddr, "Note is here.");

    // Delete row 1. This should shift the note from C3 to C2.
    m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 1);
    aAddr.IncRow(-1); // C2
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note at C3 should have shifted up to C2.", pNote, m_pDoc->GetNote(aAddr));

    m_pDoc->DeleteTab(0);
}

void Test::testNoteDeleteRow()
{
    m_pDoc->InsertTab(0, "Sheet1");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    ScAddress aPos(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aPos);
    pNote->SetText(aPos, "Hello");
    pNote->SetAuthor("Jim Bob");

    CPPUNIT_ASSERT_MESSAGE("there should be a note", m_pDoc->HasNote(1, 1, 0));

    // test with IsBlockEmpty
    bool bIgnoreNotes = true;
    CPPUNIT_ASSERT_MESSAGE("The Block should be detected as empty (no Notes)", m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100, bIgnoreNotes));
    bIgnoreNotes = false;
    CPPUNIT_ASSERT_MESSAGE("The Block should NOT be detected as empty", !m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100, bIgnoreNotes));

    m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 1, 1);

    CPPUNIT_ASSERT_MESSAGE("there should be no more note", !m_pDoc->HasNote(1, 1, 0));

    // Set values and notes into B3:B4.
    aPos = ScAddress(1,2,0); // B3
    m_pDoc->SetString(aPos, "First");
    ScNoteUtil::CreateNoteFromString(*m_pDoc, aPos, "First Note", false, false);

    aPos = ScAddress(1,3,0); // B4
    m_pDoc->SetString(aPos, "Second");
    ScNoteUtil::CreateNoteFromString(*m_pDoc, aPos, "Second Note", false, false);

    // Delete row 2.
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rDocFunc.DeleteCells(ScRange(0,1,0,MAXCOL,1,0), &aMark, DelCellCmd::CellsUp, true);

    // Check to make sure the notes have shifted upward.
    pNote = m_pDoc->GetNote(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("B2 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("First Note"), pNote->GetText());
    pNote = m_pDoc->GetNote(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Second Note"), pNote->GetText());
    pNote = m_pDoc->GetNote(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("B4 should NOT have a note.", !pNote);

    // Undo.

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT_MESSAGE("Failed to get undo manager.", pUndoMgr);
    m_pDoc->CreateAllNoteCaptions(); // to make sure that all notes have their corresponding caption objects...

    pUndoMgr->Undo();
    pNote = m_pDoc->GetNote(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("B2 should NOT have a note.", !pNote);
    pNote = m_pDoc->GetNote(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("First Note"), pNote->GetText());
    pNote = m_pDoc->GetNote(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("B4 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Second Note"), pNote->GetText());

    // Delete row 3.
    rDocFunc.DeleteCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, DelCellCmd::CellsUp, true);

    pNote = m_pDoc->GetNote(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Second Note"), pNote->GetText());
    pNote = m_pDoc->GetNote(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("B4 should NOT have a note.", !pNote);

    // Undo and check the result.
    pUndoMgr->Undo();
    pNote = m_pDoc->GetNote(ScAddress(1,2,0));
    CPPUNIT_ASSERT_MESSAGE("B3 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("First Note"), pNote->GetText());
    pNote = m_pDoc->GetNote(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("B4 should have a note.", pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Second Note"), pNote->GetText());

    m_pDoc->DeleteTab(0);
}

void Test::testNoteDeleteCol()
{
    m_pDoc->InsertTab(0, "Sheet1");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, "Hello");
    pNote->SetAuthor("Jim Bob");

    CPPUNIT_ASSERT_MESSAGE("there should be a note", m_pDoc->HasNote(1, 1, 0));

    m_pDoc->DeleteCol(0, 0, MAXROW, 0, 1, 1);

    CPPUNIT_ASSERT_MESSAGE("there should be no more note", !m_pDoc->HasNote(1, 1, 0));

    m_pDoc->DeleteTab(0);
}

void Test::testNoteLifeCycle()
{
    m_pDoc->InsertTab(0, "Test");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    ScAddress aPos(1,1,0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new cell comment.", pNote);

    pNote->SetText(aPos, "New note");
    std::unique_ptr<ScPostIt> pNote2 = m_pDoc->ReleaseNote(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This note instance is expected to be identical to the original.", pNote, pNote2.get());
    CPPUNIT_ASSERT_MESSAGE("The note shouldn't be here after it's been released.", !m_pDoc->HasNote(aPos));

    // Modify the internal state of the note instance to make sure it's really
    // been released.
    pNote->SetText(aPos, "New content");

    // Re-insert the note back to the same place.
    m_pDoc->SetNote(aPos, std::move(pNote2));
    SdrCaptionObj* pCaption = pNote->GetOrCreateCaption(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to create a caption object.", pCaption);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This caption should belong to the drawing layer of the document.",
                           m_pDoc->GetDrawLayer(), static_cast<ScDrawLayer*>(&pCaption->getSdrModelFromSdrObject()));

    // Copy B2 with note to a clipboard.

    ScClipParam aClipParam(aPos, false);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aMarkData(m_pDoc->GetSheetLimits());
    aMarkData.SelectOneTable(0);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMarkData, false, true);

    ScPostIt* pClipNote = aClipDoc.GetNote(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to copy note to the clipboard.", pClipNote);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note on the clipboard should share the same caption object from the original.",
                           pCaption, pClipNote->GetCaption());


    // Move B2 to B3 with note, which creates an ScUndoDragDrop, and Undo.

    ScAddress aOrigPos(aPos);
    ScAddress aMovePos(1,2,0);
    ScPostIt* pOrigNote = m_pDoc->GetNote(aOrigPos);
    const SdrCaptionObj* pOrigCaption = pOrigNote->GetOrCreateCaption(aOrigPos);
    bool const bCut = true;       // like Drag&Drop
    bool bRecord = true;    // record Undo
    bool const bPaint = false;    // don't care about
    bool bApi = true;       // API to prevent dialogs
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    bool bMoveDone = rDocFunc.MoveBlock(ScRange(aOrigPos, aOrigPos), aMovePos, bCut, bRecord, bPaint, bApi);
    CPPUNIT_ASSERT_MESSAGE("Cells not moved", bMoveDone);

    // Verify the note move.
    ScPostIt* pGoneNote = m_pDoc->GetNote(aOrigPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to move the note from source.", !pGoneNote);
    ScPostIt* pMoveNote = m_pDoc->GetNote(aMovePos);
    CPPUNIT_ASSERT_MESSAGE("Failed to move the note to destination.", pMoveNote);

    // The caption object should not be identical, it was newly created upon
    // Drop from clipboard.
    // pOrigCaption is a dangling pointer.
    const SdrCaptionObj* pMoveCaption = pMoveNote->GetOrCreateCaption(aMovePos);
    CPPUNIT_ASSERT_MESSAGE("Captions identical after move.", pOrigCaption != pMoveCaption);

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();   // this should not crash ... tdf#92995

    // Verify the note move Undo.
    pMoveNote = m_pDoc->GetNote(aMovePos);
    CPPUNIT_ASSERT_MESSAGE("Failed to undo the note move from destination.", !pMoveNote);
    pOrigNote = m_pDoc->GetNote(aOrigPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to undo the note move to source.", pOrigNote);

    // The caption object still should not be identical.
    // pMoveCaption is a dangling pointer.
    pOrigCaption = pOrigNote->GetOrCreateCaption(aOrigPos);
    CPPUNIT_ASSERT_MESSAGE("Captions identical after move undo.", pOrigCaption != pMoveCaption);


    // Create a note at B4, merge B4 and B5 with ScUndoMerge, and Undo.

    ScAddress aPosB4(1,3,0);
    ScPostIt* pNoteB4 = m_pDoc->GetOrCreateNote(aPosB4);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert cell comment at B4.", pNoteB4);
    const SdrCaptionObj* pCaptionB4 = pNoteB4->GetOrCreateCaption(aPosB4);
    ScCellMergeOption aCellMergeOption(1,3,2,3);
    rDocFunc.MergeCells( aCellMergeOption, true /*bContents*/, bRecord, bApi, false /*bEmptyMergedCells*/ );

    SfxUndoManager* pMergeUndoManager = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pMergeUndoManager);
    pMergeUndoManager->Undo();  // this should not crash ... tdf#105667

    // Undo contained the original caption object pointer which was still alive
    // at B4 after the merge and not cloned nor recreated during Undo.
    ScPostIt* pUndoNoteB4 = m_pDoc->GetNote(aPosB4);
    CPPUNIT_ASSERT_MESSAGE("No cell comment at B4 after Undo.", pUndoNoteB4);
    const SdrCaptionObj* pUndoCaptionB4 = pUndoNoteB4->GetCaption();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Captions not identical after Merge Undo.", pCaptionB4, pUndoCaptionB4);


    // In a second document copy a note from B5 to clipboard, close the
    // document and then paste the note into this document.
    {
        ScDocShellRef xDocSh2;
        getNewDocShell(xDocSh2);
        ScDocument* pDoc2 = &xDocSh2->GetDocument();
        pDoc2->InsertTab(0, "OtherSheet1");
        pDoc2->InitDrawLayer(xDocSh2.get());

        ScAddress aPosB5(1,4,0);
        ScPostIt* pOtherNoteB5 = pDoc2->GetOrCreateNote(aPosB5);
        CPPUNIT_ASSERT_MESSAGE("Failed to insert cell comment at B5.", pOtherNoteB5);
        const SdrCaptionObj* pOtherCaptionB5 = pOtherNoteB5->GetOrCreateCaption(aPosB5);
        CPPUNIT_ASSERT_MESSAGE("No caption at B5.", pOtherCaptionB5);

        ScDocument aClipDoc2(SCDOCMODE_CLIP);
        copyToClip( pDoc2, aPosB5, &aClipDoc2);

        // There's no ScTransferObject involved in the "fake" clipboard copy
        // and ScDocument dtor asking IsClipboardSource() gets no, so emulate
        // the part that normally is responsible for forgetting the caption
        // objects.
        aClipDoc2.ClosingClipboardSource();

        pDoc2->DeleteTab(0);
        closeDocShell(xDocSh2);

        pasteFromClip( m_pDoc, aPosB5, &aClipDoc2); // should not crash... tdf#104967
        ScPostIt* pNoteB5 = m_pDoc->GetNote(aPosB5);
        CPPUNIT_ASSERT_MESSAGE("Failed to paste cell comment at B5.", pNoteB5);
        const SdrCaptionObj* pCaptionB5 = pNoteB5->GetOrCreateCaption(aPosB5);
        CPPUNIT_ASSERT_MESSAGE("No caption at pasted B5.", pCaptionB5);
        // Do not test if  pCaptionB5 != pOtherCaptionB5  because since pDoc2
        // has been closed and the caption been deleted objects *may* be
        // allocated at the very same memory location.
    }

    m_pDoc->DeleteTab(0);
}

void Test::testNoteCopyPaste()
{
    m_pDoc->InsertTab(0, "Test");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    // Insert in B2 a text and cell comment.
    ScAddress aPos(1,1,0);
    m_pDoc->SetString(aPos, "Text");
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aPos);
    CPPUNIT_ASSERT(pNote);
    pNote->SetText(aPos, "Note1");

    // Insert in B4 a number and cell comment.
    aPos.SetRow(3);
    m_pDoc->SetValue(aPos, 1.1);
    pNote = m_pDoc->GetOrCreateNote(aPos);
    CPPUNIT_ASSERT(pNote);
    pNote->SetText(aPos, "Note2");

    // Copy B2:B4 to clipboard.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    ScRange aCopyRange(1,1,0,1,3,0);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    ScClipParam aClipParam(aCopyRange, false);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Make sure the notes are in the clipboard.
    pNote = aClipDoc.GetNote(ScAddress(1,1,0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Note1"), pNote->GetText());

    pNote = aClipDoc.GetNote(ScAddress(1,3,0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Note2"), pNote->GetText());

    // Paste to B6:B8 but only cell notes.
    ScRange aDestRange(1,5,0,1,7,0);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::NOTE, nullptr, &aClipDoc);

    // Make sure the notes are there.
    pNote = m_pDoc->GetNote(ScAddress(1,5,0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Note1"), pNote->GetText());

    pNote = m_pDoc->GetNote(ScAddress(1,7,0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(OUString("Note2"), pNote->GetText());

    // Test that GetNotesInRange includes the end of its range
    // and so can find the note
    std::vector<sc::NoteEntry> aNotes;
    m_pDoc->GetNotesInRange(ScRange(1,7,0), aNotes);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aNotes.size());

    m_pDoc->DeleteTab(0);
}

// tdf#112454
void Test::testNoteContainsNotesInRange() {
    m_pDoc->InsertTab(0, "PostIts");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    ScAddress aAddr(2, 2, 0); // cell C3

    CPPUNIT_ASSERT_MESSAGE("Claiming there's notes in a document that doesn't have any.",
                           !m_pDoc->ContainsNotesInRange((ScRange(ScAddress(0, 0, 0), aAddr))));

    m_pDoc->GetOrCreateNote(aAddr);

    CPPUNIT_ASSERT_MESSAGE("Claiming there's notes in range that doesn't have any.",
                           !m_pDoc->ContainsNotesInRange(ScRange(ScAddress(0, 0, 0), ScAddress(0, 1, 0))));
    CPPUNIT_ASSERT_MESSAGE("Note not detected that lies on border of range.",
                           m_pDoc->ContainsNotesInRange((ScRange(ScAddress(0, 0, 0), aAddr))));
    CPPUNIT_ASSERT_MESSAGE("Note not detected that lies in inner area of range.",
                           m_pDoc->ContainsNotesInRange((ScRange(ScAddress(0, 0, 0), ScAddress(3, 3, 0)))));
}

void Test::testAreasWithNotes()
{
    m_pDoc->InsertTab(0, "Sheet1");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    ScAddress rAddr(1, 5, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, "Hello");
    pNote->SetAuthor("Jim Bob");
    ScAddress rAddrMin(2, 2, 0);
    ScPostIt* pNoteMin = m_pDoc->GetOrCreateNote(rAddrMin);
    pNoteMin->SetText(rAddrMin, "Hello");

    SCCOL col;
    SCROW row;
    bool dataFound;

    // only cell notes (empty content)

    dataFound = m_pDoc->GetDataStart(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No DataStart found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong col for notes", static_cast<SCCOL>(1), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong row for notes", static_cast<SCROW>(2), row);

    dataFound = m_pDoc->GetCellArea(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No CellArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong col for notes", static_cast<SCCOL>(2), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong row for notes", static_cast<SCROW>(5), row);

    bool bNotes = true;
    dataFound = m_pDoc->GetPrintArea(0,col,row, bNotes);

    CPPUNIT_ASSERT_MESSAGE("No PrintArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong col for notes", static_cast<SCCOL>(2), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong row for notes", static_cast<SCROW>(5), row);

    bNotes = false;
    dataFound = m_pDoc->GetPrintArea(0,col,row, bNotes);
    CPPUNIT_ASSERT_MESSAGE("No PrintArea should be found", !dataFound);

    bNotes = true;
    dataFound = m_pDoc->GetPrintAreaVer(0,0,1,row, bNotes); // cols 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row for notes", static_cast<SCROW>(5), row);

    dataFound = m_pDoc->GetPrintAreaVer(0,2,3,row, bNotes); // cols 2 & 3
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row for notes", static_cast<SCROW>(2), row);

    bNotes = false;
    dataFound = m_pDoc->GetPrintAreaVer(0,0,1,row, bNotes); // col 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer should be found", !dataFound);

    // now add cells with value, check that notes are taken into account in good cases

    m_pDoc->SetString(0, 3, 0, "Some Text");
    m_pDoc->SetString(3, 3, 0, "Some Text");

    dataFound = m_pDoc->GetDataStart(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No DataStart found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong col", static_cast<SCCOL>(0), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong row", static_cast<SCROW>(2), row);

    dataFound = m_pDoc->GetCellArea(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No CellArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong col", static_cast<SCCOL>(3), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong row", static_cast<SCROW>(5), row);

    bNotes = true;
    dataFound = m_pDoc->GetPrintArea(0,col,row, bNotes);

    CPPUNIT_ASSERT_MESSAGE("No PrintArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong col", static_cast<SCCOL>(3), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong row", static_cast<SCROW>(5), row);

    bNotes = false;
    dataFound = m_pDoc->GetPrintArea(0,col,row, bNotes);
    CPPUNIT_ASSERT_MESSAGE("No PrintArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong col", static_cast<SCCOL>(3), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong row", static_cast<SCROW>(3), row);

    bNotes = true;
    dataFound = m_pDoc->GetPrintAreaVer(0,0,1,row, bNotes); // cols 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row", static_cast<SCROW>(5), row);

    dataFound = m_pDoc->GetPrintAreaVer(0,2,3,row, bNotes); // cols 2 & 3
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row", static_cast<SCROW>(3), row);

    bNotes = false;
    dataFound = m_pDoc->GetPrintAreaVer(0,0,1,row, bNotes); // cols 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row", static_cast<SCROW>(3), row);

    m_pDoc->DeleteTab(0);
}

void Test::testAnchoredRotatedShape()
{
    m_pDoc->InsertTab(0, "TestTab");
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("new sheet should have all rows visible", SCROW(MAXROW), nRow2);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != nullptr);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != nullptr);
    m_pDoc->SetRowHeightRange(0, MAXROW, 0, o3tl::toTwips(1000, o3tl::Length::mm100));
    constexpr tools::Long TOLERANCE = 30; //30 hmm
    for ( SCCOL nCol = 0; nCol < MAXCOL; ++nCol )
        m_pDoc->SetColWidth(nCol, 0, o3tl::toTwips(1000, o3tl::Length::mm100));
    {
        //Add a rect
        tools::Rectangle aRect( 4000, 5000, 10000, 7000 );

        tools::Rectangle aRotRect( 6000, 3000, 8000, 9000 );
        SdrRectObj *pObj = new SdrRectObj(*pDrawLayer, aRect);
        pPage->InsertObject(pObj);
        Point aRef1(pObj->GetSnapRect().Center());
        Degree100 nAngle = 9000_deg100; //90 deg.
        double nSin = 1.0; // sin(90 deg)
        double nCos = 0.0; // cos(90 deg)
        pObj->Rotate(aRef1,nAngle,nSin,nCos);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, true);

        tools::Rectangle aSnap = pObj->GetSnapRect();
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.GetHeight(), aSnap.GetHeight(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.GetWidth(), aSnap.GetWidth(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.Left(), aSnap.Left(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.Top(), aSnap.Top(), TOLERANCE ) );

        ScDrawObjData aAnchor;
        ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object meta-data.", pData);

        aAnchor.maStart = pData->maStart;
        aAnchor.maEnd = pData->maEnd;

        m_pDoc->SetDrawPageSize(0);

        // increase row 5 by 2000 hmm
        m_pDoc->SetRowHeight(5, 0, o3tl::toTwips(3000, o3tl::Length::mm100));
        // increase col 6 by 1000 hmm
        m_pDoc->SetColWidth(6, 0, o3tl::toTwips(2000, o3tl::Length::mm100));

        aRotRect.setWidth( aRotRect.GetWidth() + 1000 );
        aRotRect.setHeight( aRotRect.GetHeight() + 2000 );

        m_pDoc->SetDrawPageSize(0);

        aSnap = pObj->GetSnapRect();

        // ensure that width and height have been adjusted accordingly
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.GetHeight(), aSnap.GetHeight(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.GetWidth(), aSnap.GetWidth(), TOLERANCE ) );

        // ensure that anchor start and end addresses haven't changed
        CPPUNIT_ASSERT_EQUAL( aAnchor.maStart.Row(), pData->maStart.Row() ); // start row 0
        CPPUNIT_ASSERT_EQUAL( aAnchor.maStart.Col(), pData->maStart.Col() ); // start column 5
        CPPUNIT_ASSERT_EQUAL( aAnchor.maEnd.Row(), pData->maEnd.Row() ); // end row 3
        CPPUNIT_ASSERT_EQUAL( aAnchor.maEnd.Col(), pData->maEnd.Col() ); // end col 7
    }
    m_pDoc->DeleteTab(0);
}

void Test::testCellTextWidth()
{
    m_pDoc->InsertTab(0, "Test");

    ScAddress aTopCell(0, 0, 0);

    // Sheet is empty.
    std::unique_ptr<ScColumnTextWidthIterator> pIter(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
    CPPUNIT_ASSERT_MESSAGE("Column should have no text widths stored.", !pIter->hasCell());

    // Sheet only has one cell.
    m_pDoc->SetString(0, 0, 0, "Only one cell");
    pIter.reset(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
    CPPUNIT_ASSERT_MESSAGE("Column should have a cell.", pIter->hasCell());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), pIter->getPos());

    // Setting a text width here should commit it to the column.
    sal_uInt16 nTestVal = 432;
    pIter->setValue(nTestVal);
    CPPUNIT_ASSERT_EQUAL(nTestVal, m_pDoc->GetTextWidth(aTopCell));

    // Set values to row 2 through 6.
    for (SCROW i = 2; i <= 6; ++i)
        m_pDoc->SetString(0, i, 0, "foo");

    // Set values to row 10 through 18.
    for (SCROW i = 10; i <= 18; ++i)
        m_pDoc->SetString(0, i, 0, "foo");

    {
        // Full range.
        pIter.reset(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
        SCROW aRows[] = { 0, 2, 3, 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(aRows); ++i, pIter->next())
        {
            CPPUNIT_ASSERT_MESSAGE("Cell expected, but not there.", pIter->hasCell());
            CPPUNIT_ASSERT_EQUAL(aRows[i], pIter->getPos());
        }
        CPPUNIT_ASSERT_MESSAGE("Iterator should have ended.", !pIter->hasCell());
    }

    {
        // Specify start and end rows (6 - 16)
        ScAddress aStart = aTopCell;
        aStart.SetRow(6);
        pIter.reset(new ScColumnTextWidthIterator(*m_pDoc, aStart, 16));
        SCROW aRows[] = { 6, 10, 11, 12, 13, 14, 15, 16 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(aRows); ++i, pIter->next())
        {
            CPPUNIT_ASSERT_MESSAGE("Cell expected, but not there.", pIter->hasCell());
            CPPUNIT_ASSERT_EQUAL(aRows[i], pIter->getPos());
        }
        CPPUNIT_ASSERT_MESSAGE("Iterator should have ended.", !pIter->hasCell());
    }

    // Clear from row 3 to row 17. After this, we should only have cells at rows 0, 2 and 18.
    clearRange(m_pDoc, ScRange(0, 3, 0, 0, 17, 0));

    {
        // Full range again.
        pIter.reset(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
        SCROW aRows[] = { 0, 2, 18 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(aRows); ++i, pIter->next())
        {
            CPPUNIT_ASSERT_MESSAGE("Cell expected, but not there.", pIter->hasCell());
            CPPUNIT_ASSERT_EQUAL(aRows[i], pIter->getPos());
        }
        CPPUNIT_ASSERT_MESSAGE("Iterator should have ended.", !pIter->hasCell());
    }

    // Delete row 2 which shifts all cells below row 2 upward. After this, we
    // should only have cells at rows 0 and 17.
    m_pDoc->DeleteRow(0, 0, MAXCOL, MAXTAB, 2, 1);
    {
        // Full range again.
        pIter.reset(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
        SCROW aRows[] = { 0, 17 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(aRows); ++i, pIter->next())
        {
            CPPUNIT_ASSERT_MESSAGE("Cell expected, but not there.", pIter->hasCell());
            CPPUNIT_ASSERT_EQUAL(aRows[i], pIter->getPos());
        }
        CPPUNIT_ASSERT_MESSAGE("Iterator should have ended.", !pIter->hasCell());
    }

    m_pDoc->DeleteTab(0);
}

static bool checkEditTextIterator(sc::EditTextIterator& rIter, const char** pChecks)
{
    const EditTextObject* pText = rIter.first();
    const char* p = *pChecks;

    for (int i = 0; i < 100; ++i) // cap it to 100 loops.
    {
        if (!pText)
            // No more edit cells. The check string array should end too.
            return p == nullptr;

        if (!p)
            // More edit cell, but no more check string. Bad.
            return false;

        if (pText->GetParagraphCount() != 1)
            // For this test, we don't handle multi-paragraph text.
            return false;

        if (pText->GetText(0) != OUString::createFromAscii(p))
            // Text differs from what's expected.
            return false;

        pText = rIter.next();
        ++pChecks;
        p = *pChecks;
    }

    return false;
}

void Test::testEditTextIterator()
{
    m_pDoc->InsertTab(0, "Test");

    {
        // First, try with an empty sheet.
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    ScFieldEditEngine& rEditEngine = m_pDoc->GetEditEngine();

    {
        // Only set one edit cell.
        rEditEngine.SetTextCurrentDefaults("A2");
        m_pDoc->SetEditText(ScAddress(0,1,0), rEditEngine.CreateTextObject());
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { "A2", nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    {
        // Add a series of edit cells.
        rEditEngine.SetTextCurrentDefaults("A5");
        m_pDoc->SetEditText(ScAddress(0,4,0), rEditEngine.CreateTextObject());
        rEditEngine.SetTextCurrentDefaults("A6");
        m_pDoc->SetEditText(ScAddress(0,5,0), rEditEngine.CreateTextObject());
        rEditEngine.SetTextCurrentDefaults("A7");
        m_pDoc->SetEditText(ScAddress(0,6,0), rEditEngine.CreateTextObject());
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { "A2", "A5", "A6", "A7", nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    {
        // Add more edit cells to column C. Skip column B.
        rEditEngine.SetTextCurrentDefaults("C1");
        m_pDoc->SetEditText(ScAddress(2,0,0), rEditEngine.CreateTextObject());
        rEditEngine.SetTextCurrentDefaults("C3");
        m_pDoc->SetEditText(ScAddress(2,2,0), rEditEngine.CreateTextObject());
        rEditEngine.SetTextCurrentDefaults("C4");
        m_pDoc->SetEditText(ScAddress(2,3,0), rEditEngine.CreateTextObject());
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { "A2", "A5", "A6", "A7", "C1", "C3", "C4", nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    {
        // Add some numeric, string and formula cells.  This shouldn't affect the outcome.
        m_pDoc->SetString(ScAddress(0,99,0), "=ROW()");
        m_pDoc->SetValue(ScAddress(1,3,0), 1.2);
        m_pDoc->SetString(ScAddress(2,4,0), "Simple string");
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { "A2", "A5", "A6", "A7", "C1", "C3", "C4", nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testImportStream()
{
    sc::AutoCalcSwitch aAC(*m_pDoc, true); // turn on auto calc.
    sc::UndoSwitch aUndo(*m_pDoc, true); // enable undo.

    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,1,0), "=SUM(A1:C1)"); // A2

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // CSV import options.
    ScAsciiOptions aOpt;
    aOpt.SetFieldSeps(",");

    // Import values to A1:C1.
    ScImportExport aObj(*m_pDoc, ScAddress(0,0,0));
    aObj.SetImportBroadcast(true);
    aObj.SetExtOptions(aOpt);
    aObj.ImportString("1,2,3", SotClipboardFormatId::STRING);

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    // Formula value should have been updated.
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Undo, and check the result.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT_MESSAGE("Failed to get the undo manager.", pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(0,1,0))); // formula

    // Redo, and check the result.
    pUndoMgr->Redo();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(2,0,0)));

    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(0,1,0))); // formula

    pUndoMgr->Clear();

    m_pDoc->DeleteTab(0);
}

void Test::testDeleteContents()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.
    sc::UndoSwitch aUndoSwitch(*m_pDoc, true); // enable undo.

    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(ScAddress(3,1,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,2,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,3,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,4,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,5,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,6,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,7,0), 1.0);
    m_pDoc->SetValue(ScAddress(3,8,0), 1.0);
    m_pDoc->SetString(ScAddress(3,15,0), "=SUM(D2:D15)");

    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(3,15,0))); // formula

    // Delete D2:D6.
    ScRange aRange(3,1,0,3,5,0);
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    aMark.SetMarkArea(aRange);

    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(*m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aRange, InsertDeleteFlags::CONTENTS, false, *pUndoDoc, &aMark);
    ScUndoDeleteContents aUndo(m_xDocShell.get(), aMark, aRange, std::move(pUndoDoc), false, InsertDeleteFlags::CONTENTS, true);

    clearRange(m_pDoc, aRange);
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(3,15,0))); // formula

    aUndo.Undo();
    CPPUNIT_ASSERT_EQUAL(8.0, m_pDoc->GetValue(ScAddress(3,15,0))); // formula

    aUndo.Redo();
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(3,15,0))); // formula

    m_pDoc->DeleteTab(0);
}

void Test::testTransliterateText()
{
    m_pDoc->InsertTab(0, "Test");

    // Set texts to A1:A3.
    m_pDoc->SetString(ScAddress(0,0,0), "Mike");
    m_pDoc->SetString(ScAddress(0,1,0), "Noah");
    m_pDoc->SetString(ScAddress(0,2,0), "Oscar");

    // Change them to uppercase.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SetMarkArea(ScRange(0,0,0,0,2,0));
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    rFunc.TransliterateText(
        aMark, TransliterationFlags::LOWERCASE_UPPERCASE, true);

    CPPUNIT_ASSERT_EQUAL(OUString("MIKE"), m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("NOAH"), m_pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("OSCAR"), m_pDoc->GetString(ScAddress(0,2,0)));

    // Test the undo and redo.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT_MESSAGE("Failed to get undo manager.", pUndoMgr);

    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("Mike"), m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Noah"), m_pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Oscar"), m_pDoc->GetString(ScAddress(0,2,0)));

    pUndoMgr->Redo();
    CPPUNIT_ASSERT_EQUAL(OUString("MIKE"), m_pDoc->GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("NOAH"), m_pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("OSCAR"), m_pDoc->GetString(ScAddress(0,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaToValue()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, "Test");

    std::vector<std::vector<const char*>> aData = {
        { "=1", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=2", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=3", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=4", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=5", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=6", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
    };

    ScAddress aPos(1,2,0); // B3
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1",  "2", "TRUE" },
            { "2",  "4", "TRUE" },
            { "3",  "6", "TRUE" },
            { "4",  "8", "TRUE" },
            { "5", "10", "TRUE" },
            { "6", "12", "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Convert B5:C6 to static values, and check the result.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScRange aConvRange(1,4,0,2,5,0); // B5:C6
    rFunc.ConvertFormulaToValue(aConvRange, false);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1",  "2",  "TRUE" },
            { "2",  "4",  "TRUE" },
            { "3",  "6", "FALSE" },
            { "4",  "8", "FALSE" },
            { "5", "10",  "TRUE" },
            { "6", "12",  "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Converted");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Make sure that B3:B4 and B7:B8 are formula cells.
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,6,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,7,0)));

    // Make sure that B5:C6 are numeric cells.
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(2,4,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(2,5,0)));

    // Make sure that formula cells in C3:C4 and C7:C8 are grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(2,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    pFC = m_pDoc->GetFormulaCell(ScAddress(2,6,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1",  "2", "TRUE" },
            { "2",  "4", "TRUE" },
            { "3",  "6", "TRUE" },
            { "4",  "8", "TRUE" },
            { "5", "10", "TRUE" },
            { "6", "12", "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "After undo");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // B3:B8 should all be (ungrouped) formula cells.
    for (SCROW i = 2; i <= 7; ++i)
    {
        pFC = m_pDoc->GetFormulaCell(ScAddress(1,i,0));
        CPPUNIT_ASSERT(pFC);
        CPPUNIT_ASSERT(!pFC->IsShared());
    }

    // C3:C8 should be shared formula cells.
    pFC = m_pDoc->GetFormulaCell(ScAddress(2,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());

    // Redo and check.
    pUndoMgr->Redo();
    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1",  "2",  "TRUE" },
            { "2",  "4",  "TRUE" },
            { "3",  "6", "FALSE" },
            { "4",  "8", "FALSE" },
            { "5", "10",  "TRUE" },
            { "6", "12",  "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Converted");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Make sure that B3:B4 and B7:B8 are formula cells.
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,3,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,6,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(ScAddress(1,7,0)));

    // Make sure that B5:C6 are numeric cells.
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(1,4,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(1,5,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(2,4,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(ScAddress(2,5,0)));

    // Make sure that formula cells in C3:C4 and C7:C8 are grouped.
    pFC = m_pDoc->GetFormulaCell(ScAddress(2,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());
    pFC = m_pDoc->GetFormulaCell(ScAddress(2,6,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(2), pFC->GetSharedLength());

    // Undo again and make sure the recovered formulas in C5:C6 still track B5:B6.
    pUndoMgr->Undo();
    m_pDoc->SetValue(ScAddress(1,4,0), 10);
    m_pDoc->SetValue(ScAddress(1,5,0), 11);
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(ScAddress(2,4,0)));
    CPPUNIT_ASSERT_EQUAL(22.0, m_pDoc->GetValue(ScAddress(2,5,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaToValue2()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    m_pDoc->InsertTab(0, "Test");

    std::vector<std::vector<const char*>> aData = {
        { "=1", "=ISFORMULA(RC[-1])" },
        { "=2", "=ISFORMULA(RC[-1])" },
        {  "3", "=ISFORMULA(RC[-1])" },
        { "=4", "=ISFORMULA(RC[-1])" },
        { "=5", "=ISFORMULA(RC[-1])" },
    };

    // Insert data into B2:C6.
    ScAddress aPos(1,1,0); // B2
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1", "TRUE" },
            { "2", "TRUE" },
            { "3", "FALSE" },
            { "4", "TRUE" },
            { "5", "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Convert B3:B5 to a value.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScRange aConvRange(1,2,0,1,4,0); // B3:B5
    rFunc.ConvertFormulaToValue(aConvRange, false);

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1", "TRUE" },
            { "2", "FALSE" },
            { "3", "FALSE" },
            { "4", "FALSE" },
            { "5", "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    {
        // Expected output table content.  0 = empty cell
        std::vector<std::vector<const char*>> aOutputCheck = {
            { "1", "TRUE" },
            { "2", "TRUE" },
            { "3", "FALSE" },
            { "4", "TRUE" },
            { "5", "TRUE" },
        };

        bool bSuccess = checkOutput(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testColumnFindEditCells()
{
    m_pDoc->InsertTab(0, "Test");

    // Test the basics with real edit cells, using Column A.

    SCROW nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,MAXROW,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no edit cells.", SCROW(-1), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no edit cells.", SCROW(-1), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScRange(0,0,0,0,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no edit cells.", SCROW(-1), nResRow);

    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetTextCurrentDefaults("Test");
    m_pDoc->SetEditText(ScAddress(0,0,0), rEE.CreateTextObject());
    const EditTextObject* pObj = m_pDoc->GetEditText(ScAddress(0,0,0));
    CPPUNIT_ASSERT_MESSAGE("There should be an edit cell here.", pObj);

    ScRange aRange(0,0,0,0,0,0);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There is an edit cell here.", SCROW(0), nResRow);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be an edit cell in specified range.", SCROW(-1), nResRow);

    aRange.aStart.SetRow(2);
    aRange.aEnd.SetRow(4);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be an edit cell in specified range.", SCROW(-1), nResRow);

    aRange.aStart.SetRow(0);
    aRange.aEnd.SetRow(MAXROW);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be an edit cell in specified range.", SCROW(0), nResRow);

    m_pDoc->SetString(ScAddress(0,0,0), "Test");
    m_pDoc->SetValue(ScAddress(0,2,0), 1.0);
    ScRefCellValue aCell;
    aCell.assign(*m_pDoc, ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a string cell.", CELLTYPE_STRING, aCell.meType);
    aCell.assign(*m_pDoc, ScAddress(0,1,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be an empty cell.", CELLTYPE_NONE, aCell.meType);
    aCell.assign(*m_pDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be a numeric cell.", CELLTYPE_VALUE, aCell.meType);
    aCell.assign(*m_pDoc, ScAddress(0,3,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This should be an empty cell.", CELLTYPE_NONE, aCell.meType);

    aRange.aStart.SetRow(1);
    aRange.aEnd.SetRow(1);
    nResRow = m_pDoc->GetFirstEditTextRow(aRange);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be an edit cell in specified range.", SCROW(-1), nResRow);

    // Test with non-edit cell but with ambiguous script type.

    m_pDoc->SetString(ScAddress(1,11,0), "Some text");
    m_pDoc->SetString(ScAddress(1,12,0), "Some text");
    m_pDoc->SetString(ScAddress(1,13,0), "Other text");

    m_pDoc->SetScriptType(ScAddress(1,11,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));
    m_pDoc->SetScriptType(ScAddress(1,12,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));
    m_pDoc->SetScriptType(ScAddress(1,13,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));

    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(1,11,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(11), nResRow);
    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(1,12,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(12), nResRow);

    for (SCROW i = 0; i <= 5; ++i)
        m_pDoc->SetString(ScAddress(2,i,0), "Text");

    m_pDoc->SetScriptType(ScAddress(2,5,0), (SvtScriptType::LATIN | SvtScriptType::ASIAN));

    nResRow = m_pDoc->GetFirstEditTextRow(ScAddress(2,1,0));
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(-1), nResRow);

    m_pDoc->DeleteTab(0);
}


void Test::testSetFormula()
{
    m_pDoc->InsertTab(0, "Test");

    static struct aInputs
    {
        SCROW nRow;
        SCCOL nCol;
        const char* aFormula1;      // Represents the formula that is input to SetFormula function.
        const char* aFormula2;      // Represents the formula that is actually stored in the cell.
        formula::FormulaGrammar::Grammar const eGram;

    } const aTest[] = {
        { 5 , 4 , "=SUM($D$2:$F$3)"             ,"=SUM($D$2:$F$3)" , formula::FormulaGrammar::Grammar::GRAM_ENGLISH     },
        { 5 , 5 , "=A1-$C2+B$3-$F$4"            ,"=A1-$C2+B$3-$F$4", formula::FormulaGrammar::Grammar::GRAM_NATIVE      },
        { 6 , 6 , "=A1-$C2+B$3-$F$4"            ,"=A1-$C2+B$3-$F$4", formula::FormulaGrammar::Grammar::GRAM_NATIVE_XL_A1},
        { 7 , 8 , "=[.A1]-[.$C2]+[.G$3]-[.$F$4]","=A1-$C2+G$3-$F$4", formula::FormulaGrammar::Grammar::GRAM_ODFF        }
    };

    for(size_t i = 0; i < SAL_N_ELEMENTS(aTest); ++i)
    {
        m_pDoc->SetFormula(ScAddress(aTest[i].nCol, aTest[i].nRow, 0), OUString::createFromAscii(aTest[i].aFormula1), aTest[i].eGram);
        OUString aBuffer = m_pDoc->GetFormula(aTest[i].nCol, aTest[i].nRow, 0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to set formula", OUString::createFromAscii(aTest[i].aFormula2), aBuffer);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testMultipleDataCellsInRange()
{
    m_pDoc->InsertTab(0, "Test");

    ScRange aRange(1,2,0); // B3
    sc::MultiDataCellState aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::Empty, aState.meState);

    // Set a numeric value to B3.
    m_pDoc->SetValue(ScAddress(1,2,0), 1.0);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasOneCell, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), aState.mnRow1);

    // Set another numeric value to B4.
    m_pDoc->SetValue(ScAddress(1,3,0), 2.0);
    aRange.aEnd.SetRow(3); // B3:B4
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasMultipleCells, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), aState.mnRow1);

    // Set the query range to B4:B5.  Now it should only report one cell, with
    // B4 being the first non-empty cell.
    aRange.aStart.SetRow(3);
    aRange.aEnd.SetRow(4);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasOneCell, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(3), aState.mnRow1);

    // Set the query range to A1:C3.  The first non-empty cell should be B3.
    aRange = ScRange(0,0,0,2,2,0);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasOneCell, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(2), aState.mnRow1);

    // Set string cells to D4 and F5, and query D3:F5.  D4 should be the first
    // non-empty cell.
    m_pDoc->SetString(ScAddress(3,3,0), "foo");
    m_pDoc->SetString(ScAddress(5,4,0), "bar");
    aRange = ScRange(3,2,0,5,4,0);
    aState = m_pDoc->HasMultipleDataCells(aRange);
    CPPUNIT_ASSERT_EQUAL(sc::MultiDataCellState::HasMultipleCells, aState.meState);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), aState.mnCol1);
    CPPUNIT_ASSERT_EQUAL(SCROW(3), aState.mnRow1);

    // TODO : add more test cases as needed.

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaWizardSubformula()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(1,0,0), "=1");          // B1
    m_pDoc->SetString(ScAddress(1,1,0), "=1/0");        // B2
    m_pDoc->SetString(ScAddress(1,2,0), "=gibberish");  // B3

    ScSimpleFormulaCalculator aFCell1( *m_pDoc, ScAddress(0,0,0), "=B1:B3", true );
    FormulaError nErrCode = aFCell1.GetErrCode();
    CPPUNIT_ASSERT( nErrCode == FormulaError::NONE || aFCell1.IsMatrix() );
    CPPUNIT_ASSERT_EQUAL( OUString("{1|#DIV/0!|#NAME?}"), aFCell1.GetString().getString() );

    m_pDoc->SetString(ScAddress(1,0,0), "=NA()");       // B1
    m_pDoc->SetString(ScAddress(1,1,0), "2");           // B2
    m_pDoc->SetString(ScAddress(1,2,0), "=1+2");        // B3
    ScSimpleFormulaCalculator aFCell2( *m_pDoc, ScAddress(0,0,0), "=B1:B3", true );
    nErrCode = aFCell2.GetErrCode();
    CPPUNIT_ASSERT( nErrCode == FormulaError::NONE || aFCell2.IsMatrix() );
    CPPUNIT_ASSERT_EQUAL( OUString("{#N/A|2|3}"), aFCell2.GetString().getString() );

    m_pDoc->DeleteTab(0);
}

void Test::testDiagonalBorders()
{
    m_pDoc->InsertTab(0, "Diagonal");

    ScAddress aPos;
    const editeng::SvxBorderLine* pLine;
    const ScPatternAttr* pPat;

    // diagonal down border
    ::editeng::SvxBorderLine dDownBorderLine(nullptr, 1);
    SvxLineItem dDownLineItem(ATTR_BORDER_TLBR);
    dDownLineItem.SetLine(&dDownBorderLine);

    // set diagonal down border to cell(A1)
    m_pDoc->ApplyAttr(0, 0, 0, dDownLineItem);

    aPos = { 0, 0, 0 };
    pPat = m_pDoc->GetPattern(aPos);
    CPPUNIT_ASSERT(pPat);

    pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
    CPPUNIT_ASSERT_MESSAGE("Diagonal down border was expected, but not found!", pLine);

    // diagonal up border
    ::editeng::SvxBorderLine dUpBorderLine(nullptr, 1);
    SvxLineItem dUpLineItem(ATTR_BORDER_BLTR);
    dUpLineItem.SetLine(&dUpBorderLine);

    // set diagonal up border to cell(A2)
    m_pDoc->ApplyAttr(0, 1, 0, dUpLineItem);

    aPos = { 0, 1, 0 };
    pPat = m_pDoc->GetPattern(aPos);
    CPPUNIT_ASSERT(pPat);

    pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
    CPPUNIT_ASSERT_MESSAGE("Diagonal up border was expected, but not found!", pLine);

    // diagonal down and up border in the same cell (A5)
    m_pDoc->ApplyAttr(0, 4, 0, dDownLineItem);
    m_pDoc->ApplyAttr(0, 4, 0, dUpLineItem);

    // test if both borders are applied successfully in the same cell (A5)
    aPos = { 0, 4, 0 };
    pPat = m_pDoc->GetPattern(aPos);
    CPPUNIT_ASSERT(pPat);

    pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
    CPPUNIT_ASSERT_MESSAGE("Diagonal down border was expected, but not found!", pLine);
    pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
    CPPUNIT_ASSERT_MESSAGE("Diagonal up border was expected, but not found!", pLine);

    // test if both borders are removed successfully
    dDownLineItem.SetLine(nullptr);
    dUpLineItem.SetLine(nullptr);

    // SetLine(nullptr) should remove the lines from (A5)
    m_pDoc->ApplyAttr(0, 4, 0, dDownLineItem);
    m_pDoc->ApplyAttr(0, 4, 0, dUpLineItem);

    pPat = m_pDoc->GetPattern(aPos);
    CPPUNIT_ASSERT(pPat);

    pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
    CPPUNIT_ASSERT_MESSAGE("Diagonal down border was not expected, but is found!", !pLine);
    pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
    CPPUNIT_ASSERT_MESSAGE("Diagonal up border was not expected, but is found!", !pLine);

    m_pDoc->DeleteTab(0);
}

void Test::testSetStringAndNote()
{
    m_pDoc->InsertTab(0, "Test");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(m_xDocShell.get());

    //note on A1
    ScAddress aAdrA1 (0, 0, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aAdrA1);
    pNote->SetText(aAdrA1, "Hello world in A1");

    m_pDoc->SetString(0, 0, 0, "");

    pNote = m_pDoc->GetNote(aAdrA1);
    CPPUNIT_ASSERT(pNote);

    m_pDoc->DeleteTab(0);
}

void Test::testUndoDataAnchor()
{
    m_pDoc->InsertTab(0, "Tab1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be only 1 sheets to begin with",
                               static_cast<SCTAB>(1), m_pDoc->GetTableCount());

    m_pDoc->InitDrawLayer();
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);

    // Insert an object.
    tools::Rectangle aObjRect(2,1000,100,1100);
    SdrObject* pObj = new SdrRectObj(*pDrawLayer, aObjRect);
    pPage->InsertObject(pObj);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0, false);

    // Get anchor data
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);

    ScAddress aOldStart = pData->maStart;
    ScAddress aOldEnd   = pData->maEnd;

    // Get non rotated anchor data
    ScDrawObjData* pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);

    ScAddress aNOldStart = pNData->maStart;
    ScAddress aNOldEnd   = pNData->maEnd;
    CPPUNIT_ASSERT_EQUAL(aOldStart, aNOldStart);
    CPPUNIT_ASSERT_EQUAL(aOldEnd, aNOldEnd);

    //pDrawLayer->BeginCalcUndo(false);
    // Insert a new row at row 3.
    ScDocFunc& rFunc = m_xDocShell->GetDocFunc();
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    rFunc.InsertCells(ScRange( 0, aOldStart.Row() - 1, 0, MAXCOL, aOldStart.Row(), 0 ), &aMark, INS_INSROWS_BEFORE, true, true);

    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);

    ScAddress aNewStart = pData->maStart;
    ScAddress aNewEnd   = pData->maEnd;

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);

    ScAddress aNNewStart = pNData->maStart;
    ScAddress aNNewEnd   = pNData->maEnd;
    CPPUNIT_ASSERT_EQUAL(aNewStart, aNNewStart);
    CPPUNIT_ASSERT_EQUAL(aNewEnd, aNNewEnd);
    CPPUNIT_ASSERT_MESSAGE("Failed to compare Address.", aNewStart  != aOldStart );
    CPPUNIT_ASSERT_MESSAGE("Failed to compare Address.", aNewEnd  != aOldEnd );
    CPPUNIT_ASSERT_MESSAGE("Failed to compare Address.", aNNewStart != aNOldStart );
    CPPUNIT_ASSERT_MESSAGE("Failed to compare Address.", aNNewEnd != aNOldEnd );

    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    // Check state
    ScAnchorType oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Failed to check state SCA_CELL.", SCA_CELL, oldType);

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);

    // Check if data has moved to new rows
    CPPUNIT_ASSERT_EQUAL(pData->maStart, aOldStart);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd, aOldEnd);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart, aNOldStart);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd, aNOldEnd);

    pUndoMgr->Redo();

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);

    // Check if data has moved to new rows
    CPPUNIT_ASSERT_EQUAL(pData->maStart, aNewStart);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd, aNewEnd);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart, aNNewStart);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd, aNNewEnd);

    m_pDoc->DeleteTab(0);
}


void Test::testEmptyCalcDocDefaults()
{
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), m_pDoc->GetCellCount() );
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), m_pDoc->GetFormulaGroupCount() );
    CPPUNIT_ASSERT_EQUAL( sal_uLong(0), m_pDoc->GetCodeCount() );
    CPPUNIT_ASSERT_EQUAL( int(CharCompressType::NONE), static_cast<int>(m_pDoc->GetAsianCompression()) );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasPrintRange() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInVBAMode() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasNotes() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsCutMode() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsEmbedFonts() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsEmbedUsedFontsOnly() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsEmbedFontScriptLatin() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsEmbedFontScriptAsian() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsEmbedFontScriptComplex() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsEmbedded() );

    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsDocEditable() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsDocProtected() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsDocVisible() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsUserInteractionEnabled() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasAnyCalcNotification() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsAutoCalcShellDisabled() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsForcedFormulaPending() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsCalculatingFormulaTree() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsClipOrUndo() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsClipboard() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsUndo() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsUndoEnabled() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsCutMode() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsClipboardSource() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInsertingFromOtherDoc() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->PastingDrawFromOtherDoc() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsAdjustHeightLocked() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsExecuteLinkEnabled() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsChangeReadOnlyEnabled() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IdleCalcTextWidth() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsIdleEnabled() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsDetectiveDirty() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasLinkFormulaNeedingCheck() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsChartListenerCollectionNeedsUpdate() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasRangeOverflow() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsImportingXML() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsCalcingAfterLoad() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->GetNoListening() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsValidAsianCompression() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->GetAsianKerning() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsValidAsianKerning() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInInterpreter() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInInterpreterTableOp() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInDtorClear() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsExpandRefs() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInLinkUpdate() );

    SCTAB tab = m_pDoc->GetVisibleTab();

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsVisible(tab) );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsDefaultTabBgColor(tab) );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasTable(tab) );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsActiveScenario(tab) );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasCalcNotification(tab) );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasManualBreaks(tab) );
}

void Test::checkPrecisionAsShown( OUString& rCode, double fValue, double fExpectedRoundVal )
{
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    sal_uInt32 nFormat = pFormatter->GetEntryKey( rCode );
    if ( nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        sal_Int32 nCheckPos = 0;
        SvNumFormatType nType;
        pFormatter->PutEntry( rCode, nCheckPos, nType, nFormat );
        CPPUNIT_ASSERT_EQUAL( sal_Int32(0), nCheckPos );
    }
    double fRoundValue = m_pDoc->RoundValueAsShown( fValue, nFormat );
    OString aMessage = "Format \"" +
        OUStringToOString( rCode, RTL_TEXTENCODING_ASCII_US ) +
        "\" is not correctly rounded";
    CPPUNIT_ASSERT_EQUAL_MESSAGE( aMessage.getStr(), fExpectedRoundVal, fRoundValue );
}

void Test::testPrecisionAsShown()
{
    m_pDoc->InsertTab(0, "Test");

    // Turn on "precision as shown" option.
    setCalcAsShown( m_pDoc, true);

    OUString aCode;
    double fValue, fExpectedRoundVal;
    {   // decimal rounding
        aCode = "0.00";
        fValue = 1.0/3.0;
        fExpectedRoundVal = 0.33;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 10.001;
        fExpectedRoundVal = 10.0;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
    }
    {   // thousand rounding tdf#106253
        aCode = "0,,";
        fValue = 4.0e9 / 7.0;
        fExpectedRoundVal = 571e6;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        aCode = "\"k\"[$$-409]* #,;[RED]-\"k\"[$$-409]* #,";
        fValue = 4.0e8 / 7.0;
        fExpectedRoundVal = 57.143e6;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
    }
    {   // percent rounding
        aCode = "0.00%";
        fValue = 4.0 / 7.0;
        fExpectedRoundVal = 0.5714;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 40.0 / 7.0;
        fExpectedRoundVal = 5.7143;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
    }
    {   // scientific rounding
        aCode = "0.00E0";
        fValue = 400000.0 / 7.0;
        fExpectedRoundVal = 57100.0;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 70000.0;
        fExpectedRoundVal = 5.71e-5;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        // engineering rounding tdf#106252
        aCode = "##0.000E0";
        fValue = 400000.0 / 7.0;
        fExpectedRoundVal = 57.143e3;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4000000.0 / 7.0;
        fExpectedRoundVal = 571.429e3;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 40000000.0 / 7.0;
        fExpectedRoundVal = 5.714e6;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 70000.0;
        fExpectedRoundVal = 57.143e-6;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 7000.0;
        fExpectedRoundVal = 571.429e-6;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 700.0;
        fExpectedRoundVal = 5.714e-3;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        aCode = "##?0.0#E0";
        fValue = 400000.0 / 7.0;
        fExpectedRoundVal = 5.71e4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4000000.0 / 7.0;
        fExpectedRoundVal = 57.14e4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 40000000.0 / 7.0;
        fExpectedRoundVal = 571.43e4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 400000000.0 / 7.0;
        fExpectedRoundVal = 5714.29e4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 70000.0;
        fExpectedRoundVal = 5714.29e-8;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 7000.0;
        fExpectedRoundVal = 5.71e-4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 700.0;
        fExpectedRoundVal = 57.14e-4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
        fValue = 4.0 / 70.0;
        fExpectedRoundVal = 571.43e-4;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
    }
    {   // fraction rounding tdf#105657
        aCode = "# ?/?";
        fValue = 0.35;
        fExpectedRoundVal = 1.0/3.0;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
    }
    {   // exact fraction
        aCode = "# ?/??";
        fValue = 0.35;
        fExpectedRoundVal = 0.35;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        checkPrecisionAsShown( aCode, -fValue, -fExpectedRoundVal );
    }
    {   // several sub-formats tdf#106052
        aCode = "0.00;-0.000";
        fValue = 1.0/3.0;
        fExpectedRoundVal = 0.33;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
        fValue = -1.0/3.0;
        fExpectedRoundVal = -0.333;
        checkPrecisionAsShown( aCode,  fValue,  fExpectedRoundVal );
    }

    setCalcAsShown( m_pDoc, false);
    m_pDoc->DeleteTab(0);
}

void Test::testProtectedSheetEditByRow()
{
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    m_pDoc->InsertTab(0, "Protected");

    {
        // Remove protected flags from rows 2-5.
        ScPatternAttr aAttr(m_pDoc->GetPool());
        aAttr.GetItemSet().Put(ScProtectionAttr(false));
        m_pDoc->ApplyPatternAreaTab(0, 1, MAXCOL, 4, 0, aAttr);

        // Protect the sheet without any options.
        ScTableProtection aProtect;
        aProtect.setProtected(true);
        m_pDoc->SetTabProtection(0, &aProtect);

        // Try to delete row 3.  It should fail.
        ScRange aRow3(0,2,0,MAXCOL,2,0);
        ScMarkData aMark(m_pDoc->GetSheetLimits());
        aMark.SelectOneTable(0);
        bool bDeleted = rDocFunc.DeleteCells(aRow3, &aMark, DelCellCmd::Rows, true);
        CPPUNIT_ASSERT_MESSAGE("deletion of row 3 should fail.", !bDeleted);

        // Protect the sheet but allow row deletion.
        aProtect.setOption(ScTableProtection::DELETE_ROWS, true);
        m_pDoc->SetTabProtection(0, &aProtect);

        // Now we should be able to delete row 3.
        bDeleted = rDocFunc.DeleteCells(aRow3, &aMark, DelCellCmd::Rows, true);
        CPPUNIT_ASSERT_MESSAGE("deletion of row 3 should succeed.", bDeleted);

        // But, row deletion should still fail on a protected row.
        ScRange aRow10(0,9,0,MAXCOL,9,0);
        bDeleted = rDocFunc.DeleteCells(aRow10, &aMark, DelCellCmd::Rows, true);
        CPPUNIT_ASSERT_MESSAGE("deletion of row 10 should not be allowed.", !bDeleted);

        // Try inserting a new row.  It should fail.
        bool bInserted = rDocFunc.InsertCells(aRow3, &aMark, INS_INSROWS_AFTER, true, true);
        CPPUNIT_ASSERT_MESSAGE("row insertion at row 3 should fail.", !bInserted);

        // Allow row insertions.
        aProtect.setOption(ScTableProtection::INSERT_ROWS, true);
        m_pDoc->SetTabProtection(0, &aProtect);

        bInserted = rDocFunc.InsertCells(aRow3, &aMark, INS_INSROWS_AFTER, true, true);
        CPPUNIT_ASSERT_MESSAGE("row insertion at row 3 should succeed.", bInserted);

        // Row insertion is allowed even when the rows above and below have protected flags set.
        bInserted = rDocFunc.InsertCells(aRow10, &aMark, INS_INSROWS_AFTER, true, true);
        CPPUNIT_ASSERT_MESSAGE("row insertion at row 10 should succeed.", bInserted);
    }

    m_pDoc->InsertTab(1, "Matrix"); // This sheet is unprotected.

    {
        // Insert matrix into B2:C3.
        ScMarkData aMark(m_pDoc->GetSheetLimits());
        aMark.SelectOneTable(1);
        m_pDoc->InsertMatrixFormula(1, 1, 2, 2, aMark, "={1;2|3;4}");

        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,1,1)));
        CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,1)));
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,1)));
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2,2,1)));

        // Try to insert a row at row 3.  It should fail because of matrix's presence.

        ScRange aRow3(0,2,1,MAXCOL,2,1);
        bool bInserted = rDocFunc.InsertCells(aRow3, &aMark, INS_INSROWS_BEFORE, true, true);
        CPPUNIT_ASSERT_MESSAGE("row insertion at row 3 should fail.", !bInserted);
    }

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testProtectedSheetEditByColumn()
{
    ScDocFunc& rDocFunc = m_xDocShell->GetDocFunc();
    m_pDoc->InsertTab(0, "Protected");

    {
        // Remove protected flags from columns B to E.
        ScPatternAttr aAttr(m_pDoc->GetPool());
        aAttr.GetItemSet().Put(ScProtectionAttr(false));
        m_pDoc->ApplyPatternAreaTab(1, 0, 4, MAXROW, 0, aAttr);

        // Protect the sheet without any options.
        ScTableProtection aProtect;
        aProtect.setProtected(true);
        m_pDoc->SetTabProtection(0, &aProtect);

        // Try to delete column C.  It should fail.
        ScRange aCol3(2,0,0,2,MAXROW,0);
        ScMarkData aMark(m_pDoc->GetSheetLimits());
        aMark.SelectOneTable(0);
        bool bDeleted = rDocFunc.DeleteCells(aCol3, &aMark, DelCellCmd::Cols, true);
        CPPUNIT_ASSERT_MESSAGE("deletion of column 3 should fail.", !bDeleted);

        // Protect the sheet but allow column deletion.
        aProtect.setOption(ScTableProtection::DELETE_COLUMNS, true);
        m_pDoc->SetTabProtection(0, &aProtect);

        // Now we should be able to delete column C.
        bDeleted = rDocFunc.DeleteCells(aCol3, &aMark, DelCellCmd::Cols, true);
        CPPUNIT_ASSERT_MESSAGE("deletion of column 3 should succeed.", bDeleted);

        // But, column deletion should still fail on a protected column.
        ScRange aCol10(9,0,0,9,MAXROW,0);
        bDeleted = rDocFunc.DeleteCells(aCol10, &aMark, DelCellCmd::Cols, true);
        CPPUNIT_ASSERT_MESSAGE("deletion of column 10 should not be allowed.", !bDeleted);

        // Try inserting a new column.  It should fail.
        bool bInserted = rDocFunc.InsertCells(aCol3, &aMark, INS_INSCOLS_AFTER, true, true);
        CPPUNIT_ASSERT_MESSAGE("column insertion at column 3 should fail.", !bInserted);

        // Allow column insertions.
        aProtect.setOption(ScTableProtection::INSERT_COLUMNS, true);
        m_pDoc->SetTabProtection(0, &aProtect);

        bInserted = rDocFunc.InsertCells(aCol3, &aMark, INS_INSCOLS_AFTER, true, true);
        CPPUNIT_ASSERT_MESSAGE("column insertion at column 3 should succeed.", bInserted);

        // Column insertion is allowed even when the columns above and below have protected flags set.
        bInserted = rDocFunc.InsertCells(aCol10, &aMark, INS_INSCOLS_AFTER, true, true);
        CPPUNIT_ASSERT_MESSAGE("column insertion at column 10 should succeed.", bInserted);
    }

    m_pDoc->InsertTab(1, "Matrix"); // This sheet is unprotected.

    {
        // Insert matrix into B2:C3.
        ScMarkData aMark(m_pDoc->GetSheetLimits());
        aMark.SelectOneTable(1);
        m_pDoc->InsertMatrixFormula(1, 1, 2, 2, aMark, "={1;2|3;4}");

        CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,1,1)));
        CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(2,1,1)));
        CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1,2,1)));
        CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2,2,1)));

        // Try to insert a column at column C.  It should fail because of matrix's presence.

        ScRange aCol3(2,0,1,2,MAXROW,1);
        bool bInserted = rDocFunc.InsertCells(aCol3, &aMark, INS_INSCOLS_BEFORE, true, true);
        CPPUNIT_ASSERT_MESSAGE("column insertion at column C should fail.", !bInserted);
    }

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
