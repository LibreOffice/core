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
        std::cout<<fLibre<<"\t"<<fExcel<<"\n";
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
