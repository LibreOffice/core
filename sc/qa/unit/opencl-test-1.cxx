/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <sal/config.h>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <document.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScOpenCLTest1
    : public ScModelTestBase
{
public:
    ScOpenCLTest1();
};

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testCompilerHorizontal)
{
    initTestEnv(u"ods/opencl/compiler/horizontal.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(12, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(12, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = pDoc->GetValue(ScAddress(13, i, 0));
        fExcel = pDocRes->GetValue(ScAddress(13, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = pDoc->GetValue(ScAddress(14, i, 0));
        fExcel = pDocRes->GetValue(ScAddress(14, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testCompilerNested)
{
    initTestEnv(u"ods/opencl/compiler/nested.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testCompilerString)
{
    initTestEnv(u"ods/opencl/compiler/string.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));

        fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testCompilerInEq)
{
    initTestEnv(u"ods/opencl/compiler/ineq.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 7; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testCompilerPrecision)
{
    initTestEnv(u"ods/opencl/compiler/precision.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    // Check that values with and without opencl are the same/similar enough.
    enableOpenCL();
    pDoc->CalcAll();
    disableOpenCL();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 3; ++i)
    {
        double fOpenCL = pDoc->GetValue(ScAddress(0, i, 0));
        double fNormal = pDocRes->GetValue(ScAddress(0, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fNormal, fOpenCL, fabs(1e-14*fOpenCL));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testSystematic)
{
    initTestEnv(u"xls/systematic.xls");

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    int nAVertBegin(0), nAVertEnd(0), nBVertBegin(0), nBVertEnd(0);
    int nAHorEnd(0), nBHorEnd(0);

    int nRow, nCol;
    for (nRow = 0; nRow < 1000; ++nRow)
    {
        if (pDoc->GetString(ScAddress(0, nRow, 0)) == "a")
        {
            nAVertBegin = nRow + 1;

            for (nCol = 0; nCol < 1000; ++nCol)
            {
                if (pDoc->GetString(ScAddress(nCol, nRow, 0)) != "a")
                {
                    nAHorEnd = nCol;
                    break;
                }
            }
            break;
        }
    }
    for (; nRow < 1000; ++nRow)
    {
        if (pDoc->GetString(ScAddress(0, nRow, 0)) != "a")
        {
            nAVertEnd = nRow;
            break;
        }
    }

    for (; nRow < 1000; ++nRow)
    {
        if (pDoc->GetString(ScAddress(0, nRow, 0)) == "b")
        {
            nBVertBegin = nRow + 1;

            for (nCol = 0; nCol < 1000; ++nCol)
            {
                if (pDoc->GetString(ScAddress(nCol, nRow, 0)) != "b")
                {
                    nBHorEnd = nCol;
                    break;
                }
            }
            break;
        }
    }
    for (; nRow < 1000; ++nRow)
    {
        if (pDoc->GetString(ScAddress(0, nRow, 0)) != "b")
        {
            nBVertEnd = nRow;
            break;
        }
    }

    CPPUNIT_ASSERT(nAVertBegin != 0);
    CPPUNIT_ASSERT(nBVertBegin != 0);
    CPPUNIT_ASSERT(nAVertEnd > nAVertBegin + 100);
    CPPUNIT_ASSERT(nBVertEnd > nBVertBegin + 100);
    CPPUNIT_ASSERT_EQUAL(nAVertEnd-nAVertBegin, nBVertEnd-nBVertBegin);
    CPPUNIT_ASSERT(nAHorEnd > 10);
    CPPUNIT_ASSERT(nBHorEnd > 10);
    CPPUNIT_ASSERT_EQUAL(nAHorEnd, nBHorEnd);

    for (SCROW i = nAVertBegin; i < nAVertEnd; ++i)
    {
        for (int j = 1; j < nAHorEnd; ++j)
        {
            double fLibre = pDoc->GetValue(ScAddress(j, i, 0));
            double fExcel = pDoc->GetValue(ScAddress(j, nBVertBegin + (i - nAVertBegin), 0));

            const OString sFailedMessage =
                OStringChar(static_cast<char>('A'+j)) +
                OString::number(i+1) +
                "!=" +
                OStringChar(static_cast<char>('A'+j)) +
                OString::number(nBVertBegin+(i-nAVertBegin)+1);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sFailedMessage.getStr(), fExcel, fLibre, 1e-10);
        }
    }
}


CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testSharedFormulaXLS)
{
    initTestEnv(u"xls/sum_ex.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 6; i < 14; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 15; i < 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 19; i < 22; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 23; i < 25; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        //double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        // There seems to be a bug in LibreOffice beta
        ASSERT_DOUBLES_EQUAL(/*fExcel*/ 60.0, fLibre);
    }

    for (SCROW i = 25; i < 27; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 28; i < 35; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    // workaround for a Calc beta bug
    ASSERT_DOUBLES_EQUAL(25.0, pDoc->GetValue(ScAddress(2, 35, 0)));
    ASSERT_DOUBLES_EQUAL(24.0, pDoc->GetValue(ScAddress(2, 36, 0)));

    for (SCROW i = 38; i < 43; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 5; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 1));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 1));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 5; i < 10; ++i)
    {
        for (SCCOL j = 6; j < 11; ++j)
        {
            double fLibre = pDoc->GetValue(ScAddress(j, i, 1));
            double fExcel = pDocRes->GetValue(ScAddress(j, i, 1));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre,
                fabs(fExcel*0.0001));
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaCos)
{
    initTestEnv(u"xls/opencl/math/cos.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaSinh)
{
    initTestEnv(u"xls/opencl/math/sinh.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialFormula)
{
    initTestEnv(u"xls/opencl/financial/general.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,2));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,3));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,3));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,4));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,4));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,5));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,5));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,6));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,6));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,7));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,7));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,8));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,8));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,9));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,9));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,10));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,10));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7,i,11));
        double fExcel = pDocRes->GetValue(ScAddress(7,i,11));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,12));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,12));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,13));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,13));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,14));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,14));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,15));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,15));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,16));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,16));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,17));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,17));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,18));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,18));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,19));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,19));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaCorrel)
{
    initTestEnv(u"ods/opencl/statistical/Correl.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaFisher)
{
    initTestEnv(u"xls/opencl/statistical/Fisher.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaFisherInv)
{
    initTestEnv(u"xls/opencl/statistical/FisherInv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaGamma)
{
    initTestEnv(u"xls/opencl/statistical/Gamma.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialFvscheduleFormula)
{
    initTestEnv(u"xls/opencl/financial/Fvschedule.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaAbs)
{
    initTestEnv(u"ods/opencl/math/Abs.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ABS Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialSYDFormula)
{
    initTestEnv(u"xls/opencl/financial/SYD.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

// this test has intermittent failures on OSX
#if !defined MACOSX
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialIRRFormula)
{
    initTestEnv(u"xls/opencl/financial/IRR.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
#endif

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaGammaLn)
{
    initTestEnv(u"xls/opencl/statistical/GammaLn.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaGauss)
{
    initTestEnv(u"xls/opencl/statistical/Gauss.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaGeoMean)
{
    initTestEnv(u"xls/opencl/statistical/GeoMean.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaHarMean)
{
    initTestEnv(u"xls/opencl/statistical/HarMean.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialSLNFormula)
{
    initTestEnv(u"xls/opencl/financial/SLN.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialMIRRFormula)
{
    initTestEnv(u"xls/opencl/financial/MIRR.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialCoupdaybsFormula)
{
    initTestEnv(u"xls/opencl/financial/Coupdaybs.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialDollardeFormula)
{
    initTestEnv(u"xls/opencl/financial/Dollarde.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialCoupdaysFormula)
{
    initTestEnv(u"xls/opencl/financial/Coupdays.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialCoupdaysncFormula)
{
    initTestEnv(u"xls/opencl/financial/Coupdaysnc.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialRateFormula)
{
    initTestEnv(u"xls/opencl/financial/RATE.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialAccrintmFormula)
{
    initTestEnv(u"xls/opencl/financial/Accrintm.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialCoupnumFormula)
{
    initTestEnv(u"xls/opencl/financial/Coupnum.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaNegbinomdist)
{
    initTestEnv(u"xls/opencl/statistical/Negbinomdist.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaSin)
{
    initTestEnv(u"xls/opencl/math/sin.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaTan)
{
    initTestEnv(u"xls/opencl/math/tan.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaTanH)
{
    initTestEnv(u"xls/opencl/math/tanh.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaSqrt)
{
    initTestEnv(u"xls/opencl/math/sqrt.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialPriceFormula)
{
    initTestEnv(u"xls/opencl/financial/Price.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialDollarfrFormula)
{
    initTestEnv(u"xls/opencl/financial/Dollarfr.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialPriceDiscFormula)
{
    initTestEnv(u"xls/opencl/financial/PriceDisc.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialODDLPRICEFormula)
{
    initTestEnv(u"xls/opencl/financial/Oddlprice.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(8, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialOddlyieldFormula)
{
    initTestEnv(u"xls/opencl/financial/Oddlyield.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(8, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialDISCFormula)
{
    initTestEnv(u"xls/opencl/financial/DISC.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialPVFormula)
{
    initTestEnv(u"xls/opencl/financial/PV.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialINTRATEFormula)
{
    initTestEnv(u"xls/opencl/financial/INTRATE.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaStandard)
{
    initTestEnv(u"xls/opencl/statistical/Standard.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaWeibull)
{
    initTestEnv(u"xls/opencl/statistical/Weibull.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre,
            fExcel == 0?1e-4:fabs(1e-4*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaPearson)
{
    initTestEnv(u"xls/opencl/statistical/Pearson.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaRsq)
{
    initTestEnv(u"xls/opencl/statistical/Rsq.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaCosh)
{
    initTestEnv(u"xls/opencl/math/cosh.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaKurt)
{
    initTestEnv(u"xls/opencl/statistical/Kurt.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaCot)
{
    initTestEnv(u"ods/opencl/math/cot.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaCsc)
{
    initTestEnv(u"ods/opencl/math/csc.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaCoth)
{
    initTestEnv(u"ods/opencl/math/coth.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialXNPVFormula)
{
    initTestEnv(u"xls/opencl/financial/XNPV.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 16; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaIntercept)
{
    initTestEnv(u"xls/opencl/statistical/Intercept.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialAmordegrcFormula)
{
    initTestEnv(u"xls/opencl/financial/Amordegrc.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialISPMTFormula)
{
    initTestEnv(u"xls/opencl/financial/ISPMT.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaMedian)
{
    initTestEnv(u"xls/opencl/statistical/Median.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaNormdist)
{
    initTestEnv(u"xls/opencl/statistical/Normdist.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaNormsdist)
{
    initTestEnv(u"xls/opencl/statistical/Normsdist.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaPermut)
{
    initTestEnv(u"xls/opencl/statistical/Permut.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaPermutation)
{
    initTestEnv(u"xls/opencl/statistical/Permutation.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaPhi)
{
    initTestEnv(u"xls/opencl/statistical/Phi.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaLogInv)
{
    initTestEnv(u"xls/opencl/statistical/LogInv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialNPERFormula)
{
    initTestEnv(u"xls/opencl/financial/NPER.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaForecast)
{
    initTestEnv(u"xls/opencl/statistical/Forecast.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialAmorlincFormula)
{
    initTestEnv(u"xls/opencl/financial/Amorlinc.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialDDBFormula)
{
    initTestEnv(u"xls/opencl/financial/ddb.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialPriceMatFormula)
{
    initTestEnv(u"xls/opencl/financial/PriceMat.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialFormulaReceived)
{
    initTestEnv(u"xls/opencl/financial/Received.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialFormulaCumipmt)
{
    initTestEnv(u"xls/opencl/financial/Cumipmt.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialFormulaCumprinc)
{
    initTestEnv(u"xls/opencl/financial/Cumprinc.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialRRIFormula)
{
    initTestEnv(u"xls/opencl/financial/RRI.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialEFFECT_ADDFormula)
{
    initTestEnv(u"xls/opencl/financial/EFFECT_ADD.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialNominalFormula)
{
    initTestEnv(u"xls/opencl/financial/Nominal.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();


    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialTBILLEQFormula)
{
    initTestEnv(u"xls/opencl/financial/TBILLEQ.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialTBILLPRICEFormula)
{
    initTestEnv(u"xls/opencl/financial/TBILLPRICE.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialTBILLYIELDFormula)
{
    initTestEnv(u"xls/opencl/financial/TBILLYIELD.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialYIELDFormula)
{
    initTestEnv(u"xls/opencl/financial/YIELD.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialYIELDDISCFormula)
{
    initTestEnv(u"xls/opencl/financial/YIELDDISC.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialYIELDMATFormula)
{
    initTestEnv(u"xls/opencl/financial/YIELDMAT.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialPMTFormula)
{
    initTestEnv(u"xls/opencl/financial/PMT.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialDurationFormula)
{
    initTestEnv(u"ods/opencl/financial/Duration.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaLogNormDist)
{
    initTestEnv(u"ods/opencl/statistical/LogNormDist.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcCos)
{
    initTestEnv(u"ods/opencl/math/ArcCos.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ACos Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialPPMTFormula)
{
    initTestEnv(u"xls/opencl/financial/PPMT.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialNPVFormula)
{
    initTestEnv(u"xls/opencl/financial/NPV.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialDuration_ADDFormula)
{
    initTestEnv(u"xls/opencl/financial/Duration_ADD.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaNorminv)
{
    initTestEnv(u"xls/opencl/statistical/Norminv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaNormsinv)
{
    initTestEnv(u"xls/opencl/statistical/Normsinv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcCosHyp)
{
    initTestEnv(u"ods/opencl/math/ArcCosHyp.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ACosH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialMDurationFormula)
{
    initTestEnv(u"xls/opencl/financial/MDuration.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcCot)
{
    initTestEnv(u"ods/opencl/math/ArcCot.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ACot Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialFVFormula)
{
    initTestEnv(u"xls/opencl/financial/FV.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialDBFormula)
{
    initTestEnv(u"xls/opencl/financial/db.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialCouppcdFormula)
{
    initTestEnv(u"xls/opencl/financial/Couppcd.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathSumIfsFormula)
{
    initTestEnv(u"xls/opencl/math/sumifs.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(7,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(8,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(8,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcCotHyp)
{
    initTestEnv(u"ods/opencl/math/ArcCotHyp.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcSin)
{
    initTestEnv(u"ods/opencl/math/ArcSin.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialVDBFormula)
{
    initTestEnv(u"xls/opencl/financial/VDB.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 15; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 30; i <= 41; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialIPMTFormula)
{
    initTestEnv(u"xls/opencl/financial/IPMT.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaConfidence)
{
    initTestEnv(u"xls/opencl/statistical/Confidence.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialCoupncdFormula)
{
    initTestEnv(u"xls/opencl/financial/Coupncd.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinancialAccrintFormula)
{
    initTestEnv(u"xls/opencl/financial/Accrint.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaCritBinom)
{
    initTestEnv(u"xls/opencl/statistical/CritBinom.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcSinHyp)
{
    initTestEnv(u"ods/opencl/math/ArcSinHyp.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ASinH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcTan)
{
    initTestEnv(u"ods/opencl/math/ArcTan.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ATan Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaArcTanHyp)
{
    initTestEnv(u"ods/opencl/math/ArcTanHyp.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ATanH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testFinacialNPER1Formula)
{
    initTestEnv(u"xls/opencl/financial/NPER1.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaBitAnd)
{
    initTestEnv(u"ods/opencl/math/BitAnd.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify BitAnd Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaLN)
{
    initTestEnv(u"xls/opencl/math/LN.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testMathFormulaRound)
{
    initTestEnv(u"xls/opencl/math/ROUND.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 15; i <= 25; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest1, testStatisticalFormulaGammaDist)
{
    initTestEnv(u"xls/opencl/statistical/GammaDist.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

ScOpenCLTest1::ScOpenCLTest1()
      : ScModelTestBase( "sc/qa/unit/data" )
{
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
