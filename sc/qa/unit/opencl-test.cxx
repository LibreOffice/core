/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <document.hxx>
#include <formulagroup.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScOpenCLTest
    : public ScBootstrapFixture
{
public:
    ScOpenCLTest();

    /**
     * Turn on OpenCL group interpreter. Call this after the document is
     * loaded and before performing formula calculation.
     */
    void enableOpenCL();
    void disableOpenCL();

    virtual void tearDown() override;

    void testSystematic();
    void testSharedFormulaXLS();
#if 0
    void testSharedFormulaXLSGroundWater();
    void testSharedFormulaXLSStockHistory();
#endif
    void testFinacialFormula();
    void testStatisticalFormulaFisher();
    void testStatisticalFormulaFisherInv();
    void testStatisticalFormulaGamma();
    void testFinacialFvscheduleFormula();
// this test has intermittent failures on OSX
#if !defined MACOSX
    void testFinacialIRRFormula();
#endif
    void testFinacialMIRRFormula();
    void testFinacialRateFormula();
    void testFinancialAccrintmFormula();
    void testFinancialAccrintFormula();
    void testCompilerHorizontal();
    void testCompilerNested();
    void testFinacialSLNFormula();
    void testStatisticalFormulaGammaLn();
    void testStatisticalFormulaGauss();
    void testStatisticalFormulaGeoMean();
    void testStatisticalFormulaHarMean();
    void testFinancialCoupdaybsFormula();
    void testFinacialDollardeFormula();
    void testCompilerString();
    void testCompilerInEq();
    void testCompilerPrecision();
    void testFinacialDollarfrFormula();
    void testFinacialSYDFormula();
    void testStatisticalFormulaCorrel();
    void testFinancialCoupdaysFormula();
    void testFinancialCoupdaysncFormula();
    void testFinacialDISCFormula();
    void testFinacialINTRATEFormula();
    void testMathFormulaCos();
    void testMathFormulaCsc();
    void testStatisticalFormulaRsq();
    void testStatisticalFormulaPearson();
    void testStatisticalFormulaNegbinomdist();
    void testFinacialXNPVFormula();
    void testFinacialPriceMatFormula();
    void testFinacialFormulaReceived();
    void testFinancialFormulaCumipmt();
    void testFinancialFormulaCumprinc();
    void testFinacialRRIFormula();
    void testFinacialEFFECT_ADDFormula();
    void testFinacialNominalFormula();
    void testFinacialTBILLEQFormula();
    void testFinacialTBILLPRICEFormula();
    void testFinacialTBILLYIELDFormula();
    void testFinacialYIELDFormula();
    void testFinacialYIELDDISCFormula();
    void testFinacialYIELDMATFormula();
    void testFinacialPMTFormula();
    void testFinacialPPMTFormula();
    void testFinancialISPMTFormula();
    void testFinacialPriceFormula();
    void testFinancialDurationFormula();
    void testFinancialCoupnumFormula();
    void testMathFormulaSinh();
    void testMathFormulaAbs();
    void testFinacialPVFormula();
    void testMathFormulaSin();
    void testMathFormulaTan();
    void testMathFormulaTanH();
    void testStatisticalFormulaStandard();
    void testStatisticalFormulaWeibull();
    void testStatisticalFormulaMedian();
    void testFinancialDuration_ADDFormula();
    void testFinancialAmordegrcFormula();
    void testFinancialAmorlincFormula();
    void testFinancialDDBFormula();
    void testFinancialFVFormula();
    void testFinancialMDurationFormula();
    void testMathSumIfsFormula();
    void testFinancialVDBFormula();
    void testStatisticalFormulaKurt();
    void testFinacialNPERFormula();
    void testStatisticalFormulaNormdist();
    void testMathFormulaArcCos();
    void testMathFormulaSqrt();
    void testMathFormulaArcCosHyp();
    void testFinacialNPVFormula();
    void testStatisticalFormulaNormsdist();
    void testStatisticalFormulaNorminv();
    void testStatisticalFormulaNormsinv();
    void testStatisticalFormulaPermut();
    void testStatisticalFormulaPermutation();
    void testStatisticalFormulaPhi();
    void testFinancialIPMTFormula();
    void testStatisticalFormulaConfidence();
    void testStatisticalFormulaIntercept();
    void testFinacialODDLPRICEFormula();
    void testFinacialOddlyieldFormula();
    void testFinacialPriceDiscFormula();
    void testFinancialDBFormula();
    void testFinancialCouppcdFormula();
    void testFinancialCoupncdFormula();
    void testStatisticalFormulaLogInv();
    void testMathFormulaArcCot();
    void testMathFormulaCosh();
    void testStatisticalFormulaCritBinom();
    void testMathFormulaArcCotHyp();
    void testMathFormulaArcSin();
    void testMathFormulaArcSinHyp();
    void testMathFormulaArcTan();
    void testMathFormulaArcTanHyp();
    void testMathFormulaBitAnd();
    void testStatisticalFormulaForecast();
    void testStatisticalFormulaLogNormDist();
    void testStatisticalFormulaGammaDist();
    void testMathFormulaLN();
    void testMathFormulaRound();
    void testMathFormulaCot();
    void testMathFormulaCoth();
    void testFinacialNPER1Formula();
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

    CPPUNIT_TEST_SUITE(ScOpenCLTest);
    CPPUNIT_TEST(testSystematic);
    CPPUNIT_TEST(testSharedFormulaXLS);
    CPPUNIT_TEST(testFinacialFormula);
    CPPUNIT_TEST(testStatisticalFormulaFisher);
    CPPUNIT_TEST(testStatisticalFormulaFisherInv);
    CPPUNIT_TEST(testStatisticalFormulaGamma);
    CPPUNIT_TEST(testFinacialFvscheduleFormula);
// this test has intermittent failures on OSX
#if !defined MACOSX
    CPPUNIT_TEST(testFinacialIRRFormula);
#endif
    CPPUNIT_TEST(testFinacialMIRRFormula);
    CPPUNIT_TEST(testFinacialRateFormula);
    CPPUNIT_TEST(testCompilerHorizontal);
    CPPUNIT_TEST(testCompilerNested);
    CPPUNIT_TEST(testFinacialSLNFormula);
    CPPUNIT_TEST(testFinancialAccrintmFormula);
    CPPUNIT_TEST(testStatisticalFormulaGammaLn);
    CPPUNIT_TEST(testStatisticalFormulaGauss);
    CPPUNIT_TEST(testStatisticalFormulaGeoMean);
    CPPUNIT_TEST(testStatisticalFormulaHarMean);
    CPPUNIT_TEST(testFinancialCoupdaybsFormula);
    CPPUNIT_TEST(testFinacialDollardeFormula);
    CPPUNIT_TEST(testCompilerString);
    CPPUNIT_TEST(testCompilerInEq);
    CPPUNIT_TEST(testCompilerPrecision);
    CPPUNIT_TEST(testFinacialDollarfrFormula);
    CPPUNIT_TEST(testFinacialSYDFormula);
    CPPUNIT_TEST(testStatisticalFormulaCorrel);
    CPPUNIT_TEST(testFinancialCoupdaysFormula);
    CPPUNIT_TEST(testFinancialCoupdaysncFormula);
    CPPUNIT_TEST(testFinacialDISCFormula);
    CPPUNIT_TEST(testFinacialINTRATEFormula);
    CPPUNIT_TEST(testMathFormulaCos);
    CPPUNIT_TEST(testStatisticalFormulaNegbinomdist);
    CPPUNIT_TEST(testStatisticalFormulaRsq);
    CPPUNIT_TEST(testStatisticalFormulaPearson);
    CPPUNIT_TEST(testMathFormulaCsc);
    CPPUNIT_TEST(testFinacialPriceMatFormula);
    CPPUNIT_TEST(testFinacialXNPVFormula);
    CPPUNIT_TEST(testFinacialFormulaReceived);
    CPPUNIT_TEST(testFinancialFormulaCumipmt);
    CPPUNIT_TEST(testFinancialFormulaCumprinc);
    CPPUNIT_TEST(testFinacialRRIFormula);
    CPPUNIT_TEST(testFinacialEFFECT_ADDFormula);
    CPPUNIT_TEST(testFinacialNominalFormula);
    CPPUNIT_TEST(testFinacialTBILLEQFormula);
    CPPUNIT_TEST(testFinacialTBILLPRICEFormula);
    CPPUNIT_TEST(testFinacialTBILLYIELDFormula);
    CPPUNIT_TEST(testFinacialYIELDFormula);
    CPPUNIT_TEST(testFinacialYIELDDISCFormula);
    CPPUNIT_TEST(testFinacialYIELDMATFormula);
    CPPUNIT_TEST(testFinacialPPMTFormula);
    CPPUNIT_TEST(testFinacialPMTFormula);
    CPPUNIT_TEST(testFinancialISPMTFormula);
    CPPUNIT_TEST(testFinacialPriceFormula);
    CPPUNIT_TEST(testFinancialDurationFormula);
    CPPUNIT_TEST(testFinancialCoupnumFormula);
    CPPUNIT_TEST(testMathFormulaSinh);
    CPPUNIT_TEST(testMathFormulaAbs);
    CPPUNIT_TEST(testFinacialPVFormula);
    CPPUNIT_TEST(testMathFormulaSin);
    CPPUNIT_TEST(testMathFormulaTan);
    CPPUNIT_TEST(testMathFormulaTanH);
    CPPUNIT_TEST(testStatisticalFormulaStandard);
    CPPUNIT_TEST(testStatisticalFormulaWeibull);
    CPPUNIT_TEST(testStatisticalFormulaMedian);
    CPPUNIT_TEST(testFinancialDuration_ADDFormula);
    CPPUNIT_TEST(testFinancialAmordegrcFormula);
    CPPUNIT_TEST(testFinancialAmorlincFormula);
    CPPUNIT_TEST(testFinancialDDBFormula);
    CPPUNIT_TEST(testFinancialFVFormula);
    CPPUNIT_TEST(testFinancialMDurationFormula);
    CPPUNIT_TEST(testMathSumIfsFormula);
    CPPUNIT_TEST(testFinancialVDBFormula);
    CPPUNIT_TEST(testStatisticalFormulaKurt);
    CPPUNIT_TEST(testFinacialNPERFormula);
    CPPUNIT_TEST(testStatisticalFormulaNormdist);
    CPPUNIT_TEST(testMathFormulaArcCos);
    CPPUNIT_TEST(testMathFormulaSqrt);
    CPPUNIT_TEST(testMathFormulaArcCosHyp);
    CPPUNIT_TEST(testFinacialNPVFormula);
    CPPUNIT_TEST(testStatisticalFormulaNormsdist);
    CPPUNIT_TEST(testStatisticalFormulaNorminv);
    CPPUNIT_TEST(testStatisticalFormulaNormsinv);
    CPPUNIT_TEST(testStatisticalFormulaPermut);
    CPPUNIT_TEST(testStatisticalFormulaPermutation);
    CPPUNIT_TEST(testStatisticalFormulaPhi);
    CPPUNIT_TEST(testFinancialIPMTFormula);
    CPPUNIT_TEST(testStatisticalFormulaConfidence);
    CPPUNIT_TEST(testStatisticalFormulaIntercept);
    CPPUNIT_TEST(testFinacialODDLPRICEFormula);
    CPPUNIT_TEST(testFinacialOddlyieldFormula);
    CPPUNIT_TEST(testFinacialPriceDiscFormula);
    CPPUNIT_TEST(testFinancialDBFormula);
    CPPUNIT_TEST(testFinancialCouppcdFormula);
    CPPUNIT_TEST(testFinancialCoupncdFormula);
    CPPUNIT_TEST(testFinancialAccrintFormula);
    CPPUNIT_TEST(testStatisticalFormulaLogInv);
    CPPUNIT_TEST(testMathFormulaArcCot);
    CPPUNIT_TEST(testMathFormulaCosh);
    CPPUNIT_TEST(testStatisticalFormulaCritBinom);
    CPPUNIT_TEST(testMathFormulaArcCotHyp);
    CPPUNIT_TEST(testMathFormulaArcSin);
    CPPUNIT_TEST(testMathFormulaArcSinHyp);
    CPPUNIT_TEST(testMathFormulaArcTan);
    CPPUNIT_TEST(testMathFormulaArcTanHyp);
    CPPUNIT_TEST(testMathFormulaBitAnd);
    CPPUNIT_TEST(testStatisticalFormulaForecast);
    CPPUNIT_TEST(testStatisticalFormulaLogNormDist);
    CPPUNIT_TEST(testStatisticalFormulaGammaDist);
    CPPUNIT_TEST(testMathFormulaLN);
    CPPUNIT_TEST(testMathFormulaRound);
    CPPUNIT_TEST(testMathFormulaCot);
    CPPUNIT_TEST(testMathFormulaCoth);
    CPPUNIT_TEST(testFinacialNPER1Formula);
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
    // Test env variables and methods
    ScDocShellRef xDocSh;
    ScDocShellRef xDocShRes;
    void initTestEnv(std::u16string_view fileName, sal_Int32 nFormat);
};

void ScOpenCLTest::initTestEnv(std::u16string_view fileName, sal_Int32 nFormat)
{
    disableOpenCL();
    xDocSh = loadDoc(fileName, nFormat);

    enableOpenCL();
    xDocShRes = loadDoc(fileName, nFormat);
}

void ScOpenCLTest::enableOpenCL()
{
    sc::FormulaGroupInterpreter::enableOpenCL_UnitTestsOnly();
}

void ScOpenCLTest::disableOpenCL()
{
    sc::FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly();
}

void ScOpenCLTest::testCompilerHorizontal()
{
    initTestEnv(u"opencl/compiler/horizontal.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(12, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(12, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = rDoc.GetValue(ScAddress(13, i, 0));
        fExcel = rDocRes.GetValue(ScAddress(13, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = rDoc.GetValue(ScAddress(14, i, 0));
        fExcel = rDocRes.GetValue(ScAddress(14, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testCompilerNested()
{
    initTestEnv(u"opencl/compiler/nested.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testCompilerString()
{
    initTestEnv(u"opencl/compiler/string.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));

        fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testCompilerInEq()
{
    initTestEnv(u"opencl/compiler/ineq.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 7; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testCompilerPrecision()
{
    initTestEnv(u"opencl/compiler/precision.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check that values with and without opencl are the same/similar enough.
    enableOpenCL();
    rDoc.CalcAll();
    disableOpenCL();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 3; ++i)
    {
        double fOpenCL = rDoc.GetValue(ScAddress(0, i, 0));
        double fNormal = rDocRes.GetValue(ScAddress(0, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fNormal, fOpenCL, fabs(1e-14*fOpenCL));
    }
}

#if 0
void ScOpenCLTest::testSharedFormulaXLSStockHistory()
{
    initTestEnv("stock-history.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 33; i < 44; ++i)
    {   // Cell H34:H44 in S&P 500 (tab 1)
        double fLibre = rDoc.GetValue(ScAddress(7, i, 1));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.0001*fExcel);
    }

    for (SCROW i = 33; i < 44; ++i)
    {   // Cell J34:J44 in S&P 500 (tab 1)
        double fLibre = rDoc.GetValue(ScAddress(9, i, 1));
        double fExcel = rDocRes.GetValue(ScAddress(9, i, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.0001*fExcel);
    }
}

void ScOpenCLTest::testSharedFormulaXLSGroundWater()
{
    initTestEnv("ground-water-daily.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 5; i <= 77; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(11,i,1));
        double fExcel = rDocRes.GetValue(ScAddress(11,i,1));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }


}
#endif

void ScOpenCLTest::testSystematic()
{
    initTestEnv(u"systematic.", FORMAT_XLS);

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    int nAVertBegin(0), nAVertEnd(0), nBVertBegin(0), nBVertEnd(0);
    int nAHorEnd(0), nBHorEnd(0);

    int nRow, nCol;
    for (nRow = 0; nRow < 1000; ++nRow)
    {
        if (rDoc.GetString(ScAddress(0, nRow, 0)) == "a")
        {
            nAVertBegin = nRow + 1;

            for (nCol = 0; nCol < 1000; ++nCol)
            {
                if (rDoc.GetString(ScAddress(nCol, nRow, 0)) != "a")
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
        if (rDoc.GetString(ScAddress(0, nRow, 0)) != "a")
        {
            nAVertEnd = nRow;
            break;
        }
    }

    for (; nRow < 1000; ++nRow)
    {
        if (rDoc.GetString(ScAddress(0, nRow, 0)) == "b")
        {
            nBVertBegin = nRow + 1;

            for (nCol = 0; nCol < 1000; ++nCol)
            {
                if (rDoc.GetString(ScAddress(nCol, nRow, 0)) != "b")
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
        if (rDoc.GetString(ScAddress(0, nRow, 0)) != "b")
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
            double fLibre = rDoc.GetValue(ScAddress(j, i, 0));
            double fExcel = rDoc.GetValue(ScAddress(j, nBVertBegin + (i - nAVertBegin), 0));

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


void ScOpenCLTest::testSharedFormulaXLS()
{
    initTestEnv(u"sum_ex.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i < 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 6; i < 14; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 15; i < 18; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 19; i < 22; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 23; i < 25; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        //double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        // There seems to be a bug in LibreOffice beta
        ASSERT_DOUBLES_EQUAL(/*fExcel*/ 60.0, fLibre);
    }

    for (SCROW i = 25; i < 27; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 28; i < 35; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    // workaround for a Calc beta bug
    ASSERT_DOUBLES_EQUAL(25.0, rDoc.GetValue(ScAddress(2, 35, 0)));
    ASSERT_DOUBLES_EQUAL(24.0, rDoc.GetValue(ScAddress(2, 36, 0)));

    for (SCROW i = 38; i < 43; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 5; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 1));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 1));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }

    for (SCROW i = 5; i < 10; ++i)
    {
        for (SCCOL j = 6; j < 11; ++j)
        {
            double fLibre = rDoc.GetValue(ScAddress(j, i, 1));
            double fExcel = rDocRes.GetValue(ScAddress(j, i, 1));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre,
                fabs(fExcel*0.0001));
        }
    }
}

void ScOpenCLTest::testMathFormulaCos()
{
    initTestEnv(u"opencl/math/cos.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSinh()
{
    initTestEnv(u"opencl/math/sinh.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaPi()
{
    initTestEnv(u"opencl/math/pi.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(0,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(0,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaRandom()
{
    initTestEnv(u"opencl/math/random.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        rDoc.GetValue(ScAddress(0,i,0)); // LO
        rDocRes.GetValue(ScAddress(0,i,0)); // Excel
        //because the random numbers will always change,so give the test "true"
        CPPUNIT_ASSERT(true);
    }
}
void ScOpenCLTest::testFinacialFormula()
{
    initTestEnv(u"opencl/financial/general.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,1));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,2));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,3));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,3));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,4));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,4));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,5));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,5));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,6));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,6));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,7));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,7));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,8));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,8));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,9));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,9));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,10));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,10));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7,i,11));
        double fExcel = rDocRes.GetValue(ScAddress(7,i,11));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,12));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,12));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 12; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,13));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,13));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,14));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,14));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,15));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,15));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,16));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,16));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,17));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,17));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,18));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,18));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,19));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,19));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaCorrel()
{
    initTestEnv(u"opencl/statistical/Correl.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testStatisticalFormulaFisher()
{
    initTestEnv(u"opencl/statistical/Fisher.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaFisherInv()
{
    initTestEnv(u"opencl/statistical/FisherInv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaGamma()
{
    initTestEnv(u"opencl/statistical/Gamma.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialFvscheduleFormula()
{
    initTestEnv(u"opencl/financial/Fvschedule.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaAbs()
{
    initTestEnv(u"opencl/math/Abs.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ABS Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialSYDFormula()
{
    initTestEnv(u"opencl/financial/SYD.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

// this test has intermittent failures on OSX
#if !defined MACOSX
void ScOpenCLTest::testFinacialIRRFormula()
{
    initTestEnv(u"opencl/financial/IRR.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
#endif

void ScOpenCLTest::testStatisticalFormulaGammaLn()
{
    initTestEnv(u"opencl/statistical/GammaLn.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaGauss()
{
    initTestEnv(u"opencl/statistical/Gauss.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaGeoMean()
{
    initTestEnv(u"opencl/statistical/GeoMean.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaHarMean()
{
    initTestEnv(u"opencl/statistical/HarMean.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialSLNFormula()
{
    initTestEnv(u"opencl/financial/SLN.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialMIRRFormula()
{
    initTestEnv(u"opencl/financial/MIRR.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialCoupdaybsFormula()
{
    initTestEnv(u"opencl/financial/Coupdaybs.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialDollardeFormula()
{
    initTestEnv(u"opencl/financial/Dollarde.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialCoupdaysFormula()
{
    initTestEnv(u"opencl/financial/Coupdays.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

}

void ScOpenCLTest::testFinancialCoupdaysncFormula()
{
    initTestEnv(u"opencl/financial/Coupdaysnc.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialRateFormula()
{
    initTestEnv(u"opencl/financial/RATE.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialAccrintmFormula()
{
    initTestEnv(u"opencl/financial/Accrintm.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialCoupnumFormula()
{
    initTestEnv(u"opencl/financial/Coupnum.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaNegbinomdist()
{
    initTestEnv(u"opencl/statistical/Negbinomdist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSin()
{
    initTestEnv(u"opencl/math/sin.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSumSQ()
{
    initTestEnv(u"opencl/math/sumsq.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaTan()
{
    initTestEnv(u"opencl/math/tan.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaTanH()
{
    initTestEnv(u"opencl/math/tanh.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSqrt()
{
    initTestEnv(u"opencl/math/sqrt.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialPriceFormula()
{
    initTestEnv(u"opencl/financial/Price.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialDollarfrFormula()
{
    initTestEnv(u"opencl/financial/Dollarfr.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialPriceDiscFormula()
{
    initTestEnv(u"opencl/financial/PriceDisc.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialODDLPRICEFormula()
{
    initTestEnv(u"opencl/financial/Oddlprice.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(8, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinacialOddlyieldFormula()
{
    initTestEnv(u"opencl/financial/Oddlyield.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(8, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialDISCFormula()
{
    initTestEnv(u"opencl/financial/DISC.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinacialPVFormula()
{
    initTestEnv(u"opencl/financial/PV.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialINTRATEFormula()
{
    initTestEnv(u"opencl/financial/INTRATE.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStandard()
{
    initTestEnv(u"opencl/statistical/Standard.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaWeibull()
{
    initTestEnv(u"opencl/statistical/Weibull.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre,
            fExcel == 0?1e-4:fabs(1e-4*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaVar()
{
    initTestEnv(u"opencl/statistical/Var.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaSkew()
{
    initTestEnv(u"opencl/statistical/Skew.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaSkewp()
{
    initTestEnv(u"opencl/statistical/Skewp.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaPearson()
{
    initTestEnv(u"opencl/statistical/Pearson.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaRsq()
{
    initTestEnv(u"opencl/statistical/Rsq.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaTrunc()
{
    initTestEnv(u"opencl/math/trunc.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCosh()
{
    initTestEnv(u"opencl/math/cosh.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testStatisticalFormulaCovar()
{
    initTestEnv(u"opencl/statistical/Covar.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 16; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaKurt()
{
    initTestEnv(u"opencl/statistical/Kurt.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCot()
{
    initTestEnv(u"opencl/math/cot.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaDevSq()
{
    initTestEnv(u"opencl/statistical/DevSq.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCsc()
{
    initTestEnv(u"opencl/math/csc.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCoth()
{
    initTestEnv(u"opencl/math/coth.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialXNPVFormula()
{
    initTestEnv(u"opencl/financial/XNPV.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }

    for (SCROW i = 16; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaIntercept()
{
    initTestEnv(u"opencl/statistical/Intercept.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialAmordegrcFormula()
{
    initTestEnv(u"opencl/financial/Amordegrc.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialISPMTFormula()
{
    initTestEnv(u"opencl/financial/ISPMT.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaMedian()
{
    initTestEnv(u"opencl/statistical/Median.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaNormdist()
{
    initTestEnv(u"opencl/statistical/Normdist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaNormsdist()
{
    initTestEnv(u"opencl/statistical/Normsdist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaPermut()
{
    initTestEnv(u"opencl/statistical/Permut.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaPermutation()
{
    initTestEnv(u"opencl/statistical/Permutation.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaPhi()
{
    initTestEnv(u"opencl/statistical/Phi.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCscH()
{
    initTestEnv(u"opencl/math/csch.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaLogInv()
{
    initTestEnv(u"opencl/statistical/LogInv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialNPERFormula()
{
    initTestEnv(u"opencl/financial/NPER.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaForecast()
{
    initTestEnv(u"opencl/statistical/Forecast.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialAmorlincFormula()
{
    initTestEnv(u"opencl/financial/Amorlinc.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialDDBFormula()
{
    initTestEnv(u"opencl/financial/ddb.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialPriceMatFormula()
{
    initTestEnv(u"opencl/financial/PriceMat.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialFormulaReceived()
{
    initTestEnv(u"opencl/financial/Received.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinancialFormulaCumipmt()
{
    initTestEnv(u"opencl/financial/Cumipmt.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinancialFormulaCumprinc()
{
    initTestEnv(u"opencl/financial/Cumprinc.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialRRIFormula()
{
    initTestEnv(u"opencl/financial/RRI.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialEFFECT_ADDFormula()
{
    initTestEnv(u"opencl/financial/EFFECT_ADD.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialNominalFormula()
{
    initTestEnv(u"opencl/financial/Nominal.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();


    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialTBILLEQFormula()
{
    initTestEnv(u"opencl/financial/TBILLEQ.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialTBILLPRICEFormula()
{
    initTestEnv(u"opencl/financial/TBILLPRICE.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialTBILLYIELDFormula()
{
    initTestEnv(u"opencl/financial/TBILLYIELD.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest::testFinacialYIELDFormula()
{
    initTestEnv(u"opencl/financial/YIELD.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialYIELDDISCFormula()
{
    initTestEnv(u"opencl/financial/YIELDDISC.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinacialYIELDMATFormula()
{
    initTestEnv(u"opencl/financial/YIELDMAT.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
void ScOpenCLTest:: testFinacialPMTFormula()
{
    initTestEnv(u"opencl/financial/PMT.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialDurationFormula()
{
    initTestEnv(u"opencl/financial/Duration.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaLogNormDist()
{
    initTestEnv(u"opencl/statistical/LogNormDist.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcCos()
{
    initTestEnv(u"opencl/math/ArcCos.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ACos Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaPower()
{
    initTestEnv(u"opencl/math/power.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinacialPPMTFormula()
{
    initTestEnv(u"opencl/financial/PPMT.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinacialNPVFormula()
{
    initTestEnv(u"opencl/financial/NPV.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialDuration_ADDFormula()
{
    initTestEnv(u"opencl/financial/Duration_ADD.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaNorminv()
{
    initTestEnv(u"opencl/statistical/Norminv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaNormsinv()
{
    initTestEnv(u"opencl/statistical/Normsinv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcCosHyp()
{
    initTestEnv(u"opencl/math/ArcCosHyp.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ACosH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialMDurationFormula()
{
    initTestEnv(u"opencl/financial/MDuration.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcCot()
{
    initTestEnv(u"opencl/math/ArcCot.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ACot Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialFVFormula()
{
    initTestEnv(u"opencl/financial/FV.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialDBFormula()
{
    initTestEnv(u"opencl/financial/db.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialCouppcdFormula()
{
    initTestEnv(u"opencl/financial/Couppcd.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathSumIfsFormula()
{
    initTestEnv(u"opencl/math/sumifs.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc();

    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(7,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 2; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(8,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(8,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcCotHyp()
{
    initTestEnv(u"opencl/math/ArcCotHyp.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcSin()
{
    initTestEnv(u"opencl/math/ArcSin.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialVDBFormula()
{
    initTestEnv(u"opencl/financial/VDB.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 15; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 30; i <= 41; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialIPMTFormula()
{
    initTestEnv(u"opencl/financial/IPMT.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaChiSqDist()
{
    initTestEnv(u"opencl/statistical/CHISQDIST.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaConfidence()
{
    initTestEnv(u"opencl/statistical/Confidence.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaFDist()
{
    initTestEnv(u"opencl/statistical/Fdist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialCoupncdFormula()
{
    initTestEnv(u"opencl/financial/Coupncd.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testFinancialAccrintFormula()
{
    initTestEnv(u"opencl/financial/Accrint.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaCritBinom()
{
    initTestEnv(u"opencl/statistical/CritBinom.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcSinHyp()
{
    initTestEnv(u"opencl/math/ArcSinHyp.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ASinH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcTan()
{
    initTestEnv(u"opencl/math/ArcTan.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ATan Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcTanHyp()
{
    initTestEnv(u"opencl/math/ArcTanHyp.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ATanH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinacialNPER1Formula()
{
    initTestEnv(u"opencl/financial/NPER1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaArcTan2()
{
    initTestEnv(u"opencl/math/ArcTan2.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify ATan2 Function
    for (SCROW i = 1; i <= 17; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.000001);
    }
}

void ScOpenCLTest::testStatisticalFormulaChiSqInv()
{
    initTestEnv(u"opencl/statistical/CHISQINV.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaBitAnd()
{
    initTestEnv(u"opencl/math/BitAnd.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify BitAnd Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaPoisson()
{
    initTestEnv(u"opencl/statistical/Poisson.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaExpondist()
{
    initTestEnv(u"opencl/statistical/Expondist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaBitOr()
{
    initTestEnv(u"opencl/math/BitOr.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify BitOr Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaOdd()
{
    initTestEnv(u"opencl/math/odd.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaLN()
{
    initTestEnv(u"opencl/math/LN.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaMod()
{
    initTestEnv(u"opencl/math/mod.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        if(fExcel == 0.0f)
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 1e-10);
        else
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaRound()
{
    initTestEnv(u"opencl/math/ROUND.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 15; i <= 25; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaGammaDist()
{
    initTestEnv(u"opencl/statistical/GammaDist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaGammaInv()
{
    initTestEnv(u"opencl/statistical/GammaInv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = rDoc.GetValue(ScAddress(4,i,0));
        fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaFInv()
{
    initTestEnv(u"opencl/statistical/FInv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = rDoc.GetValue(ScAddress(4,i,0));
        fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaFTest()
{
    initTestEnv(u"opencl/statistical/FTest.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaB()
{
    initTestEnv(u"opencl/statistical/B.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = rDoc.GetValue(ScAddress(5,i,0));
        fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaBetaDist()
{
    initTestEnv(u"opencl/statistical/BetaDist.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        fLibre = rDoc.GetValue(ScAddress(7,i,0));
        fExcel = rDocRes.GetValue(ScAddress(7,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaEven()
{
    initTestEnv(u"opencl/math/even.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaExp()
{
    initTestEnv(u"opencl/math/exp.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaChiDist()
{
    initTestEnv(u"opencl/statistical/ChiDist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaBitLshift()
{
    initTestEnv(u"opencl/math/BitLshift.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify BitLshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaBitRshift()
{
    initTestEnv(u"opencl/math/BitRshift.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify BitRshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaFloor()
{
    initTestEnv(u"opencl/math/floor.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaLog()
{
    initTestEnv(u"opencl/math/log.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testSpreadSheetFormulaVLookup()
{
    initTestEnv(u"opencl/spreadsheet/VLookup.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 40; i <= 50; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaChiInv()
{
    initTestEnv(u"opencl/statistical/ChiInv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaConvert()
{
    initTestEnv(u"opencl/math/convert.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathCountIfsFormula()
{
    initTestEnv(u"opencl/math/countifs.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc();

    for (SCROW i = 1; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaBitXor()
{
    initTestEnv(u"opencl/math/BitXor.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify BitXor Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathAverageIfsFormula()
{
    initTestEnv(u"opencl/math/averageifs.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();

    xDocSh->DoHardRecalc();

    for (SCROW i = 1; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaLog10()
{
    initTestEnv(u"opencl/math/log10.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCombina()
{
    initTestEnv(u"opencl/math/combina.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCeil()
{
    initTestEnv(u"opencl/math/Ceil.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify Ceiling Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSqrtPi()
{
    initTestEnv(u"opencl/math/sqrtpi.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaVarP()
{
    initTestEnv(u"opencl/statistical/VarP.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStDev()
{
    initTestEnv(u"opencl/statistical/StDev.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStDevP()
{
    initTestEnv(u"opencl/statistical/StDevP.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaSlope()
{
    initTestEnv(u"opencl/statistical/Slope.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaSTEYX()
{
    initTestEnv(u"opencl/statistical/STEYX.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaZTest()
{
    initTestEnv(u"opencl/statistical/ZTest.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaTTest()
{
    initTestEnv(u"opencl/statistical/TTest.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaTDist()
{
    initTestEnv(u"opencl/statistical/TDist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaTInv()
{
    initTestEnv(u"opencl/statistical/TInv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaBinomDist()
{
    initTestEnv(u"opencl/statistical/BinomDist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaProduct()
{
    initTestEnv(u"opencl/math/product.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

#if 0 //Disabled temporarily
void ScOpenCLTest::testMathFormulaKombin()
{
    initTestEnv("opencl/math/Kombin.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify Combin Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}
#endif

void ScOpenCLTest:: testArrayFormulaSumX2MY2()
{
    initTestEnv(u"opencl/array/SUMX2MY2.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 20; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaHypGeomDist()
{
    initTestEnv(u"opencl/statistical/HypGeomDist.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testArrayFormulaSumX2PY2()
{
    initTestEnv(u"opencl/array/SUMX2PY2.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 20; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaBetainv()
{
    initTestEnv(u"opencl/statistical/Betainv.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaMina()
{
    initTestEnv(u"opencl/statistical/Mina.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testArrayFormulaSumXMY2()
{
    initTestEnv(u"opencl/array/SUMXMY2.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 20; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaCountA()
{
    initTestEnv(u"opencl/statistical/counta.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaMaxa()
{
    initTestEnv(u"opencl/statistical/Maxa.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSumProduct()
{
    initTestEnv(u"opencl/math/sumproduct_mixSliding.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
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

void ScOpenCLTest::testMathFormulaAverageIf()
{
    initTestEnv(u"opencl/math/averageif.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 2; i <= 21; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaAverageA()
{
    initTestEnv(u"opencl/statistical/AverageA.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testLogicalFormulaAnd()
{
    initTestEnv(u"opencl/logical/and.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaVarA()
{
    initTestEnv(u"opencl/statistical/VarA.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaVarPA()
{
    initTestEnv(u"opencl/statistical/VarPA.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStDevA()
{
    initTestEnv(u"opencl/statistical/StDevA.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStDevPA()
{
    initTestEnv(u"opencl/statistical/StDevPA.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testFinancialMDurationFormula1()
{
    initTestEnv(u"opencl/financial/MDuration1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,fLibre,fabs(0.00000000001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSumProduct2()
{
    initTestEnv(u"opencl/math/sumproductTest.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 2; i <= 12; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,1));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,  fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testStatisticalParallelCountBug()
{
    initTestEnv(u"opencl/statistical/parallel_count_bug_243.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i < 13; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testLogicalFormulaOr()
{
    initTestEnv(u"opencl/logical/or.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testLogicalFormulaNot()
{
    initTestEnv(u"opencl/logical/not.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest:: testLogicalFormulaXor()
{
    initTestEnv(u"opencl/logical/xor.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testDatabaseFormulaDcount()
{
    initTestEnv(u"opencl/database/dcount.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDcountA()
{
    initTestEnv(u"opencl/database/dcountA.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDmax()
{
    initTestEnv(u"opencl/database/dmax.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDmin()
{
    initTestEnv(u"opencl/database/dmin.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDproduct()
{
    initTestEnv(u"opencl/database/dproduct.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDaverage()
{
    initTestEnv(u"opencl/database/daverage.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDstdev()
{
    initTestEnv(u"opencl/database/dstdev.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDstdevp()
{
    initTestEnv(u"opencl/database/dstdevp.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDsum()
{
    initTestEnv(u"opencl/database/dsum.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDvar()
{
    initTestEnv(u"opencl/database/dvar.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testDatabaseFormulaDvarp()
{
    initTestEnv(u"opencl/database/dvarp.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
}

void ScOpenCLTest::testMathFormulaRoundUp()
{
    initTestEnv(u"opencl/math/roundup.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaRoundDown()
{
    initTestEnv(u"opencl/math/rounddown.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaInt()
{
    initTestEnv(u"opencl/math/int.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaRadians()
{
    initTestEnv(u"opencl/math/radians.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaDegrees()
{
    initTestEnv(u"opencl/math/degrees.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 200; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaIsEven()
{
    initTestEnv(u"opencl/math/iseven.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaCountIf()
{
    initTestEnv(u"opencl/math/countif.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaIsOdd()
{
    initTestEnv(u"opencl/math/isodd.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaFact()
{
    initTestEnv(u"opencl/math/fact.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSEC()
{
    initTestEnv(u"opencl/math/sec.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSECH()
{
    initTestEnv(u"opencl/math/sech.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaMROUND()
{
    initTestEnv(u"opencl/math/MROUND.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 13; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaQuotient()
{
    initTestEnv(u"opencl/math/Quotient.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Verify BitAnd Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSeriesSum()
{
    initTestEnv(u"opencl/math/seriessum.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaSumIf()
{
    initTestEnv(u"opencl/math/sumif.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testAddInFormulaBesseLJ()
{
    initTestEnv(u"opencl/addin/besselj.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaAvedev()
{
    initTestEnv(u"opencl/statistical/Avedev.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testNegSub()
{
    initTestEnv(u"opencl/math/NegSub.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaAverageIf_Mix()
{
    initTestEnv(u"opencl/math/averageif_mix.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaKurt1()
{
    initTestEnv(u"opencl/statistical/Kurt1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaHarMean1()
{
    initTestEnv(u"opencl/statistical/HarMean1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaVarA1()
{
    initTestEnv(u"opencl/statistical/VarA1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaVarPA1()
{
    initTestEnv(u"opencl/statistical/VarPA1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStDevA1()
{
    initTestEnv(u"opencl/statistical/StDevA1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testStatisticalFormulaStDevPA1()
{
    initTestEnv(u"opencl/statistical/StDevPA1.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

ScOpenCLTest::ScOpenCLTest()
      : ScBootstrapFixture( "sc/qa/unit/data" )
{
}

void ScOpenCLTest::tearDown()
{
    //close test env
    if(xDocSh.is())
    {
        xDocSh->DoClose();
        xDocSh.clear();
    }
    if(xDocShRes.is())
    {
        xDocShRes->DoClose();
        xDocShRes.clear();
    }

    ScBootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenCLTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
