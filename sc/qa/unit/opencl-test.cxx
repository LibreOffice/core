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
    bool enableOpenCL();
    void SetEnv();
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
    CPPUNIT_TEST_SUITE(ScOpenclTest);
//  CPPUNIT_TEST(testSharedFormulaXLS);
//  CPPUNIT_TEST(testFinacialFormula);
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

bool ScOpenclTest::enableOpenCL()
{
    sc::FormulaGroupInterpreter::enableOpenCL(true);
    return sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(),true);
}

void ScOpenclTest::testCompilerNested()
{
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenCLTests/Compiler/nested.", ODS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenCLTests/Compiler/string.", ODS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenCLTests/Compiler/ineq.", ODS);
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

#if 0
void ScOpenclTest::testSharedFormulaXLSStockHistory()
{
    if (!enableOpenCL())
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
    if (!enableOpenCL())
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("sum_ex.", XLS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenclCase/math/cos.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("FinancialFormulaTest.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("FinancialFormulaTest.", XLS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenCLTests/statistical/Correl.", ODS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Fisher.", XLS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/FisherInv.", XLS);
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
    if (!enableOpenCL())
        return;

    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Gamma.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Fvschedule.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/SYD.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/IRR.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/GammaLn.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Gauss.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/GeoMean.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/HarMean.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/SLN.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/MIRR.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Coupdaybs.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Dollarde.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Coupdays.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Coupdaysnc.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/RATE.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Accrintm.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Negbinomdist." ,XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/statistical/Negbinomdist." ,XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Dollarfr.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/DISC.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/INTRATE.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Pearson.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/statistical/Rsq.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenCLTests/math/csc.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
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
//[AMLOEXT-92]
void ScOpenclTest::testFinacialXNPVFormula()
{
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/XNPV.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/XNPV.", XLS);
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
void ScOpenclTest::testFinacialPriceMatFormula()
{
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/PriceMat.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/PriceMat.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Received.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Received.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Cumipmt.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Cumipmt.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Cumprinc.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Cumprinc.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/RRI.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/RRI.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/EFFECT_ADD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/EFFECT_ADD.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/Nominal.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);   xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/Nominal.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/TBILLEQ.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/TBILLEQ.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/TBILLPRICE.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/TBILLPRICE.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/TBILLYIELD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/TBILLYIELD.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/YIELD.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/YIELD.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/YIELDDISC.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/YIELDDISC.", XLS);
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
    if (!enableOpenCL())
        return;
    ScDocShellRef xDocSh = loadDoc("OpenclCase/financial/YIELDMAT.", XLS);
    ScDocument *pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    ScDocShellRef xDocShRes = loadDoc("OpenclCase/financial/YIELDMAT.", XLS);
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
