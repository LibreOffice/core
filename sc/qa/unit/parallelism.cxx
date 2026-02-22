/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_feature_opencl.h>

#include "helper/qahelper.hxx"

#include <document.hxx>
#include <formulagroup.hxx>

#include <officecfg/Office/Calc.hxx>
#include <unotools/syslocaleoptions.hxx>

using namespace sc;

// test-suite suitable for loading documents to test parallelism in
// with access only to exported symbols

class ScParallelismTest : public ScModelTestBase
{
public:
    ScParallelismTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }

    virtual void setUp() override;
    virtual void tearDown() override;

private:
    bool getThreadingFlag() const;
    void setThreadingFlag(bool bSet);

    bool m_bThreadingFlagCfg;
};

bool ScParallelismTest::getThreadingFlag() const
{
    return officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::
        get();
}

void ScParallelismTest::setThreadingFlag(bool bSet)
{
    std::shared_ptr<comphelper::ConfigurationChanges> xBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::set(
        bSet, xBatch);
    xBatch->commit();
}

void ScParallelismTest::setUp()
{
    ScModelTestBase::setUp();

#if HAVE_FEATURE_OPENCL
    sc::FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly();
#endif

    m_bThreadingFlagCfg = getThreadingFlag();
    if (!m_bThreadingFlagCfg)
        setThreadingFlag(true);
}

void ScParallelismTest::tearDown()
{
    // Restore threading flag
    if (!m_bThreadingFlagCfg)
        setThreadingFlag(false);

    ScModelTestBase::tearDown();
}

// Dependency range in this document was detected as I9:I9 instead of expected I9:I367
CPPUNIT_TEST_FIXTURE(ScParallelismTest, testTdf160368)
{
    createScDoc("ods/tdf160368.ods");
    ScDocShell* pDocSh = getScDocShell();
    // without fix: ScFormulaCell::MaybeInterpret(): Assertion `!rDocument.IsThreadedGroupCalcInProgress()' failed.
    pDocSh->DoHardRecalc();
}

// VLOOKUP with CONCATENATE involving decimal numbers returns #N/A
// under threaded calculation when locale uses comma as decimal separator
CPPUNIT_TEST_FIXTURE(ScParallelismTest, testTdf167636)
{
    // Set locale to French (comma decimal separator) for the duration of the test
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString(u"fr-FR"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createScDoc("ods/tdf167636.ods");

    ScDocument* pDoc = getScDoc();

    // Recalculate via CalcFormulaTree which uses threaded group calculation.
    pDoc->CalcFormulaTree(false, false, true);

    // After threaded recalc with French locale, VLOOKUP formulas
    // involving CONCATENATE with decimal numbers should return correct
    // results, not #N/A.

    // D2 = 14 * 10 = 140
    CPPUNIT_ASSERT_DOUBLES_EQUAL(140.0, pDoc->GetValue(3, 1, 0), 1e-10);
    // D3 = 16 * 20 = 320
    CPPUNIT_ASSERT_DOUBLES_EQUAL(320.0, pDoc->GetValue(3, 2, 0), 1e-10);
    // D4 = 12 * 30 = 360
    CPPUNIT_ASSERT_DOUBLES_EQUAL(360.0, pDoc->GetValue(3, 3, 0), 1e-10);
    // D5 = 6 * 40 = 240
    CPPUNIT_ASSERT_DOUBLES_EQUAL(240.0, pDoc->GetValue(3, 4, 0), 1e-10);
    // D6 = 13 * 50 = 650
    CPPUNIT_ASSERT_DOUBLES_EQUAL(650.0, pDoc->GetValue(3, 5, 0), 1e-10);
    // D7 = 7 * 60 = 420
    CPPUNIT_ASSERT_DOUBLES_EQUAL(420.0, pDoc->GetValue(3, 6, 0), 1e-10);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
