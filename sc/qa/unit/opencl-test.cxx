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

#define TEST_BUG_FILES 0

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
    void enableOpenCL(ScDocShell* pShell);
    void SetEnv();
    virtual void setUp();
    virtual void tearDown();

    virtual bool load( const OUString &rFilter, const OUString &rURL,
            const OUString &rUserData, unsigned int nFilterFlags,
            unsigned int nClipboardID, unsigned int nFilterVersion);
    void testSharedFormulaXLS();
    void testSharedFormulaXLSGroundWater();
    void testSharedFormulaXLSStockHistory();
    void testFinacialFormula();
    void testStatisticalFormulaFisher();
    void testStatisticalFormulaFisherInv();
    void testStatisticalFormulaGamma();
    void testFinacialFvscheduleFormula();
    void testFinacialIRRFormula();
    void testFinacialMIRRFormula();
    void testFinacialRateFormula();
    void testFinancialAccrintmFormula();
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
    CPPUNIT_TEST_SUITE(ScOpenclTest);
    CPPUNIT_TEST(testSharedFormulaXLS);
    CPPUNIT_TEST(testSharedFormulaXLSGroundWater);
    CPPUNIT_TEST(testSharedFormulaXLSStockHistory);
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
void ScOpenclTest::SetEnv()
{
    #ifdef WIN32
        char *newpathfull  = (char *)malloc(1024);
        newpathfull= "PATH=;C:\\Program Files (x86)\\AMD APP\\bin\\x86_64;";
        putenv(newpathfull);
    #else
        return;
    #endif
}
void ScOpenclTest::enableOpenCL(ScDocShell* pShell)
{
    ScModule* pScMod = SC_MOD();
    ScFormulaOptions rOpt = pScMod->GetFormulaOptions();
    ScCalcConfig maSavedConfig = rOpt.GetCalcConfig();
    maSavedConfig.mbOpenCLEnabled = true;
    rOpt.SetCalcConfig(maSavedConfig);
    pShell->SetFormulaOptions(rOpt);
}

void ScOpenclTest::testCompilerNested()
{
    ScDocShellRef xDocSh = loadDoc("OpenCLTests/Compiler/nested.", ODS);

    enableOpenCL(xDocSh);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("OpenCLTests/Compiler/nested.", ODS);
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
    ScDocShellRef xDocSh = loadDoc("OpenCLTests/Compiler/string.", ODS);

    enableOpenCL(xDocSh);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("OpenCLTests/Compiler/string.", ODS);
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
    ScDocShellRef xDocSh = loadDoc("OpenCLTests/Compiler/ineq.", ODS);

    enableOpenCL(xDocSh);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("OpenCLTests/Compiler/ineq.", ODS);
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

void ScOpenclTest::testSharedFormulaXLSStockHistory()
{
#if 1
    ScDocShellRef xDocSh = loadDoc("stock-history.", XLS);

    enableOpenCL(xDocSh);

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
#endif
}

void ScOpenclTest::testSharedFormulaXLSGroundWater()
{
    ScDocShellRef xDocSh = loadDoc("ground-water-daily.", XLS);

    enableOpenCL(xDocSh);

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

void ScOpenclTest::testSharedFormulaXLS()
{
    ScDocShellRef xDocSh = loadDoc("sum_ex.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/math/cos.", XLS);
    enableOpenCL(xDocSh);   ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/math/cos.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("FinancialFormulaTest.", XLS);
    enableOpenCL(xDocSh);   ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("FinancialFormulaTest.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenCLTests/statistical/Correl.", ODS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenCLTests/statistical/Correl.", ODS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Fisher.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Fisher.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/FisherInv.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/FisherInv.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Gamma.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Gamma.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Fvschedule.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Fvschedule.", XLS);
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
//[AMLOEXT-69]
void ScOpenclTest::testFinacialSYDFormula()
{
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/SYD.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/SYD.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/IRR.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/IRR.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/GammaLn.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/GammaLn.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Gauss.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Gauss.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/GeoMean.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/GeoMean.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/HarMean.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/HarMean.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/SLN.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/SLN.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/MIRR.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/MIRR.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Coupdaybs.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Coupdaybs.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Dollarde.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Dollarde.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Coupdays.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Coupdays.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Coupdaysnc.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Coupdaysnc.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/RATE.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/RATE.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Accrintm.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Accrintm.", XLS);
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
//[AMLOEXT-57]
void ScOpenclTest::testStatisticalFormulaNegbinomdist()
{
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Negbinomdist."
        ,XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Negbinomdist."
        ,XLS);
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
//[AMLOEXT-64]
void ScOpenclTest::testFinacialDollarfrFormula()
{
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Dollarfr.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Dollarfr.", XLS);
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
//[AMLOEXT-71]
void ScOpenclTest::testFinacialDISCFormula()
{
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/DISC.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/DISC.", XLS);
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
//[AMLOEXT-75]
void ScOpenclTest::testFinacialINTRATEFormula()
{
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/INTRATE.", XLS);
    enableOpenCL(xDocSh);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/INTRATE.", XLS);
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
//[AMLOEXT-82]
void ScOpenclTest::testStatisticalFormulaPearson()
{
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Pearson.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Pearson.", XLS);
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
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Rsq.", XLS);
    enableOpenCL(xDocSh);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Rsq.", XLS);
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
//[AMLOEXT-90]
void ScOpenclTest::testMathFormulaCsc()
{
    ScDocShellRef xDocSh = loadDoc("OpenCLTests/math/csc.", ODS);
    enableOpenCL(xDocSh);   ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenCLTests/math/csc.", ODS);
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

ScOpenclTest::ScOpenclTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScOpenclTest::setUp()
{
    test::BootstrapFixture::setUp();
    SetEnv();
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
