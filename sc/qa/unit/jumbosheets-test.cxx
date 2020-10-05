/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/keycodes.hxx>
#include <comphelper/processfactory.hxx>

#include <defaultsoptions.hxx>
#include <scmod.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <com/sun/star/frame/Desktop.hpp>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScFiltersTest : public test::FiltersTest, public ScBootstrapFixture
{
public:
    ScFiltersTest();

    virtual bool load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                      SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                      unsigned int nFilterVersion) override;

    virtual void setUp() override;
    virtual void tearDown() override;

    void testTdf134392();
    void testTdf133033();

    CPPUNIT_TEST_SUITE(ScFiltersTest);

    CPPUNIT_TEST(testTdf134392);
    CPPUNIT_TEST(testTdf133033);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScFiltersTest::load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                         SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                         unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData, OUString(),
                                                       nFilterFlags, nClipboardID, nFilterVersion);
    bool bLoaded = xDocShRef.is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScFiltersTest::testTdf134392()
{
    // Without the fix in place, the file would have crashed
    ScDocShellRef xDocSh = loadDoc("tdf134392.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.
    xDocSh->DoClose();
}

void ScFiltersTest::testTdf133033()
{
    // Create an empty document
    uno::Reference<frame::XDesktop2> xDesktop
        = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    Sequence<beans::PropertyValue> args(1);
    args[0].Name = "Hidden";
    args[0].Value <<= true;

    m_xCalcComponent = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, args);
    CPPUNIT_ASSERT(m_xCalcComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(m_xCalcComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(m_xCalcComponent.get());
    CPPUNIT_ASSERT(pModelObj);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    ScViewData& rViewData = pViewShell->GetViewData();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rViewData.GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15999999), rViewData.GetCurY());
}

ScFiltersTest::ScFiltersTest()
    : ScBootstrapFixture("sc/qa/unit/data")
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());

    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(true);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);
}

void ScFiltersTest::tearDown()
{
    uno::Reference<lang::XComponent>(m_xCalcComponent, UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();

    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(false);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
