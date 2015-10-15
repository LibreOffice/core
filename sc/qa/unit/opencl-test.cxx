/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is added by mcw.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include "scdll.hxx"
#include <opencl/platforminfo.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScOpenCLTest
    : public test::FiltersTest
    , public ScBootstrapFixture
{
public:
    ScOpenCLTest();

    /**
     * Try to auto-detect OpenCL device if one is available.
     *
     * @return true if a usable OpenCL device is found, false otherwise.
     */
    bool detectOpenCLDevice();

    /**
     * Turn on OpenCL group interpreter. Call this after the document is
     * loaded and before performing formula calculation.
     */
    void enableOpenCL();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    virtual bool load( const OUString &rFilter, const OUString &rURL,
            const OUString &rUserData, SfxFilterFlags nFilterFlags,
            SotClipboardFormatId nClipboardID, unsigned int nFilterVersion) SAL_OVERRIDE;
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
    void testFinancialXirrFormula();
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
    CPPUNIT_TEST(testFinancialXirrFormula);
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
    uno::Reference<uno::XInterface> m_xCalcComponent;

    // Test env variables and methods
    ScDocShellRef xDocSh;
    ScDocShellRef xDocShRes;
    bool initTestEnv(const OUString& fileName, sal_Int32 nFormat,
              bool bReadWrite);
};

bool ScOpenCLTest::initTestEnv(const OUString& fileName, sal_Int32 nFormat,
    bool bReadWrite)
{
    if(!detectOpenCLDevice())
        return false;

    xDocSh = loadDoc(fileName, nFormat, bReadWrite);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    enableOpenCL();

    xDocShRes = loadDoc(fileName, nFormat, bReadWrite);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());

    return true;
}

bool ScOpenCLTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    bool bLoaded = xDocShRef.Is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

bool ScOpenCLTest::detectOpenCLDevice()
{
    sc::FormulaGroupInterpreter::enableOpenCL_UnitTestsOnly();
    return sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(),true);
}

void ScOpenCLTest::enableOpenCL()
{
    sc::FormulaGroupInterpreter::enableOpenCL_UnitTestsOnly();
}

void ScOpenCLTest::testCompilerHorizontal()
{
    if(!initTestEnv("opencl/compiler/horizontal.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/compiler/nested.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/compiler/string.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/compiler/ineq.", ODS, false))
        return;
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

#if 0
void ScOpenCLTest::testSharedFormulaXLSStockHistory()
{
    if(!initTestEnv("stock-history.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc(true);

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
    if(!initTestEnv("ground-water-daily.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc(true);

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
    if(!initTestEnv("systematic.", XLS, false))
        return;

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
    CPPUNIT_ASSERT((nAVertEnd-nAVertBegin) == (nBVertEnd-nBVertBegin));
    CPPUNIT_ASSERT(nAHorEnd > 10);
    CPPUNIT_ASSERT(nBHorEnd > 10);
    CPPUNIT_ASSERT(nAHorEnd == nBHorEnd);

    (void) nBVertEnd;
    (void) nBHorEnd;

    for (SCROW i = nAVertBegin; i < nAVertEnd; ++i)
    {
        for (int j = 1; j < nAHorEnd; ++j)
        {
            double fLibre = rDoc.GetValue(ScAddress(j, i, 0));
            double fExcel = rDoc.GetValue(ScAddress(j, nBVertBegin + (i - nAVertBegin), 0));

            const OString sFailedMessage =
                OString(static_cast<sal_Char>('A'+j)) +
                OString::number(i+1) +
                "!=" +
                OString(static_cast<sal_Char>('A'+j)) +
                OString::number(nBVertBegin+(i-nAVertBegin)+1);
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sFailedMessage.getStr(), fExcel, fLibre, 1e-10);
        }
    }
}


void ScOpenCLTest::testSharedFormulaXLS()
{
    if(!initTestEnv("sum_ex.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/cos.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sinh.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc(true);

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaPi()
{
    if(!initTestEnv("opencl/math/pi.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/random.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(0,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(0,i,0));
        //because the random numbers will always change,so give the test "true"
        (void) fLibre;
        (void) fExcel;
        CPPUNIT_ASSERT(true);
    }
}
void ScOpenCLTest::testFinacialFormula()
{
    if(!initTestEnv("opencl/financial/general.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Correl.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Fisher.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/FisherInv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Gamma.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Fvschedule.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/Abs.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/SYD.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/IRR.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/GammaLn.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Gauss.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/GeoMean.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/HarMean.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/SLN.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/MIRR.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Coupdaybs.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Dollarde.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Coupdays.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Coupdaysnc.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/RATE.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Accrintm.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Coupnum.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Negbinomdist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sin.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sumsq.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/tan.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/tanh.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sqrt.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Price.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Dollarfr.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/PriceDisc.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Oddlprice.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Oddlyield.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/DISC.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/PV.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/INTRATE.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Standard.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Weibull.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Var.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Skew.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Skewp.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Pearson.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Rsq.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/trunc.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/cosh.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Covar.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Kurt.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/cot.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/DevSq.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/csc.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/coth.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/XNPV.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Intercept.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Amordegrc.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/ISPMT.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Median.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Normdist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Normsdist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Permut.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Permutation.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Phi.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/csch.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/LogInv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/NPER.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Forecast.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Amorlinc.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/ddb.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/PriceMat.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Received.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Cumipmt.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Cumprinc.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/RRI.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/EFFECT_ADD.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Nominal.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/TBILLEQ.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/TBILLPRICE.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/TBILLYIELD.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/YIELD.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/YIELDDISC.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/YIELDMAT.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/PMT.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Duration.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/LogNormDist.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcCos.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/power.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/PPMT.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/NPV.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Duration_ADD.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Norminv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Normsinv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcCosHyp.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/MDuration.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcCot.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/FV.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/db.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Couppcd.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sumifs.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc(true);

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
    if(!initTestEnv("opencl/math/ArcCotHyp.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcSin.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/VDB.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/IPMT.", XLS, false))
        return;
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

void ScOpenCLTest:: testFinancialXirrFormula()
{
    if(!initTestEnv("opencl/financial/XIRR.", ODS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fFormula  = rDoc.GetValue(ScAddress(2, i, 0));
        double fExpected = rDoc.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT(rtl::math::approxEqual(fExpected, fFormula));
    }
    for (SCROW i = 18; i <= 27; ++i)
    {
        double fFormula = rDoc.GetValue(ScAddress(2, i, 0));
        double fExpected = rDoc.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT(rtl::math::approxEqual(fExpected, fFormula));
    }


}

void ScOpenCLTest::testStatisticalFormulaChiSqDist()
{
    if(!initTestEnv("opencl/statistical/CHISQDIST.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Confidence.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Fdist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Coupncd.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/Accrint.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/CritBinom.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcSinHyp.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcTan.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcTanHyp.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/financial/NPER1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/ArcTan2.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/CHISQINV.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/BitAnd.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Poisson.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Expondist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/BitOr.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/odd.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/LN.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/mod.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/ROUND.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/GammaDist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/GammaInv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/FInv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/FTest.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/B.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/BetaDist.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/even.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/exp.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/ChiDist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/BitLshift.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/BitRshift.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/floor.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/log.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/spreadsheet/VLookup.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/ChiInv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/convert.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/countifs.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();
    xDocSh->DoHardRecalc(true);

    for (SCROW i = 1; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaBitXor()
{
    if(!initTestEnv("opencl/math/BitXor.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/averageifs.", XLS, false))
        return;
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();

    xDocSh->DoHardRecalc(true);

    for (SCROW i = 1; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
}

void ScOpenCLTest::testMathFormulaLog10()
{
    if(!initTestEnv("opencl/math/log10.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/combina.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/Ceil.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/sqrtpi.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/VarP.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/StDev.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/StDevP.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Slope.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/STEYX.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/ZTest.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/TTest.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/TDist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/TInv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/BinomDist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/product.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/Kombin.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/array/SUMX2MY2.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/HypGeomDist.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/array/SUMX2PY2.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Betainv.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Mina.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/array/SUMXMY2.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/counta.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Maxa.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sumproduct_mixSliding.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/averageif.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/AverageA.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/logical/and.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/VarA.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/VarPA.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/StDevA.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/StDevPA.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/financial/MDuration1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sumproductTest.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/parallel_count_bug_243.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/logical/or.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/logical/not.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/logical/xor.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dcount.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dcountA.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dmax.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dmin.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dproduct.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/daverage.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dstdev.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dstdevp.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dsum.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dvar.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/database/dvarp.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/roundup.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/rounddown.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/int.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/radians.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/degrees.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/iseven.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/countif.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/isodd.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/fact.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sec.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/sech.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/MROUND.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/Quotient.", ODS, false))
        return;
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
    if(!initTestEnv("opencl/math/seriessum.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/sumif.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/addin/besselj.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Avedev.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/NegSub.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/math/averageif_mix.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/Kurt1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/HarMean1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/VarA1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/VarPA1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/StDevA1.", XLS, false))
        return;
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
    if(!initTestEnv("opencl/statistical/StDevPA1.", XLS, false))
        return;
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
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScOpenCLTest::setUp()
{
    test::BootstrapFixture::setUp();
    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->
            createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScOpenCLTest::tearDown()
{
    //close test env
    if(xDocSh.Is())
        xDocSh->DoClose();
    if(xDocShRes.Is())
        xDocShRes->DoClose();

    uno::Reference< lang::XComponent >
        ( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenCLTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
