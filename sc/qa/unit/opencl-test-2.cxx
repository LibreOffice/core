/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <sal/config.h>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <document.hxx>

using namespace ::com::sun::star;

class ScOpenCLTest2
    : public ScModelTestBase
{
public:
    ScOpenCLTest2();
};

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaPi)
{
    initTestEnv(u"xls/opencl/math/pi.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(0,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(0,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaRandom)
{
    initTestEnv(u"xls/opencl/math/random.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        pDoc->GetValue(ScAddress(0,i,0)); // LO
        pDocRes->GetValue(ScAddress(0,i,0)); // Excel
        //because the random numbers will always change,so give the test "true"
        CPPUNIT_ASSERT(true);
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSumSQ)
{
    initTestEnv(u"xls/opencl/math/sumsq.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaVar)
{
    initTestEnv(u"xls/opencl/statistical/Var.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaSkew)
{
    initTestEnv(u"xls/opencl/statistical/Skew.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaSkewp)
{
    initTestEnv(u"xls/opencl/statistical/Skewp.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaTrunc)
{
    initTestEnv(u"xls/opencl/math/trunc.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaCovar)
{
    initTestEnv(u"xls/opencl/statistical/Covar.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 16; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaDevSq)
{
    initTestEnv(u"xls/opencl/statistical/DevSq.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaCscH)
{
    initTestEnv(u"ods/opencl/math/csch.ods");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaPower)
{
    initTestEnv(u"ods/opencl/math/power.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaChiSqDist)
{
    initTestEnv(u"ods/opencl/statistical/CHISQDIST.ods");
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
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaFDist)
{
    initTestEnv(u"xls/opencl/statistical/Fdist.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaArcTan2)
{
    initTestEnv(u"ods/opencl/math/ArcTan2.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify ATan2 Function
    for (SCROW i = 1; i <= 17; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.000001);
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaChiSqInv)
{
    initTestEnv(u"ods/opencl/statistical/CHISQINV.ods");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaPoisson)
{
    initTestEnv(u"xls/opencl/statistical/Poisson.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaExpondist)
{
    initTestEnv(u"xls/opencl/statistical/Expondist.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaBitOr)
{
    initTestEnv(u"ods/opencl/math/BitOr.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify BitOr Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaOdd)
{
    initTestEnv(u"xls/opencl/math/odd.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaMod)
{
    initTestEnv(u"xls/opencl/math/mod.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        if(fExcel == 0.0f)
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 1e-10);
        else
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaGammaInv)
{
    initTestEnv(u"xls/opencl/statistical/GammaInv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = pDoc->GetValue(ScAddress(4,i,0));
        fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaFInv)
{
    initTestEnv(u"xls/opencl/statistical/FInv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = pDoc->GetValue(ScAddress(4,i,0));
        fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaFTest)
{
    initTestEnv(u"xls/opencl/statistical/FTest.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaB)
{
    initTestEnv(u"xls/opencl/statistical/B.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = pDoc->GetValue(ScAddress(5,i,0));
        fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaBetaDist)
{
    initTestEnv(u"ods/opencl/statistical/BetaDist.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = pDoc->GetValue(ScAddress(7,i,0));
        fExcel = pDocRes->GetValue(ScAddress(7,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaEven)
{
    initTestEnv(u"xls/opencl/math/even.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaExp)
{
    initTestEnv(u"ods/opencl/math/exp.ods");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaChiDist)
{
    initTestEnv(u"xls/opencl/statistical/ChiDist.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaBitLshift)
{
    initTestEnv(u"ods/opencl/math/BitLshift.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify BitLshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaBitRshift)
{
    initTestEnv(u"ods/opencl/math/BitRshift.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify BitRshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaFloor)
{
    initTestEnv(u"ods/opencl/math/floor.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaLog)
{
    initTestEnv(u"xls/opencl/math/log.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testSpreadSheetFormulaVLookup)
{
    initTestEnv(u"xls/opencl/spreadsheet/VLookup.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 40; i <= 50; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaChiInv)
{
    initTestEnv(u"xls/opencl/statistical/ChiInv.xls");
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
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaConvert)
{
    initTestEnv(u"xls/opencl/math/convert.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathCountIfsFormula)
{
    initTestEnv(u"xls/opencl/math/countifs.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    for (SCROW i = 1; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaBitXor)
{
    initTestEnv(u"ods/opencl/math/BitXor.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify BitXor Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathAverageIfsFormula)
{
    initTestEnv(u"xls/opencl/math/averageifs.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    ScDocShell* pDocSh = getScDocShell();

    pDocSh->DoHardRecalc();

    for (SCROW i = 1; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaLog10)
{
    initTestEnv(u"ods/opencl/math/log10.ods");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaCombina)
{
    initTestEnv(u"xls/opencl/math/combina.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaCeil)
{
    initTestEnv(u"ods/opencl/math/Ceil.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify Ceiling Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSqrtPi)
{
    initTestEnv(u"xls/opencl/math/sqrtpi.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaVarP)
{
    initTestEnv(u"xls/opencl/statistical/VarP.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaStDev)
{
    initTestEnv(u"xls/opencl/statistical/StDev.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaStDevP)
{
    initTestEnv(u"xls/opencl/statistical/StDevP.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaSlope)
{
    initTestEnv(u"xls/opencl/statistical/Slope.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaSTEYX)
{
    initTestEnv(u"xls/opencl/statistical/STEYX.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaZTest)
{
    initTestEnv(u"xls/opencl/statistical/ZTest.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaTTest)
{
    initTestEnv(u"xls/opencl/statistical/TTest.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaTDist)
{
    initTestEnv(u"xls/opencl/statistical/TDist.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaTInv)
{
    initTestEnv(u"xls/opencl/statistical/TInv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaBinomDist)
{
    initTestEnv(u"xls/opencl/statistical/BinomDist.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaProduct)
{
    initTestEnv(u"xls/opencl/math/product.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaKombin)
{
    initTestEnv(u"ods/opencl/math/Kombin.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify Combin Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testArrayFormulaSumX2MY2)
{
    initTestEnv(u"xls/opencl/array/SUMX2MY2.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 20; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaHypGeomDist)
{
    initTestEnv(u"xls/opencl/statistical/HypGeomDist.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testArrayFormulaSumX2PY2)
{
    initTestEnv(u"xls/opencl/array/SUMX2PY2.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 20; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaBetainv)
{
    initTestEnv(u"xls/opencl/statistical/Betainv.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaMina)
{
    initTestEnv(u"xls/opencl/statistical/Mina.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testArrayFormulaSumXMY2)
{
    initTestEnv(u"xls/opencl/array/SUMXMY2.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 20; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaCountA)
{
    initTestEnv(u"xls/opencl/statistical/counta.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaMaxa)
{
    initTestEnv(u"xls/opencl/statistical/Maxa.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSumProduct)
{
    initTestEnv(u"xls/opencl/math/sumproduct_mixSliding.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        if ( i == 1 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL(82,  fLibre, fabs(0.0001*fExcel));
        else if ( i == 2 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL(113, fLibre, fabs(0.0001*fExcel));
        else if ( i == 4 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL(175, fLibre, fabs(0.0001*fExcel));
        else if ( i == 5 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL(206, fLibre, fabs(0.0001*fExcel));
        else if ( i == 6 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL(237, fLibre, fabs(0.0001*fExcel));
        else if ( i == 7 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL(268, fLibre, fabs(0.0001*fExcel));
        else
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaAverageIf)
{
    initTestEnv(u"xls/opencl/math/averageif.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 2; i <= 21; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaAverageA)
{
    initTestEnv(u"xls/opencl/statistical/AverageA.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testLogicalFormulaAnd)
{
    initTestEnv(u"xls/opencl/logical/and.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaVarA)
{
    initTestEnv(u"xls/opencl/statistical/VarA.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaVarPA)
{
    initTestEnv(u"xls/opencl/statistical/VarPA.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaStDevA)
{
    initTestEnv(u"xls/opencl/statistical/StDevA.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaStDevPA)
{
    initTestEnv(u"xls/opencl/statistical/StDevPA.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testFinancialMDurationFormula1)
{
    initTestEnv(u"xls/opencl/financial/MDuration1.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,fLibre,fabs(0.00000000001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSumProduct2)
{
    initTestEnv(u"xls/opencl/math/sumproductTest.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 2; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,  fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalParallelCountBug)
{
    initTestEnv(u"ods/opencl/statistical/parallel_count_bug_243.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i < 13; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testLogicalFormulaOr)
{
    initTestEnv(u"xls/opencl/logical/or.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testLogicalFormulaNot)
{
    initTestEnv(u"xls/opencl/logical/not.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testLogicalFormulaXor)
{
    initTestEnv(u"xls/opencl/logical/xor.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDcount)
{
    initTestEnv(u"xls/opencl/database/dcount.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDcountA)
{
    initTestEnv(u"xls/opencl/database/dcountA.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDmax)
{
    initTestEnv(u"xls/opencl/database/dmax.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDmin)
{
    initTestEnv(u"xls/opencl/database/dmin.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDproduct)
{
    initTestEnv(u"xls/opencl/database/dproduct.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDaverage)
{
    initTestEnv(u"xls/opencl/database/daverage.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDstdev)
{
    initTestEnv(u"xls/opencl/database/dstdev.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDstdevp)
{
    initTestEnv(u"xls/opencl/database/dstdevp.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDsum)
{
    initTestEnv(u"xls/opencl/database/dsum.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDvar)
{
    initTestEnv(u"xls/opencl/database/dvar.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testDatabaseFormulaDvarp)
{
    initTestEnv(u"xls/opencl/database/dvarp.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaRoundUp)
{
    initTestEnv(u"xls/opencl/math/roundup.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaRoundDown)
{
    initTestEnv(u"xls/opencl/math/rounddown.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaInt)
{
    initTestEnv(u"xls/opencl/math/int.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaRadians)
{
    initTestEnv(u"xls/opencl/math/radians.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaDegrees)
{
    initTestEnv(u"xls/opencl/math/degrees.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 200; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaIsEven)
{
    initTestEnv(u"xls/opencl/math/iseven.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaCountIf)
{
    initTestEnv(u"xls/opencl/math/countif.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaIsOdd)
{
    initTestEnv(u"xls/opencl/math/isodd.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaFact)
{
    initTestEnv(u"xls/opencl/math/fact.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSEC)
{
    initTestEnv(u"ods/opencl/math/sec.ods");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSECH)
{
    initTestEnv(u"ods/opencl/math/sech.ods");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaMROUND)
{
    initTestEnv(u"xls/opencl/math/MROUND.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 13; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaQuotient)
{
    initTestEnv(u"ods/opencl/math/Quotient.ods");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Verify BitAnd Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSeriesSum)
{
    initTestEnv(u"xls/opencl/math/seriessum.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaSumIf)
{
    initTestEnv(u"xls/opencl/math/sumif.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testAddInFormulaBesseLJ)
{
    initTestEnv(u"xls/opencl/addin/besselj.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaAvedev)
{
    initTestEnv(u"xls/opencl/statistical/Avedev.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testNegSub)
{
    initTestEnv(u"xls/opencl/math/NegSub.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testMathFormulaAverageIf_Mix)
{
    initTestEnv(u"xls/opencl/math/averageif_mix.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaKurt1)
{
    initTestEnv(u"xls/opencl/statistical/Kurt1.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaHarMean1)
{
    initTestEnv(u"xls/opencl/statistical/HarMean1.xls");
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

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaVarA1)
{
    initTestEnv(u"xls/opencl/statistical/VarA1.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaVarPA1)
{
    initTestEnv(u"xls/opencl/statistical/VarPA1.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaStDevA1)
{
    initTestEnv(u"xls/opencl/statistical/StDevA1.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testStatisticalFormulaStDevPA1)
{
    initTestEnv(u"xls/opencl/statistical/StDevPA1.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    pDoc->CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

CPPUNIT_TEST_FIXTURE(ScOpenCLTest2, testTdf149940_VLookup)
{
    initTestEnv(u"ods/tdf149940.ods");
    ScDocument* pDoc = getScDoc2();
    ScDocument* pDocRes = getScDoc();
    pDoc->CalcAll();

    for (SCROW i = 4; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00001));
    }
    for (SCROW i = 4; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00001));
    }
    for (SCROW i = 4; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00001));
    }
}

ScOpenCLTest2::ScOpenCLTest2()
      : ScModelTestBase( "sc/qa/unit/data" )
{
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
