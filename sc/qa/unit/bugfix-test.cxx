/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <svx/svdograf.hxx>

#include "drwlayer.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/borderline.hxx>
#include <editeng/flditem.hxx>
#include <dbdata.hxx>
#include "validat.hxx"
#include "formulacell.hxx"
#include "userdat.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "stlsheet.hxx"
#include "docfunc.hxx"
#include "markdata.hxx"
#include "colorscale.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "editutil.hxx"
#include "cellvalue.hxx"
#include "attrib.hxx"
#include "dpshttab.hxx"
#include "tabvwsh.hxx"
#include <scopetools.hxx>
#include <columnspanset.hxx>
#include <tokenstringcontext.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScFiltersTest : public ScBootstrapFixture
{
public:

    ScFiltersTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testTdf64229();
    void testTdf36933();
    void testTdf43700();
    void testTdf43534();
    void testTdf91979();
    // void testTdf40110();
    void testTdf98657();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testTdf64229);
    CPPUNIT_TEST(testTdf36933);
    CPPUNIT_TEST(testTdf43700);
    CPPUNIT_TEST(testTdf43534);
    CPPUNIT_TEST(testTdf91979);
    // CPPUNIT_TEST(testTdf40110);
    CPPUNIT_TEST(testTdf98657);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

void ScFiltersTest::testTdf64229()
{
    ScDocShellRef xDocSh = loadDoc("fdo64229b.", FORMAT_ODS);

    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo64229b.*", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo64229b.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf36933()
{
    ScDocShellRef xDocSh = loadDoc("fdo36933test.", FORMAT_ODS);

    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo36933test.*", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo36933test.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf43700()
{
    ScDocShellRef xDocSh = loadDoc("fdo43700test.", FORMAT_ODS);

    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo43700test.*", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo43700test.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf43534()
{
    ScDocShellRef xDocSh = loadDoc("fdo43534test.", FORMAT_ODS);

    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo43534test.*", xDocSh.Is());
    // ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo43534test.", aCSVFileName);
    // testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf91979()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    Sequence < beans::PropertyValue > args(1);
    args[0].Name = "Hidden";
    args[0].Value <<= true;

    uno::Reference< lang::XComponent > xComponent = xDesktop->loadComponentFromURL(
        "private:factory/scalc",
        "_blank",
        0,
        args);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != nullptr);

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);
    auto& aViewData = pViewShell->GetViewData();
    auto* pDoc = aViewData.GetDocument();

    // Check coordinates of a distant cell
    Point aPos = aViewData.GetScrPos(MAXCOL - 1, 10000, SC_SPLIT_TOPLEFT, true);
    int nColWidth = ScViewData::ToPixel(pDoc->GetColWidth(0, 0), aViewData.GetPPTX());
    int nRowHeight = ScViewData::ToPixel(pDoc->GetRowHeight(0, 0), aViewData.GetPPTY());
    CPPUNIT_ASSERT(aPos.getX() == (MAXCOL - 1) * nColWidth);
    CPPUNIT_ASSERT(aPos.getY() == 10000 * nRowHeight);

    xDocSh->DoClose();
}

/*
void ScFiltersTest::testTdf40110()
{
    ScDocShellRef xDocSh = loadDoc("fdo40110test.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo40110test.*", xDocSh.Is());
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath(OUString("fdo40110test."), aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}
*/

void ScFiltersTest::testTdf98657()
{
    ScDocShellRef xDocSh = loadDoc("tdf98657.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    xDocSh->DoHardRecalc(true);

    // this was a NaN before the fix
    CPPUNIT_ASSERT_EQUAL(double(285.0), rDoc.GetValue(ScAddress(1, 1, 0)));
}


ScFiltersTest::ScFiltersTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScFiltersTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
