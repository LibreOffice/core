/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucalc.hxx"

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/time.h>

#include "scdll.hxx"
#include "formulacell.hxx"
#include "simpleformulacalc.hxx"
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
#include "clipcontext.hxx"

#include "docsh.hxx"
#include "docfunc.hxx"
#include "dbdocfun.hxx"
#include "funcdesc.hxx"
#include "externalrefmgr.hxx"

#include "calcconfig.hxx"
#include "interpre.hxx"
#include "columniterator.hxx"
#include "types.hxx"
#include "fillinfo.hxx"
#include "globstr.hrc"
#include "tokenarray.hxx"
#include "scopetools.hxx"
#include "dociter.hxx"
#include "queryparam.hxx"
#include "edittextiterator.hxx"
#include "editutil.hxx"
#include "cellform.hxx"
#include <asciiopt.hxx>
#include <impex.hxx>
#include <columnspanset.hxx>
#include <docoptio.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <globalnames.hxx>
#include <inputopt.hxx>

#include <editable.hxx>
#include <bcaslot.hxx>
#include <sharedformula.hxx>

#include <formula/IFunctionDescription.hxx>

#include <o3tl/make_unique.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>

#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <svl/srchitem.hxx>
#include <svl/sharedstringpool.hxx>

#include <sfx2/docfile.hxx>

#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include <com/sun/star/i18n/TransliterationModules.hpp>

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

Test::Test() :
    m_pImpl(new TestImpl),
    m_pDoc(nullptr)
{
}

Test::~Test()
{
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
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    m_pImpl->m_xDocShell->SetIsInUcalc();
    m_pImpl->m_xDocShell->DoInitUnitTest();
    m_pDoc = &m_pImpl->m_xDocShell->GetDocument();
}

void Test::tearDown()
{
    m_pImpl->m_xDocShell->DoClose();
    m_pImpl->m_xDocShell.Clear();
    BootstrapFixture::tearDown();
}

void Test::testCollator()
{
    CollatorWrapper* p = ScGlobal::GetCollator();
    sal_Int32 nRes = p->compareString("A", "B");
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
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A1 and purge again.
    clearRange(m_pDoc, ScAddress(0,0,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A2 and purge again.
    clearRange(m_pDoc, ScAddress(0,1,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rPool.getCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCountIgnoreCase());

    // Clear A3 and purge again.
    clearRange(m_pDoc, ScAddress(0,2,0));
    rPool.purge();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPool.getCount());
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
    rEE.SetText("Andy and Bruce");

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

    rEE.SetText("ANDY and BRUCE");
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
        bool check( ScDocument& rSrcDoc, ScDocument& rCopyDoc )
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
    aUndoDoc.InitUndo(m_pDoc, 0, 0);

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
    aRL.Append(ScRange(1,1,0,3,10,0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("List should have one range.", size_t(1), aRL.size());
    const ScRange* p = aRL[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get the range object.", p);
    CPPUNIT_ASSERT_MESSAGE("Wrong range.", p->aStart == ScAddress(1,1,0) && p->aEnd == ScAddress(3,10,0));

    // TODO: Add more tests here.

    m_pDoc->DeleteTab(0);
}

void Test::testMarkData()
{
    ScMarkData aMarkData;

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
    rEE.SetText("Rich Text");
    m_pDoc->SetEditText(ScAddress(3,4,0), rEE.CreateTextObject());

    // Insert Another string into D6.
    m_pDoc->SetString(ScAddress(3,5,0), "E");

    // Select B2:B8 & D2:D8 disjoint region.
    ScRangeList aRanges;
    aRanges.Append(ScRange(1,1,0,1,7,0)); // B2:B8
    aRanges.Append(ScRange(3,1,0,3,7,0)); // D2:D8
    ScMarkData aMark;
    aMark.MarkFromRangeList(aRanges, true);

    struct Check
    {
        ScSubTotalFunc meFunc;
        double mfExpected;
    };

    {
        Check aChecks[] =
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
        Check aChecks[] =
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
    ScMarkData aEmpty;

    {
        // D3 (numeric cell containing 5.)
        ScAddress aPos(3, 2, 0);

        Check aChecks[] =
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

        Check aChecks[] =
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
    aRanges.Append(ScRange(1,1,0)); // B2
    aRanges.Append(ScRange(2,2,0)); // C3
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

void Test::testCopyToDocument()
{
    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "src"));

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

    // verify note
    CPPUNIT_ASSERT_MESSAGE("There should be a note in A1 destDocument", aDestDoc.HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The notes content should be the same on both documents",
            m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(), aDestDoc.GetNote(ScAddress(0, 0, 0))->GetText());

    m_pDoc->DeleteTab(0);
}

namespace {

struct HoriIterCheck
{
    SCCOL nCol;
    SCROW nRow;
    const char* pVal;
};

template<size_t Size>
bool checkHorizontalIterator(ScDocument* pDoc, const char* pData[][Size], size_t nDataCount, const HoriIterCheck* pChecks, size_t nCheckCount)
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

        if (OUString::createFromAscii(pChecks[i].pVal) != pCell->getString(pDoc))
        {
            cerr << "String mismatch " << pChecks[i].pVal << " vs. " <<
                OUStringToOString(pCell->getString(pDoc), RTL_TEXTENCODING_UTF8).getStr() << endl;
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
            CPPUNIT_FAIL("Failed on test mixed.");
    }

    {
        // Raw data - 'hole' data
        const char* aData[][2] = {
            { "A", "B" },
            { "C",  nullptr  },
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
            CPPUNIT_FAIL("Failed on test hole.");
    }

    {
        // Very holy data
        const char* aData[][2] = {
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

        HoriIterCheck aChecks[] = {
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
            m_pDoc, aData, SAL_N_ELEMENTS(aData), aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test holy.");
    }

    {
        // Degenerate case
        const char* aData[][2] = {
            {  nullptr,   nullptr },
            {  nullptr,   nullptr },
            {  nullptr,   nullptr },
        };

        bool bRes = checkHorizontalIterator(
            m_pDoc, aData, SAL_N_ELEMENTS(aData), nullptr, 0);

        if (!bRes)
            CPPUNIT_FAIL("Failed on test degenerate.");
    }

    {
        // Data at end
        const char* aData[][2] = {
            {  nullptr,   nullptr },
            {  nullptr,   nullptr },
            {  nullptr,  "A" },
        };

        HoriIterCheck aChecks[] = {
            { 1, 2, "A" },
        };

        bool bRes = checkHorizontalIterator(
            m_pDoc, aData, SAL_N_ELEMENTS(aData), aChecks, SAL_N_ELEMENTS(aChecks));

        if (!bRes)
            CPPUNIT_FAIL("Failed on test at end.");
    }

    {
        // Data in middle
        const char* aData[][2] = {
            {  nullptr,   nullptr  },
            {  nullptr,   nullptr  },
            {  nullptr,  "A" },
            {  nullptr,  "1" },
            {  nullptr,   nullptr  },
        };

        HoriIterCheck aChecks[] = {
            { 1, 2, "A" },
            { 1, 3, "1" },
        };

        bool bRes = checkHorizontalIterator(
            m_pDoc, aData, SAL_N_ELEMENTS(aData), aChecks, SAL_N_ELEMENTS(aChecks));

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
        size_t nCheckLen = SAL_N_ELEMENTS(aChecks);
        ScValueIterator aIter(m_pDoc, ScRange(1,2,0,3,2,0));
        bool bHas = false;
        size_t nCheckPos = 0;
        double fVal;
        FormulaError nErr;
        for (bHas = aIter.GetFirst(fVal, nErr); bHas; bHas = aIter.GetNext(fVal, nErr), ++nCheckPos)
        {
            CPPUNIT_ASSERT_MESSAGE("Iteration longer than expected.", nCheckPos < nCheckLen);
            CPPUNIT_ASSERT_EQUAL(aChecks[nCheckPos], fVal);
            CPPUNIT_ASSERT_EQUAL(0, (int)nErr);
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
        size_t nCheckLen = SAL_N_ELEMENTS(aChecks);

        ScHorizontalAttrIterator aIter(m_pDoc, 0, 0, 0, 5, 5);
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

    formula::FormulaToken* pToken = pTokens->First();
    if (!pToken || pToken->GetType() != formula::svSingleRef)
    {
        cerr << "Not a single reference token." << endl;
        return nullptr;
    }

    return pToken;
}

bool checkRelativeRefToken(ScDocument& rDoc, const ScAddress& rPos, SCsCOL nRelCol, SCsROW nRelRow)
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
    double val;
    m_pDoc->GetValue(0, 0, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 2.0, val);

    // Now function with missing parameters.  Missing values should be treated
    // as zeros.
    m_pDoc->SetString(0, 0, 0, "=AVERAGE(1;;;)");
    m_pDoc->CalcFormulaTree(false, false);
    m_pDoc->GetValue(0, 0, 0, val);
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
    m_pDoc->GetValue(0, 0, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 2, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.0, val);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    m_pDoc->GetValue(0, 4, 0, val);
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
    m_pDoc->GetValue(0, 2, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.0, val);
    aVal = m_pDoc->GetString( 0, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect result", OUString("#VALUE!"), aVal);
    m_pDoc->GetValue(0, 4, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 7.4, val);

    // With "Convert only unambiguous" and "Empty string as zero"=True option.
    aConfig.meStringConversion = ScCalcConfig::StringConversion::UNAMBIGUOUS;
    aConfig.mbEmptyStringAsZero = true;
    m_pDoc->SetCalcConfig(aConfig);
    m_pDoc->CalcAll();
    m_pDoc->GetValue(0, 0, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 2, 0, val);
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
    m_pDoc->GetValue(0, 2, 0, val);
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
    m_pDoc->GetValue(0, 0, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 1, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 2, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 3, 0, val);
    ASSERT_DOUBLES_EQUAL_MESSAGE("incorrect result", 3.0, val);
    m_pDoc->GetValue(0, 4, 0, val);
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
    RangeNameDef aNames[] = {
        { "Divisor",  "$Sheet1.$A$1:$A$1048576", 1 },
        { "MyRange1", "$Sheet1.$A$1:$A$100",     2 },
        { "MyRange2", "$Sheet1.$B$1:$B$100",     3 },
        { "MyRange3", "$Sheet1.$C$1:$C$100",     4 }
    };

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (0, "Sheet1"));

    m_pDoc->SetValue (0, 0, 0, 101);

    ScRangeName* pNames = new ScRangeName;
    bool bSuccess = insertRangeNames(m_pDoc, pNames, aNames, aNames + SAL_N_ELEMENTS(aNames));
    CPPUNIT_ASSERT_MESSAGE("Failed to insert range names.", bSuccess);
    m_pDoc->SetRangeName(pNames);

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

    double result;
    m_pDoc->GetValue (1, 0, 0, result);
    ASSERT_DOUBLES_EQUAL_MESSAGE ("calculation failed", 1.0, result);

    // Test copy-ability of range names.
    ScRangeName* pCopiedRanges = new ScRangeName(*pNewRanges);
    m_pDoc->SetRangeName(pCopiedRanges);
    // Make sure the index lookup still works.
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i)
    {
        const ScRangeData* p = pCopiedRanges->findByIndex(aNames[i].mnIndex);
        CPPUNIT_ASSERT_MESSAGE("lookup of range name by index failed with the copied instance.", p);
        OUString aName = p->GetName();
        CPPUNIT_ASSERT_MESSAGE("wrong range name is retrieved with the copied instance.", aName.equalsAscii(aNames[i].mpName));
    }

    // Test using an other-sheet-local name, scope Sheet1.
    ScRangeData* pLocal1 = new ScRangeData( m_pDoc, "local1", ScAddress(0,0,0));
    ScRangeData* pLocal2 = new ScRangeData( m_pDoc, "local2", "$Sheet1.$A$1");
    ScRangeData* pLocal3 = new ScRangeData( m_pDoc, "local3", "Sheet1.$A$1");
    ScRangeData* pLocal4 = new ScRangeData( m_pDoc, "local4", "$A$1"); // implicit relative sheet reference
    ScRangeName* pLocalRangeName1 = new ScRangeName;
    pLocalRangeName1->insert(pLocal1);
    pLocalRangeName1->insert(pLocal2);
    pLocalRangeName1->insert(pLocal3);
    pLocalRangeName1->insert(pLocal4);
    m_pDoc->SetRangeName(0, pLocalRangeName1);

    CPPUNIT_ASSERT_MESSAGE ("failed to insert sheet", m_pDoc->InsertTab (1, "Sheet2"));

    // Use other-sheet-local name of Sheet1 on Sheet2.
    ScAddress aPos(1,0,1);
    OUString aFormula("=Sheet1.local1+Sheet1.local2+Sheet1.local3+Sheet1.local4");
    m_pDoc->SetString(aPos, aFormula);
    OUString aString;
    m_pDoc->GetFormula(1,0,1, aString);
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

    RangeNameDef aNames[] = {
        { "MyRange1", "$Test.$A$1:$A$100", 1 },
        { "MyRange2", "$Test.$B$1:$B$100", 2 },
        { "MyRange3", "$Test.$C$1:$C$100", 3 }
    };

    ScRangeName* pNames = new ScRangeName;
    bool bSuccess = insertRangeNames(m_pDoc, pNames, aNames, aNames + SAL_N_ELEMENTS(aNames));
    CPPUNIT_ASSERT_MESSAGE("Failed to insert range names.", bSuccess);
    m_pDoc->SetRangeName(pNames);

    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
    ScAddress aPos(1,1,0);
    rDocFunc.InsertNameList(aPos, true);

    for (size_t i = 0; i < SAL_N_ELEMENTS(aNames); ++i, aPos.IncRow())
    {
        OUString aName = m_pDoc->GetString(aPos);
        CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(aNames[i].mpName), aName);
        ScAddress aExprPos = aPos;
        aExprPos.IncCol();
        OUString aExpr = m_pDoc->GetString(aExprPos);
        OUString aExpected = "=";
        aExpected += OUString::createFromAscii(aNames[i].mpExpr);
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
    for (sal_uInt32 i = 0; i < SAL_N_ELEMENTS(aTests); i++) {
        OUString aStr(aTests[i].pStr, strlen (aTests[i].pStr), RTL_TEXTENCODING_UTF8);
        double nValue = 0.0;
        bool bResult = ScStringUtil::parseSimpleNumber
                (aStr, aTests[i].eSep == English ? '.' : ',',
                 aTests[i].eSep == English ? ',' : '.',
                 nValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE ("CSV numeric detection failure", aTests[i].bResult, bResult);
        CPPUNIT_ASSERT_EQUAL_MESSAGE ("CSV numeric value failure", aTests[i].nValue, nValue);
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of numeric type", (int)ScMatValType::Value, (int)rVal.nType);
        ASSERT_DOUBLES_EQUAL_MESSAGE("element value must be zero", 0.0, rVal.fVal);
    }
};

struct PartiallyFilledZeroMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of numeric type", (int)ScMatValType::Value, (int)rVal.nType);
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of empty type", (int)ScMatValType::Empty, (int)rVal.nType);
        ASSERT_DOUBLES_EQUAL_MESSAGE("value of \"empty\" element is expected to be zero", 0.0, rVal.fVal);
    }
};

struct PartiallyFilledEmptyMatrix
{
    void operator() (SCSIZE nCol, SCSIZE nRow, const ScMatrixValue& rVal) const
    {
        if (nCol == 1 && nRow == 1)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of boolean type", (int)ScMatValType::Boolean, (int)rVal.nType);
            ASSERT_DOUBLES_EQUAL_MESSAGE("element value is not what is expected", 1.0, rVal.fVal);
        }
        else if (nCol == 4 && nRow == 5)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of value type", (int)ScMatValType::Value, (int)rVal.nType);
            ASSERT_DOUBLES_EQUAL_MESSAGE("element value is not what is expected", -12.5, rVal.fVal);
        }
        else if (nCol == 8 && nRow == 2)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of value type", (int)ScMatValType::String, (int)rVal.nType);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element value is not what is expected", OUString("Test"), rVal.aStr.getString());
        }
        else if (nCol == 8 && nRow == 11)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of empty path type", (int)ScMatValType::EmptyPath, (int)rVal.nType);
            ASSERT_DOUBLES_EQUAL_MESSAGE("value of \"empty\" element is expected to be zero", 0.0, rVal.fVal);
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("element is not of empty type", (int)ScMatValType::Empty, (int)rVal.nType);
            ASSERT_DOUBLES_EQUAL_MESSAGE("value of \"empty\" element is expected to be zero", 0.0, rVal.fVal);
        }
    }
};

void Test::testMatrix()
{
    svl::SharedStringPool& rPool = m_pDoc->GetSharedStringPool();
    ScMatrixRef pMat, pMat2;

    // First, test the zero matrix type.
    pMat = new ScFullMatrix(0, 0, 0.0);
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
    pMat = new ScFullMatrix(2, 2, 0.0);

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
    pMat = new ScFullMatrix(10, 20);
    pMat->GetDimensions(nC, nR);
    CPPUNIT_ASSERT_MESSAGE("matrix size is not as expected", nC == 10 && nR == 20);
    checkMatrixElements<AllEmptyMatrix>(*pMat);

    pMat->PutBoolean(true, 1, 1);
    pMat->PutDouble(-12.5, 4, 5);
    pMat->PutString(rPool.intern("Test"), 8, 2);
    pMat->PutEmptyPath(8, 11);
    checkMatrixElements<PartiallyFilledEmptyMatrix>(*pMat);

    // Test resizing.
    pMat = new ScFullMatrix(0, 0);
    pMat->Resize(2, 2, 1.5);
    pMat->PutEmpty(1, 1);

    CPPUNIT_ASSERT_EQUAL(1.5, pMat->GetDouble(0, 0));
    CPPUNIT_ASSERT_EQUAL(1.5, pMat->GetDouble(0, 1));
    CPPUNIT_ASSERT_EQUAL(1.5, pMat->GetDouble(1, 0));
    CPPUNIT_ASSERT_MESSAGE("PutEmpty() call failed.", pMat->IsEmpty(1, 1));

    // Max and min values.
    pMat = new ScFullMatrix(2, 2, 0.0);
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
    pMat = new ScFullMatrix(2, 2, 10.0);
    pMat->PutBoolean(false, 0, 0);
    pMat->PutDouble(12.5, 1, 1);
    CPPUNIT_ASSERT_EQUAL(0.0, pMat->GetMinValue(false));
    CPPUNIT_ASSERT_EQUAL(12.5, pMat->GetMaxValue(false));

    // Convert matrix into a linear double array. String elements become NaN
    // and empty elements become 0.
    pMat = new ScFullMatrix(3, 3);
    pMat->PutDouble(2.5, 0, 0);
    pMat->PutDouble(1.2, 0, 1);
    pMat->PutString(rPool.intern("A"), 1, 1);
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

    pMat2 = new ScFullMatrix(3, 3, 10.0);
    pMat2->PutString(rPool.intern("B"), 1, 0);
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

void Test::testMatrixComparisonWithErrors()
{
    m_pDoc->InsertTab(0, "foo");

    // Insert the source values in A1:A2.
    m_pDoc->SetString(0, 0, 0, "=1/0");
    m_pDoc->SetValue( 0, 1, 0, 1.0);

    // Create a matrix formula in B3:B4 referencing A1:A2 and doing a greater
    // than comparison on it's values. Error value must be propagated.
    ScMarkData aMark;
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
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula( 0,0, 1,0, aMark, "=IF({1,0};TRUE();42)");  // {TRUE,42}
    m_pDoc->InsertMatrixFormula( 0,1, 1,1, aMark, "=IF({0,1};TRUE();42)");  // {42,1} aim for {42,TRUE}
    m_pDoc->InsertMatrixFormula( 0,2, 1,2, aMark, "=IF({1,0};42;FALSE())"); // {42,0} aim for {42,FALSE}
    m_pDoc->InsertMatrixFormula( 0,3, 1,3, aMark, "=IF({0,1};42;FALSE())"); // {FALSE,42}

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
    ScMarkData aMark;
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
    ScMarkData aMark;
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
                               MM_FORMULA, static_cast<ScMatrixMode>(pFC->GetMatrixFlag()));

    pFC = m_pDoc->GetFormulaCell(ScAddress(0,3,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A4 should be matrix reference.",
                               MM_REFERENCE, static_cast<ScMatrixMode>(pFC->GetMatrixFlag()));

    // Check to make sure A3:A4 combined is editable.
    ScEditableTester aTester;
    aTester.TestSelection(m_pDoc, aMark);
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
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aSrc), aStr);
    // copy to self - why not ?
    m_pDoc->CopyCellToDocument(aSrc,aDest,*m_pDoc);
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aDest), aStr);
}

void Test::testSheetCopy()
{
    m_pDoc->InsertTab(0, "TestTab");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.",
                               static_cast<SCTAB>(1), m_pDoc->GetTableCount());

    // Insert text in A1.
    m_pDoc->SetString(ScAddress(0,0,0), "copy me");

    // Insert edit cells in B1:B3.
    ScFieldEditEngine& rEE = m_pDoc->GetEditEngine();
    rEE.SetText("Edit 1");
    m_pDoc->SetEditText(ScAddress(1,0,0), rEE.CreateTextObject());
    rEE.SetText("Edit 2");
    m_pDoc->SetEditText(ScAddress(1,1,0), rEE.CreateTextObject());
    rEE.SetText("Edit 3");
    m_pDoc->SetEditText(ScAddress(1,2,0), rEE.CreateTextObject());

    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // insert a note
    ScAddress aAdrA1 (0,2,0); // empty cell content.
    ScPostIt *pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, "Hello world in A3");

    // Copy and test the result.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.",
                               static_cast<SCTAB>(2), m_pDoc->GetTableCount());

    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
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
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Copy the sheet once again.
    m_pDoc->CopyTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.",
                               static_cast<SCTAB>(2), m_pDoc->GetTableCount());
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
    m_pDoc->InsertTab(0, "TestTab1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have one sheet to begin with.", static_cast<SCTAB>(1), m_pDoc->GetTableCount());
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    //test if inserting before another sheet works
    m_pDoc->InsertTab(0, "TestTab2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document should have two sheets", static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    // Move and test the result.
    m_pDoc->MoveTab(0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("copied sheet should also have all rows visible as the original.", !bHidden && nRow1 == 0 && nRow2 == MAXROW);
    OUString aName;
    m_pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "sheets should have changed places", OUString("TestTab1"), aName);

    m_pDoc->SetRowHidden(5, 10, 0, true);
    bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);

    // Move the sheet once again.
    m_pDoc->MoveTab(1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("document now should have two sheets.", static_cast<SCTAB>(2), m_pDoc->GetTableCount());
    bHidden = m_pDoc->RowHidden(0, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 0 - 4 should be visible", !bHidden && nRow1 == 0 && nRow2 == 4);
    bHidden = m_pDoc->RowHidden(5, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 5 - 10 should be hidden", bHidden && nRow1 == 5 && nRow2 == 10);
    bHidden = m_pDoc->RowHidden(11, 1, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 11 - maxrow should be visible", !bHidden && nRow1 == 11 && nRow2 == MAXROW);
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

    // Now, set borders in some cells....
    ::editeng::SvxBorderLine aLine(nullptr, 50, table::BorderLineStyle::SOLID);
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
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A1", test.equals(a1));
    test = m_pDoc->GetString(0, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected value in Sheet1.A2", test.equals(a2));

    // Put formulas into Sheet2 to Sheet4 to reference values from Sheet1.
    m_pDoc->SetString(0, 0, 1, "=Sheet1.A1");
    m_pDoc->SetString(0, 1, 1, "=Sheet1.A2");
    m_pDoc->SetString(0, 0, 2, "=Sheet1.A1");
    m_pDoc->SetString(0, 0, 3, "=Sheet1.A2");

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
        "RAWSUBTRACT",
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
        nullptr
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
        "ROT13",
        "SEARCH",
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
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != nullptr);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != nullptr);

    {
        //Add a square
        Rectangle aOrigRect(2,2,100,100);
        SdrRectObj *pObj = new SdrRectObj(aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle &rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("must have equal position and size",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetPageAnchored(*pObj);

        //Use a range of rows guaranteed to include all of the square
        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should not change when page anchored",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should not change when page anchored",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("That shouldn't change size or positioning",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        m_pDoc->ShowRows(0, 100, 0, false);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_MESSAGE("Left and Right should be unchanged",
            aOrigRect.Left() == rNewRect.Left() && aOrigRect.Right() == rNewRect.Right());
        CPPUNIT_ASSERT_MESSAGE("Height should be minimum allowed height",
            (rNewRect.Bottom() - rNewRect.Top()) <= 1);
        m_pDoc->ShowRows(0, 100, 0, true);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should not change when page anchored",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);
    }

    {
        // Add a circle.
        Rectangle aOrigRect = Rectangle(10,10,210,210); // 200 x 200
        SdrCircObj* pObj = new SdrCircObj(OBJ_CIRC, aOrigRect);
        pPage->InsertObject(pObj);
        const Rectangle& rNewRect = pObj->GetLogicRect();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Position and size of the circle shouldn't change when inserted into the page.",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size changed when cell anchored. Not good.",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

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
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size differ.",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size changed when cell-anchored. Not good.",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        // Insert 2 rows at the top and delete them immediately.
        m_pDoc->InsertRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->DeleteRow(0, 0, MAXCOL, 0, 0, 2);
        m_pDoc->SetDrawPageSize(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Size of a line object changed after row insertion and removal.",
                               static_cast<const Rectangle &>(aOrigRect), rNewRect);

        sal_Int32 n = pObj->GetPointCount();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly 2 points in a line object.", static_cast<sal_Int32>(2), n);
        CPPUNIT_ASSERT_MESSAGE("Line shape has changed.",
                               aStartPos == pObj->GetPoint(0) && aEndPos == pObj->GetPoint(1));
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
    Rectangle aObjRect(2,2,100,100);
    SdrObject* pObj = new SdrRectObj(aObjRect);
    pPage->InsertObject(pObj);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be one object on the 1st sheet.", static_cast<size_t>(1), pPage->GetObjCount());

    const ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Object meta-data doesn't exist.", pData);
    CPPUNIT_ASSERT_MESSAGE("Wrong sheet ID in cell anchor data!", pData->maStart.Tab() == 0 && pData->maEnd.Tab() == 0);

    pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 2nd sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet shouldn't have any object.", static_cast<size_t>(0), pPage->GetObjCount());

    // Insert a new sheet at left-end, and make sure the object has moved to
    // the 2nd page.
    m_pDoc->InsertTab(0, "NewTab");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 3 sheets.", static_cast<SCTAB>(3), m_pDoc->GetTableCount());
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Size and position of the object shouldn't change.",
                           aObjRect, pObj->GetLogicRect());

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
        ScRefFinder aFinder(aFormula, aPos, m_pDoc, formula::FormulaGrammar::CONV_OOO);

        // Original
        CPPUNIT_ASSERT_MESSAGE("Does not equal the original text.", aFormula.equals(aFinder.GetText()));

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
    OUString aDBName("NONAME");

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

    ScDBData* pDBData = new ScDBData(aDBName, 0, 0, 0, nCols-1, nRows-1);
    m_pDoc->SetAnonymousDBData(0,pDBData);

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

void Test::testAutoFilterTimeValue()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,0,0), "Hours");
    m_pDoc->SetValue(ScAddress(0,1,0), 72.3604166666671);
    m_pDoc->SetValue(ScAddress(0,2,0), 265);

    ScDBData* pDBData = new ScDBData(STR_DB_GLOBAL_NONAME, 0, 0, 0, 0, 2);
    m_pDoc->SetAnonymousDBData(0, pDBData);

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
    m_pDoc->SetAnonymousDBData(0, pDBData);

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
    CPPUNIT_ASSERT_MESSAGE("rows 2-7 should be filtered out.", bFiltered && nRow1 == 1 && nRow2 == 6);

    bFiltered = m_pDoc->RowFiltered(7, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("rows 8-10 should be visible.", !bFiltered && nRow1 == 7 && nRow2 == 9);

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPaste()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");
    //test copy&paste + ScUndoPaste
    //copy local and global range names in formulas
    //string cells and value cells
    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetValue(3, 0, 0, 0);
    m_pDoc->SetValue(3, 1, 0, 1);
    m_pDoc->SetValue(3, 2, 0, 2);
    m_pDoc->SetValue(3, 3, 0, 3);
    m_pDoc->SetString(2, 0, 0, "test");
    ScAddress aAdr (0, 0, 0);

    //create some range names, local and global
    ScRangeData* pLocal1 = new ScRangeData( m_pDoc, "local1", aAdr);
    ScRangeData* pLocal2 = new ScRangeData( m_pDoc, "local2", aAdr);
    ScRangeData* pLocal3 = new ScRangeData( m_pDoc, "local3", "$Sheet1.$A$1");
    ScRangeData* pLocal4 = new ScRangeData( m_pDoc, "local4", "Sheet1.$A$1");
    ScRangeData* pLocal5 = new ScRangeData( m_pDoc, "local5", "$A$1"); // implicit relative sheet reference
    ScRangeData* pGlobal = new ScRangeData( m_pDoc, "global", aAdr);
    const OUString aGlobal2Symbol("$Sheet1.$A$1:$A$23");
    ScRangeData* pGlobal2 = new ScRangeData( m_pDoc, "global2", aGlobal2Symbol);
    ScRangeName* pGlobalRangeName = new ScRangeName();
    pGlobalRangeName->insert(pGlobal);
    pGlobalRangeName->insert(pGlobal2);
    ScRangeName* pLocalRangeName1 = new ScRangeName();
    pLocalRangeName1->insert(pLocal1);
    pLocalRangeName1->insert(pLocal2);
    pLocalRangeName1->insert(pLocal3);
    pLocalRangeName1->insert(pLocal4);
    pLocalRangeName1->insert(pLocal5);
    m_pDoc->SetRangeName(pGlobalRangeName);
    m_pDoc->SetRangeName(0, pLocalRangeName1);

    // Add formula to B1.
    OUString aFormulaString("=local1+global+SUM($C$1:$D$4)+local3+local4+local5");
    m_pDoc->SetString(1, 0, 0, aFormulaString);

    double fValue = m_pDoc->GetValue(ScAddress(1,0,0));
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 11", fValue, 11);

    // add notes to A1:C1
    ScAddress aAdrA1 (0, 0, 0); // empty cell content
    OUString aHelloA1("Hello world in A1");
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, aHelloA1);
    ScAddress aAdrB1 (1, 0, 0); // formula cell content
    OUString aHelloB1("Hello world in B1");
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAdrB1);
    pNoteB1->SetText(aAdrB1, aHelloB1);
    ScAddress aAdrC1 (2, 0, 0); // string cell content
    OUString aHelloC1("Hello world in C1");
    ScPostIt* pNoteC1 = m_pDoc->GetOrCreateNote(aAdrC1);
    pNoteC1->SetText(aAdrC1, aHelloC1);

    //copy Sheet1.A1:C1 to Sheet2.A2:C2
    ScRange aRange(0,0,0,2,0,0);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    aRange = ScRange(0,1,1,2,1,1);//target: Sheet2.A2:C2
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 1, 1, true, true);
    std::unique_ptr<ScUndoPaste> pUndo(createUndoPaste(getDocShell(), aRange, pUndoDoc));
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    m_pDoc->CopyFromClip(aRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

    //check values after copying
    OUString aString;
    m_pDoc->GetFormula(1,1,1, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula string was not copied correctly", aString, aFormulaString);
    // Only the global range points to Sheet1.A1, all copied sheet-local ranges
    // to Sheet2.A1 that is empty, hence the result is 1, not 2.
    fValue = m_pDoc->GetValue(ScAddress(1,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("copied formula should return 1", 1.0, fValue);
    fValue = m_pDoc->GetValue(ScAddress(0,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("copied value should be 1", 1.0, fValue);

    ScRange aSheet2A1(0,0,1,0,0,1);

    //check local range name after copying
    pLocal1 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("local range name 1 should be copied", pLocal1);
    ScRange aRangeLocal1;
    bool bIsValidRef1 = pLocal1->IsValidReference(aRangeLocal1);
    CPPUNIT_ASSERT_MESSAGE("local range name 1 should be valid", bIsValidRef1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 1 should now point to Sheet2.A1", aSheet2A1, aRangeLocal1);

    pLocal2 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("local2 should not be copied", !pLocal2);

    pLocal3 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL3"));
    CPPUNIT_ASSERT_MESSAGE("local range name 3 should be copied", pLocal3);
    ScRange aRangeLocal3;
    bool bIsValidRef3 = pLocal3->IsValidReference(aRangeLocal3);
    CPPUNIT_ASSERT_MESSAGE("local range name 3 should be valid", bIsValidRef3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 3 should now point to Sheet2.A1", aSheet2A1, aRangeLocal3);

    pLocal4 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL4"));
    CPPUNIT_ASSERT_MESSAGE("local range name 4 should be copied", pLocal4);
    ScRange aRangeLocal4;
    bool bIsValidRef4 = pLocal4->IsValidReference(aRangeLocal4);
    CPPUNIT_ASSERT_MESSAGE("local range name 4 should be valid", bIsValidRef4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 4 should now point to Sheet2.A1", aSheet2A1, aRangeLocal4);

    pLocal5 = m_pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL5"));
    CPPUNIT_ASSERT_MESSAGE("local range name 5 should be copied", pLocal5);
    ScRange aRangeLocal5;
    bool bIsValidRef5 = pLocal5->IsValidReference(aRangeLocal5);
    CPPUNIT_ASSERT_MESSAGE("local range name 5 should be valid", bIsValidRef5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("local range 5 should now point to Sheet2.A1", aSheet2A1, aRangeLocal5);

    // check notes after copying
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.A2", m_pDoc->HasNote(ScAddress(0, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.B2", m_pDoc->HasNote(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.C2", m_pDoc->HasNote(ScAddress(2, 1, 1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note content on Sheet1.A1 not copied to Sheet2.A2, empty cell content",
            m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(), m_pDoc->GetNote(ScAddress(0, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note content on Sheet1.B1 not copied to Sheet2.B2, formula cell content",
            m_pDoc->GetNote(ScAddress(1, 0, 0))->GetText(), m_pDoc->GetNote(ScAddress(1, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Note content on Sheet1.C1 not copied to Sheet2.C2, string cell content",
            m_pDoc->GetNote(ScAddress(2, 0, 0))->GetText(), m_pDoc->GetNote(ScAddress(2, 1, 1))->GetText());

    //check undo and redo
    pUndo->Undo();
    fValue = m_pDoc->GetValue(ScAddress(1,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("after undo formula should return nothing", fValue, 0);
    aString = m_pDoc->GetString(2, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("after undo, string should be removed", aString.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("after undo, note on A2 should be removed", !m_pDoc->HasNote(ScAddress(0, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("after undo, note on B2 should be removed", !m_pDoc->HasNote(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("after undo, note on C2 should be removed", !m_pDoc->HasNote(ScAddress(2, 1, 1)));

    pUndo->Redo();
    fValue = m_pDoc->GetValue(ScAddress(1,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("formula should return 1 after redo", 1.0, fValue);
    aString = m_pDoc->GetString(2, 1, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell Sheet2.C2 should contain: test", OUString("test"), aString);
    m_pDoc->GetFormula(1,1,1, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Formula should be correct again", aFormulaString, aString);

    CPPUNIT_ASSERT_MESSAGE("After Redo, there should be a note on Sheet2.A2", m_pDoc->HasNote(ScAddress(0, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("After Redo, there should be a note on Sheet2.B2", m_pDoc->HasNote(ScAddress(1, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("After Redo, there should be a note on Sheet2.C2", m_pDoc->HasNote(ScAddress(2, 1, 1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("After Redo, note again on Sheet2.A2, empty cell content",
            m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText(), m_pDoc->GetNote(ScAddress(0, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("After Redo, note again on Sheet2.B2, formula cell content",
            m_pDoc->GetNote(ScAddress(1, 0, 0))->GetText(), m_pDoc->GetNote(ScAddress(1, 1, 1))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("After Redo, note again on Sheet2.C2, string cell content",
            m_pDoc->GetNote(ScAddress(2, 0, 0))->GetText(), m_pDoc->GetNote(ScAddress(2, 1, 1))->GetText());


    // Copy Sheet1.A11:A13 to Sheet1.A7:A9, both within global2 range.
    aRange = ScRange(0,10,0,0,12,0);
    ScDocument aClipDoc2(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc2);

    aRange = ScRange(0,6,0,0,8,0);
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

void Test::testCopyPasteAsLink()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // Turn on auto calc.

    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetValue(ScAddress(0,0,0), 1); // A1
    m_pDoc->SetValue(ScAddress(0,1,0), 2); // A2
    m_pDoc->SetValue(ScAddress(0,2,0), 3); // A3

    ScRange aRange(0,0,0,0,2,0); // Copy A1:A3 to clip.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    aRange = ScRange(1,1,1,1,3,1); // Paste to B2:B4 on Sheet2.
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    // Paste range as link.
    m_pDoc->CopyFromClip(aRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, &aClipDoc, true, true);

    // Check pasted content to make sure they reference the correct cells.
    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(1,1,1));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(1.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,2,1));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(2.0, pFC->GetValue());

    pFC = m_pDoc->GetFormulaCell(ScAddress(1,3,1));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pFC);
    CPPUNIT_ASSERT_EQUAL(3.0, pFC->GetValue());

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteTranspose()
{
    m_pDoc->InsertTab(0, "Sheet1");
    m_pDoc->InsertTab(1, "Sheet2");

    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetString(1, 0, 0, "=A1+1");
    m_pDoc->SetString(2, 0, 0, "test");

    // add notes to A1:C1
    ScAddress aAdrA1 (0, 0, 0); // numerical cell content
    OUString aHelloA1("Hello world in A1");
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAdrA1);
    pNoteA1->SetText(aAdrA1, aHelloA1);
    ScAddress aAdrB1 (1, 0, 0); // formula cell content
    OUString aHelloB1("Hello world in B1");
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAdrB1);
    pNoteB1->SetText(aAdrB1, aHelloB1);
    ScAddress aAdrC1 (2, 0, 0); // string cell content
    OUString aHelloC1("Hello world in C1");
    ScPostIt* pNoteC1 = m_pDoc->GetOrCreateNote(aAdrC1);
    pNoteC1->SetText(aAdrC1, aHelloC1);

    // transpose clipboard, paste and check on Sheet2
    m_pDoc->InsertTab(1, "Sheet2");

    ScRange aSrcRange = ScRange(0,0,0,2,0,0);
    ScDocument aNewClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aSrcRange, &aNewClipDoc);

    ::std::unique_ptr<ScDocument> pTransClip(new ScDocument(SCDOCMODE_CLIP));
    aNewClipDoc.TransposeClip(pTransClip.get(), InsertDeleteFlags::ALL, false);

    ScRange aDestRange = ScRange(3,1,1,3,3,1);//target: Sheet2.D2:D4
    ScMarkData aMark;
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pTransClip.get());
    pTransClip.reset();

    //check cell content after transposed copy/paste
    OUString aString = m_pDoc->GetString(3, 3, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell Sheet2.D4 should contain: test", OUString("test"), aString);
    double fValue = m_pDoc->GetValue(ScAddress(3,1,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied cell should return 1", fValue, 1);
    fValue = m_pDoc->GetValue(ScAddress(3,2,1));
    ASSERT_DOUBLES_EQUAL_MESSAGE("transposed copied formula should return 2", fValue, 2);
    m_pDoc->GetFormula(3, 2, 1, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("transposed formula should point on Sheet2.D2", OUString("=D2+1"), aString);

    // check notes after transposed copy/paste
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.D2", m_pDoc->HasNote(ScAddress(3, 1, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.D3", m_pDoc->HasNote(ScAddress(3, 2, 1)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on Sheet2.D4", m_pDoc->HasNote(ScAddress(3, 3, 1)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on Sheet2.D2",
            m_pDoc->GetNote(ScAddress(3, 1, 1))->GetText(),  m_pDoc->GetNote(ScAddress(0, 0, 0))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on Sheet2.D3",
            m_pDoc->GetNote(ScAddress(3, 2, 1))->GetText(),  m_pDoc->GetNote(ScAddress(1, 0, 0))->GetText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Content of cell note on Sheet2.D4",
            m_pDoc->GetNote(ScAddress(3, 3, 1))->GetText(),  m_pDoc->GetNote(ScAddress(2, 0, 0))->GetText());

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);

}

void Test::testCopyPasteMultiRange()
{
    m_pDoc->InsertTab(0, "Test");

    // Fill A2:B6 with numbers.
    for (SCROW nRow = 1; nRow <= 5; ++nRow)
    {
        for (SCCOL nCol = 0; nCol <= 1; ++nCol)
        {
            ScAddress aPos(nCol,nRow,0);
            m_pDoc->SetValue(aPos, nRow+nCol);
        }
    }

    // Fill D9:E11 with numbers.
    for (SCROW nRow = 8; nRow <= 10; ++nRow)
    {
        for (SCCOL nCol = 3; nCol <= 4; ++nCol)
        {
            ScAddress aPos(nCol,nRow,0);
            m_pDoc->SetValue(aPos, 10.0);
        }
    }

    ScMarkData aMark;
    aMark.SelectOneTable(0);

    // Copy A2:B2, A4:B4, and A6:B6 to clipboard.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScClipParam aClipParam;
    aClipParam.maRanges.Append(ScRange(0,1,0,1,1,0)); // A2:B2
    aClipParam.maRanges.Append(ScRange(0,3,0,1,3,0)); // A4:B4
    aClipParam.maRanges.Append(ScRange(0,5,0,1,5,0)); // A6:B6
    aClipParam.meDirection = ScClipParam::Row;
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste to D9:E11, and make sure it won't crash (rhbz#1080196).
    m_pDoc->CopyMultiRangeFromClip(ScAddress(3,8,0), aMark, InsertDeleteFlags::CONTENTS, &aClipDoc);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(3,8,0)));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4,8,0)));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(3,9,0)));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(4,9,0)));
    CPPUNIT_ASSERT_EQUAL(5.0, m_pDoc->GetValue(ScAddress(3,10,0)));
    CPPUNIT_ASSERT_EQUAL(6.0, m_pDoc->GetValue(ScAddress(4,10,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteSkipEmpty()
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

        explicit TestRange( ScDocument* pDoc ) : mpDoc(pDoc) {}

        bool checkRange( const ScAddress& rPos, const Check* p, const Check* pEnd )
        {
            ScAddress aPos(rPos);
            OUString aPosStr = aPos.Format(ScRefFlags::VALID);
            for (; p != pEnd; ++p, aPos.IncRow())
            {
                if (!mpDoc->GetString(aPos).equalsAscii(p->mpStr))
                {
                    cerr << aPosStr << ": incorrect string value: expected='" << p->mpStr << "' actual='" << mpDoc->GetString(aPos) << endl;
                    return false;
                }

                const SvxBrushItem* pBrush =
                    dynamic_cast<const SvxBrushItem*>(mpDoc->GetAttr(aPos, ATTR_BACKGROUND));

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
    m_pDoc->InitDrawLayer(&getDocShell()); // for cell note objects.

    ScRange aSrcRange(0,0,0,0,4,0);
    ScRange aDestRange(1,0,0,1,4,0);

    ScMarkData aMark;
    aMark.SetMarkArea(aDestRange);

    // Put some texts in B1:B5.
    m_pDoc->SetString(ScAddress(1,0,0), "A");
    m_pDoc->SetString(ScAddress(1,1,0), "B");
    m_pDoc->SetString(ScAddress(1,2,0), "C");
    m_pDoc->SetString(ScAddress(1,3,0), "D");
    m_pDoc->SetString(ScAddress(1,4,0), "E");

    // Set the background color of B1:B5 to blue.
    ScPatternAttr aCellBackColor(m_pDoc->GetPool());
    aCellBackColor.GetItemSet().Put(SvxBrushItem(COL_BLUE, ATTR_BACKGROUND));
    m_pDoc->ApplyPatternAreaTab(1, 0, 1, 4, 0, aCellBackColor);

    // Insert notes to B1:B5.
    m_pDoc->GetOrCreateNote(ScAddress(1,0,0));
    m_pDoc->GetOrCreateNote(ScAddress(1,1,0));
    m_pDoc->GetOrCreateNote(ScAddress(1,2,0));
    m_pDoc->GetOrCreateNote(ScAddress(1,3,0));
    m_pDoc->GetOrCreateNote(ScAddress(1,4,0));

    // Prepare a clipboard content interleaved with empty cells.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    ScClipParam aParam(aSrcRange, false);
    aClipDoc.SetClipParam(aParam);
    aClipDoc.SetString(ScAddress(0,0,0), "Clip1");
    aClipDoc.SetString(ScAddress(0,2,0), "Clip2");
    aClipDoc.SetString(ScAddress(0,4,0), "Clip3");

    // Set the background color of A1:A5 to yellow.
    aCellBackColor.GetItemSet().Put(SvxBrushItem(COL_YELLOW, ATTR_BACKGROUND));
    aClipDoc.ApplyPatternAreaTab(0, 0, 0, 4, 0, aCellBackColor);

    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aClipDoc.GetCellType(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE,   aClipDoc.GetCellType(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aClipDoc.GetCellType(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE,   aClipDoc.GetCellType(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aClipDoc.GetCellType(ScAddress(0,4,0)));

    // Check the initial condition.
    {
        Check aChecks[] = {
            { "A", COL_BLUE, true },
            { "B", COL_BLUE, true },
            { "C", COL_BLUE, true },
            { "D", COL_BLUE, true },
            { "E", COL_BLUE, true },
        };

        bool bRes = aTest.checkRange(ScAddress(1,0,0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Initial check failed.", bRes);
    }

    // Create undo document.
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aDestRange, InsertDeleteFlags::ALL, false, *pUndoDoc, &aMark);

    // Paste clipboard content onto A1:A5 but skip empty cells.
    bool bSkipEmpty = true;
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, pUndoDoc, &aClipDoc, true, false, false, bSkipEmpty);

    // Create redo document.
    ScDocument* pRedoDoc = new ScDocument(SCDOCMODE_UNDO);
    pRedoDoc->InitUndo(m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aDestRange, InsertDeleteFlags::ALL, false, *pRedoDoc, &aMark);

    // Create an undo object for this.
    ScRefUndoData* pRefUndoData = new ScRefUndoData(m_pDoc);
    ScUndoPaste aUndo(&getDocShell(), aDestRange, aMark, pUndoDoc, pRedoDoc, InsertDeleteFlags::ALL, pRefUndoData);

    // Check the content after the paste.
    {
        Check aChecks[] = {
            { "Clip1", COL_YELLOW, false },
            { "B",     COL_BLUE,   true },
            { "Clip2", COL_YELLOW, false },
            { "D",     COL_BLUE,   true },
            { "Clip3", COL_YELLOW, false },
        };

        bool bRes = aTest.checkRange(ScAddress(1,0,0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Check after paste failed.", bRes);
    }

    // Undo, and check the content.
    aUndo.Undo();
    {
        Check aChecks[] = {
            { "A", COL_BLUE, true },
            { "B", COL_BLUE, true },
            { "C", COL_BLUE, true },
            { "D", COL_BLUE, true },
            { "E", COL_BLUE, true },
        };

        bool bRes = aTest.checkRange(ScAddress(1,0,0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Check after undo failed.", bRes);
    }

    // Redo, and check the content again.
    aUndo.Redo();
    {
        Check aChecks[] = {
            { "Clip1", COL_YELLOW, false },
            { "B",     COL_BLUE,   true },
            { "Clip2", COL_YELLOW, false },
            { "D",     COL_BLUE,   true },
            { "Clip3", COL_YELLOW, false },
        };

        bool bRes = aTest.checkRange(ScAddress(1,0,0), aChecks, aChecks + SAL_N_ELEMENTS(aChecks));
        CPPUNIT_ASSERT_MESSAGE("Check after redo failed.", bRes);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteSkipEmpty2()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(0,0,0), "A");
    m_pDoc->SetString(ScAddress(2,0,0), "C");

    // Copy A1:C1 to clipboard.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, static_cast<SCTAB>(0));
    copyToClip(m_pDoc, ScRange(0,0,0,2,0,0), &aClipDoc);

    // Paste to A3 with the skip empty option set.  This used to freeze. (fdo#77735)
    ScRange aDestRange(0,2,0,2,2,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc, false, false, true, true);

    CPPUNIT_ASSERT_EQUAL(OUString("A"), m_pDoc->GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("B3 should be empty.", CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("C"), m_pDoc->GetString(ScAddress(2,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testCutPasteRefUndo()
{
    // Testing scenario: A2 references B2, and B2 gets cut and pasted onto C2,
    // which updates A2's formula to reference C2. Then the paste action gets
    // undone, which should also undo A2's formula to reference back to B2.

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Test");

    // A2 references B2.
    m_pDoc->SetString(ScAddress(0,1,0), "=B2");

    ScMarkData aMark;
    aMark.SelectOneTable(0);

    // Set up clip document for cutting of B2.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    ScClipParam aParam(ScAddress(1,1,0), true);
    aClipDoc.SetClipParam(aParam);
    aClipDoc.SetValue(ScAddress(1,1,0), 12.0);

    // Set up undo document for reference update.
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 0, 0);

    // Do the pasting of 12 into C2.  This should update A2 to reference C2.
    m_pDoc->CopyFromClip(ScAddress(2,1,0), aMark, InsertDeleteFlags::CONTENTS, pUndoDoc, &aClipDoc);
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(0,1,0));

    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "C2", "A2 should be referencing C2.");

    // At this point, the ref undo document should contain a formula cell at A2 that references B2.
    ASSERT_FORMULA_EQUAL(*pUndoDoc, ScAddress(0,1,0), "B2", "A2 in the undo document should be referencing B2.");

    ScUndoPaste aUndo(&getDocShell(), ScRange(ScAddress(2,1,0)), aMark, pUndoDoc, nullptr, InsertDeleteFlags::CONTENTS, nullptr, false, nullptr);
    aUndo.Undo();

    // Now A2 should be referencing B2 once again.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "B2", "A2 should be referencing B2 after undo.");

    m_pDoc->DeleteTab(0);
}

void Test::testCutPasteGroupRefUndo()
{
    // Test that Cut&Paste part of a grouped formula adjusts references
    // correctly and Undo works.

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Test");

    // Formula data in A1:A9
    const char* aData[][1] = {
        { "1" },
        { "=A1+A1" },
        { "=A2+A1" },
        { "=A3+A2" },
        { "=A4+A3" },
        { "=A5+A4" },
        { "=A6+A5" },
        { "=A7+A6" },
        { "=A8+A7" }
    };
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to insert data", aPos, aDataRange.aStart);

    // Check initial data.
    const char* aDataCheck[][2] = {
        { "1", "" },
        { "2", "=A1+A1" },
        { "3", "=A2+A1" },
        { "5", "=A3+A2" },
        { "8", "=A4+A3" },
        { "13", "=A5+A4" },
        { "21", "=A6+A5" },
        { "34", "=A7+A6" },
        { "55", "=A8+A7" }
    };
    for (size_t i=0; i<SAL_N_ELEMENTS(aDataCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0,i,0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initial data failure", OUString::createFromAscii(aDataCheck[i][0]), aString);
        m_pDoc->GetFormula(0,i,0, aString);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initial formula failure", OUString::createFromAscii(aDataCheck[i][1]), aString);
    }

    ScMarkData aMark;
    aMark.SelectOneTable(0);

    // Set up clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    aClipDoc.ResetClip(m_pDoc, &aMark);
    // Cut A4:A6 to clipboard with Undo.
    std::unique_ptr<ScUndoCut> pUndoCut( cutToClip( getDocShell(), ScRange(0,3,0, 0,5,0), &aClipDoc, true));

    // Check data after Cut.
    const char* aCutCheck[] = {"1","2","3","","","","0","0","0"};
    for (size_t i=0; i<SAL_N_ELEMENTS(aCutCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0,i,0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cut data failure", OUString::createFromAscii(aCutCheck[i]), aString);
    }

    // Paste to B5:B7 with Undo.
    ScRange aPasteRange(1,4,0, 1,6,0);
    aMark.SetMarkArea(aPasteRange);
    ScDocument* pPasteUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pPasteUndoDoc->InitUndoSelected( m_pDoc, aMark);
    std::unique_ptr<ScUndoPaste> pUndoPaste( createUndoPaste( getDocShell(), aPasteRange, pPasteUndoDoc));
    m_pDoc->CopyFromClip( aPasteRange, aMark, InsertDeleteFlags::ALL, pPasteUndoDoc, &aClipDoc);

    // Check data after Paste.
    const char* aPasteCheck[][4] = {
        { "1",    "", "",       "" },
        { "2",    "", "=A1+A1", "" },
        { "3",    "", "=A2+A1", "" },
        { "",     "", "",       "" },
        { "",    "5", "",       "=A3+A2" },
        { "",    "8", "",       "=B5+A3" },
        { "21", "13", "=B7+B6", "=B6+B5" },
        { "34",   "", "=A7+B7", "" },
        { "55",   "", "=A8+A7", "" }
    };
    for (size_t i=0; i<SAL_N_ELEMENTS(aPasteCheck); ++i)
    {
        for (size_t j=0; j<2; ++j)
        {
            OUString aString = m_pDoc->GetString(j,i,0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Paste data failure", OUString::createFromAscii(aPasteCheck[i][j]), aString);
            m_pDoc->GetFormula(j,i,0, aString);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Paste formula failure", OUString::createFromAscii(aPasteCheck[i][2+j]), aString);
        }
    }

    // Undo Paste and check, must be same as after Cut.
    pUndoPaste->Undo();
    for (size_t i=0; i<SAL_N_ELEMENTS(aCutCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0,i,0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo Paste data failure", OUString::createFromAscii(aCutCheck[i]), aString);
    }

    // Undo Cut and check, must be initial data.
    pUndoCut->Undo();
    for (size_t i=0; i<SAL_N_ELEMENTS(aDataCheck); ++i)
    {
        OUString aString = m_pDoc->GetString(0,i,0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo Cut data failure", OUString::createFromAscii(aDataCheck[i][0]), aString);
        m_pDoc->GetFormula(0,i,0, aString);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo Cut formula failure", OUString::createFromAscii(aDataCheck[i][1]), aString);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testMoveRefBetweenSheets()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    m_pDoc->InsertTab(0, "Test1");
    m_pDoc->InsertTab(1, "Test2");

    m_pDoc->SetValue(ScAddress(0,0,0), 12.0);
    m_pDoc->SetValue(ScAddress(1,0,0), 10.0);
    m_pDoc->SetValue(ScAddress(2,0,0),  8.0);
    m_pDoc->SetString(ScAddress(0,1,0), "=A1");
    m_pDoc->SetString(ScAddress(0,2,0), "=SUM(A1:C1)");

    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // These formulas should not display the sheet name.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,0), "A1", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,0), "SUM(A1:C1)", "Wrong formula!");

    // Move Test1.A2:A3 to Test2.A2:A3.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    bool bMoved = rFunc.MoveBlock(ScRange(0,1,0,0,2,0), ScAddress(0,1,1), true, true, false, true);
    CPPUNIT_ASSERT(bMoved);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("This cell should be empty after the move.", CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(0,1,0)));
    ASSERT_DOUBLES_EQUAL(12.0, m_pDoc->GetValue(ScAddress(0,1,1)));
    ASSERT_DOUBLES_EQUAL(30.0, m_pDoc->GetValue(ScAddress(0,2,1)));

    // The reference in the pasted formula should display sheet name after the move.
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,1,1), "Test1.A1", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,2,1), "SUM(Test1.A1:C1)", "Wrong formula!");

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testUndoCut()
{
    m_pDoc->InsertTab(0, "Test");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calc.

    // Insert values into A1:A3.
    m_pDoc->SetValue(ScAddress(0,0,0), 1.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 10.0);
    m_pDoc->SetValue(ScAddress(0,2,0), 100.0);

    // SUM in A4.
    m_pDoc->SetString(ScAddress(0,3,0), "=SUM(A1:A3)");
    CPPUNIT_ASSERT_EQUAL(111.0, m_pDoc->GetValue(0,3,0));

    // Select A1:A3.
    ScMarkData aMark;
    ScRange aRange(0,0,0,0,2,0);
    aMark.SetMarkArea(aRange);
    aMark.MarkToMulti();

    // Set up an undo object for cutting A1:A3.
    ScDocument* pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
    pUndoDoc->InitUndo(m_pDoc, 0 ,0);
    m_pDoc->CopyToDocument(aRange, InsertDeleteFlags::ALL, false, *pUndoDoc);
    ASSERT_DOUBLES_EQUAL(  1.0, pUndoDoc->GetValue(ScAddress(0,0,0)));
    ASSERT_DOUBLES_EQUAL( 10.0, pUndoDoc->GetValue(ScAddress(0,1,0)));
    ASSERT_DOUBLES_EQUAL(100.0, pUndoDoc->GetValue(ScAddress(0,2,0)));
    ScUndoCut aUndo(&getDocShell(), aRange, aRange.aEnd, aMark, pUndoDoc);

    // "Cut" the selection.
    m_pDoc->DeleteSelection(InsertDeleteFlags::ALL, aMark);
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(0,3,0)); // The SUM should be zero after the "cut".

    // Undo it, and check the result.
    aUndo.Undo();
    ASSERT_DOUBLES_EQUAL(  1.0, m_pDoc->GetValue(ScAddress(0,0,0)));
    ASSERT_DOUBLES_EQUAL( 10.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    ASSERT_DOUBLES_EQUAL(100.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    ASSERT_DOUBLES_EQUAL(111.0, m_pDoc->GetValue(0,3,0)); // The SUM value should be back to the original.

    // Redo it and check.
    aUndo.Redo();
    ASSERT_DOUBLES_EQUAL(0.0, m_pDoc->GetValue(0,3,0));

    // Undo again.
    aUndo.Undo();
    ASSERT_DOUBLES_EQUAL(111.0, m_pDoc->GetValue(0,3,0));

    m_pDoc->DeleteTab(0);
}

void Test::testMoveBlock()
{
    m_pDoc->InsertTab(0, "SheetNotes");

    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetString(1, 0, 0, "=A1+1");
    m_pDoc->SetString(2, 0, 0, "test");

    // add notes to A1:C1
    ScAddress aAddrA1 (0, 0, 0);
    OUString aHelloA1("Hello world in A1");
    ScPostIt* pNoteA1 = m_pDoc->GetOrCreateNote(aAddrA1);
    pNoteA1->SetText(aAddrA1, aHelloA1);
    ScAddress aAddrB1 (1, 0, 0);
    OUString aHelloB1("Hello world in B1");
    ScPostIt* pNoteB1 = m_pDoc->GetOrCreateNote(aAddrB1);
    pNoteB1->SetText(aAddrB1, aHelloB1);
    ScAddress aAddrC1 (2, 0, 0);
    OUString aHelloC1("Hello world in C1");
    ScPostIt* pNoteC1 = m_pDoc->GetOrCreateNote(aAddrC1);
    pNoteC1->SetText(aAddrC1, aHelloC1);
    ScAddress aAddrD1 (3, 0, 0);

    // previous tests on cell note content are ok. this one fails !!! :(
    //CPPUNIT_ASSERT_MESSAGE("Note content in B1 before move block", m_pDoc->GetNote(aAddrB1)->GetText() == aHelloB1);

    // move notes to B1:D1
    bool bCut = true;
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
    bool bMoveDone = rDocFunc.MoveBlock(ScRange(0, 0 ,0 ,2 ,0 ,0), ScAddress(1, 0, 0), bCut, false, false, false);

    CPPUNIT_ASSERT_MESSAGE("Cells not moved", bMoveDone);

    //check cell content
    OUString aString = m_pDoc->GetString(3, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell D1 should contain: test", OUString("test"), aString);
    m_pDoc->GetFormula(2, 0, 0, aString);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell C1 should contain an updated formula", OUString("=B1+1"), aString);
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

void Test::testCopyPasteRelativeFormula()
{
    m_pDoc->InsertTab(0, "Formula");

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);

    // Insert values to A2 and A4.
    m_pDoc->SetValue(ScAddress(0,1,0), 1);
    m_pDoc->SetValue(ScAddress(0,3,0), 2);

    // Insert formula to B4.
    m_pDoc->SetString(ScAddress(1,3,0), "=A4");
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(1,3,0)));

    // Select and copy B3:B4 to the clipboard.
    ScRange aRange(1,2,0,1,3,0);
    ScClipParam aClipParam(aRange, false);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste it to B1:B2.
    InsertDeleteFlags nFlags = InsertDeleteFlags::ALL;
    ScRange aDestRange(1,0,0,1,1,0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

    // B2 references A2, so the value should be 1.
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(1,1,0)));

    // Clear content and start over.
    clearSheet(m_pDoc, 0);
    clearSheet(&aClipDoc, 0);

    // Insert a single formula cell in A1.
    m_pDoc->SetString(ScAddress(0,0,0), "=ROW()");
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared()); // single formula cell is never shared.

    // Copy A1 to clipboard.
    aClipParam = ScClipParam(ScAddress(0,0,0), false);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    pFC = aClipDoc.GetFormulaCell(ScAddress(0,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared());

    // Paste to A3.
    aDestRange = ScRange(0,2,0,0,2,0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

    pFC = m_pDoc->GetFormulaCell(ScAddress(0,2,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT(!pFC->IsShared());

    // Delete A3 and make sure it doesn't crash (see fdo#76132).
    clearRange(m_pDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(0,2,0)));

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteRepeatOneFormula()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);

    m_pDoc->InsertTab(0, "Test");

    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aMark;

    // Insert values in A1:B10.
    for (SCROW i = 0; i < 10; ++i)
    {
        m_pDoc->SetValue(ScAddress(0,i,0), i+1.0);        // column A
        m_pDoc->SetValue(ScAddress(1,i,0), (i+1.0)*10.0); // column B
    }

    // Insert a formula in C1.
    ScAddress aPos(2,0,0); // C1
    m_pDoc->SetString(aPos, "=SUM(A1:B1)");
    CPPUNIT_ASSERT_EQUAL(11.0, m_pDoc->GetValue(aPos));

    // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // At this point, there should be only one normal area listener listening
    // on A1:B1.
    ScRange aWholeSheet(0,0,0,MAXCOL,MAXROW,0);
    ScBroadcastAreaSlotMachine* pBASM = m_pDoc->GetBASM();
    CPPUNIT_ASSERT(pBASM);
    std::vector<sc::AreaListener> aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaInside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    const sc::AreaListener* pListener = &aListeners[0];
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,1,0,0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener shouldn't be a group listener.", !pListener->mbGroupListening);
#endif

    // Copy C1 to clipboard.
    ScClipParam aClipParam(aPos, false);
    aMark.SetMarkArea(aPos);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Paste it to C2:C10.
    InsertDeleteFlags nFlags = InsertDeleteFlags::CONTENTS;
    ScRange aDestRange(2,1,0,2,9,0);
    aMark.SetMarkArea(aDestRange);
    m_pDoc->CopyFromClip(aDestRange, aMark, nFlags, nullptr, &aClipDoc);

    // Make sure C1:C10 are grouped.
    const ScFormulaCell* pFC = m_pDoc->GetFormulaCell(aPos);
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

    // Check the formula results.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fExpected = (i+1.0)*11.0;
        CPPUNIT_ASSERT_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(2,i,0)));
    }

    // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // At this point, there should only be one area listener and it should be
    // a group listener listening on A1:B10.
    aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaInside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    pListener = &aListeners[0];
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,1,9,0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener should be a group listener.", pListener->mbGroupListening);
#endif

    // Insert a new row at row 1.
    ScRange aRowOne(0,0,0,MAXCOL,0,0);
    aMark.SetMarkArea(aRowOne);
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.InsertCells(aRowOne, &aMark, INS_INSROWS_BEFORE, true, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 should be empty.", CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(2,0,0)));

    // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // Make there we only have one group area listener listening on A2:B11.
    aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaInside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    pListener = &aListeners[0];
    CPPUNIT_ASSERT_EQUAL(ScRange(0,1,0,1,10,0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener should be a group listener.", pListener->mbGroupListening);
#endif

    // Check the formula results.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fExpected = (i+1.0)*11.0;
        CPPUNIT_ASSERT_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(2,i+1,0)));
    }

    // Delete row at row 1 to shift the cells up.
    rFunc.DeleteCells(aRowOne, &aMark, DEL_DELROWS, true);

    // Check the formula results again.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fExpected = (i+1.0)*11.0;
        CPPUNIT_ASSERT_EQUAL(fExpected, m_pDoc->GetValue(ScAddress(2,i,0)));
    }

    // This check makes only sense if group listeners are activated.
#if !defined(USE_FORMULA_GROUP_LISTENER) || USE_FORMULA_GROUP_LISTENER
    // Check the group area listener again to make sure it's listening on A1:B10 once again.
    aListeners = pBASM->GetAllListeners(aWholeSheet, sc::AreaInside);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aListeners.size());
    pListener = &aListeners[0];
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,1,9,0), pListener->maArea);
    CPPUNIT_ASSERT_MESSAGE("This listener should be a group listener.", pListener->mbGroupListening);
#endif

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteMixedReferenceFormula()
{
    sc::AutoCalcSwitch aAC(*m_pDoc, true); // turn on auto calc.
    m_pDoc->InsertTab(0, "Test");

    // Insert value to C3
    m_pDoc->SetValue(2,2,0, 1.0);

    // Insert formula to A1 with mixed relative/absolute addressing.
    m_pDoc->SetString(0,0,0, "=SUM(B:$C)");
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(0,0,0), "SUM(B:$C)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0,0,0));

    // Copy formula in A1 to clipboard.
    ScRange aRange(ScAddress(0,0,0));
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, aRange, &aClipDoc);

    // Paste formula to B1.
    aRange = ScAddress(1,0,0);
    pasteFromClip(m_pDoc, aRange, &aClipDoc);
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(1,0,0), "SUM(C:$C)", "Wrong formula.");
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0,0,0));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(1,0,0));

    // Paste formula to C1. All three results now must be circular reference.
    aRange = ScAddress(2,0,0);
    pasteFromClip(m_pDoc, aRange, &aClipDoc);
    ASSERT_FORMULA_EQUAL(*m_pDoc, ScAddress(2,0,0), "SUM($C:D)", "Wrong formula."); // reference put in order
    CPPUNIT_ASSERT_EQUAL(OUString("Err:522"), m_pDoc->GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("Err:522"), m_pDoc->GetString(1,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("Err:522"), m_pDoc->GetString(2,0,0));

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
    CPPUNIT_ASSERT_MESSAGE("did not merge cells", nEndCol == 3 && nEndRow == 3);
    ScRange aRange(0,2,0,MAXCOL,2,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    getDocShell().GetDocFunc().InsertCells(aRange, &aMark, INS_INSROWS_BEFORE, true, true);
    m_pDoc->ExtendMerge(1, 1, nEndCol, nEndRow, 0);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("table not renamed", nameToSet, nameJustSet);

    //test case 3 , rename again
    OUString anOldName;
    m_pDoc->GetName(0,anOldName);

    nameToSet = "test2";
    rDocFunc.RenameTable(0,nameToSet,false,true);
    m_pDoc->GetName(0,nameJustSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("table not renamed", nameToSet, nameJustSet);

    //test case 4 , check if  undo works
    SfxUndoAction* pUndo = new ScUndoRenameTab(&getDocShell(),0,anOldName,nameToSet);
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
    aColor=Color(COL_YELLOW);
    getDocShell().GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set",
                           aColor, m_pDoc->GetTabBgColor(0));

    Color aOldTabBgColor=m_pDoc->GetTabBgColor(0);
    aColor.SetColor(COL_BLUE);//set BLUE
    getDocShell().GetDocFunc().SetTabBgColor(0,aColor,false, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the correct color is not set the second time",
                           aColor, m_pDoc->GetTabBgColor(0));

    //now check for undo
    SfxUndoAction* pUndo = new ScUndoTabColor(&getDocShell(), 0, aOldTabBgColor, aColor);
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
    m_pDoc->InsertTab(0, "Sheet2");
    m_pDoc->GetValue(2,0,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2,1,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting first sheet formula does not return correct result", aValue, 5);

    m_pDoc->InsertTab(0, "Sheet1");
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
    aSheets.push_back("Sheet1");
    aSheets.push_back("Sheet2");
    m_pDoc->InsertTabs(0, aSheets, true);
    m_pDoc->GetValue(2, 0, 2, aValue);
    OUString aFormula;
    m_pDoc->GetFormula(2,0,2, aFormula);

    ASSERT_DOUBLES_EQUAL_MESSAGE("after inserting sheets formula does not return correct result", aValue, 3);
    m_pDoc->GetValue(2, 1, 2, aValue);
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
    m_pDoc->GetValue(0,0,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected MATCH result", 2, aValue);
    m_pDoc->InsertRow(0,0,MAXCOL,0,0,1);    // insert 1 row before row 1 in Sheet1
    m_pDoc->GetValue(0,0,1, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("unexpected MATCH result", 3, aValue);
    m_pDoc->DeleteRow(0,0,MAXCOL,0,0,1);    // delete row 1 in Sheet1
    m_pDoc->GetValue(0,0,1, aValue);
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
    ScMarkData aMarkData;
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

bool hasRange(const std::vector<ScTokenRef>& rRefTokens, const ScRange& rRange, const ScAddress& rPos)
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
                ScSingleRefData aData = *p->GetSingleRef();
                if (rRange.aStart != rRange.aEnd)
                    break;

                ScAddress aThis = aData.toAbs(rPos);
                if (aThis == rRange.aStart)
                    return true;
            }
            break;
            case formula::svDoubleRef:
            {
                ScComplexRefData aData = *p->GetDoubleRef();
                ScRange aThis = aData.toAbs(rPos);
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
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();

    {
        // C1's precedent should be A1:A2,B3.
        ScAddress aC1(2, 0, 0);
        ScRangeList aRange(aC1);
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("A1:A2 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(0, 0, 0, 0, 1, 0), aC1));
        CPPUNIT_ASSERT_MESSAGE("B3 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(1, 2, 0), aC1));
    }

    {
        // C2's precedent should be A1 only.
        ScAddress aC2(2, 1, 0);
        ScRangeList aRange(aC2);
        rDocFunc.DetectiveCollectAllPreds(aRange, aRefTokens);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("there should only be one reference token.",
                               aRefTokens.size(), static_cast<size_t>(1));
        CPPUNIT_ASSERT_MESSAGE("A1 should be a precedent of C1.",
                               hasRange(aRefTokens, ScRange(0, 0, 0), aC2));
    }

    {
        // A1's dependent should be C1:C2.
        ScAddress aA1(0, 0, 0);
        ScRangeList aRange(aA1);
        rDocFunc.DetectiveCollectAllSuccs(aRange, aRefTokens);
        CPPUNIT_ASSERT_MESSAGE("C1:C2 should be the only dependent of A1.",
                               aRefTokens.size() == 1 && hasRange(aRefTokens, ScRange(2, 0, 0, 2, 1, 0), aA1));
    }

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFill()
{
    m_pDoc->InsertTab(0, "test");

    m_pDoc->SetValue(0,0,0,1);

    ScMarkData aMarkData;
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
    m_pDoc->Fill( 0, 100, 0, 100, nullptr, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    OUString aTestValue = m_pDoc->GetString( 0, 101, 0 );
    CPPUNIT_ASSERT_EQUAL( aTestValue, OUString("February") );
    aTestValue = m_pDoc->GetString( 0, 102, 0 );
    CPPUNIT_ASSERT_EQUAL( aTestValue, OUString("March") );

    // test that two same user data list entries will not result in incremental fill
    m_pDoc->SetString( 0, 101, 0, "January" );
    m_pDoc->Fill( 0, 100, 0, 101, nullptr, aMarkData, 2, FILL_TO_BOTTOM, FILL_AUTO );
    for ( SCROW i = 102; i <= 103; ++i )
    {
        aTestValue = m_pDoc->GetString( 0, i, 0 );
        CPPUNIT_ASSERT_EQUAL( aTestValue, OUString("January") );
    }

    // Clear column A for a new test.
    clearRange(m_pDoc, ScRange(0,0,0,0,MAXROW,0));
    m_pDoc->SetRowHidden(0, MAXROW, 0, false); // Show all rows.

    // Fill A1:A6 with 1,2,3,4,5,6.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
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
        OUString aFormula;
        m_pDoc->GetFormula(100, nRow, 0, aFormula);
        CPPUNIT_ASSERT_EQUAL(aExpected, aFormula);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testAutoFillSimple()
{
    m_pDoc->InsertTab(0, "test");

    m_pDoc->SetValue(0, 0, 0, 1);
    m_pDoc->SetString(0, 1, 0, "=10");

    ScMarkData aMarkData;
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
            OString aMsg = OString("wrong value in row: ") + OString::number(nRow);
            double nVal = m_pDoc->GetValue(0, nRow, 0);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), 10.0, nVal);
        }
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

    // check that moving was successful, mainly for editable tester
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
    SfxMedium* pMedium = new SfxMedium(aDocName, StreamMode::STD_READWRITE);
    getDocShell().DoInitNew(pMedium);
    m_pDoc = &getDocShell().GetDocument();

    ScDocShellRef xExtDocSh = new ScDocShell;
    xExtDocSh->SetIsInUcalc();
    OUString aExtDocName("file:///extdata.fake");
    OUString aExtSh1Name("ExtSheet1");
    OUString aExtSh2Name("ExtSheet2");
    SfxMedium* pMed = new SfxMedium(aExtDocName, StreamMode::STD_READWRITE);
    xExtDocSh->DoInitNew(pMed);
    CPPUNIT_ASSERT_MESSAGE("external document instance not loaded.",
                           findLoadedDocShellByName(aExtDocName) != nullptr);

    ScDocument& rExtDoc = xExtDocSh->GetDocument();
    rExtDoc.InsertTab(0, aExtSh1Name);
    rExtDoc.InsertTab(1, aExtSh2Name);

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
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    m_pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    InsertDeleteFlags nFlags = InsertDeleteFlags::ALL;
    aRange = ScRange(1,1,1,1,6,1);
    ScMarkData aMarkData2;
    aMarkData2.SetMarkArea(aRange);
    rExtDoc.CopyFromClip(aRange, aMarkData2, nFlags, nullptr, &aClipDoc);

    OUString aFormula;
    rExtDoc.GetFormula(1,1,1, aFormula);
    //adjust absolute refs pointing to the copy area
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=COLUMN($B$2)"));
    rExtDoc.GetFormula(1,2,1, aFormula);
    //adjust absolute refs and keep relative refs
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$B$2+C3"));
    rExtDoc.GetFormula(1,3,1, aFormula);
    // make absolute sheet refs external refs
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("='file:///source.fake'#$Sheet2.B2"));
    rExtDoc.GetFormula(1,4,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("='file:///source.fake'#$Sheet2.$A$1"));
    rExtDoc.GetFormula(1,5,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("='file:///source.fake'#$Sheet2.B$1"));
    rExtDoc.GetFormula(1,6,1, aFormula);
    CPPUNIT_ASSERT_EQUAL(aFormula, OUString("=$ExtSheet2.$B$2"));

    xExtDocSh->DoClose();
}

void Test::testFindAreaPosVertical()
{
    const char* aData[][3] = {
        {   nullptr, "1", "1" },
        { "1",   nullptr, "1" },
        { "1", "1", "1" },
        {   nullptr, "1", "1" },
        { "1", "1", "1" },
        { "1",   nullptr, "1" },
        { "1", "1", "1" },
    };

    m_pDoc->InsertTab(0, "Test1");
    clearRange( m_pDoc, ScRange(0, 0, 0, 1, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
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

    m_pDoc->InsertTab(0, "test1");
    clearRange( m_pDoc, ScRange(0, 0, 0, 7, SAL_N_ELEMENTS(aData), 0));
    ScAddress aPos(0,0,0);
    ScRange aDataRange = insertRangeData( m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
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

void Test::testShiftCells()
{
    m_pDoc->InsertTab(0, "foo");

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
    m_pDoc->InitDrawLayer(&getDocShell());

    OUString aHello("Hello");
    OUString aJimBob("Jim Bob");
    ScAddress aPos(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aPos);
    pNote->SetText(aPos, aHello);
    pNote->SetAuthor(aJimBob);

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
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    rDocFunc.DeleteCells(ScRange(0,1,0,MAXCOL,1,0), &aMark, DEL_CELLSUP, true);

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
    rDocFunc.DeleteCells(ScRange(0,2,0,MAXCOL,2,0), &aMark, DEL_CELLSUP, true);

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
    ScDocument& rDoc = getDocShell().GetDocument();
    rDoc.InsertTab(0, "Sheet1");

    ScAddress rAddr(1, 1, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(rAddr);
    pNote->SetText(rAddr, "Hello");
    pNote->SetAuthor("Jim Bob");

    CPPUNIT_ASSERT_MESSAGE("there should be a note", rDoc.HasNote(1, 1, 0));

    rDoc.DeleteCol(0, 0, MAXROW, 0, 1, 1);

    CPPUNIT_ASSERT_MESSAGE("there should be no more note", !rDoc.HasNote(1, 1, 0));

    rDoc.DeleteTab(0);
}

void Test::testNoteLifeCycle()
{
    m_pDoc->InsertTab(0, "Test");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(&getDocShell());

    ScAddress aPos(1,1,0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new cell comment.", pNote);

    pNote->SetText(aPos, "New note");
    ScPostIt* pNote2 = m_pDoc->ReleaseNote(aPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This note instance is expected to be identical to the original.", pNote, pNote2);
    CPPUNIT_ASSERT_MESSAGE("The note shouldn't be here after it's been released.", !m_pDoc->HasNote(aPos));

    // Modify the internal state of the note instance to make sure it's really
    // been released.
    pNote->SetText(aPos, "New content");

    // Re-insert the note back to the same place.
    m_pDoc->SetNote(aPos, pNote);
    SdrCaptionObj* pCaption = pNote->GetOrCreateCaption(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to create a caption object.", pCaption);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This caption should belong to the drawing layer of the document.",
                           m_pDoc->GetDrawLayer(), static_cast<ScDrawLayer*>(pCaption->GetModel()));

    // Copy B2 with note to a clipboard.

    ScClipParam aClipParam(aPos, false);
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScMarkData aMarkData;
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
    bool bCut = true;       // like Drag&Drop
    bool bRecord = true;    // record Undo
    bool bPaint = false;    // don't care about
    bool bApi = true;       // API to prevent dialogs
    ScDocFunc& rDocFunc = getDocShell().GetDocFunc();
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

    m_pDoc->DeleteTab(0);
}

void Test::testNoteCopyPaste()
{
    m_pDoc->InsertTab(0, "Test");

    // We need a drawing layer in order to create caption objects.
    m_pDoc->InitDrawLayer(&getDocShell());

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
    ScMarkData aMark;
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

void Test::testAreasWithNotes()
{
    ScDocument& rDoc = getDocShell().GetDocument();
    rDoc.InsertTab(0, "Sheet1");

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

    dataFound = rDoc.GetDataStart(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No DataStart found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong col for notes", static_cast<SCCOL>(1), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong row for notes", static_cast<SCROW>(2), row);

    dataFound = rDoc.GetCellArea(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No CellArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong col for notes", static_cast<SCCOL>(2), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong row for notes", static_cast<SCROW>(5), row);

    bool bNotes = true;
    dataFound = rDoc.GetPrintArea(0,col,row, bNotes);

    CPPUNIT_ASSERT_MESSAGE("No PrintArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong col for notes", static_cast<SCCOL>(2), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong row for notes", static_cast<SCROW>(5), row);

    bNotes = false;
    dataFound = rDoc.GetPrintArea(0,col,row, bNotes);
    CPPUNIT_ASSERT_MESSAGE("No PrintArea should be found", !dataFound);

    bNotes = true;
    dataFound = rDoc.GetPrintAreaVer(0,0,1,row, bNotes); // cols 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row for notes", static_cast<SCROW>(5), row);

    dataFound = rDoc.GetPrintAreaVer(0,2,3,row, bNotes); // cols 2 & 3
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row for notes", static_cast<SCROW>(2), row);

    bNotes = false;
    dataFound = rDoc.GetPrintAreaVer(0,0,1,row, bNotes); // col 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer should be found", !dataFound);

    // now add cells with value, check that notes are taken into account in good cases

    m_pDoc->SetString(0, 3, 0, "Some Text");
    m_pDoc->SetString(3, 3, 0, "Some Text");

    dataFound = rDoc.GetDataStart(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No DataStart found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong col", static_cast<SCCOL>(0), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DataStart wrong row", static_cast<SCROW>(2), row);

    dataFound = rDoc.GetCellArea(0,col,row);

    CPPUNIT_ASSERT_MESSAGE("No CellArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong col", static_cast<SCCOL>(3), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CellArea wrong row", static_cast<SCROW>(5), row);

    bNotes = true;
    dataFound = rDoc.GetPrintArea(0,col,row, bNotes);

    CPPUNIT_ASSERT_MESSAGE("No PrintArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong col", static_cast<SCCOL>(3), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong row", static_cast<SCROW>(5), row);

    bNotes = false;
    dataFound = rDoc.GetPrintArea(0,col,row, bNotes);
    CPPUNIT_ASSERT_MESSAGE("No PrintArea found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong col", static_cast<SCCOL>(3), col);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintArea wrong row", static_cast<SCROW>(3), row);

    bNotes = true;
    dataFound = rDoc.GetPrintAreaVer(0,0,1,row, bNotes); // cols 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row", static_cast<SCROW>(5), row);

    dataFound = rDoc.GetPrintAreaVer(0,2,3,row, bNotes); // cols 2 & 3
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row", static_cast<SCROW>(3), row);

    bNotes = false;
    dataFound = rDoc.GetPrintAreaVer(0,0,1,row, bNotes); // cols 0 & 1
    CPPUNIT_ASSERT_MESSAGE("No PrintAreaVer found", dataFound);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PrintAreaVer wrong row", static_cast<SCROW>(3), row);

    rDoc.DeleteTab(0);
}

void Test::testAnchoredRotatedShape()
{
    m_pDoc->InsertTab(0, "TestTab");
    SCROW nRow1, nRow2;
    bool bHidden = m_pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("new sheet should have all rows visible", !bHidden && nRow1 == 0 && nRow2 == MAXROW);

    m_pDoc->InitDrawLayer();
    ScDrawLayer *pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != nullptr);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != nullptr);
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
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object meta-data.", pData);

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
    std::unique_ptr<ScColumnTextWidthIterator> pIter(new ScColumnTextWidthIterator(*m_pDoc, aTopCell, MAXROW));
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

bool checkEditTextIterator(sc::EditTextIterator& rIter, const char** pChecks)
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
        rEditEngine.SetText("A2");
        m_pDoc->SetEditText(ScAddress(0,1,0), rEditEngine.CreateTextObject());
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { "A2", nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    {
        // Add a series of edit cells.
        rEditEngine.SetText("A5");
        m_pDoc->SetEditText(ScAddress(0,4,0), rEditEngine.CreateTextObject());
        rEditEngine.SetText("A6");
        m_pDoc->SetEditText(ScAddress(0,5,0), rEditEngine.CreateTextObject());
        rEditEngine.SetText("A7");
        m_pDoc->SetEditText(ScAddress(0,6,0), rEditEngine.CreateTextObject());
        sc::EditTextIterator aIter(*m_pDoc,0);
        const char* pChecks[] = { "A2", "A5", "A6", "A7", nullptr };
        CPPUNIT_ASSERT_MESSAGE("Wrong iterator behavior.", checkEditTextIterator(aIter, pChecks));
    }

    {
        // Add more edit cells to column C. Skip column B.
        rEditEngine.SetText("C1");
        m_pDoc->SetEditText(ScAddress(2,0,0), rEditEngine.CreateTextObject());
        rEditEngine.SetText("C3");
        m_pDoc->SetEditText(ScAddress(2,2,0), rEditEngine.CreateTextObject());
        rEditEngine.SetText("C4");
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
    ScImportExport aObj(m_pDoc, ScAddress(0,0,0));
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
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    aMark.SetMarkArea(aRange);

    std::unique_ptr<ScDocument> pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(m_pDoc, 0, 0);
    m_pDoc->CopyToDocument(aRange, InsertDeleteFlags::CONTENTS, false, *pUndoDoc, &aMark);
    ScUndoDeleteContents aUndo(&getDocShell(), aMark, aRange, std::move(pUndoDoc), false, InsertDeleteFlags::CONTENTS, true);

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
    ScMarkData aMark;
    aMark.SetMarkArea(ScRange(0,0,0,0,2,0));
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    rFunc.TransliterateText(
        aMark, i18n::TransliterationModules_LOWERCASE_UPPERCASE, true);

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

    const char* aData[][3] = {
        { "=1", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=2", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=3", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=4", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=5", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
        { "=6", "=RC[-1]*2", "=ISFORMULA(RC[-1])" },
    };

    ScAddress aPos(1,2,0); // B3
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "1",  "2", "TRUE" },
            { "2",  "4", "TRUE" },
            { "3",  "6", "TRUE" },
            { "4",  "8", "TRUE" },
            { "5", "10", "TRUE" },
            { "6", "12", "TRUE" },
        };

        bool bSuccess = checkOutput<3>(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Convert B5:C6 to static values, and check the result.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScRange aConvRange(1,4,0,2,5,0); // B5:C6
    rFunc.ConvertFormulaToValue(aConvRange, false);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "1",  "2",  "TRUE" },
            { "2",  "4",  "TRUE" },
            { "3",  "6", "FALSE" },
            { "4",  "8", "FALSE" },
            { "5", "10",  "TRUE" },
            { "6", "12",  "TRUE" },
        };

        bool bSuccess = checkOutput<3>(m_pDoc, aDataRange, aOutputCheck, "Converted");
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
        const char* aOutputCheck[][3] = {
            { "1",  "2", "TRUE" },
            { "2",  "4", "TRUE" },
            { "3",  "6", "TRUE" },
            { "4",  "8", "TRUE" },
            { "5", "10", "TRUE" },
            { "6", "12", "TRUE" },
        };

        bool bSuccess = checkOutput<3>(m_pDoc, aDataRange, aOutputCheck, "After undo");
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
        const char* aOutputCheck[][3] = {
            { "1",  "2",  "TRUE" },
            { "2",  "4",  "TRUE" },
            { "3",  "6", "FALSE" },
            { "4",  "8", "FALSE" },
            { "5", "10",  "TRUE" },
            { "6", "12",  "TRUE" },
        };

        bool bSuccess = checkOutput<3>(m_pDoc, aDataRange, aOutputCheck, "Converted");
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

    const char* aData[][2] = {
        { "=1", "=ISFORMULA(RC[-1])" },
        { "=2", "=ISFORMULA(RC[-1])" },
        {  "3", "=ISFORMULA(RC[-1])" },
        { "=4", "=ISFORMULA(RC[-1])" },
        { "=5", "=ISFORMULA(RC[-1])" },
    };

    // Insert data into B2:C6.
    ScAddress aPos(1,1,0); // B2
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed to insert range data at correct position", aPos, aDataRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "1", "TRUE" },
            { "2", "TRUE" },
            { "3", "FALSE" },
            { "4", "TRUE" },
            { "5", "TRUE" },
        };

        bool bSuccess = checkOutput<2>(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Convert B3:B5 to a value.
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScRange aConvRange(1,2,0,1,4,0); // B3:B5
    rFunc.ConvertFormulaToValue(aConvRange, false);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "1", "TRUE" },
            { "2", "FALSE" },
            { "3", "FALSE" },
            { "4", "FALSE" },
            { "5", "TRUE" },
        };

        bool bSuccess = checkOutput<2>(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Undo and check.
    SfxUndoManager* pUndoMgr = m_pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "1", "TRUE" },
            { "2", "TRUE" },
            { "3", "FALSE" },
            { "4", "TRUE" },
            { "5", "TRUE" },
        };

        bool bSuccess = checkOutput<2>(m_pDoc, aDataRange, aOutputCheck, "Initial value");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    m_pDoc->DeleteTab(0);
}

void Test::testMixData()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetValue(ScAddress(1,0,0), 2.0); // B1
    m_pDoc->SetValue(ScAddress(0,1,0), 3.0); // A2

    // Copy A1:B1 to the clip document.
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,0,0,1,0,0), &aClipDoc); // A1:B1

    // Copy A2:B2 to the mix document (for arithmetic paste).
    ScDocument aMixDoc(SCDOCMODE_CLIP);
    copyToClip(m_pDoc, ScRange(0,1,0,1,1,0), &aMixDoc); // A2:B2

    // Paste A1:B1 to A2:B2 and perform addition.
    pasteFromClip(m_pDoc, ScRange(0,1,0,1,1,0), &aClipDoc);
    m_pDoc->MixDocument(ScRange(0,1,0,1,1,0), ScPasteFunc::ADD, false, &aMixDoc);

    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0,1,0)); // A2
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1,1,0)); // B2

    // Clear everything and start over.
    clearSheet(m_pDoc, 0);
    clearSheet(&aClipDoc, 0);
    clearSheet(&aMixDoc, 0);

    // Set values to A1, A2, and B1.  B2 will remain empty.
    m_pDoc->SetValue(ScAddress(0,0,0), 15.0);
    m_pDoc->SetValue(ScAddress(0,1,0), 16.0);
    m_pDoc->SetValue(ScAddress(1,0,0), 12.0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("B2 should be empty.", CELLTYPE_NONE, m_pDoc->GetCellType(ScAddress(1,1,0)));

    // Copy A1:A2 and paste it onto B1:B2 with subtraction operation.
    copyToClip(m_pDoc, ScRange(0,0,0,0,1,0), &aClipDoc);
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(ScAddress(0,0,0)), aClipDoc.GetValue(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(ScAddress(0,1,0)), aClipDoc.GetValue(ScAddress(0,1,0)));

    copyToClip(m_pDoc, ScRange(1,0,0,1,1,0), &aMixDoc);
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(ScAddress(1,0,0)), aMixDoc.GetValue(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetValue(ScAddress(1,1,0)), aMixDoc.GetValue(ScAddress(1,1,0)));

    pasteFromClip(m_pDoc, ScRange(1,0,0,1,1,0), &aClipDoc);
    m_pDoc->MixDocument(ScRange(1,0,0,1,1,0), ScPasteFunc::SUB, false, &aMixDoc);

    CPPUNIT_ASSERT_EQUAL( -3.0, m_pDoc->GetValue(ScAddress(1,0,0))); // 12 - 15
    CPPUNIT_ASSERT_EQUAL(-16.0, m_pDoc->GetValue(ScAddress(1,1,0))); //  0 - 16

    m_pDoc->DeleteTab(0);
}

void Test::testFormulaWizardSubformula()
{
    m_pDoc->InsertTab(0, "Test");

    m_pDoc->SetString(ScAddress(1,0,0), "=1");          // B1
    m_pDoc->SetString(ScAddress(1,1,0), "=1/0");        // B2
    m_pDoc->SetString(ScAddress(1,2,0), "=gibberish");  // B3

    ScSimpleFormulaCalculator aFCell1( m_pDoc, ScAddress(0,0,0), "=B1:B3", true );
    FormulaError nErrCode = aFCell1.GetErrCode();
    CPPUNIT_ASSERT( nErrCode == FormulaError::NONE || aFCell1.IsMatrix() );
    CPPUNIT_ASSERT_EQUAL( OUString("{1;#DIV/0!;#NAME?}"), aFCell1.GetString().getString() );

    m_pDoc->SetString(ScAddress(1,0,0), "=NA()");       // B1
    m_pDoc->SetString(ScAddress(1,1,0), "2");           // B2
    m_pDoc->SetString(ScAddress(1,2,0), "=1+2");        // B3
    ScSimpleFormulaCalculator aFCell2( m_pDoc, ScAddress(0,0,0), "=B1:B3", true );
    nErrCode = aFCell2.GetErrCode();
    CPPUNIT_ASSERT( nErrCode == FormulaError::NONE || aFCell2.IsMatrix() );
    CPPUNIT_ASSERT_EQUAL( OUString("{#N/A;2;3}"), aFCell2.GetString().getString() );

    m_pDoc->DeleteTab(0);
}

void Test::testSetStringAndNote()
{
    m_pDoc->InsertTab(0, "Test");

    //note on A1
    ScAddress aAdrA1 (0, 0, 0);
    ScPostIt* pNote = m_pDoc->GetOrCreateNote(aAdrA1);
    pNote->SetText(aAdrA1, "Hello world in A1");

    m_pDoc->SetString(0, 0, 0, "");

    pNote = m_pDoc->GetNote(aAdrA1);
    CPPUNIT_ASSERT(pNote);

    m_pDoc->DeleteTab(0);
}

void Test::testCopyPasteMatrixFormula()
{
    m_pDoc->InsertTab(0, "hcv");

    // Set Values to B1, C1, D1
    m_pDoc->SetValue(ScAddress(1,0,0), 2.0);    // B1
    m_pDoc->SetValue(ScAddress(2,0,0), 5.0);    // C1
    m_pDoc->SetValue(ScAddress(3,0,0), 3.0);    // D1

    // Set Values to B2, C2
    m_pDoc->SetString(ScAddress(1,1,0), "B2");  // B2
    //m_pDoc->SetString(ScAddress(2,1,0), "C2");  // C2
    m_pDoc->SetString(ScAddress(3,1,0), "D2");  // D2

    // Set Values to D3
    //m_pDoc->SetValue(ScAddress(1,2,0), 9.0);    // B3
    //m_pDoc->SetString(ScAddress(2,2,0), "C3");  // C3
    m_pDoc->SetValue(ScAddress(3,2,0), 11.0);   // D3

    // Insert matrix formula to A1
    ScMarkData aMark;
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 0, aMark, "=COUNTIF(ISBLANK(B1:D1);TRUE())");
    m_pDoc->CalcAll();
    // A1 should containg 0
    CPPUNIT_ASSERT_EQUAL( 0.0, m_pDoc->GetValue(ScAddress(0,0,0)) ); // A1

    // Copy cell A1 to clipboard.
    ScAddress aPos(0,0,0);  // A1
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScClipParam aParam(aPos, false);
    m_pDoc->CopyToClip(aParam, &aClipDoc, &aMark, false, false);
    // Formula string should be equal.
    CPPUNIT_ASSERT_EQUAL(m_pDoc->GetString(aPos), aClipDoc.GetString(aPos));

    // First try single range.
    // Paste matrix formula to A2
    pasteFromClip(m_pDoc, ScRange(0,1,0,0,1,0), &aClipDoc); // A2
    // A2 Cell value should contain 1.0
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));

    // Paste matrix formula to A3
    pasteFromClip(m_pDoc, ScRange(0,2,0,0,2,0), &aClipDoc); // A3
    // A3 Cell value should contain 2.0
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,2,0)));

    // Paste matrix formula to A4
    pasteFromClip(m_pDoc, ScRange(0,3,0,0,3,0), &aClipDoc); // A4
    // A4 Cell value should contain 3.0
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(0,3,0)));

    // Clear cell A2:A4
    clearRange(m_pDoc, ScRange(0,1,0,0,3,0));

    // Paste matrix formula to range A2:A4
    pasteFromClip(m_pDoc, ScRange(0,1,0,0,3,0), &aClipDoc); // A2:A4

    // A2 Cell value should contain 1.0
    CPPUNIT_ASSERT_EQUAL( 1.0, m_pDoc->GetValue(ScAddress(0,1,0)));
    // A3 Cell value should contain 2.0
    CPPUNIT_ASSERT_EQUAL( 2.0, m_pDoc->GetValue(ScAddress(0,2,0)));
    // A4 Cell value should contain 3.0
    CPPUNIT_ASSERT_EQUAL( 3.0, m_pDoc->GetValue(ScAddress(0,3,0)));

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
    Rectangle aObjRect(2,1000,100,1100);
    SdrObject* pObj = new SdrRectObj(aObjRect);
    pPage->InsertObject(pObj);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *m_pDoc, 0);

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
    ScDocFunc& rFunc = getDocShell().GetDocFunc();
    ScMarkData aMark;
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
    CPPUNIT_ASSERT_MESSAGE("Failed to compare Address.", aNewStart  != aOldStart  && aNewEnd  != aOldEnd &&
                                                         aNNewStart != aNOldStart && aNNewEnd != aNOldEnd );

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
    CPPUNIT_ASSERT_EQUAL( (sal_uLong) 0, m_pDoc->GetCellCount() );
    CPPUNIT_ASSERT_EQUAL( (sal_uLong) 0, m_pDoc->GetFormulaGroupCount() );
    CPPUNIT_ASSERT_EQUAL( (sal_uLong) 0, m_pDoc->GetCodeCount() );
    CPPUNIT_ASSERT_EQUAL( (sal_uInt8) 0, m_pDoc->GetAsianCompression() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasPrintRange() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsInVBAMode() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->HasNotes() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsCutMode() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsUsingEmbededFonts() );
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

    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsAdjustHeightEnabled() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsExecuteLinkEnabled() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsChangeReadOnlyEnabled() );

    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IdleCalcTextWidth() );
    CPPUNIT_ASSERT_EQUAL( true, m_pDoc->IsIdleEnabled() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->IsDetectiveDirty() );
    CPPUNIT_ASSERT_EQUAL( false, m_pDoc->GetHasMacroFunc() );
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

ScDocShell* Test::findLoadedDocShellByName(const OUString& rName)
{
    ScDocShell* pShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>, false));
    while (pShell)
    {
        SfxMedium* pMedium = pShell->GetMedium();
        if (pMedium)
        {
            OUString aName = pMedium->GetName();
            if (aName.equals(rName))
                return pShell;
        }
        pShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pShell, checkSfxObjectShell<ScDocShell>, false));
    }
    return nullptr;
}

bool Test::insertRangeNames(
    ScDocument* pDoc, ScRangeName* pNames, const RangeNameDef* p, const RangeNameDef* pEnd)
{
    ScAddress aA1(0, 0, 0);
    for (; p != pEnd; ++p)
    {
        ScRangeData* pNew = new ScRangeData(
            pDoc,
            OUString::createFromAscii(p->mpName),
            OUString::createFromAscii(p->mpExpr),
            aA1, ScRangeData::Type::Name,
            formula::FormulaGrammar::GRAM_ENGLISH);
        pNew->SetIndex(p->mnIndex);
        bool bSuccess = pNames->insert(pNew);
        if (!bSuccess)
        {
            cerr << "Insertion failed." << endl;
            return false;
        }
    }

    return true;
}

void Test::printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption)
{
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    svl::GridPrinter printer(nRow2 - nRow1 + 1, nCol2 - nCol1 + 1, CALC_DEBUG_OUTPUT != 0);
    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            ScAddress aPos(nCol, nRow, rRange.aStart.Tab());
            ScRefCellValue aCell(*pDoc, aPos);
            OUString aVal = ScCellFormat::GetOutputString(*pDoc, aPos, aCell);
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
        rRange.aEnd.Col(), rRange.aEnd.Row(), aMarkData, InsertDeleteFlags::CONTENTS);
}

void Test::clearSheet(ScDocument* pDoc, SCTAB nTab)
{
    ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
    clearRange(pDoc, aRange);
}

ScUndoCut* Test::cutToClip(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pClipDoc, bool bCreateUndo)
{
    ScDocument* pSrcDoc = &rDocSh.GetDocument();

    ScClipParam aClipParam(rRange, true);
    ScMarkData aMark;
    aMark.SetMarkArea(rRange);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aMark, false, false);

    // Taken from ScViewFunc::CutToClip()
    ScDocument* pUndoDoc = nullptr;
    if (bCreateUndo)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndoSelected( pSrcDoc, aMark );
        // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
        ScRange aCopyRange = rRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(pSrcDoc->GetTableCount()-1);
        pSrcDoc->CopyToDocument( aCopyRange,
                (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS,
                false, *pUndoDoc );
    }

    aMark.MarkToMulti();
    pSrcDoc->DeleteSelection( InsertDeleteFlags::ALL, aMark );
    aMark.MarkToSimple();

    if (pUndoDoc)
        return new ScUndoCut( &rDocSh, rRange, rRange.aEnd, aMark, pUndoDoc );

    return nullptr;
}

void Test::copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc)
{
    ScClipParam aClipParam(rRange, false);
    ScMarkData aMark;
    aMark.SetMarkArea(rRange);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aMark, false, false);
}

void Test::pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc)
{
    ScMarkData aMark;
    aMark.SetMarkArea(rDestRange);
    pDestDoc->CopyFromClip(rDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pClipDoc);
}

void Test::pasteOneCellFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc, InsertDeleteFlags eFlags)
{
    ScMarkData aMark;
    aMark.SetMarkArea(rDestRange);
    sc::CopyFromClipContext aCxt(*pDestDoc, nullptr, pClipDoc, eFlags, false, false);
    aCxt.setDestRange(rDestRange.aStart.Col(), rDestRange.aStart.Row(),
            rDestRange.aEnd.Col(), rDestRange.aEnd.Row());
    aCxt.setTabRange(rDestRange.aStart.Tab(), rDestRange.aEnd.Tab());
    pDestDoc->CopyOneCellFromClip(aCxt, rDestRange.aStart.Col(), rDestRange.aStart.Row(),
            rDestRange.aEnd.Col(), rDestRange.aEnd.Row());
}

ScUndoPaste* Test::createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pUndoDoc)
{
    ScDocument& rDoc = rDocSh.GetDocument();
    ScMarkData aMarkData;
    aMarkData.SetMarkArea(rRange);
    ScRefUndoData* pRefUndoData = new ScRefUndoData(&rDoc);

    return new ScUndoPaste(
        &rDocSh, rRange, aMarkData, pUndoDoc, nullptr, InsertDeleteFlags::ALL, pRefUndoData, false);
}

void Test::setExpandRefs(bool bExpand)
{
    ScModule* pMod = SC_MOD();
    ScInputOptions aOpt = pMod->GetInputOptions();
    aOpt.SetExpandRefs(bExpand);
    pMod->SetInputOptions(aOpt);
}

void Test::setCalcAsShown(ScDocument* pDoc, bool bCalcAsShown)
{
    ScDocOptions aOpt = pDoc->GetDocOptions();
    aOpt.SetCalcAsShown(bCalcAsShown);
    pDoc->SetDocOptions(aOpt);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
