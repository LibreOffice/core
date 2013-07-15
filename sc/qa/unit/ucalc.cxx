/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/time.h>

#include "scdll.hxx"
#include "formulacell.hxx"
#include "stringutil.hxx"
#include "scmatrix.hxx"
#include "drwlayer.hxx"
#include "scitems.hxx"
#include "reffind.hxx"
#include "markdata.hxx"
#include "clipparam.hxx"
#include "refundo.hxx"
#include "undoblk.hxx"
#include "undotab.hxx"
#include "queryentry.hxx"
#include "postit.hxx"
#include "attrib.hxx"
#include "dbdata.hxx"
#include "reftokenhelper.hxx"
#include "userdat.hxx"

#include "docsh.hxx"
#include "docfunc.hxx"
#include "dbdocfun.hxx"
#include "funcdesc.hxx"
#include "externalrefmgr.hxx"

#include "calcconfig.hxx"
#include "interpre.hxx"
#include "columniterator.hxx"
#include "types.hxx"
#include "conditio.hxx"
#include "globstr.hrc"
#include "tokenarray.hxx"
#include "scopetools.hxx"
#include "dociter.hxx"
#include "docsh.hxx"
#include "queryparam.hxx"

#include "formula/IFunctionDescription.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <editeng/boxitem.hxx>

#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include "svl/srchitem.hxx"

#include <sfx2/docfile.hxx>

#include <iostream>
#include <sstream>
#include <vector>

#include "ucalc.hxx"

struct TestImpl
{
    ScDocShellRef m_xDocShell;
};

FormulaGrammarSwitch::FormulaGrammarSwitch(ScDocument* pDoc, formula::FormulaGrammar::Grammar eGrammar) :
    mpDoc(pDoc), meOldGrammar(pDoc->GetGrammar())
{
    mpDoc->SetGrammar(eGrammar);
}

FormulaGrammarSwitch::~FormulaGrammarSwitch()
{
    mpDoc->SetGrammar(meOldGrammar);
}

class MeasureTimeSwitch
{
    double& mrDiff;
    TimeValue maTimeBefore;
public:
    MeasureTimeSwitch(double& rDiff) : mrDiff(rDiff)
    {
        mrDiff = 9999.0;
        osl_getSystemTime(&maTimeBefore);
    }

    ~MeasureTimeSwitch()
    {
        TimeValue aTimeAfter;
        osl_getSystemTime(&aTimeAfter);
        mrDiff = getTimeDiff(aTimeAfter, maTimeBefore);
    }

    double getTimeDiff(const TimeValue& t1, const TimeValue& t2) const
    {
        double tv1 = t1.Seconds;
        double tv2 = t2.Seconds;
        tv1 += t1.Nanosec / 1000000000.0;
        tv2 += t2.Nanosec / 1000000000.0;

        return tv1 - tv2;
    }
};

Test::Test() :
    m_pImpl(new TestImpl),
    m_pDoc(0)
{
}

Test::~Test()
{
    delete m_pImpl;
}

ScDocShell& Test::getDocShell()
{
    return *m_pImpl->m_xDocShell;
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_pImpl->m_xDocShell = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);

    m_pImpl->m_xDocShell->DoInitUnitTest();
    m_pDoc = m_pImpl->m_xDocShell->GetDocument();
}

void Test::tearDown()
{
    m_pImpl->m_xDocShell.Clear();
    BootstrapFixture::tearDown();
}

#define PERF_ASSERT(df,scale,time,message) \
    do { \
        double dfscaled = df / scale; \
        if ((dfscaled) >= (time)) \
        { \
            std::ostringstream os; \
            os << message << " took " << dfscaled << " pseudo-cycles (" << df << " real-time seconds), expected: " << time << " pseudo-cycles."; \
            CPPUNIT_FAIL(os.str().c_str()); \
        } \
    } while (0)

void Test::testPerf()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

    // First do a set of simple operations to try to work out
    // how fast (or not) this particular machine is:
    double scale;
    {
        MeasureTimeSwitch aTime(scale);
        for (int i = 0; i < 10000000; ++i)
        {
            // Bang on the allocator
            volatile ScRange *pRange = new ScRange (ScAddress (0,0,0));
            // Calc does quite a bit of string conversion
            volatile double it = OUString::number ((double)i/253.0).toDouble();
            // Do we have floating point math ?
            volatile double another = rtl::math::sin (it);
            (void)another;
            delete pRange;
        }
    }
    printf("CPU scale factor %g\n", scale);

    // FIXME: we should check if this already took too long
    // and if so not run the perf. tests to have pity on some
    // slow ARM machines - I think.

    // to make the numbers more round and helpful,
    // but the calculation of scale reasonably precise.
    scale /= 100000.0;

    double diff;

    // Clearing an already empty sheet should finish in a fraction of a
    // second.  Flag failure if it takes more than one second.  Clearing 100
    // columns should be large enough to flag if something goes wrong.
    {
        MeasureTimeSwitch aTime(diff);
        clearRange(m_pDoc, ScRange(0,0,0,99,MAXROW,0));
    }
    PERF_ASSERT(diff, scale, 1.0, "Clearing an empty sheet");

    {
        // Switch to R1C1 to make it easier to input relative references in multiple cells.
        FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

        // Insert formulas in B1:B100000. This shouldn't take long, but may take
        // close to a second on a slower machine. We don't measure this yet, for
        // now.
        for (SCROW i = 0; i < 100000; ++i)
            m_pDoc->SetString(ScAddress(1,i,0), "=RC[-1]");

        // Now, Delete B2:B100000. This should complete in a fraction of a second
        // (0.06 sec on my machine).
        {
            MeasureTimeSwitch aTime(diff);
            clearRange(m_pDoc, ScRange(1,1,0,1,99999,0));
        }
        PERF_ASSERT(diff, scale, 2000, "Removal of a large array of formula cells");
    }

    clearRange(m_pDoc, ScRange(0,0,0,1,MAXROW,0)); // Clear columns A:B.
    CPPUNIT_ASSERT_MESSAGE("Column A shouldn't have any broadcasters.", !m_pDoc->HasBroadcaster(0,0));
    CPPUNIT_ASSERT_MESSAGE("Column B shouldn't have any broadcasters.", !m_pDoc->HasBroadcaster(0,1));

    {
        // Measure the performance of repeat-pasting a single cell to a large
        // cell range, undoing it, and redoing it.

        ScAddress aPos(0,0,0);
        m_pDoc->SetString(aPos, "test");
        ScMarkData aMark;
        aMark.SelectOneTable(0);

        // Copy cell A1 to clipboard.
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        ScClipParam aParam(aPos, false);
        m_pDoc->CopyToClip(aParam, &aClipDoc, &aMark);
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), aClipDoc.GetString(aPos));

        ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
        pUndoDoc->InitUndo(m_pDoc, 0, 0);
        m_pDoc->CopyToDocument(ScRange(aPos), IDF_CONTENTS, false, pUndoDoc, &aMark);

        // Paste it to A2:A100000, and measure its duration.
        ScRange aPasteRange(0,1,0,0,99999,0);
        aMark.SetMarkArea(aPasteRange);

        {
            MeasureTimeSwitch aTime(diff);
            m_pDoc->CopyFromClip(aPasteRange, aMark, IDF_CONTENTS, pUndoDoc, &aClipDoc);
        }
        PERF_ASSERT(diff, scale, 1500.0, "Pasting a single cell to A2:A100000");

        ScDocument* pRedoDoc = new ScDocument(SCDOCMODE_UNDO);
        pRedoDoc->InitUndo(m_pDoc, 0, 0);
        m_pDoc->CopyToDocument(aPasteRange, IDF_CONTENTS, false, pRedoDoc, &aMark);

        // Create an undo object for this.
        ScRefUndoData* pRefUndoData = new ScRefUndoData(m_pDoc);
        ScUndoPaste aUndo(&getDocShell(), aPasteRange, aMark, pUndoDoc, pRedoDoc, IDF_CONTENTS, pRefUndoData);

        // Make sure it did what it's supposed to do.
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aEnd));

        {
            MeasureTimeSwitch aTime(diff);
            aUndo.Undo();
        }
        PERF_ASSERT(diff, scale, 500.0, "Undoing a pasting of a cell to A2:A100000");

        // Make sure it's really undone.
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, m_pDoc->GetCellType(aPos));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPasteRange.aEnd));

        // Now redo.
        {
            MeasureTimeSwitch aTime(diff);
            aUndo.Redo();
        }
        PERF_ASSERT(diff, scale, 1000.0, "Redoing a pasting of a cell to A2:A100000");

        // Make sure it's really redone.
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aEnd));
    }

    clearRange(m_pDoc, ScRange(0,0,0,1,MAXROW,0)); // Clear columns A:B.
    CPPUNIT_ASSERT_MESSAGE("Column A shouldn't have any broadcasters.", !m_pDoc->HasBroadcaster(0,0));
    CPPUNIT_ASSERT_MESSAGE("Column B shouldn't have any broadcasters.", !m_pDoc->HasBroadcaster(0,1));

    {
        // Measure the performance of repeat-pasting 2 adjacent cells to a
        // large cell range, undoing it, and redoing it.  The bottom one of
        // the two source cells is empty.

        ScRange aSrcRange(0,0,0,0,1,0); // A1:A2

        ScAddress aPos(0,0,0);
        m_pDoc->SetValue(aPos, 12);
        ScMarkData aMark;
        aMark.SetMarkArea(aSrcRange);

        // Copy to clipboard.
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        ScClipParam aParam(aSrcRange, false);
        m_pDoc->CopyToClip(aParam, &aClipDoc, &aMark);
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), aClipDoc.GetString(aPos));

        ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
        pUndoDoc->InitUndo(m_pDoc, 0, 0);
        m_pDoc->CopyToDocument(aSrcRange, IDF_CONTENTS, false, pUndoDoc, &aMark);

        // Paste it to A3:A100001, and measure its duration.
        ScRange aPasteRange(0,2,0,0,100000,0);
        aMark.SetMarkArea(aPasteRange);

        {
            MeasureTimeSwitch aTime(diff);
            m_pDoc->CopyFromClip(aPasteRange, aMark, IDF_CONTENTS, pUndoDoc, &aClipDoc);
        }
        PERF_ASSERT(diff, scale, 1000.0, "Pasting A1:A2 to A3:A100001");

        ScDocument* pRedoDoc = new ScDocument(SCDOCMODE_UNDO);
        pRedoDoc->InitUndo(m_pDoc, 0, 0);
        m_pDoc->CopyToDocument(aPasteRange, IDF_CONTENTS, false, pRedoDoc, &aMark);

        // Create an undo object for this.
        ScRefUndoData* pRefUndoData = new ScRefUndoData(m_pDoc);
        ScUndoPaste aUndo(&getDocShell(), aPasteRange, aMark, pUndoDoc, pRedoDoc, IDF_CONTENTS, pRefUndoData);

        // Make sure it did what it's supposed to do.
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aEnd));
        ScAddress aTmp = aPasteRange.aStart;
        aTmp.IncRow();
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aTmp));

        {
            MeasureTimeSwitch aTime(diff);
            aUndo.Undo();
        }
        PERF_ASSERT(diff, scale, 500.0, "Undoing");

        // Make sure it's really undone.
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, m_pDoc->GetCellType(aPos));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPasteRange.aEnd));

        // Now redo.
        {
            MeasureTimeSwitch aTime(diff);
            aUndo.Redo();
        }
        PERF_ASSERT(diff, scale, 800.0, "Redoing");

        // Make sure it's really redone.
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), m_pDoc->GetString(aPasteRange.aEnd));
    }

    clearRange(m_pDoc, ScRange(0,0,0,1,MAXROW,0)); // Clear columns A:B.
    CPPUNIT_ASSERT_MESSAGE("Column A shouldn't have any broadcasters.", !m_pDoc->HasBroadcaster(0,0));
    CPPUNIT_ASSERT_MESSAGE("Column B shouldn't have any broadcasters.", !m_pDoc->HasBroadcaster(0,1));

    {
        // Measure the performance of repeat-pasting 2 adjacent cells to a
        // large cell range, undoing it, and redoing it.  The bottom one of
        // the two source cells is empty.  In this scenario, the non-empty
        // cell is a formula cell referencing a cell to the right, which
        // inserts a broadcaster to cell it references. So it has a higher
        // overhead than the previous scenario.

        ScRange aSrcRange(0,0,0,0,1,0); // A1:A2

        ScAddress aPos(0,0,0);
        m_pDoc->SetString(aPos, "=B1");
        ScMarkData aMark;
        aMark.SetMarkArea(aSrcRange);

        // Copy to clipboard.
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        ScClipParam aParam(aSrcRange, false);
        m_pDoc->CopyToClip(aParam, &aClipDoc, &aMark);
        CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), aClipDoc.GetString(aPos));

        ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
        pUndoDoc->InitUndo(m_pDoc, 0, 0);
        m_pDoc->CopyToDocument(aSrcRange, IDF_CONTENTS, false, pUndoDoc, &aMark);

        // Paste it to A3:A50001, and measure its duration.
        ScRange aPasteRange(0,2,0,0,50000,0);
        aMark.SetMarkArea(aPasteRange);

        {
            MeasureTimeSwitch aTime(diff);
            m_pDoc->CopyFromClip(aPasteRange, aMark, IDF_CONTENTS, pUndoDoc, &aClipDoc);
        }
        PERF_ASSERT(diff, scale, 2000.0, "Pasting");

        ScDocument* pRedoDoc = new ScDocument(SCDOCMODE_UNDO);
        pRedoDoc->InitUndo(m_pDoc, 0, 0);
        m_pDoc->CopyToDocument(aPasteRange, IDF_CONTENTS, false, pRedoDoc, &aMark);

        // Create an undo object for this.
        ScRefUndoData* pRefUndoData = new ScRefUndoData(m_pDoc);
        ScUndoPaste aUndo(&getDocShell(), aPasteRange, aMark, pUndoDoc, pRedoDoc, IDF_CONTENTS, pRefUndoData);

        // Make sure it did what it's supposed to do.
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(aPasteRange.aEnd));
        ScAddress aTmp = aPasteRange.aStart;
        aTmp.IncRow();
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aTmp));

#if 0 // TODO: Undo and redo of this scenario is currently not fast enough to be tested reliably.
        {
            MeasureTimeSwitch aTime(diff);
            aUndo.Undo();
        }
        PERF_ASSERT(diff, scale, 1.0, "Undoing");

        // Make sure it's really undone.
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(aPos));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPasteRange.aEnd));

        // Now redo.
        {
            MeasureTimeSwitch aTime(diff);
            aUndo.Redo();
        }
        PERF_ASSERT(diff, scale, 1.0, "Redoing");

        // Make sure it's really redone.
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(aPasteRange.aStart));
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, m_pDoc->GetCellType(aPasteRange.aEnd));
#endif
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCollator()
{
    OUString s1("A");
    OUString s2("B");
    CollatorWrapper* p = ScGlobal::GetCollator();
    sal_Int32 nRes = p->compareString(s1, s2);
    CPPUNIT_ASSERT_MESSAGE("these strings are supposed to be different!", nRes != 0);
}

void Test::testRangeList()
{
    m_pDoc->InsertTab(0, "foo");

    ScRangeList aRL;
    aRL.Append(ScRange(1,1,0,3,10,0));
    CPPUNIT_ASSERT_MESSAGE("List should have one range.", aRL.size() == 1);
    const ScRange* p = aRL[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get the range object.", p);
    CPPUNIT_ASSERT_MESSAGE("Wrong range.", p->aStart == ScAddress(1,1,0) && p->aEnd == ScAddress(3,10,0));

    // TODO: Add more tests here.

    m_pDoc->DeleteTab(0);
}

void Test::testInput()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    OUString numstr("'10.5");
    OUString str("'apple'");
    OUString test;

    m_pDoc->SetString(0, 0, 0, numstr);
    test = m_pDoc->GetString(0, 0, 0);
    bool bTest = test == "10.5";
    CPPUNIT_ASSERT_MESSAGE("String number should have the first apostrophe stripped.", bTest);
    m_pDoc->SetString(0, 0, 0, str);
    test = m_pDoc->GetString(0, 0, 0);
    bTest = test == "'apple'";
    CPPUNIT_ASSERT_MESSAGE("Text content should have retained the first apostrophe.", bTest);

    // Customized string handling policy.
    ScSetStringParam aParam;
    aParam.setTextInput();
    m_pDoc->SetString(0, 0, 0, "000123", &aParam);
    test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Text content should have been treated as string, not number.", test == "000123");

    m_pDoc->DeleteTab(0);
}

void Test::testCopyToDocument()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "src"));

    m_pDoc->SetString(0, 0, 0, "Header");
    m_pDoc->SetString(0, 1, 0, "1");
    m_pDoc->SetString(0, 2, 0, "2");
    m_pDoc->SetString(0, 3, 0, "3");
    m_pDoc->SetString(0, 4, 0, "=4/2");
    m_pDoc->CalcAll();

    // Copy statically to another document.

    ScDocument aDestDoc(SCDOCMODE_DOCUMENT);
    aDestDoc.InsertTab(0, "src");
    m_pDoc->CopyStaticToDocument(ScRange(0,1,0,0,3,0), 0, &aDestDoc); // Copy A2:A4
    m_pDoc->CopyStaticToDocument(ScAddress(0,0,0), 0, &aDestDoc); // Copy A1
    m_pDoc->CopyStaticToDocument(ScRange(0,4,0,0,7,0), 0, &aDestDoc); // Copy A5:A8

    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,0,0), aDestDoc.GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,1,0), aDestDoc.GetString(0,1,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,2,0), aDestDoc.GetString(0,2,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,3,0), aDestDoc.GetString(0,3,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,4,0), aDestDoc.GetString(0,4,0));

    m_pDoc->DeleteTab(0);
}

void Test::testSheetsFunc()
{
    OUString aTabName1("test1");
    OUString aTabName2("test2");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName1));

    m_pDoc->SetString(0, 0, 0, OUString("=SHEETS()"));
    m_pDoc->CalcFormulaTree(false, false);
    double original;
    m_pDoc->GetValue(0, 0, 0, original);

    CPPUNIT_ASSERT_MESSAGE("result of SHEETS() should equal the number of sheets, but doesn't.",
                           static_cast<SCTAB>(original) == m_pDoc->GetTableCount());

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (SC_TAB_APPEND, aTabName2));

    double modified;
    m_pDoc->GetValue(0, 0, 0, modified);
    CPPUNIT_ASSERT_MESSAGE("result of SHEETS() did not get updated after sheet insertion.",
                           modified - original == 1.0);

    SCTAB nTabCount = m_pDoc->GetTableCount();
    m_pDoc->DeleteTab(--nTabCount);

    m_pDoc->GetValue(0, 0, 0, modified);
    CPPUNIT_ASSERT_MESSAGE("result of SHEETS() did not get updated after sheet removal.",
                           modified - original == 0.0);

    m_pDoc->DeleteTab(--nTabCount);
}

void Test::testVolatileFunc()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    double val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->SetString(0, 1, 0, OUString("=IF(A1>0;NOW();0"));
    double now1;
    m_pDoc->GetValue(0, 1, 0, now1);
    CPPUNIT_ASSERT_MESSAGE("Value of NOW() should be positive.", now1 > 0.0);

    val = 0;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, false);
    double zero;
    m_pDoc->GetValue(0, 1, 0, zero);
    CPPUNIT_ASSERT_MESSAGE("Result should equal the 3rd parameter of IF, which is zero.", zero == 0.0);

    val = 1;
    m_pDoc->SetValue(0, 0, 0, val);
    m_pDoc->CalcFormulaTree(false, false);
    double now2;
    m_pDoc->GetValue(0, 1, 0, now2);
    CPPUNIT_ASSERT_MESSAGE("Result should be the value of NOW() again.", (now2 - now1) >= 0.0);

    m_pDoc->DeleteTab(0);
}

namespace {

struct HoriIterCheck
{
    SCCOL nCol;
    SCROW nRow;
    const char* pVal;
};

template<size_t _Size>
bool checkHorizontalIterator(ScDocument* pDoc, const char* pData[][_Size], size_t nDataCount, const HoriIterCheck* pChecks, size_t nCheckCount)
{
    ScAddress aPos(0,0,0);
    Test::insertRangeData(pDoc, aPos, pData, nDataCount);
    ScHorizontalCellIterator aIter(pDoc, 0, 0, 0, 1, nDataCount-1);

    SCCOL nCol;
    SCROW nRow;
    size_t i = 0;
    for (ScRefCellValue* pCell = aIter.GetNext(nCol, nRow); pCell; pCell = aIter.GetNext(nCol, nRow), ++i)
    {
        if (i >= nCheckCount)
            CPPUNIT_FAIL("Iterator claims there is more data than there should be.");

        if (pChecks[i].nCol != nCol)
            return false;

        if (pChecks[i].nRow != nRow)
            return false;

        if (OUString::createFromAscii(pChecks[i].pVal) != pCell->getString())
            return false;
    }

    return true;
}

}

void Test::testHorizontalIterator()
{
    m_pDoc->InsertTab(0, "test");

    {
        // Raw data
        const char* aData[][2] = {
            { "A", "B" },
            { "C", "1" },
            { "D", "2" },
            { "E", "3" }
        };

        HoriIterCheck aChecks[] = {
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
            m_pDoc, aData, SAL_N_ELEMENTS(aData), aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test 1.");
    }

    {
        // Raw data
        const char* aData[][2] = {
            { "A", "B" },
            { "C",  0  },
            { "D", "E" },
        };

        HoriIterCheck aChecks[] = {
            { 0, 0, "A" },
            { 1, 0, "B" },
            { 0, 1, "C" },
            { 0, 2, "D" },
            { 1, 2, "E" },
        };

        bool bRes = checkHorizontalIterator(
            m_pDoc, aData, SAL_N_ELEMENTS(aData), aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test 2.");
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTracking()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    // B2 listens on D2.
    m_pDoc->SetString(1, 1, 0, "=D2");
    double val = -999.0; // dummy initial value
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Referencing an empty cell should yield zero.", val == 0.0);

    // Changing the value of D2 should trigger recalculation of B2.
    m_pDoc->SetValue(3, 1, 0, 1.1);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on value change.", val == 1.1);

    // And again.
    m_pDoc->SetValue(3, 1, 0, 2.2);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on value change.", val == 2.2);

    clearRange(m_pDoc, ScRange(0, 0, 0, 10, 10, 0));

    // Now, let's test the range dependency tracking.

    // B2 listens on D2:E6.
    m_pDoc->SetString(1, 1, 0, "=SUM(D2:E6)");
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Summing an empty range should yield zero.", val == 0.0);

    // Set value to E3. This should trigger recalc on B2.
    m_pDoc->SetValue(4, 2, 0, 2.4);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", val == 2.4);

    // Set value to D5 to trigger recalc again.  Note that this causes an
    // addition of 1.2 + 2.4 which is subject to binary floating point
    // rounding error.  We need to use approxEqual to assess its value.

    m_pDoc->SetValue(3, 4, 0, 1.2);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 3.6));

    // Change the value of D2 (boundary case).
    m_pDoc->SetValue(3, 1, 0, 1.0);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 4.6));

    // Change the value of E6 (another boundary case).
    m_pDoc->SetValue(4, 5, 0, 2.0);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 6.6));

    // Change the value of D6 (another boundary case).
    m_pDoc->SetValue(3, 5, 0, 3.0);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 9.6));

    // Change the value of E2 (another boundary case).
    m_pDoc->SetValue(4, 1, 0, 0.4);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 10.0));

    // Change the existing non-empty value cell (E2).
    m_pDoc->SetValue(4, 1, 0, 2.4);
    m_pDoc->GetValue(1, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("Failed to recalculate on single value change.", rtl::math::approxEqual(val, 12.0));

    clearRange(m_pDoc, ScRange(0, 0, 0, 10, 10, 0));

    // Now, column-based dependency tracking.  We now switch to the R1C1
    // syntax which is easier to use for repeated relative references.

    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    val = 0.0;
    for (SCROW nRow = 1; nRow <= 9; ++nRow)
    {
        // Static value in column 1.
        m_pDoc->SetValue(0, nRow, 0, ++val);

        // Formula in column 2 that references cell to the left.
        m_pDoc->SetString(1, nRow, 0, "=RC[-1]");

        // Formula in column 3 that references cell to the left.
        m_pDoc->SetString(2, nRow, 0, "=RC[-1]*2");
    }

    // Check formula values.
    val = 0.0;
    for (SCROW nRow = 1; nRow <= 9; ++nRow)
    {
        ++val;
        CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(1, nRow, 0) == val);
        CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(2, nRow, 0) == val*2.0);
    }

    // Intentionally insert a formula in column 1. This will break column 1's
    // uniformity of consisting only of static value cells.
    m_pDoc->SetString(0, 4, 0, "=R2C3");
    CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(0, 4, 0) == 2.0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(1, 4, 0) == 2.0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected formula value.", m_pDoc->GetValue(2, 4, 0) == 4.0);

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaDepTracking2()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "foo"));

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    double val = 2.0;
    m_pDoc->SetValue(0, 0, 0, val);
    val = 4.0;
    m_pDoc->SetValue(1, 0, 0, val);
    val = 5.0;
    m_pDoc->SetValue(0, 1, 0, val);
    m_pDoc->SetString(2, 0, 0, "=A1/B1");
    m_pDoc->SetString(1, 1, 0, "=B1*C1");

    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1, 1, 0)); // B2 should equal 2.

    clearRange(m_pDoc, ScAddress(2, 0, 0)); // Delete C1.

    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(1, 1, 0)); // B2 should now equal 0.

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

ScToken* getSingleRefToken(ScDocument& rDoc, const ScAddress& rPos)
{
    ScFormulaCell* pFC = rDoc.GetFormulaCell(rPos);
    if (!pFC)
    {
        cerr << "Formula cell expected, but not found." << endl;
        return NULL;
    }

    ScTokenArray* pTokens = pFC->GetCode();
    if (!pTokens)
    {
        cerr << "Token array is not present." << endl;
        return NULL;
    }

    ScToken* pToken = static_cast<ScToken*>(pTokens->First());
    if (!pToken || pToken->GetType() != formula::svSingleRef)
    {
        cerr << "Not a single reference token." << endl;
        return NULL;
    }

    return pToken;
}

bool checkRelativeRefToken(ScDocument& rDoc, const ScAddress& rPos, SCsCOL nRelCol, SCsROW nRelRow)
{
    ScToken* pToken = getSingleRefToken(rDoc, rPos);
    if (!pToken)
        return false;

    ScSingleRefData& rRef = pToken->GetSingleRef();
    if (!rRef.IsColRel() || rRef.nRelCol != nRelCol)
    {
        cerr << "Unexpected relative column address." << endl;
        return false;
    }

    if (!rRef.IsRowRel() || rRef.nRelRow != nRelRow)
    {
        cerr << "Unexpected relative row address." << endl;
        return false;
    }

    return true;
}

bool checkDeletedRefToken(ScDocument& rDoc, const ScAddress& rPos)
{
    ScToken* pToken = getSingleRefToken(rDoc, rPos);
    if (!pToken)
        return false;

    ScSingleRefData& rRef = pToken->GetSingleRef();
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

    // Switch to R1C1 to make it easier to input relative references in multiple cells.
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    // Have B1:B20 reference A1:A20.
    val = 0.0;
    for (SCROW i = 0; i < 20; ++i)
    {
        m_pDoc->SetValue(ScAddress(0,i,0), val++);
        m_pDoc->SetString(ScAddress(1,i,0), "=RC[-1]");
    }

    // Ensure that the formula cells show correct values, and the referenced
    // cells have broadcasters.
    val = 0.0;
    for (SCROW i = 0; i < 20; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(val++, m_pDoc->GetValue(ScAddress(1,i,0)));
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

    m_pDoc->DeleteTab(0);
}

void Test::testFuncParam()
{
    OUString aTabName("foo");
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet",
                            m_pDoc->InsertTab (0, aTabName));

    // First, the normal case, with no missing parameters.
    m_pDoc->SetString(0, 0, 0, OUString("=AVERAGE(1;2;3)"));
    m_pDoc->CalcFormulaTree(false, false);
    double val;
    m_pDoc->GetValue(0, 0, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 2);

    // Now function with missing parameters.  Missing values should be treated
    // as zeros.
    m_pDoc->SetString(0, 0, 0, OUString("=AVERAGE(1;;;)"));
    m_pDoc->CalcFormulaTree(false, false);
    m_pDoc->GetValue(0, 0, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 0.25);

    // Conversion of string to numeric argument.
    m_pDoc->SetString(0, 0, 0, OUString("=\"\"+3"));    // empty string
    m_pDoc->SetString(0, 1, 0, OUString("=\" \"+3"));   // only blank
    m_pDoc->SetString(0, 2, 0, OUString("=\" 4 \"+3")); // number in blanks
    m_pDoc->SetString(0, 3, 0, OUString("=\" x \"+3")); // non-numeric => #VALUE! error

    OUString aVal;
    ScCalcConfig aConfig;

    // With "Empty string as zero" option.
    aConfig.mbEmptyStringAsZero = true;
    ScInterpreter::SetGlobalConfig(aConfig);
    m_pDoc->CalcAll();
    m_pDoc->GetValue(0, 0, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 3);
    m_pDoc->GetValue(0, 1, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 3);
    m_pDoc->GetValue(0, 2, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 7);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");

    // Without "Empty string as zero" option.
    aConfig.mbEmptyStringAsZero = false;
    ScInterpreter::SetGlobalConfig(aConfig);
    m_pDoc->CalcAll();
    aVal = m_pDoc->GetString( 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");
    aVal = m_pDoc->GetString( 0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");
    m_pDoc->GetValue(0, 2, 0, val);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", val == 7);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("incorrect result", aVal == "#VALUE!");

    m_pDoc->DeleteTab(0);
}

void Test::testNamedRange()
{
    struct {
        const char* pName; const char* pExpr; sal_uInt16 nIndex;
    } aNames[] = {
        { "Divisor",  "$Sheet1.$A$1:$A$1048576", 1 },
        { "MyRange1", "$Sheet1.$A$1:$A$100",     2 },
        { "MyRange2", "$Sheet1.$B$1:$B$100",     3 },
        { "MyRange3", "$Sheet1.$C$1:$C$100",     4 }
    };

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "Sheet1"));

    m_pDoc->SetValue (0, 0, 0, 101);

    ScAddress aA1(0, 0, 0);
    ScRangeName* pNewRanges = new ScRangeName();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        ScRangeData* pNew = new ScRangeData(
            m_pDoc,
            OUString::createFromAscii(aNames[i].pName),
            OUString::createFromAscii(aNames[i].pExpr),
            aA1, 0, formula::FormulaGrammar::GRAM_ENGLISH);
        pNew->SetIndex(aNames[i].nIndex);
        bool bSuccess = pNewRanges->insert(pNew);
        CPPUNIT_ASSERT_MESSAGE ("insertion failed", bSuccess);
    }

    // Make sure the index lookup does the right thing.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = pNewRanges->findByIndex(aNames[i].nIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed.", p);
        OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved.", aName.equalsAscii(aNames[i].pName));
    }

    // Test usage in formula expression.
    m_pDoc->SetRangeName(pNewRanges);
    m_pDoc->SetString (1, 0, 0, OUString("=A1/Divisor"));
    m_pDoc->CalcAll();

    double result;
    m_pDoc->GetValue (1, 0, 0, result);
    CPPUNIT_ASSERT_MESSAGE ("calculation failed", result == 1.0);

    // Test copy-ability of range names.
    ScRangeName* pCopiedRanges = new ScRangeName(*pNewRanges);
    m_pDoc->SetRangeName(pCopiedRanges);
    // Make sure the index lookup still works.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = pCopiedRanges->findByIndex(aNames[i].nIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed with the copied instance.", p);
        OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved with the copied instance.", aName.equalsAscii(aNames[i].pName));
    }

    m_pDoc->SetRangeName(NULL); // Delete the names.
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
        { "1.000.00",  European, false, 0.0 }
    };
    for (sal_uInt32 i = 0; i < SAL_N_ELEMENTS(aTests); i++) {
        OUString aStr(aTests[i].pStr, strlen (aTests[i].pStr), RTL_TEXTENCODING_UTF8);
        double nValue = 0.0;
        bool bResult = ScStringUtil::parseSimpleNumber
                (aStr, aTests[i].eSep == English ? '.' : ',',
                 aTests[i].eSep == English ? ',' : '.',
                 nValue);
        CPPUNIT_ASSERT_MESSAGE ("CSV numeric detection failure", bResult == aTests[i].bResult);
        CPPUNIT_ASSERT_MESSAGE ("CSV numeric value failure", nValue == aTests[i].nValue);
    }
}

template<typename Evaluator>
void checkMatrixElements(const ScMatrix& rMat)
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

struct AllZeroMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of numeric type", rVal.nType == SC_MATVAL_VALUE);
        CPPUNIT_ASSERT_MESSAGE("element value must be zero", rVal.fVal == 0.0);
    }
};

struct PartiallyFilledZeroMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of numeric type", rVal.nType == SC_MATVAL_VALUE);
        if (1 <= nCol && nCol <= 2 && 2 <= nRow && nRow <= 8)
        {
            CPPUNIT_ASSERT_MESSAGE("element value must be 3.0", rVal.fVal == 3.0);
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("element value must be zero", rVal.fVal == 0.0);
        }
    }
};

struct AllEmptyMatrix
{
    void operator() (SCSIZE /*nCol*/, SCSIZE /*nRow*/, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_MESSAGE("element is not of empty type", rVal.nType == SC_MATVAL_EMPTY);
        CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
    }
};

struct PartiallyFilledEmptyMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        if (nCol == 1 && nRow == 1)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of boolean type", rVal.nType == SC_MATVAL_BOOLEAN);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.fVal == 1.0);
        }
        else if (nCol == 4 && nRow == 5)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of value type", rVal.nType == SC_MATVAL_VALUE);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.fVal == -12.5);
        }
        else if (nCol == 8 && nRow == 2)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of value type", rVal.nType == SC_MATVAL_STRING);
            CPPUNIT_ASSERT_MESSAGE("element value is not what is expected", rVal.aStr == "Test");
        }
        else if (nCol == 8 && nRow == 11)
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of empty path type", rVal.nType == SC_MATVAL_EMPTYPATH);
            CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("element is not of empty type", rVal.nType == SC_MATVAL_EMPTY);
            CPPUNIT_ASSERT_MESSAGE("value of \"empty\" element is expected to be zero", rVal.fVal == 0.0);
        }
    }
};

void Test::testMatrix()
{
    ScMatrixRef pMat, pMat2;

    // First, test the zero matrix type.
    pMat = new ScMatrix(0, 0, 0.0);
    SCSIZE nC, nR;
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix is not empty", nC == 0 && nR == 0);
    pMat->Resize(4, 10, 0.0);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 4 && nR == 10);
    CPPUNIT_ASSERT_MESSAGE("both 'and' and 'or' should evaluate to false",
                           !pMat->And() && !pMat->Or());

    // Resizing into a larger matrix should fill the void space with zeros.
    checkMatrixElements<AllZeroMatrix>(*pMat);

    pMat->FillDouble(3.0, 1, 2, 2, 8);
    checkMatrixElements<PartiallyFilledZeroMatrix>(*pMat);
    CPPUNIT_ASSERT_MESSAGE("matrix is expected to be numeric", pMat->IsNumeric());
    CPPUNIT_ASSERT_MESSAGE("partially non-zero matrix should evaluate false on 'and' and true on 'or",
                           !pMat->And() && pMat->Or());
    pMat->FillDouble(5.0, 0, 0, nC-1, nR-1);
    CPPUNIT_ASSERT_MESSAGE("fully non-zero matrix should evaluate true both on 'and' and 'or",
                           pMat->And() && pMat->Or());

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

    // Now test the emtpy matrix type.
    pMat = new ScMatrix(10, 20);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 10 && nR == 20);
    checkMatrixElements<AllEmptyMatrix>(*pMat);

    pMat->PutBoolean(true, 1, 1);
    pMat->PutDouble(-12.5, 4, 5);
    OUString aStr("Test");
    pMat->PutString(aStr, 8, 2);
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
    pMat->PutString("Test", 0, 0);
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
    pMat->PutString("A", 1, 1);
    pMat->PutDouble(2.3, 2, 1);
    pMat->PutDouble(-20, 2, 2);

    double fNaN;
    rtl::math::setNan(&fNaN);

    std::vector<double> aDoubles;
    pMat->GetDoubleArray(aDoubles);

    {
        const double pChecks[] = { 2.5, 1.2, 0, 0, fNaN, 0, 0, 2.3, -20 };
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(pChecks), aDoubles.size());
        for (size_t i = 0, n = aDoubles.size(); i < n; ++i)
        {
            if (rtl::math::isNan(pChecks[i]))
                CPPUNIT_ASSERT_MESSAGE("NaN is expected, but it's not.", rtl::math::isNan(aDoubles[i]));
            else
                CPPUNIT_ASSERT_EQUAL(pChecks[i], aDoubles[i]);
        }
    }

    pMat2 = new ScMatrix(3, 3, 10.0);
    pMat2->PutString("B", 1, 0);
    pMat2->MergeDoubleArray(aDoubles, ScMatrix::Mul);

    {
        const double pChecks[] = { 25, 12, 0, fNaN, fNaN, 0, 0, 23, -200 };
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(pChecks), aDoubles.size());
        for (size_t i = 0, n = aDoubles.size(); i < n; ++i)
        {
            if (rtl::math::isNan(pChecks[i]))
                CPPUNIT_ASSERT_MESSAGE("NaN is expected, but it's not.", rtl::math::isNan(aDoubles[i]));
            else
                CPPUNIT_ASSERT_EQUAL(pChecks[i], aDoubles[i]);
        }
    }
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
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 3, 1, 4, aMark, "=A1:B2", NULL);

    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(0,0,0), m_pDoc->GetString(0,3,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(1,0,0), m_pDoc->GetString(1,3,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(0,1,0), m_pDoc->GetValue(0,4,0));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(1,1,0), m_pDoc->GetValue(1,4,0));

    m_pDoc->DeleteTab(0);
}

void Test::testSheetCopy()
{
    m_pDoc->InsertTab(0, "TestTab");
    m_pDoc->SetString(ScAddress(0,0,0), "copy me");
    CPPUNIT_ASSERT_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount() == 1);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Copy and test the result.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount() == 2);
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
    m_pDoc->DeleteTab(1);

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Copy the sheet once again.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount() == 2);
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testSheetMove()
{
    OUString aTabName("TestTab1");
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount(), static_cast<SCTAB>(1));
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    //test if inserting before another sheet works
    m_pDoc->InsertTab(0, OUString("TestTab2"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have two sheets", m_pDoc->GetTableCount(), static_cast<SCTAB>(2));
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Move and test the result.
    m_pDoc->MoveTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount(), static_cast<SCTAB>(2));
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
    OUString aName;
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_MESSAGE( "sheets should have changed places", aName == "TestTab1" );

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Move the sheet once again.
    m_pDoc->MoveTab(1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", m_pDoc->GetTableCount(), static_cast<SCTAB>(2));
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_MESSAGE( "sheets should have changed places", aName == "TestTab2" );
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

ScDocShell* findLoadedDocShellByName(const OUString& rName)
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(&aType, false));
    while (pShell)
    {
        SfxMedium* pMedium = pShell->GetMedium();
        if (pMedium)
        {
            OUString aName = pMedium->GetName();
            if (aName.equals(rName))
                return pShell;
        }
        pShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pShell, &aType, false));
    }
    return NULL;
}

ScRange getCachedRange(const ScExternalRefCache::TableTypeRef& pCacheTab)
{
    ScRange aRange;

    vector<SCROW> aRows;
    pCacheTab->getAllRows(aRows);
    vector<SCROW>::const_iterator itrRow = aRows.begin(), itrRowEnd = aRows.end();
    bool bFirst = true;
    for (; itrRow != itrRowEnd; ++itrRow)
    {
        SCROW nRow = *itrRow;
        vector<SCCOL> aCols;
        pCacheTab->getAllCols(nRow, aCols);
        vector<SCCOL>::const_iterator itrCol = aCols.begin(), itrColEnd = aCols.end();
        for (; itrCol != itrColEnd; ++itrCol)
        {
            SCCOL nCol = *itrCol;
            if (bFirst)
            {
                aRange.aStart = ScAddress(nCol, nRow, 0);
                aRange.aEnd = aRange.aStart;
                bFirst = false;
            }
            else
            {
                if (nCol < aRange.aStart.Col())
                    aRange.aStart.SetCol(nCol);
                else if (aRange.aEnd.Col() < nCol)
                    aRange.aEnd.SetCol(nCol);

                if (nRow < aRange.aStart.Row())
                    aRange.aStart.SetRow(nRow);
                else if (aRange.aEnd.Row() < nRow)
                    aRange.aEnd.SetRow(nRow);
            }
        }
    }
    return aRange;
}

void Test::testExternalRef()
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("Data1");
    OUString aExtSh2Name("Data2");
    OUString aExtSh3Name("Data3");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != NULL);

    // Populate the external source document.
    ScDocument* pExtDoc = xExtDocSh->GetDocument();
    pExtDoc->InsertTab(0, aExtSh1Name);
    pExtDoc->InsertTab(1, aExtSh2Name);
    pExtDoc->InsertTab(2, aExtSh3Name);

    OUString name("Name");
    OUString value("Value");
    OUString andy("Andy");
    OUString bruce("Bruce");
    OUString charlie("Charlie");
    OUString david("David");
    OUString edward("Edward");
    OUString frank("Frank");
    OUString george("George");
    OUString henry("Henry");

    // Sheet 1
    pExtDoc->SetString(0, 0, 0, name);
    pExtDoc->SetString(0, 1, 0, andy);
    pExtDoc->SetString(0, 2, 0, bruce);
    pExtDoc->SetString(0, 3, 0, charlie);
    pExtDoc->SetString(0, 4, 0, david);
    pExtDoc->SetString(1, 0, 0, value);
    double val = 10;
    pExtDoc->SetValue(1, 1, 0, val);
    val = 11;
    pExtDoc->SetValue(1, 2, 0, val);
    val = 12;
    pExtDoc->SetValue(1, 3, 0, val);
    val = 13;
    pExtDoc->SetValue(1, 4, 0, val);

    // Sheet 2 remains empty.

    // Sheet 3
    pExtDoc->SetString(0, 0, 2, name);
    pExtDoc->SetString(0, 1, 2, edward);
    pExtDoc->SetString(0, 2, 2, frank);
    pExtDoc->SetString(0, 3, 2, george);
    pExtDoc->SetString(0, 4, 2, henry);
    pExtDoc->SetString(1, 0, 2, value);
    val = 99;
    pExtDoc->SetValue(1, 1, 2, val);
    val = 98;
    pExtDoc->SetValue(1, 2, 2, val);
    val = 97;
    pExtDoc->SetValue(1, 3, 2, val);
    val = 96;
    pExtDoc->SetValue(1, 4, 2, val);

    // Test external refernces on the main document while the external
    // document is still in memory.
    m_pDoc->InsertTab(0, OUString("Test Sheet"));
    m_pDoc->SetString(0, 0, 0, OUString("='file:///extdata.fake'#Data1.A1"));
    OUString test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Value is different from the original", test.equals(name));

    // After the initial access to the external document, the external ref
    // manager should create sheet cache entries for *all* sheets from that
    // document.  Note that the doc may have more than 3 sheets but ensure
    // that the first 3 are what we expect.
    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    vector<OUString> aTabNames;
    pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
    CPPUNIT_ASSERT_MESSAGE("There should be at least 3 sheets.", aTabNames.size() >= 3);
    CPPUNIT_ASSERT_MESSAGE("Unexpected sheet name.", aTabNames[0].equals(aExtSh1Name));
    CPPUNIT_ASSERT_MESSAGE("Unexpected sheet name.", aTabNames[1].equals(aExtSh2Name));
    CPPUNIT_ASSERT_MESSAGE("Unexpected sheet name.", aTabNames[2].equals(aExtSh3Name));

    m_pDoc->SetString(1, 0, 0, OUString("='file:///extdata.fake'#Data1.B1"));
    test = m_pDoc->GetString(1, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Value is different from the original", test.equals(value));

    m_pDoc->SetString(0, 1, 0, OUString("='file:///extdata.fake'#Data1.A2"));
    m_pDoc->SetString(0, 2, 0, OUString("='file:///extdata.fake'#Data1.A3"));
    m_pDoc->SetString(0, 3, 0, OUString("='file:///extdata.fake'#Data1.A4"));
    m_pDoc->SetString(0, 4, 0, OUString("='file:///extdata.fake'#Data1.A5"));
    m_pDoc->SetString(0, 5, 0, OUString("='file:///extdata.fake'#Data1.A6"));

    {
        // Referencing an empty cell should display '0'.
        const char* pChecks[] = { "Andy", "Bruce", "Charlie", "David", "0" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            test = m_pDoc->GetString(0, static_cast<SCROW>(i+1), 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }
    m_pDoc->SetString(1, 1, 0, OUString("='file:///extdata.fake'#Data1.B2"));
    m_pDoc->SetString(1, 2, 0, OUString("='file:///extdata.fake'#Data1.B3"));
    m_pDoc->SetString(1, 3, 0, OUString("='file:///extdata.fake'#Data1.B4"));
    m_pDoc->SetString(1, 4, 0, OUString("='file:///extdata.fake'#Data1.B5"));
    m_pDoc->SetString(1, 5, 0, OUString("='file:///extdata.fake'#Data1.B6"));
    {
        double pChecks[] = { 10, 11, 12, 13, 0 };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            m_pDoc->GetValue(1, static_cast<SCROW>(i+1), 0, val);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", val == pChecks[i]);
        }
    }

    m_pDoc->SetString(2, 0, 0, OUString("='file:///extdata.fake'#Data3.A1"));
    m_pDoc->SetString(2, 1, 0, OUString("='file:///extdata.fake'#Data3.A2"));
    m_pDoc->SetString(2, 2, 0, OUString("='file:///extdata.fake'#Data3.A3"));
    m_pDoc->SetString(2, 3, 0, OUString("='file:///extdata.fake'#Data3.A4"));
    {
        const char* pChecks[] = { "Name", "Edward", "Frank", "George" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            test = m_pDoc->GetString(2, static_cast<SCROW>(i), 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }

    m_pDoc->SetString(3, 0, 0, OUString("='file:///extdata.fake'#Data3.B1"));
    m_pDoc->SetString(3, 1, 0, OUString("='file:///extdata.fake'#Data3.B2"));
    m_pDoc->SetString(3, 2, 0, OUString("='file:///extdata.fake'#Data3.B3"));
    m_pDoc->SetString(3, 3, 0, OUString("='file:///extdata.fake'#Data3.B4"));
    {
        const char* pChecks[] = { "Value", "99", "98", "97" };
        for (size_t i = 0; i < SAL_N_ELEMENTS(pChecks); ++i)
        {
            test = m_pDoc->GetString(3, static_cast<SCROW>(i), 0);
            CPPUNIT_ASSERT_MESSAGE("Unexpected cell value.", test.equalsAscii(pChecks[i]));
        }
    }

    // At this point, all accessed cell data from the external document should
    // have been cached.
    ScExternalRefCache::TableTypeRef pCacheTab = pRefMgr->getCacheTable(
        nFileId, aExtSh1Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 1 should exist.", pCacheTab.get() != NULL);
    ScRange aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cached data range.",
                           aCachedRange.aStart.Col() == 0 && aCachedRange.aEnd.Col() == 1 &&
                           aCachedRange.aStart.Row() == 0 && aCachedRange.aEnd.Row() == 4);

    // Sheet2 is not referenced at all; the cache table shouldn't even exist.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh2Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 2 should *not* exist.", pCacheTab.get() == NULL);

    // Sheet3's row 5 is not referenced; it should not be cached.
    pCacheTab = pRefMgr->getCacheTable(nFileId, aExtSh3Name, false);
    CPPUNIT_ASSERT_MESSAGE("Cache table for sheet 3 should exist.", pCacheTab.get() != NULL);
    aCachedRange = getCachedRange(pCacheTab);
    CPPUNIT_ASSERT_MESSAGE("Unexpected cached data range.",
                           aCachedRange.aStart.Col() == 0 && aCachedRange.aEnd.Col() == 1 &&
                           aCachedRange.aStart.Row() == 0 && aCachedRange.aEnd.Row() == 3);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == NULL);

    m_pDoc->DeleteTab(0);
}

void testExtRefFuncT(ScDocument* pDoc, ScDocument* pExtDoc)
{
    Test::clearRange(pDoc, ScRange(0, 0, 0, 1, 9, 0));
    Test::clearRange(pExtDoc, ScRange(0, 0, 0, 1, 9, 0));

    pExtDoc->SetString(0, 0, 0, OUString("'1.2"));
    pExtDoc->SetString(0, 1, 0, OUString("Foo"));
    pExtDoc->SetValue(0, 2, 0, 12.3);
    pDoc->SetString(0, 0, 0, OUString("=T('file:///extdata.fake'#Data.A1)"));
    pDoc->SetString(0, 1, 0, OUString("=T('file:///extdata.fake'#Data.A2)"));
    pDoc->SetString(0, 2, 0, OUString("=T('file:///extdata.fake'#Data.A3)"));
    pDoc->CalcAll();

    OUString aRes = pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE( "Unexpected result with T.", aRes == "1.2" );
    aRes = pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE( "Unexpected result with T.", aRes == "Foo" );
    aRes = pDoc->GetString(0, 2, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected result with T.", aRes.isEmpty());
}

void Test::testExternalRefFunctions()
{
    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != NULL);

    ScExternalRefManager* pRefMgr = m_pDoc->GetExternalRefManager();
    CPPUNIT_ASSERT_MESSAGE("external reference manager doesn't exist.", pRefMgr);
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aExtDocName);
    const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
    CPPUNIT_ASSERT_MESSAGE("file name registration has somehow failed.",
                           pFileName && pFileName->equals(aExtDocName));

    // Populate the external source document.
    ScDocument* pExtDoc = xExtDocSh->GetDocument();
    pExtDoc->InsertTab(0, OUString("Data"));
    double val = 1;
    pExtDoc->SetValue(0, 0, 0, val);
    // leave cell B1 empty.
    val = 2;
    pExtDoc->SetValue(0, 1, 0, val);
    pExtDoc->SetValue(1, 1, 0, val);
    val = 3;
    pExtDoc->SetValue(0, 2, 0, val);
    pExtDoc->SetValue(1, 2, 0, val);
    val = 4;
    pExtDoc->SetValue(0, 3, 0, val);
    pExtDoc->SetValue(1, 3, 0, val);

    m_pDoc->InsertTab(0, OUString("Test"));

    struct {
        const char* pFormula; double fResult;
    } aChecks[] = {
        { "=SUM('file:///extdata.fake'#Data.A1:A4)",     10 },
        { "=SUM('file:///extdata.fake'#Data.B1:B4)",     9 },
        { "=AVERAGE('file:///extdata.fake'#Data.A1:A4)", 2.5 },
        { "=AVERAGE('file:///extdata.fake'#Data.B1:B4)", 3 },
        { "=COUNT('file:///extdata.fake'#Data.A1:A4)",   4 },
        { "=COUNT('file:///extdata.fake'#Data.B1:B4)",   3 }
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        m_pDoc->SetString(0, 0, 0, OUString::createFromAscii(aChecks[i].pFormula));
        m_pDoc->CalcAll();
        m_pDoc->GetValue(0, 0, 0, val);
        CPPUNIT_ASSERT_MESSAGE("unexpected result involving external ranges.", val == aChecks[i].fResult);
    }

    pRefMgr->clearCache(nFileId);
    testExtRefFuncT(m_pDoc, pExtDoc);

    // Unload the external document shell.
    xExtDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("external document instance should have been unloaded.",
                           findLoadedDocShellByName(aExtDocName) == NULL);

    m_pDoc->DeleteTab(0);
}

void Test::testDataArea()
{
    m_pDoc->InsertTab(0, OUString("Data"));

    // Totally empty sheet should be rightfully considered empty in all accounts.
    CPPUNIT_ASSERT_MESSAGE("Sheet is expected to be empty.", m_pDoc->IsPrintEmpty(0, 0, 0, 100, 100));
    CPPUNIT_ASSERT_MESSAGE("Sheet is expected to be empty.", m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // Now, set borders in some cells....
    ::editeng::SvxBorderLine aLine(NULL, 50, table::BorderLineStyle::SOLID);
    SvxBoxItem aBorderItem(ATTR_BORDER);
    aBorderItem.SetLine(&aLine, BOX_LINE_LEFT);
    aBorderItem.SetLine(&aLine, BOX_LINE_RIGHT);
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
    m_pDoc->SetString(0, 0, 0, OUString("Some text"));
    CPPUNIT_ASSERT_MESSAGE("Now the block should not be empty with a real cell content.",
                           !m_pDoc->IsBlockEmpty(0, 0, 0, 100, 100));

    // TODO: Add more tests for normal data area calculation.

    m_pDoc->DeleteTab(0);
}

void Test::testStreamValid()
{
    m_pDoc->InsertTab(0, OUString("Sheet1"));
    m_pDoc->InsertTab(1, OUString("Sheet2"));
    m_pDoc->InsertTab(2, OUString("Sheet3"));
    m_pDoc->InsertTab(3, OUString("Sheet4"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We should have 4 sheet instances.", m_pDoc->GetTableCount(), static_cast<SCTAB>(4));

    OUString a1("A1");
    OUString a2("A2");
    OUString test;

    // Put values into Sheet1.
    m_pDoc->SetString(0, 0, 0, a1);
    m_pDoc->SetString(0, 1, 0, a2);
    test = m_pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A1", test.equals(a1));
    test = m_pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A2", test.equals(a2));

    // Put formulas into Sheet2 to Sheet4 to reference values from Sheet1.
    m_pDoc->SetString(0, 0, 1, OUString("=Sheet1.A1"));
    m_pDoc->SetString(0, 1, 1, OUString("=Sheet1.A2"));
    m_pDoc->SetString(0, 0, 2, OUString("=Sheet1.A1"));
    m_pDoc->SetString(0, 0, 3, OUString("=Sheet1.A2"));

    test = m_pDoc->GetString(0, 0, 1);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet2.A1", test.equals(a1));
    test = m_pDoc->GetString(0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet2.A2", test.equals(a2));
    test = m_pDoc->GetString(0, 0, 2);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet3.A1", test.equals(a1));
    test = m_pDoc->GetString(0, 0, 3);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet3.A1", test.equals(a2));

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
    CPPUNIT_ASSERT_MESSAGE("Cell A1 should not have moved.", test.equals(a1));
    test = m_pDoc->GetString(0, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("the old cell A2 should now be at A4.", test.equals(a2));
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
        0
    };

    const char* aDateTime[] = {
        "DATE",
        "DATEDIF",
        "DATEVALUE",
        "DAY",
        "DAYS",
        "DAYS360",
        "EASTERSUNDAY",
        "HOUR",
        "MINUTE",
        "MONTH",
        "NOW",
        "SECOND",
        "TIME",
        "TIMEVALUE",
        "TODAY",
        "WEEKDAY",
        "WEEKNUM",
        "YEAR",
        0
    };

    const char* aFinancial[] = {
        "CUMIPMT",
        "CUMPRINC",
        "DB",
        "DDB",
        "DURATION",
        "EFFECTIVE",
        "FV",
        "IPMT",
        "IRR",
        "ISPMT",
        "MIRR",
        "NOMINAL",
        "NPER",
        "NPV",
        "PMT",
        "PPMT",
        "PV",
        "RATE",
        "RRI",
        "SLN",
        "SYD",
        "VDB",
        0
    };

    const char* aInformation[] = {
        "CELL",
        "CURRENT",
        "FORMULA",
        "INFO",
        "ISBLANK",
        "ISERR",
        "ISERROR",
        "ISFORMULA",
        "ISLOGICAL",
        "ISNA",
        "ISNONTEXT",
        "ISNUMBER",
        "ISREF",
        "ISTEXT",
        "N",
        "NA",
        "TYPE",
        0
    };

    const char* aLogical[] = {
        "AND",
        "FALSE",
        "IF",
        "IFERROR",
        "IFNA",
        "NOT",
        "OR",
        "TRUE",
        "XOR",
        0
    };

    const char* aMathematical[] = {
        "ABS",
        "ACOS",
        "ACOSH",
        "ACOT",
        "ACOTH",
        "ASIN",
        "ASINH",
        "ATAN",
        "ATAN2",
        "ATANH",
        "AVERAGEIF",
        "AVERAGEIFS",
        "BITAND",
        "BITLSHIFT",
        "BITOR",
        "BITRSHIFT",
        "BITXOR",
        "CEILING",
        "COMBIN",
        "COMBINA",
        "CONVERT",
        "COS",
        "COSH",
        "COT",
        "COTH",
        "COUNTBLANK",
        "COUNTIF",
        "COUNTIFS",
        "CSC",
        "CSCH",
        "DEGREES",
        "EUROCONVERT",
        "EVEN",
        "EXP",
        "FACT",
        "FLOOR",
        "GCD",
        "INT",
        "ISEVEN",
        "ISODD",
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
        "ROUND",
        "ROUNDDOWN",
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
        0
    };

    const char* aArray[] = {
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
        0
    };

    const char* aStatistical[] = {
        "AVEDEV",
        "AVERAGE",
        "AVERAGEA",
        "B",
        "BETADIST",
        "BETAINV",
        "BINOMDIST",
        "CHIDIST",
        "CHIINV",
        "CHISQDIST",
        "CHISQINV",
        "CHITEST",
        "CONFIDENCE",
        "CORREL",
        "COUNT",
        "COUNTA",
        "COVAR",
        "CRITBINOM",
        "DEVSQ",
        "EXPONDIST",
        "FDIST",
        "FINV",
        "FISHER",
        "FISHERINV",
        "FORECAST",
        "FTEST",
        "GAMMA",
        "GAMMADIST",
        "GAMMAINV",
        "GAMMALN",
        "GAUSS",
        "GEOMEAN",
        "HARMEAN",
        "HYPGEOMDIST",
        "INTERCEPT",
        "KURT",
        "LARGE",
        "LOGINV",
        "LOGNORMDIST",
        "MAX",
        "MAXA",
        "MEDIAN",
        "MIN",
        "MINA",
        "MODE",
        "NEGBINOMDIST",
        "NORMDIST",
        "NORMINV",
        "NORMSDIST",
        "NORMSINV",
        "PEARSON",
        "PERCENTILE",
        "PERCENTRANK",
        "PERMUT",
        "PERMUTATIONA",
        "PHI",
        "POISSON",
        "PROB",
        "QUARTILE",
        "RANK",
        "RSQ",
        "SKEW",
        "SKEWP",
        "SLOPE",
        "SMALL",
        "STANDARDIZE",
        "STDEV",
        "STDEVA",
        "STDEVP",
        "STDEVPA",
        "STEYX",
        "TDIST",
        "TINV",
        "TRIMMEAN",
        "TTEST",
        "VAR",
        "VARA",
        "VARP",
        "VARPA",
        "WEIBULL",
        "ZTEST",
        0
    };

    const char* aSpreadsheet[] = {
        "ADDRESS",
        "AREAS",
        "CHOOSE",
        "COLUMN",
        "COLUMNS",
        "DDE",
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
        0
    };

    const char* aText[] = {
        "ARABIC",
        "ASC",
        "BAHTTEXT",
        "BASE",
        "CHAR",
        "CLEAN",
        "CODE",
        "CONCATENATE",
        "DECIMAL",
        "DOLLAR",
        "EXACT",
        "FILTERXML",
        "FIND",
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
        "REPLACE",
        "REPT",
        "RIGHT",
        "RIGHTB",
        "ROMAN",
        "SEARCH",
        "SUBSTITUTE",
        "T",
        "TEXT",
        "TRIM",
        "UNICHAR",
        "UNICODE",
        "UPPER",
        "VALUE",
        "WEBSERVICE",
        0
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
        { "Add-in",       0 },
        { 0, 0 }
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
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected function name", pFunc->getFunctionName(), OUString::createFromAscii(aTests[i].Functions[j]));
        }
    }
}

void Test::testGraphicsInGroup()
{
    OUString aTabName("TestTab");
    m_pDoc->InsertTab(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE("document should have one sheet to begin with.", m_pDoc->GetTableCount() == 1);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != NULL);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != NULL);

    {
        //Add a square
        Rectangle aOrigRect(2,2,100,100);
        SdrRectObj *pObj = new SdrRectObj(aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle &rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_MESSAGE("must have equal position and size", aOrigRect == rNewRect);

        ScDrawLayer::SetPageAnchored(*pObj);

        //Use a range of rows guaranteed to include all of the square
        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_MESSAGE("That shouldn't change size or positioning", aOrigRect == rNewRect);

        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Left and Right should be unchanged",
            aOrigRect.Left() == rNewRect.Left() && aOrigRect.Right() == rNewRect.Right());
        CPPUNIT_ASSERT_MESSAGE("Height should be minimum allowed height",
            (rNewRect.Bottom() - rNewRect.Top()) <= 1);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Should not change when page anchored", aOrigRect == rNewRect);
    }

    {
        // Add a circle.
        Rectangle aOrigRect = Rectangle(10,10,210,210); // 200 x 200
        SdrCircObj* pObj = new SdrCircObj(OBJ_CIRC, aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_MESSAGE("Position and size of the circle shouldn't change when inserted into the page.",
                               aOrigRect == rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_MESSAGE("Size changed when cell anchored. Not good.",
                               aOrigRect == rNewRect);

        // Insert 2 rows at the top.  This should push the circle object down.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);

        // Make sure the size of the circle is still identical.
        CPPUNIT_ASSERT_MESSAGE("Size of the circle has changed, but shouldn't!",
                               aOrigRect.GetSize() == rNewRect.GetSize());

        // Delete 2 rows at the top.  This should bring the circle object to its original position.
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Failed to move back to its original position.", aOrigRect == rNewRect);
    }

    {
        // Add a line.
        basegfx::B2DPolygon aTempPoly;
        Point aStartPos(10,300), aEndPos(110,200); // bottom-left to top-right.
        Rectangle aOrigRect(10,200,110,300); // 100 x 100
        aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
        aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
        SdrPathObj* pObj = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
        pObj->NbcSetLogicRect(aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_MESSAGE("Size differ.", aOrigRect == rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_MESSAGE("Size changed when cell-anchored. Not good.",
                               aOrigRect == rNewRect);

        // Insert 2 rows at the top and delete them immediately.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Size of a line object changed after row insertion and removal.",
                               aOrigRect == rNewRect);

        sal_Int32 n = pObj->GetPointCount();
        CPPUNIT_ASSERT_MESSAGE("There should be exactly 2 points in a line object.", n == 2);
        CPPUNIT_ASSERT_MESSAGE("Line shape has changed.",
                               aStartPos == pObj->GetPoint(0) && aEndPos == pObj->GetPoint(1));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testGraphicsOnSheetMove()
{
    m_pDoc->InsertTab(0, OUString("Tab1"));
    m_pDoc->InsertTab(1, OUString("Tab2"));
    CPPUNIT_ASSERT_MESSAGE("There should be only 2 sheets to begin with", m_pDoc->GetTableCount() == 2);

    m_pDoc->InitDrawLayer();
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);

    // Insert an object.
    Rectangle aObjRect(2,2,100,100);
    SdrObject* pObj = new SdrRectObj(aObjRect);
    pPage->InsertObject(pObj);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one object on the 1st sheet.", pPage->GetObjCount(), static_cast<sal_uIntPtr>(1));

    const ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Object meta-data doesn't exist.", pData);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 0 && pData->maEnd.Tab() == 0);

    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 2nd sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet shouldn't have any object.", pPage->GetObjCount(), static_cast<sal_uIntPtr>(0));

    // Insert a new sheet at left-end, and make sure the object has moved to
    // the 2nd page.
    m_pDoc->InsertTab(0, OUString("NewTab"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 3 sheets.", m_pDoc->GetTableCount(), static_cast<SCTAB>(3));
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have no object.", pPage && pPage->GetObjCount() == 0);
    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("2nd sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    pPage = pDrawLayer->GetPage(2);
    CPPUNIT_ASSERT_MESSAGE("3rd sheet should have no object.", pPage && pPage->GetObjCount() == 0);

    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 1 && pData->maEnd.Tab() == 1);

    // Now, delete the sheet that just got inserted. The object should be back
    // on the 1st sheet.
    m_pDoc->DeleteTab(0);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    CPPUNIT_ASSERT_MESSAGE("Size and position of the object shouldn't change.",
                           pObj->GetLogicRect() == aObjRect);

    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 0 && pData->maEnd.Tab() == 0);

    // Move the 1st sheet to the last position.
    m_pDoc->MoveTab(0, 1);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("1st sheet should have no object.", pPage && pPage->GetObjCount() == 0);
    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("2nd sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 1 && pData->maEnd.Tab() == 1);

    // Copy the 2nd sheet, which has one drawing object to the last position.
    m_pDoc->CopyTab(1, 2);
    pPage = pDrawLayer->GetPage(2);
    CPPUNIT_ASSERT_MESSAGE("Copied sheet should have one object.", pPage && pPage->GetObjCount() == 1);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object meta-data.", pData);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 2 && pData->maEnd.Tab() == 2);

    m_pDoc->DeleteTab(2);
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPostIts()
{
    OUString aHello("Hello world");
    OUString aJimBob("Jim Bob");
    OUString aTabName("PostIts");
    OUString aTabName2("Table2");
    m_pDoc->InsertTab(0, aTabName);

    ScAddress rAddr(2, 2, 0); // cell C3
    ScPostIt *pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    ScPostIt *pGetNote = m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr);
    CPPUNIT_ASSERT_MESSAGE("note should be itself", pGetNote == pNote );

    // Insert one row at row 1.
    bool bInsertRow = m_pDoc->InsertRow(0, 0, MAXCOL, 0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("failed to insert row", bInsertRow );

    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == NULL);
    rAddr.IncRow(); // cell C4
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    // Insert column at column A.
    bool bInsertCol = m_pDoc->InsertCol(0, 0, MAXROW, 0, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("failed to insert column", bInsertCol );

    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == NULL);
    rAddr.IncCol(); // cell D4
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    // Insert a new sheet to shift the current sheet to the right.
    m_pDoc->InsertTab(0, aTabName2);
    CPPUNIT_ASSERT_MESSAGE("note hasn't moved", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == NULL);
    rAddr.IncTab(); // Move to the next sheet.
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    m_pDoc->DeleteTab(0);
    rAddr.IncTab(-1);
    CPPUNIT_ASSERT_MESSAGE("note not there", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    // Insert cell at C4.  This should NOT shift the note position.
    bInsertRow = m_pDoc->InsertRow(2, 0, 2, 0, 3, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert cell at C4.", bInsertRow);
    CPPUNIT_ASSERT_MESSAGE("Note shouldn't have moved but it has.", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    // Delete cell at C4.  Again, this should NOT shift the note position.
    m_pDoc->DeleteRow(2, 0, 2, 0, 3, 1);
    CPPUNIT_ASSERT_MESSAGE("Note shouldn't have moved but it has.", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    // Now, with the note at D4, delete cell D3. This should shift the note one cell up.
    m_pDoc->DeleteRow(3, 0, 3, 0, 2, 1);
    rAddr.IncRow(-1); // cell D3
    CPPUNIT_ASSERT_MESSAGE("Note at D4 should have shifted up to D3.", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    // Delete column C. This should shift the note one cell left.
    m_pDoc->DeleteCol(0, 0, MAXROW, 0, 2, 1);
    rAddr.IncCol(-1); // cell C3
    CPPUNIT_ASSERT_MESSAGE("Note at D3 should have shifted left to C3.", m_pDoc->GetNotes(rAddr.Tab())->findByAddress(rAddr) == pNote);

    m_pDoc->DeleteTab(0);
}

void Test::testToggleRefFlag()
{
    // In this test, there is no need to insert formula string into a cell in
    // the document, as ScRefFinder does not depend on the content of the
    // document except for the sheet names.

    OUString aTabName("Test");
    m_pDoc->InsertTab(0, aTabName);

    {
        // Calc A1: basic 2D reference

        OUString aFormula("=B100");
        ScAddress aPos(1, 5, 0);
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_OOO);

        // Original
        CPPUNIT_ASSERT_MESSAGE("Does not equal the original text.", aFormula.equals(aFinder.GetText()));

        // column relative / row relative -> column absolute / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=$B$100" );

        // column absolute / row absolute -> column relative / row absolute
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=B$100" );

        // column relative / row absolute -> column absolute / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=$B100" );

        // column absolute / row relative -> column relative / row relative
        aFinder.ToggleRel(0, aFormula.getLength());
        aFormula = aFinder.GetText();
        CPPUNIT_ASSERT_MESSAGE( "Wrong conversion.", aFormula == "=B100" );
    }

    {
        // Excel R1C1: basic 2D reference

        OUString aFormula("=R2C1");
        ScAddress aPos(3, 5, 0);
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_XL_R1C1);

        // Original
        CPPUNIT_ASSERT_MESSAGE("Does not equal the original text.", aFormula.equals(aFinder.GetText()));

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
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_XL_R1C1);

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
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_OOO);

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
    OUString aTabName("Test");
    OUString aDBName("NONAME");

    m_pDoc->InsertTab( 0, aTabName );

    // cell contents (0 = empty cell)
    const char* aData[][3] = {
        { "C1", "C2", "C3" },
        {  "0",  "1",  "A" },
        {  "1",  "2",    0 },
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

    ScDBData* pDBData = new ScDBData(aDBName, 0, 0, 0, nCols-1, nRows-1);
    m_pDoc->SetAnonymousDBData(0,pDBData);

    pDBData->SetAutoFilter(true);
    ScRange aRange;
    pDBData->GetArea(aRange);
    m_pDoc->ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(), aRange.aStart.Row(),
                           aRange.aStart.Tab(), SC_MF_AUTO);

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
    CPPUNIT_ASSERT_MESSAGE("rows 2 & 3 should be hidden", bHidden && nRow1 == 2 && nRow2 == 3);

    // Remove filtering.
    rEntry.Clear();
    m_pDoc->Query(0, aParam, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("All rows should be shown.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Filter for non-empty cells by column C.
    rEntry.bDoQuery = true;
    rEntry.nField = 2;
    rEntry.SetQueryByNonEmpty();
    m_pDoc->Query(0, aParam, true);

    // only row 3 should be hidden.  The rest should be visible.
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 1 & 2 should be visible.", !bHidden && nRow1 == 0 && nRow2 == 1);
    bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 3 should be hidden.", bHidden && nRow1 == 2 && nRow2 == 2);
    bHidden = m_pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 4 and down should be visible.", !bHidden && nRow1 == 3 && nRow2 == MAXROW);

    // Now, filter for empty cells by column C.
    rEntry.SetQueryByEmpty();
    m_pDoc->Query(0, aParam, true);

    // Now, only row 1 and 3, and 6 and down should be visible.
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 1 should be visible.", !bHidden && nRow1 == 0 && nRow2 == 0);
    bHidden = m_pDoc->RowHidden(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 2 should be hidden.", bHidden && nRow1 == 1 && nRow2 == 1);
    bHidden = m_pDoc->RowHidden(2, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("row 3 should be visible.", !bHidden && nRow1 == 2 && nRow2 == 2);
    bHidden = m_pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 4 & 5 should be hidden.", bHidden && nRow1 == 3 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 6 and down should be all visible.", !bHidden && nRow1 == 5 && nRow2 == MAXROW);

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPaste()
{
    m_pDoc->InsertTab(0, OUString("Sheet1"));
    m_pDoc->InsertTab(1, OUString("Sheet2"));
    //test copy&paste + ScUndoPaste
    //copy local and global range names in formulas
    //string cells and value cells
    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetValue(3, 0, 0, 0);
    m_pDoc->SetValue(3, 1, 0, 1);
    m_pDoc->SetValue(3, 2, 0, 2);
    m_pDoc->SetValue(3, 3, 0, 3);
    m_pDoc->SetString(2, 0, 0, OUString("test"));
    ScAddress aAdr (0, 0, 0);

    //create some range names, local and global
    ScRangeData* pLocal1 = new ScRangeData(m_pDoc, OUString("local1"), aAdr);
    ScRangeData* pLocal2 = new ScRangeData(m_pDoc, OUString("local2"), aAdr);
    ScRangeData* pGlobal = new ScRangeData(m_pDoc, OUString("global"), aAdr);
    ScRangeName* pGlobalRangeName = new ScRangeName();
    pGlobalRangeName->insert(pGlobal);
    ScRangeName* pLocalRangeName1 = new ScRangeName();
    pLocalRangeName1->insert(pLocal1);
    pLocalRangeName1->insert(pLocal2);
    m_pDoc->SetRangeName(pGlobalRangeName);
    m_pDoc->SetRangeName(0, pLocalRangeName1);

    // Add formula to B1.
    OUString aFormulaString("=local1+global+SUM($C$1:$D$4)");
    m_pDoc->SetString(1, 0, 0, aFormulaString);

    double fValue = m_pDoc->GetValue(ScAddress(1,0,0));
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 8", fValue, 8);

    //copy Sheet1.A1:C1 to Sheet2.A2:C2
    ScRange aRange(0,0,0,2,0,0);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    aRange = ScRange(0,1,1,2,1,1);//target: Sheet2.A2:C2
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 1, 1, true, true);
    boost::scoped_ptr<ScUndoPaste> pUndo(createUndoPaste(getDocShell(), aRange, pUndoDoc));
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    m_pDoc->CopyFromClip(aRange, aMark, IDF_ALL, NULL, &aClipDoc);

    //check values after copying
    OUString aString;
    fValue = m_pDoc->GetValue(ScAddress(1,1,1));
    m_pDoc->GetFormula(1,1,1, aString);
    ASSERT_DOUBLES_EQUAL_MESSAGE("copied formula should return 2", fValue, 2);
    CPPUNIT_ASSERT_MESSAGE("formula string was not copied correctly", aString == aFormulaString);
    fValue = m_pDoc->GetValue(ScAddress(0,1,1));
    CPPUNIT_ASSERT_MESSAGE("copied value should be 1", fValue == 1);

    //chack local range name after copying
    pLocal1 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("local range name 1 should be copied", pLocal1);
    ScRange aRangeLocal1;
    pLocal1->IsValidReference(aRangeLocal1);
    CPPUNIT_ASSERT_MESSAGE("local range 1 should still point to Sheet1.A1",aRangeLocal1 == ScRange(0,0,0,0,0,0));
    pLocal2 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("local2 should not be copied", pLocal2 == NULL);


    //check undo and redo
    pUndo->Undo();
    fValue = m_pDoc->GetValue(ScAddress(1,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("after undo formula should return nothing", fValue, 0);
    aString = m_pDoc->GetString(2, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("after undo string should be removed", aString.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("")));

    pUndo->Redo();
    fValue = m_pDoc->GetValue(ScAddress(1,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 2 after redo", fValue, 2);
    aString = m_pDoc->GetString(2, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("Cell Sheet2.C2 should contain: test", aString.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("test")));
    m_pDoc->GetFormula(1,1,1, aString);
    CPPUNIT_ASSERT_MESSAGE("Formula should be correct again", aString == aFormulaString);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testMergedCells()
{
    //test merge and unmerge
    //TODO: an undo/redo test for this would be a good idea
    m_pDoc->InsertTab(0, OUString("Sheet1"));
    m_pDoc->DoMerge(0, 1, 1, 3, 3, false);
    SCCOL nEndCol = 1;
    SCROW nEndRow = 1;
    m_pDoc->ExtendMerge( 1, 1, nEndCol, nEndRow, 0, false);
    CPPUNIT_ASSERT_MESSAGE("did not merge cells", nEndCol == 3 && nEndRow == 3);
    ScRange aRange(0,2,0,MAXCOL,2,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    getDocShell().GetDocFunc().InsertCells(aRange, &aMark, INS_INSROWS, true, true);
    m_pDoc->ExtendMerge(1, 1, nEndCol, nEndRow, 0, false);
    cout << nEndRow << nEndCol;
    CPPUNIT_ASSERT_MESSAGE("did not increase merge area", nEndCol == 3 && nEndRow == 4);
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
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
    CPPUNIT_ASSERT_MESSAGE("name same as another table is being set", !rDocFunc.RenameTable(1,nameToSet,false,true) );

    //test case 2 , simple rename to check name
    nameToSet = "test1";
    getDocShell().GetDocFunc().RenameTable(0,nameToSet,false,true);
    OUString nameJustSet;
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("table not renamed", nameToSet == nameJustSet);

    //test case 3 , rename again
    OUString anOldName;
    m_pDoc->GetName(0,anOldName);

    nameToSet = "test2";
    rDocFunc.RenameTable(0,nameToSet,false,true);
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("table not renamed", nameToSet == nameJustSet);

    //test case 4 , check if  undo works
    SfxUndoAction* pUndo = new ScUndoRenameTab(&getDocShell(),0,anOldName,nameToSet);
    pUndo->Undo();
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("the correct name is not set after undo", nameJustSet == anOldName);

    pUndo->Redo();
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set after redo", nameJustSet == nameToSet);

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
    aColor=Color(COL_YELLOW);
    getDocShell().GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set",
                           m_pDoc->GetTabBgColor(0) == aColor);


    Color aOldTabBgColor=m_pDoc->GetTabBgColor(0);
    aColor.SetColor(COL_BLUE);//set BLUE
    getDocShell().GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set the second time",
                           m_pDoc->GetTabBgColor(0) == aColor);

    //now check for undo
    SfxUndoAction* pUndo = new ScUndoTabColor(&getDocShell(), 0, aOldTabBgColor, aColor);
    pUndo->Undo();
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set after undo", m_pDoc->GetTabBgColor(0)== aOldTabBgColor);
    pUndo->Redo();
    CPPUNIT_ASSERT_MESSAGE("the correct color is not set after undo", m_pDoc->GetTabBgColor(0)== aColor);
    m_pDoc->DeleteTab(0);
}



void Test::testUpdateReference()
{
    //test that formulas are correctly updated during sheet delete
    //TODO: add tests for relative references, updating of named ranges, ...
    OUString aSheet1("Sheet1");
    OUString aSheet2("Sheet2");
    OUString aSheet3("Sheet3");
    OUString aSheet4("Sheet4");
    m_pDoc->InsertTab(0, aSheet1);
    m_pDoc->InsertTab(1, aSheet2);
    m_pDoc->InsertTab(2, aSheet3);
    m_pDoc->InsertTab(3, aSheet4);

    m_pDoc->SetValue(0,0,2, 1);
    m_pDoc->SetValue(1,0,2, 2);
    m_pDoc->SetValue(1,1,3, 4);
    m_pDoc->SetString(2,0,2, OUString("=A1+B1"));
    m_pDoc->SetString(2,1,2, OUString("=Sheet4.B2+A1"));

    double aValue;
    m_pDoc->GetValue(2,0,2, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2,1,2, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula does not return correct result", aValue, 5);

    //test deleting both sheets: one is not directly before the sheet, the other one is
    m_pDoc->DeleteTab(0);
    m_pDoc->GetValue(2,0,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting first sheet formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2,1,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting first sheet formula does not return correct result", aValue, 5);

    m_pDoc->DeleteTab(0);
    m_pDoc->GetValue(2,0,0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting second sheet formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2,1,0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting second sheet formula does not return correct result", aValue, 5);

    //test adding two sheets
    m_pDoc->InsertTab(0, aSheet2);
    m_pDoc->GetValue(2,0,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2,1,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 5);

    m_pDoc->InsertTab(0, aSheet1);
    m_pDoc->GetValue(2,0,2, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting second sheet formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2,1,2, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting second sheet formula does not return correct result", aValue, 5);

    //test new DeleteTabs/InsertTabs methods
    m_pDoc->DeleteTabs(0, 2);
    m_pDoc->GetValue(2, 0, 0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting sheets formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2, 1, 0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after deleting sheets formula does not return correct result", aValue, 5);

    std::vector<OUString> aSheets;
    aSheets.push_back(aSheet1);
    aSheets.push_back(aSheet2);
    m_pDoc->InsertTabs(0, aSheets, false, true);
    m_pDoc->GetValue(2, 0, 2, aValue);
    OUString aFormula;
    m_pDoc->GetFormula(2,0,2, aFormula);
    std::cout << "formel: " << OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    std::cout << std::endl << aValue << std::endl;
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2, 1, 2, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 5);

    m_pDoc->DeleteTab(3);
    m_pDoc->DeleteTab(2);
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
    aItem.SetSearchString(OUString("A"));
    aItem.SetCommand(SVX_SEARCHCMD_FIND_ALL);
    ScMarkData aMarkData;
    aMarkData.SelectOneTable(0);
    SCCOL nCol = 0;
    SCROW nRow = 0;
    SCTAB nTab = 0;
    ScRangeList aMatchedRanges;
    OUString aUndoStr;
    m_pDoc->SearchAndReplace(aItem, nCol, nRow, nTab, aMarkData, aMatchedRanges, aUndoStr);

    CPPUNIT_ASSERT_MESSAGE("There should be exactly 3 matching cells.", aMatchedRanges.size() == 3);
    ScAddress aHit(0,0,0);
    CPPUNIT_ASSERT_MESSAGE("A1 should be inside the matched range.", aMatchedRanges.In(aHit));
    aHit.SetRow(2);
    CPPUNIT_ASSERT_MESSAGE("A3 should be inside the matched range.", aMatchedRanges.In(aHit));
    aHit.SetRow(4);
    CPPUNIT_ASSERT_MESSAGE("A5 should be inside the matched range.", aMatchedRanges.In(aHit));

    m_pDoc->DeleteTab(0);
}

namespace {

bool checkFormulaPosition(ScDocument& rDoc, const ScAddress& rPos)
{
    OUString aStr;
    rPos.Format(aStr, SCA_VALID);
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(rPos);
    if (!pFC)
    {
        cerr << "Formula cell expected at " << aStr << " but not found." << endl;
        return false;
    }

    if (pFC->aPos != rPos)
    {
        OUString aStr2;
        pFC->aPos.Format(aStr2, SCA_VALID);
        cerr << "Formula cell at " << aStr << " has incorrect position of " << aStr2 << endl;
        return false;
    }

    return true;
}

void checkFormulaPositions(ScDocument& rDoc, const ScAddress& rPos, const SCROW* pRows, size_t nRowCount)
{
    ScAddress aPos = rPos;
    for (size_t i = 0; i < nRowCount; ++i)
    {
        SCROW nRow = pRows[i];
        aPos.SetRow(nRow);

        if (!checkFormulaPosition(rDoc, aPos))
        {
            OUString aStr;
            aPos.Format(aStr, SCA_VALID);
            std::ostringstream os;
            os << "Formula cell position failed at " << aStr;
            CPPUNIT_FAIL(os.str().c_str());
        }
    }
}

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
        checkFormulaPositions(*m_pDoc, aPos, aRows, SAL_N_ELEMENTS(aRows));
    }

    m_pDoc->InsertRow(0,0,0,0,1,5); // Insert 5 rows at A2.
    {
        SCROW aRows[] = { 0, 6, 8 };
        checkFormulaPositions(*m_pDoc, aPos, aRows, SAL_N_ELEMENTS(aRows));
    }

    m_pDoc->DeleteTab(0);
}

namespace {

bool hasRange(const std::vector<ScTokenRef>& rRefTokens, const ScRange& rRange)
{
    std::vector<ScTokenRef>::const_iterator it = rRefTokens.begin(), itEnd = rRefTokens.end();
    for (; it != itEnd; ++it)
    {
        const ScTokenRef& p = *it;
        if (!ScRefTokenHelper::isRef(p) || ScRefTokenHelper::isExternalRef(p))
            continue;

        switch (p->GetType())
        {
            case formula::svSingleRef:
            {
                ScSingleRefData aData = p->GetSingleRef();
                if (rRange.aStart != rRange.aEnd)
                    break;

                ScAddress aThis(aData.nCol, aData.nRow, aData.nTab);
                if (aThis == rRange.aStart)
                    return true;
            }
            break;
            case formula::svDoubleRef:
            {
                ScComplexRefData aData = p->GetDoubleRef();
                ScRange aThis(aData.Ref1.nCol, aData.Ref1.nRow, aData.Ref1.nTab, aData.Ref2.nCol, aData.Ref2.nRow, aData.Ref2.nTab);
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
    m_pDoc->InsertTab(0, OUString("Test"));

    m_pDoc->SetString(2, 0, 0, OUString("=A1+A2+B3")); // C1
    m_pDoc->SetString(2, 1, 0, OUString("=A1"));       // C2
    m_pDoc->CalcAll();

    std::vector<ScTokenRef> aRefTokens;
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();

    {
        // C1's precedent should be A1:A2,B3.
        ScRangeList aRange(ScRange(2, 0, 0));
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("A1:A2 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(0, 0, 0, 0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("B3 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(1, 2, 0)));
    }

    {
        // C2's precedent should be A1 only.
        ScRangeList aRange(ScRange(2, 1, 0));
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("there should only be one reference token.",
                               aRefTokens.size(), static_cast<size_t>(1));
        CPPUNIT_ASSERT_MESSAGE("A1 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(0, 0, 0)));
    }

    {
        // A1's dependent should be C1:C2.
        ScRangeList aRange(ScRange(0, 0, 0));
        rDocFunc.DetectiveCollectAllSuccs(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("C1:C2 should be the only dependent of A1.",
                               aRefTokens.size() == 1 && hasRange(aRefTokens, ScRange(2, 0, 0, 2, 1, 0)));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFill()
{
    m_pDoc->InsertTab(0, "test");

    m_pDoc->SetValue(0,0,0,1);

    ScMarkData aMarkData;
    aMarkData.SelectTable(0, true);

    m_pDoc->Fill( 0, 0, 0, 0, NULL, aMarkData, 5);
    for (SCROW i = 0; i< 6; ++i)
        ASSERT_DOUBLES_EQUAL(static_cast<double>(i+1.0), m_pDoc->GetValue(0, i, 0));

    // check that hidden rows are not affected by autofill
    // set values for hidden rows
    m_pDoc->SetValue(0,1,0,10);
    m_pDoc->SetValue(0,2,0,10);

    m_pDoc->SetRowHidden(1, 2, 0, true);
    m_pDoc->Fill( 0, 0, 0, 0, NULL, aMarkData, 8);

    ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,1,0));
    ASSERT_DOUBLES_EQUAL(10.0, m_pDoc->GetValue(0,2,0));
    for (SCROW i = 3; i< 8; ++i)
        ASSERT_DOUBLES_EQUAL(static_cast<double>(i-1.0), m_pDoc->GetValue(0, i, 0));

    m_pDoc->Fill( 0, 0, 0, 8, NULL, aMarkData, 5, FILL_TO_RIGHT );
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
            else if (j == 1 || j== 2)
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
    m_pDoc->Fill( 0, 100, 0, 100, NULL, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    OUString aTestValue = m_pDoc->GetString( 0, 101, 0 );
    CPPUNIT_ASSERT_EQUAL( aTestValue, OUString("February") );
    aTestValue = m_pDoc->GetString( 0, 102, 0 );
    CPPUNIT_ASSERT_EQUAL( aTestValue, OUString("March") );

    // test that two same user data list entries will not result in incremental fill
    m_pDoc->SetString( 0, 101, 0, "January" );
    m_pDoc->Fill( 0, 100, 0, 101, NULL, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    for ( SCROW i = 102; i <= 103; ++i )
    {
        aTestValue = m_pDoc->GetString( 0, i, 0 );
        CPPUNIT_ASSERT_EQUAL( aTestValue, OUString("January") );
    }
    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteFormulas()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(0,0,0, "=COLUMN($A$1)");
    m_pDoc->SetString(0,1,0, "=$A$1+B2" );
    m_pDoc->SetString(0,2,0, "=$Sheet2.A1");
    m_pDoc->SetString(0,3,0, "=$Sheet2.$A$1");
    m_pDoc->SetString(0,4,0, "=$Sheet2.A$1");

    // to prevent ScEditableTester in ScDocFunc::MoveBlock
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(0,0,0), 1.0);
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(0,1,0), 1.0);
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
    bool bMoveDone = rDocFunc.MoveBlock(ScRange(0,0,0,0,4,0), ScAddress( 10, 10, 0), false, false, false, true);

    // check that moving was succesful, mainly for editable tester
    CPPUNIT_ASSERT(bMoveDone);
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(10,10,0), 1.0);
    ASSERT_DOUBLES_EQUAL(m_pDoc->GetValue(10,11,0), 1.0);
    OUString aFormula;
    m_pDoc->GetFormula(10,10,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=COLUMN($A$1)"));
    m_pDoc->GetFormula(10,11,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$A$1+L12"));
    m_pDoc->GetFormula(10,12,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$Sheet2.K11"));
    m_pDoc->GetFormula(10,13,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$Sheet2.$A$1"));
    m_pDoc->GetFormula(10,14,0, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$Sheet2.K$1"));
}

void Test::testCopyPasteFormulasExternalDoc()
{
    OUString aDocName("file:///source.fake");
    SfxMedium* pMedium = new SfxMedium(aDocName, STREAM_STD_READWRITE);
    getDocShell().DoInitNew(pMedium);
    m_pDoc = getDocShell().GetDocument();

    ScDocShellRef xExtDocSh = new ScDocShell;
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("ExtSheet1");
    OUString aExtSh2Name("ExtSheet2");
    SfxMedium* pMed = new SfxMedium(aExtDocName, STREAM_STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != NULL);

    ScDocument* pExtDoc = xExtDocSh->GetDocument();
    pExtDoc->InsertTab(0, aExtSh1Name);
    pExtDoc->InsertTab(1, aExtSh2Name);

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetString(0,0,0, "=COLUMN($A$1)");
    m_pDoc->SetString(0,1,0, "=$A$1+B2" );
    m_pDoc->SetString(0,2,0, "=$Sheet2.A1");
    m_pDoc->SetString(0,3,0, "=$Sheet2.$A$1");
    m_pDoc->SetString(0,4,0, "=$Sheet2.A$1");
    m_pDoc->SetString(0,5,0, "=$Sheet1.$A$1");

    ScRange aRange(0,0,0,0,5,0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    ScDocument* pClipDoc = new ScDocument(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, pClipDoc, &aMark);

    sal_uInt16 nFlags = IDF_ALL;
    aRange = ScRange(1,1,1,1,6,1);
    ScMarkData aMarkData2;
    aMarkData2.SetMarkArea(aRange);
    pExtDoc->CopyFromClip(aRange, aMarkData2, nFlags, NULL, pClipDoc);

    OUString aFormula;
    pExtDoc->GetFormula(1,1,1, aFormula);
    //adjust absolute refs pointing to the copy area
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=COLUMN($B$2)"));
    pExtDoc->GetFormula(1,2,1, aFormula);
    //adjust absolute refs and keep relative refs
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$B$2+C3"));
    pExtDoc->GetFormula(1,3,1, aFormula);
    // make absolute sheet refs external refs
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("='file:///source.fake'#$Sheet2.B2"));
    pExtDoc->GetFormula(1,4,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("='file:///source.fake'#$Sheet2.$A$1"));
    pExtDoc->GetFormula(1,5,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("='file:///source.fake'#$Sheet2.B$1"));
    pExtDoc->GetFormula(1,6,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$ExtSheet2.$B$2"));
}

void Test::testFindAreaPosVertical()
{
    const char* aData[][3] = {
        {   0, "1", "1" },
        { "1",   0, "1" },
        { "1", "1", "1" },
        {   0, "1", "1" },
        { "1", "1", "1" },
        { "1",   0, "1" },
        { "1", "1", "1" },
    };

    m_pDoc->InsertTab(0, "Test1");
    clearRange( m_pDoc, ScRange(0, 0, 0, 1, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

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

    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(MAXROW), nRow);
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
    const char* aData[][7] = {
        { "", "1", "1", "", "1", "1", "1" },
        { "", "", "1", "1", "1", "", "1" }, };

    OUString aTabName1("test1");
    m_pDoc->InsertTab(0, aTabName1);
    clearRange( m_pDoc, ScRange(0, 0, 0, 7, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

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
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(MAXCOL), nCol);

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

// regression test fo fdo#53814, sorting doens't work as expected
// if cells in the sort are referenced by formulas
void Test::testSortWithFormulaRefs()
{
    ScDocument* pDoc = getDocShell().GetDocument();
    OUString aTabName1("List1");
    OUString aTabName2("List2");
    pDoc->InsertTab(0, aTabName1);
    pDoc->InsertTab(1, aTabName2);

    const char* aFormulaData[6] = {
        "=IF($List1.A2<>\"\",$List1.A2,\"\")",
        "=IF($List1.A3<>\"\",$List1.A3,\"\")",
        "=IF($List1.A4<>\"\",$List1.A4,\"\")",
        "=IF($List1.A5<>\"\",$List1.A5,\"\")",
        "=IF($List1.A6<>\"\",$List1.A6,\"\")",
        "=IF($List1.A7<>\"\",$List1.A7,\"\")",
    };

    const char* aTextData[4] = {
        "bob",
        "tim",
        "brian",
        "larry",
    };

    const char* aResults[ 6 ] = {
        "bob",
        "brian",
        "larry",
        "tim",
        "",
        "",
    };
    // insert data to sort
    SCROW nStart = 1, nEnd = 4;
    for ( SCROW i = nStart; i <= nEnd; ++i )
        pDoc->SetString( 0, i, 0, OUString::createFromAscii(aTextData[i-1]) );
    // insert forumulas
    nStart = 0;
    nEnd = SAL_N_ELEMENTS(aFormulaData);
    for ( SCROW i = nStart; i < nEnd; ++i )
        pDoc->SetString( 0, i, 1, OUString::createFromAscii(aFormulaData[i]) );

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 0;
    aSortData.nRow1 = 1;
    aSortData.nRow2 = 7;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;

    pDoc->Sort(0, aSortData, false, NULL);

    nEnd = SAL_N_ELEMENTS( aResults );
    for ( SCROW i = nStart; i < nEnd; ++i )
    {
        OUString sResult = pDoc->GetString( 0, i + 1, 0);
        CPPUNIT_ASSERT_EQUAL( OUString::createFromAscii( aResults[ i ] ), sResult );
    }
    pDoc->DeleteTab(0);
    pDoc->DeleteTab(1);
}

void Test::testSort()
{
    OUString aTabName1("test1");
    m_pDoc->InsertTab(0, aTabName1);

    ScRange aDataRange;
    ScAddress aPos(0,0,0);
    {
        const char* aData[][2] = {
            { "2", "4" },
            { "4", "1" },
            { "1", "2" },
            { "1", "23" },
        };

        clearRange(m_pDoc, ScRange(0, 0, 0, 1, SAL_N_ELEMENTS(aData), 0));
        aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
        CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);
    }

    // Insert note in cell B2.
    OUString aHello("Hello");
    OUString aJimBob("Jim Bob");
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    ScSortParam aSortData;
    aSortData.nCol1 = 1;
    aSortData.nCol2 = 1;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 2;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 1;
    aSortData.maKeyState[0].bAscending = true;

    m_pDoc->Sort(0, aSortData, false, NULL);
    double nVal = m_pDoc->GetValue(1,0,0);
    ASSERT_DOUBLES_EQUAL(nVal, 1.0);

    // check that note is also moved
    pNote = m_pDoc->GetNotes(0)->findByAddress( 1, 0 );
    CPPUNIT_ASSERT(pNote);

    clearRange(m_pDoc, ScRange(0, 0, 0, 1, 9, 0)); // Clear A1:B10.
    {
        // 0 = empty cell
        const char* aData[][1] = {
            { "Title" },
            { 0 },
            { 0 },
            { "12" },
            { "b" },
            { "1" },
            { "9" },
            { "123" }
        };

        aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
        CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);
    }

    aSortData.nCol1 = aDataRange.aStart.Col();
    aSortData.nCol2 = aDataRange.aEnd.Col();
    aSortData.nRow1 = aDataRange.aStart.Row();
    aSortData.nRow2 = aDataRange.aEnd.Row();
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].nField = 0;
    m_pDoc->Sort(0, aSortData, false, NULL);

    // Title should stay at the top, numbers should be sorted numerically,
    // numbers always come before strings, and empty cells always occur at the
    // end.
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(OUString("1"), m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(OUString("9"), m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(OUString("12"), m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(OUString("123"), m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(OUString("b"), m_pDoc->GetString(aPos));
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(aPos));

    m_pDoc->DeleteTab(0);
}

void Test::testShiftCells()
{
    m_pDoc->InsertTab(0, "foo");

    OUString aTestVal("Some Text");

    // Text into cell E5.
    m_pDoc->SetString(4, 3, 0, aTestVal);

    // Insert cell at D5. This should shift the string cell to right.
    m_pDoc->InsertCol(3, 0, 3, 0, 3, 1);
    OUString aStr = m_pDoc->GetString(5, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("We should have a string cell here.", aStr == aTestVal);
    CPPUNIT_ASSERT_MESSAGE("D5 is supposed to be blank.", m_pDoc->IsBlockEmpty(0, 3, 4, 3, 4));

    // Delete cell D5, to shift the text cell back into D5.
    m_pDoc->DeleteCol(3, 0, 3, 0, 3, 1);
    aStr = m_pDoc->GetString(4, 3, 0);
    CPPUNIT_ASSERT_MESSAGE("We should have a string cell here.", aStr == aTestVal);
    CPPUNIT_ASSERT_MESSAGE("E5 is supposed to be blank.", m_pDoc->IsBlockEmpty(0, 4, 4, 4, 4));

    m_pDoc->DeleteTab(0);
}

void Test::testDeleteRow()
{
    ScDocument* pDoc = getDocShell().GetDocument();
    OUString aSheet1("Sheet1");
    pDoc->InsertTab(0, aSheet1);

    OUString aHello("Hello");
    OUString aJimBob("Jim Bob");
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    pDoc->DeleteRow(0, 0, MAXCOL, 0, 1, 1);

    CPPUNIT_ASSERT(m_pDoc->GetNotes(0)->empty());
    pDoc->DeleteTab(0);
}

void Test::testDeleteCol()
{
    ScDocument* pDoc = getDocShell().GetDocument();
    OUString aSheet1("Sheet1");
    pDoc->InsertTab(0, aSheet1);

    OUString aHello("Hello");
    OUString aJimBob("Jim Bob");
    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetNotes(rAddr.Tab())->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, aHello);
    pNote->SetAuthor(aJimBob);

    pDoc->DeleteCol(0, 0, MAXROW, 0, 1, 1);

    CPPUNIT_ASSERT(m_pDoc->GetNotes(0)->empty());
    pDoc->DeleteTab(0);
}

void Test::testAnchoredRotatedShape()
{
    OUString aTabName("TestTab");
    m_pDoc->InsertTab(0, aTabName);
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != NULL);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != NULL);
    m_pDoc->SetRowHeightRange( 0, MAXROW, 0, sc::HMMToTwips( 1000 ) );
    const long TOLERANCE = 30; //30 hmm
    for ( SCCOL nCol = 0; nCol < MAXCOL; ++nCol )
        m_pDoc->SetColWidth( nCol, 0, sc::HMMToTwips( 1000 ) );
    {
        //Add a rect
        Rectangle aRect( 4000, 5000, 10000, 7000 );

        Rectangle aRotRect( 6000, 3000, 8000, 9000 );
        SdrRectObj *pObj = new SdrRectObj(aRect);
        pPage->InsertObject(pObj);
        Point aRef1(pObj->GetSnapRect().Center());
        int nAngle = 9000; //90 deg.
        double nSin=sin(nAngle*nPi180);
        double nCos=cos(nAngle*nPi180);
        pObj->Rotate(aRef1,nAngle,nSin,nCos);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);

        Rectangle aSnap = pObj->GetSnapRect();
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.GetHeight(), aSnap.GetHeight(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.GetWidth(), aSnap.GetWidth(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.Left(), aSnap.Left(), TOLERANCE ) );
        CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRotRect.Top(), aSnap.Top(), TOLERANCE ) );

        ScDrawObjData aAnchor;
        ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );

        aAnchor.maStart = pData->maStart;
        aAnchor.maEnd = pData->maEnd;

        m_pDoc->SetDrawPageSize(0);

        // increase row 5 by 2000 hmm
        m_pDoc->SetRowHeight( 5, 0, sc::HMMToTwips( 3000 ) );
        // increase col 6 by 1000 hmm
        m_pDoc->SetColWidth( 6, 0, sc::HMMToTwips( 2000 ) );

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
    boost::scoped_ptr<ScColumnTextWidthIterator> pIter(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
    CPPUNIT_ASSERT_MESSAGE("Column should have no text widths stored.", !pIter->hasCell());

    // Sheet only has one cell.
    m_pDoc->SetString(0, 0, 0, "Only one cell");
    pIter.reset(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
    CPPUNIT_ASSERT_MESSAGE("Column should have a cell.", pIter->hasCell());
    SCROW nTestRow = 0;
    CPPUNIT_ASSERT_EQUAL(nTestRow, pIter->getPos());

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
        size_t n = SAL_N_ELEMENTS(aRows);
        for (size_t i = 0; i < n; ++i, pIter->next())
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
        size_t n = SAL_N_ELEMENTS(aRows);
        for (size_t i = 0; i < n; ++i, pIter->next())
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
        size_t n = SAL_N_ELEMENTS(aRows);
        for (size_t i = 0; i < n; ++i, pIter->next())
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
        size_t n = SAL_N_ELEMENTS(aRows);
        for (size_t i = 0; i < n; ++i, pIter->next())
        {
            CPPUNIT_ASSERT_MESSAGE("Cell expected, but not there.", pIter->hasCell());
            CPPUNIT_ASSERT_EQUAL(aRows[i], pIter->getPos());
        }
        CPPUNIT_ASSERT_MESSAGE("Iterator should have ended.", !pIter->hasCell());
    }

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaGrouping()
{
    if ( !getenv("SC_FORMULAGROUP") )
        return;

    static const struct {
        const char *pFormula[3];
        bool  bGroup[3];
    } aGroupTests[] = {
        { { "=B1", "=B2", "" }, // relative reference
          { true, true, false } },
        { { "=B1", "=B2", "=B3" },
          { true, true, true } },
        { { "=B1", "", "=B3" }, // a gap
          { false, false, false } },
        { { "=$B$1", "=$B$1", "" }, // absolute reference
          { true, true, false } },
        { { "=$Z$10", "=$Z$10", "=$Z$10" },
          { true, true, true } },
        { { "=C1+$Z$10", "=C2+$Z$10", "=C3+$Z$10" }, // mixture
          { true, true, true } },
        { { "=C1+$Z$11", "=C2+$Z$12", "=C3+$Z$12" }, // mixture
          { false, true, true } },
        { { "=SUM(B1)",  "", "=SUM(B3)" },    // a gap
          { false, false, false } },
    };

    m_pDoc->InsertTab( 0, "sheet" );

    for (size_t i = 0; i < SAL_N_ELEMENTS(aGroupTests); ++i)
    {
        for (size_t j = 0; j < SAL_N_ELEMENTS(aGroupTests[0].pFormula); ++j)
        {
            OUString aFormula = OUString::createFromAscii(aGroupTests[i].pFormula[j]);
            m_pDoc->SetString(0, static_cast<SCROW>(j), 0, aFormula);
        }
        m_pDoc->RebuildFormulaGroups();

        for (size_t j = 0; j < SAL_N_ELEMENTS(aGroupTests[0].pFormula); ++j)
        {
            ScRefCellValue aCell;
            aCell.assign(*m_pDoc, ScAddress(0, static_cast<SCROW>(j), 0));
            if (aCell.isEmpty())
            {
                CPPUNIT_ASSERT_MESSAGE("invalid empty cell", !aGroupTests[i].bGroup[j]);
                continue;
            }
            CPPUNIT_ASSERT_MESSAGE("Cell expected, but not there.", !aCell.isEmpty());
            CPPUNIT_ASSERT_MESSAGE("Cell wrong type.",
                                   aCell.meType == CELLTYPE_FORMULA);
            ScFormulaCell *pCur = aCell.mpFormula;

            if( !!pCur->GetCellGroup().get() ^ aGroupTests[i].bGroup[j] )
            {
                cout << "expected group test " << i << " at row " << j << " to be "
                    << aGroupTests[i].bGroup[j] << " but is " << !!pCur->GetCellGroup().get() << endl;
                CPPUNIT_FAIL("Failed");
            }
        }
    }
}

void Test::testCondFormatINSDEL()
{
    // fdo#62206
    m_pDoc->InsertTab(0, "Test");
    ScConditionalFormatList* pList = m_pDoc->GetCondFormList(0);

    ScConditionalFormat* pFormat = new ScConditionalFormat(1, m_pDoc);
    ScRangeList aRangeList(ScRange(0,0,0,0,3,0));
    pFormat->AddRange(aRangeList);
    ScCondFormatEntry* pEntry = new ScCondFormatEntry(SC_COND_DIRECT,"=B2","",m_pDoc,ScAddress(0,0,0),ScGlobal::GetRscString(STR_STYLENAME_RESULT));
    pFormat->AddEntry(pEntry);

    m_pDoc->AddCondFormatData(pFormat->GetRange(), 0, 1);
    pList->InsertNew(pFormat);

    m_pDoc->InsertCol(0,0,MAXROW,0,0,2);
    const ScRangeList& rRange = pFormat->GetRange();
    CPPUNIT_ASSERT(rRange == ScRange(2,0,0,2,3,0));

    OUString aExpr = pEntry->GetExpression(ScAddress(2,0,0), 0);
    CPPUNIT_ASSERT_EQUAL(aExpr, OUString("D2"));

    m_pDoc->DeleteTab(0);
}

void Test::printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption)
{
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    SheetPrinter printer(nRow2 - nRow1 + 1, nCol2 - nCol1 + 1);
    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            OUString aVal = pDoc->GetString(nCol, nRow, rRange.aStart.Tab());
            printer.set(nRow-nRow1, nCol-nCol1, aVal);
        }
    }
    printer.print(pCaption);
}

void Test::clearRange(ScDocument* pDoc, const ScRange& rRange)
{
    ScMarkData aMarkData;
    aMarkData.SetMarkArea(rRange);
    pDoc->DeleteArea(
        rRange.aStart.Col(), rRange.aStart.Row(),
        rRange.aEnd.Col(), rRange.aEnd.Row(), aMarkData, IDF_CONTENTS);
}

void Test::copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc)
{
    ScClipParam aClipParam(rRange, false);
    ScMarkData aMark;
    aMark.SetMarkArea(rRange);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aMark);
}

void Test::pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc)
{
    ScMarkData aMark;
    aMark.SetMarkArea(rDestRange);
    pDestDoc->CopyFromClip(rDestRange, aMark, IDF_ALL, NULL, pClipDoc);
}

ScUndoPaste* Test::createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pUndoDoc)
{
    ScDocument* pDoc = rDocSh.GetDocument();
    ScMarkData aMarkData;
    aMarkData.SetMarkArea(rRange);
    ScRefUndoData* pRefUndoData = new ScRefUndoData(pDoc);

    return new ScUndoPaste(
        &rDocSh, rRange, aMarkData, pUndoDoc, NULL, IDF_ALL, pRefUndoData, false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
