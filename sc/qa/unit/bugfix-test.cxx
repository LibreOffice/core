/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "validat.hxx"
#include "tabvwsh.hxx"
#include <com/sun/star/frame/Desktop.hpp>
#include "helper/qahelper.hxx"

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
    void testTdf88821();
    void testTdf88821_2();
    void testTdf103960();
    void testRhbz1390776();
    void testTdf104310();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testTdf64229);
    CPPUNIT_TEST(testTdf36933);
    CPPUNIT_TEST(testTdf43700);
    CPPUNIT_TEST(testTdf43534);
    CPPUNIT_TEST(testTdf91979);
    // CPPUNIT_TEST(testTdf40110);
    CPPUNIT_TEST(testTdf98657);
    CPPUNIT_TEST(testTdf88821);
    CPPUNIT_TEST(testTdf88821_2);
    CPPUNIT_TEST(testTdf103960);
    CPPUNIT_TEST(testRhbz1390776);
    CPPUNIT_TEST(testTdf104310);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

void ScFiltersTest::testTdf64229()
{
    ScDocShellRef xDocSh = loadDoc("fdo64229b.", FORMAT_ODS);

    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo64229b.*", xDocSh.is());
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

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo36933test.*", xDocSh.is());
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

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo43700test.*", xDocSh.is());
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

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo43534test.*", xDocSh.is());
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
    CPPUNIT_ASSERT(xDocSh.get() != nullptr);

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);
    auto& aViewData = pViewShell->GetViewData();
    auto* pDoc = aViewData.GetDocument();

    // Check coordinates of a distant cell
    Point aPos = aViewData.GetScrPos(MAXCOL - 1, 10000, SC_SPLIT_TOPLEFT, true);
    int nColWidth = ScViewData::ToPixel(pDoc->GetColWidth(0, 0), aViewData.GetPPTX());
    int nRowHeight = ScViewData::ToPixel(pDoc->GetRowHeight(0, 0), aViewData.GetPPTY());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>((MAXCOL - 1) * nColWidth), aPos.getX());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10000 * nRowHeight), aPos.getY());

    xComponent->dispose();
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

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf88821()
{
    ScDocShellRef xDocSh = loadDoc("tdf88821.", FORMAT_HTML);
    ScDocument& rDoc = xDocSh->GetDocument();

    // B2 should be 'Périmètre', not 'PÃ©rimÃ¨tre'
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("P\xC3\xA9rim\xC3\xA8tre", RTL_TEXTENCODING_UTF8), rDoc.GetString(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf88821_2()
{
    ScDocShellRef xDocSh = loadDoc("tdf88821-2.", FORMAT_HTML);
    ScDocument& rDoc = xDocSh->GetDocument();

    // A2 should be 'ABCabcČŠŽčšž', not 'ABCabcÄŒÅ Å½ÄﾍÅ¡Å¾'
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("ABCabc\xC4\x8C\xC5\xA0\xC5\xBD\xC4\x8D\xC5\xA1\xC5\xBE", RTL_TEXTENCODING_UTF8), rDoc.GetString(0, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf103960()
{
    ScDocShellRef xDocSh = loadDoc("tdf103960.", FORMAT_HTML);
    ScDocument& rDoc = xDocSh->GetDocument();

    // A1 should be 'Data', not the entire content of the file
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("Data", RTL_TEXTENCODING_UTF8), rDoc.GetString(0, 0, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testRhbz1390776()
{
    ScDocShellRef xDocSh = loadDoc("rhbz1390776.", FORMAT_XLS_XML);
    ScDocument& rDoc = xDocSh->GetDocument();

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0, 27, 0), "SUM(A18:A23)", "Wrong range");

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf104310()
{
    // 1. Test x14 extension
    {
        ScDocShellRef xDocSh = loadDoc("tdf104310.", FORMAT_XLSX);
        ScDocument& rDoc = xDocSh->GetDocument();

        const ScValidationData* pData = rDoc.GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        // Make sure the list is correct.
        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));
        CPPUNIT_ASSERT_EQUAL(size_t(5), aList.size());
        for (size_t i = 0; i < 5; ++i)
            CPPUNIT_ASSERT_DOUBLES_EQUAL(double(i + 1), aList[i].GetValue(), 1e-8);

        xDocSh->DoClose();
    }

    // 2. Test x12ac extension
    {
        ScDocShellRef xDocSh = loadDoc("tdf104310-2.", FORMAT_XLSX);
        ScDocument& rDoc = xDocSh->GetDocument();

        const ScValidationData* pData = rDoc.GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        // Make sure the list is correct.
        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));
        CPPUNIT_ASSERT_EQUAL(size_t(3), aList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("1"),   aList[0].GetString());
        CPPUNIT_ASSERT_EQUAL(OUString("2,3"), aList[1].GetString());
        CPPUNIT_ASSERT_EQUAL(OUString("4"),   aList[2].GetString());

        xDocSh->DoClose();
    }
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
