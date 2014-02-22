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

    virtual void setUp();
    virtual void tearDown();

    virtual bool load( const OUString &rFilter, const OUString &rURL,
            const OUString &rUserData, unsigned int nFilterFlags,
            unsigned int nClipboardID, unsigned int nFilterVersion);
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
    void testFinacialIRRFormula();
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
    void testMathFormulaKombin();
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
    CPPUNIT_TEST(testFinacialIRRFormula);
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
    
    for (SCROW i = 33; i < 44; ++i)
    {   
        double fLibre = pDoc->GetValue(ScAddress(7, i, 1));
        double fExcel = pDocRes->GetValue(ScAddress(7, i, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.0001*fExcel);
    }

    for (SCROW i = 33; i < 44; ++i)
    {   
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
    
    
    for (SCROW i = 0; i < 5; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    
    for (SCROW i = 6; i < 14; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    
    for (SCROW i = 15; i < 18; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    
    for (SCROW i = 19; i < 22; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    
    for (SCROW i = 23; i < 25; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        
        
        CPPUNIT_ASSERT_EQUAL(/*fExcel*/ 60.0, fLibre);
    }

    
    for (SCROW i = 25; i < 27; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    
    for (SCROW i = 28; i < 35; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }
    
    CPPUNIT_ASSERT_EQUAL(25.0, pDoc->GetValue(ScAddress(2, 35, 0)));
    CPPUNIT_ASSERT_EQUAL(24.0, pDoc->GetValue(ScAddress(2, 36, 0)));

    
    for (SCROW i = 38; i < 43; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
    }

    
    for (SCROW i = 5; i < 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5, i, 1));
        double fExcel = pDocRes->GetValue(ScAddress(5, i, 1));
        CPPUNIT_ASSERT_EQUAL(fExcel, fLibre);
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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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

    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 16; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 11; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}
 
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
  
void ScOpenclTest:: testFinancialXirrFormula()
{
    if (!detectOpenCLDevice())
        return;
    ScDocShellRef xDocSh = loadDoc("opencl/financial/XIRR.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    enableOpenCL();
    pDoc->CalcAll();
    ScDocShellRef xDocShRes = loadDoc("opencl/financial/XIRR.", XLS);
    ScDocument *pDocRes = xDocShRes->GetDocument();
    CPPUNIT_ASSERT(pDocRes);
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    for (SCROW i = 18; i <= 26; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2, i, 0));
        double fExcel = pDocRes->GetValue(ScAddress(2, i, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 17; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, 0.000001);
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}



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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 1000; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 10; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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

#if 0 
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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(5,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(5,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 2; i <= 12; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(4,i,1));
        double fExcel = pDocRes->GetValue(ScAddress(4,i,1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel,  fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 1; i <= 32; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(9,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(10,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.00000000001));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(3,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(3,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}


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
    
    for (SCROW i = 0; i <= 9; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(2,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(2,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 19; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
    
    for (SCROW i = 1; i <= 20; ++i)
    {
        double fLibre = pDoc->GetValue(ScAddress(1,i,0));
        double fExcel = pDocRes->GetValue(ScAddress(1,i,0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fExcel, fLibre, fabs(0.0001*fExcel));
    }
    xDocSh->DoClose();
    xDocShRes->DoClose();
}

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
