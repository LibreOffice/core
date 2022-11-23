/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <sal/config.h>

#include <string_view>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <document.hxx>
#include <formulagroup.hxx>

#include <com/sun/star/document/MacroExecMode.hpp>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScOpenCLTest
    : public ScModelTestBase
{
public:
    ScOpenCLTest();

    /**
     * Turn on OpenCL group interpreter. Call this after the document is
     * loaded and before performing formula calculation.
     */
    void enableOpenCL();
    void disableOpenCL();

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
    void initTestEnv(std::u16string_view fileName);

    ScDocument* getScDoc2();
};

void ScOpenCLTest::initTestEnv(std::u16string_view fileName)
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

ScDocument* ScOpenCLTest::getScDoc2()
{
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent2.get());
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj->GetDocument();
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

void ScOpenCLTest::testCompilerNested()
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

void ScOpenCLTest::testCompilerString()
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

void ScOpenCLTest::testCompilerInEq()
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

void ScOpenCLTest::testCompilerPrecision()
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

#if 0
void ScOpenCLTest::testSharedFormulaXLSStockHistory()
{
    initTestEnv(u"xls/stock-history.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 33; i < 44; ++i)
    {   // Cell H34:H44 in S&P 500 (tab 1)
        double fLibre = pDoc->GetValue(ScAddress(7, i, 1));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.0001*fExcel);
    }

    for (SCROW i = 33; i < 44; ++i)
    {   // Cell J34:J44 in S&P 500 (tab 1)
        double fLibre = pDoc->GetValue(ScAddress(9, i, 1));
        double fExcel = pDocRes->GetValue(ScAddress(9, i, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.0001*fExcel);
    }
}

void ScOpenCLTest::testSharedFormulaXLSGroundWater()
{
    initTestEnv(u"xls/ground-water-daily.xls");
    ScDocument* pDoc = getScDoc();
    ScDocument* pDocRes = getScDoc2();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 5; i <= 77; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(11,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(11,i,1));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }


}
#endif

void ScOpenCLTest::testSystematic()
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


void ScOpenCLTest::testSharedFormulaXLS()
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

void ScOpenCLTest::testMathFormulaCos()
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

void ScOpenCLTest::testMathFormulaSinh()
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

void ScOpenCLTest::testMathFormulaPi()
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

void ScOpenCLTest::testMathFormulaRandom()
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
void ScOpenCLTest::testFinacialFormula()
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

void ScOpenCLTest::testStatisticalFormulaCorrel()
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
void ScOpenCLTest::testStatisticalFormulaFisher()
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

void ScOpenCLTest::testStatisticalFormulaFisherInv()
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

void ScOpenCLTest::testStatisticalFormulaGamma()
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

void ScOpenCLTest::testFinacialFvscheduleFormula()
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

void ScOpenCLTest::testMathFormulaAbs()
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

void ScOpenCLTest::testFinacialSYDFormula()
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
void ScOpenCLTest::testFinacialIRRFormula()
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

void ScOpenCLTest::testStatisticalFormulaGammaLn()
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

void ScOpenCLTest::testStatisticalFormulaGauss()
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

void ScOpenCLTest::testStatisticalFormulaGeoMean()
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

void ScOpenCLTest::testStatisticalFormulaHarMean()
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

void ScOpenCLTest::testFinacialSLNFormula()
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

void ScOpenCLTest::testFinacialMIRRFormula()
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

void ScOpenCLTest::testFinancialCoupdaybsFormula()
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

void ScOpenCLTest::testFinacialDollardeFormula()
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

void ScOpenCLTest::testFinancialCoupdaysFormula()
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

void ScOpenCLTest::testFinancialCoupdaysncFormula()
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
void ScOpenCLTest::testFinacialRateFormula()
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

void ScOpenCLTest::testFinancialAccrintmFormula()
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

void ScOpenCLTest::testFinancialCoupnumFormula()
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

void ScOpenCLTest::testStatisticalFormulaNegbinomdist()
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

void ScOpenCLTest::testMathFormulaSin()
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

void ScOpenCLTest::testMathFormulaSumSQ()
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

void ScOpenCLTest::testMathFormulaTan()
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

void ScOpenCLTest::testMathFormulaTanH()
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

void ScOpenCLTest::testMathFormulaSqrt()
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

void ScOpenCLTest::testFinacialPriceFormula()
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

void ScOpenCLTest::testFinacialDollarfrFormula()
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

void ScOpenCLTest::testFinacialPriceDiscFormula()
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

void ScOpenCLTest::testFinacialODDLPRICEFormula()
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

void ScOpenCLTest:: testFinacialOddlyieldFormula()
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

void ScOpenCLTest::testFinacialDISCFormula()
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

void ScOpenCLTest:: testFinacialPVFormula()
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

void ScOpenCLTest::testFinacialINTRATEFormula()
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

void ScOpenCLTest::testStatisticalFormulaStandard()
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

void ScOpenCLTest::testStatisticalFormulaWeibull()
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

void ScOpenCLTest::testStatisticalFormulaVar()
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

void ScOpenCLTest::testStatisticalFormulaSkew()
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

void ScOpenCLTest::testStatisticalFormulaSkewp()
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

void ScOpenCLTest::testStatisticalFormulaPearson()
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

void ScOpenCLTest::testStatisticalFormulaRsq()
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

void ScOpenCLTest::testMathFormulaTrunc()
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

void ScOpenCLTest::testMathFormulaCosh()
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
void ScOpenCLTest::testStatisticalFormulaCovar()
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

void ScOpenCLTest::testStatisticalFormulaKurt()
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

void ScOpenCLTest::testMathFormulaCot()
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

void ScOpenCLTest::testStatisticalFormulaDevSq()
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

void ScOpenCLTest::testMathFormulaCsc()
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

void ScOpenCLTest::testMathFormulaCoth()
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

void ScOpenCLTest::testFinacialXNPVFormula()
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

void ScOpenCLTest::testStatisticalFormulaIntercept()
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

void ScOpenCLTest::testFinancialAmordegrcFormula()
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

void ScOpenCLTest:: testFinancialISPMTFormula()
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

void ScOpenCLTest::testStatisticalFormulaMedian()
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

void ScOpenCLTest::testStatisticalFormulaNormdist()
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

void ScOpenCLTest::testStatisticalFormulaNormsdist()
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

void ScOpenCLTest::testStatisticalFormulaPermut()
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

void ScOpenCLTest::testStatisticalFormulaPermutation()
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

void ScOpenCLTest::testStatisticalFormulaPhi()
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

void ScOpenCLTest::testMathFormulaCscH()
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

void ScOpenCLTest::testStatisticalFormulaLogInv()
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

void ScOpenCLTest::testFinacialNPERFormula()
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

void ScOpenCLTest::testStatisticalFormulaForecast()
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

void ScOpenCLTest::testFinancialAmorlincFormula()
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

void ScOpenCLTest::testFinancialDDBFormula()
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
void ScOpenCLTest::testFinacialPriceMatFormula()
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
void ScOpenCLTest::testFinacialFormulaReceived()
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
void ScOpenCLTest::testFinancialFormulaCumipmt()
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
void ScOpenCLTest::testFinancialFormulaCumprinc()
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
void ScOpenCLTest::testFinacialRRIFormula()
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
void ScOpenCLTest::testFinacialEFFECT_ADDFormula()
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
void ScOpenCLTest::testFinacialNominalFormula()
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
void ScOpenCLTest::testFinacialTBILLEQFormula()
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
void ScOpenCLTest::testFinacialTBILLPRICEFormula()
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
void ScOpenCLTest::testFinacialTBILLYIELDFormula()
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
void ScOpenCLTest::testFinacialYIELDFormula()
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

void ScOpenCLTest::testFinacialYIELDDISCFormula()
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

void ScOpenCLTest::testFinacialYIELDMATFormula()
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
void ScOpenCLTest:: testFinacialPMTFormula()
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

void ScOpenCLTest:: testFinancialDurationFormula()
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

void ScOpenCLTest::testStatisticalFormulaLogNormDist()
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

void ScOpenCLTest::testMathFormulaArcCos()
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

void ScOpenCLTest::testMathFormulaPower()
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

void ScOpenCLTest:: testFinacialPPMTFormula()
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

void ScOpenCLTest:: testFinacialNPVFormula()
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

void ScOpenCLTest:: testFinancialDuration_ADDFormula()
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

void ScOpenCLTest::testStatisticalFormulaNorminv()
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

void ScOpenCLTest::testStatisticalFormulaNormsinv()
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

void ScOpenCLTest::testMathFormulaArcCosHyp()
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

void ScOpenCLTest:: testFinancialMDurationFormula()
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

void ScOpenCLTest::testMathFormulaArcCot()
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

void ScOpenCLTest:: testFinancialFVFormula()
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

void ScOpenCLTest::testFinancialDBFormula()
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

void ScOpenCLTest::testFinancialCouppcdFormula()
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

void ScOpenCLTest::testMathSumIfsFormula()
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

void ScOpenCLTest::testMathFormulaArcCotHyp()
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

void ScOpenCLTest::testMathFormulaArcSin()
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

void ScOpenCLTest:: testFinancialVDBFormula()
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

void ScOpenCLTest:: testFinancialIPMTFormula()
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

void ScOpenCLTest::testStatisticalFormulaChiSqDist()
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

void ScOpenCLTest::testStatisticalFormulaConfidence()
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

void ScOpenCLTest::testStatisticalFormulaFDist()
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

void ScOpenCLTest::testFinancialCoupncdFormula()
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

void ScOpenCLTest::testFinancialAccrintFormula()
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

void ScOpenCLTest::testStatisticalFormulaCritBinom()
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

void ScOpenCLTest::testMathFormulaArcSinHyp()
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

void ScOpenCLTest::testMathFormulaArcTan()
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

void ScOpenCLTest::testMathFormulaArcTanHyp()
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

void ScOpenCLTest:: testFinacialNPER1Formula()
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

void ScOpenCLTest::testMathFormulaArcTan2()
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

void ScOpenCLTest::testStatisticalFormulaChiSqInv()
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

void ScOpenCLTest::testMathFormulaBitAnd()
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

void ScOpenCLTest::testStatisticalFormulaPoisson()
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

void ScOpenCLTest::testStatisticalFormulaExpondist()
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

void ScOpenCLTest::testMathFormulaBitOr()
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

void ScOpenCLTest::testMathFormulaOdd()
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

void ScOpenCLTest::testMathFormulaLN()
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

void ScOpenCLTest::testMathFormulaMod()
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

void ScOpenCLTest::testMathFormulaRound()
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

void ScOpenCLTest::testStatisticalFormulaGammaDist()
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

void ScOpenCLTest::testStatisticalFormulaGammaInv()
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

void ScOpenCLTest::testStatisticalFormulaFInv()
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

void ScOpenCLTest::testStatisticalFormulaFTest()
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

void ScOpenCLTest::testStatisticalFormulaB()
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

void ScOpenCLTest::testStatisticalFormulaBetaDist()
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

void ScOpenCLTest::testMathFormulaEven()
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

void ScOpenCLTest::testMathFormulaExp()
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

void ScOpenCLTest::testStatisticalFormulaChiDist()
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

void ScOpenCLTest::testMathFormulaBitLshift()
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

void ScOpenCLTest::testMathFormulaBitRshift()
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

void ScOpenCLTest::testMathFormulaFloor()
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

void ScOpenCLTest::testMathFormulaLog()
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

void ScOpenCLTest::testSpreadSheetFormulaVLookup()
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

void ScOpenCLTest::testStatisticalFormulaChiInv()
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

void ScOpenCLTest::testMathFormulaConvert()
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

void ScOpenCLTest::testMathCountIfsFormula()
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

void ScOpenCLTest::testMathFormulaBitXor()
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

void ScOpenCLTest::testMathAverageIfsFormula()
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

void ScOpenCLTest::testMathFormulaLog10()
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

void ScOpenCLTest::testMathFormulaCombina()
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

void ScOpenCLTest::testMathFormulaCeil()
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

void ScOpenCLTest::testMathFormulaSqrtPi()
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

void ScOpenCLTest::testStatisticalFormulaVarP()
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

void ScOpenCLTest::testStatisticalFormulaStDev()
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

void ScOpenCLTest::testStatisticalFormulaStDevP()
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

void ScOpenCLTest::testStatisticalFormulaSlope()
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

void ScOpenCLTest::testStatisticalFormulaSTEYX()
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

void ScOpenCLTest::testStatisticalFormulaZTest()
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

void ScOpenCLTest::testStatisticalFormulaTTest()
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

void ScOpenCLTest::testStatisticalFormulaTDist()
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

void ScOpenCLTest::testStatisticalFormulaTInv()
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

void ScOpenCLTest::testStatisticalFormulaBinomDist()
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

void ScOpenCLTest::testMathFormulaProduct()
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
void ScOpenCLTest::testMathFormulaKombin()
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

void ScOpenCLTest:: testArrayFormulaSumX2MY2()
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

void ScOpenCLTest::testStatisticalFormulaHypGeomDist()
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

void ScOpenCLTest:: testArrayFormulaSumX2PY2()
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

void ScOpenCLTest::testStatisticalFormulaBetainv()
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

void ScOpenCLTest::testStatisticalFormulaMina()
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

void ScOpenCLTest:: testArrayFormulaSumXMY2()
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

void ScOpenCLTest::testStatisticalFormulaCountA()
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

void ScOpenCLTest::testStatisticalFormulaMaxa()
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

void ScOpenCLTest::testMathFormulaSumProduct()
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

void ScOpenCLTest::testMathFormulaAverageIf()
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

void ScOpenCLTest::testStatisticalFormulaAverageA()
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

void ScOpenCLTest:: testLogicalFormulaAnd()
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

void ScOpenCLTest::testStatisticalFormulaVarA()
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

void ScOpenCLTest::testStatisticalFormulaVarPA()
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

void ScOpenCLTest::testStatisticalFormulaStDevA()
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

void ScOpenCLTest::testStatisticalFormulaStDevPA()
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

void ScOpenCLTest:: testFinancialMDurationFormula1()
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

void ScOpenCLTest::testMathFormulaSumProduct2()
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

void ScOpenCLTest:: testStatisticalParallelCountBug()
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

void ScOpenCLTest:: testLogicalFormulaOr()
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

void ScOpenCLTest:: testLogicalFormulaNot()
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

void ScOpenCLTest:: testLogicalFormulaXor()
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

void ScOpenCLTest::testDatabaseFormulaDcount()
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

void ScOpenCLTest::testDatabaseFormulaDcountA()
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

void ScOpenCLTest::testDatabaseFormulaDmax()
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

void ScOpenCLTest::testDatabaseFormulaDmin()
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

void ScOpenCLTest::testDatabaseFormulaDproduct()
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

void ScOpenCLTest::testDatabaseFormulaDaverage()
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

void ScOpenCLTest::testDatabaseFormulaDstdev()
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

void ScOpenCLTest::testDatabaseFormulaDstdevp()
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

void ScOpenCLTest::testDatabaseFormulaDsum()
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

void ScOpenCLTest::testDatabaseFormulaDvar()
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

void ScOpenCLTest::testDatabaseFormulaDvarp()
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

void ScOpenCLTest::testMathFormulaRoundUp()
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

void ScOpenCLTest::testMathFormulaRoundDown()
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

void ScOpenCLTest::testMathFormulaInt()
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

void ScOpenCLTest::testMathFormulaRadians()
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

void ScOpenCLTest::testMathFormulaDegrees()
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

void ScOpenCLTest::testMathFormulaIsEven()
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

void ScOpenCLTest::testMathFormulaCountIf()
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

void ScOpenCLTest::testMathFormulaIsOdd()
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

void ScOpenCLTest::testMathFormulaFact()
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

void ScOpenCLTest::testMathFormulaSEC()
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

void ScOpenCLTest::testMathFormulaSECH()
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

void ScOpenCLTest::testMathFormulaMROUND()
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

void ScOpenCLTest::testMathFormulaQuotient()
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

void ScOpenCLTest::testMathFormulaSeriesSum()
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

void ScOpenCLTest::testMathFormulaSumIf()
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

void ScOpenCLTest::testAddInFormulaBesseLJ()
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

void ScOpenCLTest::testStatisticalFormulaAvedev()
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

void ScOpenCLTest::testNegSub()
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

void ScOpenCLTest::testMathFormulaAverageIf_Mix()
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

void ScOpenCLTest::testStatisticalFormulaKurt1()
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

void ScOpenCLTest::testStatisticalFormulaHarMean1()
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

void ScOpenCLTest::testStatisticalFormulaVarA1()
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

void ScOpenCLTest::testStatisticalFormulaVarPA1()
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

void ScOpenCLTest::testStatisticalFormulaStDevA1()
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

void ScOpenCLTest::testStatisticalFormulaStDevPA1()
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

ScOpenCLTest::ScOpenCLTest()
      : ScModelTestBase( "sc/qa/unit/data" )
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenCLTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
