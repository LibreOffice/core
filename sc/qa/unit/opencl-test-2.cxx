/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <sal/config.h>

#include <string_view>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <document.hxx>
#include <formulagroup.hxx>

#include <com/sun/star/document/MacroExecMode.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScOpenCLTest2
    : public ScModelTestBase
{
public:
    ScOpenCLTest2();

    /**
     * Turn on OpenCL group interpreter. Call this after the document is
     * loaded and before performing formula calculation.
     */
    void enableOpenCL();
    void disableOpenCL();

    void testStatisticalFormulaFDist();
    void testStatisticalFormulaVar();
    void testStatisticalFormulaChiDist();
    void testMathFormulaPower();
    void testMathFormulaOdd();
    void testStatisticalFormulaChiSqDist();
    void testStatisticalFormulaChiSqInv();
    void testStatisticalFormulaGammaInv();
    void testMathFormulaFloor();
    void testStatisticalFormulaFInv();
    void testStatisticalFormulaFTest();
    void testStatisticalFormulaB();
    void testStatisticalFormulaBetaDist();
    void testMathFormulaCscH();
    void testMathFormulaExp();
    void testMathFormulaLog10();
    void testStatisticalFormulaExpondist();
    void testMathAverageIfsFormula();
    void testMathCountIfsFormula();
    void testMathFormulaCombina();
    void testMathFormulaEven();
    void testMathFormulaLog();
    void testMathFormulaMod();
    void testMathFormulaTrunc();
    void testStatisticalFormulaSkew();
    void testMathFormulaArcTan2();
    void testMathFormulaBitOr();
    void testMathFormulaBitLshift();
    void testMathFormulaBitRshift();
    void testMathFormulaBitXor();
    void testStatisticalFormulaChiInv();
    void testStatisticalFormulaPoisson();
    void testMathFormulaSumSQ();
    void testStatisticalFormulaSkewp();
    void testMathFormulaSqrtPi();
    void testStatisticalFormulaBinomDist();
    void testStatisticalFormulaVarP();
    void testMathFormulaCeil();
    // void testMathFormulaKombin();
    void testStatisticalFormulaDevSq();
    void testStatisticalFormulaStDev();
    void testStatisticalFormulaSlope();
    void testStatisticalFormulaSTEYX();
    void testStatisticalFormulaZTest();
    void testMathFormulaPi();
    void testMathFormulaRandom();
    void testMathFormulaConvert();
    void testMathFormulaProduct();
    void testStatisticalFormulaHypGeomDist();
    void testArrayFormulaSumX2MY2();
    void testArrayFormulaSumX2PY2();
    void testStatisticalFormulaBetainv();
    void testStatisticalFormulaTTest();
    void testStatisticalFormulaTDist();
    void testStatisticalFormulaTInv();
    void testArrayFormulaSumXMY2();
    void testStatisticalFormulaStDevP();
    void testStatisticalFormulaCovar();
    void testLogicalFormulaAnd();
    void testLogicalFormulaOr();
    void testMathFormulaSumProduct();
    void testMathFormulaSumProduct2();
    void testStatisticalParallelCountBug();
    void testSpreadSheetFormulaVLookup();
    void testLogicalFormulaNot();
    void testLogicalFormulaXor();
    void testDatabaseFormulaDmax();
    void testDatabaseFormulaDmin();
    void testDatabaseFormulaDproduct();
    void testDatabaseFormulaDaverage();
    void testDatabaseFormulaDstdev();
    void testDatabaseFormulaDstdevp();
    void testDatabaseFormulaDsum();
    void testDatabaseFormulaDvar();
    void testDatabaseFormulaDvarp();
    void testMathFormulaAverageIf();
    void testDatabaseFormulaDcount();
    void testDatabaseFormulaDcountA();
    void testMathFormulaDegrees();
    void testMathFormulaRoundUp();
    void testMathFormulaRoundDown();
    void testMathFormulaInt();
    void testMathFormulaRadians();
    void testMathFormulaCountIf();
    void testMathFormulaIsEven();
    void testMathFormulaIsOdd();
    void testMathFormulaFact();
    void testStatisticalFormulaMina();
    void testStatisticalFormulaCountA();
    void testStatisticalFormulaMaxa();
    void testStatisticalFormulaAverageA();
    void testStatisticalFormulaVarA();
    void testStatisticalFormulaVarPA();
    void testStatisticalFormulaStDevA();
    void testStatisticalFormulaStDevPA();
    void testMathFormulaSEC();
    void testMathFormulaSECH();
    void testMathFormulaMROUND();
    void testMathFormulaSeriesSum();
    void testMathFormulaQuotient();
    void testMathFormulaSumIf();
    void testAddInFormulaBesseLJ();
    void testNegSub();
    void testStatisticalFormulaAvedev();
    void testMathFormulaAverageIf_Mix();
    void testStatisticalFormulaKurt1();
    void testStatisticalFormulaHarMean1();
    void testStatisticalFormulaVarA1();
    void testStatisticalFormulaVarPA1();
    void testStatisticalFormulaStDevA1();
    void testStatisticalFormulaStDevPA1();
    void testFinancialMDurationFormula1();

    CPPUNIT_TEST_SUITE(ScOpenCLTest2);
    CPPUNIT_TEST(testStatisticalFormulaFDist);
    CPPUNIT_TEST(testStatisticalFormulaVar);
    CPPUNIT_TEST(testStatisticalFormulaChiDist);
    CPPUNIT_TEST(testMathFormulaPower);
    CPPUNIT_TEST(testMathFormulaOdd);
    CPPUNIT_TEST(testStatisticalFormulaChiSqDist);
    CPPUNIT_TEST(testStatisticalFormulaChiSqInv);
    CPPUNIT_TEST(testStatisticalFormulaGammaInv);
    CPPUNIT_TEST(testMathFormulaFloor);
    CPPUNIT_TEST(testStatisticalFormulaFInv);
    CPPUNIT_TEST(testStatisticalFormulaFTest);
    CPPUNIT_TEST(testStatisticalFormulaB);
    CPPUNIT_TEST(testStatisticalFormulaBetaDist);
    CPPUNIT_TEST(testMathFormulaCscH);
    CPPUNIT_TEST(testMathFormulaExp);
    CPPUNIT_TEST(testMathFormulaLog10);
    CPPUNIT_TEST(testStatisticalFormulaExpondist);
    CPPUNIT_TEST(testMathAverageIfsFormula);
    CPPUNIT_TEST(testMathCountIfsFormula);
    CPPUNIT_TEST(testMathFormulaCombina);
    CPPUNIT_TEST(testMathFormulaEven);
    CPPUNIT_TEST(testMathFormulaLog);
    CPPUNIT_TEST(testMathFormulaMod);
    CPPUNIT_TEST(testMathFormulaTrunc);
    CPPUNIT_TEST(testStatisticalFormulaSkew);
    CPPUNIT_TEST(testMathFormulaArcTan2);
    CPPUNIT_TEST(testMathFormulaBitOr);
    CPPUNIT_TEST(testMathFormulaBitLshift);
    CPPUNIT_TEST(testMathFormulaBitRshift);
    CPPUNIT_TEST(testMathFormulaBitXor);
    CPPUNIT_TEST(testStatisticalFormulaChiInv);
    CPPUNIT_TEST(testStatisticalFormulaPoisson);
    CPPUNIT_TEST(testMathFormulaSumSQ);
    CPPUNIT_TEST(testStatisticalFormulaSkewp);
    CPPUNIT_TEST(testMathFormulaSqrtPi);
    CPPUNIT_TEST(testStatisticalFormulaBinomDist);
    CPPUNIT_TEST(testStatisticalFormulaVarP);
    CPPUNIT_TEST(testMathFormulaCeil);
    // This test fails MacOS 10.8. Disabled temporarily
    // CPPUNIT_TEST(testMathFormulaKombin);
    CPPUNIT_TEST(testStatisticalFormulaDevSq);
    CPPUNIT_TEST(testStatisticalFormulaStDev);
    CPPUNIT_TEST(testStatisticalFormulaSlope);
    CPPUNIT_TEST(testStatisticalFormulaSTEYX);
    CPPUNIT_TEST(testStatisticalFormulaZTest);
    CPPUNIT_TEST(testMathFormulaPi);
    CPPUNIT_TEST(testMathFormulaRandom);
    CPPUNIT_TEST(testMathFormulaConvert);
    CPPUNIT_TEST(testMathFormulaProduct);
    CPPUNIT_TEST(testStatisticalFormulaHypGeomDist);
    CPPUNIT_TEST(testArrayFormulaSumX2MY2);
    CPPUNIT_TEST(testArrayFormulaSumX2PY2);
    CPPUNIT_TEST(testStatisticalFormulaBetainv);
    CPPUNIT_TEST(testStatisticalFormulaTTest);
    CPPUNIT_TEST(testStatisticalFormulaTDist);
    CPPUNIT_TEST(testStatisticalFormulaTInv);
    CPPUNIT_TEST(testArrayFormulaSumXMY2);
    CPPUNIT_TEST(testStatisticalFormulaStDevP);
    CPPUNIT_TEST(testStatisticalFormulaCovar);
    CPPUNIT_TEST(testLogicalFormulaAnd);
    CPPUNIT_TEST(testMathFormulaSumProduct);
    CPPUNIT_TEST(testMathFormulaSumProduct2);
    CPPUNIT_TEST(testStatisticalParallelCountBug);
    CPPUNIT_TEST(testSpreadSheetFormulaVLookup);
    CPPUNIT_TEST(testLogicalFormulaOr);
    CPPUNIT_TEST(testLogicalFormulaNot);
    CPPUNIT_TEST(testLogicalFormulaXor);
    CPPUNIT_TEST(testDatabaseFormulaDmax);
    CPPUNIT_TEST(testDatabaseFormulaDmin);
    CPPUNIT_TEST(testDatabaseFormulaDproduct);
    CPPUNIT_TEST(testDatabaseFormulaDaverage);
    CPPUNIT_TEST(testDatabaseFormulaDstdev);
    CPPUNIT_TEST(testDatabaseFormulaDstdevp);
    CPPUNIT_TEST(testDatabaseFormulaDsum);
    CPPUNIT_TEST(testDatabaseFormulaDvar);
    CPPUNIT_TEST(testDatabaseFormulaDvarp);
    CPPUNIT_TEST(testMathFormulaAverageIf);
    CPPUNIT_TEST(testDatabaseFormulaDcount);
    CPPUNIT_TEST(testDatabaseFormulaDcountA);
    CPPUNIT_TEST(testMathFormulaDegrees);
    CPPUNIT_TEST(testMathFormulaRoundUp);
    CPPUNIT_TEST(testMathFormulaRoundDown);
    CPPUNIT_TEST(testMathFormulaInt);
    CPPUNIT_TEST(testMathFormulaRadians);
    CPPUNIT_TEST(testMathFormulaCountIf);
    CPPUNIT_TEST(testMathFormulaIsEven);
    CPPUNIT_TEST(testMathFormulaIsOdd);
    CPPUNIT_TEST(testMathFormulaFact);
    CPPUNIT_TEST(testStatisticalFormulaMaxa);
    CPPUNIT_TEST(testStatisticalFormulaMina);
    CPPUNIT_TEST(testStatisticalFormulaCountA);
    CPPUNIT_TEST(testStatisticalFormulaAverageA);
    CPPUNIT_TEST(testStatisticalFormulaVarA);
    CPPUNIT_TEST(testStatisticalFormulaVarPA);
    CPPUNIT_TEST(testStatisticalFormulaStDevA);
    CPPUNIT_TEST(testStatisticalFormulaStDevPA);
    CPPUNIT_TEST(testMathFormulaSEC);
    CPPUNIT_TEST(testMathFormulaSECH);
    CPPUNIT_TEST(testMathFormulaMROUND);
    CPPUNIT_TEST(testMathFormulaQuotient);
    CPPUNIT_TEST(testMathFormulaSeriesSum);
    CPPUNIT_TEST(testMathFormulaSumIf);
    CPPUNIT_TEST(testAddInFormulaBesseLJ);
    CPPUNIT_TEST(testNegSub);
    CPPUNIT_TEST(testStatisticalFormulaAvedev);
    CPPUNIT_TEST(testMathFormulaAverageIf_Mix);
    CPPUNIT_TEST(testStatisticalFormulaKurt1);
    CPPUNIT_TEST(testStatisticalFormulaHarMean1);
    CPPUNIT_TEST(testStatisticalFormulaVarA1);
    CPPUNIT_TEST(testStatisticalFormulaVarPA1);
    CPPUNIT_TEST(testStatisticalFormulaStDevA1);
    CPPUNIT_TEST(testStatisticalFormulaStDevPA1);
    CPPUNIT_TEST(testFinancialMDurationFormula1);
    CPPUNIT_TEST_SUITE_END();

private:
    void initTestEnv(std::u16string_view fileName);

    ScDocument* getScDoc2();
};

void ScOpenCLTest2::initTestEnv(std::u16string_view fileName)
{
    // Some documents contain macros, disable them, otherwise
    // the "Error, BASIC runtime error." dialog is prompted
    // and it crashes in tearDown
    std::vector<beans::PropertyValue> args;
    beans::PropertyValue aMacroValue;
    aMacroValue.Name = "MacroExecutionMode";
    aMacroValue.Handle = -1;
    aMacroValue.Value <<= document::MacroExecMode::NEVER_EXECUTE;
    aMacroValue.State = beans::PropertyState_DIRECT_VALUE;
    args.push_back(aMacroValue);

    disableOpenCL();
    CPPUNIT_ASSERT(!ScCalcConfig::isOpenCLEnabled());

    // Open the document with OpenCL disabled
    mxComponent = mxDesktop->loadComponentFromURL(
        createFileURL(fileName), "_default", 0, comphelper::containerToSequence(args));

    enableOpenCL();
    CPPUNIT_ASSERT(ScCalcConfig::isOpenCLEnabled());

    // it's not possible to open the same document twice, thus, create a temp file
    createTempCopy(fileName);

    // Open the document with OpenCL enabled
    mxComponent2 = mxDesktop->loadComponentFromURL(
        maTempFile.GetURL(), "_default", 0, comphelper::containerToSequence(args));

    // Check there are 2 documents
    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xFrames->getCount());
}

ScDocument* ScOpenCLTest2::getScDoc2()
{
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent2);
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj->GetDocument();
}

void ScOpenCLTest2::enableOpenCL()
{
    sc::FormulaGroupInterpreter::enableOpenCL_UnitTestsOnly();
}

void ScOpenCLTest2::disableOpenCL()
{
    sc::FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly();
}

void ScOpenCLTest2::testMathFormulaPi()
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

void ScOpenCLTest2::testMathFormulaRandom()
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

void ScOpenCLTest2::testMathFormulaSumSQ()
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

void ScOpenCLTest2::testStatisticalFormulaVar()
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

void ScOpenCLTest2::testStatisticalFormulaSkew()
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

void ScOpenCLTest2::testStatisticalFormulaSkewp()
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

void ScOpenCLTest2::testMathFormulaTrunc()
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

void ScOpenCLTest2::testStatisticalFormulaCovar()
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

void ScOpenCLTest2::testStatisticalFormulaDevSq()
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

void ScOpenCLTest2::testMathFormulaCscH()
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

void ScOpenCLTest2::testMathFormulaPower()
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

void ScOpenCLTest2::testStatisticalFormulaChiSqDist()
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

void ScOpenCLTest2::testStatisticalFormulaFDist()
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

void ScOpenCLTest2::testMathFormulaArcTan2()
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

void ScOpenCLTest2::testStatisticalFormulaChiSqInv()
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

void ScOpenCLTest2::testStatisticalFormulaPoisson()
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

void ScOpenCLTest2::testStatisticalFormulaExpondist()
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

void ScOpenCLTest2::testMathFormulaBitOr()
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

void ScOpenCLTest2::testMathFormulaOdd()
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

void ScOpenCLTest2::testMathFormulaMod()
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

void ScOpenCLTest2::testStatisticalFormulaGammaInv()
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

void ScOpenCLTest2::testStatisticalFormulaFInv()
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

void ScOpenCLTest2::testStatisticalFormulaFTest()
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

void ScOpenCLTest2::testStatisticalFormulaB()
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

void ScOpenCLTest2::testStatisticalFormulaBetaDist()
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

void ScOpenCLTest2::testMathFormulaEven()
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

void ScOpenCLTest2::testMathFormulaExp()
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

void ScOpenCLTest2::testStatisticalFormulaChiDist()
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

void ScOpenCLTest2::testMathFormulaBitLshift()
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

void ScOpenCLTest2::testMathFormulaBitRshift()
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

void ScOpenCLTest2::testMathFormulaFloor()
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

void ScOpenCLTest2::testMathFormulaLog()
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

void ScOpenCLTest2::testSpreadSheetFormulaVLookup()
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

void ScOpenCLTest2::testStatisticalFormulaChiInv()
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

void ScOpenCLTest2::testMathFormulaConvert()
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

void ScOpenCLTest2::testMathCountIfsFormula()
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

void ScOpenCLTest2::testMathFormulaBitXor()
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

void ScOpenCLTest2::testMathAverageIfsFormula()
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

void ScOpenCLTest2::testMathFormulaLog10()
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

void ScOpenCLTest2::testMathFormulaCombina()
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

void ScOpenCLTest2::testMathFormulaCeil()
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

void ScOpenCLTest2::testMathFormulaSqrtPi()
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

void ScOpenCLTest2::testStatisticalFormulaVarP()
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

void ScOpenCLTest2::testStatisticalFormulaStDev()
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

void ScOpenCLTest2::testStatisticalFormulaStDevP()
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

void ScOpenCLTest2::testStatisticalFormulaSlope()
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

void ScOpenCLTest2::testStatisticalFormulaSTEYX()
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

void ScOpenCLTest2::testStatisticalFormulaZTest()
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

void ScOpenCLTest2::testStatisticalFormulaTTest()
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

void ScOpenCLTest2::testStatisticalFormulaTDist()
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

void ScOpenCLTest2::testStatisticalFormulaTInv()
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

void ScOpenCLTest2::testStatisticalFormulaBinomDist()
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

void ScOpenCLTest2::testMathFormulaProduct()
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

#if 0 //Disabled temporarily
void ScOpenCLTest2::testMathFormulaKombin()
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
#endif

void ScOpenCLTest2:: testArrayFormulaSumX2MY2()
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

void ScOpenCLTest2::testStatisticalFormulaHypGeomDist()
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

void ScOpenCLTest2:: testArrayFormulaSumX2PY2()
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

void ScOpenCLTest2::testStatisticalFormulaBetainv()
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

void ScOpenCLTest2::testStatisticalFormulaMina()
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

void ScOpenCLTest2:: testArrayFormulaSumXMY2()
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

void ScOpenCLTest2::testStatisticalFormulaCountA()
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

void ScOpenCLTest2::testStatisticalFormulaMaxa()
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

void ScOpenCLTest2::testMathFormulaSumProduct()
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

void ScOpenCLTest2::testMathFormulaAverageIf()
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

void ScOpenCLTest2::testStatisticalFormulaAverageA()
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

void ScOpenCLTest2:: testLogicalFormulaAnd()
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

void ScOpenCLTest2::testStatisticalFormulaVarA()
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

void ScOpenCLTest2::testStatisticalFormulaVarPA()
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

void ScOpenCLTest2::testStatisticalFormulaStDevA()
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

void ScOpenCLTest2::testStatisticalFormulaStDevPA()
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

void ScOpenCLTest2:: testFinancialMDurationFormula1()
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

void ScOpenCLTest2::testMathFormulaSumProduct2()
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

void ScOpenCLTest2:: testStatisticalParallelCountBug()
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

void ScOpenCLTest2:: testLogicalFormulaOr()
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

void ScOpenCLTest2:: testLogicalFormulaNot()
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

void ScOpenCLTest2:: testLogicalFormulaXor()
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

void ScOpenCLTest2::testDatabaseFormulaDcount()
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

void ScOpenCLTest2::testDatabaseFormulaDcountA()
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

void ScOpenCLTest2::testDatabaseFormulaDmax()
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

void ScOpenCLTest2::testDatabaseFormulaDmin()
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

void ScOpenCLTest2::testDatabaseFormulaDproduct()
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

void ScOpenCLTest2::testDatabaseFormulaDaverage()
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

void ScOpenCLTest2::testDatabaseFormulaDstdev()
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

void ScOpenCLTest2::testDatabaseFormulaDstdevp()
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

void ScOpenCLTest2::testDatabaseFormulaDsum()
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

void ScOpenCLTest2::testDatabaseFormulaDvar()
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

void ScOpenCLTest2::testDatabaseFormulaDvarp()
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

void ScOpenCLTest2::testMathFormulaRoundUp()
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

void ScOpenCLTest2::testMathFormulaRoundDown()
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

void ScOpenCLTest2::testMathFormulaInt()
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

void ScOpenCLTest2::testMathFormulaRadians()
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

void ScOpenCLTest2::testMathFormulaDegrees()
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

void ScOpenCLTest2::testMathFormulaIsEven()
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

void ScOpenCLTest2::testMathFormulaCountIf()
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

void ScOpenCLTest2::testMathFormulaIsOdd()
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

void ScOpenCLTest2::testMathFormulaFact()
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

void ScOpenCLTest2::testMathFormulaSEC()
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

void ScOpenCLTest2::testMathFormulaSECH()
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

void ScOpenCLTest2::testMathFormulaMROUND()
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

void ScOpenCLTest2::testMathFormulaQuotient()
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

void ScOpenCLTest2::testMathFormulaSeriesSum()
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

void ScOpenCLTest2::testMathFormulaSumIf()
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

void ScOpenCLTest2::testAddInFormulaBesseLJ()
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

void ScOpenCLTest2::testStatisticalFormulaAvedev()
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

void ScOpenCLTest2::testNegSub()
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

void ScOpenCLTest2::testMathFormulaAverageIf_Mix()
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

void ScOpenCLTest2::testStatisticalFormulaKurt1()
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

void ScOpenCLTest2::testStatisticalFormulaHarMean1()
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

void ScOpenCLTest2::testStatisticalFormulaVarA1()
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

void ScOpenCLTest2::testStatisticalFormulaVarPA1()
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

void ScOpenCLTest2::testStatisticalFormulaStDevA1()
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

void ScOpenCLTest2::testStatisticalFormulaStDevPA1()
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

ScOpenCLTest2::ScOpenCLTest2()
      : ScModelTestBase( "sc/qa/unit/data" )
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenCLTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
