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
    virtual void setUp();
    virtual void tearDown();

    virtual bool load( const OUString &rFilter, const OUString &rURL,
            const OUString &rUserData, unsigned int nFilterFlags,
            unsigned int nClipboardID, unsigned int nFilterVersion);
    void testSharedFormulaXLS();
    CPPUNIT_TEST_SUITE(ScOpenclTest);
    CPPUNIT_TEST(testSharedFormulaXLS);
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

void ScOpenclTest::enableOpenCL(ScDocShell* pShell)
{
    ScModule* pScMod = SC_MOD();
    ScFormulaOptions rOpt = pScMod->GetFormulaOptions();
    ScCalcConfig maSavedConfig = rOpt.GetCalcConfig();
    maSavedConfig.mbOpenCLEnabled = true;
    rOpt.SetCalcConfig(maSavedConfig);
    pShell->SetFormulaOptions(rOpt);
}

#if 0
void ScOpenclTest::testSharedFormulaXLS()
{
    ScDocShellRef xDocSh = loadDoc("ground-water-daily.", XLS);

    enableOpenCL(xDocSh);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);

    ScDocShellRef xDocShRes = loadDoc("ground-water-daily-result.", XLS);
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
#else
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

    xDocSh->DoClose();
    xDocShRes->DoClose();
}


#endif

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
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScOpenclTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOpenclTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
