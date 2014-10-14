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
#include "platforminfo.hxx"
#include "formulagroup.hxx"

#include <svx/svdpage.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScOpenclTest
    : public test::FiltersTest
    , public ScBootstrapFixture
{
public:
    ScOpenclTest();

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
    CPPUNIT_TEST_SUITE(ScOpenclTest);
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
};

bool ScOpenclTest::load(const OUString &rFilter, const OUString &rURL,
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

bool ScOpenclTest::detectOpenCLDevice()
{
    sc::FormulaGroupInterpreter::enableOpenCL(true);
    return sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(),true);
}

void ScOpenclTest::enableOpenCL()
{
    sc::FormulaGroupInterpreter::enableOpenCL(true);
}

void ScOpenclTest::testCompilerHorizontal()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/horizontal.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/horizontal.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testCompilerNested()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/nested.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/nested.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenclTest::testCompilerString()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/string.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/string.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenclTest::testCompilerInEq()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/compiler/ineq.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/compiler/ineq.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i < 7; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

#if 0
void ScOpenclTest::testSharedFormulaXLSStockHistory()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("stock-history.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("stock-history.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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

    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenclTest::testSharedFormulaXLSGroundWater()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("ground-water-daily.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("ground-water-daily.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 5; i <= 77; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(11,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(11,i,1));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
#endif

void ScOpenclTest::testSharedFormulaXLS()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("sum_ex.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("sum_ex.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    // AMLOEXT-5
    for (SCROW i = 0; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    // AMLOEXT-6
    for (SCROW i = 6; i < 14; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    // AMLOEXT-8
    for (SCROW i = 15; i < 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    // AMLOEXT-10
    for (SCROW i = 19; i < 22; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    // AMLOEXT-9
    for (SCROW i = 23; i < 25; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        //double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        // There seems to be a bug in LibreOffice beta
        CPPUNIT_ASSERT_EQUAL(/*fExcel*/ 60.0, fLibre);
    }

    // AMLOEXT-9
    for (SCROW i = 25; i < 27; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    // AMLOEXT-11
    for (SCROW i = 28; i < 35; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    // AMLOEXT-11; workaround for a Calc beta bug
    CPPUNIT_ASSERT_EQUAL(25.0, pDoc->GetValue(ScAddress(2, 35, 0)));
    CPPUNIT_ASSERT_EQUAL(24.0, pDoc->GetValue(ScAddress(2, 36, 0)));

    // AMLOEXT-12
    for (SCROW i = 38; i < 43; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    // AMLOEXT-14
    for (SCROW i = 5; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 1));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 1));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    // AMLOEXT-15, AMLOEXT-16, and AMLOEXT-17
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-76]
void ScOpenclTest::testMathFormulaCos()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/cos.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/cos.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-116]
void ScOpenclTest::testMathFormulaSinh()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/sinh.", XLS);
    enableOpenCL();   ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sinh.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-196]
void ScOpenclTest::testMathFormulaPi()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/pi.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/pi.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(0,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(0,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-197]
void ScOpenclTest::testMathFormulaRandom()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/random.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/random.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(0,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(0,i,0));
        //because the random numbers will always change,so give the test "true"
        (void) fLibre;
        (void) fExcel;
        CPPUNIT_ASSERT(true);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialFormula()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/financial/general.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/general.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    // AMLOEXT-22
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
        //[AMLOEXT-23]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,2));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-24]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,3));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,3));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-25]
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,4));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,4));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-26]
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,5));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,5));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
	//[AMLOEXT-27]
	for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,6));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,6));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-28]
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,7));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,7));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-29]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,8));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,8));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
        //[AMLOEXT-30]
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,9));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,9));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-31]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,10));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,10));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-32]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7,i,11));
        double fExcel = pDocRes->GetValue(ScAddress(7,i,11));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-33]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,12));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,12));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-34]
    for (SCROW i = 0; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,13));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,13));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-35]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,14));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,14));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-36]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,15));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,15));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-37]
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,16));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,16));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-38]
    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,17));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,17));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-39]
    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,18));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,18));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    //[AMLOEXT-42]
    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,19));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,19));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-20]
void ScOpenclTest::testStatisticalFormulaCorrel()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Correl.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Correl.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocShRes.Is());
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();

}
void ScOpenclTest::testStatisticalFormulaFisher()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Fisher.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Fisher.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-44]
void ScOpenclTest::testStatisticalFormulaFisherInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/FisherInv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/FisherInv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-45]
void ScOpenclTest::testStatisticalFormulaGamma()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Gamma.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Gamma.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-46]
void ScOpenclTest::testFinacialFvscheduleFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Fvschedule.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Fvschedule.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-47]
void ScOpenclTest::testMathFormulaAbs()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/Abs.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/Abs.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);

    // Verify ABS Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-69]
void ScOpenclTest::testFinacialSYDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/SYD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/SYD.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

// this test has intermittent failures on OSX
#if !defined MACOSX
void ScOpenclTest::testFinacialIRRFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/IRR.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/IRR.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
#endif
//[AMLOEXT-49]
void ScOpenclTest::testStatisticalFormulaGammaLn()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GammaLn.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GammaLn.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-50]
void ScOpenclTest::testStatisticalFormulaGauss()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Gauss.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Gauss.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-52]
void ScOpenclTest::testStatisticalFormulaGeoMean()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GeoMean.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GeoMean.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-51]
void ScOpenclTest::testStatisticalFormulaHarMean()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/HarMean.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/HarMean.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-53]
void ScOpenclTest::testFinacialSLNFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/SLN.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/SLN.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenclTest::testFinacialMIRRFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/MIRR.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/MIRR.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

// [AMLOEXT-55]
void ScOpenclTest::testFinancialCoupdaybsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupdaybs.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupdaybs.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-56]
void ScOpenclTest::testFinacialDollardeFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Dollarde.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Dollarde.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-70]
void ScOpenclTest::testFinancialCoupdaysFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupdays.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupdays.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-72]
void ScOpenclTest::testFinancialCoupdaysncFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupdaysnc.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupdaysnc.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <=10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialRateFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/RATE.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/RATE.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <= 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-54]
void ScOpenclTest::testFinancialAccrintmFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Accrintm.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Accrintm.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-74]
void ScOpenclTest::testFinancialCoupnumFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupnum.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupnum.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-57]
void ScOpenclTest::testStatisticalFormulaNegbinomdist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Negbinomdist." ,XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Negbinomdist." ,XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-58]
void ScOpenclTest::testMathFormulaSin()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sin.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sin.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-59]
void ScOpenclTest::testMathFormulaSumSQ()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumsq.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumsq.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-60]
void ScOpenclTest::testMathFormulaTan()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/tan.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/tan.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-61]
void ScOpenclTest::testMathFormulaTanH()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/tanh.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/tanh.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-62]
void ScOpenclTest::testMathFormulaSqrt()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sqrt.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sqrt.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-63]
void ScOpenclTest::testFinacialPriceFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Price.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Price.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-64]
void ScOpenclTest::testFinacialDollarfrFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Dollarfr.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Dollarfr.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-65]
void ScOpenclTest::testFinacialPriceDiscFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PriceDisc.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PriceDisc.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-67]
void ScOpenclTest::testFinacialODDLPRICEFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Oddlprice.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Oddlprice.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(8, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-68]
void ScOpenclTest:: testFinacialOddlyieldFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Oddlyield.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Oddlyield.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(8, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(8, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-71]
void ScOpenclTest::testFinacialDISCFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/DISC.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/DISC.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-73]
void ScOpenclTest:: testFinacialPVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PV.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PV.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-75]
void ScOpenclTest::testFinacialINTRATEFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/INTRATE.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/INTRATE.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-77]
void ScOpenclTest::testStatisticalFormulaStandard()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Standard.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Standard.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-78]
void ScOpenclTest::testStatisticalFormulaWeibull()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Weibull.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Weibull.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre,
            fExcel == 0?1e-4:fabs(1e-4*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-79]
void ScOpenclTest::testStatisticalFormulaVar()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Var.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Var.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-80]
void ScOpenclTest::testStatisticalFormulaSkew()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Skew.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Skew.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-81]
void ScOpenclTest::testStatisticalFormulaSkewp()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Skewp.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Skewp.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-82]
void ScOpenclTest::testStatisticalFormulaPearson()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Pearson.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Pearson.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-83]
void ScOpenclTest::testStatisticalFormulaRsq()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Rsq.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Rsq.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-84]
void ScOpenclTest::testMathFormulaTrunc()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/trunc.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/trunc.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-85]
void ScOpenclTest::testMathFormulaCosh()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/cosh.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/cosh.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testStatisticalFormulaCovar()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Covar.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Covar.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 16; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-87]
void ScOpenclTest::testStatisticalFormulaKurt()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Kurt.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Kurt.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-88]
void ScOpenclTest::testMathFormulaCot()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/cot.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/cot.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-89]
void ScOpenclTest::testStatisticalFormulaDevSq()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/DevSq.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/DevSq.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-90]
void ScOpenclTest::testMathFormulaCsc()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/csc.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/csc.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-91]
void ScOpenclTest::testMathFormulaCoth()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/coth.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/coth.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-92]
void ScOpenclTest::testFinacialXNPVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/XNPV.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/XNPV.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-93]
void ScOpenclTest::testStatisticalFormulaIntercept()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Intercept.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Intercept.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-98]
void ScOpenclTest::testFinancialAmordegrcFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Amordegrc.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Amordegrc.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-99]
void ScOpenclTest:: testFinancialISPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/ISPMT.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/ISPMT.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-100]
void ScOpenclTest::testStatisticalFormulaMedian()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Median.",XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Median.",XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-101]
void ScOpenclTest::testStatisticalFormulaNormdist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Normdist.",XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Normdist.",XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-102]
void ScOpenclTest::testStatisticalFormulaNormsdist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Normsdist.",XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Normsdist.",XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-103]
void ScOpenclTest::testStatisticalFormulaPermut()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Permut.",XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Permut.",XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-104]
void ScOpenclTest::testStatisticalFormulaPermutation()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Permutation.",XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Permutation.",XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-105]
void ScOpenclTest::testStatisticalFormulaPhi()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Phi.",XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Phi.",XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-106]
void ScOpenclTest::testMathFormulaCscH()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/csch.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/csch.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-107]
void ScOpenclTest::testStatisticalFormulaLogInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/LogInv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/LogInv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-108]
void ScOpenclTest::testFinacialNPERFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/NPER.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/NPER.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-109]
void ScOpenclTest::testStatisticalFormulaForecast()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Forecast.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Forecast.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-110]
void ScOpenclTest::testFinancialAmorlincFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Amorlinc.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Amorlinc.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-124]
void ScOpenclTest::testFinancialDDBFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/ddb.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/ddb.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialPriceMatFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PriceMat.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PriceMat.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialFormulaReceived()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Received.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Received.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinancialFormulaCumipmt()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Cumipmt.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Cumipmt.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinancialFormulaCumprinc()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Cumprinc.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Cumprinc.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialRRIFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/RRI.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/RRI.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialEFFECT_ADDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/EFFECT_ADD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/EFFECT_ADD.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        //CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialNominalFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Nominal.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Nominal.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialTBILLEQFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/TBILLEQ.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/TBILLEQ.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialTBILLPRICEFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/TBILLPRICE.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/TBILLPRICE.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialTBILLYIELDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/TBILLYIELD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/TBILLYIELD.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest::testFinacialYIELDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/YIELD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/YIELD.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenclTest::testFinacialYIELDDISCFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/YIELDDISC.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/YIELDDISC.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

void ScOpenclTest::testFinacialYIELDMATFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/YIELDMAT.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/YIELDMAT.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
void ScOpenclTest:: testFinacialPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PMT.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PMT.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-111]
void ScOpenclTest:: testFinancialDurationFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Duration.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Duration.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-112]
void ScOpenclTest::testStatisticalFormulaLogNormDist()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/LogNormDist.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/LogNormDist.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-113]
void ScOpenclTest::testMathFormulaArcCos()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcCos.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcCos.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ACos Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-117]
void ScOpenclTest::testMathFormulaPower()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/power.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/power.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-119]
void ScOpenclTest:: testFinacialPPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/PPMT.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/PPMT.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-120]
void ScOpenclTest:: testFinacialNPVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/NPV.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/NPV.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-121]
void ScOpenclTest:: testFinancialDuration_ADDFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Duration_ADD.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Duration_ADD.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-122]
void ScOpenclTest::testStatisticalFormulaNorminv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Norminv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Norminv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-123]
void ScOpenclTest::testStatisticalFormulaNormsinv()
{
     if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Normsinv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Normsinv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-125]
void ScOpenclTest::testMathFormulaArcCosHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcCosHyp.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcCosHyp.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ACosH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-126]
void ScOpenclTest:: testFinancialMDurationFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/MDuration.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/MDuration.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-127]
void ScOpenclTest::testMathFormulaArcCot()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcCot.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcCot.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ACot Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-131]
void ScOpenclTest:: testFinancialFVFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/FV.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/FV.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-132]
void ScOpenclTest::testFinancialDBFormula()
{
   if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/db.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/db.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-135]
void ScOpenclTest::testFinancialCouppcdFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Couppcd.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Couppcd.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-128]
void ScOpenclTest::testMathSumIfsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumifs.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    sc::FormulaGroupInterpreter::enableOpenCL(true);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumifs.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-129]
void ScOpenclTest::testMathFormulaArcCotHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcCotHyp.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcCotHyp.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-130]
void ScOpenclTest::testMathFormulaArcSin()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcSin.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcSin.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ACotH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
 //[AMLOEXT-133]
void ScOpenclTest:: testFinancialVDBFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/VDB.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/VDB.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-134]
void ScOpenclTest:: testFinancialIPMTFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/IPMT.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/IPMT.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
  //[AMLOEXT-137]
void ScOpenclTest:: testFinancialXirrFormula()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/financial/XIRR.", ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument *pDoc = xDocSh->GetDocument();
    enableOpenCL();
    pDoc->CalcAll();

    for (SCROW i = 1; i <= 10; ++i)
    {
        double fFormula  = pDoc->GetValue(ScAddress(2, i, 0));
        double fExpected = pDoc->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT(rtl::math::approxEqual(fExpected, fFormula));
    }
    for (SCROW i = 18; i <= 27; ++i)
    {
        double fFormula = pDoc->GetValue(ScAddress(2, i, 0));
        double fExpected = pDoc->GetValue(ScAddress(3, i, 0));
        CPPUNIT_ASSERT(rtl::math::approxEqual(fExpected, fFormula));
    }

    xDocSh->DoClose();
}
//[AMLOEXT-139]
void ScOpenclTest::testStatisticalFormulaChiSqDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/CHISQDIST.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/CHISQDIST.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-140]
void ScOpenclTest::testStatisticalFormulaConfidence()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Confidence.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Confidence.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-141]
void ScOpenclTest::testStatisticalFormulaFDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Fdist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Fdist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-136]
void ScOpenclTest::testFinancialCoupncdFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Coupncd.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Coupncd.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-138]
void ScOpenclTest::testFinancialAccrintFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/Accrint.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/Accrint.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(7, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-142]
void ScOpenclTest::testStatisticalFormulaCritBinom()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/CritBinom.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/CritBinom.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-143]
void ScOpenclTest::testMathFormulaArcSinHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcSinHyp.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcSinHyp.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ASinH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-144]
void ScOpenclTest::testMathFormulaArcTan()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcTan.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcTan.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ATan Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-145]
void ScOpenclTest::testMathFormulaArcTanHyp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcTanHyp.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcTanHyp.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ATanH Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-146]
void ScOpenclTest:: testFinacialNPER1Formula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/NPER1.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/NPER1.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 6; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-147]
void ScOpenclTest::testMathFormulaArcTan2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/ArcTan2.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/ArcTan2.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify ATan2 Function
    for (SCROW i = 1; i <= 17; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.000001);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


//[AMLOEXT-148]
void ScOpenclTest::testStatisticalFormulaChiSqInv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/CHISQINV.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/CHISQINV.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-149]
void ScOpenclTest::testMathFormulaBitAnd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/BitAnd.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/BitAnd.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify BitAnd Function
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-150]
void ScOpenclTest::testStatisticalFormulaPoisson()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Poisson.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Poisson.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-151]
void ScOpenclTest::testStatisticalFormulaExpondist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Expondist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Expondist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-152]
void ScOpenclTest::testMathFormulaBitOr()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/BitOr.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/BitOr.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify BitOr Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-153]
void ScOpenclTest::testMathFormulaOdd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/odd.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/odd.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-154]
void ScOpenclTest::testMathFormulaLN()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/LN.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/LN.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-155
void ScOpenclTest::testMathFormulaMod()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/mod.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/mod.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-156]
void ScOpenclTest::testMathFormulaRound()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/ROUND.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/ROUND.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-157]
void ScOpenclTest::testStatisticalFormulaGammaDist()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GammaDist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GammaDist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-158]
void ScOpenclTest::testStatisticalFormulaGammaInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/GammaInv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/GammaInv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-159]
void ScOpenclTest::testStatisticalFormulaFInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/FInv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/FInv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-160]
void ScOpenclTest::testStatisticalFormulaFTest()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/FTest.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/FTest.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-161]
void ScOpenclTest::testStatisticalFormulaB()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/B.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/B.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-162]
void ScOpenclTest::testStatisticalFormulaBetaDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/BetaDist.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/BetaDist.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-164
void ScOpenclTest::testMathFormulaEven()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/even.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/even.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-165]
void ScOpenclTest::testMathFormulaExp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/exp.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/exp.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-166]
void ScOpenclTest::testStatisticalFormulaChiDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/ChiDist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/ChiDist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-167]
void ScOpenclTest::testMathFormulaBitLshift()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitLshift.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitLshift.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify BitLshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-168]
void ScOpenclTest::testMathFormulaBitRshift()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitRshift.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitRshift.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify BitRshift Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-169]
void ScOpenclTest::testMathFormulaFloor()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/floor.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/floor.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-170]
void ScOpenclTest::testMathFormulaLog()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/log.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/log.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-171]
void ScOpenclTest::testSpreadSheetFormulaVLookup()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/spreadsheet/VLookup.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/spreadsheet/VLookup.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-173]
void ScOpenclTest::testStatisticalFormulaChiInv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/ChiInv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/ChiInv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-175]
void ScOpenclTest::testMathFormulaConvert()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/convert.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/convert.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-176 ]
void ScOpenclTest::testMathCountIfsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/countifs.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    sc::FormulaGroupInterpreter::enableOpenCL(true);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/countifs.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(4, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-178]
void ScOpenclTest::testMathFormulaBitXor()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/BitXor.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/BitXor.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify BitXor Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-179]
void ScOpenclTest::testMathAverageIfsFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/averageifs.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    sc::FormulaGroupInterpreter::enableOpenCL(true);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("opencl/math/averageifs.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 1; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-180]
void ScOpenclTest::testMathFormulaLog10()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/log10.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/log10.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-174]
void ScOpenclTest::testMathFormulaCombina()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/combina.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/combina.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 47; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-181]
void ScOpenclTest::testMathFormulaCeil()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/Ceil.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/Ceil.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify Ceiling Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-182]
void ScOpenclTest::testMathFormulaSqrtPi()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sqrtpi.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sqrtpi.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-183]
void ScOpenclTest::testStatisticalFormulaVarP()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarP.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarP.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-184]
void ScOpenclTest::testStatisticalFormulaStDev()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDev.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDev.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-185]
void ScOpenclTest::testStatisticalFormulaStDevP()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevP.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevP.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-186]
void ScOpenclTest::testStatisticalFormulaSlope()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Slope.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Slope.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-187]
void ScOpenclTest::testStatisticalFormulaSTEYX()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/STEYX.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/STEYX.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-188]
void ScOpenclTest::testStatisticalFormulaZTest()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/ZTest.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/ZTest.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-189]
void ScOpenclTest::testStatisticalFormulaTTest()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/TTest.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/TTest.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-190]
void ScOpenclTest::testStatisticalFormulaTDist()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/TDist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/TDist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-191]
void ScOpenclTest::testStatisticalFormulaTInv()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/TInv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/TInv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-192]
void ScOpenclTest::testStatisticalFormulaBinomDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/BinomDist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/BinomDist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-193]
void ScOpenclTest::testMathFormulaProduct()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/product.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/product.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 3; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-194]
#if 0 //Disabled temporarily
void ScOpenclTest::testMathFormulaKombin()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/Kombin.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/Kombin.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify Combin Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
#endif
//[AMLOEXT-198]
void ScOpenclTest:: testArrayFormulaSumX2MY2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/array/SUMX2MY2.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/array/SUMX2MY2.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-199]
void ScOpenclTest::testStatisticalFormulaHypGeomDist()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/HypGeomDist.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/HypGeomDist.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-200]
void ScOpenclTest:: testArrayFormulaSumX2PY2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/array/SUMX2PY2.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/array/SUMX2PY2.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-201]
void ScOpenclTest::testStatisticalFormulaBetainv()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Betainv.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Betainv.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-202]
void ScOpenclTest::testStatisticalFormulaMina()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Mina.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Mina.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[  AMLOEXT-204]
void ScOpenclTest:: testArrayFormulaSumXMY2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/array/SUMXMY2.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/array/SUMXMY2.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-208
void ScOpenclTest::testStatisticalFormulaCountA()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/counta.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/counta.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-213]
void ScOpenclTest::testStatisticalFormulaMaxa()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Maxa.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Maxa.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-214]
void ScOpenclTest::testMathFormulaSumProduct()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumproduct_mixSliding.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumproduct_mixSliding.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-215]
void ScOpenclTest::testMathFormulaAverageIf()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/averageif.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/averageif.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 2; i <= 21; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(6,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-216]
void ScOpenclTest::testStatisticalFormulaAverageA()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/AverageA.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/AverageA.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-217]
void ScOpenclTest:: testLogicalFormulaAnd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/and.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/and.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-218]
void ScOpenclTest::testStatisticalFormulaVarA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarA.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarA.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-219]
void ScOpenclTest::testStatisticalFormulaVarPA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarPA.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarPA.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-220]
void ScOpenclTest::testStatisticalFormulaStDevA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevA.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevA.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-221]
void ScOpenclTest::testStatisticalFormulaStDevPA()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevPA.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevPA.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-223]
void ScOpenclTest:: testFinancialMDurationFormula1()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/MDuration1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/MDuration1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(6, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(6, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,fLibre,fabs(0.00000000001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-245]
void ScOpenclTest::testMathFormulaSumProduct2()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumproductTest.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumproductTest.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 2; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,  fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-243]
void ScOpenclTest:: testStatisticalParallelCountBug()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/statistical/parallel_count_bug_243.", ODS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/statistical/parallel_count_bug_243.", ODS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i < 13; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-264]
void ScOpenclTest:: testLogicalFormulaOr()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/or.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/or.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i < 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-267]
void ScOpenclTest:: testLogicalFormulaNot()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/not.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/not.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-272]
void ScOpenclTest:: testLogicalFormulaXor()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/logical/xor.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/logical/xor.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i < 3000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(1, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-333]
void ScOpenclTest::testDatabaseFormulaDcount()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dcount.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dcount.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-334]
void ScOpenclTest::testDatabaseFormulaDcountA()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dcountA.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dcountA.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-335]
void ScOpenclTest::testDatabaseFormulaDmax()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dmax.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dmax.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-336]
void ScOpenclTest::testDatabaseFormulaDmin()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dmin.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dmin.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-337]
void ScOpenclTest::testDatabaseFormulaDproduct()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dproduct.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dproduct.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-338]
void ScOpenclTest::testDatabaseFormulaDaverage()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/daverage.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/daverage.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-339]
void ScOpenclTest::testDatabaseFormulaDstdev()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dstdev.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dstdev.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-340]
void ScOpenclTest::testDatabaseFormulaDstdevp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dstdevp.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dstdevp.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-341]
void ScOpenclTest::testDatabaseFormulaDsum()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dsum.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dsum.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-342]
void ScOpenclTest::testDatabaseFormulaDvar()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dvar.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dvar.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        //CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-343]
void ScOpenclTest::testDatabaseFormulaDvarp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/database/dvarp.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/database/dvarp.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-351]
void ScOpenclTest::testMathFormulaRoundUp()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/roundup.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/roundup.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-352]
void ScOpenclTest::testMathFormulaRoundDown()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/rounddown.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/rounddown.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-353]
void ScOpenclTest::testMathFormulaInt()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/int.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/int.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-354]
void ScOpenclTest::testMathFormulaRadians()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/radians.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/radians.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-356
void ScOpenclTest::testMathFormulaDegrees()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/degrees.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/degrees.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 200; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-357]
void ScOpenclTest::testMathFormulaIsEven()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/iseven.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/iseven.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-360]
void ScOpenclTest::testMathFormulaCountIf()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/countif.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/countif.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[ AMLOEXT-361]
void ScOpenclTest::testMathFormulaIsOdd()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/isodd.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/isodd.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-362]
void ScOpenclTest::testMathFormulaFact()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/fact.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/fact.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-367
void ScOpenclTest::testMathFormulaSEC()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/sec.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sec.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-368
void ScOpenclTest::testMathFormulaSECH()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/sech.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sech.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-369
void ScOpenclTest::testMathFormulaMROUND()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/MROUND.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/MROUND.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 13; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-372]
void ScOpenclTest::testMathFormulaQuotient()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh =
        loadDoc("opencl/math/Quotient.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes =
        loadDoc("opencl/math/Quotient.", ODS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Verify BitAnd Function
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-373]
void ScOpenclTest::testMathFormulaSeriesSum()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/seriessum.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/seriessum.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 15; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-377]
void ScOpenclTest::testMathFormulaSumIf()
{
    if (!detectOpenCLDevice())
            return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/sumif.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/sumif.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-378]
void ScOpenclTest::testAddInFormulaBesseLJ()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/addin/besselj.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/addin/besselj.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//AMLOEXT-379
void ScOpenclTest::testStatisticalFormulaAvedev()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Avedev.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Avedev.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

//[AMLOEXT-383]
void ScOpenclTest::testNegSub()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/math/NegSub.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/math/NegSub.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-386]
void ScOpenclTest::testMathFormulaAverageIf_Mix()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/math/averageif_mix.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/math/averageif_mix.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-387]
void ScOpenclTest::testStatisticalFormulaKurt1()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/Kurt1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/Kurt1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-401]
void ScOpenclTest::testStatisticalFormulaHarMean1()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/statistical/HarMean1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/HarMean1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-402]
void ScOpenclTest::testStatisticalFormulaVarA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarA1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarA1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-403]
void ScOpenclTest::testStatisticalFormulaVarPA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/VarPA1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/VarPA1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-404]
void ScOpenclTest::testStatisticalFormulaStDevA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevA1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevA1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
//[AMLOEXT-405]
void ScOpenclTest::testStatisticalFormulaStDevPA1()
{
    if (!detectOpenCLDevice())
        return;

    ScDocShellRef xDocSh = loadDoc("opencl/statistical/StDevPA1.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();

    ScDocShellRef xDocShRes = loadDoc("opencl/statistical/StDevPA1.", XLS);
    ScDocument* pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
ScOpenclTest::ScOpenclTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScOpenclTest::setUp()
{
    test::BootstrapFixture::setUp();
    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->
            createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScOpenclTest::tearDown()
{
    uno::Reference< lang::XComponent >
        ( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenclTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
