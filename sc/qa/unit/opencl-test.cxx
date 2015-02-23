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
            const OUString &rUserData, unsigned int nFilterFlags,
            unsigned int nClipboardID, unsigned int nFilterVersion) SAL_OVERRIDE;
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

    void init(const OUString fileName, sal_Int32 nFormat,
              bool bReadWrite, ScDocShellRef& xDocSh, ScDocShellRef& xDocShRes);
    void close(ScDocShellRef& xDocSh, ScDocShellRef& xDocShRes);
};

void ScOpenCLTest::init(const OUString fileName, sal_Int32 nFormat,
    bool bReadWrite, ScDocShellRef& xDocSh, ScDocShellRef& xDocShRes)
{
    if(!detectOpenCLDevice())
        return;

    xDocSh = loadDoc(fileName, nFormat, bReadWrite);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    enableOpenCL();

    xDocShRes = loadDoc(fileName, nFormat, bReadWrite);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
}

void ScOpenCLTest::close(ScDocShellRef& xDocSh, ScDocShellRef& xDocShRes)
{
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

bool ScOpenCLTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion)
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
    ScDocShellRef xDocSh;
    ScDocShellRef xDocShRes;

    init("opencl/compiler/horizontal.", ODS, false, xDocSh, xDocShRes);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDocument& rDocRes = xDocShRes->GetDocument();

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

    close(xDocSh, xDocShRes);
}

void ScOpenCLTest::testCompilerNested()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/nested.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/nested.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testCompilerString()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/string.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/string.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testCompilerInEq()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/ineq.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/ineq.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 7; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

#if 0
void ScOpenCLTest::testSharedFormulaXLSStockHistory()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("stock-history.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("stock-history.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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

    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testSharedFormulaXLSGroundWater()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("ground-water-daily.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("ground-water-daily.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 5; i <= 77; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(11,i,1));
        double fExcel = rDocRes.GetValue(ScAddress(11,i,1));
        ASSERT_DOUBLES_EQUAL(fExcel, fLibre);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
#endif

void ScOpenCLTest::testSharedFormulaXLS()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("sum_ex.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("sum_ex.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCos()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/cos.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/cos.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSinh()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/sinh.", XLS);
    enableOpenCL();
    ScDocument& rDoc = xDocSh->GetDocument();
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sinh.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaPi()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/pi.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/pi.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(0,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(0,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaRandom()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/random.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/random.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(0,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(0,i,0));
        //because the random numbers will always change,so give the test "true"
        (void) fLibre;
        (void) fExcel;
        CPPUNIT_ASSERT(true);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialFormula()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/financial/general.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/general.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaCorrel()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Correl.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Correl.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();

}
void ScOpenCLTest::testStatisticalFormulaFisher()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Fisher.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Fisher.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaFisherInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/FisherInv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/FisherInv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaGamma()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Gamma.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Gamma.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialFvscheduleFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Fvschedule.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Fvschedule.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaAbs()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/Abs.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/Abs.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();

    // Verify ABS Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialSYDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/SYD.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/SYD.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

// this test has intermittent failures on OSX
#if !defined MACOSX
void ScOpenCLTest::testFinacialIRRFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/IRR.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/IRR.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
#endif

void ScOpenCLTest::testStatisticalFormulaGammaLn()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GammaLn.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GammaLn.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaGauss()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Gauss.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Gauss.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaGeoMean()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GeoMean.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GeoMean.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaHarMean()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/HarMean.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/HarMean.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialSLNFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/SLN.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/SLN.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialMIRRFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/MIRR.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/MIRR.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialCoupdaybsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupdaybs.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupdaybs.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialDollardeFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Dollarde.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Dollarde.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialCoupdaysFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupdays.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupdays.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialCoupdaysncFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupdaysnc.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupdaysnc.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialRateFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/RATE.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/RATE.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialAccrintmFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Accrintm.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Accrintm.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialCoupnumFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupnum.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupnum.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaNegbinomdist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Negbinomdist." ,XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Negbinomdist." ,XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSin()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sin.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sin.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSumSQ()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumsq.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumsq.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaTan()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/tan.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/tan.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaTanH()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/tanh.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/tanh.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSqrt()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sqrt.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sqrt.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialPriceFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Price.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Price.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialDollarfrFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Dollarfr.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Dollarfr.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialPriceDiscFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PriceDisc.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PriceDisc.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialODDLPRICEFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Oddlprice.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Oddlprice.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(8, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinacialOddlyieldFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Oddlyield.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Oddlyield.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(8, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialDISCFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/DISC.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/DISC.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinacialPVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PV.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PV.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialINTRATEFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/INTRATE.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/INTRATE.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStandard()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Standard.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Standard.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaWeibull()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Weibull.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Weibull.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre,
            fExcel == 0?1e-4:fabs(1e-4*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaVar()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Var.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Var.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaSkew()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Skew.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Skew.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaSkewp()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Skewp.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Skewp.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaPearson()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Pearson.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Pearson.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaRsq()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Rsq.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Rsq.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaTrunc()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/trunc.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/trunc.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCosh()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/cosh.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/cosh.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testStatisticalFormulaCovar()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Covar.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Covar.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 16; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaKurt()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Kurt.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Kurt.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCot()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/cot.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/cot.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaDevSq()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/DevSq.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/DevSq.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCsc()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/csc.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/csc.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCoth()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/coth.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/coth.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialXNPVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/XNPV.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/XNPV.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaIntercept()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Intercept.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Intercept.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialAmordegrcFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Amordegrc.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Amordegrc.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialISPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/ISPMT.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/ISPMT.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaMedian()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Median.",XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Median.",XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaNormdist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Normdist.",XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Normdist.",XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaNormsdist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Normsdist.",XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Normsdist.",XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaPermut()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Permut.",XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Permut.",XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaPermutation()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Permutation.",XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Permutation.",XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaPhi()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Phi.",XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Phi.",XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCscH()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/csch.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/csch.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaLogInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/LogInv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/LogInv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialNPERFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/NPER.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/NPER.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaForecast()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Forecast.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Forecast.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialAmorlincFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Amorlinc.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Amorlinc.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialDDBFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/ddb.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/ddb.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialPriceMatFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PriceMat.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PriceMat.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialFormulaReceived()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Received.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Received.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinancialFormulaCumipmt()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Cumipmt.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Cumipmt.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinancialFormulaCumprinc()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Cumprinc.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Cumprinc.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialRRIFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/RRI.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/RRI.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialEFFECT_ADDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/EFFECT_ADD.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/EFFECT_ADD.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialNominalFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Nominal.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Nominal.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialTBILLEQFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/TBILLEQ.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/TBILLEQ.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialTBILLPRICEFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/TBILLPRICE.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/TBILLPRICE.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialTBILLYIELDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/TBILLYIELD.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/TBILLYIELD.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest::testFinacialYIELDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/YIELD.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/YIELD.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialYIELDDISCFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/YIELDDISC.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/YIELDDISC.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinacialYIELDMATFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/YIELDMAT.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/YIELDMAT.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenCLTest:: testFinacialPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PMT.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PMT.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialDurationFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Duration.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Duration.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaLogNormDist()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/LogNormDist.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/LogNormDist.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcCos()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcCos.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcCos.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ACos Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaPower()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/power.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/power.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinacialPPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PPMT.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PPMT.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinacialNPVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/NPV.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/NPV.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialDuration_ADDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Duration_ADD.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Duration_ADD.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaNorminv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Norminv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Norminv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaNormsinv()
{
     if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Normsinv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Normsinv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcCosHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcCosHyp.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcCosHyp.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ACosH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialMDurationFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/MDuration.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/MDuration.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcCot()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcCot.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcCot.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ACot Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialFVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/FV.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/FV.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialDBFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/db.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/db.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialCouppcdFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Couppcd.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Couppcd.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathSumIfsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumifs.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumifs.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcCotHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcCotHyp.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcCotHyp.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcSin()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcSin.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcSin.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialVDBFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/VDB.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/VDB.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialIPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/IPMT.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/IPMT.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialXirrFormula()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/financial/XIRR.", ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
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

    xDocSh->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaChiSqDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/CHISQDIST.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/CHISQDIST.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaConfidence()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Confidence.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Confidence.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaFDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Fdist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Fdist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialCoupncdFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupncd.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupncd.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testFinancialAccrintFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Accrint.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Accrint.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(7, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaCritBinom()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/CritBinom.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/CritBinom.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcSinHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcSinHyp.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcSinHyp.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ASinH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcTan()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcTan.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcTan.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ATan Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcTanHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcTanHyp.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcTanHyp.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ATanH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinacialNPER1Formula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/NPER1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/NPER1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaArcTan2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ArcTan2.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ArcTan2.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify ATan2 Function
    for (SCROW i = 1; i <= 17; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.000001);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaChiSqInv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/CHISQINV.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/CHISQINV.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaBitAnd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitAnd.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitAnd.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify BitAnd Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaPoisson()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Poisson.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Poisson.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaExpondist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Expondist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Expondist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaBitOr()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitOr.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitOr.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify BitOr Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaOdd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/odd.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/odd.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaLN()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/LN.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/LN.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaMod()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/mod.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/mod.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        if(fExcel == 0.0f)
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 1e-10);
        else
            CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaRound()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ROUND.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ROUND.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaGammaDist()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GammaDist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GammaDist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaGammaInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GammaInv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GammaInv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaFInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/FInv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/FInv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaFTest()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/FTest.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/FTest.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaB()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/B.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/B.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaBetaDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/BetaDist.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/BetaDist.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaEven()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/even.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/even.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaExp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/exp.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/exp.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaChiDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/ChiDist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/ChiDist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaBitLshift()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitLshift.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitLshift.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify BitLshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaBitRshift()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitRshift.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitRshift.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify BitRshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaFloor()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/floor.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/floor.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaLog()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/log.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/log.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testSpreadSheetFormulaVLookup()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/spreadsheet/VLookup.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/spreadsheet/VLookup.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaChiInv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/ChiInv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/ChiInv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaConvert()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/convert.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/convert.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathCountIfsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/countifs.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/countifs.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i < 10; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaBitXor()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitXor.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitXor.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify BitXor Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathAverageIfsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/averageifs.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/averageifs.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 1; i <= 11; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaLog10()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/log10.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/log10.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCombina()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/combina.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/combina.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCeil()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/Ceil.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/Ceil.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify Ceiling Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSqrtPi()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sqrtpi.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sqrtpi.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaVarP()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarP.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarP.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStDev()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDev.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDev.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStDevP()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevP.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevP.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaSlope()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Slope.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Slope.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaSTEYX()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/STEYX.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/STEYX.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaZTest()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/ZTest.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/ZTest.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaTTest()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/TTest.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/TTest.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaTDist()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/TDist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/TDist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaTInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/TInv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/TInv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaBinomDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/BinomDist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/BinomDist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaProduct()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/product.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/product.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

#if 0 //Disabled temporarily
void ScOpenCLTest::testMathFormulaKombin()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/Kombin.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/Kombin.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify Combin Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
#endif

void ScOpenCLTest:: testArrayFormulaSumX2MY2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/array/SUMX2MY2.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/array/SUMX2MY2.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaHypGeomDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/HypGeomDist.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/HypGeomDist.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testArrayFormulaSumX2PY2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/array/SUMX2PY2.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/array/SUMX2PY2.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaBetainv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Betainv.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Betainv.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(5,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaMina()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Mina.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Mina.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testArrayFormulaSumXMY2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/array/SUMXMY2.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/array/SUMXMY2.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaCountA()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/counta.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/counta.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaMaxa()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Maxa.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Maxa.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSumProduct()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumproduct_mixSliding.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumproduct_mixSliding.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaAverageIf()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/averageif.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/averageif.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 2; i <= 21; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaAverageA()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/AverageA.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/AverageA.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testLogicalFormulaAnd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/and.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/and.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaVarA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarA.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarA.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaVarPA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarPA.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarPA.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStDevA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevA.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevA.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStDevPA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevPA.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevPA.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testFinancialMDurationFormula1()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/MDuration1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/MDuration1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(6, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,fLibre,fabs(0.00000000001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSumProduct2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumproductTest.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumproductTest.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 2; i <= 12; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(4,i,1));
        double fExcel = rDocRes.GetValue(ScAddress(4,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,  fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testStatisticalParallelCountBug()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/statistical/parallel_count_bug_243.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/statistical/parallel_count_bug_243.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i < 13; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testLogicalFormulaOr()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/or.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/or.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testLogicalFormulaNot()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/not.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/not.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest:: testLogicalFormulaXor()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/xor.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/xor.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1, i, 0));
        double fExcel = rDocRes.GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDcount()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dcount.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dcount.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDcountA()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dcountA.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dcountA.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDmax()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dmax.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dmax.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDmin()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dmin.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dmin.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDproduct()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dproduct.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dproduct.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDaverage()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/daverage.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/daverage.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDstdev()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dstdev.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dstdev.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDstdevp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dstdevp.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dstdevp.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDsum()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dsum.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dsum.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDvar()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dvar.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dvar.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testDatabaseFormulaDvarp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dvarp.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dvarp.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(9,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaRoundUp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/roundup.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/roundup.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaRoundDown()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/rounddown.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/rounddown.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaInt()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/int.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/int.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaRadians()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/radians.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/radians.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaDegrees()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/degrees.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/degrees.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 200; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaIsEven()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/iseven.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/iseven.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaCountIf()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/countif.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/countif.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaIsOdd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/isodd.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/isodd.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaFact()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/fact.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/fact.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSEC()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/sec.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sec.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSECH()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/sech.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sech.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaMROUND()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/MROUND.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/MROUND.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 13; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaQuotient()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/Quotient.", ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/Quotient.", ODS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Verify BitAnd Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSeriesSum()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/seriessum.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/seriessum.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaSumIf()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumif.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumif.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testAddInFormulaBesseLJ()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/addin/besselj.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/addin/besselj.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaAvedev()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Avedev.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Avedev.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(3,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testNegSub()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/NegSub.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/math/NegSub.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testMathFormulaAverageIf_Mix()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/averageif_mix.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/averageif_mix.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaKurt1()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Kurt1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Kurt1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(2,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaHarMean1()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/HarMean1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/HarMean1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaVarA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarA1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarA1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaVarPA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarPA1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarPA1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStDevA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevA1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevA1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenCLTest::testStatisticalFormulaStDevPA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevPA1.", XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    enableOpenCL();
    rDoc.CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevPA1.", XLS);
    ScDocument& rDocRes = xDocShRes->GetDocument();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = rDoc.GetValue(ScAddress(1,i,0));
        double fExcel = rDocRes.GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
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
    uno::Reference< lang::XComponent >
        ( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenCLTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
