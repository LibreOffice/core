/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include "scdll.hxx"
#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include "helper/qahelper.hxx"

#include "calcconfig.hxx"
#include "interpre.hxx"

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "formulacell.hxx"
#include "formulagroup.hxx"

#include <svx/svdpage.hxx>

using namespace css;
using namespace css::uno;

class ScParallelismTest : public ScBootstrapFixture
{
public:
    ScParallelismTest();

    virtual void setUp() override;

    void getNewDocShell(ScDocShellRef& rDocShellRef);

    void testSUMIFS();
    void testDivision();
    void testVLOOKUP();

    CPPUNIT_TEST_SUITE(ScParallelismTest);
    CPPUNIT_TEST(testSUMIFS);
    CPPUNIT_TEST(testDivision);
    CPPUNIT_TEST(testVLOOKUP);
    CPPUNIT_TEST_SUITE_END();

private:
    ScDocument *m_pDoc;

    ScDocShellRef m_xDocShell;
};

ScParallelismTest::ScParallelismTest()
      : ScBootstrapFixture( "sc/qa/unit/data" )
{
}

void ScParallelismTest::setUp()
{
    test::BootstrapFixture::setUp();

    ScDLL::Init();

    getNewDocShell(m_xDocShell);
    m_pDoc = &m_xDocShell->GetDocument();

    sc::FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly();
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

CPPUNIT_TEST_SUITE_REGISTRATION(ScParallelismTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
