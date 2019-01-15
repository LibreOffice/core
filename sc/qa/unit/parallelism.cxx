/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <scdll.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <formulagroup.hxx>
#include <scopetools.hxx>

#include <officecfg/Office/Calc.hxx>

using namespace css;
using namespace css::uno;

class ScParallelismTest : public ScBootstrapFixture
{
public:
    ScParallelismTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void getNewDocShell(ScDocShellRef& rDocShellRef);

    void testSUMIFS();
    void testDivision();
    void testVLOOKUP();
    void testVLOOKUPSUM();
    void testSingleRef();
    void testSUMIFImplicitRange();
    void testFGCycleWithPlainFormulaCell1();
    void testFGCycleWithPlainFormulaCell2();
    void testMultipleFGColumn();
    void testFormulaGroupSpanEval();
    void testFormulaGroupSpanEvalNonGroup();

    CPPUNIT_TEST_SUITE(ScParallelismTest);
    CPPUNIT_TEST(testSUMIFS);
    CPPUNIT_TEST(testDivision);
    CPPUNIT_TEST(testVLOOKUP);
    CPPUNIT_TEST(testVLOOKUPSUM);
    CPPUNIT_TEST(testSingleRef);
    CPPUNIT_TEST(testSUMIFImplicitRange);
    CPPUNIT_TEST(testFGCycleWithPlainFormulaCell1);
    CPPUNIT_TEST(testFGCycleWithPlainFormulaCell2);
    CPPUNIT_TEST(testMultipleFGColumn);
    CPPUNIT_TEST(testFormulaGroupSpanEval);
    CPPUNIT_TEST(testFormulaGroupSpanEvalNonGroup);
    CPPUNIT_TEST_SUITE_END();

private:

    bool getThreadingFlag();
    void setThreadingFlag(bool bSet);

    ScDocument *m_pDoc;

    ScDocShellRef m_xDocShell;
    bool m_bThreadingFlagCfg;
};

ScParallelismTest::ScParallelismTest()
      : ScBootstrapFixture( "sc/qa/unit/data" )
{
}

bool ScParallelismTest::getThreadingFlag()
{
    return officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get();
}

void ScParallelismTest::setThreadingFlag( bool bSet )
{
    std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::set(bSet, xBatch);
    xBatch->commit();
}

void ScParallelismTest::setUp()
{
    test::BootstrapFixture::setUp();

    ScDLL::Init();

    getNewDocShell(m_xDocShell);
    m_pDoc = &m_xDocShell->GetDocument();

    sc::FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly();

    m_bThreadingFlagCfg = getThreadingFlag();
    if (!m_bThreadingFlagCfg)
        setThreadingFlag(true);
}

void ScParallelismTest::tearDown()
{
    // Restore threading flag
    if (!m_bThreadingFlagCfg)
        setThreadingFlag(false);

    test::BootstrapFixture::tearDown();
}

void ScParallelismTest::getNewDocShell( ScDocShellRef& rDocShellRef )
{
    rDocShellRef = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
}

void ScParallelismTest::testSUMIFS()
{
    m_pDoc->InsertTab(0, "1");

    m_pDoc->SetValue(0, 0, 0, 1001);

    for (auto i = 1; i < 1000; i++)
    {
        /*A*/
        if (i%19)
            m_pDoc->SetValue(0, i, 0, i/10 + 1000);
        else
            m_pDoc->SetValue(0, i, 0, 123456);
        /*B*/ m_pDoc->SetValue(1, i, 0, i%10);
        /*C*/ m_pDoc->SetValue(2, i, 0, i%5);

        /*F*/ m_pDoc->SetValue(5, i, 0, i%17 + i%13);

        /*L*/ m_pDoc->SetValue(11, i, 0, i%10);
        /*M*/ m_pDoc->SetValue(12, i, 0, i%5);
    }

    for (auto i = 1; i < 1000; i++)
    {
        // For instance P389 will contain the formula:
        // =SUMIFS($F$2:$F$1000; $A$2:$A$1000; A$1; $B$2:$B$1000; $L389; $C$2:$C$1000; $M389)

        // In other words, it will sum those values in F2:1000 where the A value matches A1 (1001),
        // the B value matches L389 and the C value matches M389. (There should be just one such
        // value, so the formula is actually simply used to pick out that single value from the F
        // column where A,B,C match. Silly, but that is how SUMIFS is used in some corners of the
        // real world, apparently.)

        /*P*/ m_pDoc->SetFormula(ScAddress(15, i, 0),
                                "=SUMIFS($F$2:$F$1000; "
                                "$A$2:$A$1000; A$1; "
                                "$B$2:$B$1000; $L" + OUString::number(i+1) + "; "
                                "$C$2:$C$1000; $M" + OUString::number(i+1) +
                                 ")",
                                 formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    m_xDocShell->DoHardRecalc();

#if 1
    OUString sFormula;

    std::cerr << "A1=" << m_pDoc->GetValue(0, 0, 0) << std::endl;

    std::cerr << "      A,B,C  F  L,M" << std::endl;
    for (auto i = 1; i < 30; i++)
    {
        std::cerr <<
            i+1 << ": " <<
            m_pDoc->GetValue(0, i, 0) << "," <<
            m_pDoc->GetValue(1, i, 0) << "," <<
            m_pDoc->GetValue(2, i, 0) << "  " <<
            m_pDoc->GetValue(5, i, 0) << "  " <<
            m_pDoc->GetValue(11, i, 0) << "," <<
            m_pDoc->GetValue(12, i, 0) << "  \"";
        m_pDoc->GetFormula(15, i, 0, sFormula);
        std::cerr << sFormula << "\": \"" <<
            m_pDoc->GetString(15, i, 0) << "\": " <<
            m_pDoc->GetValue(15, i, 0) << std::endl;
    }
#endif

    for (auto i = 1; i < 1000; i++)
    {
        OString sMessage = "At row " + OString::number(i+1);
        if ((10+i%10)%19)
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), m_pDoc->GetValue(5, 10+i%10, 0), m_pDoc->GetValue(15, i, 0), 1e-10);
        else
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), 0, m_pDoc->GetValue(15, i, 0), 1e-10);
    }

    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testDivision()
{
    m_pDoc->InsertTab(0, "1");

    for (auto i = 1; i < 1000; i++)
    {
        /*A*/ m_pDoc->SetValue(0, i, 0, i);
        /*B*/ m_pDoc->SetValue(1, i, 0, i%10);
        /*C*/ m_pDoc->SetFormula(ScAddress(2, i, 0),
                                 "=A" + OUString::number(i+1) + "/B" + OUString::number(i+1),
                                 formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    m_xDocShell->DoHardRecalc();

    for (auto i = 1; i < 1000; i++)
    {
        OString sMessage = "At row " + OString::number(i+1);
        if (i%10)
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), static_cast<double>(i)/(i%10), m_pDoc->GetValue(2, i, 0), 1e-10);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("#DIV/0!"), m_pDoc->GetString(2, i, 0));
    }

    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testVLOOKUP()
{
    m_pDoc->InsertTab(0, "1");

    for (auto i = 1; i < 2000; i++)
    {
        if (i == 1042)
            m_pDoc->SetValue(0, i, 0, 1042.42);
        else if (i%5)
            m_pDoc->SetValue(0, i, 0, i);
        else
            m_pDoc->SetValue(0, i, 0, i+0.1);

        if (i%2)
            m_pDoc->SetValue(1, i, 0, i*10);
        else
            m_pDoc->SetString(1, i, 0, "N" + OUString::number(i*10));

        if (i < 1000)
        {
            m_pDoc->SetFormula(ScAddress(2, i, 0),
                               "=VLOOKUP(" + OUString::number(i) + "; "
                               "A$2:B$2000; 2; 0)",
                               formula::FormulaGrammar::GRAM_NATIVE_UI);
        }

        else
        {
            if (i == 1042)
                m_pDoc->SetFormula(ScAddress(2, i, 0),
                                   "=VLOOKUP(1042.42; "
                                   "A$2:B$2000; 2; 0)",
                                   formula::FormulaGrammar::GRAM_NATIVE_UI);
            else
                m_pDoc->SetFormula(ScAddress(2, i, 0),
                                   "=VLOOKUP(1.234; "
                                   "A$2:B$2000; 2; 0)",
                                   formula::FormulaGrammar::GRAM_NATIVE_UI);
        }
    }

    m_xDocShell->DoHardRecalc();

    for (auto i = 1; i < 2000; i++)
    {
        OString sMessage = "At row " + OString::number(i+1);
        if (i < 1000)
        {
            if (i%5)
            {
                if (i%2)
                    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), static_cast<double>(i*10), m_pDoc->GetValue(2, i, 0), 1e-10);
                else
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("N" + OUString::number(i*10)), m_pDoc->GetString(2, i, 0));
            }
            else
            {
                // The corresponding value in A is i+0.1
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("#N/A"), m_pDoc->GetString(2, i, 0));
            }
        }
        else
        {
            if (i == 1042)
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("N" + OUString::number(i*10)), m_pDoc->GetString(2, i, 0));
            else
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("#N/A"), m_pDoc->GetString(2, i, 0));
        }
    }

    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testVLOOKUPSUM()
{
    m_pDoc->InsertTab(0, "1");

    const size_t nNumRows = 4096*4;
    OUString aTableRef = "$A$1:$B$" + OUString::number(nNumRows);
    for (size_t i = 0; i < nNumRows; ++i)
    {
        m_pDoc->SetValue(0, i, 0, static_cast<double>(i));
        m_pDoc->SetValue(1, i, 0, static_cast<double>(5*i + 100));
        m_pDoc->SetValue(2, i, 0, static_cast<double>(nNumRows - i - 1));
    }
    for (size_t i = 0; i < nNumRows; ++i)
    {
        OUString aArgNum = "C" + OUString::number(i+1);
        m_pDoc->SetFormula(ScAddress(3, i, 0),
                           "=SUM(" + aArgNum + ";VLOOKUP(" + aArgNum + ";" + aTableRef + "; 2; 0)) + SUM($A1:$A2)",
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    m_xDocShell->DoHardRecalc();

    for (size_t i = 0; i < nNumRows; ++i)
    {
        OString aMsg = "At row " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), 6 * (nNumRows - i - 1) + 101, static_cast<size_t>(m_pDoc->GetValue(3, i, 0)));
    }
    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testSingleRef()
{
    m_pDoc->InsertTab(0, "1");

    const size_t nNumRows = 200;
    for (size_t i = 0; i < nNumRows; ++i)
    {
        m_pDoc->SetValue(0, i, 0, static_cast<double>(i));
        m_pDoc->SetFormula(ScAddress(1, i, 0), "=A" + OUString::number(i+1), formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    m_xDocShell->DoHardRecalc();

    for (size_t i = 0; i < nNumRows; ++i)
    {
        OString aMsg = "At row " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), i, static_cast<size_t>(m_pDoc->GetValue(1, i, 0)));
    }
    m_pDoc->DeleteTab(0);
}

// Common test setup steps for testSUMIFImplicitRange*()
static void lcl_setupCommon(ScDocument* pDoc, size_t nNumRows, size_t nConstCellValue)
{
    pDoc->SetValue(3, 0, 0, static_cast<double>(nConstCellValue));  // D1
    for (size_t i = 0; i <= (nNumRows*2); ++i)
    {
        pDoc->SetValue(0, i, 0, static_cast<double>(i));
        pDoc->SetFormula(ScAddress(1, i, 0),
                         "=A" + OUString::number(i+1),
                         formula::FormulaGrammar::GRAM_NATIVE_UI);
    }
}

void ScParallelismTest::testSUMIFImplicitRange()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false);
    m_pDoc->InsertTab(0, "1");

    const size_t nNumRows = 1048;
    const size_t nConstCellValue = 20;
    lcl_setupCommon(m_pDoc, nNumRows, nConstCellValue);
    OUString aSrcRange = "$A$1:$A$" + OUString::number(nNumRows);
    OUString aFormula;
    for (size_t i = 0; i < nNumRows; ++i)
    {
        aFormula = "=SUMIF(" + aSrcRange + ";$D$1;$B$1)";
        m_pDoc->SetFormula(ScAddress(2, i, 0),
                           aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    ScFormulaCell* pCell = m_pDoc->GetFormulaCell(ScAddress(2, 0, 0));
    sc::AutoCalcSwitch aACSwitch2(*m_pDoc, true);
    pCell->InterpretFormulaGroup();  // Start calculation on the F.G at C1

    for (size_t i = 0; i < nNumRows; ++i)
    {
        OString aMsg = "At row " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nConstCellValue, static_cast<size_t>(m_pDoc->GetValue(2, i, 0)));
    }
    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testFGCycleWithPlainFormulaCell1()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false);
    m_pDoc->InsertTab(0, "1");
    const size_t nNumRows = 1048;
    // Column A contains no formula-group
    // A1 = 100
    m_pDoc->SetValue(0, 0, 0, 100.0);
    // A500 = B499 + 1
    m_pDoc->SetFormula(ScAddress(0, 499, 0),
                       "=$B499 + 1",
                       formula::FormulaGrammar::GRAM_NATIVE_UI);
    // Column B has a formula-group referencing column A.
    OUString aFormula;
    for (size_t i = 0; i < nNumRows; ++i)
    {
        aFormula = "=$A" + OUString::number(i+1) + " + 100";
        m_pDoc->SetFormula(ScAddress(1, i, 0),
                           aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }
    m_xDocShell->DoHardRecalc();
    // Value at A500 must be 101
    const size_t nVal = 100;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value at A500", nVal + 1, static_cast<size_t>(m_pDoc->GetValue(0, 499, 0)));
    for (size_t i = 0; i < nNumRows; ++i)
    {
        OString aMsg = "Value at cell B" + OString::number(i+1);
        size_t nExpected = nVal;
        if (i == 0)
            nExpected = 200;
        else if (i == 499)
            nExpected = 201;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpected, static_cast<size_t>(m_pDoc->GetValue(1, i, 0)));
    }
    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testFGCycleWithPlainFormulaCell2()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false);
    m_pDoc->InsertTab(0, "1");
    const size_t nNumRows = 1048;
    // Column A
    OUString aFormula;
    for (size_t i = 0; i < nNumRows; ++i)
    {
        aFormula = "=$B" + OUString::number(i+1) + " + 1";
        m_pDoc->SetFormula(ScAddress(0, i, 0),
                           aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }
    // Column B
    for (size_t i = 0; i < nNumRows; ++i)
    {
        aFormula = "=$C" + OUString::number(i+1) + " + 1";
        m_pDoc->SetFormula(ScAddress(1, i, 0),
                           aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    // Column C has no FG but a cell at C500 that references A499
    m_pDoc->SetFormula(ScAddress(2, 499, 0), // C500
                       "=$A499 + 1",
                       formula::FormulaGrammar::GRAM_NATIVE_UI);
    m_xDocShell->DoHardRecalc();

    size_t nExpected = 0;
    for (size_t i = 0; i < nNumRows; ++i)
    {
        OString aMsg = "Value at cell A" + OString::number(i+1);
        nExpected = 2;
        if (i == 499)  // A500 must have value = 5
            nExpected = 5;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpected, static_cast<size_t>(m_pDoc->GetValue(0, i, 0)));
        aMsg = "Value at cell B" + OString::number(i+1);
        nExpected = 1;
        if (i == 499)  // B500 must have value = 4
            nExpected = 4;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpected, static_cast<size_t>(m_pDoc->GetValue(1, i, 0)));
    }

    // C500 must have value = 3
    nExpected = 3;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value at cell C500", nExpected, static_cast<size_t>(m_pDoc->GetValue(2, 499, 0)));
    m_pDoc->DeleteTab(0);
}

static void lcl_setupMultipleFGColumn(ScDocument* pDocument, size_t nNumRowsInBlock, size_t nNumFG, size_t nOffset)
{
    OUString aFormula;
    ScAddress aAddr(1, 0, 0);
    // Column B with multiple FG's
    for (size_t nFGIdx = 0; nFGIdx < nNumFG; ++nFGIdx)
    {
        size_t nRowStart = 2*nFGIdx*nNumRowsInBlock;
        for (size_t nRow = nRowStart; nRow < (nRowStart + nNumRowsInBlock); ++nRow)
        {
            aAddr.SetRow(nRow);
            aFormula = "=$C" + OUString::number(nRow+1) + " + 0";
            pDocument->SetFormula(aAddr, aFormula,
                                  formula::FormulaGrammar::GRAM_NATIVE_UI);
            // Fill Column C with doubles.
            pDocument->SetValue(2, nRow, 0, static_cast<double>(nFGIdx));
        }
    }

    // Column A with a single FG that depends on Column B.
    size_t nNumRowsInRef = nNumRowsInBlock*2;
    size_t nColAFGLen = 2*nNumRowsInBlock*nNumFG - nNumRowsInRef + 1;
    aAddr.SetCol(0);
    for (size_t nRow = nOffset; nRow < nColAFGLen; ++nRow)
    {
        aAddr.SetRow(nRow);
        aFormula = "=SUM($B" + OUString::number(nRow+1) + ":$B" + OUString::number(nRow+nNumRowsInRef) + ")";
        pDocument->SetFormula(aAddr, aFormula,
                              formula::FormulaGrammar::GRAM_NATIVE_UI);
    }
}

void ScParallelismTest::testMultipleFGColumn()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false);
    m_pDoc->InsertTab(0, "1");

    constexpr size_t nNumRowsInBlock = 200;
    constexpr size_t nNumFG = 50;
    constexpr size_t nNumRowsInRef = nNumRowsInBlock*2;
    constexpr size_t nColAFGLen = 2*nNumRowsInBlock*nNumFG - nNumRowsInRef + 1;
    constexpr size_t nColAStartOffset = nNumRowsInBlock/2;
    lcl_setupMultipleFGColumn(m_pDoc, nNumRowsInBlock, nNumFG, nColAStartOffset);

    m_xDocShell->DoHardRecalc();

    OString aMsg;
    // First cell in the FG in col A references nColAStartOffset cells in second formula-group of column B each having value 1.
    size_t nExpected = nColAStartOffset;
    size_t nIn = 0, nOut = 0;
    for (size_t nRow = nColAStartOffset; nRow < nColAFGLen; ++nRow)
    {
        aMsg = "Value at Cell A" + OString::number(nRow+1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpected, static_cast<size_t>(m_pDoc->GetValue(0, nRow, 0)));
        nIn = static_cast<size_t>(m_pDoc->GetValue(2, nRow+nNumRowsInRef, 0));
        nOut = static_cast<size_t>(m_pDoc->GetValue(2, nRow, 0));
        nExpected = nExpected + nIn - nOut;
    }

    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testFormulaGroupSpanEval()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false);
    m_pDoc->InsertTab(0, "1");

    constexpr size_t nFGLen = 2048;
    OUString aFormula;

    for (size_t nRow = 0; nRow < nFGLen; ++nRow)
    {
        aFormula = "=$C" + OUString::number(nRow+1) + " + 0";
        m_pDoc->SetFormula(ScAddress(1, nRow, 0), aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
        aFormula = "=SUM($B" + OUString::number(nRow+1) + ":$B" + OUString::number(nRow+2) + ")";
        m_pDoc->SetFormula(ScAddress(0, nRow, 0), aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    m_xDocShell->DoHardRecalc();

    for (size_t nRow = 0; nRow < nFGLen; ++nRow)
    {
        m_pDoc->SetValue(2, nRow, 0, 1.0);
        ScFormulaCell* pFCell = m_pDoc->GetFormulaCell(ScAddress(1, nRow, 0));
        pFCell->SetDirtyVar();
        pFCell = m_pDoc->GetFormulaCell(ScAddress(0, nRow, 0));
        pFCell->SetDirtyVar();
    }

    constexpr size_t nSpanStart = 100;
    constexpr size_t nSpanLen = 1024;
    constexpr size_t nSpanEnd = nSpanStart + nSpanLen - 1;

    m_pDoc->SetAutoCalc(true);

    // EnsureFormulaCellResults should only calculate the sepecified range along with the dependent spans recursively and nothing more.
    // The specified range is A99:A1124, and the dependent range is B99:B1125 (since A99 = SUM(B99:B100) and A1124 = SUM(B1124:B1125) )
    bool bAnyDirty = m_pDoc->EnsureFormulaCellResults(ScRange(0, nSpanStart, 0, 0, nSpanEnd, 0));
    CPPUNIT_ASSERT(bAnyDirty);
    m_pDoc->SetAutoCalc(false);

    OString aMsg;
    for (size_t nRow = 0; nRow < nFGLen; ++nRow)
    {
        size_t nExpectedA = 0, nExpectedB = 0;
        // For nRow from 100(nSpanStart) to 1123(nSpanEnd) column A must have the value of 2 and
        // column B should have value 1.

        // For nRow == 1124, column A should have value 0 and column B should have value 1.

        // For all other rows both column A and B must have value 0.
        if (nRow >= nSpanStart)
        {
            if (nRow <= nSpanEnd)
            {
                nExpectedA = 2;
                nExpectedB = 1;
            }
            else if (nRow == nSpanEnd + 1)
                nExpectedB = 1;
        }

        aMsg = "Value at Cell A" + OString::number(nRow+1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpectedA, static_cast<size_t>(m_pDoc->GetValue(0, nRow, 0)));
        aMsg = "Value at Cell B" + OString::number(nRow+1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpectedB, static_cast<size_t>(m_pDoc->GetValue(1, nRow, 0)));
    }

    m_pDoc->DeleteTab(0);
}

void ScParallelismTest::testFormulaGroupSpanEvalNonGroup()
{
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, false);
    m_pDoc->InsertTab(0, "1");

    constexpr size_t nFGLen = 2048;
    OUString aFormula;

    for (size_t nRow = 0; nRow < nFGLen; ++nRow)
    {
        aFormula = "=$B" + OUString::number(nRow+1) + " + 0";
        m_pDoc->SetFormula(ScAddress(0, nRow, 0), aFormula,
                           formula::FormulaGrammar::GRAM_NATIVE_UI);
    }

    m_xDocShell->DoHardRecalc();

    constexpr size_t nNumChanges = 12;
    constexpr size_t nChangeRows[nNumChanges] = {10, 11, 12, 101, 102, 103, 251, 252, 253, 503, 671, 1029};
    for (size_t nIdx = 0; nIdx < nNumChanges; ++nIdx)
    {
        size_t nRow = nChangeRows[nIdx];
        m_pDoc->SetValue(1, nRow, 0, 1.0);
        ScFormulaCell* pFCell = m_pDoc->GetFormulaCell(ScAddress(0, nRow, 0));
        pFCell->SetDirtyVar();
    }

    m_pDoc->SetAutoCalc(true);
    bool bAnyDirty = m_pDoc->EnsureFormulaCellResults(ScRange(0, 9, 0, 0, 1030, 0));
    CPPUNIT_ASSERT(bAnyDirty);
    m_pDoc->SetAutoCalc(false);

    OString aMsg;
    for (size_t nRow = 0, nIdx = 0; nRow < nFGLen; ++nRow)
    {
        size_t nExpected = 0;
        if (nIdx < nNumChanges && nRow == nChangeRows[nIdx])
        {
            nExpected = 1;
            ++nIdx;
        }

        aMsg = "Value at Cell A" + OString::number(nRow+1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), nExpected, static_cast<size_t>(m_pDoc->GetValue(0, nRow, 0)));
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScParallelismTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
