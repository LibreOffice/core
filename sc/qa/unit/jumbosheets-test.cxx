/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotest/macros_test.hxx>
#include <test/bootstrapfixture.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/keycodes.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <test/xmltesttools.hxx>

#include <defaultsoptions.hxx>
#include <scmod.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <com/sun/star/frame/Desktop.hpp>

#include "helper/qahelper.hxx"
#include "helper/xpath.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Tests for sheets larger than 1024 columns and/or 1048576 rows. */

class ScJumboSheetsTest : public unotest::MacrosTest, public ScBootstrapFixture, public XmlTestTools
{
public:
    ScJumboSheetsTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testRoundtripColumn2000Ods();
    void testRoundtripColumn2000Xlsx();
    void testRoundtripColumnRange();
    void testRoundtripNamedRanges();
    void testNamedRangeNameConflict();
    void testTdf134553();
    void testTdf134392();
    void testTdf147509();
    void testTdf133033();
    void testTdf109061();

    CPPUNIT_TEST_SUITE(ScJumboSheetsTest);

    CPPUNIT_TEST(testRoundtripColumn2000Ods);
    CPPUNIT_TEST(testRoundtripColumn2000Xlsx);
    CPPUNIT_TEST(testRoundtripColumnRange);
    CPPUNIT_TEST(testRoundtripNamedRanges);
    CPPUNIT_TEST(testNamedRangeNameConflict);
    CPPUNIT_TEST(testTdf134553);
    CPPUNIT_TEST(testTdf134392);
    CPPUNIT_TEST(testTdf147509);
    CPPUNIT_TEST(testTdf133033);
    CPPUNIT_TEST(testTdf109061);

    CPPUNIT_TEST_SUITE_END();

protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

private:
    void testRoundtripColumn2000(std::u16string_view name, int format);
};

void ScJumboSheetsTest::testRoundtripColumn2000Ods()
{
    testRoundtripColumn2000(u"value-in-column-2000.", FORMAT_ODS);
}

void ScJumboSheetsTest::testRoundtripColumn2000Xlsx()
{
    testRoundtripColumn2000(u"value-in-column-2000.", FORMAT_XLSX);
}

void ScJumboSheetsTest::testRoundtripColumn2000(std::u16string_view name, int format)
{
    ScDocShellRef xDocSh1 = loadDoc(name, format);

    {
        ScDocument& rDoc = xDocSh1->GetDocument();
        // Check the value at BXX1 (2000th column).
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(1999, 0, 0));
        // Check the formula referencing the value.
        CPPUNIT_ASSERT_EQUAL(OUString("=BXX1"), rDoc.GetFormula(0, 0, 0));
        // Recalc and check value in the reference.
        rDoc.CalcAll();
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(0, 0, 0));
    }

    ScDocShellRef xDocSh2 = saveAndReload(*xDocSh1, format);
    CPPUNIT_ASSERT(xDocSh2.is());

    {
        // Check again.
        ScDocument& rDoc = xDocSh2->GetDocument();
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(1999, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=BXX1"), rDoc.GetFormula(0, 0, 0));
        rDoc.CalcAll();
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(0, 0, 0));
    }

    xDocSh1->DoClose();
    xDocSh2->DoClose();
}

void ScJumboSheetsTest::testRoundtripColumnRange()
{
    ScDocShellRef xDocSh1 = loadDoc(u"sum-whole-column-row.", FORMAT_ODS);

    {
        ScDocument& rDoc = xDocSh1->GetDocument();
        // Check the formula referencing the whole-row range.
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), rDoc.GetFormula(0, 0, 0));
        // Check the formula referencing the whole-column range.
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), rDoc.GetFormula(1, 0, 0));
    }

    std::shared_ptr<utl::TempFileNamed> exportedFile;
    ScDocShellRef xDocSh2 = saveAndReloadNoClose(*xDocSh1, FORMAT_ODS, &exportedFile);
    CPPUNIT_ASSERT(xDocSh2.is());

    {
        ScDocument& rDoc = xDocSh2->GetDocument();
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), rDoc.GetFormula(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), rDoc.GetFormula(1, 0, 0));
        xmlDocUniquePtr pDoc = XPathHelper::parseExport(exportedFile, m_xSFactory, "content.xml");
        CPPUNIT_ASSERT(pDoc);
        assertXPath(pDoc,
                    "/office:document-content/office:body/office:spreadsheet/table:table/"
                    "table:table-row[1]/table:table-cell[1]",
                    "formula", "of:=SUM([.2:.2])");
        assertXPath(pDoc,
                    "/office:document-content/office:body/office:spreadsheet/table:table/"
                    "table:table-row[1]/table:table-cell[2]",
                    "formula", "of:=SUM([.C:.C])");
    }

    ScDocShellRef xDocSh3 = saveAndReloadNoClose(*xDocSh1, FORMAT_XLSX, &exportedFile);
    CPPUNIT_ASSERT(xDocSh3.is());

    {
        ScDocument& rDoc = xDocSh3->GetDocument();
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), rDoc.GetFormula(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), rDoc.GetFormula(1, 0, 0));
        xmlDocUniquePtr pDoc
            = XPathHelper::parseExport(exportedFile, m_xSFactory, "xl/worksheets/sheet1.xml");
        CPPUNIT_ASSERT(pDoc);
        assertXPathContent(pDoc, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]/x:f", "SUM(2:2)");
        assertXPathContent(pDoc, "/x:worksheet/x:sheetData/x:row[1]/x:c[2]/x:f", "SUM(C:C)");
    }

    xDocSh1->DoClose();
    xDocSh2->DoClose();
    xDocSh3->DoClose();
}

void ScJumboSheetsTest::testRoundtripNamedRanges()
{
    ScDocShellRef xDocSh1 = loadDoc(u"ranges-column-2000.", FORMAT_ODS);

    std::pair<OUString, OUString> ranges[] = { { "CELLBXX1", "$Sheet1.$BXX$1" },
                                               { "CELLSA4_AMJ4", "$Sheet1.$A$4:$AMJ$4" },
                                               { "CELLSBXX1_BXX10", "$Sheet1.$BXX$1:$BXX$10" },
                                               { "CELLSBXX1_BXX10_RELATIVE", "$Sheet1.BXX1:BXX10" },
                                               { "CELLSE1_E1024", "$Sheet1.$E$1:$E$1024" },
                                               { "CELLSE1_E2000000", "$Sheet1.$E$1:$E$2000000" },
                                               { "COLUMN_E", "$Sheet1.$E:$E" },
                                               { "ROW_4", "$Sheet1.$4:$4" } };
    {
        ScDocument& rDoc = xDocSh1->GetDocument();
        for (const auto& range : ranges)
        {
            ScRangeData* rangeData = rDoc.GetRangeName()->findByUpperName(range.first);
            CPPUNIT_ASSERT(rangeData);
            CPPUNIT_ASSERT_EQUAL(range.second, rangeData->GetSymbol());
        }
    }

    std::shared_ptr<utl::TempFileNamed> exportedFile;
    ScDocShellRef xDocSh2 = saveAndReloadNoClose(*xDocSh1, FORMAT_ODS, &exportedFile);
    CPPUNIT_ASSERT(xDocSh2.is());

    {
        ScDocument& rDoc = xDocSh2->GetDocument();
        for (const auto& range : ranges)
        {
            ScRangeData* rangeData = rDoc.GetRangeName()->findByUpperName(range.first);
            CPPUNIT_ASSERT(rangeData);
            CPPUNIT_ASSERT_EQUAL(range.second, rangeData->GetSymbol());
        }
    }

    ScDocShellRef xDocSh3 = saveAndReloadNoClose(*xDocSh1, FORMAT_XLSX, &exportedFile);
    CPPUNIT_ASSERT(xDocSh3.is());

    {
        ScDocument& rDoc = xDocSh3->GetDocument();
        for (const auto& range : ranges)
        {
            ScRangeData* rangeData = rDoc.GetRangeName()->findByUpperName(range.first);
            CPPUNIT_ASSERT(rangeData);
            CPPUNIT_ASSERT_EQUAL(range.second, rangeData->GetSymbol());
        }
    }

    xDocSh1->DoClose();
    xDocSh2->DoClose();
    xDocSh3->DoClose();
}

void ScJumboSheetsTest::testNamedRangeNameConflict()
{
    // The document contains named ranges named 'num1' and 'num2', that should be still treated
    // as named references even though with 16k columns those are normally NUM1 and NUM2 cells.
    ScDocShellRef xDocSh = loadDoc(u"named-range-conflict.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(10022, 0, 0)); // NUM1
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(10022, 1, 0)); // NUM2
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(0, 0, 0)); // = num1
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(0, 1, 0)); // = sheet2.num2
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(0, 2, 0)); // = SUM(NUM1:NUM2) (not named ranges)
    rDoc.SetValue(10022, 0, 0, 100); // NUM1
    rDoc.SetValue(10022, 1, 0, 200); // NUM2
    rDoc.CalcAll();
    // First two are the same, the sum changes.
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(300.0, rDoc.GetValue(0, 2, 0));

    xDocSh->DoClose();
}

void ScJumboSheetsTest::testTdf134553()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf134553.", FORMAT_XLSX);

    ScDocument& rDoc = xDocSh->GetDocument();

    const SdrOle2Obj* pOleObj = getSingleChartObject(rDoc, 0);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(pOleObj);

    // Sorry, the charts so severely suffer from DPI dependency, that I can't find motivation
    // to add huge tolerances (around 350!) here to make it pass on non-default DPI, with no
    // guarantee that the test would have any value after that. So just skip it.
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    CPPUNIT_ASSERT_EQUAL(tools::Long(12741), pOleObj->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(7620), pOleObj->GetLogicRect().getOpenHeight());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4574), pOleObj->GetLogicRect().getX());
    CPPUNIT_ASSERT_EQUAL(tools::Long(437), pOleObj->GetLogicRect().getY());

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    pViewShell->SelectObject(u"Diagram 1");

    uno::Reference<lang::XComponent> xComponent = xDocSh->GetModel();
    dispatchCommand(xComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    dispatchCommand(xComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    pOleObj = getSingleChartObject(rDoc, 0);
    CPPUNIT_ASSERT(pOleObj);

    CPPUNIT_ASSERT_EQUAL(tools::Long(12741), pOleObj->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(7620), pOleObj->GetLogicRect().getOpenHeight());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5097), pOleObj->GetLogicRect().getX());
    // tdf#147458: Without the fix in place, this test would have failed with
    // - Expected: 1058
    // - Actual  : -7421
    CPPUNIT_ASSERT_EQUAL(tools::Long(1058), pOleObj->GetLogicRect().getY());

    xDocSh->DoClose();
}

void ScJumboSheetsTest::testTdf134392()
{
    // Without the fix in place, the file would have crashed
    ScDocShellRef xDocSh = loadDoc(u"tdf134392.", FORMAT_XLSX);

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.
    xDocSh->DoClose();
}

void ScJumboSheetsTest::testTdf147509()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDoc = xDocSh->GetDocument();

    rDoc.SetString(0, 0, 0, "A");
    rDoc.SetString(1, 0, 0, "B");

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);
    ScViewData& rViewData = pViewShell->GetViewData();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rViewData.GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rViewData.GetCurY());

    uno::Reference<lang::XComponent> xComponent = xDocSh->GetModel();
    dispatchCommand(xComponent, ".uno:SelectColumn", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(xComponent, ".uno:InsertColumnsAfter", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("A"), rDoc.GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : B
    CPPUNIT_ASSERT_EQUAL(OUString(""), rDoc.GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("B"), rDoc.GetString(ScAddress(2, 0, 0)));

    xDocSh->DoClose();
}

void ScJumboSheetsTest::testTdf133033()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    uno::Reference<lang::XComponent> xComponent = xDocSh->GetModel();
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(xComponent.get());
    CPPUNIT_ASSERT(pModelObj);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    ScViewData& rViewData = pViewShell->GetViewData();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rViewData.GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16777215), rViewData.GetCurY());

    xDocSh->DoClose();
}

void ScJumboSheetsTest::testTdf109061()
{
    // Without the fix in place, the file would have crashed
    ScDocShellRef xDocSh = loadDoc(u"tdf109061.", FORMAT_XLSX);

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    CPPUNIT_ASSERT_EQUAL(6.0, rDoc.GetValue(1, 3, 0));

    xDocSh->DoClose();
}

ScJumboSheetsTest::ScJumboSheetsTest()
    : ScBootstrapFixture("sc/qa/unit/data")
{
}

void ScJumboSheetsTest::setUp()
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

void ScJumboSheetsTest::tearDown()
{
    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(false);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);

    ScBootstrapFixture::tearDown();
}

void ScJumboSheetsTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScJumboSheetsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
