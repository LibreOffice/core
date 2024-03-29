/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "helper/qahelper.hxx"

#include <document.hxx>
#include <formulagroup.hxx>

#include <officecfg/Office/Calc.hxx>

using namespace sc;

// test-suite suitable for loading documents to test parallelism in
// with access only to exported symbols

class ScParallelismTest : public ScModelTestBase
{
public:
    ScParallelismTest()
        : ScModelTestBase("sc/qa/unit/data")
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

    sc::FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly();

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
